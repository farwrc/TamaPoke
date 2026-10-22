// Som do TamaPoke no K10 PRO. A placa nao tem alto-falante nem codec, so' um buzzer ligado/desligado pelo
// expansor (EXIO8). Ele nao faz notas: cada efeito vira uma sequencia de bipes com o mesmo ritmo da melodia original.
#ifdef BOARD_K10_PRO
#include "audio.h"
#include "k10_board.h"
#include <Preferences.h>

struct Note { uint16_t f, ms; };   // f = frequencia original (so' diz "toca" ou "pausa" aqui), ms = duracao

static const Note N_TAP[]    = {{880, 35}};
static const Note N_EAT[]    = {{660, 45}, {0, 12}, {660, 45}};
static const Note N_PLAY[]   = {{784, 45}, {988, 60}};
static const Note N_HEART[]  = {{1047, 55}, {1319, 90}};
static const Note N_HATCH[]  = {{523, 80}, {659, 80}, {784, 110}, {1047, 170}};
static const Note N_EVOLVE[] = {{523, 80}, {659, 80}, {784, 80}, {1047, 90}, {1319, 230}};
static const Note N_MEDAL[]  = {{784, 70}, {0, 25}, {784, 70}, {0, 25}, {1047, 200}};
static const Note N_DENY[]   = {{300, 110}, {200, 170}};
static const Note N_BYE[]    = {{784, 150}, {659, 150}, {523, 280}};
static const Note N_LEVEL[]  = {{784, 70}, {1047, 130}};

struct SfxDef { const Note *n; uint8_t len; };
static const SfxDef SFX[SFX_COUNT] = {
  {N_TAP, 1}, {N_EAT, 3}, {N_PLAY, 2}, {N_HEART, 2}, {N_HATCH, 4},
  {N_EVOLVE, 5}, {N_MEDAL, 5}, {N_DENY, 2}, {N_BYE, 3}, {N_LEVEL, 2},
};

static QueueHandle_t gQ = nullptr;
static volatile bool gOn = true;
static volatile bool gSleeping = false;
static volatile bool gReady = false;

static void audioTask(void *) {
  uint8_t id;
  for (;;) {
    if (xQueueReceive(gQ, &id, portMAX_DELAY) && gOn && !gSleeping && id < SFX_COUNT) {
      const SfxDef &d = SFX[id];
      for (uint8_t i = 0; i < d.len; i++) {
        uint16_t ms = d.n[i].ms;
        if (d.n[i].f == 0) { vTaskDelay(pdMS_TO_TICKS(ms)); continue; }   // pausa
        if (ms < K10_BEEP_MIN_MS) ms = K10_BEEP_MIN_MS;
        k10_buzzer(true);
        vTaskDelay(pdMS_TO_TICKS(ms));
        k10_buzzer(false);
        vTaskDelay(pdMS_TO_TICKS(K10_BEEP_GAP_MS));
      }
    }
  }
}

void audioBegin() {
  Preferences p;
  p.begin("tamapoke", true);
  gOn = p.getBool("snd", true);
  p.end();
  gQ = xQueueCreate(8, sizeof(uint8_t));
  gReady = true;
  xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 1, nullptr, 0);
  sfxPlay(SFX_HATCH);   // sequencia de arranque: confirma que o buzzer funciona
}

void sfxPlay(uint8_t id) {
  if (gReady && gOn && gQ) xQueueSend(gQ, &id, 0);   // descarta se a fila estiver cheia
}

void audioSetEnabled(bool on) {
  gOn = on;
  Preferences p;
  p.begin("tamapoke", false);
  p.putBool("snd", on);
  p.end();
}
bool audioEnabled() { return gOn; }

void audioSetSleeping(bool sleeping) { gSleeping = sleeping; }
#endif
