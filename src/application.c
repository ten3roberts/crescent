#include "application.h"
#include "cr_time.h"
#include "log.h"
#include "math/math_extra.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "math/vec.h"
#include "input.h"
#include "graphics/vulkan.h"
#include "timer.h"
#include "graphics/renderer.h"

Window* main_window = NULL;

void draw()
{
	renderer_draw();
}

int application_start()
{
	Timer timer = timer_start(CT_WALL_TICKS);
	time_init();

	main_window = window_create("crescent", 800, 600, WS_WINDOWED);
	if (main_window == NULL)
		return -1;

	input_init(main_window);
	vulkan_init();
	LOG_S("Initialization took %f ms", timer_stop(&timer) * 1000);

	timer_reset(&timer);
	while (!window_get_close(main_window))
	{
		input_update();
		window_update(main_window);
		time_update();
		draw();
		
		if (timer_duration(&timer) > 0.5f)
		{
			LOG("Timer : %f", timer_duration(&timer));
			timer_reset(&timer);
			//LOG("Frame %d, %f/s", time_framecount(), time_framerate());
		}
			SLEEP(0.01);
	}
	LOG_S("Terminating");
	vulkan_terminate();
	window_destroy(main_window);

	return 0;
}

void application_send_event(Event event)
{
	if (event.type == EVENT_KEY)
		LOG("Key pressed  : %d, %c", event.idata[0], event.idata[0]);
	if (event.type == EVENT_KEY || event.type == EVENT_MOUSE_MOVED || event.type == EVENT_MOUSE_SCROLLED)
		input_send_event(&event);
}

void* application_get_window()
{
	return main_window;
}