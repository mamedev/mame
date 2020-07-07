
//============================================================
//
//  window.c - RETRO window handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// standard C headers
#include <math.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <list>
#include <memory>

// MAME headers

#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "ui/uimain.h"

// OSD headers

#include "window.h"
#include "osdretro.h"

#include "modules/render/drawretro.h"
#include "modules/monitor/monitor_common.h"


//============================================================
//  PARAMETERS
//============================================================

// these are arbitrary values since AFAIK there's no way to make X/SDL tell you
#define WINDOW_DECORATION_WIDTH (8) // should be more than plenty
#define WINDOW_DECORATION_HEIGHT (48)   // title bar + bottom drag region

// minimum window dimension
#define MIN_WINDOW_DIM                  200

#ifndef SDLMAME_WIN32
#define WMSZ_TOP            (0)
#define WMSZ_BOTTOM         (1)
#define WMSZ_BOTTOMLEFT     (2)
#define WMSZ_BOTTOMRIGHT    (3)
#define WMSZ_LEFT           (4)
#define WMSZ_TOPLEFT        (5)
#define WMSZ_TOPRIGHT       (6)
#define WMSZ_RIGHT          (7)
#endif

#define SDL_VERSION_EQUALS(v1, vnum2) (SDL_VERSIONNUM(v1.major, v1.minor, v1.patch) == vnum2)

typedef struct
{          
	int w;                      
	int h;                      
	int refresh_rate;           
        
} RETRO_DisplayMode;

class RETRO_DM_Wrapper
{
public:
	RETRO_DisplayMode mode;
};

// debugger
//static int in_background;


//============================================================
//  PROTOTYPES
//============================================================


//============================================================
//  window_init
//  (main thread)
//============================================================

bool retro_osd_interface::window_init()
{
	osd_printf_verbose("Enter sdlwindow_init\n");

	// initialize the drawers

	switch (video_config.mode)
	{

		case VIDEO_MODE_SOFT:
			renderer_retro::init(machine());
			break;
	}

	// set up the window list
	osd_printf_verbose("Leave sdlwindow_init\n");
	return true;
}


void retro_osd_interface::update_slider_list()
{
	for (auto window : osd_common_t::s_window_list)
	{
		// check if any window has dirty sliders
		if (window->renderer().sliders_dirty())
		{
			build_slider_list();
			return;
		}
	}
}

void retro_osd_interface::build_slider_list()
{
	m_sliders.clear();

	for (auto window : osd_common_t::s_window_list)
	{
		std::vector<ui::menu_item> window_sliders = window->renderer().get_slider_list();
		m_sliders.insert(m_sliders.end(), window_sliders.begin(), window_sliders.end());
	}
}

//============================================================
//  window_exit
//  (main thread)
//============================================================

void retro_osd_interface::window_exit()
{
	osd_printf_verbose("Enter sdlwindow_exit\n");

	// free all the windows
	while (!osd_common_t::s_window_list.empty())
	{
		auto window = osd_common_t::s_window_list.front();

		// Part of destroy removes the window from the list
		window->destroy();
	}

	switch(video_config.mode)
	{

		case VIDEO_MODE_SOFT:
			renderer_retro::exit();
			break;

		default:
			break;
	}
	osd_printf_verbose("Leave sdlwindow_exit\n");
}


void retro_window_info::capture_pointer()
{
	if (!m_mouse_captured)
	{
//		SDL_SetWindowGrab(platform_window<SDL_Window*>(), SDL_TRUE);
//		SDL_SetRelativeMouseMode(SDL_TRUE);
		m_mouse_captured = true;
	}
}

void retro_window_info::release_pointer()
{
	if (m_mouse_captured)
	{
//		SDL_SetWindowGrab(platform_window<SDL_Window*>(), SDL_FALSE);
//		SDL_SetRelativeMouseMode(SDL_FALSE);
		m_mouse_captured = false;
	}
}

void retro_window_info::hide_pointer()
{
	if (!m_mouse_hidden)
	{
//		SDL_ShowCursor(SDL_DISABLE);
		m_mouse_hidden = true;
	}
}

void retro_window_info::show_pointer()
{
	if (m_mouse_hidden)
	{
//		SDL_ShowCursor(SDL_ENABLE);
		m_mouse_hidden = false;
	}
}


//============================================================
//  sdlwindow_resize
//============================================================
extern int NEWGAME_FROM_OSD;
static int first_time=1;

void retro_window_info::resize(int32_t width, int32_t height)
{
	osd_dim cd = get_size();

	printf("resize-it %f\n",m_monitor->aspect());

	if (width != cd.width() || height != cd.height() ||first_time==1)
	{
//		SDL_SetWindowSize(platform_window<SDL_Window*>(), width, height);

         fb_width=width; 
         fb_height=height;
         fb_pitch=width;
         retro_aspect =m_monitor->pixel_aspect();// m_monitor->/*pixel_*/aspect();//(float)width/(float)height;
	 NEWGAME_FROM_OSD  = 1;
	 first_time =0;
		renderer().notify_changed();
	}
}


//============================================================
//  sdlwindow_clear_surface
//============================================================

void retro_window_info::notify_changed()
{
	printf("notify\n");
	renderer().notify_changed();
}


//============================================================
//  sdlwindow_toggle_full_screen
//============================================================

void retro_window_info::toggle_full_screen()
{
#if 0

	// if we are in debug mode, never go full screen
	if (machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		return;

	// If we are going fullscreen (leaving windowed) remember our windowed size
	if (!fullscreen())
	{
		m_windowed_dim = get_size();
	}

	// reset UI to main menu
	machine().ui().menu_reset();
	// kill off the drawers
	renderer_reset();
	set_platform_window(nullptr);
	bool is_osx = false;
#ifdef SDLMAME_MACOSX
	// FIXME: This is weird behaviour and certainly a bug in SDL
	is_osx = true;
#endif
	if (fullscreen() && (video_config.switchres || is_osx))
	{
		SDL_SetWindowFullscreen(platform_window<SDL_Window*>(), 0);    // Try to set mode
		SDL_SetWindowDisplayMode(platform_window<SDL_Window*>(), &m_original_mode->mode);    // Try to set mode
		SDL_SetWindowFullscreen(platform_window<SDL_Window*>(), SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}
	SDL_DestroyWindow(platform_window<SDL_Window*>());

	downcast<sdl_osd_interface &>(machine().osd()).release_keys();

	set_renderer(osd_renderer::make_for_type(video_config.mode, shared_from_this()));

	// toggle the window mode
	set_fullscreen(!fullscreen());

	complete_create();

#else
	return;
#endif
}

void retro_window_info::modify_prescale(int dir)
{
	int new_prescale = prescale();

	if (dir > 0 && prescale() < 3)
		new_prescale = prescale() + 1;
	if (dir < 0 && prescale() > 1)
		new_prescale = prescale() - 1;

	if (new_prescale != prescale())
	{
		if (m_fullscreen && video_config.switchres)
		{
			complete_destroy();

			m_prescale = new_prescale;

			complete_create();
		}
		else
		{
			notify_changed();
			m_prescale = new_prescale;
		}
		machine().ui().popup_time(1, "Prescale %d", prescale());
	}
}

//============================================================
//  sdlwindow_update_cursor_state
//  (main or window thread)
//============================================================

void retro_window_info::update_cursor_state()
{
#if 0

#if (USE_XINPUT)
	// Hack for wii-lightguns:
	// they stop working with a grabbed mouse;
	// even a ShowCursor(SDL_DISABLE) already does this.
	// To make the cursor disappear, we'll just set an empty cursor image.
	unsigned char data[]={0,0,0,0,0,0,0,0};
	SDL_Cursor *c;
	c=SDL_CreateCursor(data, data, 8, 8, 0, 0);
	SDL_SetCursor(c);
#else
	// do not do mouse capture if the debugger's enabled to avoid
	// the possibility of losing control
	if (!(machine().debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		bool should_hide_mouse = downcast<sdl_osd_interface&>(machine().osd()).should_hide_mouse();

		if (!fullscreen() && !should_hide_mouse)
		{
			show_pointer();
			release_pointer();
		}
		else
		{
			hide_pointer();
			capture_pointer();
		}

		SDL_SetCursor(nullptr); // Force an update in case the underlying driver has changed visibility
	}
#endif
#else
	return;
#endif
}

int retro_window_info::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	return renderer().xy_to_render_target(x, y, xt, yt);
}

//============================================================
//  sdlwindow_video_window_create
//  (main thread)
//============================================================

int retro_window_info::window_init()
{
	int result;
	float oldfps;

	// set the initial maximized state
	// FIXME: Does not belong here
	retro_options &options = downcast<retro_options &>(m_machine.options());
	m_startmaximized = options.maximize();

	// add us to the list
	osd_common_t::s_window_list.push_back(std::static_pointer_cast<retro_window_info>(shared_from_this()));

	set_renderer(osd_renderer::make_for_type(video_config.mode, static_cast<osd_window*>(this)->shared_from_this()));

	// load the layout
	m_target = m_machine.render().target_alloc();

	// set the specific view
	set_starting_view(m_index, options.view(), options.view(m_index));

	// make the window title
	if (video_config.numscreens == 1)
		sprintf(m_title, "%s: %s [%s]", emulator_info::get_appname(), m_machine.system().type.fullname(), m_machine.system().name);
	else
		sprintf(m_title, "%s: %s [%s] - Screen %d", emulator_info::get_appname(), m_machine.system().type.fullname(), m_machine.system().name, m_index);

	result = complete_create();
	
	oldfps=retro_fps;

    const screen_device *primary_screen = screen_device_iterator(machine().root_device()).first();

    if (primary_screen != nullptr){
        retro_fps = ATTOSECONDS_TO_HZ(primary_screen->refresh_attoseconds());
	}

	if(alternate_renderer==false){
	//test correct aspect
		render_layer_config temp=m_target->layer_config();
		retro_aspect =m_target->current_view()->effective_aspect(temp);

		if(m_target->orientation() & ORIENTATION_SWAP_XY)retro_aspect=1.0/retro_aspect;

		int tempwidth, tempheight;
		m_target->compute_minimum_size(tempwidth, tempheight);
		fb_width=tempwidth;
		fb_pitch=tempwidth;
		fb_height=tempheight;

		if(fb_width>max_width || fb_height>max_height || oldfps!=retro_fps)
			NEWGAME_FROM_OSD = 1;
		else NEWGAME_FROM_OSD = 2;

	}

	// handle error conditions
	if (result == 1)
		goto error;

	return 0;

error:
	destroy();
	return 1;
}


//============================================================
//  sdlwindow_video_window_destroy
//============================================================

void retro_window_info::complete_destroy()
{
	// Release pointer grab and hide if needed
	show_pointer();
	release_pointer();

	// release all keys ...
	downcast<retro_osd_interface &>(machine().osd()).release_keys();
}

void retro_window_info::destroy()
{
	//osd_event_wait(window->rendered_event, osd_ticks_per_second()*10);

	// remove us from the list
	osd_common_t::s_window_list.remove(std::static_pointer_cast<retro_window_info>(shared_from_this()));

	// free the textures etc
	complete_destroy();

	// free the render target, after the textures!
	machine().render().target_free(m_target);

}


//============================================================
//  pick_best_mode
//============================================================

osd_dim retro_window_info::pick_best_mode()
{
   int minimum_width, minimum_height, target_width, target_height;

   osd_dim ret(0,0);

   // determine the minimum width/height for the selected target
   m_target->compute_minimum_size(minimum_width, minimum_height);

   // use those as the target for now
   target_width = minimum_width * std::max(1, prescale());
   target_height = minimum_height * std::max(1, prescale());

   // if we're not stretching, allow some slop on the minimum since we can handle it
   {
      minimum_width -= 4;
      minimum_height -= 4;
   }

   //FIXME RETRO
   ret = osd_dim(target_width,target_height);
   osd_printf_verbose("**********************%4dx%4d@%2d -> %f\n", (int)target_width, (int)target_height,0,(double)0);
   return ret;
}

//============================================================
//  sdlwindow_video_window_update
//  (main thread)
//============================================================

void retro_window_info::update()
{
	osd_ticks_t     event_wait_ticks;
	
	// adjust the cursor state
	//sdlwindow_update_cursor_state(machine, window);

	update_cursor_state();

	// if we're visible and running and not in the middle of a resize, draw
	if (m_target != nullptr)
	{
		int tempwidth, tempheight;

		if(alternate_renderer==false){
			render_layer_config temp=m_target->layer_config();
			view_aspect =m_target->current_view()->effective_aspect(temp);
			if(m_target->orientation() & ORIENTATION_SWAP_XY)view_aspect=1.0f/view_aspect;
		}

		// see if the games video mode has changed
		m_target->compute_minimum_size(tempwidth, tempheight);
		if (osd_dim(tempwidth, tempheight) != m_minimum_dim || view_aspect!=retro_aspect)
		{
			m_minimum_dim = osd_dim(tempwidth, tempheight);


			if(alternate_renderer==false)
			{

				fb_width=tempwidth;
				fb_pitch=tempwidth;
				fb_height=tempheight;

				//if(video_changed==true)
				{
				//retro_aspect = (float)tempwidth/(float)tempheight;

				render_layer_config temp=m_target->layer_config();
				retro_aspect =m_target->current_view()->effective_aspect(temp);
				if(m_target->orientation() & ORIENTATION_SWAP_XY)retro_aspect=1.0/retro_aspect;
				view_aspect =retro_aspect;
				monitor()->refresh();
				monitor()->update_resolution(tempwidth, tempheight);
				//osd_printf_info("(%dx%d)as:%f rot:%d %d\n",tempwidth, tempheight,retro_aspect,m_target->orientation(),m_target->orientation() & ORIENTATION_SWAP_XY);

				if(fb_width>max_width || fb_height>max_height)
					NEWGAME_FROM_OSD = 1;
				else (NEWGAME_FROM_OSD==1)?NEWGAME_FROM_OSD = 1:NEWGAME_FROM_OSD = 2;

					video_changed=false;
				}

			}

			if (!this->m_fullscreen)
			{
				//Don't resize window without user interaction;
				//window_resize(blitwidth, blitheight);
			}
			else if (video_config.switchres)
			{
				osd_dim tmp = this->pick_best_mode();
				resize(tmp.width(), tmp.height());
			}
		}

		if (video_config.waitvsync && video_config.syncrefresh)
			event_wait_ticks = osd_ticks_per_second(); // block at most a second
		else
			event_wait_ticks = 0;

		if (m_rendered_event.wait(event_wait_ticks))
		{
			const int update = 1;

			// ensure the target bounds are up-to-date, and then get the primitives

			render_primitive_list &primlist = *renderer().get_primitives();

			// and redraw now

			// Some configurations require events to be polled in the worker thread
//FIXME RETRO
		//	downcast< retro_osd_interface& >(machine().osd()).process_events_buf();

			// Check whether window has vector screens

			{
				const screen_device *screen = screen_device_iterator(machine().root_device()).byindex(m_index);
				if ((screen != nullptr) && (screen->screen_type() == SCREEN_TYPE_VECTOR))
					renderer().set_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
				else
					renderer().clear_flags(osd_renderer::FLAG_HAS_VECTOR_SCREEN);
			}

			m_primlist = &primlist;

			// if no bitmap, just fill
			if (m_primlist == nullptr)
			{
			}
			// otherwise, render with our drawing system
			else
			{
				if( video_config.perftest )
					measure_fps(update);
				else
					renderer().draw(update);
			}

			/* all done, ready for next */
			m_rendered_event.set();
		}
	}
}


//============================================================
//  set_starting_view
//  (main thread)
//============================================================

void retro_window_info::set_starting_view(int index, const char *defview, const char *view)
{
	int viewindex;

	// choose non-auto over auto
	if (!strcmp(view, "auto") && !strcmp(defview, "auto") != 0)
		view = defview;

	// query the video system to help us pick a view
	viewindex = target()->configured_view(view, index, video_config.numscreens);

	// set the view
	target()->set_view(viewindex);
}


//============================================================
//  complete_create
//============================================================

int retro_window_info::complete_create()
{
	osd_dim temp(0,0);

	// clear out original mode. Needed on OSX
	if (fullscreen())
	{
		// default to the current mode exactly
		temp = monitor()->position_size().dim();

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			temp = pick_best_mode();
	}
	else if (m_windowed_dim.width() > 0)
	{
		// if we have a remembered size force the new window size to it
		temp = m_windowed_dim;
	}
	else if (m_startmaximized)
		temp = get_max_bounds(video_config.keepaspect );
	else
		temp = get_min_bounds(video_config.keepaspect );

	// create the window .....

	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */
	osd_printf_verbose("Enter sdl_info::create\n");

		m_extra_flags = 0;


#if 0
	// get monitor work area for centering
	osd_rect work = monitor()->usuable_position_size();
	//set_platform_window(retrowindow);
#endif

	// set main window
	if (m_index > 0)
	{
		for (auto w : osd_common_t::s_window_list)
		{
			if (w->m_index == 0)
			{
				set_main_window(std::dynamic_pointer_cast<osd_window>(w));
				break;
			}
		}
	}
	else
	{
		// We must be the main window
		set_main_window(shared_from_this());
	}

	// update monitor resolution after mode change to ensure proper pixel aspect
	monitor()->refresh();
	if (fullscreen() && video_config.switchres)
		monitor()->update_resolution(temp.width(), temp.height());

	// initialize the drawing backend
	if (renderer().create())
		return 1;

	// Make sure we have a consistent state

	//SDL_ShowCursor(0);
	//SDL_ShowCursor(1);

	return 0;
}


//============================================================
//  draw_video_contents
//  (window thread)
//============================================================

void retro_window_info::measure_fps(int update)
{
	const unsigned long frames_skip4fps = 100;
	static int64_t lastTime=0, sumdt=0, startTime=0;
	static unsigned long frames = 0;
	int64_t currentTime, t0;
	double dt;
	double tps;
	osd_ticks_t tps_t;

	tps_t = osd_ticks_per_second();
	tps = (double) tps_t;

	t0 = osd_ticks();

	renderer().draw(update);

	frames++;
	currentTime = osd_ticks();
	if(startTime==0||frames==frames_skip4fps)
		startTime=currentTime;
	if( frames>=frames_skip4fps )
		sumdt+=currentTime-t0;
	if( (currentTime-lastTime)>1L*osd_ticks_per_second() && frames>frames_skip4fps )
	{
		dt = (double) (currentTime-startTime) / tps; // in decimale sec.
		osd_printf_info("%6.2lfs, %4lu F, "
				"avrg game: %5.2lf FPS %.2lf ms/f, "
				"avrg video: %5.2lf FPS %.2lf ms/f, "
				"last video: %5.2lf FPS %.2lf ms/f\n",
			dt, frames-frames_skip4fps,
			(double)(frames-frames_skip4fps)/dt,                             // avrg game fps
			( (currentTime-startTime) / ((frames-frames_skip4fps)) ) * 1000.0 / osd_ticks_per_second(),
			(double)(frames-frames_skip4fps)/((double)(sumdt) / tps), // avrg vid fps
			( sumdt / ((frames-frames_skip4fps)) ) * 1000.0 / tps,
			1.0/((currentTime-t0) / osd_ticks_per_second()), // this vid fps
			(currentTime-t0) * 1000.0 / tps
		);
		lastTime = currentTime;
	}
}

int retro_window_info::wnd_extra_width()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_WIDTH;
}

int retro_window_info::wnd_extra_height()
{
	return m_fullscreen ? 0 : WINDOW_DECORATION_HEIGHT;
}


//============================================================
//  constrain_to_aspect_ratio
//  (window thread)
//============================================================

osd_rect retro_window_info::constrain_to_aspect_ratio(const osd_rect &rect, int adjustment)
{
	int32_t extrawidth = wnd_extra_width();
	int32_t extraheight = wnd_extra_height();
	int32_t propwidth, propheight;
	int32_t minwidth, minheight;
	int32_t maxwidth, maxheight;
	int32_t viswidth, visheight;
	int32_t adjwidth, adjheight;
	float pixel_aspect;
	std::shared_ptr<osd_monitor_info> monitor = m_monitor;

	// do not constrain aspect ratio for integer scaled views
	if (m_target->scale_mode() != SCALE_FRACTIONAL)
		return rect;

	// get the pixel aspect ratio for the target monitor

	//FIXME: RETRO set it to 1 seem to correct the aspect. (alternate renderer)
	pixel_aspect = monitor->pixel_aspect();

	// determine the proposed width/height
	propwidth = rect.width() - extrawidth;
	propheight = rect.height() - extraheight;

	// based on which edge we are adjusting, take either the width, height, or both as gospel
	// and scale to fit using that as our parameter
	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_TOP:
			m_target->compute_visible_area(10000, propheight, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			m_target->compute_visible_area(propwidth, 10000, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;

		default:
			m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), propwidth, propheight);
			break;
	}

	// get the minimum width/height for the current layout
	m_target->compute_minimum_size(minwidth, minheight);

	// clamp against the absolute minimum
	propwidth = std::max(propwidth, MIN_WINDOW_DIM);
	propheight = std::max(propheight, MIN_WINDOW_DIM);

	// clamp against the minimum width and height
	propwidth = std::max(propwidth, minwidth);
	propheight = std::max(propheight, minheight);

	// clamp against the maximum (fit on one screen for full screen mode)
	if (m_fullscreen)
	{
		maxwidth = monitor->position_size().width() - extrawidth;
		maxheight = monitor->position_size().height() - extraheight;
	}
	else
	{
		maxwidth = monitor->usuable_position_size().width() - extrawidth;
		maxheight = monitor->usuable_position_size().height() - extraheight;

		// further clamp to the maximum width/height in the window
		if (m_win_config.width != 0)
			maxwidth = std::min(maxwidth, m_win_config.width + extrawidth);
		if (m_win_config.height != 0)
			maxheight = std::min(maxheight, m_win_config.height + extraheight);
	}

	// clamp to the maximum
	propwidth = std::min(propwidth, maxwidth);
	propheight = std::min(propheight, maxheight);

	// compute the visible area based on the proposed rectangle
	m_target->compute_visible_area(propwidth, propheight, pixel_aspect, m_target->orientation(), viswidth, visheight);

	// compute the adjustments we need to make
	adjwidth = (viswidth + extrawidth) - rect.width();
	adjheight = (visheight + extraheight) - rect.height();

	// based on which corner we're adjusting, constrain in different ways
	osd_rect ret(rect);

	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
			ret = rect.resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_BOTTOMLEFT:
			ret = rect.move_by(-adjwidth, 0).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
		case WMSZ_TOP:
			ret = rect.move_by(-adjwidth, -adjheight).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;

		case WMSZ_TOPRIGHT:
			ret = rect.move_by(0, -adjheight).resize(rect.width() + adjwidth, rect.height() + adjheight);
			break;
}
	return ret;
}



//============================================================
//  get_min_bounds
//  (window thread)
//============================================================

osd_dim retro_window_info::get_min_bounds(int constrain)
{
	int32_t minwidth, minheight;

	//assert(GetCurrentThreadId() == window_threadid);

	// get the minimum target size
	m_target->compute_minimum_size(minwidth, minheight);

	// expand to our minimum dimensions
	if (minwidth < MIN_WINDOW_DIM)
		minwidth = MIN_WINDOW_DIM;
	if (minheight < MIN_WINDOW_DIM)
		minheight = MIN_WINDOW_DIM;

	// account for extra window stuff
	minwidth += wnd_extra_width();
	minheight += wnd_extra_height();

	// if we want it constrained, figure out which one is larger
	if (constrain && m_target->scale_mode() == SCALE_FRACTIONAL)
	{
		// first constrain with no height limit
		osd_rect test1(0,0,minwidth,10000);
		test1 = constrain_to_aspect_ratio(test1, WMSZ_BOTTOMRIGHT);

		// then constrain with no width limit
		osd_rect test2(0,0,10000,minheight);
		test2 = constrain_to_aspect_ratio(test2, WMSZ_BOTTOMRIGHT);

		// pick the larger
		if (test1.width() > test2.width())
		{
			minwidth = test1.width();
			minheight = test1.height();
		}
		else
		{
			minwidth = test2.width();
			minheight = test2.height();
		}
	}

	// remove extra window stuff
	minwidth -= wnd_extra_width();
	minheight -= wnd_extra_height();

	return osd_dim(minwidth, minheight);
}

//============================================================
//  get_size
//============================================================

osd_dim retro_window_info::get_size()
{
	int w=0; int h=0;
//	SDL_GetWindowSize(platform_window<SDL_Window*>(), &w, &h);

         w=fb_width; 
         h=fb_height;

	return osd_dim(w,h);
}


//============================================================
//  get_max_bounds
//  (window thread)
//============================================================

osd_dim retro_window_info::get_max_bounds(int constrain)
{
	//assert(GetCurrentThreadId() == window_threadid);

	// compute the maximum client area
	// m_monitor->refresh();
	osd_rect maximum = m_monitor->usuable_position_size();

	// clamp to the window's max
	int tempw = maximum.width();
	int temph = maximum.height();
	if (m_win_config.width != 0)
	{
		int temp = m_win_config.width + wnd_extra_width();
		if (temp < maximum.width())
			tempw = temp;
	}
	if (m_win_config.height != 0)
	{
		int temp = m_win_config.height + wnd_extra_height();
		if (temp < maximum.height())
			temph = temp;
	}

	maximum = maximum.resize(tempw, temph);

	// constrain to fit
	if (constrain && m_target->scale_mode() == SCALE_FRACTIONAL)
		maximum = constrain_to_aspect_ratio(maximum, WMSZ_BOTTOMRIGHT);

	// remove extra window stuff
	maximum = maximum.resize(maximum.width() - wnd_extra_width(), maximum.height() - wnd_extra_height());

	return maximum.dim();
}

//============================================================
//  construction and destruction
//============================================================

retro_window_info::retro_window_info(
		running_machine &a_machine,
		int index,
		std::shared_ptr<osd_monitor_info> a_monitor,
		const osd_window_config *config)
	: osd_window(*config)
	, m_next(nullptr)
	, m_startmaximized(0)
	// Following three are used by input code to defer resizes
	, m_minimum_dim(0, 0)
	, m_windowed_dim(0, 0)
	, m_rendered_event(0, 1)
	, m_target(nullptr)
	, m_extra_flags(0)
	, m_machine(a_machine)
	, m_monitor(a_monitor)
	, m_fullscreen(0)
	, m_mouse_captured(false)
	, m_mouse_hidden(false)
{
	m_index = index;

	//FIXME: these should be per_window in config-> or even better a bit set
	m_fullscreen = !video_config.windowed;
	m_prescale = video_config.prescale;

	m_windowed_dim = osd_dim(config->width, config->height);
	m_original_mode = global_alloc(RETRO_DM_Wrapper);
}

retro_window_info::~retro_window_info()
{
	global_free(m_original_mode);
}

//============================================================
//  osd_set_aggressive_input_focus
//============================================================

void osd_set_aggressive_input_focus(bool aggressive_focus)
{
	// dummy implementation for now?
}
