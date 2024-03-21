#include "lvgl_hal.h"
#include "pinout.h"
#include "lv_fs_littlefs.h"
#include <AnimatedGIF.h>
#include "../test_images/badgers.h"
#include "../test_images/homer.h"
#include "../test_images/homer_tiny.h"
// #include "../test_images/pattern.h"

lv_color_t *lv_disp_buf_p;

TFT_eSPI tft_gc9a01 = TFT_eSPI();
#ifdef CST816S_SUPPORT
extern TwoWire i2c0;
CST816S ts_cst816s = CST816S(CST816S_RST_PIN, CST816S_IRQ_PIN, &i2c0);
#endif

// #define BUFFER_SIZE 240 // Optimum is >= GIF width or integral division of width

// #ifdef USE_DMAs
// uint16_t usTemp[2][BUFFER_SIZE]; // Global to support DMA use
// #else
// uint16_t usTemp[1][BUFFER_SIZE]; // Global to support DMA use
// #endif
// bool dmaBuf = 0;

// // const unsigned char homer_tiny[] PROGMEM = {
// //   0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
// #define GIF_IMAGE badgers  //  No DMA  63 fps, DMA:  71fps
// AnimatedGIF gif = AnimatedGIF();

void my_log_cb(const char *buf);

void my_log_cb(const char *buf)
{
    Serial.println(buf);
}

void *gif_open_file1(const char *szFilename, int32_t *pFileSize)
{
    // Serial.printf("gif_open_file %s", szFilename);
    lv_fs_file_t *f = (lv_fs_file_t *)lv_mem_alloc(sizeof(lv_fs_file_t));
    // Serial.printf("file handler: %p", f);
    lv_fs_res_t res = lv_fs_open(f, szFilename, LV_FS_MODE_RD);
    // Serial.printf("lv_fs_open done, reuslt: %d", res);
    if (res != LV_FS_RES_OK)
        return NULL;
    lv_fs_seek(f, 0, LV_FS_SEEK_END);
    res = lv_fs_tell(f, (uint32_t *)pFileSize);
    // Serial.printf("lv_fs_tell done, reuslt: %d, file size: %d", res, *pFileSize);
    if (res != LV_FS_RES_OK)
        return NULL;
    res = lv_fs_seek(f, 0, LV_FS_SEEK_SET);
    // Serial.printf("lv_fs_seek done, reuslt: %d", res);
    if (res != LV_FS_RES_OK)
        return NULL;
    return f;
}

int32_t gif_read_file1(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    // Serial.printf("buf: %p, read length: %d", pBuf, iLen);
    lv_fs_file_t *file = (lv_fs_file_t *)pFile->fHandle;
    // Serial.printf("file handler: %p", file);
    // Serial.printf("file pos: %d", pFile->iPos);
    uint32_t realReadBytes;
    lv_fs_read(file, pBuf, iLen, &realReadBytes);
    // Serial.printf("lv_fs_read done, realReadBytes: %d", realReadBytes);
    lv_fs_tell(file, (uint32_t *)&(pFile->iPos));
    return realReadBytes;
}

void gif_close_file1(void *pHandle)
{
    lv_fs_file_t *file = (lv_fs_file_t *)pHandle;
    if (file != NULL)
    {
        lv_fs_close(file);
        lv_mem_free(file);
    }
}

int32_t gif_seek_file1(GIFFILE *pFile, int32_t iPosition)
{
    // Serial.printf("gif_seek_file");
    lv_fs_file_t *file = (lv_fs_file_t *)pFile->fHandle;
    lv_fs_seek(file, iPosition, LV_FS_SEEK_SET);
    lv_fs_tell(file, (uint32_t *)&pFile->iPos);
    return pFile->iPos;
}

/* Display flushing */
void usr_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // usr fill color
    tft_gc9a01.startWrite();
    tft_gc9a01.setAddrWindow(area->x1, area->y1, w, h);
    tft_gc9a01.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft_gc9a01.endWrite();

    lv_disp_flush_ready(disp);
}

#ifdef CST816S_SUPPORT
void touch_idle_time_clear(void);
void usr_touchpad_read(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static touch_event_t event;
    if (ts_cst816s.ready())
    {
        ts_cst816s.getTouch(&event);
    }
    if (event.finger)
    {
        data->state = LV_INDEV_STATE_PR;
        /*Set the coordinates*/
        data->point.x = event.x;
        data->point.y = event.y;
        touch_idle_time_clear();
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}
#endif

static int8_t aw9346_from_light = -1;

void tft_backlight_init(void)
{
    pinMode(LCD_BL_PIN, OUTPUT);
    digitalWrite(LCD_BL_PIN, LOW);
    delay(3); // > 2.5ms for shutdown
    aw9346_from_light = 0;
}
void tft_set_backlight(int8_t aw9346_to_light)
{
    if (aw9346_to_light > 16)
        aw9346_to_light = 16;
    if (aw9346_to_light < 0)
        aw9346_to_light = 0;
    if (aw9346_from_light == aw9346_to_light)
        return;

    if (aw9346_to_light == 0)
    {
        digitalWrite(LCD_BL_PIN, LOW);
        delay(3); // > 2.5ms for shutdown
        aw9346_from_light = 0;
        return;
    }
    if (aw9346_from_light <= 0)
    {
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(25); // > 20us for poweron
        aw9346_from_light = 16;
    }

    if (aw9346_from_light < aw9346_to_light)
        aw9346_from_light += 16;

    int8_t num = aw9346_from_light - aw9346_to_light;

    for (int8_t i = 0; i < num; i++)
    {
        digitalWrite(LCD_BL_PIN, LOW);
        delayMicroseconds(1); // 0.5us < T_low < 500us
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(1); // 0.5us < T_high
    }

    aw9346_from_light = aw9346_to_light;
}

void tft_fps_test(void)
{
    uint8_t test_sec = 3;
    uint32_t ms = millis() + test_sec * 1000;
    uint32_t frames = 0;
    const uint32_t test_colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE};
    while (ms > millis())
    {
        tft_gc9a01.fillScreen(test_colors[frames % (sizeof(test_colors) / sizeof(test_colors[0]))]);
        frames++;
    }
    Serial.println("\r\n******** lcd fps test *****\r\n");
    Serial.print("fps=");
    Serial.println(frames / test_sec, DEC);
    Serial.print("test_sec=");
    Serial.println(test_sec, DEC);
    Serial.println("\r\n***************************\r\n");
}

// // Function to convert color index from GIF palette to lv_color_t
// lv_color_t convert_palette_to_lv_colors(uint8_t* color_index, uint8_t palette_type) {

//     // Determine the type of palette (RGB565 or RGB888)
//     switch (palette_type) {
//         case GIF_PALETTE_RGB565_LE:
//         case GIF_PALETTE_RGB565_BE: {
//             // Convert the color index to RGB565 format
//             uint8_t r = (color_index[2] >> 8) & 0xF8;
//             uint8_t g = (color_index[1] >> 3) & 0xFC;
//             uint8_t b = (color_index[0] << 3) & 0xF8;
//             // Return the RGB565 color
//             return LV_COLOR_MAKE(r, g, b);
//         }
//         case GIF_PALETTE_RGB888:
//         default: {
//             // Convert the color index to RGB888 format
//             uint8_t r = color_index[2];
//             uint8_t g = color_index[1];
//             uint8_t b = color_index[0];
//             // Return the RGB888 color
//             return LV_COLOR_MAKE(r, g, b);
//         }
//     }
// }

// void GIFDraw2(GIFDRAW *pDraw) {
//     uint8_t *s;
//     uint16_t *usPalette;
//     int x, y, iWidth;
//     uint8_t *p;

//     // Display bounds check and cropping
//     iWidth = pDraw->iWidth;
//     if (iWidth + pDraw->iX > TFT_WIDTH)
//         iWidth = TFT_WIDTH - pDraw->iX;
//     usPalette = pDraw->pPalette;
//     y = pDraw->iY + pDraw->y; // current line
//     if (y >= TFT_HEIGHT || pDraw->iX >= TFT_WIDTH || iWidth < 1) {
//         // Invalid parameters, return empty buffer
//         return;
//     }

//     // Apply the new pixels to the buffer
//     s = pDraw->pPixels;
//     lv_color_t* buff = (lv_color_t*)lv_mem_alloc(sizeof(lv_color_t)*iWidth);
//     for (x = 0; x < iWidth; x++) {
//         p = (u_int8_t *)&usPalette[s[x] * 3];
//         buff[x] = convert_palette_to_lv_colors(p, GIF_PALETTE_RGB888);
//     }
//     tft_gc9a01.setAddrWindow(pDraw->iX, y, iWidth, 1);
//     tft_gc9a01.pushColors((uint16_t *)&buff->full, iWidth, true);
//     lv_mem_free(buff);
// }

// used to center image based on GIF dimensions
static int xOffset = 0;
static int yOffset = 0;

static void TFTDraw(int x, int y, int w, int h, uint16_t *lBuf)
{
    tft_gc9a01.pushRect(x + xOffset, y + yOffset, w, h, lBuf);
}

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
    Serial.println("GIFDraw");
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth > TFT_WIDTH)
        iWidth = TFT_WIDTH;
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line

    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2)
    { // restore to background color
        for (x = 0; x < iWidth; x++)
        {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency)
    { // if transparency used
        uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
        int x, iCount;
        pEnd = s + iWidth;
        x = 0;
        iCount = 0; // count non-transparent pixels
        while (x < iWidth)
        {
            c = ucTransparent - 1;
            d = usTemp;
            while (c != ucTransparent && s < pEnd)
            {
                c = *s++;
                if (c == ucTransparent)
                {        // done, stop
                    s--; // back up to treat it like transparent
                }
                else
                { // opaque
                    *d++ = usPalette[c];
                    iCount++;
                }
            } // while looking for opaque pixels
            if (iCount)
            { // any opaque pixels?
                TFTDraw(pDraw->iX + x, y, iCount, 1, (uint16_t *)usTemp);
                x += iCount;
                iCount = 0;
            }
            // no, look for a run of transparent pixels
            c = ucTransparent;
            while (c == ucTransparent && s < pEnd)
            {
                c = *s++;
                if (c == ucTransparent)
                    iCount++;
                else
                    s--;
            }
            if (iCount)
            {
                x += iCount; // skip these
                iCount = 0;
            }
        }
    }
    else
    {
        s = pDraw->pPixels;
        // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
        for (x = 0; x < iWidth; x++)
            usTemp[x] = usPalette[*s++];
        TFTDraw(pDraw->iX, y, iWidth, 1, (uint16_t *)usTemp);
    }
} /* GIFDraw() */

lv_indev_t *ts_cst816s_indev;
void lvgl_hal_init(void)
{
#ifdef CST816S_SUPPORT
    // touch screen
    ts_cst816s.begin();
    ts_cst816s.setReportRate(2);    // 20ms
    ts_cst816s.setReportMode(0x60); // touch + gesture generated interrupt
    ts_cst816s.setMotionMask(0);    // disable motion
    ts_cst816s.setAutoRst(0);       // disable auto reset
    ts_cst816s.setLongRst(0);       // disable long press reset
    ts_cst816s.setDisAutoSleep(1);  // disable auto sleep
#endif

    // display
    tft_gc9a01.begin();
    tft_gc9a01.invertDisplay(1);
    // tft_gc9a01.setRotation(2);

    tft_gc9a01.fillScreen(TFT_BLACK);
    tft_backlight_init();
    delay(50);
    tft_set_backlight(8);

    // tft_fps_test();
    // gif.begin(LITTLE_ENDIAN_PIXELS);
    lv_log_register_print_cb(my_log_cb);
    lv_init();
    lv_fs_littlefs_init();

    // Serial.printf("GIF_IMAGE: %p\n", GIF_IMAGE);

    // Serial.printf("sizeof(GIF_IMAGE) %d\n", sizeof(GIF_IMAGE));

    // tft_gc9a01.fillScreen(TFT_BLACK);

    // Put your main code here, to run repeatedly:
    // Serial.println("gif open");
    // if (gif.open("L:/welcome.gif", gif_open_file1, gif_close_file1, gif_read_file1, gif_seek_file1, GIFDraw))
    // {
    //     Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    //     // tft_gc9a01.startWrite(); // The TFT chip select is locked low
    //     // while (gif.playFrame(true, NULL))
    //     // {
    //     //     yield();
    //     // }
    //     // gif.close();
    //     // tft_gc9a01.endWrite(); // Release TFT chip select for other SPI devices
    // }
    // must static
    static lv_disp_draw_buf_t draw_buf;
    Serial.println("heap_caps_malloc");
    Serial.print("free heap size: ");
    Serial.println(esp_get_free_heap_size());
    Serial.print("LV_MEM_CUSTOM: ");
    Serial.println(LV_MEM_CUSTOM);
    // Serial.print("LV_MEM_SIZE: ");
    // Serial.println(LV_MEM_SIZE);

    Serial.println("lv init done");
    Serial.print("free heap size: ");
    Serial.println(esp_get_free_heap_size());
    lv_disp_draw_buf_init(&draw_buf, lv_disp_buf_p, NULL, TFT_WIDTH * 24);
    Serial.println("lv_disp_draw_buf_init done");
    Serial.print("free heap size: ");
    Serial.println(esp_get_free_heap_size());
    /*Initialize the display*/
    // must static
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = usr_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    // lv_disp_set_rotation(NULL, LV_DISP_ROT_180);
    Serial.println("lv_disp_drv_register done");
    Serial.print("free heap size: ");
    Serial.println(esp_get_free_heap_size());

#ifdef CST816S_SUPPORT
    /* touch screen */
    // must static
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.gesture_limit = 1;
    indev_drv.gesture_min_velocity = 1;
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = usr_touchpad_read;  /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    ts_cst816s_indev = lv_indev_drv_register(&indev_drv);
#endif

    /* set background color to black (default white) */
    lv_obj_set_style_bg_color(lv_scr_act(), LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
}
