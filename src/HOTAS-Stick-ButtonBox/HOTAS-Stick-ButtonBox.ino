#include <Joystick.h>
#include <HID_Buttons.h>

const int KEYFREE = 0;
const int KEYDOWN = 1;
const int KEYUP   = 2;
const int KEYHOLD = 3;

const int ROW_COUNT = 8;
const int COLUMN_COUNT = 2;

int rowPins [] = {21, 19, 15, 16, 20, 18, 14, 10};
int columnPins [] = {6, 7};
int keyStatus [ ROW_COUNT ][ COLUMN_COUNT ] = { };

Joystick_ Joystick(0x07, JOYSTICK_TYPE_JOYSTICK ,
                   20, // Button Count
                   2,                        // Hat Switch Count
                   true, true, false,        // No X, Y, or Z axes
                   true, true, false,        // No Rx, Ry, or Rz
                   true, true,             // No rudder or throttle
                   true, true, false);     // No accelerator, brake, or steering

int h0x = 0;
int h0y = 0;
int h1x = 0;
int h1y = 0;

//Mux control pins
int s0 = 0;
int s1 = 1;
int s2 = 2;
int s3 = 3;

//Mux in "SIG" pin
int SIG_pin = 9;

void setup() {
  Serial.begin(9600);
  
  pinMode(SIG_pin, INPUT);

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  Joystick.begin();
  Joystick.setXAxisRange(0, 1024);
  Joystick.setYAxisRange(0, 1024);
  Joystick.setRxAxisRange(0, 1024);
  Joystick.setRyAxisRange(0, 1024);
  Joystick.setRudderRange(0, 1024);
  Joystick.setThrottleRange(0, 1024);
  Joystick.setAcceleratorRange(0, 1024);
  Joystick.setBrakeRange(0, 1024);

  // Set all key status in matrix as FREE or not pressed
  for (int i = 0; i < ROW_COUNT; i++) {
    for (int j = 0; j < COLUMN_COUNT; j++) {
      keyStatus [ i ][ j ] = KEYFREE;
    }
  }
  // Configure pin column as an input and enable the internal pull-up resistor
  for (int i = 0; i < COLUMN_COUNT; i++) {
    pinMode(columnPins [i], INPUT_PULLUP);
  }
  // Configure all row pins as input
  for (int i = 0; i < ROW_COUNT; i++) {
    pinMode(rowPins [i], INPUT);
  }
}

void loop() {
  Joystick.setRudder(readMux(0));
  Joystick.setThrottle(readMux(1));
  Joystick.setAccelerator(readMux(2));
  Joystick.setBrake(readMux(3));

  Joystick.setXAxis(readMux(5));
  Joystick.setYAxis(readMux(4));

  h0x = readMux(6);
  h0y = readMux(7);
  if (h0x > 750) {
    Joystick.setHatSwitch(0, 180);
  } else if (h0x < 250) {
    Joystick.setHatSwitch(0, 0);
  } else if (h0y > 750) {
    Joystick.setHatSwitch(0, 90);
  } else if (h0y < 250) {
    Joystick.setHatSwitch(0, 270);
  } else {
    Joystick.setHatSwitch(0, JOYSTICK_HATSWITCH_RELEASE);
  }

  h1x = readMux(8);
  h1y = readMux(9);
  if (h1x > 750) {
    Joystick.setHatSwitch(1, 180);
  } else if (h1x < 250) {
    Joystick.setHatSwitch(1, 0);
  } else if (h1y > 750) {
    Joystick.setHatSwitch(1, 90);
  } else if (h1y < 250) {
    Joystick.setHatSwitch(1, 270);
  } else {
    Joystick.setHatSwitch(1, JOYSTICK_HATSWITCH_RELEASE);
  }

  Joystick.setRxAxis(readMux(11));
  Joystick.setRyAxis(readMux(10));

  Joystick.setButton(0, readMux(12) > 750);
  Joystick.setButton(1, readMux(13) > 750);
  Joystick.setButton(2, readMux(14) > 750);
  Joystick.setButton(3, readMux(15) > 750);

  int currentButton = 4;
  for (int i = 0; i < ROW_COUNT; i++) {
    pinMode(rowPins [i], OUTPUT);
    digitalWrite(rowPins [i], LOW);
    for (int j = 0; j < COLUMN_COUNT; j++) {
      int sensorVal = digitalRead(columnPins [j]);
      if (sensorVal == LOW && keyStatus[i][j] == KEYFREE) {
//        Serial.print((String)"Button " + currentButton + " :");
//        Serial.println("KEYDOWN");
        keyStatus[i][j] = KEYDOWN;
        Joystick.setButton(currentButton, true);
      } else if (sensorVal == LOW && keyStatus[i][j] == KEYDOWN) {
//        Serial.print((String)"Button " + currentButton + " :");
//        Serial.println("KEYHOLD");
        keyStatus[i][j] = KEYHOLD;
        Joystick.setButton(currentButton, true);
      } else if (sensorVal == HIGH && (keyStatus[i][j] == KEYDOWN || keyStatus[i][j] == KEYHOLD)) {
//        Serial.print((String)"Button " + currentButton + " :");
//        Serial.println("KEYUP");
        keyStatus[i][j] = KEYUP;
        Joystick.setButton(currentButton, false);
      } else if (sensorVal == HIGH && keyStatus[i][j] == KEYUP) {
//        Serial.print((String)"Button " + currentButton + " :");
//        Serial.println("KEYFREE");
        keyStatus[i][j] = KEYFREE;
        Joystick.setButton(currentButton, false);
      }
      currentButton++;
    }
    pinMode(rowPins [i], INPUT);
  }
  delay(100);
}

int readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
  delay(10);

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}
