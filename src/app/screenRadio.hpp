#pragma once

lv_obj_t *rollerWidget = NULL; // Global pointer to the roller widget
lv_obj_t *descriptionLabel;    // Global pointer for the description label
lv_obj_t *playButton = NULL; 
lv_obj_t *timeLabel = NULL; 

// LVGL calls this function when the user change radio station with the roller widget
static void lvgl_roller_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        if (strcmp(lv_label_get_text(lv_obj_get_child(playButton, 0)), LV_SYMBOL_PLAY)) {       
            debugPrint("Zmiana symbolu");
            lv_label_set_text(lv_obj_get_child(playButton, 0), LV_SYMBOL_PLAY);
        }
    }
}

// LVGL calls this function when the play button is pressed
static void lvgl_play_btn_event_cb(lv_event_t *e)
{

    int sel = lv_roller_get_selected(rollerWidget);
    debugPrint("Button pressed, selected index: ");
    debugPrint(sel);

    
    lv_obj_t *target_btn = lv_event_get_target_obj(e);
    lv_obj_t *btn_label = lv_obj_get_child(target_btn, 0); 

    if (strcmp(lv_label_get_text(btn_label), LV_SYMBOL_PLAY) != 0)
    {
        debugPrint("Pause procedure");
        lv_label_set_text(btn_label, LV_SYMBOL_PLAY);
        dev.audio_pasueResume();
    }
    else {
        debugPrint("Play procedure");
        if (sel < 0 || sel >= stations.getSourcesCount()) {
            sel = 0;
        }
        lv_label_set_text(descriptionLabel, stations.GetStation(sel).description.c_str());
        lv_label_set_text(btn_label, LV_SYMBOL_PAUSE);
        dev.audio_connectToHost(stations.GetStation(sel).sourceURL.c_str());
    }
}

// Display the play button
void createPlayButtonWidget(lv_obj_t* currentScreen)
{
    #if LV_GRADIENT_MAX_STOPS >= 8
    static const lv_color_t grad_colors[8] = {
        LV_COLOR_MAKE(0xe8, 0xe8, 0xe8),
        LV_COLOR_MAKE(0xff, 0xff, 0xff),
        LV_COLOR_MAKE(0xfa, 0xfa, 0xfa),
        LV_COLOR_MAKE(0x79, 0x79, 0x79),
        LV_COLOR_MAKE(0x48, 0x48, 0x48),
        LV_COLOR_MAKE(0x4b, 0x4b, 0x4b),
        LV_COLOR_MAKE(0x70, 0x70, 0x70),
        LV_COLOR_MAKE(0xe8, 0xe8, 0xe8),
    };
    #elif LV_GRADIENT_MAX_STOPS >= 3
        static const lv_color_t grad_colors[3] = {
            LV_COLOR_MAKE(0xe8, 0xe8, 0xe8),
            LV_COLOR_MAKE(0xff, 0xff, 0xff),
            LV_COLOR_MAKE(0x79, 0x79, 0x79),
        };
    #else
        static const lv_color_t grad_colors[2] = {
            LV_COLOR_MAKE(0xe8, 0xe8, 0xe8),
            LV_COLOR_MAKE(0x79, 0x79, 0x79),
        };
    #endif

    static lv_style_t mainButtonStyle;
    lv_style_init(&mainButtonStyle);
    lv_style_set_bg_opa(&mainButtonStyle, LV_OPA_COVER);
    lv_style_set_shadow_opa(&mainButtonStyle, LV_OPA_50);
    static lv_grad_dsc_t grad;
    lv_grad_init_stops(&grad, grad_colors, NULL, NULL, sizeof(grad_colors) / sizeof(lv_color_t));
    /*Make a conical gradient with the center in the middle of the object*/
    #if LV_GRADIENT_MAX_STOPS >= 8
        lv_grad_conical_init(&grad, 50, 25, 0, 120, LV_GRAD_EXTEND_REFLECT);
    #elif LV_GRADIENT_MAX_STOPS >= 3
        lv_grad_conical_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER, 10, 40, LV_GRAD_EXTEND_REFLECT);
    #else
        lv_grad_conical_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER, 45, 110, LV_GRAD_EXTEND_REFLECT);
    #endif
    lv_style_set_bg_grad(&mainButtonStyle, &grad);

    // Create the button widget.
    playButton = lv_button_create(currentScreen);
    // Align the button below the roller (with a 10-pixel vertical offset).
    lv_obj_set_size(playButton, 50, 50);
    lv_obj_align_to(playButton, rollerWidget, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_add_style(playButton, &mainButtonStyle, 0);
    
    // Add a label to the button.
    lv_obj_t *btn_label = lv_label_create(playButton);
    // lv_label_set_text(btn_label, "Play");
    lv_label_set_text(btn_label, LV_SYMBOL_PLAY);
    lv_obj_center(btn_label);

    lv_obj_add_event_cb(playButton, lvgl_play_btn_event_cb, LV_EVENT_CLICKED, btn_label);
}

void createDescriptionLabel(lv_obj_t* currentScreen) {
    descriptionLabel = lv_label_create(currentScreen);
    lv_obj_set_width(descriptionLabel, 190);
    lv_label_set_long_mode(descriptionLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align(descriptionLabel, LV_ALIGN_TOP_RIGHT, -15, 20);
    lv_obj_set_style_text_color(descriptionLabel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(descriptionLabel, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_align(descriptionLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(descriptionLabel, "."); //opis stacji

}

// Function to create the roller widget and return its pointer.
lv_obj_t *createRollerWidget(lv_obj_t* currentScreen) {

    static lv_style_t localRollerStyle;
    lv_style_init(&localRollerStyle);

    /*Set a background color and a radius*/
    lv_style_set_radius(&localRollerStyle, 10);
    lv_style_set_bg_opa(&localRollerStyle, LV_OPA_40);
    lv_style_set_bg_color(&localRollerStyle, lv_palette_lighten(LV_PALETTE_GREY, 2));
    lv_style_set_border_opa(&localRollerStyle, LV_OPA_0);

    rollerWidget = lv_roller_create(currentScreen);
    lv_obj_add_style(rollerWidget, &localRollerStyle, 0);
    lv_roller_set_options(rollerWidget, stations.StationsList().c_str(), LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(rollerWidget, 4);
    lv_obj_set_style_bg_opa(rollerWidget, LV_OPA_60, LV_PART_SELECTED);
    lv_obj_set_style_bg_color(rollerWidget, lv_color_hex(0x000000), LV_PART_SELECTED);


    // Align the roller to the top left with a margin.
    lv_obj_align(rollerWidget, LV_ALIGN_TOP_LEFT, 20, 40);
    lv_obj_add_event_cb(rollerWidget, lvgl_roller_event_handler, LV_EVENT_ALL, NULL);

    return rollerWidget;
}

void createTimeLabel(lv_obj_t* currentScreen) {
    timeLabel = lv_label_create(currentScreen);
    lv_obj_set_width(timeLabel, 190);
    lv_obj_set_height(timeLabel, 100);
    lv_label_set_long_mode(timeLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(timeLabel, descriptionLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_set_style_text_align(timeLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_30, 0);
    lv_label_set_text(timeLabel, "Czas...");
}

void radioScreenCreate(lv_obj_t* screen) {
    createDescriptionLabel(screen);
    createRollerWidget(screen);
    createPlayButtonWidget(screen);
    createTimeLabel(screen);
}