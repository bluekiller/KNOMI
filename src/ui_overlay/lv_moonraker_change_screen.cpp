#include "ui/ui.h"
#include "knomi.h"
#include "moonraker.h"
#include "lv_overlay.h"
#include <widget/lv_ag_gif.h>

typedef enum
{
    LV_MOONRAKER_STATE_IDLE = 0,
    LV_MOONRAKER_STATE_HOMING,
    LV_MOONRAKER_STATE_PROBING,
    LV_MOONRAKER_STATE_QGLING,
    LV_MOONRAKER_STATE_NOZZLE_HEATING,
    LV_MOONRAKER_STATE_BED_HEATING,
    LV_MOONRAKER_STATE_PRINTING,
    LV_SCREEN_STATE_INIT,
    LV_SCREEN_STATE_IDLE,
    LV_SCREEN_HEATED,
    LV_SCREEN_PRINT,
    LV_SCREEN_PRINT_OK,
    LV_SCREEN_PRINTED,
    LV_SCREEN_STATE_PLAYING,
} lv_screen_state_t;

lv_obj_t *ui_img_main_gif;
// const char * gif_idle[] = {gif_voron, gif_standby};
const char *gif_idle[] = {gif_voron, gif_standby};

static char string_buffer[8];
static lv_screen_state_t lv_screen_state = LV_MOONRAKER_STATE_IDLE;
static lv_obj_t *ui_ScreenIdle = NULL;
static lv_obj_t *ui_ScreenNow = NULL;

static void lv_goto_busy_screen(lv_obj_t **screen, lv_screen_state_t state, const char *gif, void (*screen_init_func)(void))
{
    // Serial.println("lv_goto_busy_screen");
    if (lv_screen_state == state)
        return;
    lv_screen_state = state;

    //
    if (gif)
    {
        lv_ag_gif_set_src(ui_img_main_gif, gif);
    }
    // if(ui_ScreenMainGif)
    //     lv_obj_clear_flag(ui_ScreenMainGif, LV_OBJ_FLAG_CLICKABLE);

    // backup now screen, cause lv_scr_act() is delayed updates
    if (ui_ScreenNow == NULL)
        ui_ScreenNow = lv_scr_act();
    if (*screen != ui_ScreenNow)
    {
        // backup screen before jump
        Serial.println("backup screen before jump");
        if (ui_ScreenIdle == NULL)
            ui_ScreenIdle = ui_ScreenNow;
        _ui_screen_change(screen, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0, screen_init_func);
        ui_ScreenNow = *screen;
    }
}

static void lv_goto_idle_screen(void)
{
    if (lv_screen_state == LV_MOONRAKER_STATE_IDLE)
        return;
    lv_screen_state = LV_MOONRAKER_STATE_IDLE;

    //
    if (ui_ScreenMainGif)
    {
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        Serial.println();
        lv_ag_gif_set_src(ui_img_main_gif, gif_idle[0]);
        // lv_obj_add_flag(ui_ScreenMainGif, LV_OBJ_FLAG_CLICKABLE);
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        Serial.println();
    }

    // goto the screen backed up before
    ui_ScreenNow = lv_scr_act();
    if (ui_ScreenIdle && (ui_ScreenIdle != ui_ScreenNow))
    {
        _ui_screen_change_auto_del(&ui_ScreenIdle, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 500, 0, NULL);
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        Serial.println();
        _ui_screen_delete_by_obj(ui_ScreenNow, false);
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        Serial.println();
    }
    ui_ScreenIdle = NULL;
    ui_ScreenNow = NULL;
}

#define TEMPERATURE_ERROR_RANGE 2

bool moonraker_nozzle_is_heating(void)
{
    if (moonraker.data.heating_nozzle)
        return true;
    if (moonraker.data.nozzle_actual + TEMPERATURE_ERROR_RANGE < moonraker.data.nozzle_target)
        return true;
    if ((moonraker.data.nozzle_target != 0) && (moonraker.data.nozzle_target + TEMPERATURE_ERROR_RANGE < moonraker.data.nozzle_actual))
        return true;
    return false;
}

bool moonraker_bed_is_heating(void)
{
    if (moonraker.data.heating_bed)
        return true;
    if (moonraker.data.bed_actual + TEMPERATURE_ERROR_RANGE < moonraker.data.bed_target)
        return true;
    if ((moonraker.data.bed_target != 0) && (moonraker.data.bed_target + TEMPERATURE_ERROR_RANGE < moonraker.data.bed_actual))
        return true;
    return false;
}

// screen change according to moonraker status
void lv_loop_moonraker_change_screen(void)
{
    static lv_screen_state_t screen_state = LV_SCREEN_STATE_INIT;
    // if (moonraker.data.printing) {
    //     if (lv_screen_state == 0) {
    //         lv_goto_busy_screen(ui_ScreenPrinting, LV_MOONRAKER_STATE_PRINTING, NULL);
    //         return;
    //     }
    // }
    if (moonraker.data.homing)
    {
        lv_goto_busy_screen(&ui_ScreenMainGif, LV_MOONRAKER_STATE_HOMING, gif_homing, &ui_ScreenMainGif_screen_init);
        return;
    }
    if (moonraker.data.probing)
    {
        lv_goto_busy_screen(&ui_ScreenMainGif, LV_MOONRAKER_STATE_PROBING, gif_probing, &ui_ScreenMainGif_screen_init);
        return;
    }
    if (moonraker.data.qgling)
    {
        lv_goto_busy_screen(&ui_ScreenMainGif, LV_MOONRAKER_STATE_QGLING, gif_qgling, &ui_ScreenMainGif_screen_init);
        return;
    }
    if (moonraker_nozzle_is_heating())
    {
        Serial.println("moonraker_nozzle_is_heating");
        lv_goto_busy_screen(&ui_ScreenHeatingNozzle, LV_MOONRAKER_STATE_NOZZLE_HEATING, NULL, &ui_ScreenHeatingNozzle_screen_init);
        if (moonraker.data.printing)
            screen_state = LV_SCREEN_HEATED;
        return;
    }
    if (moonraker_bed_is_heating())
    {
        Serial.println("moonraker_bed_is_heating");
        lv_goto_busy_screen(&ui_ScreenHeatingBed, LV_MOONRAKER_STATE_BED_HEATING, NULL, &ui_ScreenHeatingBed_screen_init);
        if (moonraker.data.printing)
            screen_state = LV_SCREEN_HEATED;
        return;
    }

    static lv_screen_state_t playing_next_state = LV_SCREEN_STATE_INIT;
    static uint32_t playing_ms = 0;
    static lv_screen_state_t playing_state;
    static const char *playing_img;
    switch (screen_state)
    {
    case LV_SCREEN_STATE_INIT:
        if (moonraker.data.printing)
        {
            if (moonraker.data.bed_actual + TEMPERATURE_ERROR_RANGE > moonraker.data.bed_target &&
                moonraker.data.nozzle_actual + TEMPERATURE_ERROR_RANGE > moonraker.data.nozzle_target)
            {
                screen_state = LV_SCREEN_HEATED;
                return;
            }
        }
        break;
    case LV_SCREEN_STATE_IDLE:
        if (!moonraker.data.printing)
        {
            screen_state = LV_SCREEN_PRINT_OK;
            return;
        }
        break;
    case LV_SCREEN_HEATED:
        playing_state = LV_SCREEN_HEATED;
        playing_img = gif_heated;
        screen_state = LV_SCREEN_STATE_PLAYING;
        playing_next_state = LV_SCREEN_PRINT;
        playing_ms = millis() + 7000;
        return;
    case LV_SCREEN_PRINT:
        lv_goto_busy_screen(&ui_ScreenMainGif, LV_SCREEN_PRINT, gif_print, &ui_ScreenMainGif_screen_init);
        if (moonraker.data.progress >= 1 || !moonraker.data.printing)
            screen_state = LV_SCREEN_STATE_IDLE;
        return;
    case LV_SCREEN_STATE_PLAYING:
        if (playing_ms > millis())
        {
            lv_goto_busy_screen(&ui_ScreenMainGif, playing_state, playing_img, &ui_ScreenMainGif_screen_init);
            return;
        }
        screen_state = playing_next_state;
        return;
    case LV_SCREEN_PRINT_OK:
        playing_state = LV_SCREEN_PRINT_OK;
        playing_img = gif_print_ok;
        screen_state = LV_SCREEN_STATE_PLAYING;
        playing_next_state = LV_SCREEN_PRINTED;
        playing_ms = millis() + 1600;
        return;
    case LV_SCREEN_PRINTED:
        playing_state = LV_SCREEN_PRINTED;
        playing_img = gif_printed;
        screen_state = LV_SCREEN_STATE_PLAYING;
        playing_next_state = LV_SCREEN_STATE_INIT;
        playing_ms = millis() + 7000;
        return;
    }

    // Printing must be lastest, the lowest priority
    // That The status screen(homing, heating, etc.) can occupy this screen
    if (moonraker.data.printing)
    {
        lv_goto_busy_screen(&ui_ScreenPrinting, LV_MOONRAKER_STATE_PRINTING, NULL, &ui_ScreenPrinting_screen_init);
        return;
    }
    // back to previous screen
    lv_goto_idle_screen();

    if (lv_scr_act() == ui_ScreenMainGif && ui_ScreenMainGif != NULL)
    {
        static uint8_t gif_idle_index = 0;
        static uint32_t gif_idle_ms = 0;

        if (gif_idle_ms < millis())
        {
            heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
            Serial.println();

            lv_ag_gif_set_src(ui_img_main_gif, gif_idle[gif_idle_index]);

            heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
            Serial.println();
            gif_idle_index = (gif_idle_index + 1) % ACOUNT(gif_idle);
            gif_idle_ms = millis() + 7000; // 7s
        }
    }
}

// modify all temperature label
void lv_loop_moonraker_change_screen_value(void)
{
    if (lv_scr_act() == ui_ScreenHeatingNozzle && ui_ScreenHeatingNozzle != NULL)
    {
        // nozzle target
        static int16_t nozzle_target;
        // if (nozzle_target != moonraker.data.nozzle_target)
        if (nozzle_target != moonraker.data.nozzle_target)
        {
            nozzle_target = moonraker.data.nozzle_target;
            snprintf(string_buffer, sizeof(string_buffer), "%d℃", nozzle_target);
            // lv_label_set_text(ui_label_extruder_target, string_buffer);
            // lv_label_set_text(ui_label_temp_nozzle_target, string_buffer);
            lv_label_set_text(ui_label_heating_nozzle_target, string_buffer);
        }
        // nozzle actual
        static int16_t nozzle_actual;
        if (nozzle_actual != moonraker.data.nozzle_actual)
        // if (nozzle_actual != moonraker.data.nozzle_actual)
        {
            nozzle_actual = moonraker.data.nozzle_actual;
            snprintf(string_buffer, sizeof(string_buffer), "%d℃", nozzle_actual);
            // lv_label_set_text(ui_label_extruder_actual, string_buffer);
            // lv_label_set_text(ui_label_temp_nozzle_actual, string_buffer);
            lv_label_set_text(ui_label_heating_nozzle_actual, string_buffer);
        }
    }
    if (lv_scr_act() == ui_ScreenHeatingBed && ui_ScreenHeatingBed != NULL)
    {
        // bed target
        static int16_t bed_target;
        if (bed_target != moonraker.data.bed_target)
        // if (moonraker.data.bed_target > 0)
        {
            bed_target = moonraker.data.bed_target;
            snprintf(string_buffer, sizeof(string_buffer), "%d℃", bed_target);
            // lv_label_set_text(ui_label_temp_bed_target, string_buffer);
            lv_label_set_text(ui_label_heating_bed_target, string_buffer);
        }
        // bed actual
        static int16_t bed_actual;
        // if (bed_actual != moonraker.data.bed_actual)
        if (bed_actual != moonraker.data.bed_actual)
        {
            bed_actual = moonraker.data.bed_actual;
            snprintf(string_buffer, sizeof(string_buffer), "%d℃", bed_actual);
            // lv_label_set_text(ui_label_temp_bed_actual, string_buffer);
            lv_label_set_text(ui_label_heating_bed_actual, string_buffer);
        }
    }
    if (lv_scr_act() == ui_ScreenPrinting && ui_ScreenPrinting != NULL)
    {
        // progress
        static uint8_t progress;
        if (progress != moonraker.data.progress)
        {
            progress = moonraker.data.progress;
            lv_arc_set_value(ui_arc_printing_progress, progress);
            snprintf(string_buffer, sizeof(string_buffer), "%d%%", progress);
            lv_label_set_text(ui_label_printing_progress, string_buffer);
        }
        // #ifdef LIS2DW_SUPPORT
        //         // accelerometer
        //         lv_slider_set_value(ui_slider_printing_acc_x, abs(lis2dw12_acc[0]) / 10, LV_ANIM_ON);
        //         lv_slider_set_value(ui_slider_printing_acc_y, abs(lis2dw12_acc[1]) / 10, LV_ANIM_ON);
        //         lv_slider_set_value(ui_slider_printing_acc_z, abs(lis2dw12_acc[2] + 980) / 10, LV_ANIM_ON); // +980 for counteract the value of gravitational acceleration
        // #endif
    }
    if ((moonraker.data.nozzle_target != 0) && (lv_scr_act() == ui_ScreenHeatingNozzle) && (ui_ScreenHeatingNozzle != NULL))
    {
        lv_slider_set_value(ui_slider_heating_nozzle, moonraker.data.nozzle_actual * 100 / moonraker.data.nozzle_target, LV_ANIM_ON);
    }
    if ((moonraker.data.bed_target != 0) && (lv_scr_act() == ui_ScreenHeatingBed) && (ui_ScreenHeatingBed != NULL))
    {
        lv_slider_set_value(ui_slider_heating_bed, moonraker.data.bed_actual * 100 / moonraker.data.bed_target, LV_ANIM_ON);
    }
}
