#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// OLED configuration
#define SDA_PIN 6
#define SCL_PIN 7

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

// Emotions
enum Emotion {
  HAPPY,
  SAD,
  ANGRY,
  SURPRISED,
  SLEEPY,
  NEUTRAL
};

Emotion currentEmotion = NEUTRAL;

// Animation variables
float lookX = 0;
float lookY = 0;

int targetLookX = 0;
int targetLookY = 0;

bool blinking = false;
bool talking = false;
bool mouthOpen = false;

unsigned long blinkStart = 0;
unsigned long nextBlink = 0;
unsigned long nextLook = 0;
unsigned long lastTalk = 0;
unsigned long lastFrame = 0;

const int BLINK_DURATION = 180;

// --------------------------------------------------
// DRAW EYES
// --------------------------------------------------

void drawEye(int cx, int cy, bool closed) {

  if (closed || currentEmotion == SLEEPY) {
    u8g2.drawLine(cx - 10, cy, cx + 10, cy);

    if (currentEmotion == SLEEPY) {
      u8g2.drawLine(cx - 8, cy + 3, cx + 8, cy + 3);
    }

    return;
  }

  if (currentEmotion == HAPPY) {
    u8g2.drawLine(cx - 10, cy, cx - 5, cy + 4);
    u8g2.drawLine(cx - 5, cy + 4, cx, cy + 5);
    u8g2.drawLine(cx, cy + 5, cx + 5, cy + 4);
    u8g2.drawLine(cx + 5, cy + 4, cx + 10, cy);
    return;
  }

  if (currentEmotion == SURPRISED) {
    u8g2.drawEllipse(cx, cy, 10, 13, U8G2_DRAW_ALL);
    u8g2.drawDisc(
      cx + (int)lookX,
      cy + (int)lookY,
      3,
      U8G2_DRAW_ALL
    );
    return;
  }

  // Normal eyes
  u8g2.drawRFrame(cx - 11, cy - 11, 22, 23, 7);

  u8g2.drawDisc(
    cx + (int)lookX,
    cy + (int)lookY,
    4,
    U8G2_DRAW_ALL
  );
}

// --------------------------------------------------
// DRAW EYEBROWS
// --------------------------------------------------

void drawEyebrows() {

  if (currentEmotion == ANGRY) {

    u8g2.drawLine(25, 10, 48, 17);
    u8g2.drawLine(80, 17, 103, 10);

  }

  else if (currentEmotion == SAD) {

    u8g2.drawLine(25, 15, 48, 10);
    u8g2.drawLine(80, 10, 103, 15);

  }

  else if (currentEmotion == SURPRISED) {

    u8g2.drawLine(27, 8, 48, 7);
    u8g2.drawLine(80, 7, 101, 8);

  }
}

// --------------------------------------------------
// DRAW MOUTH
// --------------------------------------------------

void drawMouth() {

  if (talking && mouthOpen) {

    u8g2.drawEllipse(64, 49, 5, 7, U8G2_DRAW_ALL);
    return;

  }

  if (talking && !mouthOpen) {

    u8g2.drawLine(57, 49, 71, 49);
    return;

  }

  switch (currentEmotion) {

    case HAPPY:

      u8g2.drawLine(51, 44, 57, 50);
      u8g2.drawLine(57, 50, 64, 53);
      u8g2.drawLine(64, 53, 71, 50);
      u8g2.drawLine(71, 50, 77, 44);

      break;

    case SAD:

      u8g2.drawLine(51, 51, 57, 46);
      u8g2.drawLine(57, 46, 64, 44);
      u8g2.drawLine(64, 44, 71, 46);
      u8g2.drawLine(71, 46, 77, 51);

      break;

    case ANGRY:

      u8g2.drawLine(53, 48, 75, 48);
      u8g2.drawLine(53, 48, 57, 51);
      u8g2.drawLine(75, 48, 71, 51);

      break;

    case SURPRISED:

      u8g2.drawEllipse(64, 49, 5, 7, U8G2_DRAW_ALL);

      break;

    case SLEEPY:

      u8g2.drawLine(58, 49, 70, 49);

      break;

    case NEUTRAL:

      u8g2.drawLine(55, 49, 73, 49);

      break;
  }
}

// --------------------------------------------------
// DRAW COMPLETE FACE
// --------------------------------------------------

void drawFace() {

  u8g2.clearBuffer();

  bool eyeClosed = blinking;

  drawEyebrows();

  drawEye(38, 29, eyeClosed);
  drawEye(90, 29, eyeClosed);

  drawMouth();

  u8g2.sendBuffer();
}

// --------------------------------------------------
// UPDATE BLINKING
// --------------------------------------------------

void updateBlink(unsigned long now) {

  if (!blinking && now >= nextBlink) {

    blinking = true;
    blinkStart = now;

  }

  if (blinking && now - blinkStart >= BLINK_DURATION) {

    blinking = false;
    nextBlink = now + random(2000, 4500);

  }
}

// --------------------------------------------------
// UPDATE EYE MOVEMENT
// --------------------------------------------------

void updateEyeMovement(unsigned long now) {

  if (now >= nextLook) {

    targetLookX = random(-4, 5);
    targetLookY = random(-2, 3);

    nextLook = now + random(1500, 3000);

  }

  lookX += (targetLookX - lookX) * 0.12;
  lookY += (targetLookY - lookY) * 0.12;
}

// --------------------------------------------------
// UPDATE TALKING ANIMATION
// --------------------------------------------------

void updateTalking(unsigned long now) {

  if (talking && now - lastTalk >= 170) {

    mouthOpen = !mouthOpen;
    lastTalk = now;

  }
}

// --------------------------------------------------
// CHANGE EMOTION
// --------------------------------------------------

void setEmotion(Emotion emotion) {

  currentEmotion = emotion;

  Serial.print("Emotion changed: ");

  switch (emotion) {

    case HAPPY:
      Serial.println("Happy");
      break;

    case SAD:
      Serial.println("Sad");
      break;

    case ANGRY:
      Serial.println("Angry");
      break;

    case SURPRISED:
      Serial.println("Surprised");
      break;

    case SLEEPY:
      Serial.println("Sleepy");
      break;

    case NEUTRAL:
      Serial.println("Neutral");
      break;
  }
}

// --------------------------------------------------
// SERIAL COMMANDS
// --------------------------------------------------

void handleSerial() {

  while (Serial.available()) {

    char command = Serial.read();

    switch (command) {

      case '1':
        setEmotion(HAPPY);
        break;

      case '2':
        setEmotion(SAD);
        break;

      case '3':
        setEmotion(ANGRY);
        break;

      case '4':
        setEmotion(SURPRISED);
        break;

      case '5':
        setEmotion(SLEEPY);
        break;

      case '6':
        setEmotion(NEUTRAL);
        break;

      case 'T':
      case 't':

        talking = !talking;
        mouthOpen = false;

        Serial.println(
          talking ? "Talking ON" : "Talking OFF"
        );

        break;

      case 'C':
      case 'c':

        setEmotion(
          (Emotion)((currentEmotion + 1) % 6)
        );

        break;

      case '?':

        Serial.println("\n--- ROBOT FACE CONTROLS ---");
        Serial.println("1 = Happy");
        Serial.println("2 = Sad");
        Serial.println("3 = Angry");
        Serial.println("4 = Surprised");
        Serial.println("5 = Sleepy");
        Serial.println("6 = Neutral");
        Serial.println("T = Toggle Talking");
        Serial.println("C = Cycle Emotions");
        Serial.println("? = Help");
        Serial.println("---------------------------");

        break;
    }
  }
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  u8g2.begin();

  randomSeed(esp_random());

  nextBlink = millis() + 1500;
  nextLook = millis() + 1000;

  setEmotion(NEUTRAL);

  Serial.println("\nESP32-C3 ROBOTIC FACE STARTED!");
  Serial.println("Open Serial Monitor at 115200 baud.");
  Serial.println("Type ? to see all commands.");

}

// --------------------------------------------------
// MAIN LOOP
// --------------------------------------------------

void loop() {

  unsigned long now = millis();

  handleSerial();

  updateBlink(now);

  updateEyeMovement(now);

  updateTalking(now);

  // Refresh display approximately every 25 ms
  if (now - lastFrame >= 25) {

    lastFrame = now;

    drawFace();

  }

}
