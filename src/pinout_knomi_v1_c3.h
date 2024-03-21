#ifndef PINOUT_KNOMI_V1_C3_H
#define PINOUT_KNOMI_V1_C3_H

#define BOOT_PIN     1

// knomi v1
#define GC9A01_MOSI_PIN 7 // In some display driver board, it might be written as "SDA" and so on.
#define GC9A01_SCLK_PIN 6
#define GC9A01_CS_PIN   10  // Chip select control pin
#define GC9A01_DC_PIN   0  // Data Command control pin
#define GC9A01_RST_PIN  5  // Reset pin (could connect to Arduino RESET pin)

// // PWM
#define LCD_BL_PIN      4

#endif
