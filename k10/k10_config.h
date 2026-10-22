#pragma once
// Ajustes do TamaPoke na Waveshare ESP32-S3-Touch-LCD-2.8B (K10 PRO).
// Todos os numeros sao fixos, em pixels ou milissegundos. Mude aqui, grave de novo e veja na tela.

// ---- Tela ----
#define K10_SCREEN_W        480        // largura da tela fisica
#define K10_SCREEN_H        640        // altura da tela fisica
#define K10_VIEW_X          7          // X do canto de cima/esquerda da area do jogo (466x466) na tela
#define K10_VIEW_Y          87         // Y do canto de cima/esquerda da area do jogo na tela
#define K10_COL_BANDS       0x0000     // cor das faixas de cima, de baixo e dos lados em volta do jogo (RGB565: 0x0000 = preto)

// ---- Toque ----
#define K10_TOUCH_RELEASE_MS 90        // sem sinal do toque por este tempo (ms) = dedo levantado

// ---- Som (buzzer da placa: so liga e desliga, sem notas) ----
#define K10_BEEP_MIN_MS     25         // duracao minima de um bipe (ms)
#define K10_BEEP_GAP_MS     35         // silencio (ms) depois de cada bipe, para dois bipes seguidos nao virarem um so

// ---- Bateria (leitura pelo ADC; a tabela de % e' uma aproximacao de bateria de litio) ----
#define K10_BAT_ABSENT_V    2.5f       // abaixo disto (V) considera que nao ha bateria ligada
#define K10_BAT_CHARGING_V  4.15f      // a partir disto (V) considera carregando / no cabo (mesma regra do icone do leitor K10)

// DIAGNOSTICO (temporario): 1 = o jogo le o toque mas IGNORA tudo (nenhum bipe, nenhuma reacao), so' para ver se a tela ainda pisca. 0 = jogo normal.
#define K10_DIAG_TOUCH_IGNORE 0
