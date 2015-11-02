#include <pebble.h>
#include <ctype.h>

#define CURRENT_VERSION 2
 
#define KEY_TEMPERATURE 0
#define KEY_ICON 1
#define KEY_ADTZ 2
#define KEY_BLUETOOTH_VIBE 3
#define KEY_HOURLY_VIBE 4
#define KEY_HOUR_RED 5
#define KEY_HOUR_GREEN 6
#define KEY_HOUR_BLUE 7
#define KEY_MINUTE_RED 8
#define KEY_MINUTE_GREEN 9
#define KEY_MINUTE_BLUE 10
#define KEY_SECOND_BAR_RED 11
#define KEY_SECOND_BAR_GREEN 12
#define KEY_SECOND_BAR_BLUE 13
#define KEY_POWER_SAVING_START 14
#define KEY_POWER_SAVING_END 15
#define KEY_SECOND_BAR 16
#define KEY_SECOND_RED 18
#define KEY_SECOND_GREEN 19
#define KEY_SECOND_BLUE 20
#define KEY_SECOND_REFRESH_RATE 21
#define KEY_12HOUR 22

#define KEY_LAST_WEATHER_UPDATE_TIME 17

#define SETTING_VIBE_DISABLED 0
#define SETTING_VIBE_LONG 1
#define SETTING_VIBE_SHORT 2
#define SETTING_VIBE_DOUBLE 3

#define SETTING_SECOND_ENABLE 0
#define SETTING_SECOND_FILL_COLOR 1
#define SETTING_SECOND_FILL_BLACK 2
#define SETTING_SECOND_FILL_COLOR_WHEN_SAVING 3
#define SETTING_SECOND_FILL_BLACK_WHEN_SAVING 4
#define SETTING_SECOND_AS_AM_PM 5

    
    
static Window *window;
static Layer *s_bg_layer, *s_time_layer, *s_fg_layer, *s_battery_layer, *s_bt_layer, *s_weather_layer, *s_date_layer, *s_tz_layer;
static GBitmap *s_bitmap, *s_window_bitmap, *s_bt_bitmap, *s_battery_bitmap[2], *s_weather_bitmap[12];

static int s_battery_level;
static int s_weather_temp;
static int s_weather_icon;
static int s_last_update_time;
static int s_ad_time_zone;
static int s_vibe_bt;
static int s_vibe_hourly;
static int s_hour_r, s_hour_g, s_hour_b;
static int s_minute_r, s_minute_g, s_minute_b;
static int s_second_bar_r, s_second_bar_g, s_second_bar_b;
static int s_second_r, s_second_g, s_second_b;
static int s_ps_start_time, s_ps_end_time;
static int s_second_bar;
static int s_second_rate;

static int s_last_second;
static int s_msecond;

static bool s_12hour;
    
static bool s_battery_charging;
static bool s_bt_connected;

static void animation_timer_callback(void *data)
{
    if (s_second_rate > 1)
    {
        if (s_second_rate == 2)
        {
            s_msecond += 500;
            app_timer_register(500, animation_timer_callback, NULL);
        }
        else if (s_second_rate == 3)
        {
            s_msecond += 250;
            app_timer_register(250, animation_timer_callback, NULL);
        }
        else if (s_second_rate == 4)
        {
            s_msecond += 100;
            app_timer_register(100, animation_timer_callback, NULL); 
        } 
        else if (s_second_rate == 5)
        {
            s_msecond += 66;
            app_timer_register(66, animation_timer_callback, NULL);
        }
    }
    else
        app_timer_register(1000, animation_timer_callback, NULL);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Read first item
    Tuple *t = dict_read_first(iterator);
    bool updated_weather = false;
    // For all items
    while(t != NULL) {
        // Which key was received?
        switch(t->key) {
        case KEY_TEMPERATURE:
            s_weather_temp = (int)t->value->int32;
            persist_write_int(KEY_TEMPERATURE, s_weather_temp);
            updated_weather = true;
            break;
        case KEY_ICON:
            s_weather_icon = (int)t->value->int32;
            persist_write_int(KEY_ICON, s_weather_icon);
            updated_weather = true;
            break;
        case KEY_ADTZ:
            s_ad_time_zone = (int)t->value->int32;
            persist_write_int(KEY_ADTZ, s_ad_time_zone);
            break;
        case KEY_BLUETOOTH_VIBE:
            s_vibe_bt = (int)t->value->int32;
            persist_write_int(KEY_BLUETOOTH_VIBE, s_vibe_bt);
            break;
        case KEY_HOURLY_VIBE:
            s_vibe_hourly = (int)t->value->int32;
            persist_write_int(KEY_HOURLY_VIBE, s_vibe_hourly);
            break;
        case KEY_HOUR_RED:
            s_hour_r = (int)t->value->int32;
            persist_write_int(KEY_HOUR_RED, s_hour_r);
            break;
        case KEY_HOUR_GREEN:
            s_hour_g = (int)t->value->int32;
            persist_write_int(KEY_HOUR_GREEN, s_hour_g);
            break;
        case KEY_HOUR_BLUE:
            s_hour_b = (int)t->value->int32;
            persist_write_int(KEY_HOUR_BLUE, s_hour_b);
            break;
        case KEY_MINUTE_RED:
            s_minute_r = (int)t->value->int32;
            persist_write_int(KEY_MINUTE_RED, s_minute_r);
            break;
        case KEY_MINUTE_GREEN:
            s_minute_g = (int)t->value->int32;
            persist_write_int(KEY_MINUTE_GREEN, s_minute_g);
            break;
        case KEY_MINUTE_BLUE:
            s_minute_b = (int)t->value->int32;
            persist_write_int(KEY_MINUTE_BLUE, s_minute_b);
            break;
        case KEY_SECOND_BAR_RED:
            s_second_bar_r = (int)t->value->int32;
            persist_write_int(KEY_SECOND_BAR_RED, s_second_bar_r);
            break;
        case KEY_SECOND_BAR_GREEN:
            s_second_bar_g = (int)t->value->int32;
            persist_write_int(KEY_SECOND_BAR_GREEN, s_second_bar_g);
            break;
        case KEY_SECOND_BAR_BLUE:
            s_second_bar_b = (int)t->value->int32;
            persist_write_int(KEY_SECOND_BAR_BLUE, s_second_bar_b);
            break;
        case KEY_POWER_SAVING_START:
            s_ps_start_time = (int)t->value->int32;
            persist_write_int(KEY_POWER_SAVING_START, s_ps_start_time);
            break;
        case KEY_POWER_SAVING_END:
            s_ps_end_time = (int)t->value->int32;
            persist_write_int(KEY_POWER_SAVING_END, s_ps_end_time);
            break;
        case KEY_SECOND_BAR:
            s_second_bar = (int)t->value->int32;
            persist_write_int(KEY_SECOND_BAR, s_second_bar);
            break;
        case KEY_SECOND_RED:
            s_second_r = (int)t->value->int32;
            persist_write_int(KEY_SECOND_RED, s_second_r);
            break;
        case KEY_SECOND_GREEN:
            s_second_g = (int)t->value->int32;
            persist_write_int(KEY_SECOND_GREEN, s_second_g);
            break;
        case KEY_SECOND_BLUE:
            s_second_b = (int)t->value->int32;
            persist_write_int(KEY_SECOND_BLUE, s_second_b);
            break;
        case KEY_SECOND_REFRESH_RATE:
            s_second_rate = (int)t->value->int32;
            persist_write_int(KEY_SECOND_REFRESH_RATE, s_second_rate);
            break;
        case KEY_12HOUR:
            s_12hour = ((int)t->value->int32 == 1);
            persist_write_bool(KEY_12HOUR, s_12hour);
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
            break;
        }
    
        // Look for next item
        t = dict_read_next(iterator);
    }
    
    if (updated_weather)
    {
        time_t now = time(NULL);
        struct tm *time = gmtime(&now);
        s_last_update_time = time->tm_mday*100+time->tm_hour;
        persist_write_int(KEY_LAST_WEATHER_UPDATE_TIME, s_last_update_time);
    }

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    s_battery_charging = state.is_charging;
    layer_mark_dirty(s_battery_layer);
}

static void bluetooth_callback(bool connected) {
    s_bt_connected = connected;
    if (!connected && s_vibe_bt != SETTING_VIBE_DISABLED)
    {
        switch (s_vibe_bt)
        {
            case SETTING_VIBE_LONG:
                vibes_long_pulse();
                break;
            case SETTING_VIBE_SHORT:
                vibes_short_pulse();
                break;
            case SETTING_VIBE_DOUBLE:
                vibes_double_pulse();
                break;
        }
    }
    layer_mark_dirty(s_bt_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(115,126,29,25), 0, GCornerNone);
    GBitmap *battery_bitmap;
    if (s_battery_charging)
    {
        battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
    }
    else 
    {
        battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
    }
        
    graphics_draw_bitmap_in_rect(ctx, battery_bitmap, GRect(120,125,13,7));
    gbitmap_destroy(battery_bitmap);
    
    if (s_battery_charging) return;
        
    if (s_battery_level <= 20) graphics_context_set_fill_color(ctx, GColorRed);
    else graphics_context_set_fill_color(ctx, GColorGreen);
    
    graphics_fill_rect(ctx, GRect(121,126,s_battery_level/10,5), 0, GCornerNone);
}

static void bt_update_proc(Layer *layer, GContext *ctx)
{
    if (s_bt_connected)
        graphics_draw_bitmap_in_rect(ctx, s_bt_bitmap, GRect(123,136,6,11));
}

static void weather_update_proc(Layer *layer, GContext *ctx)
{
    if (s_weather_icon >= 0 && s_weather_icon < 12)
        graphics_draw_bitmap_in_rect(ctx, s_weather_bitmap[s_weather_icon], GRect(116,23,16,16));
    if (s_weather_temp <= -100) return;
    char *buffer = malloc(4);
    snprintf(buffer, 3, "%02d", s_weather_temp);
    graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(116,35,16,14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    free(buffer);
}

static void bg_update_proc(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0,0,144,168), 0, GCornerNone);
}

static void tz_update_proc(Layer *layer, GContext *ctx)
{
    if (s_ad_time_zone == 0) return;
    
    time_t now = time(NULL);
    struct tm *bt = gmtime(&now);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(11,27,32,20), 0, GCornerNone);
    
    char *buffer = malloc(3);
    snprintf(buffer, 3, "%02d", (bt->tm_hour + s_ad_time_zone - 13 + 24)%24);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS), GRect(7,24,40,26), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    free(buffer);
    
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_window_bitmap, GRect(7, 25, 39, 24));
    
}

static void time_update_proc(Layer *layer, GContext *ctx)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    bool saving = false;
    bool pm = t->tm_hour >= 12;
    if (s_ps_start_time-1 > s_ps_end_time)
    {
        if (t->tm_hour >= s_ps_start_time-1 || t->tm_hour <= s_ps_end_time)
            saving = true;
    }
    else
    {
        if (t->tm_hour >= s_ps_start_time-1 && t->tm_hour <= s_ps_end_time)
            saving = true;
    }
    
    int first = t->tm_sec - 30;
    if (first < -1) first = -1;
    int second = t->tm_sec;
    if (second > 30) second = 30;
    
    switch (s_second_bar)
    {
        case SETTING_SECOND_FILL_COLOR:
            first = 30;
            second = 30;
            break;
        case SETTING_SECOND_FILL_BLACK:
            first = -1;
            second = -1;
            break;
        case SETTING_SECOND_FILL_COLOR_WHEN_SAVING:
            if (saving)
            {
                first = 30;
                second = 30;
            }
            break;
        case SETTING_SECOND_FILL_BLACK_WHEN_SAVING:
            if (saving)
            {
                first = -1;
                second = -1;
            }
            break;
        case SETTING_SECOND_AS_AM_PM:
            second = pm ? -1 : 30;
            first = pm ? 30 : -1;
            break;
    }
    
    if (s_ps_start_time == 0) saving = false;
    
    // min
    char *buffer = malloc(3);

    for (int i = -2; i < 5; i++)
    {
        snprintf(buffer, 3, "%02d", (t->tm_min + i + 60)%60);
        if (i == 0) graphics_context_set_text_color(ctx, GColorFromRGB(s_minute_r, s_minute_g, s_minute_b));
        else graphics_context_set_text_color(ctx, GColorDarkGray);
        
        graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS), GRect(75-i*39+(saving?0:(39*t->tm_sec/60)),74,40,38), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);    
    }
    
    // sec
    if (s_second_rate > 0 && !saving)
    {
        for (int i = -2; i < 5; i++)
        {
            float progress = 0;
            if (s_second_rate > 1)
            {
                progress = s_msecond/1000.0;
            }
            if (t->tm_sec != s_last_second)
            {
                s_msecond = 0;
                s_last_second = t->tm_sec;
            }
            snprintf(buffer, 3, "%02d", (t->tm_sec + i + 60)%60);
            if (i == 0) graphics_context_set_text_color(ctx, GColorFromRGB(s_second_r, s_second_g, s_second_b));
            else graphics_context_set_text_color(ctx, GColorDarkGray);
            
            graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS), GRect(75-i*26+(26*progress),120,40,20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);    
        }
        graphics_context_set_stroke_color(ctx, GColorDarkGray);
        graphics_draw_line(ctx, GPoint(0,121), GPoint(144,121));
        graphics_draw_line(ctx, GPoint(0,143), GPoint(144,143));
    }

    // hour
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(23,12,46,142), 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(23,12,46,142), 0, GCornerNone);
    
    for (int i = -4; i < 6; i++)
    {
        if (s_12hour) 
            snprintf(buffer, 3, "%02d", (t->tm_hour + i == 12) ? 12 : (t->tm_hour + i + 12)%12);
        else
            snprintf(buffer, 3, "%02d", (t->tm_hour + i + 24)%24);
        if (i == 0) graphics_context_set_text_color(ctx, GColorFromRGB(s_hour_r, s_hour_g, s_hour_b));
        else graphics_context_set_text_color(ctx, GColorDarkGray);
        graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS), GRect(21,65-i*27+(saving?0:(27*t->tm_min/60)),50,36), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
    
    if (s_second_bar != SETTING_SECOND_AS_AM_PM && s_12hour)
    {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_rect(ctx, GRect(12,80,5,26), 0, GCornerNone);
        
        graphics_context_set_fill_color(ctx, GColorFromRGB(s_hour_r, s_hour_g, s_hour_b));
        if (pm)
            graphics_fill_rect(ctx, GRect(12,93,5,13), 0, GCornerNone);
        else
            graphics_fill_rect(ctx, GRect(12,80,5,13), 0, GCornerNone);
    }
    
    free(buffer);
    
    // SEC
    graphics_context_set_stroke_color(ctx, GColorFromRGB(s_second_bar_r, s_second_bar_g, s_second_bar_b)); 
    graphics_draw_line(ctx, GPoint(29,61), GPoint(29+second,61));
    graphics_draw_line(ctx, GPoint(34,116), GPoint(34+first,116));
    
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, GPoint(30+second,61), GPoint(60,61));
    graphics_draw_line(ctx, GPoint(35+first,116), GPoint(65,116));
}

static void fg_update_proc(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_bitmap, gbitmap_get_bounds(s_bitmap));
}

static void date_update_proc(Layer *layer, GContext *ctx)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // DATE
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(51,35,60,11), 0, GCornerNone);
    
    char *buffer = malloc(10);
    strftime(buffer, 10, "%a %m/%d", t);
    
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(51,31,60,14), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    
    free(buffer);
}

static void request_weather()
{
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_INFO, "Weather request sended.");
}

static void window_load()
{
    // Create window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
  
    // Background layer
    s_bg_layer = layer_create(bounds);
    layer_set_update_proc(s_bg_layer, bg_update_proc);
    layer_add_child(window_layer, s_bg_layer);
    
    // Time layer
    s_time_layer = layer_create(bounds);
    layer_set_update_proc(s_time_layer, time_update_proc);
    layer_add_child(window_layer, s_time_layer);
    
    // Date layer
    s_date_layer = layer_create(bounds);
    layer_set_update_proc(s_date_layer, date_update_proc);
    layer_add_child(window_layer, s_date_layer);
    
    // Battery layer
    s_battery_bitmap[0] = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
    s_battery_bitmap[1] = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
    s_battery_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(window_layer, s_battery_layer);
    
    // Bluetooth layer
    s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH);
    s_bt_layer = layer_create(bounds);
    layer_set_update_proc(s_bt_layer, bt_update_proc);
    layer_add_child(window_layer, s_bt_layer);
    
    // Weather layer
    s_weather_bitmap[0] = gbitmap_create_with_resource(RESOURCE_ID_W01D);
    s_weather_bitmap[1] = gbitmap_create_with_resource(RESOURCE_ID_W01N);
    s_weather_bitmap[2] = gbitmap_create_with_resource(RESOURCE_ID_W02D);
    s_weather_bitmap[3] = gbitmap_create_with_resource(RESOURCE_ID_W02N);
    s_weather_bitmap[4] = gbitmap_create_with_resource(RESOURCE_ID_W03);
    s_weather_bitmap[5] = gbitmap_create_with_resource(RESOURCE_ID_W04);
    s_weather_bitmap[6] = gbitmap_create_with_resource(RESOURCE_ID_W09);
    s_weather_bitmap[7] = gbitmap_create_with_resource(RESOURCE_ID_W10D);
    s_weather_bitmap[8] = gbitmap_create_with_resource(RESOURCE_ID_W10N);
    s_weather_bitmap[9] = gbitmap_create_with_resource(RESOURCE_ID_W11);
    s_weather_bitmap[10] = gbitmap_create_with_resource(RESOURCE_ID_W13);
    s_weather_bitmap[11] = gbitmap_create_with_resource(RESOURCE_ID_W50);
    s_weather_layer = layer_create(bounds);
    layer_set_update_proc(s_weather_layer, weather_update_proc);
    layer_add_child(window_layer, s_weather_layer);
    
    // Foreground layer
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME);
    s_fg_layer = layer_create(bounds);
    layer_set_update_proc(s_fg_layer, fg_update_proc);
    layer_add_child(window_layer, s_fg_layer);
    
    // Time Zone layer
    s_window_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WINDOW);
    s_tz_layer = layer_create(bounds);
    layer_set_update_proc(s_tz_layer, tz_update_proc);
    layer_add_child(window_layer, s_tz_layer);
    
    // Init callback
    bluetooth_callback(bluetooth_connection_service_peek());
    battery_callback(battery_state_service_peek());
    
    // Init
    s_weather_temp = persist_exists(KEY_TEMPERATURE) ? persist_read_int(KEY_TEMPERATURE) : -100;
    s_weather_icon = persist_exists(KEY_ICON) ? persist_read_int(KEY_ICON) : -1;
    s_last_update_time = persist_exists(KEY_LAST_WEATHER_UPDATE_TIME) ? persist_read_int(KEY_LAST_WEATHER_UPDATE_TIME) : -1;
    s_ad_time_zone = persist_exists(KEY_ADTZ) ? persist_read_int(KEY_ADTZ) : 0;
    s_vibe_bt = persist_exists(KEY_BLUETOOTH_VIBE) ? persist_read_int(KEY_BLUETOOTH_VIBE) : 0;
    s_vibe_hourly = persist_exists(KEY_HOURLY_VIBE) ? persist_read_int(KEY_HOURLY_VIBE) : 0;
    s_hour_r = persist_exists(KEY_HOUR_RED) ? persist_read_int(KEY_HOUR_RED) : 0;
    s_hour_g = persist_exists(KEY_HOUR_GREEN) ? persist_read_int(KEY_HOUR_GREEN) : 170;
    s_hour_b = persist_exists(KEY_HOUR_BLUE) ? persist_read_int(KEY_HOUR_BLUE) : 255;
    s_minute_r = persist_exists(KEY_MINUTE_RED) ? persist_read_int(KEY_MINUTE_RED) : 255;
    s_minute_g = persist_exists(KEY_MINUTE_GREEN) ? persist_read_int(KEY_MINUTE_GREEN) : 0;
    s_minute_b = persist_exists(KEY_MINUTE_BLUE) ? persist_read_int(KEY_MINUTE_BLUE) : 0;
    s_second_r = persist_exists(KEY_SECOND_RED) ? persist_read_int(KEY_SECOND_RED) : 0;
    s_second_g = persist_exists(KEY_SECOND_GREEN) ? persist_read_int(KEY_SECOND_GREEN) : 255;
    s_second_b = persist_exists(KEY_SECOND_BLUE) ? persist_read_int(KEY_SECOND_BLUE) : 0;
    s_ps_start_time = persist_exists(KEY_POWER_SAVING_START) ? persist_read_int(KEY_POWER_SAVING_START) : 0;
    s_ps_end_time = persist_exists(KEY_POWER_SAVING_END) ? persist_read_int(KEY_POWER_SAVING_END) : 0;
    s_second_bar = persist_exists(KEY_SECOND_BAR) ? persist_read_int(KEY_SECOND_BAR) : 0;
    s_second_bar_r = persist_exists(KEY_SECOND_BAR_RED) ? persist_read_int(KEY_SECOND_BAR_RED) : 0;
    s_second_bar_g = persist_exists(KEY_SECOND_BAR_GREEN) ? persist_read_int(KEY_SECOND_BAR_GREEN) : 255;
    s_second_bar_b = persist_exists(KEY_SECOND_BAR_BLUE) ? persist_read_int(KEY_SECOND_BAR_BLUE) : 0;
    s_second_rate = persist_exists(KEY_SECOND_REFRESH_RATE) ? persist_read_int(KEY_SECOND_REFRESH_RATE) : 0;
    s_12hour = persist_exists(KEY_12HOUR) ? persist_read_bool(KEY_12HOUR) : false;
    
    if (s_second_rate < 2)
    {
        app_timer_register(1000, animation_timer_callback, NULL);
    }   
    else if (s_second_rate == 2)
    {
        app_timer_register(500, animation_timer_callback, NULL);
    }
    else if (s_second_rate == 3)
    {
        app_timer_register(250, animation_timer_callback, NULL);
    }
    else if (s_second_rate == 4)
    {
        app_timer_register(100, animation_timer_callback, NULL); 
    }
    else if (s_second_rate == 5)
    {
        app_timer_register(66, animation_timer_callback, NULL);
    }
    s_last_second = -1;
    s_msecond = 0;

}

static void window_unload()
{
    layer_destroy(s_bg_layer);
    layer_destroy(s_time_layer);
    layer_destroy(s_fg_layer);
    layer_destroy(s_battery_layer);
    layer_destroy(s_bt_layer);
    layer_destroy(s_weather_layer);
    layer_destroy(s_date_layer);
    layer_destroy(s_tz_layer);
    gbitmap_destroy(s_bitmap);
    gbitmap_destroy(s_window_bitmap);
    gbitmap_destroy(s_bt_bitmap);
    gbitmap_destroy(s_battery_bitmap[0]);
    gbitmap_destroy(s_battery_bitmap[1]);
    for (int i = 0; i < 12; i++)
    {
        gbitmap_destroy(s_weather_bitmap[i]);
    }
}

static void tick(struct tm *tick_time, TimeUnits units_changed)
{
    if (s_second_rate < 2)
    {
        layer_mark_dirty(s_date_layer);
        layer_mark_dirty(s_time_layer);
    }
    
    if (tick_time->tm_min == 0 && tick_time->tm_sec == 0 && s_vibe_hourly != SETTING_VIBE_DISABLED)
    {
        switch (s_vibe_hourly)
        {
            case SETTING_VIBE_LONG:
                vibes_long_pulse();
                break;
            case SETTING_VIBE_SHORT:
                vibes_short_pulse();
                break;
            case SETTING_VIBE_DOUBLE:
                vibes_double_pulse();
                break;
        }
    }
    
    // Get weather update every 2 hours
    if (s_weather_icon == -1 || s_weather_temp == -100 || tick_time->tm_mday*100+tick_time->tm_hour > s_last_update_time + 1)
    {
        request_weather();
    }
    if(tick_time->tm_hour % 2 == 0 && tick_time->tm_min == 0 && tick_time->tm_sec == 0)
    {
        request_weather();
    }
}

static void init()
{
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) { .load = window_load, .unload = window_unload });
    
    window_stack_push(window, true);
    
    // subscribe
    tick_timer_service_subscribe(SECOND_UNIT, tick);
    battery_state_service_subscribe(battery_callback);
    bluetooth_connection_service_subscribe(bluetooth_callback);
    
    // app message
    app_message_register_inbox_received(inbox_received_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Started");
}

static void deinit()
{
    // unsubscribe
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    
    window_destroy(window);
}

int main() {
    init();
    app_event_loop();
    deinit();
}