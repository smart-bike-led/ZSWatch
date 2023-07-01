#include <zds/zds_ui.h>
#include <lvgl.h>

static void on_button_pressed(lv_event_t *e);
static void set_img_rotation(void * obj, int32_t v);

static lv_obj_t *root_page = NULL;
static lv_obj_t *img;
static uint32_t current_rotation = 0;

void zds_ui_show(lv_obj_t *root)
{
    lv_obj_t *float_btn;

    assert(root_page == NULL);

    // Create the root container
    root_page = lv_obj_create(root);
    // Remove the default border
    lv_obj_set_style_border_width(root_page, 0, LV_PART_MAIN);
    // Make root container fill the screen
    lv_obj_set_size(root_page, LV_PCT(100), LV_PCT(100));
    // Don't want it to be scollable. Putting anything close the edges
    // then LVGL automatically makes the page scrollable and shows a scroll bar.
    // Does not loog very good on the round display.
    lv_obj_set_scrollbar_mode(root_page, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_bg_color(root_page, lv_color_hex(0x49936E), LV_PART_MAIN);

    LV_IMG_DECLARE(zephyr_project);

    img = lv_img_create(root_page);
    lv_img_set_src(img, &zephyr_project);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // Add a button so we can easily close the application
    float_btn = lv_btn_create(root_page);
    lv_obj_set_size(float_btn, 50, 50);
    lv_obj_add_flag(float_btn, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(float_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_add_event_cb(float_btn, on_button_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(img, on_button_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_set_style_radius(float_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_img_src(float_btn, LV_SYMBOL_REFRESH, 0);
    lv_obj_set_style_text_font(float_btn, lv_theme_get_font_large(float_btn), 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, set_img_rotation);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&a, 0);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_start(&a);

    lv_group_focus_obj(float_btn);
}

void zds_ui_remove(void)
{
    lv_obj_del(root_page);
    root_page = NULL;
}

void zds_ui_reset_rotation(void)
{
    current_rotation = 0;
    lv_img_set_angle(img, current_rotation);
}

static void on_button_pressed(lv_event_t *e)
{
    current_rotation = (current_rotation + 100) % 3600;
    lv_img_set_angle(img, current_rotation);
}

static void set_img_rotation(void * obj, int32_t v)
{
    lv_img_set_angle(obj, v);
}
