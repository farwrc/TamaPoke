#pragma once
// Pinos e tamanho do JOGO para a Waveshare ESP32-S3-Touch-LCD-2.8B (K10 PRO).
// Os pinos da tela RGB, do toque (GT911), da bateria e do expansor ficam nos drivers copiados do leitor K10
// (Display_ST7701.h, Touch_GT911.h, BAT_Driver.h, TCA9554PWR.h).

// Barramento I2C (compartilhado por toque GT911, RTC PCF85063, expansor TCA9554 e IMU)
#define IIC_SDA 15
#define IIC_SCL 7

// Cartao SD (SD_MMC em modo 1 bit). O CS/D3 do cartao fica no expansor (EXIO4) e precisa estar em nivel alto.
#define SDMMC_CLK  2
#define SDMMC_CMD  1
#define SDMMC_DATA 42

// Area do jogo, em pixels: o jogo desenha 466x466 (o tamanho da tela redonda original).
// A tela do K10 e' 480x640; onde essa area fica na tela esta em k10_config.h.
#define LCD_WIDTH  466
#define LCD_HEIGHT 466
