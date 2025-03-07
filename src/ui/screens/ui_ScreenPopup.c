// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: Knomi2

#include "../ui.h"

void ui_ScreenPopup_screen_init(void)
{
    ui_ScreenPopup = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPopup, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_ScreenPopup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ScreenPopup, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_img_popup = lv_img_create(ui_ScreenPopup);
    lv_img_set_src(ui_img_popup, &ui_img_popup_png);
    lv_obj_set_width(ui_img_popup, 175);
    lv_obj_set_height(ui_img_popup, 130);
    lv_obj_set_x(ui_img_popup, 0);
    lv_obj_set_y(ui_img_popup, 0);
    lv_obj_set_align(ui_img_popup, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_img_popup, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_img_popup, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_textarea_popup = lv_textarea_create(ui_img_popup);
    lv_obj_set_width(ui_textarea_popup, 135);
    lv_obj_set_height(ui_textarea_popup, 90);
    lv_obj_set_x(ui_textarea_popup, 0);
    lv_obj_set_y(ui_textarea_popup, -5);
    lv_obj_set_align(ui_textarea_popup, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_textarea_popup,
                      LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE |
                      LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_set_style_text_color(ui_textarea_popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_textarea_popup, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_textarea_popup, &ui_font_InterSemiBold14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_textarea_popup, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_textarea_popup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_textarea_popup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_btn_popup_ok = lv_imgbtn_create(ui_ScreenPopup);
    // lv_imgbtn_set_src(ui_btn_popup_ok, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_dialog_png, NULL);
    // lv_obj_set_width(ui_btn_popup_ok, 56);
    // lv_obj_set_height(ui_btn_popup_ok, 56);
    // lv_obj_set_x(ui_btn_popup_ok, 0);
    // lv_obj_set_y(ui_btn_popup_ok, -10);
    // lv_obj_set_align(ui_btn_popup_ok, LV_ALIGN_BOTTOM_MID);

    // ui_img_popup_ok = lv_img_create(ui_btn_popup_ok);
    // lv_img_set_src(ui_img_popup_ok, &ui_img_img_ok_png);
    // lv_obj_set_width(ui_img_popup_ok, 24);
    // lv_obj_set_height(ui_img_popup_ok, 17);
    // lv_obj_set_align(ui_img_popup_ok, LV_ALIGN_CENTER);
    // lv_obj_add_flag(ui_img_popup_ok, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    // lv_obj_clear_flag(ui_img_popup_ok, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    // lv_obj_add_event_cb(ui_btn_popup_ok, ui_event_btn_popup_ok, LV_EVENT_ALL, NULL);

}
