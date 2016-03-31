// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    crsshair.c

    Crosshair handling.
***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "rendutil.h"
#include "config.h"
#include "xmlfile.h"
#include "crsshair.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_RAW_SIZE      100
#define CROSSHAIR_RAW_ROWBYTES  ((CROSSHAIR_RAW_SIZE + 7) / 8)


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* raw bitmap */
static const UINT8 crosshair_raw_top[] =
{
	0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,
	0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x00,
	0x00,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf0,0x00,
	0x01,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf8,0x00,
	0x03,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xfc,0x00,
	0x07,0xfe,0x00,0x00,0x00,0x0f,0xfe,0x00,0x00,0x00,0x07,0xfe,0x00,
	0x0f,0xff,0x00,0x00,0x01,0xff,0xff,0xf0,0x00,0x00,0x0f,0xff,0x00,
	0x1f,0xff,0x80,0x00,0x1f,0xff,0xff,0xff,0x00,0x00,0x1f,0xff,0x80,
	0x3f,0xff,0x80,0x00,0xff,0xff,0xff,0xff,0xe0,0x00,0x1f,0xff,0xc0,
	0x7f,0xff,0xc0,0x03,0xff,0xff,0xff,0xff,0xf8,0x00,0x3f,0xff,0xe0,
	0xff,0xff,0xe0,0x07,0xff,0xff,0xff,0xff,0xfc,0x00,0x7f,0xff,0xf0,
	0x7f,0xff,0xf0,0x1f,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xe0,
	0x3f,0xff,0xf8,0x7f,0xff,0xff,0xff,0xff,0xff,0xc1,0xff,0xff,0xc0,
	0x0f,0xff,0xf8,0xff,0xff,0xff,0xff,0xff,0xff,0xe1,0xff,0xff,0x00,
	0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfb,0xff,0xfe,0x00,
	0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x00,
	0x01,0xff,0xff,0xff,0xff,0xf0,0x01,0xff,0xff,0xff,0xff,0xf8,0x00,
	0x00,0x7f,0xff,0xff,0xff,0x00,0x00,0x1f,0xff,0xff,0xff,0xe0,0x00,
	0x00,0x3f,0xff,0xff,0xf8,0x00,0x00,0x03,0xff,0xff,0xff,0xc0,0x00,
	0x00,0x1f,0xff,0xff,0xe0,0x00,0x00,0x00,0xff,0xff,0xff,0x80,0x00,
	0x00,0x0f,0xff,0xff,0x80,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,
	0x00,0x03,0xff,0xfe,0x00,0x00,0x00,0x00,0x0f,0xff,0xfc,0x00,0x00,
	0x00,0x01,0xff,0xfc,0x00,0x00,0x00,0x00,0x07,0xff,0xf8,0x00,0x00,
	0x00,0x03,0xff,0xf8,0x00,0x00,0x00,0x00,0x01,0xff,0xf8,0x00,0x00,
	0x00,0x07,0xff,0xfc,0x00,0x00,0x00,0x00,0x03,0xff,0xfc,0x00,0x00,
	0x00,0x0f,0xff,0xfe,0x00,0x00,0x00,0x00,0x07,0xff,0xfe,0x00,0x00,
	0x00,0x0f,0xff,0xff,0x00,0x00,0x00,0x00,0x0f,0xff,0xfe,0x00,0x00,
	0x00,0x1f,0xff,0xff,0x80,0x00,0x00,0x00,0x1f,0xff,0xff,0x00,0x00,
	0x00,0x1f,0xff,0xff,0x80,0x00,0x00,0x00,0x1f,0xff,0xff,0x00,0x00,
	0x00,0x3f,0xfe,0xff,0xc0,0x00,0x00,0x00,0x3f,0xff,0xff,0x80,0x00,
	0x00,0x7f,0xfc,0x7f,0xe0,0x00,0x00,0x00,0x7f,0xe7,0xff,0xc0,0x00,
	0x00,0x7f,0xf8,0x3f,0xf0,0x00,0x00,0x00,0xff,0xc3,0xff,0xc0,0x00,
	0x00,0xff,0xf8,0x1f,0xf8,0x00,0x00,0x01,0xff,0x83,0xff,0xe0,0x00,
	0x00,0xff,0xf0,0x07,0xf8,0x00,0x00,0x01,0xfe,0x01,0xff,0xe0,0x00,
	0x00,0xff,0xf0,0x03,0xfc,0x00,0x00,0x03,0xfc,0x01,0xff,0xe0,0x00,
	0x01,0xff,0xe0,0x01,0xfe,0x00,0x00,0x07,0xf8,0x00,0xff,0xf0,0x00,
	0x01,0xff,0xe0,0x00,0xff,0x00,0x00,0x0f,0xf0,0x00,0xff,0xf0,0x00,
	0x01,0xff,0xc0,0x00,0x3f,0x80,0x00,0x1f,0xc0,0x00,0x7f,0xf0,0x00,
	0x01,0xff,0xc0,0x00,0x1f,0x80,0x00,0x1f,0x80,0x00,0x7f,0xf0,0x00,
	0x03,0xff,0xc0,0x00,0x0f,0xc0,0x00,0x3f,0x00,0x00,0x7f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x07,0xe0,0x00,0x7e,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x01,0xf0,0x00,0xf8,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x00,0xf8,0x01,0xf0,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x00,0x78,0x01,0xe0,0x00,0x00,0x3f,0xf8,0x00,
	0x07,0xff,0x00,0x00,0x00,0x3c,0x03,0xc0,0x00,0x00,0x3f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x0e,0x07,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x07,0x0e,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x03,0x9c,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x01,0x98,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x1f,0xfc,0x00
};

/* per-player colors */
static const rgb_t crosshair_colors[] =
{
	rgb_t(0x40,0x40,0xff),
	rgb_t(0xff,0x40,0x40),
	rgb_t(0x40,0xff,0x40),
	rgb_t(0xff,0xff,0x40),
	rgb_t(0xff,0x40,0xff),
	rgb_t(0x40,0xff,0xff),
	rgb_t(0xff,0xff,0xff)
};

//**************************************************************************
//  CROSSHAIR MANAGER
//**************************************************************************

//-------------------------------------------------
//  crosshair_manager - constructor
//-------------------------------------------------

crosshair_manager::crosshair_manager(running_machine &machine)
	: m_machine(machine)
{
	/* request a callback upon exiting */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(crosshair_manager::exit), this));

	/* setup the default auto visibility time */
	m_auto_time = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;

	/* determine who needs crosshairs */
	for (ioport_port &port : machine.ioport().ports())
		for (ioport_field &field : port.fields())
			if (field.crosshair_axis() != CROSSHAIR_AXIS_NONE)
			{
				int player = field.player();

				assert(player < MAX_PLAYERS);

				/* mark as used and set the default visibility and mode */
				m_usage = TRUE;
				m_used[player] = TRUE;
				m_mode[player] = CROSSHAIR_VISIBILITY_DEFAULT;
				m_visible[player] = (CROSSHAIR_VISIBILITY_DEFAULT == CROSSHAIR_VISIBILITY_OFF) ? FALSE : TRUE;

				/* for now, use the main screen */
				m_screen[player] = machine.first_screen();

				create_bitmap(player);
			}

	/* register callbacks for when we load/save configurations */
	if (m_usage)
		machine.configuration().config_register("crosshairs", config_saveload_delegate(FUNC(crosshair_manager::config_load), this), config_saveload_delegate(FUNC(crosshair_manager::config_save), this));

	/* register the animation callback */
	if (machine.first_screen() != nullptr)
		machine.first_screen()->register_vblank_callback(vblank_state_delegate(FUNC(crosshair_manager::animate), this));
}

/*-------------------------------------------------
exit - free memory allocated for
the crosshairs
-------------------------------------------------*/

void crosshair_manager::exit()
{
	/* free bitmaps and textures for each player */
	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		machine().render().texture_free(m_texture[player]);
		m_texture[player] = nullptr;
		m_bitmap[player] = nullptr;
	}
}

/*-------------------------------------------------
    create_bitmap - create the rendering
    structures for the given player
-------------------------------------------------*/

void crosshair_manager::create_bitmap(int player)
{
	int x, y;
	char filename[20];
	rgb_t color = crosshair_colors[player];

	/* if we have a bitmap and texture for this player, kill it */
	if (m_bitmap[player] == nullptr) {
		m_bitmap[player] = std::make_unique<bitmap_argb32>();
		m_texture[player] = machine().render().texture_alloc(render_texture::hq_scale);
	}

	emu_file crossfile(machine().options().crosshair_path(), OPEN_FLAG_READ);
	if (m_name[player][0] != 0)
	{
		/* look for user specified file */
		sprintf(filename, "%s.png", m_name[player]);
		render_load_png(*m_bitmap[player], crossfile, nullptr, filename);
	}
	else
	{
		/* look for default cross?.png in crsshair\game dir */
		sprintf(filename, "cross%d.png", player + 1);
		render_load_png(*m_bitmap[player], crossfile, machine().system().name, filename);

		/* look for default cross?.png in crsshair dir */
		if (!m_bitmap[player]->valid())
			render_load_png(*m_bitmap[player], crossfile, nullptr, filename);
	}

	/* if that didn't work, use the built-in one */
	if (!m_bitmap[player]->valid())
	{
		/* allocate a blank bitmap to start with */
		m_bitmap[player]->allocate(CROSSHAIR_RAW_SIZE, CROSSHAIR_RAW_SIZE);
		m_bitmap[player]->fill(rgb_t(0x00,0xff,0xff,0xff));

		/* extract the raw source data to it */
		for (y = 0; y < CROSSHAIR_RAW_SIZE / 2; y++)
		{
			/* assume it is mirrored vertically */
			UINT32 *dest0 = &m_bitmap[player]->pix32(y);
			UINT32 *dest1 = &m_bitmap[player]->pix32(CROSSHAIR_RAW_SIZE - 1 - y);

			/* extract to two rows simultaneously */
			for (x = 0; x < CROSSHAIR_RAW_SIZE; x++)
				if ((crosshair_raw_top[y * CROSSHAIR_RAW_ROWBYTES + x / 8] << (x % 8)) & 0x80)
					dest0[x] = dest1[x] = rgb_t(0xff,0x00,0x00,0x00) | color;
		}
	}

	/* reference the new bitmap */
	m_texture[player]->set_bitmap(*m_bitmap[player], m_bitmap[player]->cliprect(), TEXFORMAT_ARGB32);
}

/*-------------------------------------------------
    get_user_settings - return the
    current crosshair settings for a player
    Note: auto_time is common for all players
-------------------------------------------------*/

void crosshair_manager::get_user_settings(UINT8 player, crosshair_user_settings *settings)
{
	settings->auto_time = m_auto_time;
	settings->used = m_used[player];
	settings->mode = m_mode[player];
	strcpy(settings->name, m_name[player]);
}


/*-------------------------------------------------
    set_user_settings - modify the
    current crosshair settings for a player
    Note: auto_time is common for all players
-------------------------------------------------*/

void crosshair_manager::set_user_settings(UINT8 player, crosshair_user_settings *settings)
{
	m_auto_time = settings->auto_time;
	m_used[player] = settings->used;
	m_mode[player] = settings->mode;

	/* set visibility as specified by mode */
	/* auto mode starts with visibility off */
	m_visible[player] = (settings->mode == CROSSHAIR_VISIBILITY_ON) ? TRUE : FALSE;

	/* update bitmap if name has changed */
	int changed = strcmp(settings->name, m_name[player]);
	strcpy(m_name[player], settings->name);
	if (changed != 0)
		create_bitmap(player);
}


/*-------------------------------------------------
    animate - animates the crosshair once a frame
-------------------------------------------------*/

void crosshair_manager::animate(screen_device &device, bool vblank_state)
{
	int player;

	/* only animate once per frame, when vblank_state is 1 */
	if (!vblank_state)
		return;

	/* increment animation counter */
	m_animation_counter += 0x08;

	/* compute a fade factor from the current animation value */
	if (m_animation_counter < 0x80)
		m_fade = 0xa0 + (0x60 * ( m_animation_counter & 0x7f) / 0x80);
	else
		m_fade = 0xa0 + (0x60 * (~m_animation_counter & 0x7f) / 0x80);

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		/* read all the lightgun values */
		if (m_used[player])
			device.machine().ioport().crosshair_position(player, m_x[player], m_y[player]);

		/* auto visibility */
		if (m_mode[player] == CROSSHAIR_VISIBILITY_AUTO)
		{
			if ((m_x[player] != m_last_x[player]) || (m_y[player] != m_last_y[player]))
			{
				/* crosshair has moved, keep crosshair visible */
				m_visible[player] = TRUE;
				m_last_x[player] = m_x[player];
				m_last_y[player] = m_y[player];
				m_time[player] = 0;
			}
			else
			{
				/* see if the player has been motionless for time specified */
				/* slightly confusing formula, but the effect is: */
				/* auto_time = 0 makes the crosshair barely visible while moved */
				/* every increment in auto_time is about .2s at 60Hz */
				if (m_time[player] > m_auto_time * 12 + 2)
					/* time exceeded so turn crosshair invisible */
					m_visible[player] = FALSE;

				/* increment player visibility time */
				m_time[player]++;
			}
		}
	}
}


/*-------------------------------------------------
    render - render the crosshairs
    for the given screen
-------------------------------------------------*/

void crosshair_manager::render(screen_device &screen)
{
	int player;

	for (player = 0; player < MAX_PLAYERS; player++)
		/* draw if visible and the right screen */
		if (m_visible[player] &&
			((m_screen[player] == &screen) || (m_screen[player] == CROSSHAIR_SCREEN_ALL)))
		{
			/* add a quad assuming a 4:3 screen (this is not perfect) */
			screen.container().add_quad(m_x[player] - 0.03f, m_y[player] - 0.04f,
										m_x[player] + 0.03f, m_y[player] + 0.04f,
										rgb_t(0xc0, m_fade, m_fade, m_fade),
										m_texture[player], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
}

/*-------------------------------------------------
    config_load - read and apply data from the
    configuration file
-------------------------------------------------*/

void crosshair_manager::config_load(config_type cfg_type, xml_data_node *parentnode)
{
	/* Note: crosshair_load() is only registered if croshairs are used */

	xml_data_node *crosshairnode;
	int auto_time;

	/* we only care about game files */
	if (cfg_type != config_type::CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == nullptr)
		return;

	/* loop and get player crosshair info */
	for (crosshairnode = xml_get_sibling(parentnode->child, "crosshair"); crosshairnode; crosshairnode = xml_get_sibling(crosshairnode->next, "crosshair"))
	{
		int  player, mode;

		player = xml_get_attribute_int(crosshairnode, "player", -1);

		/* check to make sure we have a valid player */
		/* also check if the player really uses a crosshair */
		if (player >=0 && player < MAX_PLAYERS && m_used[player])
		{
			/* get, check, and store visibility mode */
			mode = xml_get_attribute_int(crosshairnode, "mode", CROSSHAIR_VISIBILITY_DEFAULT);
			if (mode >= CROSSHAIR_VISIBILITY_OFF && mode <= CROSSHAIR_VISIBILITY_AUTO)
			{
				m_mode[player] = (UINT8)mode;
				/* set visibility as specified by mode */
				/* auto mode starts with visibility off */
				m_visible[player] = (mode == CROSSHAIR_VISIBILITY_ON) ? TRUE : FALSE;
			}

			/* get and store crosshair pic name, truncate name to max length */
			strncpy(m_name[player], xml_get_attribute_string(crosshairnode, "pic", ""), CROSSHAIR_PIC_NAME_LENGTH);
			/* update bitmap */
			create_bitmap(player);
		}
	}

	/* get, check, and store auto visibility time */
	crosshairnode = xml_get_sibling(parentnode->child, "autotime");
	if (crosshairnode != nullptr)
	{
		auto_time = xml_get_attribute_int(crosshairnode, "val", CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT);
		if ((auto_time >= CROSSHAIR_VISIBILITY_AUTOTIME_MIN) && (auto_time <= CROSSHAIR_VISIBILITY_AUTOTIME_MAX))
			m_auto_time = (UINT8)auto_time;
	}
}


/*-------------------------------------------------
    config_save -  save data to the
    configuration file
-------------------------------------------------*/

void crosshair_manager::config_save(config_type cfg_type, xml_data_node *parentnode)
{
	/* Note: crosshair_save() is only registered if crosshairs are used */

	xml_data_node *crosshairnode;
	int player;

	/* we only care about game files */
	if (cfg_type != config_type::CONFIG_TYPE_GAME)
		return;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		if (m_used[player])
		{
			/* create a node */
			crosshairnode = xml_add_child(parentnode, "crosshair", nullptr);

			if (crosshairnode != nullptr)
			{
				int changed = FALSE;

				xml_set_attribute_int(crosshairnode, "player", player);

				if (m_visible[player] != CROSSHAIR_VISIBILITY_DEFAULT)
				{
					xml_set_attribute_int(crosshairnode, "mode", m_mode[player]);
					changed = TRUE;
				}

				/* the default graphic name is "", so only save if not */
				if (*(m_name[player]) != 0)
				{
					xml_set_attribute(crosshairnode, "pic", m_name[player]);
					changed = TRUE;
				}

				/* if nothing changed, kill the node */
				if (!changed)
					xml_delete_node(crosshairnode);
			}
		}
	}

	/* always store autotime so that it stays at the user value if it is needed */
	if (m_auto_time != CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT)
	{
		/* create a node */
		crosshairnode = xml_add_child(parentnode, "autotime", nullptr);

		if (crosshairnode != nullptr)
			xml_set_attribute_int(crosshairnode, "val", m_auto_time);
	}

}
