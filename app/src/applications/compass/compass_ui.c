#include <compass/compass_ui.h>
#include <lvgl.h>
#include "math.h"


static lv_obj_t *root_page = NULL;

static lv_obj_t *compass_panel;
static lv_obj_t *compass_img;
static lv_obj_t *compass_label;

#define M_PI    3.1415

typedef struct CompassLabel
{
    lv_obj_t *label;
    char text[9];
} CompassLabel_t;

typedef struct CompassCardinal
{
    lv_obj_t *label;
    lv_point_t position;
    lv_point_t offset;
} CompassCardinal_t;

/*typedef struct CompassGraduation
{
    lv_obj_t *meter;
    lv_meter_scale_t *scale;
} CompassGraduation_t;*/

/* Compass screen context object */
typedef struct CompassScreen
{
    CompassCardinal_t northCardinal;
    CompassCardinal_t eastCardinal;
    CompassCardinal_t southCardinal;
    CompassCardinal_t westCardinal;

    //CompassGraduation_t compassGraduation;

    lv_obj_t *northMarker;
    lv_obj_t *display;

    CompassLabel_t compassAzimuth;
    CompassLabel_t compassTemperature;
} CompassScreen_t;

CompassScreen_t  compassScreenS;
CompassScreen_t * compassScreen = &compassScreenS;


static const char *cardinals[] =
{
    "N",
    "NE",
    "E",
    "SE",
    "S",
    "SW",
    "W",
    "NW"
};

static void create_cardinal(CompassCardinal_t *cardidanl, const char *cardinalText, lv_color_t textColor, lv_coord_t x, lv_coord_t y, lv_obj_t *parent)
{
    cardidanl->label = lv_label_create(parent);
    lv_label_set_text_static(cardidanl->label, cardinalText);
    lv_obj_set_style_text_font(cardidanl->label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(cardidanl->label, textColor, LV_PART_MAIN);
    lv_obj_update_layout(cardidanl->label); //Needed to compute the height and width of the label, if not called, lv_obj_get_width and height return 0.
    //Let's compute the offset to move the origin of the letter to it's center :
    cardidanl->offset.x = lv_obj_get_width(cardidanl->label)/2;
    cardidanl->offset.y = lv_obj_get_height(cardidanl->label)/2;

    cardidanl->position.x = x - cardidanl->offset.x;
    cardidanl->position.y = y - cardidanl->offset.y;
    lv_obj_set_pos(cardidanl->label, cardidanl->position.x, cardidanl->position.y);
}

static void rotate_cardinal(CompassCardinal_t *cardinal, uint16_t azimuth)
{
    lv_coord_t rotated_x, rotated_y;
    float azimuth_in_rads = (float)azimuth*M_PI/180.0;

    rotated_x = (float)(cardinal->position.x-120+cardinal->offset.x)*cos(azimuth_in_rads) + (float)(cardinal->position.y-120+cardinal->offset.y)*sin(azimuth_in_rads) + 120-cardinal->offset.x;
    rotated_y = (float)(cardinal->position.y-120+cardinal->offset.y)*cos(azimuth_in_rads) - (float)(cardinal->position.x-120+cardinal->offset.x)*sin(azimuth_in_rads) + 120-cardinal->offset.y;
    lv_obj_set_pos(cardinal->label, rotated_x, rotated_y);
}


static void create_ui(lv_obj_t *root)
{
    memset(compassScreen, 0, sizeof(CompassScreen_t));
    strcpy(compassScreen->compassAzimuth.text, "0° N");

    // Create the root container


    //We declare all the needed assets by the compass screen:
    LV_IMG_DECLARE(compass_marker)
    LV_IMG_DECLARE(compass_marker_north)

    //We create our parent screen :
    if(compassScreen->display)
    {
        LV_LOG_ERROR("display should be NULL here !");
        lv_obj_del(compassScreen->display);
        compassScreen->display = NULL;
    }

    compassScreen->display = lv_obj_create(root);
    // Create the root container
    // Remove the default border
    lv_obj_set_style_border_width(compassScreen->display, 0, LV_PART_MAIN);
    // Make root container fill the screen
    lv_obj_set_size(compassScreen->display, LV_PCT(100), LV_PCT(100));
    // Don't want it to be scollable. Putting anything close the edges
    // then LVGL automatically makes the page scrollable and shows a scroll bar.
    // Does not loog very good on the round display.
    lv_obj_set_scrollbar_mode(compassScreen->display, LV_SCROLLBAR_MODE_OFF);
    /*
        // Remove the default border
    lv_obj_set_style_border_width(root_page, 0, LV_PART_MAIN);

    // Make root container fill the screen
    lv_obj_set_size(root_page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scrollbar_mode(root_page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(root_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(compassScreen->display, lv_color_white(), LV_PART_MAIN);
    */

    //Let's add some arcs
    lv_obj_t *arc = lv_arc_create(compassScreen->display);
    lv_arc_set_angles(arc, 0, 360);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 0, LV_PART_MAIN);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(arc, 168, 168);
    lv_obj_center(arc);

    arc = lv_arc_create(compassScreen->display);
    lv_arc_set_angles(arc, 0, 360);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 0, LV_PART_MAIN);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_size(arc, 210, 210);
    lv_obj_center(arc);

    //Marker images are created here
    lv_obj_t *compassMarker = lv_img_create(compassScreen->display);
    lv_img_set_src(compassMarker, &compass_marker);
    lv_obj_set_pos(compassMarker, 113, 0);

    //Azimuth label is created here
    if(compassScreen->compassAzimuth.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->compassAzimuth.label);
        compassScreen->compassAzimuth.label = NULL;
    }
    compassScreen->compassAzimuth.label = lv_label_create(compassScreen->display);
    lv_label_set_text_static(compassScreen->compassAzimuth.label, compassScreen->compassAzimuth.text);
    lv_obj_set_style_text_font(compassScreen->compassAzimuth.label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_center(compassScreen->compassAzimuth.label);

    //Temperature label is created here
    if(compassScreen->compassTemperature.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->compassTemperature.label);
        compassScreen->compassTemperature.label = NULL;
    }
    compassScreen->compassTemperature.label = lv_label_create(compassScreen->display);
    lv_label_set_text_static(compassScreen->compassTemperature.label, compassScreen->compassTemperature.text);
    lv_obj_set_style_text_font(compassScreen->compassTemperature.label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(compassScreen->compassTemperature.label, LV_ALIGN_CENTER, 0, -22);

    //Add some graduation : ( was nice but is too CPU intensive :( )
    /*if(compassScreen->compassGraduation.meter)
    {
        LV_LOG_ERROR("meter should be NULL here !");
        lv_obj_del(compassScreen->compassGraduation.meter);
        compassScreen->compassGraduation.meter = NULL;
    }
    compassScreen->compassGraduation.meter = lv_meter_create(compassScreen->display);
    lv_obj_remove_style(compassScreen->compassGraduation.meter, NULL, LV_PART_INDICATOR);
    lv_obj_remove_style(compassScreen->compassGraduation.meter, NULL, LV_PART_MAIN);
    lv_obj_set_size(compassScreen->compassGraduation.meter, 204, 204);
    lv_obj_center(compassScreen->compassGraduation.meter);

    compassScreen->compassGraduation.scale = lv_meter_add_scale(compassScreen->compassGraduation.meter);
    lv_meter_set_scale_ticks(compassScreen->compassGraduation.meter, compassScreen->compassGraduation.scale, 12, 1, 0, lv_color_black());
    lv_meter_set_scale_major_ticks(compassScreen->compassGraduation.meter, compassScreen->compassGraduation.scale, 1, 1, 0, lv_color_black(), 11);
    lv_meter_set_scale_range(compassScreen->compassGraduation.meter, compassScreen->compassGraduation.scale, 0, 330, 330, -90);*/

    if(compassScreen->northMarker)
    {
        LV_LOG_ERROR("img should be NULL here !");
        lv_obj_del(compassScreen->northMarker);
        compassScreen->northMarker = NULL;
    }
    compassScreen->northMarker = lv_img_create(compassScreen->display);
    lv_img_set_src(compassScreen->northMarker, &compass_marker_north);
    lv_obj_set_pos(compassScreen->northMarker, 113, 18);
    lv_img_set_pivot(compassScreen->northMarker, 7, 101);

    //Cardinal labels are created here
    if(compassScreen->northCardinal.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->northCardinal.label);
        compassScreen->northCardinal.label = NULL;
    }
    create_cardinal(&compassScreen->northCardinal, cardinals[0], lv_palette_main(LV_PALETTE_RED), 120, 53, compassScreen->display);

    if(compassScreen->eastCardinal.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->eastCardinal.label);
        compassScreen->eastCardinal.label = NULL;
    }
    create_cardinal(&compassScreen->eastCardinal, cardinals[2], lv_color_black(), 187, 120, compassScreen->display);

    if(compassScreen->southCardinal.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->southCardinal.label);
        compassScreen->southCardinal.label = NULL;
    }
    create_cardinal(&compassScreen->southCardinal, cardinals[4], lv_color_black(), 120, 187, compassScreen->display);

    if(compassScreen->westCardinal.label)
    {
        LV_LOG_ERROR("label should be NULL here !");
        lv_obj_del(compassScreen->westCardinal.label);
        compassScreen->westCardinal.label = NULL;
    }
    create_cardinal(&compassScreen->westCardinal, cardinals[6], lv_color_black(), 53, 120, compassScreen->display);

    //We register the event callback to handle gestures
    //lv_obj_add_event_cb(compassScreen->display, &(gesture_event_cb), LV_EVENT_GESTURE, compassScreen);
    //We register the event callback to handle the cleanup
    //lv_obj_add_event_cb(compassScreen->display, &(cleanup_event_cb), LV_EVENT_DELETE, compassScreen);
}

void compass_ui_show(lv_obj_t *root)
{
    create_ui(root);
}

void compass_ui_remove(void)
{
    lv_obj_del(compassScreen->display);
    if(!compassScreen)
    {
        LV_LOG_ERROR("NULL pointer given !");
        return;
    }

    compassScreen->display                  = NULL;
    compassScreen->compassAzimuth.label     = NULL;
    compassScreen->compassTemperature.label = NULL;
    compassScreen->northCardinal.label      = NULL;
    compassScreen->eastCardinal.label       = NULL;
    compassScreen->southCardinal.label      = NULL;
    compassScreen->westCardinal.label       = NULL;
    compassScreen->northMarker              = NULL;
    //compassScreen->compassGraduation.meter  = NULL;
    //compassScreen->compassGraduation.scale  = NULL;
}

void compass_ui_set_heading(int azimuth)
{
    if(!compassScreen)
    {
        LV_LOG_ERROR("NULL pointer given !");
        return;
    }

    //Mandatory, if the screen is not displayed anymore, we should still be able to call this function !
    if(!compassScreen->display) return;

    //We make sure the azimuth is in the right range ie 0 to 359
    azimuth %= 360;
    uint16_t azimuthAdjusted = 359 - azimuth;

    uint8_t index;

    if(azimuthAdjusted >= 23 && azimuthAdjusted <= 67)
        index = 1;//NE
    else if(azimuthAdjusted >= 68 && azimuthAdjusted <= 112)
        index = 2;//E
    else if(azimuthAdjusted >= 113 && azimuthAdjusted <= 157)
        index = 3;//SE
    else if(azimuthAdjusted >= 158 && azimuthAdjusted <= 202)
        index = 4;//S
    else if(azimuthAdjusted >= 203 && azimuthAdjusted <= 247)
        index = 5;//SW
    else if(azimuthAdjusted >= 248 && azimuthAdjusted <= 292)
        index = 6;//W
    else if(azimuthAdjusted >= 293 && azimuthAdjusted <= 337)
        index = 7;//NW
    else
        index = 0;//N

    //Update the center label
    sprintf(compassScreen->compassAzimuth.text, "%u° %s", azimuthAdjusted, cardinals[index]);
    lv_label_set_text_static(compassScreen->compassAzimuth.label, compassScreen->compassAzimuth.text);

    //Rotate each cardinal with the current azimuth
    rotate_cardinal(&compassScreen->northCardinal, azimuth);
#if HARDWARE_PLATFORM == SMART_WATCH_BREADBOARD
    lv_img_set_angle(compassScreen->northMarker, azimuth*10);
#elif HARDWARE_PLATFORM == SMART_WATCH_PCB
    lv_img_set_angle(compassScreen->northMarker, -azimuth*10);
#endif
    //lv_meter_set_scale_range(compassScreen->compassGraduation.meter, compassScreen->compassGraduation.scale, 0, 330, 330, azimuth-90);
    
    rotate_cardinal(&compassScreen->eastCardinal, azimuth);
    rotate_cardinal(&compassScreen->southCardinal, azimuth);
    rotate_cardinal(&compassScreen->westCardinal, azimuth);
}



void compass_screen_set_temperature(CompassScreen_t * const compassScreen, float temperature)
{
    if(!compassScreen)
    {
        LV_LOG_ERROR("NULL pointer given !");
        return;
    }

    //Mandatory, if the screen is not displayed anymore, we should still be able to call this function !
    if(!compassScreen->display) return;

    //Update the temperature label
    sprintf(compassScreen->compassTemperature.text, "%.2f°C", temperature);
    lv_label_set_text_static(compassScreen->compassTemperature.label, compassScreen->compassTemperature.text);
}