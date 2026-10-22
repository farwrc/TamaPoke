#include "k10_prefs.h"
#include <SD_MMC.h>
#include <map>
#include <vector>
#include <string>

// Um "namespace" = um arquivo. Formato: repete [u8 tamanho da chave][chave][u16 tamanho do valor][valor].
struct Ns {
  std::string name;
  std::map<std::string, std::vector<uint8_t>> kv;
  bool dirty = false;
  uint32_t lastPut = 0;
};
static std::vector<Ns *> sNs;
extern bool sdReady;   // sdmon.cpp

[[noreturn]] static void fail(const char *msg, const char *what) {
  // Falha ALTO e para: sem o SD o jogo nao tem onde guardar o bichinho.
  for (;;) {
    Serial.printf("[k10 prefs] ERRO FATAL: %s (%s)\n", msg, what);
    delay(1000);
  }
}

static String pathOf(const std::string &name) { return String("/save/") + name.c_str() + ".prf"; }

static void load(Ns *n) {
  String path = pathOf(n->name);
  if (!SD_MMC.exists(path)) return;   // primeira vez: vazio
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) fail("nao abriu o arquivo de dados", path.c_str());
  while (f.available()) {
    int kl = f.read();
    if (kl <= 0) fail("arquivo de dados corrompido (tamanho da chave)", path.c_str());
    char key[256];
    if (f.readBytes(key, kl) != (size_t)kl) fail("arquivo de dados corrompido (chave)", path.c_str());
    key[kl] = 0;
    uint8_t lb[2];
    if (f.readBytes((char *)lb, 2) != 2) fail("arquivo de dados corrompido (tamanho do valor)", path.c_str());
    uint16_t vl = lb[0] | (lb[1] << 8);
    std::vector<uint8_t> v(vl);
    if (vl && f.readBytes((char *)v.data(), vl) != vl) fail("arquivo de dados corrompido (valor)", path.c_str());
    n->kv[key] = v;
  }
  f.close();
}

static void save(Ns *n) {
  SD_MMC.mkdir("/save");
  String tmp = pathOf(n->name) + ".tmp";
  File f = SD_MMC.open(tmp, FILE_WRITE);
  if (!f) fail("nao abriu o arquivo para gravar", tmp.c_str());
  for (auto &e : n->kv) {
    uint8_t kl = (uint8_t)e.first.size();
    uint16_t vl = (uint16_t)e.second.size();
    uint8_t lb[2] = {(uint8_t)(vl & 0xFF), (uint8_t)(vl >> 8)};
    f.write(&kl, 1);
    f.write((const uint8_t *)e.first.data(), kl);
    f.write(lb, 2);
    if (vl) f.write(e.second.data(), vl);
  }
  f.close();
  String dst = pathOf(n->name);
  if (SD_MMC.exists(dst)) SD_MMC.remove(dst);
  if (!SD_MMC.rename(tmp, dst)) fail("nao renomeou o arquivo gravado", dst.c_str());
  n->dirty = false;
}

bool K10Prefs::begin(const char *name, bool) {
  if (!sdReady) fail("o cartao SD nao esta montado: sem ele nao ha onde guardar o bichinho", name);
  for (size_t i = 0; i < sNs.size(); i++)
    if (sNs[i]->name == name) { ns = (int)i; return true; }
  Ns *n = new Ns();
  n->name = name;
  load(n);
  sNs.push_back(n);
  ns = (int)sNs.size() - 1;
  return true;
}

void K10Prefs::end() {
  if (ns >= 0 && sNs[ns]->dirty) save(sNs[ns]);
  ns = -1;
}

void K10Prefs::clear() {
  sNs[ns]->kv.clear();
  sNs[ns]->dirty = true;
  sNs[ns]->lastPut = millis();
}

bool K10Prefs::isKey(const char *key) { return sNs[ns]->kv.count(key) > 0; }

size_t K10Prefs::putBytes(const char *key, const void *value, size_t len) {
  Ns *n = sNs[ns];
  const uint8_t *p = (const uint8_t *)value;
  auto it = n->kv.find(key);
  if (it != n->kv.end() && it->second.size() == len && memcmp(it->second.data(), p, len) == 0) return len;  // nada mudou
  n->kv[key] = std::vector<uint8_t>(p, p + len);
  n->dirty = true;
  n->lastPut = millis();
  return len;
}

size_t K10Prefs::getBytes(const char *key, void *buf, size_t maxLen) {
  auto it = sNs[ns]->kv.find(key);
  if (it == sNs[ns]->kv.end()) return 0;
  size_t n = it->second.size() < maxLen ? it->second.size() : maxLen;
  memcpy(buf, it->second.data(), n);
  return n;
}

String K10Prefs::getString(const char *k, String d) {
  auto it = sNs[ns]->kv.find(k);
  if (it == sNs[ns]->kv.end()) return d;
  return String((const char *)it->second.data());
}

size_t K10Prefs::getString(const char *k, char *buf, size_t maxLen) {
  auto it = sNs[ns]->kv.find(k);
  if (it == sNs[ns]->kv.end()) { buf[0] = 0; return 0; }
  size_t n = it->second.size() < maxLen ? it->second.size() : maxLen;
  memcpy(buf, it->second.data(), n);
  buf[maxLen - 1 < n ? maxLen - 1 : n - 1] = 0;
  return n;
}

void k10_prefs_tick() {
  for (Ns *n : sNs)
    if (n->dirty && millis() - n->lastPut > 1000) save(n);
}

void k10_prefs_flush() {
  for (Ns *n : sNs)
    if (n->dirty) save(n);
}
