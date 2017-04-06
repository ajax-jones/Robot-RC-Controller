// https://shantamraj.wordpress.com/2014/11/30/auto-ack-completely-fixed/
#include<SPI.h>
#include<nRF24L01.h>
#include<RF24.h>
#include <Wire.h>  // Include Wire if you're using I2C
#include "U8glib.h"

int rec[1] = {5};
bool stat = true;
RF24 radio(7,8);
const uint64_t pipe[1] = {0xF0F0F0F0E1LL};

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

bool setLED = false;
int pin   = 9;      // LED Pin
int pin2  = 2;     // Switch input
volatile int state = LOW;

// Decide where you are going to plug the joystick into the circuit board.
int ForeAft_Pin   = 0;       // Plug Joystick Fore/Aft into Analog pin 0
int LeftRight_Pin = 1;      // Plug Joystick Left/Right into Analog pin 1

// Create variables to read joystick values
float ForeAft_Input ;       // Variable to store data for Fore/Aft input from joystick
float LeftRight_Input ;     // Variable to store data for for Left/Right input from joystick

// Create variables to transmit servo value
int ForeAft_Output;         // Expected range 0 - 180 degrees
int LeftRight_Output;       // Expected range 0 - 180 degrees

// These variables allow for math conversions and later error checking as the program evolves.
int Fore_Limit = 1023;      // High ADC Range of Joystick ForeAft
int Aft_Limit  = 1;         // Low ADC Range of Joystick ForeAft

int Right_Limit = 1023;     // High ADC Range of Joystick LeftRight
int Left_Limit  = 1;        // Low ADC Range of Joystick LeftRight

typedef struct {
    int FR;   // Fwd Rev joystick
    int LR;   // Left right joystick
    bool S1;  // Switch 1
    bool S2;  // Switch 2
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

  // flip screen, if required
 //  u8g.setRot180();
    u8g.setColorIndex(255);     // white
  

  
  Serial.begin(57600);
    //set LED on pin9 mode
     pinMode(pin, OUTPUT);
    // set pin2 mode & set internal pull up resistor 
    pinMode(pin2, INPUT);  
    digitalWrite(pin2, HIGH); 
    //attach interrupt 'listener' to pin2 on Arduino
    attachInterrupt(0, ISR1, HIGH);

  payload.FR = 90;
  payload.LR = 90;
  payload.S1 = false;
  payload.S2 = false;

  radio.begin();
  delay(100);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.stopListening();
  radio.openWritingPipe(pipe[0]);
  radio.setRetries(15,15);
  int channel = radio.getChannel();
  int speed = radio.getDataRate();
  Serial.print("TX Booting up on channel ");
  Serial.print(channel);
  Serial.print(" at ");
  Serial.println(speed);
  radio.printDetails();
  
 }
void loop()
{

  // picture loop
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  

  
    ForeAft_Input     = analogRead(ForeAft_Pin) ;          // Read the Fore/Aft joystick value
    LeftRight_Input   = analogRead(LeftRight_Pin) ;        // Read the Left/Right joystick value
    
    ForeAft_Output    = convertForeAftToServo(ForeAft_Input) ;     // Convert the Fore/Aft joystick value to a Servo value (0-180)
    LeftRight_Output  = convertLeftRightToServo(LeftRight_Input) ; // Convert the Left/Right joystick value to a Servo value (0-180)
    payload.FR = ForeAft_Output;
    payload.LR = 90; //not connected LeftRight_Output;
  //delay (4000);
if(stat)
{
    //if(radio.write(msg,sizeof(msg)))
    if(radio.write(&payload,sizeof(payload)))
    {
    Serial.print("We going to TX  FR:");
    Serial.print(payload.FR);
    Serial.print(" LR:");
    Serial.print(payload.LR);
    Serial.print(" S1:");
    Serial.print(payload.S1);
    Serial.print(" S2:");
    Serial.print(payload.S2);
      if(radio.isAckPayloadAvailable())
      {
       // radio.read(rec,sizeof(int));
        radio.read(&telemetry,sizeof(telemetry));
        Serial.print(" and ack payload is : ");
        //Serial.println(rec[0]);
        Serial.print(telemetry.temp);
        Serial.print(" ");
        Serial.print(telemetry.volts);
        Serial.print(" ");
        Serial.println(telemetry.distance);

        if (rec[0]> 50 ){
          digitalWrite(pin, LOW);
        } else {
          digitalWrite(pin, HIGH);
        }
      }
      else
      {
        //stat = false; //doing this completely shuts down the transmitter if an ack payload is not received !!
        Serial.println("No acks being recieved, so we lost link so stop here....");
      }
    }
 }

   if(state == HIGH){
      Serial.println("--EVENT TRIGGERED--");
      Serial.println(state);
      Serial.println(digitalRead(pin));
      setLED = !setLED;
      //digitalWrite(pin, setLED);
      payload.S1 = setLED;
      delay(15000);
      state = LOW;
  } 
}

void ISR1(){  
 //tone(8, 350, 250);
 state = HIGH;
}


// Function to convert and scale the Fore/Aft data
float convertForeAftToServo(float y) {
    int result;
    result = ((y - Aft_Limit) / (Fore_Limit - Aft_Limit) * 180);
    return result;
}

// Function to convert and scale the Left / Right data
// Can be replaced with Map function
float convertLeftRightToServo(float x) {
    int result;
    result = ((x - Left_Limit) / (Right_Limit - Left_Limit) * 180);
    return result;
}

void draw(void) {
  u8g.setFont(u8g_font_8x13B);
  u8g.drawStr( 0, 10, "TX:ok RX:ok");
  u8g.drawStr( 0, 21, "Temp:32  Dist:18cm");
  u8g.drawStr( 0, 32, "TXv: 6.8 RXv:7.4v");
}

