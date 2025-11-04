#include <Servo.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define PIN_SRV 9
#define PIN_IR 2
#define PIN_BTN 3

#define P_A 1000
#define P_B 1900
#define MOVE_MS 500

#define REF_IR 150
#define REF_BTN 150
#define IDLE_MS 150
#define TIMEOUT_MS 2000

Servo srv;

volatile bool woke = false;
volatile int why = 0;
int currPulse = P_A;
float lastMs = 0;

void onIR() {
  why = 1;
  woke = true;
  detachInterrupt(digitalPinToInterrupt(PIN_IR));
  detachInterrupt(digitalPinToInterrupt(PIN_BTN));
}

void onBTN() {
  why = 2;
  woke = true;
  detachInterrupt(digitalPinToInterrupt(PIN_IR));
  detachInterrupt(digitalPinToInterrupt(PIN_BTN));
}

void waitIdle(int stable = IDLE_MS, int limit = TIMEOUT_MS) {
  float t0 = (float)millis();
  while (((float)millis() - t0) < (float)limit) {
    if (digitalRead(PIN_IR) == HIGH && digitalRead(PIN_BTN) == HIGH) {
      delay(stable);
      if (digitalRead(PIN_IR) == HIGH && digitalRead(PIN_BTN) == HIGH) return;
    }
    delay(5);
  }
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  EIFR = _BV(INTF0) | _BV(INTF1);

  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(PIN_IR), onIR, LOW);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), onBTN, LOW);
  sleep_enable();
  interrupts();

  if (digitalRead(PIN_IR) == LOW || digitalRead(PIN_BTN) == LOW) {
    sleep_disable();
    noInterrupts();
    detachInterrupt(digitalPinToInterrupt(PIN_IR));
    detachInterrupt(digitalPinToInterrupt(PIN_BTN));
    interrupts();
    why = (digitalRead(PIN_IR) == LOW) ? 1 : 2;
    woke = true;
    return;
  }

  float t0 = (float)millis();
  while (((float)millis() - t0) < 80.0f) {
    if (digitalRead(PIN_IR) == LOW || digitalRead(PIN_BTN) == LOW) {
      sleep_disable();
      noInterrupts();
      detachInterrupt(digitalPinToInterrupt(PIN_IR));
      detachInterrupt(digitalPinToInterrupt(PIN_BTN));
      interrupts();
      why = (digitalRead(PIN_IR) == LOW) ? 1 : 2;
      woke = true;
      return;
    }
    delay(1);
  }

  sleep_cpu();
  sleep_disable();
}

void setup() {
  pinMode(PIN_IR, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  srv.detach();
  delay(300);
  waitIdle();
}

void loop() {
  sleepNow();
  if (!woke) return;

  noInterrupts();
  int cause = why;
  why = 0;
  woke = false;
  interrupts();

  float now = (float)millis();
  int refms = (cause == 1) ? REF_IR : REF_BTN;
  if (lastMs > 0 && (now - lastMs) < (float)refms) {
    waitIdle();
    return;
  }

  if (cause == 1) delay(120); else delay(40);

  srv.attach(PIN_SRV);
  if (currPulse == P_A) {
    srv.writeMicroseconds(P_B);
    currPulse = P_B;
  } else {
    srv.writeMicroseconds(P_A);
    currPulse = P_A;
  }
  delay(MOVE_MS);
  srv.detach();

  lastMs = (float)millis();
  waitIdle();
}
