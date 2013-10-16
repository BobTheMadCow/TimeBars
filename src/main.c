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
Layer hour_bar_layer;
Layer minute_bar_layer;

#define GRect( x, y, w,	h ) ((GRect){{(x), (y)}, {(w), (h)}})
	
#define CORNER_MASK GCornersTop
#define CORNER_SIZE 6

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
	PblTm pblTime;
	get_time(&pblTime);
	int hour = pblTime.tm_hour; 
	int start = pblTime.tm_hour;
	int hour_mode;
	float adjusted_hour_unit_height = HOUR_UNIT_HEIGHT;
	int16_t x, y, w, h;
	
	if(clock_is_24h_style())
	{
		adjusted_hour_unit_height = HOUR_UNIT_HEIGHT / 2.0f;
		hour_mode = 24;
		if(hour == 0){ start = hour_mode; }
	}
	else
	{
		if(hour > 12){hour -= 12;}
		hour_mode = 12;
		if(hour == 1){ start = hour_mode; }
	}

	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

	x = HOUR_LOC;
	w = HOUR_WIDTH;
	
	for(int i = start; i >= hour; i--)
	{
		y = BAR_MIN_LOC - (int)(i * adjusted_hour_unit_height);
		h = (int)(i * adjusted_hour_unit_height);
	
		graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);

	  	graphics_text_draw(ctx, "hour", FONT, GRect(HOUR_LOC, (y - BAR_MAX_LOC), 72, BAR_MAX_LOC), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	}
}

void update_minute_bar_callback(Layer *me, GContext* ctx)
{
	PblTm pblTime;
	get_time(&pblTime);
	int minute  = pblTime.tm_min;
	int start = pblTime.tm_min;
	
	if( minute == 0)
	{
		start = 60;
	}
	int16_t x, y, w, h;
	
	x = MINUTE_LOC;
	w = MINUTE_WIDTH;
	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

	for(int i = start; i >= minute; i--)
	{
		y = BAR_MIN_LOC - (int)(i * MINUTE_UNIT_HEIGHT);
		h = (int)(i * MINUTE_UNIT_HEIGHT);

		graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);
	  	graphics_text_draw(ctx, "minute", FONT, GRect(MINUTE_LOC, (y - BAR_MAX_LOC), 72, BAR_MAX_LOC), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	}
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
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
	
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

  	graphics_text_draw(ctx, HOUR_LABEL_TEXT, FONT, GRect(0, BAR_MIN_LOC, 72, (168 - BAR_MIN_LOC)), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  	graphics_text_draw(ctx, MINUTE_LABEL_TEXT, FONT, GRect(72, BAR_MIN_LOC, 72, (168 - BAR_MIN_LOC)), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	layer_init(&hour_bar_layer, window.layer.frame);
	hour_bar_layer.update_proc = update_hour_bar_callback;
	layer_add_child(&window.layer, &hour_bar_layer);

	layer_init(&minute_bar_layer, window.layer.frame);
	minute_bar_layer.update_proc = update_minute_bar_callback;
	layer_add_child(&window.layer, &minute_bar_layer);
}

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