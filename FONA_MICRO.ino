
/*
  SintWind
*/

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
//#define fonaSerial Serial1
#define ARDUINO_RX 4//should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 5//connect to RX of the module
SoftwareSerial myMP3(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*basic commands*/
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F//stop interruptting with a song, just stop the interlude

/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
  #define DEV_TF 0X01
#define CMD_IC_MODE 0X35
  #define CMD_SLEEP   0X03
  #define CMD_WAKE_UP 0X02
  #define CMD_RESET   0X05

/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43

/*Special commands*/
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31

#define CMD_SET_PLAY_MODE 0X33
  #define ALL_CYCLE 0X00
  #define SINGLE_CYCLE 0X01

#define CMD_PLAY_COMBINE 0X45//can play combination up to 15 songs

void sendCommand(int8_t command, int16_t dat );

// Pins which are connected to Sim800L
#define FONA_RX            3
#define FONA_TX            2
#define FONA_RST             7

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


#define VBATPIN A1
float measuredvbat;

volatile byte clicks = 0;
long timer_0 = millis();
long timer;
uint8_t velocitaDelVento = 0;
int direzione;
int const potPin = A3;                                                  // Sensore di direzione
int potDir;
bool oldReed = HIGH;
int angle;
int8_t callstat;
uint16_t bat_percent;
uint8_t valoriMediaVelocita[10];
uint8_t valoriMediaDir[10];
uint8_t valoriMediaClicks[5];
uint8_t valoriMediaPotDir[5];
uint8_t velocitaMedia = 0;
uint8_t dirMedia =0;
uint8_t velocitaMediaVoce = 0;
uint8_t direzioneMediaVoce = 0;
uint8_t clicksMedia = 0;
uint8_t potDirMedia = 0;
int count = 0;
bool messaggioInviato = 0;
//const int pinLed = 8; //Pin LED



void setup() {//analogReadResolution(12); // Set analog input resolution to max, 12-bits
  powerUpOrDown();
  myMP3.begin(9600);
  delay(500);//Wait chip initialization is complete
    sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card 
  
  Serial.begin(115200);
  Serial.println(F("FONA incoming call example"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  Serial.println(F("FONA is OK"));
  pinMode(13, INPUT_PULLUP);                                                    // Sensore anemometro
  int reed_state = LOW;
  fona.setAudio(FONA_HEADSETAUDIO);

  //pinMode(pinLed, OUTPUT);
  
  timer_0 = 1000;

}

void loop() {

  //callstat = fona.getCallStatus();                                      // memorizzo lo stato in callstat il valore di return getCallStatus

  //Serial.println("Start");

  if (digitalRead(13) == HIGH && oldReed == LOW) {
    oldReed = HIGH;
    clicks++;
    // digitalWrite(pinLed, HIGH);                                                  // "delay(10);" in caso di utilizzo sensore reed
    //delay(100);
  }
  else if (digitalRead(13) == LOW)
  {
    oldReed = LOW;
    //digitalWrite(pinLed, LOW);
  }
  //Serial.println(oldReed);

  // Codice di rilevazione direzione con sensore a 8 resistenze del tipo Sparkfun

  potDir = analogRead(potPin);                              // Lettura valore sul pin analogico A3
  //Serial.print ("analogRead");
  //Serial.println(potDir);
  potDir = map(potDir, 0, 1023, 0, 500);
  //Serial.print ("1023analogRead");
  //Serial.println(potDir);

  

for ( int y = 0; y < 2 ; y++ ) {
    if (potDir < (32 + 2)) {
      angle = 113;
      break;
    }
    if (potDir < (41 + 2)) {
      angle = 68;
      break;
    }
    if (potDir < (45 + 2)) {
      angle = 90;
      break;
    }
    if (potDir < (62 + 2)) {
      angle = 158;
      break;
    }
    if (potDir < (90 + 2)) {
      angle = 135;
      break;
    }
    if (potDir < (119 + 2)) {
      angle = 203;
      break;
    }
    if (potDir < (140 + 2)) {
      angle = 180;
      break;
    }
    if (potDir < (198 + 2)) {
      angle = 23;
      break;
    }
    if (potDir < (225 + 2)) {
      angle = 45;
      break;
    }
    if (potDir < (293 + 2)) {
      angle = 248;
      break;
    }
    if (potDir < (308 + 2)) {
      angle = 225;
      break;
    }
    if (potDir < (343 + 2)) {
      angle = 338;
      break;
    }
    if (potDir < (384 + 2)) {
      angle = 0;
      break;
    }
    if (potDir < (404 + 2)) {
      angle = 293;
      break;
    }
    if (potDir < (433 + 2)) {
      angle = 315;
      break;
    }
    if (potDir < (462 + 2)) {
      angle = 270;
      break;
    }
  }

    //Serial.println("Direzione");
    //Serial.println(angle);                                   

  if (timer_0 > millis()) timer_0 = millis();
  //Serial.println((millis() - timer_0));
  if (millis() - timer_0 > 1000)                               ///////////////// OGNI SECONDO //////////////////
  {
    timer_0 = millis();
    measuredvbat = analogRead(VBATPIN);
    // measuredvbat *= 2; // we divided by 2, so multiply back
    measuredvbat *= 4.98; // Multiply by 5V, our reference voltage
    measuredvbat /= 1023; // convert to voltage
    Serial.print("VBat: "); Serial.println(measuredvbat);
    fona.getBattPercent(&bat_percent);
    Serial.print("Percentuale batteria: %");
    Serial.println(bat_percent);
    if (measuredvbat < 3.40 && messaggioInviato == 0) {
      // fona.sendSMS("3884385195", "Batteria quasi scarica");

      Serial.println("Messaggio inviato");
      messaggioInviato = 1;
      delay(5000);
    }
    Serial.print("Un secondo count=");
    Serial.println(count);
    if (count < 5)
    {
      valoriMediaClicks[count] = clicks;
      valoriMediaPotDir[count] = potDir;
      Serial.print("valoriMediaPotDir ");
      Serial.println(valoriMediaPotDir[count]);
    }

    if (count >= 5)
    {
      for (int t = 0; t < 4; t++)
      {
        valoriMediaClicks[t] = valoriMediaClicks[t + 1];
        valoriMediaPotDir[t] = valoriMediaPotDir[t + 1];
        Serial.print("valoriMediaPotDir ");
      Serial.println(valoriMediaPotDir[t]);
      }
      valoriMediaClicks[4] = clicks;
      valoriMediaPotDir[4] = potDir;
    }

    for (int t = 0; t < 5; t++)
    {
      clicksMedia += valoriMediaClicks[t];
      potDirMedia += valoriMediaPotDir[t];
    }
    /*
      Serial.print("count");
      Serial.println(count+1);
      Serial.println(valoriMediaClicks[0]);
      Serial.println(valoriMediaClicks[1]);
      Serial.println(valoriMediaClicks[2]);
      Serial.println(valoriMediaClicks[3]);
      Serial.println(valoriMediaClicks[4]);*/
    if (count >= 5)
    {
      clicksMedia /= 5;
      potDirMedia /= 5;
    }
    else
      clicksMedia /= count + 1;
      potDirMedia /= count + 1;
    //Serial.println("clicksMedia di cinque s ");
    //Serial.println(clicksMedia);
    clicksMedia *= 2.4;
    velocitaDelVento = clicksMedia;

    clicks = 0;                                         // Azzera contatore clicks

    if (count < 10)
    {
      valoriMediaVelocita[count] = velocitaDelVento;
      valoriMediaDir[count] = potDirMedia;
    }

    if (count >= 10)
    {
      for (int t = 0; t < count - 1; t++)
      {
        valoriMediaVelocita[t] = valoriMediaVelocita[t + 1];
        valoriMediaDir[t] = valoriMediaDir[t + 1];
      }
      valoriMediaVelocita[count - 1] = velocitaDelVento;
      valoriMediaDir[count - 1] = potDirMedia;
    }

    for (int t = 0; t < 10; t++)
    {
      velocitaMedia += valoriMediaVelocita[t];
      dirMedia += valoriMediaDir[t];
    }

    /*  Serial.println("valoriMediaVelocita");
      Serial.println(valoriMediaVelocita[0]);
      Serial.println(valoriMediaVelocita[1]);
      Serial.println(valoriMediaVelocita[2]);
      Serial.println(valoriMediaVelocita[3]);
      Serial.println(valoriMediaVelocita[4]);
      Serial.println(valoriMediaVelocita[5]);
      Serial.println(valoriMediaVelocita[6]);
      Serial.println(valoriMediaVelocita[7]);
      Serial.println(valoriMediaVelocita[8]);
      Serial.println(valoriMediaVelocita[9]);*/
    if (count >= 10)
    {
      velocitaMedia /= 10;
      dirMedia /= 10;
    }
    else
      velocitaMedia /= count + 1;
      dirMedia /= count + 1;
    velocitaMediaVoce = velocitaMedia;
    direzioneMediaVoce = dirMedia;
    //Serial.print("velocitaMedia ");
    //Serial.println(velocitaMedia);
    clicksMedia = 0;
    potDirMedia = 0;
    velocitaMedia = 0;
    dirMedia = 0;
    
    count++;
    if (count >= 10) count = 10;


    

    //-----------------------

    callstat = fona.getCallStatus();                                      // memorizzo lo stato in callstat il valore di return getCallStatus
    Serial.println(callstat);
    //delay(1000);
    //Serial.print ("1023analogRead");
    //Serial.println(potDir);
    //Serial.print ("angle");
    //Serial.println(angle);
  }

  

  if (callstat == 3)                                                    // e se corrisponde a "ringing", cioè che sta squillando
  {

    delay(2500);
    fona.pickUp();                                                      // risponde alla chiamata
    delay(1000);
    mp3_6bytes_cartella(CMD_PLAY_FILE_NAME, 0x01, convert_decimal_to_hex(117));                                    // play la voce di benvenuto
    delay(11500);

    for ( int y = 0; y < 2 ; y++ ) {
    if (direzioneMediaVoce < (32 + 2)) {
      angle = 113;
      break;
    }
    if (direzioneMediaVoce < (41 + 2)) {
      angle = 68;
      break;
    }
    if (direzioneMediaVoce < (45 + 2)) {
      angle = 90;
      break;
    }
    if (direzioneMediaVoce < (62 + 2)) {
      angle = 158;
      break;
    }
    if (direzioneMediaVoce < (90 + 2)) {
      angle = 135;
      break;
    }
    if (direzioneMediaVoce < (119 + 2)) {
      angle = 203;
      break;
    }
    if (direzioneMediaVoce < (140 + 2)) {
      angle = 180;
      break;
    }
    if (direzioneMediaVoce < (198 + 2)) {
      angle = 23;
      break;
    }
    if (direzioneMediaVoce < (225 + 2)) {
      angle = 45;
      break;
    }
    if (direzioneMediaVoce < (293 + 2)) {
      angle = 248;
      break;
    }
    if (direzioneMediaVoce < (308 + 2)) {
      angle = 225;
      break;
    }
    if (direzioneMediaVoce < (343 + 2)) {
      angle = 338;
      break;
    }
    if (direzioneMediaVoce < (384 + 2)) {
      angle = 0;
      break;
    }
    if (direzioneMediaVoce < (404 + 2)) {
      angle = 293;
      break;
    }
    if (direzioneMediaVoce < (433 + 2)) {
      angle = 315;
      break;
    }
    if (direzioneMediaVoce < (462 + 2)) {
      angle = 270;
      break;
    }
  }

    // esegue per tre volte le rilevazioni del vento e le play i rispettivi dati prima di riattaccare

    for (int i = 0; i < 3; i++)
    {
      if (angle > 349 || angle <= 11 ) direzione = 101;                   // Nord
      if (angle > 11 && angle <= 34 ) direzione = 109;                    // Nord tendente a Nord/Est
      if (angle > 34 && angle <= 56 ) direzione = 102;                    // Nord/Est
      if (angle > 56 && angle <= 79 ) direzione = 110;                    // Est tendente a Nord/Est
      if (angle > 79 && angle <= 101 ) direzione = 103;                   // Est
      if (angle > 101 && angle <= 124 ) direzione = 111;                  // Est tendente a Sud/Est
      if (angle > 124 && angle <= 146 ) direzione = 104;                  // Sud/Est
      if (angle > 146 && angle <= 169 ) direzione = 112;                  // Sud tendente a Sud/Est
      if (angle > 169 && angle <= 191 ) direzione = 105;                  // Sud
      if (angle > 191 && angle <= 214 ) direzione = 113;                  // Sud tendente a Sud/Ovest
      if (angle > 214 && angle <= 237 ) direzione = 106;                  // Sud/Ovest
      if (angle > 237 && angle <= 259 ) direzione = 114;                  // Ovest tendente a Sud/Ovest
      if (angle > 259 && angle <= 281 ) direzione = 107;                  // Ovest
      if (angle > 281 && angle <= 304 ) direzione = 115;                  // Ovest tendente a Nord/ovest
      if (angle > 304 && angle <= 326 ) direzione = 108;                  // Nord/Ovest
      if (angle > 326 && angle <= 349 ) direzione = 116;                  // Nord tendente a Nord/Ovest

      mp3_6bytes_cartella(CMD_PLAY_FILE_NAME, 0x01, convert_decimal_to_hex(direzione));       // play file di direzione     
      Serial.print ("angle");
      Serial.println(angle);
      Serial.println(velocitaMediaVoce);
      delay(4000);
      mp3_6bytes_cartella(CMD_PLAY_FILE_NAME, 0x01, convert_decimal_to_hex(velocitaMediaVoce));    // play file di velocità       
      delay(5000);
      clicks = 0;
      callstat = fona.getCallStatus();                                      // memorizzo lo stato in callstat il valore di return getCallStatus
      Serial.println(callstat);

      if (callstat == 0) return;
    }
    delay(3000);
    fona.hangUp();// chiude la telefonata
    
  }

}

void powerUpOrDown()
{
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}


void setVolume(int8_t vol)
{
  mp3_5bytes(CMD_SET_VOLUME, vol);
}
void playWithVolume(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_W_VOL, dat);
}

/*cycle play with an index*/
void cyclePlay(int16_t index)
{
  mp3_6bytes(CMD_SET_PLAY_MODE,index);
}

void setCyleMode(int8_t AllSingle)
{
  mp3_5bytes(CMD_SET_PLAY_MODE,AllSingle);
}


void playCombine(int8_t song[][2], int8_t number)
{
  if(number > 15) return;//number of songs combined can not be more than 15
  uint8_t nbytes;//the number of bytes of the command with starting byte and ending byte
  nbytes = 2*number + 4;
  int8_t Send_buf[nbytes];
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = nbytes - 2; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = CMD_PLAY_COMBINE; 
  for(uint8_t i=0; i < number; i++)//
  {
    Send_buf[i*2+3] = song[i][0];
  Send_buf[i*2+4] = song[i][1];
  }
  Send_buf[nbytes - 1] = 0xef;
  sendBytes(nbytes);
}


void sendCommand(int8_t command, int16_t dat = 0)
{
  delay(20);
  if((command == CMD_PLAY_W_VOL)||(command == CMD_SET_PLAY_MODE)||(command == CMD_PLAY_COMBINE))
    return;
  else if(command < 0x10) 
  {
  mp3Basic(command);
  }
  else if(command < 0x40)
  { 
  mp3_5bytes(command, dat);
  }
  else if(command < 0x50)
  { 
  mp3_6bytes(command, dat);
  }
  else return;
 
}

void mp3Basic(int8_t command)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x02; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = 0xef; //
  sendBytes(4);
}
void mp3_5bytes(int8_t command, uint8_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x03; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = dat; //
  Send_buf[4] = 0xef; //
  sendBytes(5);
}
void mp3_6bytes(int8_t command, int16_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x04; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = (int8_t)(dat >> 8);//datah
  Send_buf[4] = (int8_t)(dat); //datal
  Send_buf[5] = 0xef; //
  sendBytes(6);
}
void mp3_6bytes_cartella(int8_t command, int8_t cartella, int8_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x04; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = cartella;//cartella
  Send_buf[4] = (int8_t)(dat); //datal
  Send_buf[5] = 0xef; //
  sendBytes(6);
}
long convert_decimal_to_hex(int valore){
    String value = String(valore, HEX);
    value = ("0x" + value);
    int base = 16;
    int length = value.length() +1;
    char valueAsArray[length];
    value.toCharArray(valueAsArray, length);
    return strtol(valueAsArray, NULL, base);
}
void sendBytes(uint8_t nbytes)
{
  for(uint8_t i=0; i < nbytes; i++)//
  {
    myMP3.write(Send_buf[i]) ;
  }
}

