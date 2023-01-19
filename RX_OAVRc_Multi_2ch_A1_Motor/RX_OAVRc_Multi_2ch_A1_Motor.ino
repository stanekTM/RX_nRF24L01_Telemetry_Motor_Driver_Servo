
//************************************************************************************************************************************************************************
//Communication nRF24L01P "Stanek". Fixed RF channel, fixed address.
//Support for OpenAVRc transmitter https://github.com/Ingwie/OpenAVRc_Dev
//and for the Multiprotocol TX modul https://github.com/stanekTM/DIY-Multiprotocol-TX-Module
//************************************************************************************************************************************************************************

#include <RF24.h>         //https://github.com/nRF24/RF24
//#include <printf.h>       //print the radio debug info
#include <DigitalIO.h>    //https://github.com/greiman/DigitalIO
#include "PWMFrequency.h" //used locally https://github.com/TheDIYGuy999/PWMFrequency


//setting RF channels address (5 bytes number or character)
const byte address[] = "jirka";

//RF communication channel settings (0-125, 2.4Ghz + 76 = 2.476Ghz)
#define RADIO_CHANNEL  76

//setting the reaction of the motor to be rotated after the lever has been moved. Settings (0-255)
#define ACCELERATE_MOTOR_A  0
#define ACCELERATE_MOTOR_B  0

//setting the maximum motor power. Suitable for TX transmitters without endpoint setting. Settings (0-255)
#define MAX_MOTOR_A  255
#define MAX_MOTOR_B  255

//brake setting, no brake 0, max brake 255. Settings (0-255)
#define BRAKE_MOTOR_A  0
#define BRAKE_MOTOR_B  0

//LED alarm battery voltage setting
#define BATTERY_VOLTAGE    4.2
#define MONITORED_VOLTAGE  3.35

//setting the dead zone of poor quality joysticks TX for the motor controller
#define DEAD_ZONE  15

//setting the control range value
#define MIN_CONTROL_VAL  1000
#define MID_CONTROL_VAL  1500
#define MAX_CONTROL_VAL  2000

//settings PWM (pin D5 or D6 are paired on timer0/8-bit, functions delay, millis, micros and delayMicroseconds)
//1024 = 61Hz, 256 = 244Hz, 64 = 976Hz(default), 8 = 7812Hz
//#define PWM_MOTOR_A  64

//settings PWM (pin D9 or D10 are paired on timer1/16-bit, Servo library)
//1024 = 30Hz, 256 = 122Hz, 64 = 488Hz(default), 8 = 3906Hz
#define PWM_MOTOR_A  256

//settings PWM (pin D3 or D11 are paired on timer2/8-bit, ServoTimer2 library)
//1024 = 30Hz, 256 = 122Hz, 128 = 244Hz, 64 = 488Hz(default), 32 = 976Hz, 8 = 3906Hz
#define PWM_MOTOR_B  256

//free pins
//pin                      0
//pin                      1
//pin                      4
//pin                      5
//pin                      6
//pin                      7
//pin                      8
//pin                      12 //MISO
//pin                      13 //SCK
//pin                      A5
//pin                      A6
 
//pwm pins for motor
#define PIN_PWM_1_MOTOR_A  9
#define PIN_PWM_2_MOTOR_A  10
#define PIN_PWM_3_MOTOR_B  3
#define PIN_PWM_4_MOTOR_B  11 //MOSI

//LED RX battery and RF on/off
#define PIN_LED            2

//input RX battery
#define PIN_RX_BATTERY     A7

//pins for nRF24L01
#define PIN_CE             A0
#define PIN_CSN            A1

//software SPI https://nrf24.github.io/RF24/md_docs_arduino.html
//----- SCK           16 - A2
//----- MOSI          17 - A3
//----- MISO          18 - A4

//setting of CE and CSN pins
RF24 radio(PIN_CE, PIN_CSN);

//************************************************************************************************************************************************************************
//this structure defines the received data in bytes (structure size max. 32 bytes) ***************************************************************************************
//************************************************************************************************************************************************************************
struct rc_packet_size
{
  unsigned int ch_motorA = MID_CONTROL_VAL;
  unsigned int ch_motorB = MID_CONTROL_VAL;
};
rc_packet_size rc_packet;

//************************************************************************************************************************************************************************
//this struct defines data, which are embedded inside the ACK payload ****************************************************************************************************
//************************************************************************************************************************************************************************
struct telemetry_packet_size
{
  byte rssi;       //0-255 for OpenAVRc and OpenTX Multiprotocol telemetry
  byte RX_batt_A1; //0-255 for OpenAVRc and OpenTX Multiprotocol telemetry
  byte RX_batt_A2; //0-255 for OpenAVRc and OpenTX Multiprotocol telemetry (not used yet)
};
telemetry_packet_size telemetry_packet;

//************************************************************************************************************************************************************************
//fail safe, settings 1000-2000 ​​(MIN_CONTROL_VAL = 1000, MID_CONTROL_VAL = 1500, MAX_CONTROL_VAL = 2000) *****************************************************************
//************************************************************************************************************************************************************************
void fail_safe()
{
  rc_packet.ch_motorA = MID_CONTROL_VAL;
  rc_packet.ch_motorB = MID_CONTROL_VAL;
}

//************************************************************************************************************************************************************************
//setup frequencies and motors control ***********************************************************************************************************************************
//************************************************************************************************************************************************************************
int value_motorA = 0, value_motorB = 0;

void output_PWM()
{
  setPWMPrescaler(PIN_PWM_1_MOTOR_A, PWM_MOTOR_A);
  setPWMPrescaler(PIN_PWM_3_MOTOR_B, PWM_MOTOR_B);
  
  //forward motorA
  if (rc_packet.ch_motorA > MID_CONTROL_VAL + DEAD_ZONE)
  {
    value_motorA = map(rc_packet.ch_motorA, MID_CONTROL_VAL + DEAD_ZONE, MAX_CONTROL_VAL, ACCELERATE_MOTOR_A, MAX_MOTOR_A);
    value_motorA = constrain(value_motorA, ACCELERATE_MOTOR_A, MAX_MOTOR_A);
    analogWrite(PIN_PWM_2_MOTOR_A, value_motorA); 
    digitalWrite(PIN_PWM_1_MOTOR_A, LOW);
  }
  //back motorA
  else if (rc_packet.ch_motorA < MID_CONTROL_VAL - DEAD_ZONE)
  {
    value_motorA = map(rc_packet.ch_motorA, MID_CONTROL_VAL - DEAD_ZONE, MIN_CONTROL_VAL, ACCELERATE_MOTOR_A, MAX_MOTOR_A);
    value_motorA = constrain(value_motorA, ACCELERATE_MOTOR_A, MAX_MOTOR_A);
    analogWrite(PIN_PWM_1_MOTOR_A, value_motorA);
    digitalWrite(PIN_PWM_2_MOTOR_A, LOW);
  }
  else
  {
    analogWrite(PIN_PWM_1_MOTOR_A, BRAKE_MOTOR_A);
    analogWrite(PIN_PWM_2_MOTOR_A, BRAKE_MOTOR_A);
  }
  //Serial.println(value_motorA); //print value ​​on a serial monitor
  
  //forward motorB
  if (rc_packet.ch_motorB > MID_CONTROL_VAL + DEAD_ZONE)
  {
    value_motorB = map(rc_packet.ch_motorB, MID_CONTROL_VAL + DEAD_ZONE, MAX_CONTROL_VAL, ACCELERATE_MOTOR_B, MAX_MOTOR_B);
    value_motorB = constrain(value_motorB, ACCELERATE_MOTOR_B, MAX_MOTOR_B);
    analogWrite(PIN_PWM_4_MOTOR_B, value_motorB);
    digitalWrite(PIN_PWM_3_MOTOR_B, LOW);
  }
  //back motorB
  else if (rc_packet.ch_motorB < MID_CONTROL_VAL - DEAD_ZONE)
  {
    value_motorB = map(rc_packet.ch_motorB, MID_CONTROL_VAL - DEAD_ZONE, MIN_CONTROL_VAL, ACCELERATE_MOTOR_B, MAX_MOTOR_B);
    value_motorB = constrain(value_motorB, ACCELERATE_MOTOR_B, MAX_MOTOR_B);
    analogWrite(PIN_PWM_3_MOTOR_B, value_motorB);
    digitalWrite(PIN_PWM_4_MOTOR_B, LOW);
  }
  else
  {
    analogWrite(PIN_PWM_3_MOTOR_B, BRAKE_MOTOR_B);
    analogWrite(PIN_PWM_4_MOTOR_B, BRAKE_MOTOR_B);
  }
  //Serial.println(value_motorB); //print value ​​on a serial monitor
}

//************************************************************************************************************************************************************************
//initial main settings **************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void setup()
{
  //Serial.begin(9600); //print value on a serial monitor
  //printf_begin();     //print the radio debug info
  
  pinMode(PIN_PWM_1_MOTOR_A, OUTPUT);
  pinMode(PIN_PWM_2_MOTOR_A, OUTPUT);
  pinMode(PIN_PWM_3_MOTOR_B, OUTPUT);
  pinMode(PIN_PWM_4_MOTOR_B, OUTPUT);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RX_BATTERY, INPUT);
  
  fail_safe();
  
  //define the radio communication
  radio.begin();
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(0, 0);
  radio.setChannel(RADIO_CHANNEL);
  radio.setDataRate(RF24_250KBPS);   //RF24_250KBPS (fails for units without +), RF24_1MBPS, RF24_2MBPS
  radio.setPALevel(RF24_PA_MIN);     //RF24_PA_MIN (-18dBm), RF24_PA_LOW (-12dBm), RF24_PA_HIGH (-6dbm), RF24_PA_MAX (0dBm)
  radio.openReadingPipe(1, address);
  radio.startListening();
}

//************************************************************************************************************************************************************************
//program loop ***********************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void loop()
{
  fail_safe_time();
  send_and_receive_data();
  output_PWM();
  
  //Serial.println("Radio details *****************");
  //radio.printDetails(); //print the radio debug info
}

//************************************************************************************************************************************************************************
//get time after losing RF data or turning off the TX, reset data and the LED flashing ***********************************************************************************
//************************************************************************************************************************************************************************
unsigned long fs_time = 0;

void fail_safe_time()
{
  if (millis() - fs_time > 1000) //1s
  {
    fail_safe();
    RF_off_check();
  }
}

//************************************************************************************************************************************************************************
//send and receive data **************************************************************************************************************************************************
//************************************************************************************************************************************************************************
byte telemetry_counter = 0;
unsigned int packet_state;
unsigned long rssi_time = 0;

void send_and_receive_data()
{
  if (radio.available())
  {
    radio.writeAckPayload(1, &telemetry_packet, sizeof(telemetry_packet_size));
    
    radio.read(&rc_packet, sizeof(rc_packet_size));

    telemetry_counter++;
    
    RX_batt_check();
    fs_time = millis();
  }
  else
  {
    if (micros() - rssi_time > 3800)
    {
      rssi_time = micros();
      packet_state++;
    }
  }
  
  if (packet_state > 254)
  {
    //telemetry_packet.rssi = telemetry_counter;
    telemetry_packet.rssi = map(telemetry_counter, 0, 254, 0, 100);
    telemetry_counter = 0;
    packet_state = 0;
  }
}

//************************************************************************************************************************************************************************
//reading adc RX battery. After receiving RF data, the monitored RX battery is activated *********************************************************************************
//when RX BATTERY_VOLTAGE < MONITORED_VOLTAGE = LED alarm RX flash at a interval of 0.5s. Battery OK = LED RX is lit *****************************************************
//************************************************************************************************************************************************************************
unsigned long adc_time = 0, led_time = 0;
bool batt_detect, led_state;

void RX_batt_check()
{
  if (millis() - adc_time > 1000) //delay adc reading RX battery
  {
    adc_time = millis();
    
    telemetry_packet.RX_batt_A1 = map(analogRead(PIN_RX_BATTERY), 0, 1023, 0, 255);
  }
  
  batt_detect = telemetry_packet.RX_batt_A1 <= (255 / BATTERY_VOLTAGE) * MONITORED_VOLTAGE;
  
  if (millis() - led_time > 500)
  {
    led_time = millis();
    
    if (led_state >= !batt_detect + HIGH)
    {
      led_state = LOW;
    }
    else
    {
      led_state = HIGH;
    }
    digitalWrite(PIN_LED, led_state);
  }
  //Serial.println(telemetry_packet.RX_batt_A1); //print value ​​on a serial monitor
}

//************************************************************************************************************************************************************************
//when RX is switched on and TX is switched off, or after the loss of RF data = LED RX flash at a interval of 0.1s. Normal mode = LED RX is lit **************************
//************************************************************************************************************************************************************************
void RF_off_check()
{
  if (millis() - led_time > 100)
  {
    led_time = millis();
    
    if (led_state)
    {
      led_state = LOW;
    }
    else
    {
      led_state = HIGH;
    }
    digitalWrite(PIN_LED, led_state);
  }
}
 
