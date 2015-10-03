#include <pebble.h>

static Window *s_window;
static TextLayer *time_layer, *battery_layer, *date_layer, *day_date_layer;
static GFont *time_font, *date_font;
static BitmapLayer *background_layer, *battery_draw;
static GBitmap *background_bitmap;

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  layer_set_frame(bitmap_layer_get_layer(battery_draw),GRect(127,7,new_state.charge_percent/10,4));
}

static void onLoad(Window *window) {
	// Create GBitmap, then set to created BitmapLayer
	background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(background_layer, background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));

	// Create GFont
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JAPFONT_32));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JAPFONT_16));

  	// Create date TextLayer
	date_layer = text_layer_create(GRect(42, 0, 60, 30));
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer,date_font);

	// Create day date TextLayer
	day_date_layer = text_layer_create(GRect(1, 0, 40, 30));
	text_layer_set_text_color(day_date_layer, GColorWhite);
	text_layer_set_background_color(day_date_layer, GColorClear);
	text_layer_set_text_alignment(day_date_layer, GTextAlignmentLeft);
	text_layer_set_font(day_date_layer,date_font);

	// Add to Window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_date_layer));

	// Create time TextLayer
	time_layer = text_layer_create(GRect(0, 130, 144, 38));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_text(time_layer, "00:00");

	// Improve the layout to be more like a watchface
	// Apply to TextLayer
	text_layer_set_font(time_layer, time_font);
	//text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

	battery_draw = bitmap_layer_create(GRect(128,4,10,5));
  	bitmap_layer_set_background_color(battery_draw,GColorWhite);
  	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_draw));

  	// Get the current battery level
  	battery_handler(battery_state_service_peek());
}

static void onUnload(Window *window) {
	// Destroy Time TextLayer
	text_layer_destroy(time_layer);
	// Destroy Battery TextLayer
	text_layer_destroy(battery_layer);
	// Destroy Date Layer
	text_layer_destroy(date_layer);
	// Destroy Day Date Layer
	text_layer_destroy(day_date_layer);
	// Destroy Battery DrawLayer
	bitmap_layer_destroy(battery_draw);
	// Unload GFont
	fonts_unload_custom_font(time_font);
	fonts_unload_custom_font(date_font);
	// Destroy GBitmap
	gbitmap_destroy(background_bitmap);
	// Destroy BitmapLayer
	bitmap_layer_destroy(background_layer);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, buffer);

  // Copy date into buffer from tm structure
	static char date_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%d %b", tick_time);
	static char day_buffer[16];
	strftime(day_buffer, sizeof(day_buffer), "%a", tick_time);

	// Show the date
	text_layer_set_text(date_layer, date_buffer);
	text_layer_set_text(day_date_layer,day_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void init() {
	// Create main Window element and assign to pointer
	s_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = onLoad,
		.unload = onUnload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_window, true);

	// Update time
	update_time();

	// Subscribe to onMinute change
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	// Subscribe to Battery Events
	battery_state_service_subscribe(battery_handler);
}

static void deinit() {
	window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}