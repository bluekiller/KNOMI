#include <stdio.h> // sprintf
#include <Arduino.h>

#include "lvgl_hal.h"
#include "lvgl_usr.h"
#include "ui/ui.h"
#include "moonraker.h"
#include "ui_overlay/lv_overlay.h"
#include <AnimatedGIF.h>

extern lv_color_t *lv_disp_buf_p;
extern u_int8_t *gif_buf;

const char *extrude_len = {
    EXTRUDE_MM_0_LABEL "\n" EXTRUDE_MM_1_LABEL "\n" EXTRUDE_MM_2_LABEL "\n" EXTRUDE_MM_3_LABEL "\n" EXTRUDE_MM_4_LABEL};
const char *EXTRUDE_MM_S = {
    EXTRUDE_MM_S_0_LABEL "\n" EXTRUDE_MM_S_1_LABEL "\n" EXTRUDE_MM_S_2_LABEL "\n" EXTRUDE_MM_S_3_LABEL "\n" EXTRUDE_MM_S_4_LABEL};

/****************** lvgl ui call function ******************/
//
// void lv_tft_set_backlight(lv_event_t *e)
// {
//     int32_t light = lv_slider_get_value(ui_slider_backlight);
//     tft_set_backlight(light);
// }

// extruder speed
// void lv_btn_set_extrude(lv_event_t *e)
// {
//     // Initialize parameter values from roller settings
//     char roller_str[10];
//     lv_roller_get_selected_str(ui_roller_set_extrude_length, roller_str, sizeof(roller_str));
//     lv_label_set_text(ui_label_extruder_length, roller_str);
//     uint32_t sel = lv_roller_get_selected(ui_roller_set_extrude_length);
//     lv_obj_set_user_data(ui_label_extruder_length, (void *)sel);
//     lv_roller_get_selected_str(ui_roller_set_extrude_speed, roller_str, sizeof(roller_str));
//     lv_label_set_text(ui_label_extruder_speed, roller_str);
//     sel = lv_roller_get_selected(ui_roller_set_extrude_speed);
//     lv_obj_set_user_data(ui_label_extruder_speed, (void *)sel);
// }

// set extruder roller
// void lv_roller_set_extrude(lv_event_t *e)
// {
//     // Initialize parameter values from roller settings
//     uint32_t sel = (uint32_t)lv_obj_get_user_data(ui_label_extruder_length);
//     lv_roller_set_selected(ui_roller_set_extrude_length, sel, LV_ANIM_OFF);

//     sel = (uint32_t)lv_obj_get_user_data(ui_label_extruder_speed);
//     lv_roller_set_selected(ui_roller_set_extrude_speed, sel, LV_ANIM_OFF);
// }
/***********************************************************/

void lv_popup_warning(const char *warning, bool clickable);
void lv_popup_remove(lv_event_t *e);

// lvgl ui
void lvgl_ui_task(void *parameter)
{
    void *buffer = lv_mem_alloc(TFT_WIDTH * 24 * sizeof(lv_color_t) + 150 * 150 * 2);
    lv_disp_buf_p = static_cast<lv_color_t *>(buffer);
    if (lv_disp_buf_p == nullptr)
        Serial.println("lv_port_disp_init malloc failed!");
    gif_buf = (u_int8_t *)buffer + TFT_WIDTH * 24 * sizeof(lv_color_t);
    lvgl_hal_init();
    // lv_btn_init();
    ui_init();
    Serial.println("ui_init_done");

    // #ifndef LIS2DW_SUPPORT
    // progress in center if no lis2dw accelerometer data to display
    //     // delete unused accelerometer data
    //     lv_obj_del(ui_slider_printing_acc_x);
    //     lv_obj_del(ui_slider_printing_acc_y);
    //     lv_obj_del(ui_slider_printing_acc_z);
    //     lv_obj_del(ui_label_printing_acc_x);
    //     lv_obj_del(ui_label_printing_acc_y);
    //     lv_obj_del(ui_label_printing_acc_z);
    // #endif

    // Add all button style
    // lv_btn_add_style();

    // Set theme color
    // lv_theme_color_style();
    Serial.println("lv_theme_color_style done");

    for (;;)
    {
        // lvgl task, must run in loop first.
        lv_timer_handler();

        wifi_status_t status = wifi_get_connect_status();

        lv_loop_wifi_change_screen(status);

        if (status == WIFI_STATUS_CONNECTED)
        {

            lv_loop_popup_screen();
            // lv_loop_set_temp_screen();

            if (!moonraker.unconnected && !moonraker.unready)
            {
                if (moonraker.data_unlock)
                {
                    lv_loop_moonraker_change_screen();
                }
                lv_loop_moonraker_change_screen_value();
            }
        }

        // lv_loop_auto_idle(status);
        // lv_loop_btn_event();

        delay(5);
    }
}
