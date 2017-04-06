#include<SPI.h>
#include<nRF24L01.h>
#include<RF24.h>
const uint64_t pipe[1]= {0xF0F0F0F0E1LL};
RF24 radio(7,8);
int rec[1] = {2};
int pin = 9;      // LED Pin

typedef struct {
    int FR; // Fwd Rev joystick
    int LR; // Left right joystick
    bool S1; // Switch 1
    bool S2; // Switch 2
  } data;
data payload;

typedef struct{
  int temp;       //temperature
  int volts;      // voltage
  int distance;   // distance sensor
  bool front; 
  bool rear;
} info;
info telemetry;

void setup()
{
  Serial.begin(57600);
  radio.begin();
  delay(100);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openReadingPipe(1,pipe[0]);
  radio.startListening();
  radio.setRetries(15,15);
  rec[0]=0;
  pinMode(pin, OUTPUT);
  Serial.println("RX Ready");
  // telemetry
  telemetry.temp      = 27;
  telemetry.volts     = 7;
  telemetry.distance  = 34;
  telemetry.front     = false;
  telemetry.rear      = false;
  
}

void loop()
{
  if ( radio.available() ) {
    //radio.writeAckPayload( 1, rec, sizeof(int) );
    radio.writeAckPayload( 1, &telemetry, sizeof(telemetry) );
    radio.read( &payload,sizeof(payload) );
    
    rec[0]+=2;
    if(rec[0]>=100){
       rec[0]=1;
    }
  
    Serial.print("data we got is FR:");
    Serial.print(payload.FR);
    Serial.print(" LR:");
    Serial.print(payload.LR);
    Serial.print(" S1:");
    Serial.print(payload.S1);
    Serial.print(" S2:");
    Serial.println(payload.S2);
    digitalWrite(pin, payload.S1);
    
}else {
  //Serial.print( " no");
}
}
