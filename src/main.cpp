#include <Arduino.h>
#include <SPI.h>

//================================================= DEBUGGING

#define ENABLE_DEBUG
#define BAUD 9600
#ifdef ENABLE_DEBUG
  #define DEBUG(a) (Serial.println(a))
#else
  #define DEBUG(a)
#endif

//================================================= CALIPER

// store scanned caliper data
float caliperData = 0;
void scanCaliper(unsigned int axis);
// resets calipers to zero
void resetCalipers();

//================================================= SPI PROTOCOL

// holds registry data
volatile unsigned int reg;

// rescan axis position
void scanAxis(unsigned int axis);

// checks if axis position has finished scanning
volatile bool isAxisReady = false;

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
}

//================================================= SPI interrupt routine
ISR (SPI_STC_vect) {
  reg = SPDR;
  if(reg==0) {
    resetCalipers();
  } else if(reg>0 && reg<10) {
    DEBUG("Scan axis:");
    DEBUG(reg);
    scanAxis(reg);
  } else if(reg==10) {
    DEBUG("Check axis ready, return position status");
    if(isAxisReady) {
      SPDR=11;
    }
  } else if(reg==100) {
    DEBUG("Request axis value:");
    DEBUG(axisValue);
    incrementAxisValueIndex();
    SPDR = 100 + axisValue[axisValueIndex];
  }
}

//================================================= MAIN LOOP
void loop(){
  // runs if SPI not active
}

//================================================= IMPLEMENTATIONS

void scanCaliper(unsigned int axis) {
  // TODO: dummy data used; scan actual caliper value
  caliperData += 0.2f;
  delay(1000);
  // prevent negative values
  if(caliperData<0) {
    caliperData *= -1;
  }
}

void resetCalipers() {
  // TODO: dummy reset; need to interupt caliper power
  caliperData = 0;
}

void scanAxis(unsigned int axis) {
  isAxisReady = false;
  scanCaliper(axis);
  dtostrf(caliperData, AXIS_VALUE_SIZE, AXIS_VALUE_DECIMALS, axisValue);
  // pad unused values with 0
  unsigned int i=0;
  while(axisValue[i]==' ' && i<AXIS_VALUE_SIZE) {
    axisValue[i]='0';
    i++;
  }
  axisValueIndex = 0;
  isAxisReady = true;
}

void incrementAxisValueIndex() {
  if(axisValueIndex < AXIS_VALUE_SIZE-1){
    axisValueIndex++;
  } else {
    axisValueIndex=0;
  }
}