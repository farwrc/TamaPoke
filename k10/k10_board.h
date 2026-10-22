#pragma once
// Camada da placa Waveshare ESP32-S3-Touch-LCD-2.8B (K10 PRO) para o TamaPoke.
// Troca: tela AMOLED redonda 466x466 por tela RGB 480x640, toque CST9217 por GT911,
// PMU AXP2101 por leitura de bateria no ADC + botao BOOT, e audio ES8311 por buzzer.
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "k10_pins.h"
#include "k10_config.h"

// "Tela" que o Arduino_Canvas do jogo usa como saida: empurra o quadro para o painel RGB do K10.
class Arduino_K10Panel : public Arduino_G {
 public:
  Arduino_K10Panel() : Arduino_G(K10_SCREEN_W, K10_SCREEN_H) {}
  bool begin(int32_t speed = GFX_NOT_DEFINED) override;
  void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
  // O jogo so usa o canvas inteiro (draw16bitRGBBitmap). Os outros desenhos nao sao usados: se alguem chamar, avisa alto.
  void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) override;
  void drawIndexedBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t *color_index, int16_t w, int16_t h, int16_t x_skip = 0) override;
  void draw3bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
  void draw24bitRGBBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override;
  void setBrightness(uint8_t v255);   // 0..255, como no painel AMOLED original
};
extern Arduino_K10Panel *panel;

void k10_board_begin();     // I2C, expansor, luz de fundo (apagada), tela, toque, bateria, buzzer em silencio

// Toque em coordenadas do jogo (0..465). Retorna true enquanto o dedo estiver apoiado dentro da area do jogo.
// Toque que COMECA fora da area do jogo (nas faixas) e' ignorado ate' o dedo levantar.
bool k10_touch_get(int16_t *x, int16_t *y);

// Botao BOOT (GPIO0): true enquanto apertado
bool k10_boot_down();

// Bateria e USB
float k10_bat_volts();
int k10_bat_percent();      // 0..100, ou -1 se nao ha bateria
bool k10_bat_charging();
bool k10_usb_present();
void k10_step(uint32_t n);      // DIAGNOSTICO temporario
void k10_report_boot();

// O barramento I2C e' usado pelo jogo (laco principal) e pela tarefa do buzzer: este travamento evita os dois falarem ao mesmo tempo.
void k10_i2c_lock();
void k10_i2c_unlock();

// Buzzer (EXIO8 do expansor)
void k10_buzzer(bool on);
