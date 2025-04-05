#include <Arduino.h>
#include <L298N.h>
#include <Servo.h>
#include <string.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <math.h>

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// A====> left
// B====> Right

#define PWMA 4
#define PWMB 5
#define AIN1 22
#define AIN2 23
#define BIN1 24
#define BIN2 25

// TCS3200 (HW-067) required pin definitions
#define S0 27  // Frequency scaling
#define S2 26  // Filter select
#define S3 29  // Filter select
#define OUT 28 // Frequency output

// TCS3200 (HW-067) required pin definitions for Ball detect
#define S0_b 44  // Frequency scaling
#define S2_b 43  // Filter select
#define S3_b 46  // Filter select
#define OUT_b 47 // Frequency output


#define YELLOW_LED 41  // Yellow LED indicator
#define WHITE_LED 43   // White LED indicator
#define RED_LED 40
#define ORANGE_LED 42

#define R_trig 30
#define R_echo 31
#define L_trig 35 // finalized
#define L_echo 34 // finalized
#define F_trig 32
#define F_echo 33

// For wall  follow
#define DESIRED_DISTANCE 15  // Target wall distance in cm
// PID constants
float Kp_w = 8;
float Ki_w = 0.0;
float Kd_w = 4;

float error_w = 0, previousError_w = 0;
float P_w = 0, I_w = 0, D_w = 0;
float errorArray_w[2];  // For advanced PID
float lfspeed_w = 120;   // Base speed
float lsp_w, rsp_w;




unsigned long redMinWhite, redMaxWhite, greenMinWhite, greenMaxWhite, blueMinWhite, blueMaxWhite;
unsigned long redMinYellow, redMaxYellow, greenMinYellow, greenMaxYellow, blueMinYellow, blueMaxYellow;

unsigned long redFreq, greenFreq, blueFreq;

// Create servo objects for each joint
Servo baseServo;      // Base rotation (0-360° capable)
Servo shoulderServo;  // Shoulder joint (0-180°)
Servo elbowServo;    // Elbow joint (0-180°)
Servo gripperServo;  // Gripper (0-180°)


// Pin definitions
const int BASE_PIN = 7;
const int SHOULDER_PIN = 8;
const int ELBOW_PIN = 6;
const int GRIPPER_PIN = 9;
const int RIGHT_PIN = 10;
const int LEFT_PIN = 11;


// Variables to store servo positions
int basePos = 120;     // Initial position for base (0-360°)
int shoulderPos = 0;  // Initial position for shoulder (0-180°)
int elbowPos = 180;   // Initial position for elbow (0-180°)
int gripperPos = 0;   // Gripper fully closed initially (0-180°)
int ball_count=0;


// for IR_array
const int SensorCount = 8; // Total number of sensors
const int analogSensorCount = 8; // Number of analog sensors
const int analogSensorPins[analogSensorCount] = {A7, A6, A5, A4, A3, A2, A1, A0}; // Analog pins for the remaining sensors

int IR_weight[SensorCount] = {-50, -30, -15, -5, 5, 15, 30, 50};
float errorArray[50] = {0};
int sensorValues[SensorCount];
int sensorValues_w[SensorCount];

// PID control parameters
float Kp = 5; // Proportional term
float Ki = 0; // Integral term
float Kd = 5; // Derivative term
float sum;
int increment_count = 1;

long ldistance;
long rdistance;
float error;

// PID variables
float P, I, D, previousError = 0;
float lsp, rsp;
int lfspeed = 100;

String junction = "a";

// Function prototypes
void readSensors(int *values);

void Task1();
void Task2();
void Task3();
void Task4();
void Task5(); // Hidden
void Task6();

void moveStraightPID(); // use with a while loop terminating with encoderCountA or B
void encoder_backward();  //use with a while loop terminating with encoderCountA or B
void moveTurnRightPID();  // use with a while loop terminating with encoderCountA or B
void moveTurnLeftPID(); // use with a while loop terminating with encoderCountA or B

void Go_straight_Normal_line();   // can go straight and correct using perpendicular lines

void turnLeft();  //can use
void turnRight(); //can use
void half_rotation(); // clockwise
void half_rotation_L(); // counter clockwise
void forward_encoder(int count); 
void backward_encoder(int count); 


void encoderISR_A();
void encoderISR_B();

void line_follow_juction_turns(); // can use anywhere // able to take decision at junctions
void line_follow(int *sensorValues);  // make a function like line_follow_juction_turns()

void check_juction( int count);
void check_juction_second(int *sensorValues, int count);
bool isImmediateTurnL(int *sensorValues);
bool isImmediateTurnR(int *sensorValues);
bool isImmediateTurnT(int *sensorValues);

int Green_White_Detect(); // Detect Green and White balls (green--> 1, white --> 0)
void colomcheck();  // for task 1 check for the ball
void colomcheck2();
void move_for_grabbing(int count);
//void turnRight45();
//void turnLeft45();
void task1_start();

float calculatePID(int *sensorValues);
void motor_drive(int left, int right);
void PID_Linefollow(float pidValue);        //use with calculatePID
void PID_Linefollow_Normal(float pidValue);   // Adjest Normal to the line //use with calculatePID

void smoothMoveServo(Servo &servo, int &currentPos, int targetPos);
String ball_colour(); 
void calibrateColors();   // only with white and yellow balls********************************** Have to calibrate others
void readColors();

float measureDistance(int trigPin, int echoPin);
void wall_follow(float pidValue);
float calculatePID_wall();

// while(true){
//   float pidvalue= calculatePID_wall();
//   wall_follow(pidvalue);
// }

L298N motor1(PWMA, AIN1, AIN2);
L298N motor2(PWMB, BIN1, BIN2);

#define ENCODER_A1 2 // Encoder A channel (Interrupt pin)
#define ENCODER_B1 3 // Encoder B channel (Interrupt pin)

// Encoder counts
volatile long encoderCountA = 0; // Encoder count for Motor A
volatile long encoderCountB = 0; // Encoder count for Motor B
const long targetCounts = 150;   // 185       // Desired encoder counts for 90 degree turns
const long targetCounts_rotation = 300; // Desired encoder counts for 180 degree turns
const int count_moveForward = 55;
const int count_moveForward_w = 125;

// PID parameters for encoders
float Kp_e = 5; // Proportional term
float Ki_e = 0; // Integral term
float Kd_e = 5; // Derivative term
float errorenco = 0;
float previousErrorenco = 0;
float integralenco = 0;
float derivativeenco = 0;

// Target speed for motors
int baseSpeed = 100; // Base speed for both motors (0 to 255)///////// for Line Navigation 115

int plus = 0;
bool is_plus=false;


int green_detect = 0; // if 1 --> green, 0 --> white     ********************************************************
bool check_again = false;
//int green_temp_array[5]= {2,0,1,1,2};
//char temp_yellow_white[5]={'W','Y','W','Y','Y'};
//int tem_pos=0;

// moving out of the grid 
int Tdetect=0;

void setup() {
  for (int i = 0; i < analogSensorCount; i++) {
    pinMode(analogSensorPins[i], INPUT);
  }

  // Initialize motor driver pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  //pinMode(53, OUTPUT); // led indicate in virtual box

  // Encoder pins setup
  pinMode(ENCODER_A1, INPUT_PULLUP);
  pinMode(ENCODER_B1, INPUT_PULLUP);

  // Attach interrupts for encoders
  attachInterrupt(digitalPinToInterrupt(ENCODER_A1), encoderISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B1), encoderISR_B, RISING);

  // set sonnar pins HC-SR04
  pinMode(L_trig, OUTPUT);   // Set trig pin as output
  pinMode(L_echo, INPUT);    // Set echo pin as input
  pinMode(R_trig, OUTPUT);   // Set trig pin as output
  pinMode(R_echo, INPUT);    // Set echo pin as input
  pinMode(F_trig, OUTPUT);   // Set trig pin as output
  pinMode(F_echo, INPUT);    // Set echo pin as input

  // give initial values to servo motors

  // Attach servos to their respective pins
  baseServo.attach(BASE_PIN);
  shoulderServo.attach(SHOULDER_PIN);
  elbowServo.attach(ELBOW_PIN);
  gripperServo.attach(GRIPPER_PIN);


  // Initialize TCS34725 color sensor
  pinMode(S0, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  // Set pin modes for LEDs
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);

  // Initial state: LEDs off
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(WHITE_LED, LOW);
  pinMode(RED_LED, LOW);
  pinMode(ORANGE_LED, LOW);

  digitalWrite(S0, HIGH); // LOW to HIGH

  Serial.begin(9600);

  // Set initial positions
  baseServo.write(basePos);         // Base: 0-360°
  shoulderServo.write(shoulderPos); // Others: 0-180°
  elbowServo.write(elbowPos);
  gripperServo.write(gripperPos);

  //calibrateColors(); // Only for White and Yellow balls

}

void loop() {
  Task1();
  //line_follow_juction_turns();
  //readSensors(sensorValues);
  //check_juction(count_moveForward);

}

void Go_straight_Normal_line(){
  while (true){
    readSensors(sensorValues);
    bool Is_black = (sensorValues[0] == 0 && sensorValues[1] == 0 && sensorValues[2] == 0 && sensorValues[3] == 0 && sensorValues[4] == 0 && sensorValues[5] == 0 && sensorValues[6] == 0 && sensorValues[7] == 0);


  if(Is_black){

    forward_encoder(50);

  }else{
    float pidValue = calculatePID(sensorValues);
    PID_Linefollow_Normal(pidValue);

  }
  }
  
}

void calibrateColors() {
  // Calibrate White
  //Serial.println("Place a WHITE object in front of the sensor and press any key...");
  digitalWrite(WHITE_LED, HIGH);  // Indicate white calibration
  digitalWrite(YELLOW_LED, LOW);

  delay(5000);
  Serial.read();  // Clear buffer
  digitalWrite(WHITE_LED, LOW);  // Turn off LED during reading

  readColors();
  redMinWhite = redFreq - 10; redMaxWhite = redFreq + 10;
  greenMinWhite = greenFreq - 10; greenMaxWhite = greenFreq + 10;
  blueMinWhite = blueFreq - 10; blueMaxWhite = blueFreq + 10;

  delay(1000);  // Brief pause

  // Calibrate Yellow
  Serial.println("Place a YELLOW object in front of the sensor and press any key...");
  digitalWrite(YELLOW_LED, HIGH);  // Indicate yellow calibration
  digitalWrite(WHITE_LED, LOW);

  delay(5000);

  Serial.read();  // Clear buffer
  digitalWrite(YELLOW_LED, LOW);  // Turn off LED during reading

  readColors();
  redMinYellow = redFreq - 10; redMaxYellow = redFreq + 10;
  greenMinYellow = greenFreq - 10; greenMaxYellow = greenFreq + 10;
  blueMinYellow = blueFreq - 10; blueMaxYellow = blueFreq + 10;
}


// Have to calibrate this with the arena
void readSensors(int *values) {
  // Read analog sensors
  for (int i = 0; i < analogSensorCount; i++) {
    values[i] = analogRead(analogSensorPins[i]) > 80 ? 0 : 1; // Assuming higher values indicate no line (sensor readings > 135) black ====> return 0 (no line)
    //Serial.print(values[i]);
    //Serial.print("\t");
  }
  Serial.println();
}

void line_follow(int *sensorValues) {
  if (isImmediateTurnL(sensorValues)) {
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction(count_moveForward);
  } else if (isImmediateTurnR(sensorValues)) {
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction(count_moveForward);
  } else if (isImmediateTurnT(sensorValues)) {
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction(count_moveForward);
  } else { 
    float pidValue = calculatePID(sensorValues);
    PID_Linefollow(pidValue);
  }
}

float calculatePID(int *sensorValues) {
  float position = 0;
  int onLine = 0;

  // Loop through all sensors
  for (int i = 0; i < SensorCount; i++) {
    if (sensorValues[i] == 1) { // Assuming 1 indicates line detected
      position += IR_weight[i];
      onLine++;
    }
  }

  // If no line is detected by any sensor, use the last known error value
  if (onLine == 0) {
    error = -previousError;
  } else {
    position /= onLine;
    error = 0 - position;   // desired value is zero(the line is middle respect to the IR array)
  }
  for (int i = 49; i > 0; i--) {        // // /////Shift the error values in the array********************FOR More advanced PID
    errorArray[i] = errorArray[i - 1];
  }
  errorArray[0] = error;
  P = error;
  I += error;
  D = error - previousError;
  previousError = error;

  float pidValue = (Kp * P) + (Ki * I) + (Kd * D);

  return pidValue;
}

void motor_drive(float left, float right) {
    int absRight = abs(right); // Absolute value for right speed
    int absLeft = abs(left); // Absolute value for left speed
  
    if (left > 0) {
      motor2.setSpeed(absLeft);
      motor2.forward();
    } else {
      motor2.setSpeed(absLeft);
      motor2.backward();
    }

    if (right > 0) {
      motor1.setSpeed(absRight);
      motor1.forward();
    } else {
      motor1.setSpeed(absRight);
      motor1.backward();
    }
  }

void PID_Linefollow(float pidValue) {
  lsp = lfspeed - pidValue;
  rsp = lfspeed + pidValue;

  if (lsp > 255) {
    lsp = 255;
  }
  if (lsp < -255) {
    lsp = -255;
  }
  if (rsp > 255) {
    rsp = 255;
  }
  if (rsp < -255) {
    rsp = 255;
  }
  motor_drive(lsp, rsp);
}

void PID_Linefollow_Normal(float pidValue) {
  lsp = 120 + pidValue;
  rsp = 120 - pidValue;

  if (lsp > 255) {
    lsp = 255;
  }
  if (lsp < -255) {
    lsp = -255;
  }
  if (rsp > 255) {
    rsp = 255;
  }
  if (rsp < -255) {
    rsp = 255;
  }
  motor_drive(lsp, rsp);
}



bool isImmediateTurnL(int *sensorValues) {
  bool turnLeft = (sensorValues[0] == 0 && sensorValues[1] == 1 && sensorValues[2] == 1 && sensorValues[3] == 1 && sensorValues[4] == 1 && sensorValues[5] == 1 && sensorValues[6] == 1 && sensorValues[7] == 1) || (sensorValues[0] == 0 && sensorValues[1] == 0 && sensorValues[2] == 1 && sensorValues[3] == 1 && sensorValues[4] == 1 && sensorValues[5] == 1 && sensorValues[6] == 1 && sensorValues[7] == 1);
  return turnLeft;
}

bool isImmediateTurnR(int *sensorValues) {
  bool turnRight = (sensorValues[0] == 1 && sensorValues[1] == 1 && sensorValues[2] == 1 && sensorValues[3] == 1 && sensorValues[4] == 1 && sensorValues[5] == 1 && sensorValues[6] == 1 && sensorValues[7] == 0) || (sensorValues[0] == 1 && sensorValues[1] == 1 && sensorValues[2] == 1 && sensorValues[3] == 1 && sensorValues[4] == 1 && sensorValues[5] == 1 && sensorValues[6] == 0 && sensorValues[7] == 0);
  return turnRight;
}

bool isImmediateTurnT(int *sensorValues) {
  bool turnT = sensorValues[0] == 1 && sensorValues[1] == 1 && sensorValues[2] == 1 && sensorValues[3] == 1 && sensorValues[4] == 1 && sensorValues[5] == 1 && sensorValues[6] == 1 && sensorValues[7] == 1;
  return turnT;
}

void check_juction(int count) {

  forward_encoder(15);

  motor1.stop();
  motor2.stop();
  delay(5);
  readSensors(sensorValues);
  if (isImmediateTurnL(sensorValues)) {
    junction = "L";
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction_second(sensorValues, count);
  } else if (isImmediateTurnR(sensorValues)) {
    junction = "R";
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction_second(sensorValues, count);
  } else if (isImmediateTurnT(sensorValues)) {
    junction = "T";
    motor1.stop();
    motor2.stop();
    delay(5);
    check_juction_second(sensorValues, count);
  } else {
    float pidValue = calculatePID(sensorValues);
    PID_Linefollow(pidValue);
  }
}

void forward_encoder(int count) {
  encoderCountA = 0;
  encoderCountB = 0;

  while (encoderCountA < count && encoderCountB < count) {
    moveStraightPID();
  }
  motor1.stop();
  motor2.stop();
  delay(1000);
}

void backward_encoder(int count) {
  encoderCountA = 0;
  encoderCountB = 0;

  while (encoderCountA < count && encoderCountB < count) {
    encoder_backward();
  }
  motor1.stop();
  motor2.stop();
  delay(1000);
}


void check_juction_second(int *sensorValues, int count) {
  readSensors(sensorValues);
  encoderCountA = 0;
  encoderCountB = 0;

  while (encoderCountA < count && encoderCountB < count) {
    moveStraightPID();
  }
  motor1.stop();
  motor2.stop();
  delay(50);
  readSensors(sensorValues);
  delay(50);

  if (junction == "L") {
    if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] < 2) {
      junction = "LL";
    } else if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] > 6) {
      junction = "TB";
    } else {
      junction = "LT";
    }
  }
  if (junction == "R") {
    if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] < 2) {
      junction = "RR";
    } else if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] > 6) {
      junction = "TB";
    } else {
      junction = "RT";
    }
  }
  if (junction == "T") {
    if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] < 2) {
      junction = "TT";
    } else if (sensorValues[0] + sensorValues[1] + sensorValues[2] + sensorValues[3] + sensorValues[4] + sensorValues[5] + sensorValues[6] + sensorValues[7] > 6) {
      junction = "TB";
    } else {
      junction = "TP";
    }
  }
  Serial.println(junction);
}

void turnRight() {
  encoderCountA = 0;
  encoderCountB = 0;

  while (encoderCountA < targetCounts && encoderCountB < targetCounts) {
    moveTurnRightPID();
  }
  motor1.stop();
  motor2.stop();
}

void turnLeft() {
  encoderCountA = 0;
  encoderCountB = 0;

  while (encoderCountB < targetCounts && encoderCountA < targetCounts) {
    moveTurnLeftPID();
  }
  motor1.stop();
  motor2.stop();
}


void task1_start() {
  turnRight();

  forward_encoder(150);
  motor1.stop();
  motor2.stop();
  delay(5);

  turnLeft();
}

void encoderISR_A() {
  encoderCountA++;
}

void encoderISR_B() {
  encoderCountB++;
}

void line_follow_juction_turns() {
  readSensors(sensorValues);
  line_follow(sensorValues);

  if (junction == "LL") {
    turnLeft();
  } else if (junction == "LT") {

    is_plus = true;

    //green_detect = Green_White_Detect();

  } else if (junction == "RR") {
    turnRight();
  } else if (junction == "RT") {
    //line_follow(sensorValues);
    is_plus = true;

    //green_detect = Green_White_Detect();

  } else if (junction == "TP") {
    is_plus = true;
    //green_detect = Green_White_Detect();

  

    //green_detect = Green_White_Detect();
    // if (plus==0){
    //   green_detect = Green_White_Detect();
    // }
    // else if (plus==1 or plus==2){
    //   turnLeft();
    //   green_detect = Green_White_Detect();
    //   turnRight();
    // }
          //*******************************************************
    //Serial.println(plus);


    //Serial.println(green_detect);
  } else if (junction == "TT") {
      Tdetect = 1;
  } else if (junction == "TB") {
    motor1.stop();
    motor2.stop();
    //delay(3000);
    increment_count++;
  }
  junction = "a";
}

void moveStraightPID() {
  errorenco = encoderCountA - encoderCountB;

  integralenco += errorenco;
  derivativeenco = errorenco - previousErrorenco;

  float correction = (Kp_e * errorenco) + (Ki_e * integralenco) + (Kd_e * derivativeenco);

  int motor1Speed = baseSpeed - correction;
  int motor2Speed = baseSpeed + correction;

  motor1Speed = constrain(motor1Speed, 0, 255);
  motor2Speed = constrain(motor2Speed, 0, 255);

  motor1.setSpeed(motor1Speed);
  motor1.forward();
  motor2.setSpeed(motor2Speed);
  motor2.forward();

  previousErrorenco = errorenco;
}



void half_rotation() {
  // Perform a sharp left turn by 
  // Assuming motor1 is the right motor and motor2 is the left motor
  encoderCountA = 0;
  encoderCountB = 0;

  while ((encoderCountB < targetCounts_rotation) && (encoderCountA < targetCounts_rotation)) {
    //Serial.println(encoderCountA,encoderCountB);

    moveTurnRightPID();
  
    // motor2.setSpeed(120);
    // motor2.forward();
    // motor1.setSpeed(120);
    // motor1.backward();
  }
  motor1.stop();
  motor2.stop();

}

void half_rotation_L() {
  // Perform a sharp left turn by 
  // Assuming motor1 is the right motor and motor2 is the left motor
  encoderCountA = 0;
  encoderCountB = 0;

  while ((encoderCountB < targetCounts_rotation) && (encoderCountA < targetCounts_rotation)) {
    //Serial.println(encoderCountA,encoderCountB);

    moveTurnLeftPID();
  

  }
  motor1.stop();
  motor2.stop();

}




void encoder_backward(){

    // Calculate the error (difference between encoder counts)

  errorenco = encoderCountA - encoderCountB;
  //Serial.println(errorenco);

  // Calculate the integral and derivative terms
  integralenco += errorenco;
  derivativeenco = errorenco - previousErrorenco;

  // Compute the PID correction
  float correction = (Kp_e * errorenco) + (Ki_e * integralenco) + (Kd_e * derivativeenco);

  // Adjust motor speeds based on the correction
  int motor1Speed = baseSpeed - correction;  // Motor A
  int motor2Speed = baseSpeed + correction;  // Motor B

  // Constrain the motor speeds to valid PWM values (0 to 255)
  motor1Speed = constrain(motor1Speed, 0, 255);
  motor2Speed = constrain(motor2Speed, 0, 255);
  //Serial.println(motor1Speed);
  //Serial.println(motor2Speed);

  // Set motor speeds
  motor1.setSpeed(motor1Speed);
  motor1.backward();
  motor2.setSpeed(motor2Speed);
  motor2.backward();
  // Save the current error for the next iteration
  previousErrorenco = errorenco;

}     



void moveTurnRightPID() {
  // Calculate the error (difference between encoder counts)

  errorenco = encoderCountA - encoderCountB;
  //Serial.println(errorenco);

  // Calculate the integral and derivative terms
  integralenco += errorenco;
  derivativeenco = errorenco - previousErrorenco;

  // Compute the PID correction
  float correction = (Kp_e * errorenco) + (Ki_e * integralenco) + (Kd_e * derivativeenco);

  // Adjust motor speeds based on the correction
  int motor1Speed = 130 - correction;  // Motor A
  int motor2Speed = 130 + correction;  // Motor B

  // Constrain the motor speeds to valid PWM values (0 to 255)
  motor1Speed = constrain(motor1Speed, 0, 255);
  motor2Speed = constrain(motor2Speed, 0, 255);
  //Serial.println(motor1Speed);
  //Serial.println(motor2Speed);

  // Set motor speeds
  motor1.setSpeed(motor1Speed);
  motor1.forward();
  motor2.setSpeed(motor2Speed);
  motor2.backward();
  // Save the current error for the next iteration
  previousErrorenco = errorenco;

}

void moveTurnLeftPID() {
  // Calculate the error (difference between encoder counts)

  errorenco = encoderCountA - encoderCountB;
  //Serial.println(errorenco);

  // Calculate the integral and derivative terms
  integralenco += errorenco;
  derivativeenco = errorenco - previousErrorenco;

  // Compute the PID correction
  float correction = (Kp_e * errorenco) + (Ki_e * integralenco) + (Kd_e * derivativeenco);

  // Adjust motor speeds based on the correction
  int motor1Speed = 130 - correction;  // Motor A
  int motor2Speed = 130 + correction;  // Motor B

  // Constrain the motor speeds to valid PWM values (0 to 255)
  motor1Speed = constrain(motor1Speed, 0, 255);
  motor2Speed = constrain(motor2Speed, 0, 255);
  //Serial.println(motor1Speed);
  //Serial.println(motor2Speed);

  // Set motor speeds
  motor1.setSpeed(motor1Speed);
  motor1.backward();
  motor2.setSpeed(motor2Speed);
  motor2.forward();
  // Save the current error for the next iteration
  previousErrorenco = errorenco;

}


void Task1(){  
  task1_start();

  while(true){
    colomcheck();
    if (ball_count==5 && plus==0){
      motor1.stop();
      motor2.stop();
      delay(1000);
        // readSensors(sensorValues);
        // line_follow(sensorValues);

        // if (junction=="TT"){
        //   motor1.stop();
        //   motor2.stop();
        //   delay(6000);
        // }
      turnLeft();
      forward_encoder(180);

      turnRight();
      forward_encoder(370);

      break;
    }
  }
  Task2();
}  




void Task2(){
  while(true){
  
  }
  Task3();
}

void Task3(){
  while(true){
    motor1.stop();
    motor2.stop();
    delay(1000);
  }
  Task4();
}

void Task4(){
  while(true){
    motor1.stop();
    motor2.stop();
    delay(1000);
  }
  Task5();
}


void Task5(){
  while(true){
    motor1.stop();
    motor2.stop();
    delay(1000);
  }
  Task6();
}

void Task6(){
  while(true){
    motor1.stop();
    motor2.stop();
    delay(1000);
  }

}
 
// int Green_White_Detect(){
//   uint16_t r, g, b, c;
  
//   tcs.getRawData(&r, &g, &b, &c);
//   //Serial.println(g);
//   if (35<g && g<230){ // then green
//     //digitalWrite(53,HIGH);
//     //digitalWrite(51,LOW);
//     //digitalWrite(13, HIGH);
//     return 1 ;
//   } 
//   else{// white
//     //digitalWrite(13, LOW);
//     return 0;
//   }
// }

// Function to read color frequencies from TCS3200
void readColors() {
  unsigned long redSum = 0, greenSum = 0, blueSum = 0;
  const int samples = 5;  // Average over 5 readings

  for (int i = 0; i < samples; i++) {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redSum += pulseIn(OUT, LOW, 10000);  // 10ms timeout

    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenSum += pulseIn(OUT, LOW, 10000);

    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    blueSum += pulseIn(OUT, LOW, 10000);

    delay(10);  // Small delay between samples
  }

  redFreq = redSum / samples;
  greenFreq = greenSum / samples;
  blueFreq = blueSum / samples;
}

String ball_colour(){
  
  readColors();
  if (redFreq>redMinYellow && blueFreq>blueMinYellow){
    //Serial.println("Yellow");
    return "Yellow";

  }
  else {
    //Serial.println("White");
    return "White";

  }
}

int Green_White_Detect(){
  encoderCountA=0;
  encoderCountB=0;
  while((encoderCountA < 30) && (encoderCountB < 30)){
    //delay(5000);
    readSensors(sensorValues);
    float pidValue = calculatePID(sensorValues);
    PID_Linefollow(pidValue);
    //moveStraightPID();
  }
  motor1.stop();
  motor2.stop();
  delay(1000);
  readColors();

  encoderCountA=0;
  encoderCountB=0;
  while((encoderCountA < 25) && (encoderCountB < 25)){
    //delay(5000);
    encoder_backward();
  }
  motor1.stop();
  motor2.stop();
  delay(1000);
  if (redFreq>10 && blueFreq>10){
    //Serial.println("Yellow");
    return 1;
  }
  else {
    //Serial.println("White");
    return 0;
  }

  
  
}

void move_for_grabbing(){
  encoderCountA=0;
  encoderCountB=0;
  while((encoderCountA < 65) && (encoderCountB < 65)){
    //delay(5000);
    readSensors(sensorValues);
    float pidValue = calculatePID(sensorValues);
    PID_Linefollow(pidValue);
  }
 
  motor1.stop();
  motor2.stop();

  smoothMoveServo(baseServo,basePos,10);
  delay(500);
  smoothMoveServo(shoulderServo,shoulderPos,50);
  delay(500);
  smoothMoveServo(elbowServo,elbowPos,55);
  delay(500);
  smoothMoveServo(shoulderServo,shoulderPos,75);
  delay(500);
  smoothMoveServo(elbowServo,elbowPos,50);
  delay(500);
  String colour_ = ball_colour();       
  smoothMoveServo(gripperServo,gripperPos,150);
  delay(500);
  //String colour_ = ball_colour();
  smoothMoveServo(elbowServo,elbowPos,180);
  delay(500);
  smoothMoveServo(shoulderServo,shoulderPos,0);
  delay(500);
  if (colour_=="Yellow"){   //(temp_yellow_white[tem_pos]=='Y'){
    smoothMoveServo(baseServo,basePos,110);
    digitalWrite(30, HIGH);
    delay(500); 
    digitalWrite(30, LOW);

  }else{
    smoothMoveServo(baseServo,basePos,150);
    digitalWrite(31, HIGH);
    delay(500);
    digitalWrite(31, LOW);
  }
  delay(500);
  smoothMoveServo(gripperServo,gripperPos,0);

  //grabing code
  delay(500);

  encoderCountA=0;
  encoderCountB=0;
  while((encoderCountA < 30) && (encoderCountB < 30)){
    encoder_backward();
  }
  motor1.stop();
  motor2.stop();
  delay(500);
  //half_rotation();
  

  ball_count++;

  //tem_pos++;        *************************
  
}


void smoothMoveServo(Servo &servo, int &currentPos, int targetPos) {
  // Constrain target to the servo's specific range
  targetPos = constrain(targetPos, 0, 180);

  // Calculate the direction and total steps
  int delta = targetPos - currentPos;
  int steps = abs(delta) / 1;
  int direction = (delta > 0) ? 1 : -1;

  // Move incrementally toward the target
  for (int i = 0; i <= steps; i++) {
    currentPos += direction * 1;
    if (direction > 0 && currentPos > targetPos) currentPos = targetPos;
    if (direction < 0 && currentPos < targetPos) currentPos = targetPos;
    
    servo.write(currentPos);
    delay(15);
  }

  // Ensure final position is exact
  currentPos = targetPos;
  servo.write(currentPos);
}

// void colomcheck(){
//   line_follow_juction_turns();
//   if(is_plus ==true && plus == 0 && green_detect == 1  ){ //green_temp_array[tem_pos]==0) { 
//     //turnRight45();
//     turnRight();
//     //move_for_grabbing();
//     ball_count++;
//     delay(2000);
//     turnLeft();
//     //turnLeft45();
//     plus = 0;
//     is_plus =false;
//   }
//   else if (is_plus ==true  && plus == 0 && green_detect == 0) {//green_temp_array[tem_pos]!=0){ 
//     delay(500);
//     turnRight();
//     plus+=1;
//     is_plus =false;
    
//   }
//   else if (is_plus ==true  && plus == 1 && green_detect == 1){  //green_temp_array[tem_pos]==1){ 
//     //turnLeft45();
//     //move_for_grabbing();
//     ball_count++;
//     delay(2000);
//     half_rotation_L();
//     plus = 2;
//     is_plus =false;     
//   }
//   else if (is_plus ==true  && plus == 1 && green_detect == 0){ // green_temp_array[tem_pos]!=1){ 
//     //turnRight();
//     delay(500);
//     plus=2;
//     is_plus =false;
//     //check_again = true; 
//     //half_rotation(); 
//     turnLeft();
//     delay(2000);
//     //move_for_grabbing();
//     ball_count++;
//     turnLeft(); 
//   }
//   // else if (is_plus ==true  && plus == 2 && green_detect == 1){ // green_temp_array[tem_pos]==2){ 
//   //   //turnLeft45();
//   //   //move_for_grabbing();

//   //   half_rotation_L();
//   //   plus = 3;
//   //   is_plus =false;     
//   // }
//   // else if (is_plus ==true  && plus == 2 && green_detect == 0){ //green_temp_array[tem_pos]!=2){
//   //   delay(500);
//   //   half_rotation();
//   //   check_again = true;
//   //   plus = 3;
//   //   is_plus =false;     
//   // }
//   // else if (is_plus ==true && plus == 2){
//   //   delay(500);
//   //   plus=3;
//   //   is_plus =false;    
//   // }
//   else if (is_plus ==true && plus==2){
//     delay(500);
//     turnRight();
    
//     // if (check_again == true){
//     //   small_backward();
//     //   check_again = false;
//     //   half_rotation();
//     // }
//     plus=0;
//     is_plus=false;
//   }
  

// }


void colomcheck(){
  line_follow_juction_turns();
  if(is_plus ==true && plus == 0 ){ //green_temp_array[tem_pos]==0) { 
    //delay(500);
    green_detect= Green_White_Detect();
    if (green_detect == 1){
      //turnRight45();
      turnRight();
      //move_for_grabbing();
      ball_count++;   //change
      delay(2000);

      turnLeft();
      //turnLeft45();
      plus = 0;
    }
    else {
      delay(500);
      turnRight();
      plus+=1;
    }
    is_plus =false;

  }
  if (is_plus ==true  && plus == 1){  //green_temp_array[tem_pos]==1){ 
    turnLeft();
    green_detect= Green_White_Detect();
    if (green_detect == 1){
      //turnRight45();
      turnRight();
      //move_for_grabbing();
      ball_count++;
      delay(2000);
    
      //turnLeft();
      half_rotation_L();
      plus = 4;
    }
    else{
      delay(500);
      //turnRight();
      turnRight();
      plus+=1;
    }
    is_plus =false;
    
  }
  if (is_plus ==true  && plus == 2){  //green_temp_array[tem_pos]==1){ 
    motor1.stop();
    motor2.stop();
    delay(500);
    turnLeft();
    green_detect= Green_White_Detect();
    if (green_detect == 1){
      turnRight();
      //turnLeft45();
      //move_for_grabbing();
      ball_count++;
      delay(2000);

      half_rotation_L();
      plus = 3;   
    }
    else{
      delay(500);
      turnRight();
      half_rotation();
      check_again = true;
      plus = 3;   
    }
    delay(500);
    is_plus =false;

     
  }
  if (is_plus ==true && plus == 3){
    delay(500);
    plus=4;
    is_plus =false;    
  }
  if (is_plus ==true && plus==4){
    delay(500);
    turnRight();
    
    if (check_again == true){
      backward_encoder(150);
      check_again = false;
    }
    plus=0;
    is_plus=false;
  }
  
}

float measureDistance(int trigPin, int echoPin) {
  long duration;
  float distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // Timeout after 20ms
  distance = duration * 0.034 / 2.0;
  //Serial.println(distance);
  /*if (distanceCm == 0 || distanceCm > 100) {
    distanceCm = DESIRED_DISTANCE;  // Assume default if out of range
  }
  Serial.println(distanceCm);*/
  return distance;
}

void wall_follow(float pidValue){
  lsp_w=lfspeed_w-pidValue;
  rsp_w=lfspeed_w+pidValue;

  lsp_w=constrain(lsp_w,-200,200);
  rsp_w=constrain(rsp_w,-200,200);

  motor_drive(lsp_w,rsp_w);

}

float calculatePID_wall() {
  float distance = measureDistance(L_trig,L_echo);
  distance=constrain(distance, 3, 20);
  Serial.println(distance);
  error_w = DESIRED_DISTANCE - distance;
  //Serial.println(error);

  for (int i = 2; i > 0; i--) {
    errorArray_w[i] = errorArray_w[i - 1];
  }
  errorArray_w[0] = error_w;

  P_w = error_w;
  I_w += error_w;
  D_w = error_w - previousError_w;
  previousError_w= error_w;

  float pidValue = (Kp_w * P_w) + (Ki_w * I_w) + (Kd_w * D_w);
  return pidValue;
}

void colomcheck2(){
  line_follow_juction_turns();
  if(is_plus ==true && plus == 0 && green_detect == 1  ){ //green_temp_array[tem_pos]==0) { 
    //turnRight45();
    //turnRight();
    //move_for_grabbing();
    //turnLeft();
    //turnLeft45();
    plus = 0;
    is_plus =false;
  }
  else if (is_plus ==true  && plus == 0 && green_detect == 0) {//green_temp_array[tem_pos]!=0){ 
    delay(500);
    //turnRight();
    plus+=1;
    is_plus =false;
    
  }
  else if (is_plus ==true  && plus == 1 && green_detect == 1){  //green_temp_array[tem_pos]==1){ 
    //turnLeft45();
    //move_for_grabbing();
    
    half_rotation_L();
    plus = 4;
    is_plus =false;     
  }
  else if (is_plus ==true  && plus == 1 && green_detect == 0){ // green_temp_array[tem_pos]!=1){ 
    //turnRight();
    delay(500);
    plus+=1;
    is_plus =false;    
  }
  else if (is_plus ==true  && plus == 2 && green_detect == 1){ // green_temp_array[tem_pos]==2){ 
    //turnLeft45();
    //move_for_grabbing();

    half_rotation_L();
    plus = 3;
    is_plus =false;     
  }
  else if (is_plus ==true  && plus == 2 && green_detect == 0){ //green_temp_array[tem_pos]!=2){
    delay(500);
    half_rotation();
    check_again = true;
    plus = 3;
    is_plus =false;     
  }
  else if (is_plus ==true && plus == 3){
    delay(500);
    plus=4;
    is_plus =false;    
  }
  else if (is_plus ==true && plus==4){
    delay(500);
    turnRight();
    
    if (check_again == true){
      backward_encoder(150);
      check_again = false;
    }
    plus=0;
    is_plus=false;
  }
  

}