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

/* maximum crosshair pic filename size */
#define CROSSHAIR_PIC_NAME_LENGTH               12



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* user-controllable settings for a player */
struct crosshair_user_settings
{
	UINT8           used;       /* is used */
	UINT8           mode;       /* visibility mode */
	UINT8           auto_time;  /* time in seconds to blank crosshair if no movement */
	char            name[CROSSHAIR_PIC_NAME_LENGTH + 1];        /* bitmap name */
};


// ======================> crosshair_manager

class crosshair_manager
{
public:
	// construction/destruction
	crosshair_manager(running_machine &machine);

	/* draws crosshair(s) in a given screen, if necessary */
	void render(screen_device &screen);

	/* sets the screen(s) for a given player's crosshair */
	void set_screen(int player, screen_device *screen) { m_screen[player] = screen; }

	/* return TRUE if any crosshairs are used */
	int get_usage() { return m_usage; }

	/* return the current crosshair settings for the given player */
	void get_user_settings(UINT8 player, crosshair_user_settings *settings);

	/* modify the current crosshair settings for the given player */
	void set_user_settings(UINT8 player, crosshair_user_settings *settings);
	// getters
	running_machine &machine() const { return m_machine; }
private:
	void create_bitmap(int player);
	void exit();
	void animate(screen_device &device, bool vblank_state);

	void config_load(config_type cfg_type, xml_data_node *parentnode);
	void config_save(config_type cfg_type, xml_data_node *parentnode);

	// internal state
	running_machine &   m_machine;                  // reference to our machine

	UINT8               m_usage;                  /* true if any crosshairs are used */
	UINT8               m_used[MAX_PLAYERS];      /* usage per player */
	UINT8               m_mode[MAX_PLAYERS];      /* visibility mode per player */
	UINT8               m_visible[MAX_PLAYERS];   /* visibility per player */
	std::unique_ptr<bitmap_argb32>  m_bitmap[MAX_PLAYERS];    /* bitmap per player */
	render_texture *    m_texture[MAX_PLAYERS];   /* texture per player */
	screen_device *     m_screen[MAX_PLAYERS];    /* the screen on which this player's crosshair is drawn */
	float               m_x[MAX_PLAYERS];         /* current X position */
	float               m_y[MAX_PLAYERS];         /* current Y position */
	float               m_last_x[MAX_PLAYERS];    /* last X position */
	float               m_last_y[MAX_PLAYERS];    /* last Y position */
	UINT8               m_fade;                   /* color fading factor */
	UINT8               m_animation_counter;      /* animation frame index */
	UINT16              m_auto_time;              /* time in seconds to turn invisible */
	UINT16              m_time[MAX_PLAYERS];      /* time since last movement */
	char                m_name[MAX_PLAYERS][CROSSHAIR_PIC_NAME_LENGTH + 1];   /* name of crosshair png file */
};

#endif  /* __CRSSHAIR_H__ */
