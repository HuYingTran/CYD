#include <Arduino.h>

#include <lvgl.h>                // LVGL 9
#include <TFT_eSPI.h>            // TFT_eSPI
#include <XPT2046_Touchscreen.h> // XPT2046
#include "ui/ui.h"           // UI export từ EEZ Studio

// ================== CONFIG ================== //
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Chân kết nối XPT2046
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Bộ đệm vẽ (10% màn hình, mỗi pixel = lv_color_t)
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

SPIClass touchscreenSPI = SPIClass(SPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Calibration
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700;
uint16_t touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

// ================== LVGL Display Flush ================== //
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// ================== LVGL Input (Touch) ================== //
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 0, SCREEN_WIDTH);
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 0, SCREEN_HEIGHT);
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ================== SETUP ================== //
void setup()
{
  Serial.begin(115200);
  Serial.println("LVGL9 + TFT_eSPI + XPT2046 + EEZ Studio UI");

  // TFT init
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Touchscreen init
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(3);

  // LVGL init
  lv_init();

  // --- Allocate buffer ---
  static lv_color_t *buf1 = (lv_color_t *)malloc(DRAW_BUF_SIZE * sizeof(lv_color_t));

  // --- Create display ---
  lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf1, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // --- Input device ---
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // --- Load EEZ UI ---
  ui_init();
}

// ================== LOOP ================== //
uint32_t lastTick = 0;
void loop()
{
  uint32_t now = millis();
  lv_tick_inc(now - lastTick);
  lastTick = now;

  lv_timer_handler();
  delay(5);
}