// Substituto do Preferences (NVS) para o K10 PRO: guarda os dados no cartao SD.
//
// POR QUE: no K10 o painel RGB le o framebuffer da PSRAM por interrupcao. Qualquer leitura/escrita
// na flash interna (NVS = Preferences) desliga o cache por instantes, a interrupcao do painel
// continua rodando e acessa a PSRAM -> "Cache disabled but cached memory region accessed" (reset).
// O cartao SD nao usa a flash interna, entao nao tem esse problema.
//
// Mesma interface do Preferences do Arduino (so' o que o jogo usa). Os dados ficam na RAM e sao
// gravados em /sdcard/save/<namespace>.prf pouco depois da ultima alteracao (k10_prefs_tick()).
#pragma once
#include <Arduino.h>

class K10Prefs {
 public:
  bool begin(const char *name, bool readOnly = false);
  void end();
  void clear();
  bool isKey(const char *key);

  size_t putBytes(const char *key, const void *value, size_t len);
  size_t getBytes(const char *key, void *buf, size_t maxLen);

  size_t putBool(const char *k, bool v)      { return putBytes(k, &v, sizeof(v)); }
  size_t putChar(const char *k, int8_t v)    { return putBytes(k, &v, sizeof(v)); }
  size_t putUChar(const char *k, uint8_t v)  { return putBytes(k, &v, sizeof(v)); }
  size_t putShort(const char *k, int16_t v)  { return putBytes(k, &v, sizeof(v)); }
  size_t putUShort(const char *k, uint16_t v){ return putBytes(k, &v, sizeof(v)); }
  size_t putUInt(const char *k, uint32_t v)  { return putBytes(k, &v, sizeof(v)); }
  size_t putString(const char *k, const char *v) { return putBytes(k, v, strlen(v) + 1); }
  size_t putString(const char *k, String v)      { return putString(k, v.c_str()); }

  bool     getBool(const char *k, bool d = false)       { return get<bool>(k, d); }
  int8_t   getChar(const char *k, int8_t d = 0)         { return get<int8_t>(k, d); }
  uint8_t  getUChar(const char *k, uint8_t d = 0)       { return get<uint8_t>(k, d); }
  int16_t  getShort(const char *k, int16_t d = 0)       { return get<int16_t>(k, d); }
  uint16_t getUShort(const char *k, uint16_t d = 0)     { return get<uint16_t>(k, d); }
  uint32_t getUInt(const char *k, uint32_t d = 0)       { return get<uint32_t>(k, d); }
  String   getString(const char *k, String d = String());
  size_t   getString(const char *k, char *buf, size_t maxLen);

 private:
  template <typename T> T get(const char *k, T d) {
    T v = d;
    if (isKey(k)) getBytes(k, &v, sizeof(v));
    return v;
  }
  int ns = -1;
};

// Chamar do loop(): grava no SD as alteracoes pendentes (espera ~1 s sem novas alteracoes).
void k10_prefs_tick();
// Grava ja' tudo que estiver pendente (usado ao desligar a tela, etc.).
void k10_prefs_flush();

#define Preferences K10Prefs
