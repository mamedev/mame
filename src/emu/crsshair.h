// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    crsshair.h

    Crosshair handling.
***************************************************************************/

#pragma once

#ifndef __CRSSHAIR_H__
#define __CRSSHAIR_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_SCREEN_NONE   ((screen_device *) 0)
#define CROSSHAIR_SCREEN_ALL    ((screen_device *) ~0)

/* user settings for visibility mode */
#define CROSSHAIR_VISIBILITY_OFF                0
#define CROSSHAIR_VISIBILITY_ON                 1
#define CROSSHAIR_VISIBILITY_AUTO               2
#define CROSSHAIR_VISIBILITY_DEFAULT            CROSSHAIR_VISIBILITY_AUTO

/* range allowed for auto visibility */
#define CROSSHAIR_VISIBILITY_AUTOTIME_MIN           0
#define CROSSHAIR_VISIBILITY_AUTOTIME_MAX           50
#define CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT       15



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> render_crosshair

class render_crosshair
{
public:
	// construction/destruction
	render_crosshair(running_machine &machine, int player);
	~render_crosshair();

	// getters
	running_machine &machine() const { return m_machine; }
	int player() const { return m_player; }
	bool is_used() const { return m_used; }
	UINT8 mode() const { return m_mode; }
	bool is_visible() const { return m_visible; }
	screen_device *screen() const { return m_screen; }
	float x() const { return m_x; }
	float y() const { return m_y; }
	const char *bitmap_name() const { return m_name.c_str(); }

	// setters
	void set_used(bool used) { m_used = used; }
	void set_mode(UINT8 mode) { m_mode = mode; }
	void set_visible(bool visible) { m_visible = visible; }
	void set_screen(screen_device *screen) { m_screen = screen; }
	//void setxy(float x, float y);
	void set_bitmap_name(const char *name);
	void set_default_bitmap();

	// updates
	void animate(UINT16 auto_time);
	void draw(render_container &container, UINT8 fade);

private:
	// private helpers
	void create_bitmap();

	// private state
	running_machine &   m_machine;  // reference to our machine
	int                 m_player;   // player number
	bool                m_used;     // usage for this crosshair
	UINT8               m_mode;     // visibility mode for this crosshair
	bool                m_visible;  // visibility for this crosshair
	std::unique_ptr<bitmap_argb32>  m_bitmap;    // bitmap for this crosshair
	render_texture *    m_texture;  // texture for this crosshair
	screen_device *     m_screen;   // the screen on which this crosshair is drawn
	float               m_x;        // current X position
	float               m_y;        // current Y position
	float               m_last_x;   // last X position
	float               m_last_y;   // last Y position
	UINT16              m_time;     // time since last movement
	std::string         m_name;     // name of png file
};


// ======================> crosshair_manager

class crosshair_manager
{
public:
	// construction/destruction
	crosshair_manager(running_machine &machine);

	/* draws crosshair(s) in a given screen, if necessary */
	void render(screen_device &screen);

	// return true if any crosshairs are used
	bool get_usage() const { return m_usage; }

	// getters
	running_machine &machine() const { return m_machine; }
	render_crosshair &get_crosshair(int player) const { assert(player >= 0 && player < MAX_PLAYERS); assert(m_crosshair[player] != nullptr); return *m_crosshair[player]; }
	UINT16 auto_time() const { return m_auto_time; }
	void set_auto_time(UINT16 auto_time) { m_auto_time = auto_time; }

private:
	void exit();
	void animate(screen_device &device, bool vblank_state);

	void config_load(config_type cfg_type, xml_data_node *parentnode);
	void config_save(config_type cfg_type, xml_data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	bool                m_usage;                    // true if any crosshairs are used
	std::unique_ptr<render_crosshair> m_crosshair[MAX_PLAYERS]; // per-player crosshair state
	UINT8               m_fade;                     // color fading factor
	UINT8               m_animation_counter;        // animation frame index
	UINT16              m_auto_time;                // time in seconds to turn invisible
};

#endif  /* __CRSSHAIR_H__ */
