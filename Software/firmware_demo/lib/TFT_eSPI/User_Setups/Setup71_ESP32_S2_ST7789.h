// Setup for the ESP32 S2 with ST7789 display
// Note SPI DMA with ESP32 S2 is not currently supported
#define USER_SETUP_ID 71
// See SetupX_Template.h for all options available

#define ST7789_DRIVER     // Configure all registers


// #define TFT_CS   34 //     10 or 34
// #define TFT_MOSI 35 //     11 or 35
// #define TFT_SCLK 37 //     12 or 36
// #define TFT_MISO 36 //     13 or 37

#define ESP32S2_12K
#if defined(ESP32S2_12K)    // #define TFT_RES_PIN 9
    #define TFT_DC 13
    #define TFT_SCLK 12
    #define TFT_MOSI 11
    #define TFT_CS 10
    #define TFT_MISO 13
    #define TFT_RST  9
#else
    #define TFT_MISO 19
    #define TFT_MOSI 23
    #define TFT_SCLK 18
    #define TFT_CS    -1 // Not connected
    #define TFT_DC    17
    #define TFT_RST   16  // Connect reset to ensure display initialises
#endif

// #define TFT_DC   13
// #define TFT_RST  -1

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY   27000000

#define SPI_TOUCH_FREQUENCY  2500000
