#include <Arduino.h>

// Pins utilitzats
const int LED_PIN = 2;
const int PIN_SUBIR = 47;
const int PIN_BAJAR = 48;

// Període inicial del parpelleig en microsegons
volatile uint32_t periodoActual = 500000;

// Flags modificats per les interrupcions
volatile bool flagTimer = false;
volatile bool flagSubir = false;
volatile bool flagBajar = false;

// Variables del timer
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// ISR del timer
void IRAM_ATTR onTimer() {
  flagTimer = true;
}

// ISR del pulsador per augmentar freqüència
void IRAM_ATTR isrSubir() {
  flagSubir = true;
}

// ISR del pulsador per disminuir freqüència
void IRAM_ATTR isrBajar() {
  flagBajar = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIN_SUBIR, INPUT_PULLUP);
  pinMode(PIN_BAJAR, INPUT_PULLUP);

  attachInterrupt(PIN_SUBIR, isrSubir, FALLING);
  attachInterrupt(PIN_BAJAR, isrBajar, FALLING);

  // Configuració del timer
  // 80 MHz / 80 = 1 tick cada microsegon
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, periodoActual, true);
  timerAlarmEnable(timer);

  Serial.println("Programa iniciat");
  Serial.println("LED amb frequencia variable mitjancant dos pulsadors");
}

void loop() {
  // Canvi d'estat del LED quan el timer genera una interrupció
  if (flagTimer) {
    flagTimer = false;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  // Debounce dels pulsadors
  static uint32_t lastDebounceTime = 0;
  uint32_t currentTime = millis();

  if (currentTime - lastDebounceTime > 200) {
    if (flagSubir || flagBajar) {

      portENTER_CRITICAL(&timerMux);

      if (flagSubir && periodoActual > 100000) {
        periodoActual -= 100000;
      }

      if (flagBajar && periodoActual < 2000000) {
        periodoActual += 100000;
      }

      timerAlarmWrite(timer, periodoActual, true);

      flagSubir = false;
      flagBajar = false;

      portEXIT_CRITICAL(&timerMux);

      Serial.print("Nou periode del LED: ");
      Serial.print(periodoActual);
      Serial.println(" us");

      lastDebounceTime = currentTime;
    }
  }
}