#include "BAT_Driver.h"

float BAT_analogVolts = 0;

void BAT_Init(void)
{
  //set the resolution to 12 bits (0-4095)
  analogReadResolution(12);
  // Deixa a media/suavizacao ja "assentada" num valor real antes de
  // qualquer tela usar a leitura, em vez de comecar de zero.
  for (int i = 0; i < 8; i++) BAT_Get_Volts();
}
float BAT_Get_Volts(void)
{
  // Media de 8 amostras cruas: uma unica leitura do ADC sozinha e
  // barulhenta (a faixa entre os icones BAT4 e BAT5, por exemplo, tem so'
  // 0.15V de largura, facil de ser cruzada por ruido eletrico e nao por
  // mudanca real de energia).
  uint32_t acc = 0;
  const int N = 8;
  for (int i = 0; i < N; i++) {
    acc += analogReadMilliVolts(BAT_ADC_PIN); // millivolts
    delayMicroseconds(150);
  }
  float sample = (float)((acc / N) * 3.0 / 1000.0) / Measurement_offset;

  // Suavizacao entre chamadas (media movel exponencial): o valor exposto
  // em BAT_analogVolts anda aos poucos, nao pula de uma vez. Como a tela
  // da biblioteca e a tela de leitura leem essa mesma variavel global em
  // momentos diferentes, elas passam a concordar entre si em vez de cada
  // uma tirar sua propria amostra instantanea.
  static bool primed = false;
  if (!primed) {
    BAT_analogVolts = sample;
    primed = true;
  } else {
    BAT_analogVolts = BAT_analogVolts * 0.85f + sample * 0.15f;
  }
  // printf("BAT voltage : %.2f V\r\n", BAT_analogVolts);
  return BAT_analogVolts;
}