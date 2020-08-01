#include <HCSR04.h>

// MonsterShield Pins
int inApin[2] = {7, 4};  // INA: Clockwise input
int inBpin[2] = {8, 9}; // INB: Counter-clockwise input
int pwmpin[2] = {5, 6}; // PWM input
int cspin[2] = {2, 3}; // CS: Current sense ANALOG input
int enpin[2] = {0, 1}; // EN: Status of switches output (Analog pin)
int statpin = 13; // StatusLed pin

UltraSonicDistanceSensor ultrasonicLeft(11, 12);
UltraSonicDistanceSensor ultrasonicRight(A4, A5);

// MonsterShield States
#define BRAKEVCC 0
#define CW   1
#define CCW  2
#define BRAKEGND 3
#define CS_THRESHOLD 100

//
bool limitLeft = false;
bool limitRight = false;
#define stopDistance 20

// UltraSonic1 variables
unsigned long duration1;
int distance1;
unsigned long previousMicros1 = 0;
int triggerState1 = LOW;

// UltraSonic2 variables
unsigned long duration2;
int distance2;
unsigned long previousMicros2 = 0;
int triggerState2 = LOW;

// General UltraSonic variables
long OnTime = 10; //microseconds of on-time
long OffTime = 3; //microseconds of off-time

void setup() {
  Serial.begin(9600);
  // MonsterShield status pin mode
  pinMode(statpin, OUTPUT);

  // MonsterShield motor pin modes
  for (int i = 0; i < 2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }

  // MotorShield initialize motors as breaked
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
}

// Message via serial
String serialInput = "";

void loop() {
  //checkUltrasonic();
  safetyCheck();
  while (Serial.available() > 0) {

    char serialChar = Serial.read();

    if (serialChar == 'w' || 'a' || 's' || 'd' || 'x') {
      serialInput += serialChar;
    }


    if (serialChar == '\n') {
      if (serialInput.charAt(0) == 'w') {
        motorGo(0, CCW, 100);
        motorGo(1, CW, 100);
      } else if (serialInput.charAt(0) == 'a') {
        motorGo(0, CCW, 100);
        motorGo(1, CCW, 100);
      } else if (serialInput.charAt(0) == 'd') {
        motorGo(0, CW, 100);
        motorGo(1, CW, 100);
      } else if (serialInput.charAt(0) == 's') {
        motorGo(0, CW, 100);
        motorGo(1, CCW, 100);
      } else if (serialInput.charAt(0) == 'x') {
        motorOff(0);
        motorOff(1);
      }

      serialInput = "";
    }
  }
}

void motorOff(int motor)
{
  // Initialize braked
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  analogWrite(pwmpin[motor], 0);
}

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{
  if (motor <= 1)
  {
    if (direct <= 4)
    {
      // Set inA[motor]
      if (direct <= 1)
        digitalWrite(inApin[motor], HIGH);
      else
        digitalWrite(inApin[motor], LOW);

      // Set inB[motor]
      if ((direct == 0) || (direct == 2))
        digitalWrite(inBpin[motor], HIGH);
      else
        digitalWrite(inBpin[motor], LOW);

      analogWrite(pwmpin[motor], pwm);
    }
  }
}

void safetyCheck() {
  double distanceLeft = ultrasonicLeft.measureDistanceCm();
  double distanceRight = ultrasonicRight.measureDistanceCm();

  if (distanceLeft == -1) {
    distanceLeft = stopDistance + 10;
  }
  if (distanceRight == -1) {
    distanceRight = stopDistance + 10;
  }

  if (distanceLeft <= stopDistance) {
    motorOff(0);
    motorOff(1);
    Serial.println("distance-stop:left");
    motorGo(0, CW, 80);
    motorGo(1, CCW, 80);
    delay(200);
    motorOff(0);
    motorOff(1);
    limitLeft = true;
  } else {
    limitLeft = false;
  }
  if (distanceRight <= stopDistance) {
    motorOff(0);
    motorOff(1);
    Serial.println("distance-stop:left");
    motorGo(0, CW, 80);
    motorGo(1, CCW, 80);
    delay(200);
    motorOff(0);
    motorOff(1);
    limitRight = true;
  } else {
    limitRight = false;
  }
}
