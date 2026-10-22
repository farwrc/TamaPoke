# TamaPoke no K10 PRO (Waveshare ESP32-S3-Touch-LCD-2.8B)

Port do TamaPoke para a placa **Waveshare ESP32-S3-Touch-LCD-2.8B**, a mesma do leitor K10 PRO.
O jogo original foi feito para a Waveshare ESP32-S3-Touch-AMOLED-1.75. Os arquivos do jogo na raiz continuam os do projeto
original; os desvios para o K10 estao marcados com `#ifdef BOARD_K10_PRO`, e o codigo da placa fica nesta pasta (`k10/`).

## O que mudou em relacao a placa original

| Original (AMOLED 1.75) | K10 PRO |
|---|---|
| Tela redonda 466x466 (QSPI) | Tela 480x640 (RGB). O jogo continua em 466x466, centralizado; as faixas em volta sao pretas |
| Toque CST9217 | Toque GT911 |
| PMU AXP2101 (bateria, botao PWR) | Bateria lida pelo ADC; o botao **BOOT** liga/desliga a tela |
| Audio ES8311 + alto-falante | **Buzzer** da placa: so liga e desliga, entao os efeitos viram bipes com o ritmo da melodia |
| Cartao SD formatado sozinho se nao montar | **Nunca formata**: se o cartao nao monta, mostra o erro e deixa o cartao como esta |

## Ajustes (todos em `k10/k10_config.h`, numeros fixos)

- `K10_VIEW_X` / `K10_VIEW_Y`: onde fica o jogo na tela.
- `K10_COL_BANDS`: cor das faixas em volta.
- `K10_TOUCH_RELEASE_MS`: tempo sem toque para considerar o dedo levantado.
- `K10_BEEP_MIN_MS` / `K10_BEEP_GAP_MS`: duracao minima e pausa entre os bipes.

## Como gravar (VS Code + PlatformIO)

1. Abra a pasta do fork no VS Code (**File > Open Folder**) e espere o PlatformIO terminar de carregar.
2. Na barra azul de baixo, clique no **✓** (Build) e espere `SUCCESS`.
3. Ligue a placa pelo USB e clique na **seta →** (Upload).

## Sprites no cartao SD

1. Use um cartao **so para o TamaPoke** (FAT32). Nao use o cartao do leitor de livros.
2. Copie a pasta `tools/sdcard/mons` para a **raiz** do cartao (fica `/mons/p001.bin`, `/mons/thumbs.bin` ...).
3. Coloque o cartao na placa e ligue.

## Licencas

Codigo: MIT (projeto original de Quique Tortosa). Sprites: PMD SpriteCollab, CC BY-NC 4.0 (so uso nao comercial).
Pokemon e seus nomes pertencem a Nintendo, Game Freak e The Pokemon Company. Este port e um presente pessoal, sem fins comerciais.
