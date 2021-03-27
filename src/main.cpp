#include <Arduino.h>
#include <SPI.h>
#include<caliper.h>

//================================================= DEBUGGING

//#define ENABLE_DEBUG
#define BAUD 9600
#ifdef ENABLE_DEBUG
  #define DEBUG(a) (Serial.println(a))
#else
  #define DEBUG(a)
#endif

//================================================= CALIPER

// init caliper
Caliper caliper(8,9,2);

// store scanned caliper data
float caliperData = 0;

// resets calipers to zero
// TODO: add caliper reset pin
void resetCalipers();

//================================================= SPI PROTOCOL

// holds registry data
volatile unsigned int reg;

// which axis is currently selected
volatile unsigned int axisSelected;

// used to signal an axis rescan from the ISR
volatile bool axisReady = true;

// serialize caliper data for SPI transmission
void parseAxisValue();

#define AXIS_VALUE_SIZE 6
#define AXIS_VALUE_DECIMALS 2
// current axis value (eg: 101.43)
// negative values not allowed!
char axisValue[AXIS_VALUE_SIZE];
// current index for the axis value
volatile unsigned int axisValueIndex = 0;
// increments the index for the axis value,
// making shure to start over if the end was reached
void incrementAxisValueIndex();

// used to signal a caliper reset from the ISR
volatile bool caliperReady = false;

//================================================= SETUP

void setup(){
  #ifdef ENABLE_DEBUG
    Serial.begin(BAUD);
  #endif
  DEBUG("Debug on");

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // turn on interrupts
  SPCR |= _BV(SPIE);

  // set clock speed
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

//================================================= SPI interrupt routine
ISR (SPI_STC_vect) {
  reg = SPDR;
  if(reg==0) {
    caliperReady = false;
  } else if(reg>0 && reg<10) {
    axisSelected = reg;
    axisReady = false;
  } else if(reg==10 && axisReady) {
    SPDR=10 + axisSelected;
  } else if(reg==100 && axisReady && caliperReady) {
    incrementAxisValueIndex();
    SPDR = 100 + axisValue[axisValueIndex];
  }
}

//================================================= MAIN LOOP
void loop(){
  // runs if ISR not active
  if (!caliperReady) {
    DEBUG("Start caliper reset");
    resetCalipers();
    DEBUG("End caliper reset");
    caliperReady = true;
  }
  if (!axisReady && caliperReady) {
    DEBUG("Start axis scan");
    if (caliper.read(&caliperData)) {
      // negative data not currently supported
      if(caliperData<0) caliperData = 0.0f;
      parseAxisValue();
      DEBUG("End axis scan:");
      DEBUG(axisValue);
      axisReady = true;
    } else {
      DEBUG("Axis scan error");
    }
  }
}

//================================================= IMPLEMENTATIONS

void resetCalipers() {
  // TODO: dummy reset; need to interupt caliper power
  caliperData = 0;
  delay(500);
}

void parseAxisValue() {
  dtostrf(caliperData, AXIS_VALUE_SIZE, AXIS_VALUE_DECIMALS, axisValue);
  // pad unused values with 0
  unsigned int i=0;
  while(axisValue[i]==' ' && i<AXIS_VALUE_SIZE) {
    axisValue[i]='0';
    i++;
  }
  axisValueIndex = 0;
}

void incrementAxisValueIndex() {
  if(axisValueIndex < AXIS_VALUE_SIZE-1){
    axisValueIndex++;
  } else {
    axisValueIndex=0;
  }
}