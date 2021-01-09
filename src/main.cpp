#include <Arduino.h>
#include <SPI.h>

// debugging
#define ENABLE_DEBUG
#define BAUD 9600
#ifdef ENABLE_DEBUG
  #define DEBUG(a) (Serial.println(a))
#else
  #define DEBUG(a)
#endif

// caliper data
float caliperLeft = 0;
float caliperRight = 0;

// define response format
#define CALIPER_STR_SIZE 7 // eg -100.00
#define RES_SIZE CALIPER_STR_SIZE*2 + 3
char caliperStr[CALIPER_STR_SIZE + 1];
char res[RES_SIZE + 1];
#define RES_START     "^"
#define RES_END       "$"
#define RES_SEPARATOR "|"


void floatToCaliperStr(const float *f) {
  dtostrf(*f, CALIPER_STR_SIZE * -1, 2, caliperStr);
  unsigned int i=0;
  bool trimmed = false;
  while(!trimmed && i < CALIPER_STR_SIZE) {
    if(caliperStr[i] == ' ') {
      caliperStr[i] = '\0';
      trimmed = true;
    }
    i++;
  }
}

inline void formatRes() {
  strcpy(res, RES_START);
  floatToCaliperStr(&caliperLeft);
  strcat(res, caliperStr);
  strcat(res, RES_SEPARATOR);
  floatToCaliperStr(&caliperRight);
  strcat(res, caliperStr);
  strcat(res, RES_END);
}

inline void sendRes() {
  SPI.transfer(0x00);
  for (unsigned int i=0; i<strlen(res); i++){
    // send the response value via the data register
    SPI.transfer(res[i]);
  }
}

void setup(){
  #ifdef ENABLE_DEBUG
    Serial.begin(BAUD);
  #endif
  DEBUG("Debug on");

  // switch to slave
  pinMode(SS, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);

  SPCR = _BV(SPE) | _BV(SPR0);
}

void loop(){
  formatRes();
  DEBUG("Send:");
  DEBUG(res);
  sendRes();
  caliperLeft += 0.02f;
  caliperRight -= 0.02f;
  delay(1000);
}