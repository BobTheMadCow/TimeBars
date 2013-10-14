#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x07, 0xCE, 0xF0, 0x49, 0x59, 0xB8, 0x47, 0x40, 0x8F, 0x96, 0xF4, 0x1F, 0x8C, 0x26, 0x0A, 0xD3 }
PBL_APP_INFO(MY_UUID,
             "TimeBars", "BobTheMadCow",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer hour_label_layer;
TextLayer minute_label_layer;
TextLayer hh_label_layer;
TextLayer mm_label_layer;
Layer hour_bar_layer;
Layer minute_bar_layer;

#define GRect( x, y, w,	h ) ((GRect){{(x), (y)}, {(w), (h)}})
	
#define CORNER_MASK GCornersTop
#define CORNER_SIZE 2

//origin of Pebble's 2D coordinate system is in the upper, lefthand corner its x-axis 
//extends to the right and its y-axis extends to the bottom of the screen. 
/********************
	 5 62 10 62 5
	+------------+ ^
	|  12    59  | 24
	| ,--.  ,--. | X
	| |  |  |  | | |
	| |  |  |  | | 144
	| |  |  |  | | |
	| +--+  +--+ | X
	| HOUR  MINS | 24
	+------------+ v
*********************/
#define BAR_MAX_LOC 24
#define BAR_MIN_LOC 144
#define MAX_HEIGHT (BAR_MIN_LOC - BAR_MAX_LOC)
#define HOUR_LOC 5
#define MINUTE_LOC 77
#define HOUR_WIDTH 62
#define MINUTE_WIDTH 62
#define HOUR_UNIT_HEIGHT (MAX_HEIGHT/12.0f)
#define MINUTE_UNIT_HEIGHT (MAX_HEIGHT/60.0f)

#define HOUR_LABEL_TEXT "HOUR"
#define MINUTE_LABEL_TEXT "MINS"
#define FONT fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)

#define BACKGROUND_COLOR GColorWhite
#define FOREGROUND_COLOR GColorBlack

void update_hour_bar_callback(Layer *me, GContext* ctx)
{
	(void)ctx;
	
	PblTm pblTime;
	get_time(&pblTime);
	int hour = pblTime.tm_hour;
	
	int16_t x, y, w, h;
	float adjusted_hour_unit_height = HOUR_UNIT_HEIGHT;
	
	if(clock_is_24h_style())
	{
		adjusted_hour_unit_height = HOUR_UNIT_HEIGHT / 2.0f;
	}
	else
	{
		hour = hour % 12;
	}
	
	x = HOUR_LOC;
	y = BAR_MIN_LOC - (int)(hour * adjusted_hour_unit_height);
	w = HOUR_WIDTH;
	h = (int)(hour * adjusted_hour_unit_height);
	
	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	
	graphics_fill_rect(
		ctx,//GContext *ctx, 
		GRect( x, y, w, h ),//GRect rect, 
		CORNER_SIZE,//uint8_t corner_radius, 
		CORNER_MASK //GCornerMask corner_mask
	);
}

void update_minute_bar_callback(Layer *me, GContext* ctx)
{
	(void)ctx;
	
	PblTm pblTime;
	get_time(&pblTime);
	
	int16_t x, y, w, h;
	
	x = MINUTE_LOC;
	y = BAR_MIN_LOC - (int)(pblTime.tm_min * MINUTE_UNIT_HEIGHT);
	w = MINUTE_WIDTH;
	h = (int)(pblTime.tm_min * MINUTE_UNIT_HEIGHT);

	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
		
	graphics_fill_rect(
		ctx,//GContext *ctx, 
		GRect( x, y, w, h ),//GRect rect, 
		CORNER_SIZE,//uint8_t corner_radius, 
		CORNER_MASK //GCornerMask corner_mask
	);
}

void mm_label_update_callback(Layer *me, GContext* ctx)
{
	PblTm pblTime;
	get_time(&pblTime);
	minute = pblTime.tm_min;
	int16_t y = BAR_MIN_LOC - (int)(minute * MINUTE_UNIT_HEIGHT) - BAR_MAX_LOC;
		
	layer_set_frame(me, GRect( MINUTE_LOC, y, 72, BAR_MAX_LOC));
	text_layer_set_text(me, minute);
}
	
void hh_label_update_callback(Layer *me, GContext* ctx)
{
	PblTm pblTime;
	get_time(&pblTime);
	int hour = pblTime.tm_hour;
	
	float adjusted_hour_unit_height = HOUR_UNIT_HEIGHT;
	
	if(clock_is_24h_style())
	{
		adjusted_hour_unit_height = HOUR_UNIT_HEIGHT / 2.0f;
	}
	else
	{
		hour = hour % 12;
	}

	int16_t y = BAR_MIN_LOC - (int)(hour * adjusted_hour_unit_height) - BAR_MAX_LOC;
		
	layer_set_frame(me, GRect( HOUR_LOC, y, 72, BAR_MAX_LOC));
	text_layer_set_text(me, minute);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
	(void)t;
	(void)ctx;
	
	PblTm pblTime;
	get_time(&pblTime);
	
	if( pblTime.tm_min == 0 )
	{
		layer_mark_dirty(&hour_bar_layer);
	}
	layer_mark_dirty(&minute_bar_layer);
}

void handle_init(AppContextRef ctx) 
{
	(void)ctx;

	window_init(&window, "TimeBars");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, BACKGROUND_COLOR);
	
	text_layer_init(&hour_label_layer, window.layer.frame);
	text_layer_set_text_color(&hour_label_layer, FOREGROUND_COLOR);
	text_layer_set_background_color(&hour_label_layer, BACKGROUND_COLOR);
	layer_set_frame(&hour_label_layer.layer, GRect(0, BAR_MIN_LOC, 72, (168 - BAR_MIN_LOC)));
	text_layer_set_font(&hour_label_layer, FONT);
	text_layer_set_text(&hour_label_layer, HOUR_LABEL_TEXT);
	layer_add_child(&window.layer, &hour_label_layer.layer);
	
	text_layer_init(&minute_label_layer, window.layer.frame);
	text_layer_set_text_color(&minute_label_layer, FOREGROUND_COLOR);
	text_layer_set_background_color(&minute_label_layer, BACKGROUND_COLOR);
	layer_set_frame(&minute_label_layer.layer, GRect(72, BAR_MIN_LOC, 72, (168 - BAR_MIN_LOC)));
	text_layer_set_font(&minute_label_layer, FONT);
	text_layer_set_text(&minute_label_layer, MINUTE_LABEL_TEXT);
	layer_add_child(&window.layer, &minute_label_layer.layer);
	
	layer_init(&hour_bar_layer, window.layer.frame);
	hour_bar_layer.update_proc = update_hour_bar_callback;
	layer_add_child(&window.layer, &hour_bar_layer);

	layer_init(&minute_bar_layer, window.layer.frame);
	minute_bar_layer.update_proc = update_minute_bar_callback;
	layer_add_child(&window.layer, &minute_bar_layer);
	
	text_layer_init(&hh_label_layer, window.layer.frame);
	text_layer_set_text_color(&hh_label_layer, FOREGROUND_COLOR);
	text_layer_set_background_color(&hh_label_layer, BACKGROUND_COLOR);
	layer_set_frame(&hh_label_layer.layer, GRect( 0, 0, 72, BAR_MAX_LOC));
	text_layer_set_font(&hh_label_layer, FONT);
	text_layer_set_text(&hh_label_layer, "HH");
	hh_label_layer.update_proc = hh_label_update_callback;
	layer_add_child(&hour_bar_layer, &hh_label_layer.layer);

	text_layer_init(&mm_label_layer, window.layer.frame);
	text_layer_set_text_color(&mm_label_layer, FOREGROUND_COLOR);
	text_layer_set_background_color(&mm_label_layer, BACKGROUND_COLOR);
	layer_set_frame(&mm_label_layer.layer, GRect( 72, 0, 72, BAR_MAX_LOC));
	text_layer_set_font(&mm_label_layer, FONT);
	text_layer_set_text(&mm_label_layer, "MM");
	mm_label_layer.update_proc = mm_label_update_callback;
	layer_add_child(&minute_bar_layer, &mm_label_layer.layer);}


void pbl_main(void *params) 
{
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
		.tick_handler = &handle_minute_tick,
		.tick_units = MINUTE_UNIT
    },
  };
  app_event_loop(params, &handlers);
}