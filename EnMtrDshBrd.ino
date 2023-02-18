/*****************************************************************************
  Copyright (c) Future Technology Devices International 2014
  propriety of Future Technology devices International.

  Software License Agreement

  This code is provided as an example only and is not guaranteed by FTDI.
  FTDI accept no responsibility for any issues resulting from its use.
  The developer of the final application incorporating any parts of this
  sample project is responsible for ensuring its safe and correct operation
  and for any consequences resulting from its use.
*****************************************************************************/
/**
  @file                           EnMtrDshBrd.ino
  @brief                          Sketch to display energy consumption, can be used in home to monitor power consumption.
								  Tested platform version: Arduino 1.8.19 and later
  @version                        0.1.0
  @date                           12/Nov/2022

13 Nov 2022 - update graph every 8 minutes\use 
Global variable value is used in this program
Macro MAX defines the maximum watt / value

   Tasks needed
   RTC
   MODBUS
   axis name and marking
*/


/* This application demonstrates the usage of FT800 library on NHD_43RTP_SHIELD platform */

/* Arduino standard includes */
#include "SPI.h"
#include "Wire.h"

/* Platform specific includes */
#include "FT_NHD_43RTP_SHIELD.h"

#define TXT_LINE1_YPOS  20
#define TXT_LINE2_YPOS  (TXT_LINE1_YPOS + 40)
#define TXT_LINE3_YPOS  (TXT_LINE2_YPOS + 40)
#define TXT_LINE3_XPOS  40


#define BTN1_Xpos 420
#define BTN1_Ypos 20

#define BTN2_Xpos BTN1_Xpos
#define BTN2_Ypos (BTN1_Ypos + 60)

#define BTN3_Xpos BTN1_Xpos
#define BTN3_Ypos (BTN2_Ypos + 60)

#define BTN4_Xpos BTN1_Xpos
#define BTN4_Ypos (BTN3_Ypos + 60)

#define BTN_WDT 60
#define BTN_HGT 40

#define DISDST_XPOS 20
#define DISDST_YPOS (TXT_LINE1_YPOS + 30)

#define DISPC2_XPOS 280
#define DISPC2R1_YPOS (TXT_LINE1_YPOS + 30)
#define DISPC2R2_YPOS (DISPC2R1_YPOS + 40)
#define DISPC2R3_YPOS (DISPC2R2_YPOS + 40)
#define DISPC2R4_YPOS (DISPC2R3_YPOS + 40)
#define DISPC2R5_YPOS (DISPC2R4_YPOS + 40)

#define DISPCOL2_YPOS (TXT_LINE1_YPOS + 30)

#define CENTIMETER  1
#define INCHES  2
#define MAX 1200

/* Global object for FT800 Implementation */
FT800IMPL_SPI FTImpl(FT_CS_PIN, FT_PDN_PIN, FT_INT_PIN);

char DispStr[32];
char dUnit;
uint16_t value, width = 200, height = 100, xPos = 20, yPos = 130;

/* Api to bootup ft800, verify FT800 hardware and configure display/audio pins */
/* Returns 0 in case of success and 1 in case of failure */
int16_t BootupConfigure()
{
  FTImpl.Init(FT_DISPLAY_RESOLUTION);//configure the display to the WQVGA
  delay(20);//for safer side

  /* Set the Display & audio pins */
  FTImpl.SetDisplayEnablePin(FT_DISPENABLE_PIN);
  FTImpl.SetAudioEnablePin(FT_AUDIOENABLE_PIN);
  FTImpl.DisplayOn();
  FTImpl.AudioOn();
  return 0;
}

/*Draw the bar graph based on the position, options, new value, and range of the data.  The data represented is located in RAM_G and it's the size is 'w' bytes*/
void Bargraph(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range)
{
  uint16_t tempVal = 0;
  static int16_t index = 0, ramOffset = 0;

  /*update the supplied value to the correct location in RAM_G.  Data wrapping around is done by updating the RAM_G index location only if the value is greater than 0.*/
  tempVal = map(val, 0, range, h, 0); // h - ((h * val) / range); //

  FTImpl.Cmd_Memset(ramOffset + index, tempVal, 1);

  FTImpl.ColorRGB(25, 25, 125);
  FTImpl.LineWidth(10);
  FTImpl.Begin(FT_BITMAPS);

  FTImpl.BitmapSource(ramOffset);
  FTImpl.BitmapLayout(FT_BARGRAPH, 1, 1);
  FTImpl.BitmapSize(FT_NEAREST, FT_BORDER, FT_BORDER, 1, h);
  FTImpl.Vertex2f((x) * 16, (y) * 16);

  FTImpl.BitmapSource(ramOffset);
  FTImpl.BitmapLayout(FT_BARGRAPH, w - 1, 1);
  FTImpl.BitmapSize(FT_NEAREST, FT_BORDER, FT_BORDER, index, h);
  FTImpl.Vertex2f((x + 1) * 16, (y) * 16);

  FTImpl.Begin(FT_LINES);
  FTImpl.LineWidth(10);
  FTImpl.ColorRGB(255, 255, 255);
  FTImpl.Vertex2f((x) * 16, y * 16);
  FTImpl.Vertex2f((x) * 16, (y + h) * 16);
  FTImpl.Vertex2f((x) * 16, (y + h) * 16);
  FTImpl.Vertex2f((x + w) * 16, (y + h) * 16);

  sprintf(DispStr, "[%d, %d]", index, val);                               // display
  FTImpl.Cmd_Text(xPos + width / 2, yPos, 16, 0, DispStr);
  
//  int i = 0;
//  for(tempVal = x-1; tempVal < x+w; tempVal += (x/5)) {
//    sprintf(DispStr, "'%d]", (i)); 
//    FTImpl.Cmd_Text(tempVal, y+h+5, 16, 0, DispStr); 
//    i += 6;
//  }
  index +=2;
  if (index > w - 1) {
    index = 0;
  }
}

/* API to display Hello World string on the screen */
void DisplayDist(void)
{
  const char Display_ProjName[] = "Energy Dash board";
  int32_t tagval, tagoption;
  int16_t xvalue, yvalue;
  int distance;
  value < MAX ? value += 5 : value=0 ;
  sTagXY sTagxy;  // sTagXY is a structure

  /* Display list to display "Hello World" at the centre of display area */
  FTImpl.DLStart();   //start the display list. Note DLStart and DLEnd are helper apis, Cmd_DLStart() and Display() can also be utilized.

  FTImpl.ColorRGB(0xFF, 0xFF, 0xFF); //set the color of the string to while color
  FTImpl.Cmd_Text(FT_DISPLAYWIDTH / 2, TXT_LINE1_YPOS, 29, FT_OPT_CENTER, Display_ProjName); //Text display at center of screen using font handle 29

  FTImpl.GetTagXY(sTagxy);      // read the X, Y location and tag value

  xvalue = sTagxy.x;
  yvalue = sTagxy.y;            //or the xy coordinates can be directly read from register

  tagval = sTagxy.tag;          //read tag from register
  FTImpl.Cmd_FGColor(0x1f3f00);
  FTImpl.TagMask(1);

  tagoption = 0;                //No touch is default 3d effect and touch is flat effect
  if (tagval == 12)    {
    tagoption = FT_OPT_FLAT;
    // any user code to be executed when button pressed
  }
  FTImpl.Tag(12); //assign tag value 12 to the button
  FTImpl.Cmd_Button(BTN1_Xpos, BTN1_Ypos, BTN_WDT, BTN_HGT, 26, tagoption, "MENU");

  tagoption = 0;              //No touch is default 3d effect and touch is flat effect
  if (tagval == 13)    {
    tagoption = FT_OPT_FLAT;
    // any user code to be executed when button pressed
  }
  FTImpl.Tag(13); //assign tag value 13 to the button
  FTImpl.Cmd_Button(BTN2_Xpos, BTN2_Ypos, BTN_WDT, BTN_HGT, 26, tagoption, " UP ");

  tagoption = 0;
  if (tagval == 14) {
    tagoption = FT_OPT_FLAT;
    // any user code to be executed when button pressed
  }
  FTImpl.Tag(14); //assign tag value 14 to the button
  FTImpl.Cmd_Button(BTN3_Xpos, BTN3_Ypos, BTN_WDT, BTN_HGT, 26, tagoption, "DOWN");

  tagoption = 0;
  if (tagval == 15) {
    tagoption = FT_OPT_FLAT;
    // any user code to be executed when button pressed
  }
  FTImpl.Tag(15); //assign tag value 15 to the button
  FTImpl.Cmd_Button(BTN4_Xpos, BTN4_Ypos, BTN_WDT, BTN_HGT, 26, tagoption, "BACK");

  distance = 12345;                                               //
  sprintf(DispStr, "%d", distance);                               // display
  strcat(DispStr, " W");
  FTImpl.ColorRGB(0xFF, 0, 0);                                    //set the color of string to RED
  FTImpl.Cmd_Text(DISDST_XPOS, DISDST_YPOS, 31, 0, DispStr);      // display value

  FTImpl.ColorRGB(0x00, 0xFF, 0x00);                              //set the color of string to light Green
  FTImpl.Cmd_Text(DISPC2_XPOS, DISPC2R1_YPOS, 24, 0, "230.1 V");
  FTImpl.Cmd_Text(DISPC2_XPOS, DISPC2R2_YPOS, 24, 0, "2.34 A");
  FTImpl.Cmd_Text(DISPC2_XPOS, DISPC2R3_YPOS, 24, 0, "0.87 PF");
  FTImpl.Cmd_Text(DISPC2_XPOS, DISPC2R4_YPOS, 24, 0, "50.4 Hz");
  FTImpl.Cmd_Text(DISPC2_XPOS, DISPC2R5_YPOS, 24, 0, "460.2 VA");

  FTImpl.ColorRGB(0x00, 0x00, 0xFF);
  FTImpl.Cmd_Text(DISPC2_XPOS + 40, DISPC2R5_YPOS + 50, 16, 0, "12/11/2022 20:33");

  Bargraph(xPos, yPos, width, height, 0, value, MAX);           //draw the bargraph
  FTImpl.DLEnd();                                               //end the display list
  FTImpl.Finish();                                              //render the display list and wait for the completion of the DL
}
int DispOnCntr;
bool DispStat;
void setup()
{
  Serial.begin(9600);

  Serial.println("--Start Application--");
  BootupConfigure();                                          // Set the Display Enable pin
  FTImpl.Cmd_Memset(0, 255, width);                           //clear bargraph memory location in RAM_G.
  Serial.println("--Energy Dash board--");
  DispOnCntr = 100;
  DispStat = true;
}

void loop()
{
  DisplayDist();                    // display content
  if (FTImpl.IsPendown());          // if touch pressed, load 100 counter(20 sec)
  delay(200);
}
