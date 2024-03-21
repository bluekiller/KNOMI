/**
 * @file lv_ag_gif.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ag_gif.h"
#include "AnimatedGIF.h"
#include <esp_heap_caps.h>
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_ag_gif_class
u_int8_t *gif_buf;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ag_gif_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_ag_gif_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void next_frame_task_cb(lv_timer_t *t);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_ag_gif_class = {
    .base_class = &lv_img_class,
    .constructor_cb = lv_ag_gif_constructor,
    .destructor_cb = lv_ag_gif_destructor,
    .user_data = NULL,                       // user_data (if not used)
    .event_cb = NULL,                        // event_cb (if not used)
    .width_def = LV_SIZE_CONTENT,            // width_def
    .height_def = LV_SIZE_CONTENT,           // height_def
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE, // editable (if not used)
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_ag_gif_t)};
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void *gif_open_file(const char *szFilename, int32_t *pFileSize)
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

int32_t gif_read_file(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
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

void gif_close_file(void *pHandle)
{
    lv_fs_file_t *file = (lv_fs_file_t *)pHandle;
    if (file != NULL)
    {
        lv_fs_close(file);
        lv_mem_free(file);
    }
}

void GIFDraw1(GIFDRAW *pDraw)
{
    u_int8_t *buffer = (u_int8_t *)pDraw->pUser;
    if (LV_COLOR_DEPTH == 16)
    {
        uint8_t c, *s, *pEnd;
        uint16_t *d, *pPal = (uint16_t *)pDraw->pPalette;
        // LV_LOG_INFO("buffer@%p", buffer);
        s = pDraw->pPixels;
        pEnd = s + pDraw->iWidth; // faster way to loop over the source pixels - eliminates a counter variable
        d = (uint16_t *)buffer;   // dest pointer to the cooked pixels
        d = &d[pDraw->iX + (pDraw->iY + pDraw->y) * pDraw->iCanvasWidth];
        // LV_LOG_INFO("draw@%p", d);
        // LV_LOG_INFO("ucHasTransparency: %d, DisposalMethod: %d", pDraw->ucHasTransparency, pDraw->ucDisposalMethod);
        // Apply the new pixels to the main image
        if (pDraw->ucHasTransparency)
        { // if transparency used
            uint8_t ucTransparent = pDraw->ucTransparent;
            if (pDraw->ucDisposalMethod == 2)
            { // restore to background color
                uint16_t u16BG = pPal[pDraw->ucBackground];
                while (s < pEnd)
                {
                    c = *s++;
                    if (c != ucTransparent)
                    {
                        *d++ = pPal[c];
                    }
                    else
                    {
                        *d++ = u16BG; // transparent pixel is restored to background color
                    }
                }
            }
            else
            { // no disposal, just write non-transparent pixels
                while (s < pEnd)
                {
                    c = *s++;
                    if (c != ucTransparent)
                    {
                        *d++ = pPal[c];
                    }
                    else
                    {
                        *d++;
                    }
                }
            }
        }
        else
        { // convert all pixels through the palette without transparency
            while (s < pEnd)
            {
                c = *s++;       // just write the new opaque pixels over the old
                *d++ = pPal[c]; // and create the cooked pixels through the palette
            }
        }
    }
}

int32_t gif_seek_file(GIFFILE *pFile, int32_t iPosition)
{
    // Serial.printf("gif_seek_file");
    lv_fs_file_t *file = (lv_fs_file_t *)pFile->fHandle;
    lv_fs_seek(file, iPosition, LV_FS_SEEK_SET);
    lv_fs_tell(file, (uint32_t *)&pFile->iPos);
    return pFile->iPos;
}

lv_obj_t *lv_ag_gif_create(lv_obj_t *parent)
{

    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_ag_gif_set_framebuf(lv_obj_t *obj, uint8_t *buf)
{
    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;
    LV_LOG_INFO("buf: %p", buf);
    gifobj->buf = buf;
}

void lv_ag_gif_set_src(lv_obj_t *obj, const void *src)
{
    LV_LOG_INFO("lv_ag_gif_set_src %s", src);
    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;

    /*Close previous gif if any*/
    if (gifobj->gif)
    {
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        LV_LOG_INFO("");
        lv_img_cache_invalidate_src(&gifobj->imgdsc);
        LV_LOG_INFO("gifobj->gif->close()");
        gifobj->gif->close();
        // delete gifobj->gif;
        // gifobj->gif = NULL;
        gifobj->imgdsc.data = NULL;
        // lv_mem_free(gifobj->buf);
        LV_LOG_INFO("old closed");
        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
        LV_LOG_INFO("");
    }

    int open_result;
    if (lv_img_src_get_type(src) == LV_IMG_SRC_VARIABLE)
    {
        if (!gifobj->gif)
        {
            AnimatedGIF *animatedGIF = new AnimatedGIF();
            gifobj->gif = animatedGIF;
        }
        const lv_img_dsc_t *img_dsc = (lv_img_dsc_t *)src;
        gifobj->gif->begin(LITTLE_ENDIAN_PIXELS);
        LV_LOG_INFO("begin");
        open_result = gifobj->gif->open((uint8_t *)img_dsc->data, (int)img_dsc->data_size, NULL);
    }
    else if (lv_img_src_get_type(src) == LV_IMG_SRC_FILE)
    {
        if (!gifobj->gif)
        {
            LV_LOG_INFO("new AnimatedGIF");
            AnimatedGIF *animatedGIF = new AnimatedGIF();
            LV_LOG_INFO("new AnimatedGIF done");
            gifobj->gif = animatedGIF;
            LV_LOG_INFO("set pointer");
        }
        gifobj->gif->begin(LITTLE_ENDIAN_PIXELS);
        LV_LOG_INFO("begin");
        open_result = gifobj->gif->open((char *)src, gif_open_file, gif_close_file, gif_read_file, gif_seek_file, GIFDraw1);
        LV_LOG_INFO("gifobj->gif->open %s", src);
    }
    if (open_result == 0)
    {
        LV_LOG_WARN("Could't load the source");
        return;
    }
    // gifobj->gif->setFrameBuf(gifobj->buf);
    // LV_LOG_INFO("setFrameBuf %p", gifobj->buf);
    // gifobj->gif->setDrawType(GIF_DRAW_RAW);
    LV_LOG_INFO("setDrawType");
    // gifobj->buf = (u_int8_t *)lv_mem_alloc(gifobj->gif->getCanvasHeight() * gifobj->gif->getCanvasWidth() * 2);
    gifobj->buf = gif_buf;
    if (!gifobj->buf)
    {
        LV_LOG_WARN("Could't allocate memory");
        return;
    }
    gifobj->gif->playFrame(false, (int *)&gifobj->delay, gifobj->buf);
    LV_LOG_INFO("playFrame");
    gifobj->imgdsc.header.always_zero = 0;
    gifobj->imgdsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    gifobj->imgdsc.header.h = gifobj->gif->getCanvasHeight();
    gifobj->imgdsc.header.w = gifobj->gif->getCanvasWidth();
    gifobj->imgdsc.data = gifobj->buf;
    LV_LOG_INFO("imgdsc.data = gifobj->gif->getFrameBuf()");
    gifobj->last_call = lv_tick_get();

    LV_LOG_INFO("set image data %p", gifobj->imgdsc.data);
    lv_img_set_src(obj, &gifobj->imgdsc);

    lv_timer_resume(gifobj->timer);
    lv_timer_reset(gifobj->timer);

    next_frame_task_cb(gifobj->timer);
}

void lv_ag_gif_restart(lv_obj_t *obj)
{
    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;
    // gd_rewind(gifobj->gif);
    lv_timer_resume(gifobj->timer);
    lv_timer_reset(gifobj->timer);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ag_gif_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);

    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;

    gifobj->gif = NULL;
    gifobj->timer = lv_timer_create(next_frame_task_cb, 10, obj);
    lv_timer_pause(gifobj->timer);
}

static void lv_ag_gif_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;
    lv_img_cache_invalidate_src(&gifobj->imgdsc);
    if (gifobj->gif)
    {
        gifobj->gif->close();
        delete gifobj->gif;
        gifobj->gif = NULL;
    }
    // if (gifobj->buf)
    //     lv_mem_free(gifobj->buf);
    lv_timer_del(gifobj->timer);
}

static void next_frame_task_cb(lv_timer_t *t)
{
    // LV_LOG_INFO("next_frame_task_cb");
    lv_obj_t *obj = (lv_obj_t *)t->user_data;
    lv_ag_gif_t *gifobj = (lv_ag_gif_t *)obj;
    uint32_t elaps = lv_tick_elaps(gifobj->last_call);
    if (elaps < gifobj->delay)
        return;

    gifobj->last_call = lv_tick_get();

    int delayMilliseconds;
    int has_next = gifobj->gif->playFrame(false, &delayMilliseconds, gifobj->buf);
    // LV_LOG_INFO("playFrame result: %d", has_next);
    lv_img_cache_invalidate_src(lv_img_get_src(obj));
    lv_obj_invalidate(obj);
    if (has_next == 0)
    {
        /*It was the last repeat*/
        // lv_res_t res = lv_event_send(obj, LV_EVENT_READY, NULL);
        // lv_timer_pause(t);
        // if (res != LV_FS_RES_OK)
        //     return;
    }
    // printf("Hexadecimal representation of data@%p: ", gifobj->buf);
    // for (int i = 0; i < gifobj->gif->getCanvasHeight() * gifobj->gif->getCanvasWidth() * 3; i++)
    // {
    //     printf("%02X ", gifobj->buf[i]);
    // }
    // printf("\n");
    // gd_render_frame(gifobj->gif, (uint8_t *)gifobj->imgdsc.data);
}
