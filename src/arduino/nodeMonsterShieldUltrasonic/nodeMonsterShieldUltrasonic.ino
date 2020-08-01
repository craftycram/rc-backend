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
bool emergencyStop = false;
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
  while (Serial.available() > 0 && !emergencyStop) {

    char serialChar = Serial.read();

    if (isDigit(serialChar) || serialChar == 'l' || 'r' || '-') {
      serialInput += serialChar;
    }

    bool forward = true;

    if (serialChar == '\n') {
      if (serialInput.charAt(0) == 'l') {

        serialInput.remove(0, 1);

        if (serialInput.charAt(0) == '-') {
          forward = false;
          serialInput.remove(0, 1);
        } else {
          forward = true;
        }

        if (serialInput.toInt() > 0) {
          if (forward) {
            motorGo(0, CCW, serialInput.toInt());
          } else {
            motorGo(0, CW, serialInput.toInt());
          }
          Serial.println("left > " + serialInput);
          Serial.println("left - forward > " + forward);
        } else {
          motorOff(0);
          Serial.println("left > stop");
        }
      } else if (serialInput.charAt(0) == 'r') {
        serialInput.remove(0, 1);

        if (serialInput.charAt(0) == '-') {
          forward = false;
          serialInput.remove(0, 1);
        } else {
          forward = true;
        }

        if (serialInput.toInt() > 0) {
          if (forward) {
            motorGo(1, CW, serialInput.toInt());
          } else {
            motorGo(1, CCW, serialInput.toInt());
          }
          Serial.println("right > " + serialInput);
          Serial.println("right - forward > " + forward);
        } else {
          motorOff(1);
          Serial.println("right > stop");
        }
      }

      serialInput = "";
    }
  }
}

// Functions

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

/* motorGo() will set a motor going in a specific direction
  the motor will continue going in that direction, at that speed
  until told to do otherwise.

  motor: this should be either 0 or 1, will selet which of the two
  motors to be controlled

  direct: Should be between 0 and 3, with the following result
  0: Brake to VCC
  1: Clockwise
  2: CounterClockwise
  3: Brake to GND

  pwm: should be a value between ? and 1023, higher the number, the faster
  it'll go
*/
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
  if (ultrasonicLeft.measureDistanceCm() <= stopDistance || ultrasonicRight.measureDistanceCm() <= stopDistance) {
    motorOff(0);
    motorOff(1);
    Serial.println("Abstand zu klein! Stop!");
    emergencyStop = true;
    while (Serial.available() > 0) {
      Serial.read();
    }
  } else {
    emergencyStop = false;
  }
}
