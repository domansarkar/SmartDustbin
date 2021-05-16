/*
 * SMART DUSTIN project aims towards the main problem faced by many people in times of a global pendamic, as we go somewhere (hospitals or grocery stores) and if we want to throw 
 * stuff then we have to physically touch the lid which can help the virus to come in contact with our skin. So this project will not really help you to collect the garbage but also
 * will help you to monitor the garbage and alert you if there is garbage left from previous days. 
 * 
 * COMPONENTS :-
 * s.no.  component                       quantity       other info
 * 1.     ESP8266(WIFI)                   01             any arduino ide compatible microcontroller with wifi (if you are not using the written components then some changes needs to be made in code)
 * 2.     HC-SR04 ultrasonic sensor       02
 * 3.     SSD1306 128x64 oled display     01
 * 4.     servo 9gm                       01
 * 
 * SOFTWARE NEEDED :-
 * >ARDUINO IDE (necessary)
 * >lcd-image-converter (not necessary)------- link- https://sourceforge.net/projects/lcd-image-converter/files/lcd-image-converter-20190317.zip/download
 * 
 * CONNECTIONS :-
 * OLED--------ESP8266
 * VCC          VCC
 * GND          GND
 * SCL          SCL
 * SDA          SDA
 * 
 * HC-SR04 (hand detectin)-----ESP8266
 * VCC                          VCC
 * GND                          GND
 * TRIG                         GPIO 02
 * ECHO                         GPIO 0
 * 
 * HC-SR04 (garbage level)-----ESP8266
 * VCC                          VCC
 * GND                          GND
 * TRIG                         GPIO 02
 * ECHO                         GPIO 15
 * 
 * SERVO---------ESP8266
 * VCC            VCC
 * GND            GND
 * PWM            GPIO 13
 * 
 * CAUTION!
 * MAKE SHURE YOU HAVE WORKING WIFI AND THE WIFI SSID AND PASSWORD VARIABLES ARE CHANGED ACCORDINGLY(ssid, password)
 * MAKE SHURE YOU CHANGE THE canlen VARIABLE TO YOU CHOICE
 * 
 * This project is made by Doman Sarkar
*/


////////////////HEADERS/////////////////
#include <ESP8266WiFi.h>        //header file for esp8266
#include "NTPClient.h"          //header file for NTP for date & time
#include "WiFiUdp.h"            //sending and receiving data through wifi
#include <EEPROM.h>             //header file for EEPROM for storing data permanent
#include <SPI.h>                //header for spi 
#include <Wire.h>               //header contains default wiring for boards
#include <Adafruit_GFX.h>       //gfx header for display
#include <Adafruit_SSD1306.h>   //header file for ssd 1306 display
#include<Servo.h>               //header for servo


//////////////VARIABLES MACROS AND OBJECTS//////////////////
Servo lid;                      //object of servo - lid

#define lidPin 13               //servo signal pin for controlling the lid

#define SCREEN_WIDTH 128        //oled display screen width in pixels
#define SCREEN_HEIGHT 64        //oled display screen height in pixels

#define OLED_RESET     -1       //since we dont have reset pin in the display so pin is set to -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   //creating and instantiating the object of the display

#define LOGO_HEIGHT   64        //this is the height of the bitmap image we draw on screen (all of them are of same dimentions)
#define LOGO_WIDTH    64        //this is the width of the bitmap image we draw on screen (all of them are of same dimentions)

#define echoPin 0               //echo pin for the hand detecting ultra sonic sensor and it is shared by the garbage level sensor
#define trigPin 2               //trigger pin for the hand detecting sensor
#define echoPin1 15             //echo pin for the garbage level sensor

long duration;                  //duration after which the hand detection sensor receives the triggered signal
int distance;                   //for storing the distance value for the hand detection sensor
long duration1;                 //variable for the garbage level sensor
int distance1;                  //for storing the distance value for the garbage level sensor

const char* ssid = "SSID";      //wifi ssid/name 
const char* password = "PASSWORD";   //wifi password

int day;                        //variable for storing the day of week value

String notice;                  //variable for storing the message for new day otherwise it is null

float coll=0, canlen=28;        //coll for collected garbage and canlen for the length of the garbage can (in our case 28 cm)

WiFiServer server(80);          //web server is on port 80 (default is 80)

 WiFiUDP ntpUDP;                //object to send and receive data
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);   //object for the NTP client (object_of_WiFiUDP, server_name, timezone_offset, update_time)


/////////////////////////DATA ARRAY//////////////////////

/*------------------open bin array------------------*/
static const uint8_t open_bin[512] =                          //bitmap image array for OPEN 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0b, 0xfb, 0xc8, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0b, 0x7b, 0xe8, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0b, 0xef, 0xc8, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x0b, 0xfe, 0xe8, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x5f, 0xbf, 0xef, 0xd0, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x50, 0xa0, 0x28, 0x50, 0x00, 0x00, 
    0x00, 0x00, 0x5f, 0xbf, 0xef, 0xd0, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*--------------close bin array---------------*/
static const uint8_t close_bin[512] =                         //bitmap array for CLOSE
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 
    0x00, 0x00, 0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 
    0x00, 0x00, 0x40, 0x00, 0x00, 0x08, 0x00, 0x00, 
    0x00, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 
    0x00, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x7f, 0xbf, 0xf7, 0xfa, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x40, 0xa0, 0x14, 0x0a, 0x00, 0x00, 
    0x00, 0x01, 0x7f, 0xbf, 0xf7, 0xfa, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*-------------bootup array------------*/
static const uint8_t bootup[512] =                            //bitmap image array for CONNECTING
{ 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0c, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 
    0x00, 0x00, 0x00, 0x7f, 0xc0, 0x11, 0xf1, 0x00, 
    0x00, 0x00, 0x00, 0x80, 0x20, 0x13, 0x19, 0x00, 
    0x00, 0x01, 0xff, 0xff, 0xff, 0xf6, 0xec, 0x80, 
    0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0xb0, 0x80, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x23, 0x58, 0x80, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x20, 0xe0, 0x80, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x21, 0xb0, 0x80, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x41, 0x00, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x06, 0x0c, 0x00, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0xf0, 0x00, 
    0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
    0x00, 0x04, 0xff, 0xff, 0xff, 0xe4, 0x00, 0x00, 
    0x00, 0x05, 0xf7, 0xfb, 0xfd, 0xf4, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0x14, 0x0a, 0x05, 0x14, 0x00, 0x00, 
    0x00, 0x05, 0xf7, 0xfb, 0xfd, 0xf4, 0x00, 0x00, 
    0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 
    0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };


/////////////////////PROGRAM FUNCTIONS//////////////////////
/*------------booting-------------*/
void booting(){                               //device startup text
  display.clearDisplay();                     // clear the display buffer
  display.setTextSize(2);                     //setting text size
  display.setTextColor(WHITE);                //setting text color
  display.setCursor(34,4);                    //for setting the corsor position
  display.println("SMART");                   //printing the text/ sending the text to buffer
  display.setCursor(22,24);
  display.println("DUSTBIN");
  display.setTextSize(1);
  display.println("\n\n      by Doman Sarkar");
  display.display();                          //for displaying the buffer data on display (default constructor)
  delay(3000);                                // for delay of 3 seconds (3000)
}

/*---------------connecting----------*/
void connecting(){                            //indication of connecting to wifi
  display.clearDisplay();
  display.drawBitmap((display.width()  - LOGO_WIDTH ) / 2, (display.height() - LOGO_HEIGHT) / 2, bootup, LOGO_WIDTH, LOGO_HEIGHT, 1); //for displaying the bitmap (pos_x, pos_y, bitmap_array_name, logo_width, logo_height, 1)
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,56);
  display.print("CONNECTING  ");
  display.display();
  for(int i=0;i<10;i++)                       //loop for 10 cycles to print "="
  {
    display.print("=");
    display.display();
    delay(10);
  }
}

/*-------------open lid-----------*/
void openLid(){                               //when open is pressed or hand is detected
  lid.write(90);                              //to open servo to 90 degrees
  display.clearDisplay();                     
  display.drawBitmap(64, 0, open_bin, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,32);
  display.print("OPEN");
  display.display();
  delay(2000);                                //waiting for the user to put the garbage in the container
}

/*-----------close lid-------------*/
void closeLid(){                              //when lid is closed
  display.clearDisplay();
  display.drawBitmap(64, 0, close_bin, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,32);
  display.print("CLOSE");
  display.display();
  delay(2000);
  lid.write(180);                             //sevo to rotate to close position (default position)
  delay(2000);                                //wait for some time as the sevo closes otherwise false coll data will be registered
  notice = "\0";                              //notice variable contains null
  coll = ((canlen - checkLevel()) / canlen) * 100;  //for the garbage collected in percentage --checkLevel() to check the garbage level
  if(coll > 100 || coll < 0)                  //if coll is greater than 100 or less than 0 then the detected value is wrong and is resetted
  {
    coll = 0;
  }
  timeClient.update();                        //to update the current date
  day=timeClient.getDay();                    //to get the day of week (0-6)--(sun-sat)
  EEPROM.write(day,coll);                     //writing coll value to the EEPROM permanently
  EEPROM.commit();                            //commting the changes to EEPROM
  display.clearDisplay();
  display.display();
  int data[7]={EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(6)};   //for reading the data of all the garbage collected by the days from EEPROM
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("sun\nmon\ntue\nwed\nthu\nfri\nsat"); //for printing the chart on the display
  display.fillRect(20, 3, 1+data[0], 4, WHITE);       //fillRect for the bars (starting_x, starting_y, rect_width, rect_height, rect_color)
  display.fillRect(20, 11, 1+data[1], 4, WHITE);
  display.fillRect(20, 19, 1+data[2], 4, WHITE);
  display.fillRect(20, 27, 1+data[3], 4, WHITE);
  display.fillRect(20, 35, 1+data[4], 4, WHITE);
  display.fillRect(20, 43, 1+data[5], 4, WHITE);
  display.fillRect(20, 51, 1+data[6], 4, WHITE);
  display.display();
}

/*--------------message-------------*/
void message(){                               //for displaying the message when new day starts
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("MESSAGE :\n\nGOODMORNING\n\nDo not forget to take out the trash!\n\nHave a nice day.");
  display.display();
  delay(3000);
}

/*------------check lid---------------*/
bool checkLid(){                              //for hand detection returns a boolean value true if hand is detected
  digitalWrite(trigPin, LOW);                 //staring signal of the pulse
  delayMicroseconds(2);                       //delay in microseconds (these are very precise delays)
  digitalWrite(trigPin, HIGH);                //signal for the transmitting of ultrasonic sound/signal
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);                 //so the signal transmitted _ _ - - - - - - - - - - _ 
  duration = pulseIn(echoPin, HIGH);          //duration after the above pulse is received
  distance = duration * 0.034 / 2;            //s=d/t and we need distance d=s*t and since we have the duration from above (time) 
                                              //and velocity of sound is 0.034 cm/s and since the time taken is doubled as the sound travels 
                                              //from the sensor to the object and to the sensor back so we devide it by 2
  if(distance < 10)                           //if the sensor detects a distance less than 10 cm then we detected  a hand
  {
    return true;
  }
  return false;
}

/*-----------check level------------*/
int checkLevel(){                             //for checking  the distance of which the garbage at returns an int value for distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.034 / 2;
  return distance1;
}

/*--------calibrate------------*/
void calibrate(){                             //resets the garbage collected data and restarts the device
  EEPROM.write(0,0);                          //EEPROM data for sunday (adress, data)
  EEPROM.write(1,0);                          //EEPROM data for monday
  EEPROM.write(2,0);
  EEPROM.write(3,0);
  EEPROM.write(4,0);
  EEPROM.write(5,0);
  EEPROM.write(6,0);
  EEPROM.write(7,0);                          //EEPROM data for garbage values
  EEPROM.write(8,7);                          //EEPROM data set to 7 and data of 7 is not used anywhere and 7 cannot be the day value
  EEPROM.commit();                            //commiting the changes
  ESP.restart();                              //restarting the ESP8266
}


///////////////////////////MAIN FUNCTIONS (SETUP AND LOOP)//////////////////////////
/*----------------------SETUP---------------------*/
void setup()                                  //SETUP of the program
{
  /*-----------EEPROM------------*/
  EEPROM.begin(9);                            //initializing the amount of EEPROM segments needed (7 for days + 1 for garbage value + 1 for previous day = 9)

  /*-----------serial monitor-----------*/
  Serial.begin(115200);                       //serial monitor begin (baud rate)

  /*-------------ultrasonic sensors------------*/
  pinMode(trigPin, OUTPUT);                   //define pinmode of trigger pin for hand detection and garbage level
  pinMode(echoPin, INPUT);                    //define pinmode of echo pin for hand detection
  pinMode(echoPin1, INPUT);                   //define pinmode of echo pin for garbage level

  /*------------ssd1306 oled display-----------*/
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {    //check if display is detected at the adress
      Serial.println(F("SSD1306 allocation failed")); 
      for(;;);                                // Don't proceed, loop forever
  }

  /*--------------boot screen----------------*/
  booting();                                  //for the boot screen

  /*----------------connecting to wifi-------------*/
  Serial.print("Connecting to Internet ");    
  WiFi.begin(ssid, password);                 //connecting to wifi
  connecting();                               //connecting screen on display
  while (WiFi.status() != WL_CONNECTED)       //loop till wifi not connected
  {
    delay(500);
    Serial.print(".");
  }
  display.clearDisplay();                     //clearing display buffer
  display.display();                          //displaying the buffer
  Serial.println("WiFi connected");

 /*---------------web server---------------------*/
  server.begin();                             //start the webserver
  Serial.println("Server started");
  Serial.print("IP Address of network: ");
  Serial.println(WiFi.localIP());             //printing the ip adress of device in serial monitor of ide
  Serial.print("Copy and paste the following URL: https://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  /*-------------date and time client-------------*/
  timeClient.begin();                         //for the date and time

  /*--------------initializing servo movement------------*/
  lid.attach(lidPin);                         //for the servo (servo _signal_pin)
  lid.write(180);                             //intializing the servo angle

  /*-------------date and time initialize----------------*/
  timeClient.update();                        //upadting the server data for date and time
  day = timeClient.getDay();                  //to get the current day of week accurate upto 60 seconds or 1 minute (60000 milisecond)
}



/*---------------------LOOP-----------------*/
void loop()                                   //LOOP of the program
  {
    WiFiClient client = server.available();   //make new client when client available client-->phone or anything connecting to the webserver we created

    /*----------------------this part will always run if there are no clients (means ideally)-------------------------*/
    if (!client)                              //if there is no client then ill get into this condition and do some processes and return
    {
      timeClient.update();                    //updates the timeclient servers
      day=timeClient.getDay();                //get and store the current day of week
      if(day != EEPROM.read(8))               //if previous day of week is does not match to the updated week then ill go inside the condition
      {
        message();                            //calls message to print the message when new day starts
        notice = "Good morning! Don't forget to take out the trash. Thank you";   //message value
        coll = 0;                             //reset coll
        timeClient.update();                  
        day=timeClient.getDay();
        EEPROM.write(8, day);                 //store the new data to the prev state for the day of week
        EEPROM.commit();                      //commit changes to EEPROM
      }
      //Serial.println("lid: " + (String)timeClient.getDay());
      //Serial.println("level: " + (String)checkLevel());
      if(checkLid())                          //check lid of hand is detected if it returns true then ill get inside the condition
      {
        openLid();                            //open the lid and perform the tasks associated with it
        timeClient.update();                  //update date and time
        closeLid();                           //close lid and perform associated tasks with it
      }
      return;                                 //return statement if no client is found
    }
    

  /*-----------------------This part will only activate if it finds any clients----------------------*/  
  Serial.println("Waiting for new client");   
  while(!client.available())                  //loops till it connecs to the current client
  {
    delay(1);
  }
 
  String request = client.readStringUntil('\r'); //so that the request does not repeat itself like "HTTP/1.1 200 OK HTTP/1.1 200 OK"
  Serial.println(request);
  client.flush();                             //discard any bytes that have been written to the client but not yet read
 
 
  if (request.indexOf("/OPEN") != -1)         //if the user clicks on open then ill go inside the condition
  {
    openLid();                              
    timeClient.update();
    closeLid();
  }
  if (request.indexOf("/CALIBRATE") != -1)    //if user clicks on calibrate ill call calibrate function
  {
    calibrate();
  }
 
/*------------------Creating html page---------------------*/

  int data[7]={EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(6)}; //array of all the data day wise 
  client.println("HTTP/1.1 200 OK");          //request has been sucessful transfering html data
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");                   //the actual html webpage html tag
  client.println("<head>");                   //head
  client.println("<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>"); //script
  client.println("<script type=\"text/javascript\">");
  client.println("google.charts.load('current', {'packages':['bar']});");
  client.println("google.charts.setOnLoadCallback(drawChart);");
  client.println("function drawChart() {");
  client.println("var data = google.visualization.arrayToDataTable([");
  client.println("['day', 'collected ammount (%)'],");
  client.println("['SUN', "+(String)data[0]+"],");
  client.println("['MON', "+(String)data[1]+"],");
  client.println("['TUE', "+(String)data[2]+"],");
  client.println("['WED', "+(String)data[3]+"],");
  client.println("['THU', "+(String)data[4]+"],");
  client.println("['FRI', "+(String)data[5]+"],");
  client.println("['SAT', "+(String)data[6]+"]");
  client.println("]);");
  client.println("var options = {");
  client.println("chart: {");
  client.println("title: 'Collection ammount',");
  client.println("subtitle: 'from monday to sunday',");
  client.println("}};");
  client.println("var chart = new google.charts.Bar(document.getElementById('columnchart_material'));");
  client.println("chart.draw(data, google.charts.Bar.convertOptions(options));");
  client.println(" }");
  client.println("</script>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1><center>SMART DUSTBIN</center></h1>");
  client.println("<br><br><br>");
  if(notice != "\0")                          //if the notice is not null then it will display the notice
  {
    client.println("<center>Message!</center>");
    client.println("<i>"+notice+"</i>");
  }
  client.println("<center><a href=\"/OPEN\"\"><button><h2>OPEN</h2></button></a></center><br><br><br><br>");
  client.println("<center><a href=\"/CALIBRATE\"\"><button><h2>CALIBRATE</h2></button></a><br /></center><br><br><br>");
  client.println("<center><div id=\"columnchart_material\" style=\"width: 800px; height: 500px;\"></div></center>");
  client.println("<br><br><br><br>");
  client.println("<i><p style=\"text-align:right;\">smart dustbin project</p></i>");
  client.println("<i><p style=\"text-align:right;\">made by Doman Sarkar</p></i>");
  client.println("<br><br><br><br>");
  client.println("<i>NOTE :-</i><br>");
  client.println("<i>     1. Calibrate button is to calibrate and erase older data of the smart dustbin.</i><br>");
  client.println("<i>     2. open button is for opening the lid of the smart dustbin.</i>");
  client.println("</body>");
  client.println("</html>");                  //end of html webpage client disconnected 

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}
