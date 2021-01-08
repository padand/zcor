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

// define response format
char res[10];
#define RES_OK       'y'
#define RES_NOK      'n'
#define RES_STR(STR) do{ strcpy(res, '^'); strcat(res, STR); strcat(res, '$') }while(false)
#define SEND_

// define request format
volatile char req;
#define REQ_RESET     'z'
#define REQ_POS_LEFT  'l'
#define REQ_POS_RIGHT 'r'

inline void sendRes() {
  for (unsigned int i=0; i<strlen(res); i++){
    DEBUG("Send:");
    DEBUG(res[i]);
    // send the response value via the data register
    SPDR = res[i];
  }
}

void setup(){
  #ifdef ENABLE_DEBUG
    Serial.begin(BAUD);
  #endif
  DEBUG("Debug on");

  // set MISO as OUTPUT (have to send data to master in)
  pinMode(MISO,OUTPUT);
  // set MOSI as INPUT (have to receive data from master out)
  pinMode(MOSI,INPUT);

  // turn on SPI in Slave Mode
  SPCR |= _BV(SPE);

  // turn on SPI interrupts
  SPI.attachInterrupt();
}

// SPI interrupt routine
ISR (SPI_STC_vect){
  // grab byte from SPI Data Register
  req = SPDR;
  DEBUG("Received:");
  DEBUG(req);

  switch(req) {
    case REQ_RESET:
      SPDR = RES_OK;
      break;
    default:
      SPDR = RES_NOK;
      break;
  }
}

void loop(){
  delay(500);
}