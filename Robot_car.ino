// ==================================================
// FINAL: 6 IR + FIXED TURN LOGIC + SMOOTH PID + OBSTACLE
// + 45 DEGREE TURN SENSOR IGNORE FIX
// ==================================================

// -------- IR Sensors --------
#define S1  34
#define S1A 16
#define S2  35
#define S3  32
#define S2A 17
#define S4  23

// -------- Obstacle Sensor --------
#define OBS 18

// -------- Motor Pins --------
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14
#define ENA 33
#define ENB 19

int freq = 5000;
int resolution = 8;

// -------- SPEED --------
int baseSpeed = 150;
int minSpeed  = 120;
int maxSpeed  = 255;

// -------- TURN SPEED --------
int turnSpeedStrong = 180;
int obstacleTurnSpeed = 190;

// -------- PID --------
float Kp = 11.0;
float Kd = 3.2;

float lastError = 0;
float lastCorrection = 0;

// -------- SMOOTH --------
int currentL = 0;
int currentR = 0;
int rampStep = 6;

// -------- TURN FIX --------
const int turnIgnoreTime = 200;   // ignore old line for 200ms

// ==================================================
void setup() {

  pinMode(S1, INPUT);
  pinMode(S1A, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S2A, INPUT);
  pinMode(S4, INPUT);

  pinMode(OBS, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(0, freq, resolution);
  ledcAttachPin(ENA, 0);

  ledcSetup(1, freq, resolution);
  ledcAttachPin(ENB, 1);
}

// ==================================================
void applyMotor(int targetL, int targetR) {

  if (currentL < targetL) currentL += rampStep;
  else if (currentL > targetL) currentL -= rampStep;

  if (currentR < targetR) currentR += rampStep;
  else if (currentR > targetR) currentR -= rampStep;

  currentL = constrain(currentL, 0, maxSpeed);
  currentR = constrain(currentR, 0, maxSpeed);

  ledcWrite(0, currentL);
  ledcWrite(1, currentR);
}

// ==================================================
void move(int left, int right) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  if (left > 0 && left < minSpeed) left = minSpeed;
  if (right > 0 && right < minSpeed) right = minSpeed;

  left = constrain(left, 0, maxSpeed);
  right = constrain(right, 0, maxSpeed);

  applyMotor(left, right);
}

// ==================================================
void moveBackward(int l, int r) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  l = constrain(l, 0, maxSpeed);
  r = constrain(r, 0, maxSpeed);

  applyMotor(l, r);
}

// ==================================================
void turnLeftStrong() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  applyMotor(turnSpeedStrong, turnSpeedStrong);
}

// ==================================================
void turnRightStrong() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  applyMotor(turnSpeedStrong, turnSpeedStrong);
}

// ==================================================
bool obstacleDetected() {

  int c = 0;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(OBS) == 0) c++;
    delay(5);
  }
  return (c >= 4);
}

// ==================================================
void findLine() {

  unsigned long start = millis();

  while (!(digitalRead(S2)==0 && digitalRead(S3)==0)) {

    if (digitalRead(S2A)==0 || digitalRead(S4)==0) {
      move(120, 85);
    }
    else if (digitalRead(S1A)==0 || digitalRead(S1)==0) {
      move(85, 120);
    }
    else {
      move(105,105);
    }

    if (millis() - start > 2000) break;
  }

  move(120,120);
  delay(80);
}

// ==================================================
void avoidObstacle() {

  move(0,0);
  delay(80);

  // longer reverse
  for (int i=0; i<8; i++) {
    moveBackward(125 + i*5, 125 + i*5);
    delay(80);
  }

  move(0,0);
  delay(100);

  // BIG turn away
  turnRightStrong();
  delay(450);

  // wide arc
  for (int i=0; i<10; i++) {
    move(150, 75);
    delay(85);
  }

  // bypass
  move(130,130);
  delay(1000);

  // return
  turnLeftStrong();
  delay(520);

  findLine();
}

// ==================================================
void loop() {

  if (obstacleDetected()) {
    avoidObstacle();
    return;
  }

  int s1  = digitalRead(S1);
  int s1a = digitalRead(S1A);
  int s2  = digitalRead(S2);
  int s3  = digitalRead(S3);
  int s2a = digitalRead(S2A);
  int s4  = digitalRead(S4);

  // ==================================================
  // 45° LEFT TURN FIX
  // ==================================================
  if ((s4==0 && s2a==0 && s3==0 && s2==0 && s1a==1 && s1==1) ||
      (s4==0 && s2a==0 && s3==0 && s2==0 && s1a==0 && s1==1)) {

    move(160,160);
    delay(40);

    // Ignore old line for 200ms
    unsigned long ignoreStart = millis();
    while (millis() - ignoreStart < turnIgnoreTime) {
      turnLeftStrong();
    }

    // After ignore time, now search for NEW line
    unsigned long t = millis();
    while (!(digitalRead(S2)==0 && digitalRead(S3)==0)) {
      turnLeftStrong();
      if (millis()-t > 900) break;
    }

    move(125,125);
    delay(80);
    return;
  }

  // ==================================================
  // 45° RIGHT TURN FIX
  // ==================================================
  if ((s4==1 && s2a==1 && s3==0 && s2==0 && s1a==0 && s1==0) ||
      (s4==1 && s2a==0 && s3==0 && s2==0 && s1a==0 && s1==0)) {

    move(160,160);
    delay(40);

    // Ignore old line for 200ms
    unsigned long ignoreStart = millis();
    while (millis() - ignoreStart < turnIgnoreTime) {
      turnRightStrong();
    }

    // After ignore time, now search for NEW line
    unsigned long t = millis();
    while (!(digitalRead(S2)==0 && digitalRead(S3)==0)) {
      turnRightStrong();
      if (millis()-t > 900) break;
    }

    move(125,125);
    delay(80);
    return;
  }

  // ==================================================
  // LOST LINE
  // ==================================================
  if (s1==1 && s1a==1 && s2==1 && s3==1 && s2a==1 && s4==1) {

    if (lastError > 0) turnRightStrong();
    else turnLeftStrong();

    return;
  }

  // ==================================================
  // PID (CENTER STABLE)
  // ==================================================
  int sum = 0;
  int count = 0;

  if (s1  == 0) { sum += -6; count++; }
  if (s1a == 0) { sum += -3; count++; }
  if (s2  == 0) { sum += -1; count++; }
  if (s3  == 0) { sum +=  1; count++; }
  if (s2a == 0) { sum +=  3; count++; }
  if (s4  == 0) { sum +=  6; count++; }

  float error = (count > 0) ? (float)sum / count : lastError;

  float derivative = error - lastError;
  float raw = (Kp * error) + (Kd * derivative);

  float correction = (0.84 * lastCorrection) + (0.16 * raw);

  lastError = error;
  lastCorrection = correction;

  int leftSpeed  = baseSpeed + correction;
  int rightSpeed = baseSpeed - correction;

  move(leftSpeed, rightSpeed);
}