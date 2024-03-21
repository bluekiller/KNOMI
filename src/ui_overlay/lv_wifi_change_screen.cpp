#include "ui/ui.h"
#include "knomi.h"
#include "lv_overlay.h"

static wifi_status_t lv_wifi_status = WIFI_STATUS_INIT;

static void lv_goto_wifi_screen(wifi_status_t state)
{
    if (lv_wifi_status == state)
    {
        return;
    }

    static lv_obj_t *running_menu = NULL;
    if (lv_wifi_status == WIFI_STATUS_CONNECTED)
    {
        running_menu = lv_scr_act();
    }
    switch (state)
    {
    case WIFI_STATUS_ERROR:
        _ui_screen_change(&ui_ScreenWelcome, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_ScreenWelcome_screen_init);
        _ui_screen_delete(&ui_ScreenWIFIConnecting);
        _ui_screen_delete(&ui_ScreenWIFIDisconnect);
        break;
    case WIFI_STATUS_CONNECTING:
        _ui_screen_change(&ui_ScreenWIFIConnecting, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_ScreenWIFIConnecting_screen_init);
        _ui_screen_delete(&ui_ScreenWelcome);
        _ui_screen_delete(&ui_ScreenWIFIDisconnect);
        break;
    case WIFI_STATUS_CONNECTED:
        if (running_menu)
            _ui_screen_change(&running_menu, LV_SCR_LOAD_ANIM_NONE, 0, 0, NULL);
        else
            _ui_screen_change(&ui_ScreenMainGif, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_ScreenMainGif_screen_init);
        _ui_screen_delete(&ui_ScreenWelcome);
        _ui_screen_delete(&ui_ScreenWIFIConnecting);
        _ui_screen_delete(&ui_ScreenWIFIDisconnect);
        break;
    case WIFI_STATUS_DISCONNECT:
        _ui_screen_change(&ui_ScreenWIFIDisconnect, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_ScreenWIFIDisconnect_screen_init);
        _ui_screen_delete(&ui_ScreenWelcome);
        _ui_screen_delete(&ui_ScreenWIFIConnecting);
        break;
    default:
        break;
    }
    lv_wifi_status = state;
}

void lv_loop_wifi_change_screen(wifi_status_t status)
{
    lv_goto_wifi_screen(status);
}
