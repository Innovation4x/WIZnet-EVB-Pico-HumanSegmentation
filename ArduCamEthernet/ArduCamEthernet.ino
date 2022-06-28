/*
  Telnet client

 This sketch connects to a a telnet server (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.  You'll need a telnet server
 to test this with.
 Processing's ChatServer example (part of the network library) works well,
 running on port 10002. It can be found as part of the examples
 in the Processing application, available at
 http://processing.org/

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 14 Sep 2010
 modified 9 Apr 2012
 by Tom Igoe
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <SPI.h>
#include "ArduCAM_OV2640.h"
#include "memorysaver.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 100, 3);
//IPAddress ip(10, 5, 15, 109);

// Enter the IP address of the server you're connecting to:
IPAddress server(192, 168, 100, 5);
//IPAddress myDns(192, 168, 100, 1);
//IPAddress server(10, 5, 15, 103);
IPAddress myDns(192, 168, 100, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet;
// if you're using Processing's ChatServer, use port 10002):
#define max_transfer 1024
#define max_buffer  (20 * max_transfer)

EthernetClient client;
byte img_buf[max_buffer];

void ethernet_transfer(byte *bptr, size_t len) {
  size_t sent = 0;
  for (; sent + max_transfer < len; ) {
    client.write(bptr, max_transfer);
    sent += max_transfer;
    bptr += max_transfer;
    //Serial.println(max_transfer);
  }
  client.write(bptr, len - sent);

  Serial.print("Sent: "); Serial.println(len);
}

//=========================================================
void ArduCam_setup();
void ArduCam_sendImg();

//const int CS = 5; //GPIO-5 for SPI
const int CS = 13;  //GPIO-13 for SPI1
uint8_t vid, pid;
ArduCAM myCAM( OV2640, CS );

void ArduCam_setup() {
  uint8_t temp;

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  Wire.setSDA(8);
  Wire.setSCL(9);
  // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
  
  // Using SPI
  //SPI.setSCK(2);
  //SPI.setTX(3);
  //SPI.setRX(4);
  //SPI.setCS(CS);
  //myCAM.Arducam_init(&SPI, &Wire);
  
  // Using SPI1
  SPI1.setSCK(10);
  SPI1.setTX(11);
  SPI1.setRX(12);
  SPI1.setCS(CS);
  myCAM.Arducam_init(&SPI1, &Wire);
  
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  while(1){
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55){
      Serial.println(temp);
      Serial.println("ACK CMD SPI interface Error! END");
      delay(1000);
      continue;
    }else{
      Serial.println("ACK CMD SPI interface OK. END");
      break;
    }
  }

  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println("ACK CMD Can't find OV2640 module! END");
      delay(1000);continue;
    }
    else{
      Serial.println("ACK CMD OV2640 detected. END");break;
    } 
  }

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  //myCAM.OV2640_set_JPEG_size(OV2640_176x144);
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  //myCAM.OV2640_set_JPEG_size(OV2640_640x480);
  delay(50);
  myCAM.OV2640_set_Light_Mode(Auto);
  delay(50);
  //myCAM.OV2640_set_Color_Saturation(Saturation0);
  myCAM.OV2640_set_Color_Saturation(Saturation0);
  delay(50);
  //myCAM.OV2640_set_Brightness(Brightness0);
  myCAM.OV2640_set_Brightness(Brightness0);
  delay(50);
  //myCAM.OV2640_set_Contrast(Contrast0);
  myCAM.OV2640_set_Contrast(Contrast0);
  delay(50);
  myCAM.OV2640_set_Special_effects(Bluish);
  //myCAM.OV2640_set_Special_effects(Greenish);
  //myCAM.OV2640_set_Special_effects(Reddish);
  //myCAM.OV2640_set_Special_effects(BW);
  //myCAM.OV2640_set_Special_effects(Negative);
  //myCAM.OV2640_set_Special_effects(BWnegative);
  //myCAM.OV2640_set_Special_effects(Antique);
  myCAM.OV2640_set_Special_effects(Normal);
  myCAM.OV2640_set_Light_Mode(Office);
  delay(1000);
  myCAM.clear_fifo_flag();
  delay(50);
}

/*
void ArduCam_sendImg() {
  uint32_t length = 0;
  uint32_t read_len = 0;
  
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    delay(10);

  Serial.print("CAM Capture Done. ");
  delay(50);
  
  //read_fifo_burst(myCAM);
  length = myCAM.read_fifo_length();
  Serial.println(length);

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode

  while (1) {
    if (length > max_transfer) {
      read_len = myCAM.transferBytes((uint8_t*)NULL, (uint8_t*)img_buf, (uint32_t)max_transfer);
      Serial.println(read_len);
      ethernet_transfer(img_buf, read_len);
      length -= read_len;
    } else {
      read_len = myCAM.transferBytes((uint8_t*)NULL, (uint8_t*)img_buf, (uint32_t)length);
      Serial.println(read_len);
      ethernet_transfer(img_buf, read_len);
      break;
    }
    //delayMicroseconds(15);
  }

  myCAM.CS_HIGH();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
}
/*/
void ArduCam_sendImg() {
  uint32_t length = 0;
  uint32_t read_len = 0;
  byte *bptr;
  
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();

  Serial.println("Wait CAP_DONE_MASK...");
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    delay(1);

  Serial.print("CAM Image Ready.");
  delay(1);
  
  //read_fifo_burst(myCAM);
  length = myCAM.read_fifo_length();
  Serial.println(length);

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode

  bptr = img_buf;
  
  // Read image from ArduCam
  while (1) {
    if (length > max_transfer) {
      read_len = myCAM.transferBytes((uint8_t*)NULL, (uint8_t*)bptr, (uint32_t)max_transfer);
      //Serial.println(read_len);
      //ethernet_transfer(buf, read_len);
      length -= read_len;
      bptr += read_len;
    } else {
      read_len = myCAM.transferBytes((uint8_t*)NULL, (uint8_t*)bptr, (uint32_t)length);
      //Serial.println(read_len);
      //ethernet_transfer(buf, read_len);
      break;
    }
    //delayMicroseconds(15);
  }

  myCAM.CS_HIGH();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  Serial.println("Copied to buffer");

  // Find end of image
  bptr = img_buf+2;
  for (length = 2; length<max_buffer ;++length, ++bptr) {
    if (*bptr == 0xFF) {
      if (*(bptr+1) == 0xD9) {
        Serial.print("End of image. New length = ");
        length += 2;  // Include 0xFF 0xD9
        Serial.println(length);
        break;
      }
    }
  }

  // Sned image to server.
  ethernet_transfer(img_buf, length);

  //delay(10);
  //return length
}
//*/

//=========================================================

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
  Ethernet.init(17);  // WIZnet EVB-Pico (GPIO-17)
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

   // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  /*
  //SPIClassRP2040 SPI1(spi1, PIN_SPI1_MISO, PIN_SPI1_SS, PIN_SPI1_SCK, PIN_SPI1_MOSI);
  Serial.print("MISO="); Serial.println(PIN_SPI1_MISO);
  Serial.print("SS="); Serial.println(PIN_SPI1_SS);
  Serial.print("SCLK="); Serial.println(PIN_SPI1_SCK);
  Serial.print("MOSI="); Serial.println(PIN_SPI1_MOSI);
  */
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 12345)) {
    Serial.println("connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  // Initial setup for ArduCam
  ArduCam_setup();
}

int count = 0;
char c = 0;
bool is_capture = false;

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    c = client.read();
    Serial.print((int)c);
  }

  /*
  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  while (Serial.available() > 0) {
    c = Serial.read();
    if (c == 'c') is_capture = true;
    Serial.print(c);    
//    if (client.connected()) {
//      client.print(c);
//    }
  }
  */
  
  /*
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    // do nothing:
    while (true) {
      delay(1);
    }
  }
  */
  
  // if the server's disconnected, reconnect the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnected. Reconnecting...");
    if (client.connect(server, 12345)) {
      Serial.println("connected");
      count = 0;
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
      delay(1000);
    }
  }

  // if the server is ready, send captured image:
  if (c == 0x96)
  {
    c = 0;
    ArduCam_sendImg();
    count++;
    Serial.println(count);
  }

  /*
  //if (count >= 100) {
  if (count >= 1) {
    Serial.println("Finished!!");
    // do nothing:
    while (true) {
      delay(1);
    }
  }
  //*/
}
