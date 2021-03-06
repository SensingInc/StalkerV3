// for adalogger m0

#define PREFIX_LENGTH 5 //Define the filename prefix length
#define SCREEN_DIM 128  //Define the pixel count of the OLED screen rows
#define SCLK_P 24 //Define the pin for the SPI clock
#define MOSI_P 23 //Define the pin for the SPI MOSI port
#define DC_P 12 //Define the DC pin for the SPI OLED screen
#define CS_P 10 //Define the CS pin for the SPI OLED screen
#define RST_P 11  //Define the reset pin for the SPI OLED screen
#define LOAD_DATA A4  //Define the data pin for the HX711 load cell ADC
#define LOAD_CLK A5 //Define the clock pin for the HX711 load cell ADC
#define BLACK 0x0000  //Define the screen color for black in 8 bytes
#define WHITE 0xFFFF  //Define the screen color for white in 8 bytes

#include <stdlib.h> //Include the standard library
#include <avr/dtostrf.h>  //Include the double to string function library
#include <U8x8lib.h>  //Include the OLED screen library
#include <SPI.h>  //Include the SPI library
#include <Wire.h> //Include the Wire (I2C) library
#include <SD.h> //Include the SD library
#include <SparkFun_MMA8452Q.h>  //Include the accelerometer library
#include <HX711_ADC.h>  //Include the load cell ADC library
#include <RTCZero.h>

U8X8_SH1106_128X64_NONAME_4W_HW_SPI tft(/* cs=*/ CS_P, /* dc=*/ DC_P, /* reset=*/ RST_P); //Create a screen instance
MMA8452Q accel;   //Create an accelerometer instance
HX711_ADC LoadCell(LOAD_DATA, LOAD_CLK);  //Create a load cell ADC instance
File file;
RTCZero rtc;

int hours = 0;
int minutes = 0;
int seconds = 0;
int day = 3;
int month = 3;
int year = 21;
int up=5; //Create a button instance for the up button
int down=6; //Create a button instance for the down button
int sel=9; //Create a button instance for the sel button
int file_num = 0; //Create a count integer to hold number of files
long start_time=0;
String PREFIX = "S002"; //Create a string to hold the prefix
String compileDate = (__DATE__);
String compileTime = (__TIME__);

void setup() {  //Enter the setup function
  Serial.begin(9600);
  rtc.begin();
  hours = compileTime.substring(0, 2).toInt();    // convert hours to an int
  minutes = compileTime.substring(3, 5).toInt();  // convert minutes to an int
  seconds = compileTime.substring(6, 8).toInt();  // convert seconds to an int
  rtc.setDate(day, month, year);
  rtc.setTime(hours, minutes, seconds);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(sel, INPUT_PULLUP);
  accel.init(); //Initialize the accelerometer instance
  accel.read(); //Read the accelerometer
  LoadCell.begin(); //Initialize the load cell ADC instance
  LoadCell.start(10); //Start communications with the load cell ADC
  LoadCell.setCalFactor(696.0); //Set the load cell ADC calibration gain factor
  attachInterrupt(LOAD_DATA, ISR, FALLING); //Attach an interrupt to the load cell ADC data pin
  tft.begin();  //Initialize the screen
  tft.setFont(u8x8_font_chroma48medium8_r); //Set the font for use on the screen
  if (!SD.begin(4)) { //Initialize the SD card instance, and if it doesn't start correctly
    tft.setCursor(0, 0);  //Set the cursor on the screen to the top left corner
    tft.println("SD ERR");  //Write "SD ERR" to the screen
    while (1); //And hang until reset
  }
  while (SD.exists(PREFIX + String(file_num) + ".csv")) file_num++;
  file = SD.open(PREFIX + String(file_num) + ".csv", FILE_WRITE);
  file.println("date,time,angle (°),force (arbs)"); //Add a header line to the file with the three output variable names
  delay(500);
  start_time = millis(); //Create a long called start_time and set it to the current number of msec at the start of sampling
}

void loop() { //Enter the loop function
  tft.setCursor(0,0);
  if(rtc.getHours() < 10){
    tft.print('0');
  }
  tft.print(rtc.getHours());
  tft.print(':');
  if(rtc.getMinutes() < 10){
    tft.print('0');
  }
  tft.print(rtc.getMinutes());
  tft.print(':');
  tft.println(rtc.getSeconds());
  delay(100);
  accel.read(); //Read the accelerometer
  double angle; //Create a double variable called angle
  long cur_time = 0;  //Create a long called cur_time to store current time
  float force = 0;  //Create a float called force to store the current force from the load cell
  char angle_str[7];  //Create a char array of 7 characters called angle_str to store the angle in string form
  char force_str[8];  //Create a char array of 8 characters called force_str to store the force in string form
  angle = 90.*accel.z / 1024.;  //Set angle equal to the normalized acceleration mapped to degrees
  angle = abs(angle); //Take the absolute value of angle
  dtostrf(angle, 6, 1, angle_str);  //Create a string in the char array angle_str to store the angle
  force = LoadCell.getData(); //Set force equal to the data coming back from the load cell
  dtostrf(force, 7, 1, force_str);  //Create a string in the char array force_str to store the angle
  cur_time = millis() - start_time; //Set cur_time as the current time from millis() minus the start time in msec
    file.print(rtc.getDay());
  file.print("/");
  file.print(rtc.getMonth());
  file.print("/");
  file.print(rtc.getYear()+2000);
  file.print(",");
  file.print(rtc.getHours());
  file.print(":");
  if(rtc.getMinutes() < 10)
    file.print('0');      // Trick to add leading zero for formatting
  file.print(rtc.getMinutes());
  file.print(":");
  if(rtc.getSeconds() < 10)
    file.print('0');      // Trick to add leading zero for formatting
  file.print(rtc.getSeconds());
  file.print(",");  //Print a comma into the data file
  file.print(angle_str);  //Print the angle_str variable into the data file
  file.print(",");  //Print a comma into the data file
  file.println(force_str);  //Print the force_str variable into the data file
  tft.print(PREFIX);
  tft.println(file_num);
  tft.print("Angle: ");
  tft.println(angle_str); //Print the angle on the screen
  tft.print("Force: ");
  tft.println(force_str); //Print a line with the force on the screen
  tft.print(cur_time / 1000); //Print the current time in sec
  tft.println(" s"); //Print a line saying " s" on the screen
  file.flush();
}

void ISR() {  //Enter the ISR function for assigning the interrupt service
  LoadCell.update();  //Update data from the load cell ADC
}
