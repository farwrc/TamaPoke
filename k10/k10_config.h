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

// Botoes de baixo (comer, jogar, luz, banho): centro de cada um em pixels do jogo (area 466x466). Editar aqui para ajustar.
#define K10_BTN_Y    404            // todos na mesma linha (reta)
#define K10_BTN1_X   140
#define K10_BTN2_X   202
#define K10_BTN3_X   264
#define K10_BTN4_X   326

// Bateria na faixa de cima da tela fisica (480x640): canto de cima/esquerda do icone (24 x 11), em pixels da TELA.
#define K10_BAT_X    441
#define K10_BAT_Y    14

// Tamanho da area do jogo (o canvas do TamaPoke).
#define K10_GAME_W 466
#define K10_GAME_H 466
