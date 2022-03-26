// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    crsshair.cpp

    Crosshair handling.

***************************************************************************/

#include "emu.h"
#include "crsshair.h"

#include "config.h"
#include "emuopts.h"
#include "fileio.h"
#include "render.h"
#include "rendutil.h"
#include "screen.h"

#include "xmlfile.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_RAW_SIZE      100
#define CROSSHAIR_RAW_ROWBYTES  ((CROSSHAIR_RAW_SIZE + 7) / 8)


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* raw bitmap */
static const u8 crosshair_raw_top[] =
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
//  RENDER CROSSHAIR
//**************************************************************************

//-------------------------------------------------
//  render_crosshair - constructor
//-------------------------------------------------

render_crosshair::render_crosshair(running_machine &machine, int player)
	: m_machine(machine)
	, m_player(player)
	, m_used(false)
	, m_mode(CROSSHAIR_VISIBILITY_OFF)
	, m_visible(false)
	, m_texture(nullptr)
	, m_x(0.0f)
	, m_y(0.0f)
	, m_last_x(0.0f)
	, m_last_y(0.0f)
	, m_time(0)
{
	// for now, use the main screen
	m_screen = screen_device_enumerator(machine.root_device()).first();
}


//-------------------------------------------------
//  render_crosshair - destructor
//-------------------------------------------------

render_crosshair::~render_crosshair()
{
	m_machine.render().texture_free(m_texture);
}


//-------------------------------------------------
//  set_bitmap_name - change the bitmap name
//-------------------------------------------------

void render_crosshair::set_bitmap_name(const char *name)
{
	// update bitmap if name has changed
	bool changed = name != m_name;
	m_name = name;
	if (changed)
		create_bitmap();
}


//-------------------------------------------------
//  set_default_bitmap - reset to default bitmap
//-------------------------------------------------

void render_crosshair::set_default_bitmap()
{
	// update bitmap if name has changed
	bool changed = !m_name.empty();
	m_name.clear();
	if (changed || m_bitmap == nullptr)
		create_bitmap();
}


//-------------------------------------------------
//  create_bitmap - create the rendering
//  structures for the given player
//-------------------------------------------------

void render_crosshair::create_bitmap()
{
	rgb_t color = m_player < std::size(crosshair_colors) ? crosshair_colors[m_player] : rgb_t::white();

	// if we have a bitmap and texture for this player, kill it
	if (!m_bitmap)
	{
		m_bitmap = std::make_unique<bitmap_argb32>();
		m_texture = m_machine.render().texture_alloc(render_texture::hq_scale);
	}
	else
	{
		m_bitmap->reset();
	}

	emu_file crossfile(m_machine.options().crosshair_path(), OPEN_FLAG_READ);
	if (!m_name.empty())
	{
		// look for user specified file
		if (!crossfile.open(m_name + ".png"))
		{
			render_load_png(*m_bitmap, crossfile);
			crossfile.close();
		}
	}
	else
	{
		// look for default cross?.png in crsshair/game dir
		std::string const filename = string_format("cross%d.png", m_player + 1);
		if (!crossfile.open(m_machine.system().name + (PATH_SEPARATOR + filename)))
		{
			render_load_png(*m_bitmap, crossfile);
			crossfile.close();
		}

		// look for default cross?.png in crsshair dir
		if (!m_bitmap->valid() && !crossfile.open(filename))
		{
			render_load_png(*m_bitmap, crossfile);
			crossfile.close();
		}
	}

	/* if that didn't work, use the built-in one */
	if (!m_bitmap->valid())
	{
		/* allocate a blank bitmap to start with */
		m_bitmap->allocate(CROSSHAIR_RAW_SIZE, CROSSHAIR_RAW_SIZE);
		m_bitmap->fill(rgb_t(0x00,0xff,0xff,0xff));

		/* extract the raw source data to it */
		for (int y = 0; y < CROSSHAIR_RAW_SIZE / 2; y++)
		{
			/* assume it is mirrored vertically */
			u32 *const dest0 = &m_bitmap->pix(y);
			u32 *const dest1 = &m_bitmap->pix(CROSSHAIR_RAW_SIZE - 1 - y);

			/* extract to two rows simultaneously */
			for (int x = 0; x < CROSSHAIR_RAW_SIZE; x++)
				if ((crosshair_raw_top[y * CROSSHAIR_RAW_ROWBYTES + x / 8] << (x % 8)) & 0x80)
					dest0[x] = dest1[x] = rgb_t(0xff,0x00,0x00,0x00) | color;
		}
	}

	/* reference the new bitmap */
	m_texture->set_bitmap(*m_bitmap, m_bitmap->cliprect(), TEXFORMAT_ARGB32);
}


//-------------------------------------------------
//  update_position - update the crosshair values
//  for the given player
//-------------------------------------------------

void render_crosshair::update_position()
{
	// read all the lightgun values
	bool gotx = false, goty = false;
	for (auto const &port : m_machine.ioport().ports())
		for (ioport_field const &field : port.second->fields())
			if (field.player() == m_player && field.crosshair_axis() != CROSSHAIR_AXIS_NONE && field.enabled())
			{
				// handle X axis
				if (field.crosshair_axis() == CROSSHAIR_AXIS_X)
				{
					m_x = field.crosshair_read();
					gotx = true;
					if (field.crosshair_altaxis() != 0)
					{
						m_y = field.crosshair_altaxis();
						goty = true;
					}
				}

				// handle Y axis
				else
				{
					m_y = field.crosshair_read();
					goty = true;
					if (field.crosshair_altaxis() != 0)
					{
						m_x = field.crosshair_altaxis();
						gotx = true;
					}
				}

				// if we got both, stop
				if (gotx && goty)
					return;
			}
}


//-------------------------------------------------
//  animate - update the crosshair state
//-------------------------------------------------

void render_crosshair::animate(u16 auto_time)
{
	// read all the port values
	if (m_used)
		update_position();

	// auto visibility
	if (m_mode == CROSSHAIR_VISIBILITY_AUTO)
	{
		if ((m_x != m_last_x) || (m_y != m_last_y))
		{
			// crosshair has moved, keep crosshair visible
			m_visible = true;
			m_last_x = m_x;
			m_last_y = m_y;
			m_time = 0;
		}
		else
		{
			// see if the player has been motionless for time specified
			// slightly confusing formula, but the effect is:
			// auto_time = 0 makes the crosshair barely visible while moved
			// every increment in auto_time is about .2s at 60Hz
			if (m_time > auto_time * 12 + 2)
				// time exceeded so turn crosshair invisible
				m_visible = false;

			// increment player visibility time
			m_time++;
		}
	}
}


//-------------------------------------------------
//  draw - render the crosshair to the container
//-------------------------------------------------

void render_crosshair::draw(render_container &container, u8 fade)
{
	// add a quad assuming a 4:3 screen (this is not perfect)
	container.add_quad(m_x - 0.03f, m_y - 0.04f, m_x + 0.03f, m_y + 0.04f,
						rgb_t(0xc0, fade, fade, fade),
						m_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


//**************************************************************************
//  CROSSHAIR MANAGER
//**************************************************************************

//-------------------------------------------------
//  crosshair_manager - constructor
//-------------------------------------------------

crosshair_manager::crosshair_manager(running_machine &machine)
	: m_machine(machine)
	, m_usage(false)
	, m_fade(0)
	, m_animation_counter(0)
	, m_auto_time(CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT)
{
	/* request a callback upon exiting */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&crosshair_manager::exit, this));

	for (int player = 0; player < MAX_PLAYERS; player++)
		m_crosshair[player] = std::make_unique<render_crosshair>(machine, player);

	/* determine who needs crosshairs */
	for (auto &port : machine.ioport().ports())
		for (ioport_field const &field : port.second->fields())
			if (field.crosshair_axis() != CROSSHAIR_AXIS_NONE)
			{
				int player = field.player();

				assert(player < MAX_PLAYERS);

				/* mark as used and set the default visibility and mode */
				m_usage = true;
				m_crosshair[player]->set_used(true);
				m_crosshair[player]->set_mode(CROSSHAIR_VISIBILITY_DEFAULT);
				m_crosshair[player]->set_visible(CROSSHAIR_VISIBILITY_DEFAULT != CROSSHAIR_VISIBILITY_OFF);
				m_crosshair[player]->set_default_bitmap();
			}

	/* register callbacks for when we load/save configurations */
	if (m_usage)
	{
		machine.configuration().config_register("crosshairs",
				configuration_manager::load_delegate(&crosshair_manager::config_load, this),
				configuration_manager::save_delegate(&crosshair_manager::config_save, this));
	}

	/* register the animation callback */
	screen_device *first_screen = screen_device_enumerator(machine.root_device()).first();
	if (first_screen)
		first_screen->register_vblank_callback(vblank_state_delegate(&crosshair_manager::animate, this));
}

/*-------------------------------------------------
exit - free memory allocated for
the crosshairs
-------------------------------------------------*/

void crosshair_manager::exit()
{
	/* free bitmaps and textures for each player */
	for (int player = 0; player < MAX_PLAYERS; player++)
		m_crosshair[player] = nullptr;
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
		m_crosshair[player]->animate(m_auto_time);
}


/*-------------------------------------------------
    render - render the crosshairs
    for the given screen
-------------------------------------------------*/

void crosshair_manager::render(screen_device &screen)
{
	int player;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		render_crosshair &crosshair = *m_crosshair[player];

		// draw if visible and the right screen
		if (crosshair.is_visible() && ((crosshair.screen() == &screen) || (crosshair.screen() == CROSSHAIR_SCREEN_ALL)))
			crosshair.draw(screen.container(), m_fade);
	}
}

/*-------------------------------------------------
    config_load - read and apply data from the
    configuration file
-------------------------------------------------*/

void crosshair_manager::config_load(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode)
{
	// Note: crosshair_load() is only registered if croshairs are used

	// we only care about system-specific configuration
	if ((cfg_type != config_type::SYSTEM) || !parentnode)
		return;

	// loop and get player crosshair info
	for (util::xml::data_node const *crosshairnode = parentnode->get_child("crosshair"); crosshairnode; crosshairnode = crosshairnode->get_next_sibling("crosshair"))
	{
		int const player = crosshairnode->get_attribute_int("player", -1);

		// check to make sure we have a valid player
		if (player >= 0 && player < MAX_PLAYERS)
		{
			// check if the player really uses a crosshair
			render_crosshair &crosshair = *m_crosshair[player];
			if (crosshair.is_used())
			{
				// get, check, and store visibility mode
				int const mode = crosshairnode->get_attribute_int("mode", CROSSHAIR_VISIBILITY_DEFAULT);
				if (mode >= CROSSHAIR_VISIBILITY_OFF && mode <= CROSSHAIR_VISIBILITY_AUTO)
				{
					crosshair.set_mode(u8(mode));
					// set visibility as specified by mode - auto mode starts with visibility off
					crosshair.set_visible(mode == CROSSHAIR_VISIBILITY_ON);
				}

				// get and store crosshair pic name
				crosshair.set_bitmap_name(crosshairnode->get_attribute_string("pic", ""));
			}
		}
	}

	// get, check, and store auto visibility time
	util::xml::data_node const *crosshairnode = parentnode->get_child("autotime");
	if (crosshairnode)
	{
		int const auto_time = crosshairnode->get_attribute_int("val", CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT);
		if ((auto_time >= CROSSHAIR_VISIBILITY_AUTOTIME_MIN) && (auto_time <= CROSSHAIR_VISIBILITY_AUTOTIME_MAX))
			m_auto_time = u8(auto_time);
	}
}


/*-------------------------------------------------
    config_save -  save data to the
    configuration file
-------------------------------------------------*/

void crosshair_manager::config_save(config_type cfg_type, util::xml::data_node *parentnode)
{
	// Note: crosshair_save() is only registered if crosshairs are used

	// we only create system-specific configuration
	if (cfg_type != config_type::SYSTEM)
		return;

	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		const render_crosshair &crosshair = *m_crosshair[player];

		if (crosshair.is_used())
		{
			// create a node
			util::xml::data_node *const crosshairnode = parentnode->add_child("crosshair", nullptr);

			if (crosshairnode != nullptr)
			{
				bool changed = false;

				crosshairnode->set_attribute_int("player", player);

				if (crosshair.mode() != CROSSHAIR_VISIBILITY_DEFAULT)
				{
					crosshairnode->set_attribute_int("mode", crosshair.mode());
					changed = true;
				}

				// only save graphic name if not the default
				if (*crosshair.bitmap_name() != '\0')
				{
					crosshairnode->set_attribute("pic", crosshair.bitmap_name());
					changed = true;
				}

				// if nothing changed, kill the node
				if (!changed)
					crosshairnode->delete_node();
			}
		}
	}

	// always store autotime so that it stays at the user value if it is needed
	if (m_auto_time != CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT)
	{
		// create a node
		util::xml::data_node *const crosshairnode = parentnode->add_child("autotime", nullptr);
		if (crosshairnode)
			crosshairnode->set_attribute_int("val", m_auto_time);
	}

}
