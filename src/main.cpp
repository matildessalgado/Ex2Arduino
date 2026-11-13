#include "Arduino.h"

#define NUM_LEDS 30
#define PIN_SOM A0

// ---------- FITA (pino 6, Arduino Uno a 16 MHz) ----------
static inline __attribute__((always_inline)) void enviarByte(uint8_t b) {
  uint8_t n = 8;
  asm volatile(
    "1:\n"
    "sbi %[port], %[bit]\n"
    "nop\nnop\nnop\n"
    "sbrs %[b], 7\n"
    "cbi %[port], %[bit]\n"
    "nop\nnop\nnop\nnop\nnop\nnop\n"
    "cbi %[port], %[bit]\n"
    "nop\n"
    "lsl %[b]\n"
    "dec %[n]\n"
    "brne 1b\n"
    : [b] "+r" (b), [n] "+r" (n)
    : [port] "I" (_SFR_IO_ADDR(PORTD)), [bit] "I" (6)
  );
}

// Acende "acesos" LEDs (a contar do início) com a cor r, g, b
void desenhar(int acesos, int r, int g, int b) {
  noInterrupts();
  for (int i = 0; i != NUM_LEDS; i++) {
    if (acesos > i) {
      enviarByte(g); enviarByte(r); enviarByte(b);
    } else {
      enviarByte(0); enviarByte(0); enviarByte(0);
    }
  }
  interrupts();
  delay(1);
}

// ---------- SOM ----------
// Diferença entre o valor mais alto e o mais baixo do som (cerca de 17 ms)
int lerNivel() {
  int minimo = 1023;
  int maximo = 0;
  for (int k = 0; k != 150; k++) {
    int v = analogRead(PIN_SOM);
    if (minimo > v) minimo = v;
    if (v > maximo) maximo = v;
  }
  return maximo - minimo;
}

// ---------- VARIÁVEIS ----------
int media = 0;                 // média recente do som
int brilho = 0;                // de 0 a 100, vai descendo depois de cada batida
int cor = 0;                   // 0 rosa, 1 roxo, 2 laranja, 3 amarelo
int acesos = 0;                // quantos LEDs estão acesos
unsigned long ultimaBatida = 0;

void setup() {
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
}

void loop() {
  int nivel = lerNivel();

  // média móvel: acompanha o volume da música
  media = (media * 19 + nivel) / 20;

  // há batida se o som passar claramente a média
  // e se já passaram 150 ms desde a última batida
  if (nivel > media + media / 2 + 10 && millis() - ultimaBatida > 150) {

    ultimaBatida = millis();
    brilho = 100;

    // batida forte acende a fita toda, batida fraca só metade
    if (nivel > media * 2 + 10) {
      acesos = 30;
    } else {
      acesos = 15;
    }

    // a cor depende da força da batida
    if (nivel > media * 3 + 10) {
      cor = 3;          // muito forte: amarelo
    } else if (nivel > media * 2 + 10) {
      cor = 2;          // forte: laranja
    } else if (nivel > media + media / 2 + 30) {
      cor = 1;          // média: roxo
    } else {
      cor = 0;          // fraca: rosa
    }

  } else {
    // sem batida, a luz apaga devagar
    if (brilho > 3) {
      brilho = brilho * 85 / 100;
    } else {
      brilho = 0;
    }
  }

  // cor base (valores de 0 a 255, baixos para o USB)
  int r = 0;
  int g = 0;
  int b = 0;

  if (cor == 0) {          // rosa
    r = 40; g = 4; b = 14;
  } else if (cor == 1) {   // roxo
    r = 20; g = 0; b = 40;
  } else if (cor == 2) {   // laranja
    r = 40; g = 10; b = 0;
  } else {                 // amarelo
    r = 40; g = 28; b = 0;
  }

  // aplica o brilho atual
  r = r * brilho / 100;
  g = g * brilho / 100;
  b = b * brilho / 100;

  desenhar(acesos, r, g, b);
}