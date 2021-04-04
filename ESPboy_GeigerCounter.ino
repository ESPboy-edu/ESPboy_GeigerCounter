/*
ESPboy Geiger Counter module
for www.ESPboy.com project by RomanS
v1.0
*/

#include "ESPboyInit.h"
#include "ESPboyLED.h"

#define DELAY_PARTICLES 100000
#define DELAY_BETWEEN_UPDATE 200
#define DELAY_BETWEEN_FILLING_DATA 5000
#define MAX_HEIGHT 60
#define MAX_DATA 43
#define IMPULSES_PER_HOUR_EQUAL_TO_ONE_MICRORENTGEN 90
#define TIME_TO_COUNT_AVERAGE_MICRORENTGEN 30000

#define COLOR_GREEN_LEVEL 10 //green - less than 10 per DELAY_BETWEEN_UPDATE
#define COLOR_YELLOW_LEVEL 10 //yellow - more than 10 per DELAY_BETWEEN_UPDATE
#define COLOR_RED_LEVEL 30 //red - more than 30 per DELAY_BETWEEN_UPDATE


ESPboyInit myESPboy;

volatile uint32_t counterParticles=0, counterParticlesFromStart=0, counterMicroRentgen=0;
volatile uint64_t delayParticles=0;

uint32_t data[MAX_DATA];

void ICACHE_RAM_ATTR countGeiger(){
  if (ESP.getCycleCount()-delayParticles > DELAY_PARTICLES){
    delayParticles = ESP.getCycleCount();
    counterParticles++;
    counterParticlesFromStart++;
    counterMicroRentgen++;
  }
  delay(0);
};


void setup(){  

//init ESPboy
  myESPboy.begin("Geiger counter");

//high voltage start
  pinMode(3, OUTPUT);
  analogWriteFreq(10000);
  analogWrite(3, 512);

//counter interrupt start
  pinMode(D8, INPUT);
  attachInterrupt (digitalPinToInterrupt(D8), countGeiger, FALLING);
  myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);

//clear graph buffer
  memset(data, 0, sizeof(data));
}


void loop(){
 static uint32_t countUpdate, countFillData, countMicroRentgen, previousParticlCount;
 static float scalingFactor;
 static uint32_t colour, microrentgen=0;

 delay(0);

//beep if particle detected
  if(previousParticlCount != counterParticlesFromStart){
    previousParticlCount = counterParticlesFromStart;
    myESPboy.playTone(50,50);
  };


 //redraw screen
  if(millis()-countUpdate > DELAY_BETWEEN_UPDATE){
    countUpdate = millis();
    
    if(data[MAX_DATA-1] < COLOR_GREEN_LEVEL) colour=TFT_GREEN;
    if(data[MAX_DATA-1] > COLOR_YELLOW_LEVEL) colour=TFT_YELLOW;
    if(data[MAX_DATA-1] > COLOR_RED_LEVEL) colour=TFT_RED;

    myESPboy.tft.setTextColor(colour, TFT_BLACK);
    
    myESPboy.tft.setTextSize(1);
    myESPboy.tft.setCursor(0,0);
    myESPboy.tft.print("mR/h average");

    myESPboy.tft.setTextSize(3);
    myESPboy.tft.setCursor(0,13);
    String toPrint;
    if (microrentgen) toPrint = microrentgen;
    else toPrint = "--";
    myESPboy.tft.print(toPrint);
    myESPboy.tft.print("  ");

    myESPboy.tft.drawLine (0, 10, 128, 10, TFT_BLACK);
    myESPboy.tft.drawLine (0, 10, 128*(millis()-countMicroRentgen)/TIME_TO_COUNT_AVERAGE_MICRORENTGEN, 10, TFT_BLUE);

    myESPboy.tft.drawLine (0, 127, 128, 127, TFT_BLACK);
    myESPboy.tft.drawLine (0, 127, 128*(millis()-countFillData)/DELAY_BETWEEN_FILLING_DATA, 127, TFT_BLUE);

    myESPboy.tft.setTextSize(1);
    myESPboy.tft.setCursor(0,38);
    myESPboy.tft.print("impulses total");
    
    myESPboy.tft.setTextSize(2);
    myESPboy.tft.setCursor(0,48);
    myESPboy.tft.print(counterParticlesFromStart);
    myESPboy.tft.print("  ");
  }


 //write data to array and draw data
  if(millis()-countFillData > DELAY_BETWEEN_FILLING_DATA){
    countFillData = millis();
    
    memmove(&data[0], &data[1], sizeof(uint32_t)*(MAX_DATA-1));
    data[MAX_DATA-1] = counterParticles;
    counterParticles = 0;

    uint32_t maxDataVal=0;
    for (uint8_t i=0; i<MAX_DATA; i++){
      if(data[i] > maxDataVal) maxDataVal = data[i];}
    if (maxDataVal > MAX_HEIGHT) scalingFactor = (float)MAX_HEIGHT / (float)maxDataVal;
    else scalingFactor = 1;

    myESPboy.tft.fillRect(0,128 - 6 - MAX_HEIGHT, 128, MAX_HEIGHT, TFT_BLACK);
    
    for (uint8_t i=0; i<MAX_DATA; i++){
      if(data[i] < COLOR_GREEN_LEVEL) colour=TFT_GREEN;
      if(data[i] > COLOR_YELLOW_LEVEL) colour=TFT_YELLOW;
      if(data[i] > COLOR_RED_LEVEL) colour=TFT_RED;
      myESPboy.tft.drawFastVLine(i*3, 128 - 6 - data[i]*scalingFactor+1, data[i]*scalingFactor+1, colour);
      myESPboy.tft.drawFastVLine(i*3+1, 128 - 6 - data[i]*scalingFactor+1, data[i]*scalingFactor+1, colour);
    }
  }


//count average micro rentgen
  if(millis()-countMicroRentgen > TIME_TO_COUNT_AVERAGE_MICRORENTGEN){
    countMicroRentgen = millis();
    microrentgen = ((3600000/TIME_TO_COUNT_AVERAGE_MICRORENTGEN) * counterMicroRentgen) / IMPULSES_PER_HOUR_EQUAL_TO_ONE_MICRORENTGEN; //1 hour = 3 600 000 miliseconds
    counterMicroRentgen=0;
  }
}
