
//*******************************************************************************************************************************
// RC receiver 10ch (motor-servo driver, telemetry)
//**************************************************
// Simple RC receiver from my repository https://github.com/stanekTM/RX_nRF24_Motor_Servo/tree/master/RX_nRF24_10ch_Motor_Servo
//
// The hardware includes nRF24L01+ transceiver and ATmega328P processor.
// The firmware will be used for cars, boats, tanks, robots and aircraft. The code is Arduino.
//
// Works with RC transmitters:
// TX_nRF24_2ch_OLED          https://github.com/stanekTM/TX_nRF24_2ch_OLED
// TX_nRF24_5ch_LED           https://github.com/stanekTM/TX_nRF24_5ch_LED
// OpenAVRc                   https://github.com/Ingwie/OpenAVRc_Dev
// Multiprotocol from my fork https://github.com/stanekTM/TX_FW_Multi_Stanek
//*******************************************************************************************************************************

#include <RF24.h>         // https://github.com/nRF24/RF24
//#include <printf.h>       // print the radio debug info
#include <DigitalIO.h>    // https://github.com/greiman/DigitalIO
#include <Servo.h>        // Arduino standard library
#include "PWMFrequency.h" // used locally https://github.com/TheDIYGuy999/PWMFrequency


//setting a unique address (5 bytes number or character)
const byte address[] = "jirka";

//RF communication channel settings (0-125, 2.4Ghz + 76 = 2.476Ghz)
#define RADIO_CHANNEL  76

//setting the reaction of the motor to be rotated after the lever has been moved. Settings (0-255)
#define ACCELERATE_MOTOR_A  0
#define ACCELERATE_MOTOR_B  0

//setting the maximum motor power. Suitable for TX transmitters without endpoint setting. Settings (0-255)
#define MAX_FORW_MOTOR_A  255
#define MAX_BACK_MOTOR_A  255
#define MAX_FORW_MOTOR_B  255
#define MAX_BACK_MOTOR_B  255

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
#define PWM_MOTOR_A  64

//settings PWM (pin D9 or D10 are paired on timer1/16-bit, Servo library)
//1024 = 30Hz, 256 = 122Hz, 64 = 488Hz(default), 8 = 3906Hz
//#define PWM_MOTOR_A  256

//settings PWM (pin D3 or D11 are paired on timer2/8-bit, ServoTimer2 library)
//1024 = 30Hz, 256 = 122Hz, 128 = 244Hz, 64 = 488Hz(default), 32 = 976Hz, 8 = 3906Hz
#define PWM_MOTOR_B  256

//free pins
//pin                      0
//pin                      1
//pin                      A6

//pins for servos
#define PIN_SERVO_1        2
#define PIN_SERVO_2        4
#define PIN_SERVO_3        7
#define PIN_SERVO_4        8
#define PIN_SERVO_5        9
#define PIN_SERVO_6        10
#define PIN_SERVO_7        12 //MISO
#define PIN_SERVO_8        13 //SCK
 
//pwm pins for motor
#define PIN_PWM_1_MOTOR_A  5
#define PIN_PWM_2_MOTOR_A  6
#define PIN_PWM_3_MOTOR_B  3
#define PIN_PWM_4_MOTOR_B  11 //MOSI

//LED battery and RF on/off
#define PIN_LED            A5

//input battery
#define PIN_BATTERY        A7

//pins for nRF24L01
#define PIN_CE             A0
#define PIN_CSN            A1

//software SPI https://nrf24.github.io/RF24/md_docs_arduino.html
//----- SCK           16 - A2
//----- MOSI          17 - A3
//----- MISO          18 - A4

//setting of CE and CSN pins
RF24 radio(PIN_CE, PIN_CSN);

//*********************************************************************************************************************
//this structure defines the received data in bytes (structure size max. 32 bytes) ************************************
//*********************************************************************************************************************
struct rc_packet_size
{
  unsigned int ch_motorA = MID_CONTROL_VAL;
  unsigned int ch_motorB = MID_CONTROL_VAL;
  unsigned int ch_servo1 = MID_CONTROL_VAL;
  unsigned int ch_servo2 = MID_CONTROL_VAL;
  unsigned int ch_servo3 = MID_CONTROL_VAL;
  unsigned int ch_servo4 = MID_CONTROL_VAL;
  unsigned int ch_servo5 = MID_CONTROL_VAL;
  unsigned int ch_servo6 = MID_CONTROL_VAL;
  unsigned int ch_servo7 = MID_CONTROL_VAL;
  unsigned int ch_servo8 = MID_CONTROL_VAL;
};
rc_packet_size rc_packet;

//*********************************************************************************************************************
//this struct defines data, which are embedded inside the ACK payload *************************************************
//*********************************************************************************************************************
struct telemetry_packet_size
{
  byte rssi;
  byte batt_A1;
  byte batt_A2;
};
telemetry_packet_size telemetry_packet;

//*********************************************************************************************************************
//fail safe, settings 1000-2000 (MIN_CONTROL_VAL = 1000, MID_CONTROL_VAL = 1500, MAX_CONTROL_VAL = 2000) **************
//*********************************************************************************************************************
void fail_safe()
{
  rc_packet.ch_motorA = MID_CONTROL_VAL;
  rc_packet.ch_motorB = MID_CONTROL_VAL;
  rc_packet.ch_servo1 = MID_CONTROL_VAL;
  rc_packet.ch_servo2 = MID_CONTROL_VAL;
  rc_packet.ch_servo3 = MID_CONTROL_VAL;
  rc_packet.ch_servo4 = MID_CONTROL_VAL;
  rc_packet.ch_servo5 = MID_CONTROL_VAL;
  rc_packet.ch_servo6 = MID_CONTROL_VAL;
  rc_packet.ch_servo7 = MID_CONTROL_VAL;
  rc_packet.ch_servo8 = MID_CONTROL_VAL;
}

//*********************************************************************************************************************
//attach servo pins ***************************************************************************************************
//*********************************************************************************************************************
Servo servo1, servo2, servo3, servo4, servo5, servo6, servo7, servo8;

void attach_servo_pins()
{
  servo1.attach(PIN_SERVO_1);
  servo2.attach(PIN_SERVO_2);
  servo3.attach(PIN_SERVO_3);
  servo4.attach(PIN_SERVO_4);
  servo5.attach(PIN_SERVO_5);
  servo6.attach(PIN_SERVO_6);
  servo7.attach(PIN_SERVO_7);
  servo8.attach(PIN_SERVO_8);
}

//*********************************************************************************************************************
//servo outputs *******************************************************************************************************
//*********************************************************************************************************************
void output_servo()
{
  servo1.writeMicroseconds(rc_packet.ch_servo1);
  servo2.writeMicroseconds(rc_packet.ch_servo2);
  servo3.writeMicroseconds(rc_packet.ch_servo3);
  servo4.writeMicroseconds(rc_packet.ch_servo4);
  servo5.writeMicroseconds(rc_packet.ch_servo5);
  servo6.writeMicroseconds(rc_packet.ch_servo6);
  servo7.writeMicroseconds(rc_packet.ch_servo7);
  servo8.writeMicroseconds(rc_packet.ch_servo8);
  
  //Serial.println(rc_packet.ch_servo1);
}

//*********************************************************************************************************************
//setup frequencies and motors control ********************************************************************************
//*********************************************************************************************************************
int value_motorA = 0, value_motorB = 0;

void output_PWM()
{
  //forward motorA
  if (rc_packet.ch_motorA > MID_CONTROL_VAL + DEAD_ZONE)
  {
    value_motorA = map(rc_packet.ch_motorA, MID_CONTROL_VAL + DEAD_ZONE, MAX_CONTROL_VAL, ACCELERATE_MOTOR_A, MAX_FORW_MOTOR_A);
    value_motorA = constrain(value_motorA, ACCELERATE_MOTOR_A, MAX_FORW_MOTOR_A);
    analogWrite(PIN_PWM_2_MOTOR_A, value_motorA); 
    digitalWrite(PIN_PWM_1_MOTOR_A, LOW);
  }
  //back motorA
  else if (rc_packet.ch_motorA < MID_CONTROL_VAL - DEAD_ZONE)
  {
    value_motorA = map(rc_packet.ch_motorA, MID_CONTROL_VAL - DEAD_ZONE, MIN_CONTROL_VAL, ACCELERATE_MOTOR_A, MAX_BACK_MOTOR_A);
    value_motorA = constrain(value_motorA, ACCELERATE_MOTOR_A, MAX_BACK_MOTOR_A);
    analogWrite(PIN_PWM_1_MOTOR_A, value_motorA);
    digitalWrite(PIN_PWM_2_MOTOR_A, LOW);
  }
  else
  {
    analogWrite(PIN_PWM_1_MOTOR_A, BRAKE_MOTOR_A);
    analogWrite(PIN_PWM_2_MOTOR_A, BRAKE_MOTOR_A);
  }
  //Serial.println(value_motorA);
  
  //forward motorB
  if (rc_packet.ch_motorB > MID_CONTROL_VAL + DEAD_ZONE)
  {
    value_motorB = map(rc_packet.ch_motorB, MID_CONTROL_VAL + DEAD_ZONE, MAX_CONTROL_VAL, ACCELERATE_MOTOR_B, MAX_FORW_MOTOR_B);
    value_motorB = constrain(value_motorB, ACCELERATE_MOTOR_B, MAX_FORW_MOTOR_B);
    analogWrite(PIN_PWM_4_MOTOR_B, value_motorB);
    digitalWrite(PIN_PWM_3_MOTOR_B, LOW);
  }
  //back motorB
  else if (rc_packet.ch_motorB < MID_CONTROL_VAL - DEAD_ZONE)
  {
    value_motorB = map(rc_packet.ch_motorB, MID_CONTROL_VAL - DEAD_ZONE, MIN_CONTROL_VAL, ACCELERATE_MOTOR_B, MAX_BACK_MOTOR_B);
    value_motorB = constrain(value_motorB, ACCELERATE_MOTOR_B, MAX_BACK_MOTOR_B);
    analogWrite(PIN_PWM_3_MOTOR_B, value_motorB);
    digitalWrite(PIN_PWM_4_MOTOR_B, LOW);
  }
  else
  {
    analogWrite(PIN_PWM_3_MOTOR_B, BRAKE_MOTOR_B);
    analogWrite(PIN_PWM_4_MOTOR_B, BRAKE_MOTOR_B);
  }
  //Serial.println(value_motorB);
}

//*********************************************************************************************************************
//initial main settings ***********************************************************************************************
//*********************************************************************************************************************
void setup()
{
  //Serial.begin(9600); //print value on a serial monitor
  //printf_begin();     //print the radio debug info
  
  pinMode(PIN_PWM_1_MOTOR_A, OUTPUT);
  pinMode(PIN_PWM_2_MOTOR_A, OUTPUT);
  pinMode(PIN_PWM_3_MOTOR_B, OUTPUT);
  pinMode(PIN_PWM_4_MOTOR_B, OUTPUT);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BATTERY, INPUT);
  
  fail_safe();
  attach_servo_pins();
  
  //frequency setting
  setPWMPrescaler(PIN_PWM_1_MOTOR_A, PWM_MOTOR_A);
  setPWMPrescaler(PIN_PWM_3_MOTOR_B, PWM_MOTOR_B);
  
  //define the radio communication
  radio.begin();
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(5, 5);
  radio.setChannel(RADIO_CHANNEL);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); //RF24_PA_MIN (-18dBm), RF24_PA_LOW (-12dBm), RF24_PA_HIGH (-6dbm), RF24_PA_MAX (0dBm)
  radio.openReadingPipe(1, address);
  radio.startListening();
}

//*********************************************************************************************************************
//program loop ********************************************************************************************************
//*********************************************************************************************************************
void loop()
{
  //radio.printDetails();       //(smaller) print raw register values
  //radio.printPrettyDetails(); //(larger) print human readable data
  
  output_servo();
  output_PWM();
  fail_safe_time();
  send_and_receive_data();
}

//*********************************************************************************************************************
//get time after losing RF data or turning off the TX, reset data and the LED flashing ********************************
//*********************************************************************************************************************
unsigned long fs_time = 0;

void fail_safe_time()
{
  if (millis() - fs_time > 1000) //after 1 second
  {
    fail_safe();
    RF_off_check();
  }
}

//*********************************************************************************************************************
//send and receive data ***********************************************************************************************
//*********************************************************************************************************************
byte packet_counter = 0;
unsigned long packet_time = 0;

void send_and_receive_data()
{
  if (radio.available())
  {
    radio.writeAckPayload(1, &telemetry_packet, sizeof(telemetry_packet_size));
    
    radio.read(&rc_packet, sizeof(rc_packet_size));
    
    packet_counter++;
    
    battery_check();
    fs_time = millis();
  }
  
  if (millis() - packet_time > 1000)
  {
    packet_time = millis();
    //telemetry_packet.rssi = packet_counter;
    telemetry_packet.rssi = map(packet_counter, 0, 10, 0, 100);
    //telemetry_packet.rssi = constrain(telemetry_packet.rssi, 0, 100);
    packet_counter = 0;
  }
}

//*********************************************************************************************************************
//reading adc battery. After receiving RF data, the monitored battery is activated. Battery OK = LED is lit ***********
//When BATTERY_VOLTAGE < MONITORED_VOLTAGE = LED alarm flash at a interval of 0.5s ************************************
//*********************************************************************************************************************
unsigned long adc_time = 0, led_time = 0;
bool low_batt_detect = 0, previous_state_batt, batt_led_state = 1, RF_led_state;

void battery_check()
{
  if (millis() - adc_time > 1000) //delay adc reading battery
  {
    adc_time = millis();
    
    telemetry_packet.batt_A1 = map(analogRead(PIN_BATTERY), 0, 1023, 0, 255);
    
    low_batt_detect = telemetry_packet.batt_A1 <= (255 / BATTERY_VOLTAGE) * MONITORED_VOLTAGE;
  }
  
  digitalWrite(PIN_LED, batt_led_state);
  
  if (low_batt_detect)
  {
    previous_state_batt = 1;
    
    if (millis() - led_time > 500)
    {
      led_time = millis();
      
      batt_led_state = !batt_led_state;
    }
  }
  low_batt_detect = previous_state_batt;
  
  //Serial.println(low_batt_detect);
}

//*********************************************************************************************************************
//when RX is switched on and TX is switched off, or after the loss of RF data = LED flash at a interval of 0.1s *******
//Normal mode = LED is lit ********************************************************************************************
//*********************************************************************************************************************
void RF_off_check()
{
  digitalWrite(PIN_LED, RF_led_state);
  
  if (millis() - led_time > 100)
  {
    led_time = millis();
    
    RF_led_state = !RF_led_state;
  }
}
 
