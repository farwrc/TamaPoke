#include <esp_system.h>
#include <esp_core_dump.h>
#include "k10_board.h"       // primeiro: traz o Arduino_GFX antes dos drivers (que definem High/Low)
#if ARDUINO_USB_MODE == 0
#include <USB.h>
#endif
#include "Display_ST7701.h"
#include "BAT_Driver.h"

Arduino_K10Panel *panel = new Arduino_K10Panel();

// ---------------------------------------------------------------- tela
bool Arduino_K10Panel::begin(int32_t) {
  if (!LCD_rgb_alive()) {
    Serial.println("[k10] ERRO: o painel RGB nao foi criado - chame k10_board_begin() antes de gfx->begin()");
    return false;
  }
  return true;
}

void Arduino_K10Panel::draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) {
  LCD_addWindow(x, y, x + w - 1, y + h - 1, (uint8_t *)bitmap);
}

#define K10_NOT_USED(nome) Serial.println("[k10] ERRO: " nome " nao e' usado pelo jogo e nao foi implementado para a tela do K10")
void Arduino_K10Panel::drawBitmap(int16_t, int16_t, uint8_t *, int16_t, int16_t, uint16_t, uint16_t) { K10_NOT_USED("drawBitmap"); }
void Arduino_K10Panel::drawIndexedBitmap(int16_t, int16_t, uint8_t *, uint16_t *, int16_t, int16_t, int16_t) { K10_NOT_USED("drawIndexedBitmap"); }
void Arduino_K10Panel::draw3bitRGBBitmap(int16_t, int16_t, uint8_t *, int16_t, int16_t) { K10_NOT_USED("draw3bitRGBBitmap"); }
void Arduino_K10Panel::draw24bitRGBBitmap(int16_t, int16_t, uint8_t *, int16_t, int16_t) { K10_NOT_USED("draw24bitRGBBitmap"); }

void Arduino_K10Panel::setBrightness(uint8_t v255) {
  Set_Backlight((uint8_t)(((uint16_t)v255 * 100 + 127) / 255));
}

// pinta a tela inteira com uma cor (as faixas em volta do jogo ficam com ela; o centro e' sobrescrito pelo jogo)
static void fillScreen565(uint16_t color) {
  static uint16_t line[K10_SCREEN_W];
  for (int i = 0; i < K10_SCREEN_W; i++) line[i] = color;
  for (int y = 0; y < K10_SCREEN_H; y++) LCD_addWindow(0, y, K10_SCREEN_W - 1, y, (uint8_t *)line);
}

// ---------------------------------------------------------------- I2C
static SemaphoreHandle_t sI2c = nullptr;
void k10_i2c_lock() { if (sI2c) xSemaphoreTakeRecursive(sI2c, portMAX_DELAY); }
void k10_i2c_unlock() { if (sI2c) xSemaphoreGiveRecursive(sI2c); }

// ---------------------------------------------------------------- USB (so' para saber se ha cabo)
#if ARDUINO_USB_MODE == 0
static volatile bool sUsb = false;
static void usbEvent(void *, esp_event_base_t base, int32_t id, void *) {
  if (base != ARDUINO_USB_EVENTS) return;
  if (id == ARDUINO_USB_STARTED_EVENT) sUsb = true;
  else if (id == ARDUINO_USB_STOPPED_EVENT) sUsb = false;
}
// O evento de conexao dispara durante o Serial.begin(): o ouvinte precisa existir antes do setup() (mesma tecnica do leitor K10).
struct K10UsbWatch { K10UsbWatch() { USB.onEvent(usbEvent); } };
static K10UsbWatch sUsbWatch;

#else
static volatile bool sUsb = false;   // DIAGNOSTICO: modo HWCDC (USB serial/JTAG), sem TinyUSB
#endif
bool k10_usb_present() { return sUsb; }

// ---------------------------------------------------------------- DIAGNOSTICO (temporario)
// Guarda em qual passo do setup() a placa estava; sobrevive ao reset (RTC_NOINIT) e e' impresso no proximo boot.
static RTC_NOINIT_ATTR uint32_t sStepMagic;
static RTC_NOINIT_ATTR uint32_t sStep;
void k10_step(uint32_t n) { sStepMagic = 0xA5A55A5A; sStep = n; }
// Pisca a luz da tela (GPIO6) mostrando o ultimo passo, sem depender da USB: dezenas = piscadas LONGAS, unidades = piscadas CURTAS.
void k10_blink_last() {
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); delay(1500); digitalWrite(6, LOW); delay(1000);   // inicio: 1 acendida bem longa
  if (sStepMagic != 0xA5A55A5A) { delay(1000); return; }                  // sem registro: so' a acendida longa
  for (uint32_t i = 0; i < sStep / 10; i++) { digitalWrite(6, HIGH); delay(600); digitalWrite(6, LOW); delay(400); }
  delay(500);
  for (uint32_t i = 0; i < sStep % 10; i++) { digitalWrite(6, HIGH); delay(200); digitalWrite(6, LOW); delay(300); }
  delay(1500);
}
void k10_report_boot() {
  int r = (int)esp_reset_reason();
  int st = sStepMagic == 0xA5A55A5A ? (int)sStep : -1;
  static RTC_NOINIT_ATTR uint32_t sBoots;
  sBoots = (sStepMagic == 0xA5A55A5A) ? sBoots + 1 : 1;
  Serial.printf("[DIAG] partida n=%u  motivo do reset=%d  ultimo passo=%d\n", (unsigned)sBoots, r, st);
  // Se a partida anterior travou (4=panic, 5..7=watchdog), PARA aqui repetindo o aviso: sem o loop de reinicio a USB fica estavel e da' para ler.
  static esp_core_dump_summary_t sum;
  bool haveDump = false;
  if (r == 4 || r == 5 || r == 6 || r == 7) {
    esp_err_t ck = esp_core_dump_image_check();
    haveDump = (ck == ESP_OK) && esp_core_dump_get_summary(&sum) == ESP_OK;
    Serial.printf("[DIAG] coredump: check=%d resumo=%d\n", (int)ck, (int)haveDump);
  }
  while (r == 4 || r == 5 || r == 6 || r == 7) {
    if (haveDump) {
      char why[200] = "?";
      esp_core_dump_get_panic_reason(why, sizeof(why));
      Serial.printf("[DIAG] CRASH tarefa=%s pc=0x%08x causa=%u vaddr=0x%08x motivo='%s'\n", sum.exc_task, (unsigned)sum.exc_pc, (unsigned)sum.ex_info.exc_cause, (unsigned)sum.ex_info.exc_vaddr, why);
      Serial.printf("[DIAG] Backtrace:");
      for (uint32_t i = 0; i < sum.exc_bt_info.depth && i < 16; i++) Serial.printf(" 0x%08x", (unsigned)sum.exc_bt_info.bt[i]);
      Serial.printf("\n[DIAG] elf_sha=%s\n", (const char *)sum.app_elf_sha256);
    }
    Serial.printf("[DIAG] PARADO t=%lums partida n=%u: a partida anterior TRAVOU (motivo=%d) no passo %d. Aperte RESET.\n", (unsigned long)millis(), (unsigned)sBoots, r, st);
    delay(1000);
  }
}

// ---------------------------------------------------------------- inicio
void k10_board_begin() {
  sI2c = xSemaphoreCreateRecursiveMutex();
  k10_step(30); I2C_Init();
  k10_step(31); TCA9554PWR_Init(0x00);
  k10_step(32); Set_EXIO(EXIO_PIN8, Low);    // buzzer em silencio
  Set_EXIO(EXIO_PIN4, High);   // D3/CS do cartao SD precisa estar em nivel alto para o SD_MMC montar
  vTaskDelay(pdMS_TO_TICKS(10));
  k10_step(33); BAT_Init();
  k10_step(34); Backlight_Init();            // luz de fundo APAGADA (LCD_Backlight = 0); o jogo acende depois
  k10_step(35); LCD_Init();                  // reset + ST7701 + painel RGB + toque GT911
  k10_step(36); pinMode(0, INPUT_PULLUP);    // botao BOOT
  fillScreen565(K10_COL_BANDS);
  k10_step(37); Set_Backlight(40);           // DIAGNOSTICO: acende ja para ver que o LCD inicializou (remover depois)
}

// ---------------------------------------------------------------- toque
bool k10_touch_get(int16_t *x, int16_t *y) {
  static bool down = false, ignoring = false;
  static int16_t lx = 0, ly = 0;
  static uint32_t lastHit = 0;

  if (Touch_interrupts) {
    Touch_interrupts = false;
    uint16_t px[1], py[1], st[1];
    uint8_t n = 0;
    k10_i2c_lock();
    Touch_Read_Data();
    k10_i2c_unlock();
    bool got = Touch_Get_XY(px, py, st, &n, 1) && n > 0;
    if (got) {
      int fx = (int)px[0] - K10_VIEW_X, fy = (int)py[0] - K10_VIEW_Y;
      bool inside = fx >= 0 && fx < LCD_WIDTH && fy >= 0 && fy < LCD_HEIGHT;
      if (!down && !inside) ignoring = true;                 // o toque comecou numa faixa: ignora ate' levantar
      if (!ignoring) {
        lx = (int16_t)constrain(fx, 0, LCD_WIDTH - 1);       // arrastando para fora da area, prende na borda
        ly = (int16_t)constrain(fy, 0, LCD_HEIGHT - 1);
        down = true;
      }
      lastHit = millis();
    } else {                                                 // aviso do toque sem pontos = dedo levantado
      down = false;
      ignoring = false;
    }
  }
  if ((down || ignoring) && millis() - lastHit > K10_TOUCH_RELEASE_MS) { down = false; ignoring = false; }
  if (down) { *x = lx; *y = ly; }
  return down;
}

bool k10_boot_down() { return digitalRead(0) == LOW; }

// ---------------------------------------------------------------- bateria
float k10_bat_volts() { return BAT_Get_Volts(); }

int k10_bat_percent() {
  float v = BAT_Get_Volts();
  if (v < K10_BAT_ABSENT_V) return -1;
  // aproximacao de bateria de litio (V -> %), com os mesmos degraus de tensao do icone de bateria do leitor K10
  static const struct { float v; int pct; } curve[] = {
    {3.30f, 0}, {3.50f, 10}, {3.65f, 35}, {3.80f, 60}, {3.95f, 80}, {4.15f, 100},
  };
  const int n = sizeof(curve) / sizeof(curve[0]);
  if (v <= curve[0].v) return 0;
  for (int i = 1; i < n; i++) {
    if (v <= curve[i].v) {
      float t = (v - curve[i - 1].v) / (curve[i].v - curve[i - 1].v);
      return curve[i - 1].pct + (int)(t * (curve[i].pct - curve[i - 1].pct));
    }
  }
  return 100;
}

bool k10_bat_charging() {
  float v = BAT_Get_Volts();
  return v >= K10_BAT_ABSENT_V && v >= K10_BAT_CHARGING_V;
}

// ---------------------------------------------------------------- buzzer
void k10_buzzer(bool on) {
  k10_i2c_lock();
  Set_EXIO(EXIO_PIN8, on ? High : Low);
  k10_i2c_unlock();
}
