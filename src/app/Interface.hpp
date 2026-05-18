#pragma once

#include <lvgl.h>
#include <app/screenRadio.hpp>
#include <app/screenDlna.hpp>

lv_obj_t *scrRadio = NULL;
lv_obj_t *volumeForeground = NULL; 

//uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf;


// LVGL calls this function to read the touchpad
void lvgl_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (dev.display_is_touched())
    {
        //uint8_t const touch_count = dev.display_touch_count();
        JC4827W543::touch touches[dev.display_touch_count()]{};
        dev.display_get_touch(touches);
        data->point.x = touches[0].x;
        data->point.y = touches[0].y;
        data->state = LV_INDEV_STATE_PRESSED; // Touch is pressed
        screenActiveTime = millis();
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED; // No touch detected
    }
}

// LVGL calls this function to retrieve elapsed time
uint32_t lvgl_millis_cb(void)
{
    return millis();
}

// LVGL calls this function when a rendered image needs to copied to the display
void lvgl_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    dev.graphic->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

    lv_disp_flush_ready(disp);
}

// LVGL calls this function when user changes the volume
static void lvgl_volume_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        // Get the arc widget and its current value.
        lv_obj_t *arc = lv_event_get_target_obj(e);
        uint8_t vol = lv_arc_get_value(arc);

        dev.audio_setVolume(vol);

        lv_obj_set_style_transform_rotation(volumeForeground, 2700 / 21 * vol, 0);

        //Retrieve the volume label from the event's user data and update its text.
        lv_obj_t *vol_label = (lv_obj_t *)lv_event_get_user_data(e);
        if (vol_label)
        {  
            lv_label_set_text_fmt(vol_label, "%d", vol);
        }
    }
}

void createVolumeControllerWidget(lv_obj_t* screen) {
    //background
    LV_IMAGE_DECLARE(pokretlo_tlo_male);
    lv_obj_t *volumeBackground = lv_image_create(screen);
    lv_image_set_src(volumeBackground, &pokretlo_tlo_male);
    lv_obj_align(volumeBackground, LV_ALIGN_BOTTOM_RIGHT, -50, -10);

    //knob
    LV_IMAGE_DECLARE(galka_mala);
    volumeForeground = lv_image_create(screen);
    lv_image_set_src(volumeForeground, &galka_mala);
    lv_obj_align_to(volumeForeground, volumeBackground, LV_ALIGN_CENTER, 1, -2);
    lv_obj_set_style_transform_pivot_x(volumeForeground, lv_pct(50), 0);
    lv_obj_set_style_transform_pivot_y(volumeForeground, lv_pct(50), 0);  

    // Create a volume arc widget
    lv_obj_t *volume_arc = lv_arc_create(screen);
    lv_obj_set_size(volume_arc, 120, 120);


    static lv_style_t style;
    lv_style_init(&style);
    //lv_style_set_arc_color(&style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_arc_width(&style, 50);

    // Set the arc's rotation and background angles so the gauge has a nice appearance.
    lv_arc_set_rotation(volume_arc, 135);
    lv_arc_set_bg_angles(volume_arc, 0, 270);
    lv_obj_add_style(volume_arc, &style, 0);
    lv_obj_align_to(volume_arc, volumeBackground, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_arc_opa(volume_arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(volume_arc, 0, LV_PART_INDICATOR);
    lv_obj_remove_style(volume_arc, NULL, LV_PART_KNOB);

    // Set the range of the arc to match the audio volume range (0 to 21).
    lv_arc_set_range(volume_arc, 0, 21);

    // Set the initial value of the arc using the current volume.
    lv_arc_set_value(volume_arc, dev.audio_getVolume());
    lv_obj_set_style_transform_rotation(volumeForeground, 2700 / 21 * dev.audio_getVolume(), 0);

    
    // Create a label to display the current volume value.
    lv_obj_t *volume_label = lv_label_create(screen);
    lv_label_set_text_fmt(volume_label, "%d", dev.audio_getVolume());
    lv_obj_set_style_text_color(volume_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_align_to(volume_label, volumeForeground, LV_ALIGN_CENTER, 0, 0);
    // Align the label above the volume arc.
    lv_obj_add_event_cb(volume_arc, lvgl_volume_event_cb, LV_EVENT_VALUE_CHANGED, volume_label);
}

// LVGL calls this function to print log information
void lvgl_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}

void setTimeLabel(String currentTime) {
    lv_label_set_text(timeLabel, currentTime.c_str());
}

void createBackGround(lv_obj_t* screen) {
    LV_IMAGE_DECLARE(img_bg);
    lv_obj_t *main_bg = lv_image_create(screen);
    lv_image_set_src(main_bg, &img_bg);
    lv_obj_align(main_bg, LV_ALIGN_CENTER, 0, 0);

}

void createNavigationBar(lv_obj_t* screen) {
    static lv_obj_t* navBar = lv_obj_create(screen);
    lv_obj_set_size(navBar, 55, 272);
    lv_obj_align(navBar, LV_ALIGN_TOP_LEFT, 0, 0);

    //Create the button widget.
    lv_obj_t* btnRadio = lv_button_create(screen);
    lv_obj_set_size(btnRadio, 40, 40);
    lv_obj_align_to(btnRadio, navBar, LV_ALIGN_TOP_MID, 0, 0);
    
    // LV_IMAGE_DECLARE(internetRadio_ico);
    // lv_obj_t * imgRadio = lv_imagebutton_create(screen);
    // lv_imagebutton_set_src(imgRadio, LV_IMAGEBUTTON_STATE_RELEASED, NULL, NULL, &internetRadio_ico);
    // lv_obj_set_size(imgRadio, 40, 40);
    // lv_image_set_inner_align(imgRadio, LV_IMAGE_ALIGN_STRETCH);
    // lv_obj_align_to(imgRadio, navBar, LV_ALIGN_TOP_MID, 0, 0);
    
    LV_IMAGE_DECLARE(internetRadio_ico);
    lv_obj_t* imgRadio = lv_image_create(screen);
    lv_image_set_src(imgRadio, &internetRadio_ico);
    lv_obj_set_size(imgRadio, 40, 40);
    lv_image_set_inner_align(imgRadio, LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_align_to(btnRadio, imgRadio, LV_ALIGN_CENTER, 0, 0);
}

void createRadioScreen() {
    scrRadio = lv_obj_create(NULL);
    createBackGround(scrRadio);
    radioScreenCreate(scrRadio);
    createVolumeControllerWidget(scrRadio);
}

void SetupLVGL()
{
    lv_init();                      // init LVGL
    lv_tick_set_cb(lvgl_millis_cb); // Set a tick source so that LVGL will know how much time elapsed

    // register print function for debugging
    #if LV_USE_LOG != 0 && DEBUG_MODE
        lv_log_register_print_cb(lvgl_print);
    #endif

    uint32_t bufSize = dev.getScreenWidth() * 40* sizeof(lv_color_t);

    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_draw_buf)
    {
        // remove MALLOC_CAP_INTERNAL flag try again
        disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_8BIT);
    }
    if (!disp_draw_buf)
    {
        debugPrint("LVGL disp_draw_buf allocate failed!");
        while (true)
        {
            /* no need to continue */
        }
    }

    disp = lv_display_create(dev.getScreenWidth(), dev.getScreenHeight());
    lv_display_set_flush_cb(disp, lvgl_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create input device (touchpad of the JC4827W543)
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lvgl_touchpad_read);

    //create radio screen
    createRadioScreen();
    //createNavigationBar(scrRadio);

    lv_scr_load(scrRadio);
}