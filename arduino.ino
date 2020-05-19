#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif           
#include "EMGFilters.h"         //sensor emg

#define TIMING_DEBUG 1

// pin
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST -1
#define czujnik1 A0       // pin in
#define czujnik2 A1
#define czujnik3 A2  

//SCREEN
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define BLACK       0x0000      ///<   0,   0,   0
#define NAVY        0x000F      ///<   0,   0, 128
#define DARKGREEN   0x03E0      ///<   0, 128,   0
#define DARKCYAN    0x03EF      ///<   0, 128, 128
#define MAROON      0x7800      ///< 128,   0,   0
#define PURPLE      0x780F      ///< 128,   0, 128
#define OLIVE       0x7BE0      ///< 128, 128,   0
#define LIGHTGREY   0xC618      ///< 192, 192, 192
#define DARKGREY    0x7BEF      ///< 128, 128, 128
#define BLUE        0x001F      ///<   0,   0, 255
#define GREEN       0x07E0      ///<   0, 255,   0
#define CYAN        0x07FF      ///<   0, 255, 255
#define RED         0xF800      ///< 255,   0,   0
#define MAGENTA     0xF81F      ///< 255,   0, 255
#define YELLOW      0xFFE0      ///< 255, 255,   0
#define WHITE       0xFFFF      ///< 255, 255, 255
#define ORANGE      0xFD20      ///< 255, 165,   0
#define GREENYELLOW 0xAFE5      ///< 173, 255,  47
#define PINK        0xFC18 

  //SENSOR

  EMGFilters myFilter;                  
int sampleRate = SAMPLE_FREQ_1000HZ;  //filtry dyskretne muszą działać ze stałą częstotliwością próbkowania (te poniżej pracują z wbranym czujnikiem)
int humFreq = NOTCH_FREQ_50HZ;        // w Polsce częstotliwość napięcia to 50Hz
static int Throhold = 0; //napięcie 0 - w momencie spoczynku

  unsigned long timeStamp;
  unsigned long timeBudget;

//bar charts

boolean graph_1 = true;
boolean graph_2 = true;
boolean graph_3 = false;
boolean graph_4 = false;


void setup() {
  
 myFilter.init(sampleRate, humFreq, true, true, true);
  Serial.begin(115200);
 timeBudget = 1e6 / sampleRate;              // SENSOR - mikro time
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setRotation(1);
}

void loop(void) {

   timeStamp = micros();
   int biceps = analogRead(czujnik1);   
   int triceps = analogRead(czujnik2);
   int naramienny = analogRead(czujnik3);
   
   int bicepsF = myFilter.update(biceps);     //SENSOR filter 
   int tricepsF=myFilter.update(triceps);
   int naramiennyF = myFilter.update(naramienny);  
   
   
   //int bicepsF2 = sq(bicepsF);
    // any value under throhold will be set to zero
   // bicepsF2 = (bicepsF2 > Throhold) ? bicepsF2 : 0; 

           timeStamp = micros() - timeStamp;


  DrawBarChartV(tft, 30,  210, 30, 180, 160, 600 , 30, bicepsF , 4 , 0, BLUE, CYAN, BLUE, WHITE, BLACK, "biceps", graph_1); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! przedprzedostatnia i ostatnia
  DrawBarChartV(tft, 100,  210, 30, 180, 0, 500 , 30, tricepsF , 4 , 0, MAGENTA, PINK, PINK, WHITE, BLACK, "triceps", graph_2);
  //DrawBarChartV(tft, 170,  210, 30, 180, 0, 500 , 30, naramiennyF , 4 , 0, DARKGREY, LIGHTGREY, DARKGREY, WHITE, BLACK, "naramienny", graph_3);
 // DrawBarChartV(tft, 250,  210, 30, 180, 0, 500 , 30, biceps, 4 , 0, GREEN, OLIVE, DARKGREY, WHITE, BLACK, "piersiowy", graph_4);
  

        if (TIMING_DEBUG) {
        // Serial.print("Read Data: "); Serial.println(Value);
        // Serial.print("Filtered Data: ");Serial.println(DataAfterFilter);
      
       // Serial.print("Napiecie biceps: ");
        Serial.print(bicepsF);
        Serial.print("\t");
        //Serial.print("Napiecie triceps: ");
       Serial.print(tricepsF);
        //Serial.print("\t");
       // Serial.print("Napiecie naramienny: ");
        //Serial.print(naramiennyF);
        Serial.println(""); 
        // Serial.print("Filters cost time: "); Serial.println(timeStamp);
        // the filter cost average around 520 us
    }
    delayMicroseconds(500);
                       
         

}


/*
  This method will draw a vertical bar graph for single input
  it has a rather large arguement list and is as follows

  &d = display object name
  x = position of bar graph (lower left of bar)
  y = position of bar (lower left of bar
  w = szerokość wykresu
  h =  wysokość wykresu
  lo = ldolna granica skali
  hi = górna granica skali
  inc = scale division between loval and hival
  curval = date to graph (must be between loval and hival)
  dig = format control to set number of digits to display (not includeing the decimal)
  dec = format control to set number of decimals to display (not includeing the decimal)
  barcolor = color of bar graph
  voidcolor = color of bar graph background
  bordercolor = color of the border of the graph
  textcolor = color of the text
  backcolor = color of the bar graph's background
  label = bottom lable text for the graph
  redraw = flag to redraw display only on first pass (to reduce flickering)

*/

void DrawBarChartV(Adafruit_ILI9341 & d, double x , double y , double w, double h , double loval , double hival , double inc , double curval ,  int dig , int dec, unsigned int barcolor, unsigned int voidcolor, unsigned int bordercolor, unsigned int textcolor, unsigned int backcolor, String label, boolean & redraw)
{

  double stepval, range;
  double my, level;
  double i, data;
  // draw the border, scale, and label once
  // avoid doing this on every update to minimize flicker
  if (redraw == true) {
    redraw = false;

//podpis wykresu
    d.drawRect(x - 1, y - h - 1, w + 2, h + 2, bordercolor);
    d.setTextColor(textcolor, backcolor);
    d.setTextSize(1);
    d.setCursor(x , y + 10);
    d.println(label);
    // step val basically scales the hival and low val to the height
    // deducting a small value to eliminate round off errors
    // this val may need to be adjusted
    stepval = ( inc) * (double (h) / (double (hival - loval))) - .001;
    for (i = 0; i <= h; i += stepval) {
      my =  y - h + i;
      d.drawFastHLine(x + w + 1, my,  5, textcolor);
      
      // podziałka
      d.setTextSize(1);
      d.setTextColor(textcolor, backcolor);
      d.setCursor(x + w + 3, my - 3 );
      data = hival - ( i * (inc / stepval));
      d.println(Format(data, dig, dec));
    }
  }
  // compute level of bar graph that is scaled to the  height and the hi and low vals
  // this is needed to accompdate for +/- range
  level = (h * (((curval - loval) / (hival - loval))));
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
  d.fillRect(x, y - h, w, h - level,  voidcolor);
  d.fillRect(x, y - level, w,  level, barcolor);
  
  // wartość
  d.setTextColor(textcolor, backcolor);
  d.setTextSize(2);
  d.setCursor(x , y - h - 23);
  d.println(Format(curval, dig, dec));
}





 String Format(double val, int dec, int dig ) {
  int addpad = 0;
  char sbuf[20];
  String condata = (dtostrf(val, dec, dig, sbuf));


  int slen = condata.length();
  for ( addpad = 1; addpad <= dec + dig - slen; addpad++) {
    condata = " " + condata;
  }
  return (condata);

} 
