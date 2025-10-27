#include <Servo.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define SERVO_PIN 9
#define IR_PIN 2
#define BUTTON_PIN 3

#define POS_A 1000
#define POS_B 1900
#define MOVE_TIME_MS 500

#define REFRACT_IR 150
#define REFRACT_BTN 150
#define IDLE_STABLE_MS 150
#define IDLE_TIMEOUT_MS 2000

Servo servo1;

volatile bool wake = false;
volatile uint8_t why = 0;
bool posState = false;
unsigned long lastAction = 0;

void irISR() { why = 1; wake = true; }
void btnISR() { why = 2; wake = true; }

void waitIdle(unsigned int stableMs = IDLE_STABLE_MS, unsigned int timeoutMs = IDLE_TIMEOUT_MS) {
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (digitalRead(IR_PIN) == HIGH && digitalRead(BUTTON_PIN) == HIGH) {
      delay(stableMs);
      if (digitalRead(IR_PIN) == HIGH && digitalRead(BUTTON_PIN) == HIGH) return;
    }
    delay(5);
  }
}

void sleepNow() {
  wake = false;
  EIFR = _BV(INTF0) | _BV(INTF1);
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(IR_PIN), irISR, LOW);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), btnISR, LOW);
  sleep_enable();
  interrupts();

  if (digitalRead(IR_PIN) == LOW || digitalRead(BUTTON_PIN) == LOW) {
    sleep_disable();
    noInterrupts();
    detachInterrupt(digitalPinToInterrupt(IR_PIN));
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
    interrupts();
    why = (digitalRead(IR_PIN) == LOW) ? 1 : 2;
    wake = true;
    return;
  }

  sleep_cpu();
  sleep_disable();
  noInterrupts();
  detachInterrupt(digitalPinToInterrupt(IR_PIN));
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
  interrupts();

  if (!wake) {
    why = (digitalRead(IR_PIN) == LOW) ? 1 : 2;
    wake = true;
  }
}

void setup() {
  pinMode(IR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  servo1.detach();
  delay(300);
  waitIdle();
}

void loop() {
  sleepNow();
  if (!wake) return;

  noInterrupts();
  uint8_t c = why;
  why = 0;
  wake = false;
  interrupts();

  unsigned long now = millis();
  unsigned int ref = (c == 1) ? REFRACT_IR : REFRACT_BTN;
  if (lastAction != 0 && now - lastAction < ref) {
    waitIdle();
    return;
  }

  if (c == 1) delay(120); else delay(40);

  servo1.attach(SERVO_PIN);
  servo1.writeMicroseconds(posState ? POS_A : POS_B);
  delay(MOVE_TIME_MS);
  servo1.detach();

  posState = !posState;
  lastAction = millis();

  waitIdle();
}
