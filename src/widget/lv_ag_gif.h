/**
 * @file lv_gif.h
 *
 */

#ifndef LV_AG_GIF_H
#define LV_AG_GIF_H



/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "AnimatedGIF.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_img_t img;
    AnimatedGIF *gif;
    lv_timer_t * timer;
    lv_img_dsc_t imgdsc;
    uint32_t last_call;
    uint32_t delay;
    uint8_t* buf;
} lv_ag_gif_t;

extern const lv_obj_class_t lv_ag_gif_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_obj_t * lv_ag_gif_create(lv_obj_t * parent);
void lv_ag_gif_set_src(lv_obj_t * obj, const void * src);
void lv_ag_gif_restart(lv_obj_t * gif);
void lv_ag_gif_set_framebuf(lv_obj_t *obj, uint8_t *buf);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GIF*/



