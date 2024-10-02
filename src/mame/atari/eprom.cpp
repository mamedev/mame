// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Atari Escape hardware

    driver by Aaron Giles

    Games supported:
        * Escape From The Planet Of The Robot Monsters (1989) [2 sets]
        * Klax prototypes [2 sets]
        * Guts n' Glory (prototype)

    Known bugs:
        * none at this time

    TODO:
        * the RGB generator visible in the schematics is not properly modeled.


****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"

#include "atarijsa.h"
#include "atarimo.h"

#include "cpu/m68000/m68000.h"
#include "machine/eeprompar.h"
#include "machine/adc0808.h"
#include "machine/timer.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class eprom_state : public driver_device
{
public:
	eprom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_playfield_tilemap(*this, "playfield"),
		m_alpha_tilemap(*this, "alpha"),
		m_mob(*this, "mob"),
		m_jsa(*this, "jsa"),
		m_shared_ram(*this, "shared_ram"),
		m_adc(*this, "adc"),
		m_extra(*this, "extra"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram")
	{ }

	void guts(machine_config &config);
	void eprom(machine_config &config);
	void klaxp(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	required_device<atari_jsa_base_device> m_jsa;
	required_shared_ptr<uint16_t> m_shared_ram;
	optional_device<adc0808_device> m_adc;
	optional_device<cpu_device> m_extra;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_paletteram;
	uint8_t m_screen_intensity = 0U;
	uint8_t m_video_disable = 0U;
	static const atari_motion_objects_config s_mob_config;
	static const atari_motion_objects_config s_guts_mob_config;

	void video_int_ack_w(uint16_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	uint8_t adc_r(offs_t offset);
	void eprom_latch_w(uint8_t data);
	template<bool maincpu> void sync_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(guts_get_playfield_tile_info);
	uint32_t screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_palette();
	void extra_map(address_map &map) ATTR_COLD;
	void guts_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Palette
 *
 *************************************/

void eprom_state::update_palette()
{
	for (int color = 0; color < 0x800; ++color)
	{
		uint16_t const data = m_paletteram[color];

		/* FIXME this is only a very crude approximation of the palette output.
		 * The circuit involves a dozen transistors and probably has an output
		 * which is quite different from this.
		 * This is, however, good enough to match the video and description
		 * of MAMETesters bug #02677.
		 */
		int i = (((data >> 12) & 15) + 1) * (4 - m_screen_intensity);
		if (i < 0)
			i = 0;

		int const r = ((data >> 8) & 15) * i / 4;
		int const g = ((data >> 4) & 15) * i / 4;
		int const b = ((data >> 0) & 15) * i / 4;

		m_palette->set_pen_color(color, r, g, b);
	}
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(eprom_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0x3ff;
	int const color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int const opaque = data & 0x8000;
	tileinfo.set(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(eprom_state::get_playfield_tile_info)
{
	uint16_t const data1 = m_playfield_tilemap->basemem_read(tile_index);
	uint16_t const data2 = m_playfield_tilemap->extmem_read(tile_index) >> 8;
	int const code = data1 & 0x7fff;
	int const color = 0x10 + (data2 & 0x0f);
	tileinfo.set(0, code, color, (data1 >> 15) & 1);
}


TILE_GET_INFO_MEMBER(eprom_state::guts_get_playfield_tile_info)
{
	uint16_t const data1 = m_playfield_tilemap->basemem_read(tile_index);
	uint16_t const data2 = m_playfield_tilemap->extmem_read(tile_index) >> 8;
	int const code = data1 & 0x7fff;
	int const color = 0x10 + (data2 & 0x0f);
	tileinfo.set(2, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config eprom_state::s_mob_config =
{
	0,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	1,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0x100,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0x03ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }}, // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles
	{{ 0,0,0,0x0007 }}, // mask for the height, in tiles
	{{ 0,0,0,0x0008 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0,0x0070,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};

void eprom_state::video_start()
{
	// save states
	save_item(NAME(m_screen_intensity));
	save_item(NAME(m_video_disable));

	m_screen_intensity = 0;
	m_video_disable = 0;
}


const atari_motion_objects_config eprom_state::s_guts_mob_config =
{
	0,                  // index to which gfx system
	1,                  // number of motion object banks
	1,                  // are the entries linked?
	0,                  // are the entries split?
	0,                  // render in reverse order?
	0,                  // render in swapped X/Y order?
	0,                  // does the neighbor bit affect the next object?
	8,                  // pixels per SLIP entry (0 for no-slip)
	0,                  // pixel offset for SLIPs
	0,                  // maximum number of links to visit/scanline (0=all)

	0x100,              // base palette entry
	0x100,              // maximum number of colors
	0,                  // transparent pen index

	{{ 0x03ff,0,0,0 }}, // mask for the link
	{{ 0,0x7fff,0,0 }}, // mask for the code index
	{{ 0,0,0x000f,0 }}, // mask for the color
	{{ 0,0,0xff80,0 }}, // mask for the X position
	{{ 0,0,0,0xff80 }}, // mask for the Y position
	{{ 0,0,0,0x0070 }}, // mask for the width, in tiles
	{{ 0,0,0,0x000f }}, // mask for the height, in tiles
	{{ 0,0x8000,0,0 }}, // mask for the horizontal flip
	{{ 0 }},            // mask for the vertical flip
	{{ 0,0,0x0070,0 }}, // mask for the priority
	{{ 0 }},            // mask for the neighbor
	{{ 0 }},            // mask for absolute coordinates

	{{ 0 }},            // mask for the special value
	0                   // resulting value to indicate "special"
};


/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(eprom_state::scanline_update)
{
	// update the playfield
	if (param == 0)
	{
		int const xscroll = (m_alpha_tilemap->basemem_read(0x780) >> 7) & 0x1ff;
		int const yscroll = (m_alpha_tilemap->basemem_read(0x781) >> 7) & 0x1ff;
		m_playfield_tilemap->set_scrollx(0, xscroll);
		m_playfield_tilemap->set_scrolly(0, yscroll);
		m_mob->set_scroll(xscroll, yscroll);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t eprom_state::screen_update_eprom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_disable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	update_palette();

	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					/* verified from the GALs on the real PCB; equations follow
					 *
					 *      --- FORCEMC0 forces 3 bits of the MO color to 0 under some conditions
					 *      FORCEMC0=!PFX3*PFX4*PFX5*!MPR0
					 *          +!PFX3*PFX5*!MPR1
					 *          +!PFX3*PFX4*!MPR0*!MPR1
					 *
					 *      --- SHADE selects an alternate color bank for the playfield
					 *      !SHADE=!MPX0
					 *          +MPX1
					 *          +MPX2
					 *          +MPX3
					 *          +!MPX4*!MPX5*!MPX6*!MPX7
					 *          +FORCEMC0
					 *
					 *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
					 *      !PF/M=MPR0*MPR1
					 *          +PFX3
					 *          +!PFX4*MPR1
					 *          +!PFX5*MPR1
					 *          +!PFX5*MPR0
					 *          +!PFX4*!PFX5*!MPR0*!MPR1
					 *
					 *      --- M7 is passed as the upper MO bit to the GPC ASIC
					 *      M7=MPX0*!MPX1*!MPX2*!MPX3
					 *
					 *      --- CL10-9 are outputs from the GPC, specifying which layer to render
					 *      CL10 = 1 if pf
					 *      CL9 = 1 if mo
					 *
					 *      --- CRA10 is the 0x200 bit of the color RAM index; it comes directly from the GPC
					 *      CRA10 = CL10
					 *
					 *      --- CRA9 is the 0x100 bit of the color RAM index; is comes directly from the GPC
					 *          or if the SHADE flag is set, it affects the playfield color bank
					 *      CRA9 = SHADE*CL10
					 *          +CL9
					 *
					 *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
					 */
					int const mopriority = (mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT) & 7;
					int const pfpriority = (pf[x] >> 4) & 3;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
						continue;

					// compute the FORCEMC signal
					int forcemc0 = 0;
					if (!(pf[x] & 8))
					{
						if (((pfpriority == 3) && !(mopriority & 1)) ||
							((pfpriority & 2) && !(mopriority & 2)) ||
							((pfpriority & 1) && (mopriority == 0)))
							forcemc0 = 1;
					}

					// compute the SHADE signal
					int shade = 1;
					if (((mo[x] & 0x0f) != 1) ||
						((mo[x] & 0xf0) == 0) ||
						forcemc0)
						shade = 0;

					// compute the PF/M signal
					int pfm = 1;
					if ((mopriority == 3) ||
						(pf[x] & 8) ||
						(!(pfpriority & 1) && (mopriority & 2)) ||
						(!(pfpriority & 2) && (mopriority & 2)) ||
						(!(pfpriority & 2) && (mopriority & 1)) ||
						((pfpriority == 0) && (mopriority == 0)))
						pfm = 0;

					// compute the M7 signal
					int m7 = 0;
					if ((mo[x] & 0x0f) == 1)
						m7 = 1;

					// PF/M and M7 go in the GPC ASIC and select playfield or MO layers
					if (!pfm && !m7)
					{
						if (!forcemc0)
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
						else
							pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK & ~0x70;
					}
					else
					{
						if (shade)
							pf[x] |= 0x100;
						if (m7)
							pf[x] |= 0x080;
					}
				}
		}

	// add the alpha on top
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// now go back and process the upper bit of MO priority
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority might mean palette kludges
					if (mopriority & 4)
					{
						// if bit 2 is set, start setting high palette bits
						if (mo[x] & 2)
							m_mob->apply_stain(bitmap, pf, mo, x, y);
					}
				}
		}
	return 0;
}


uint32_t eprom_state::screen_update_guts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_disable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	update_palette();

	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					int const mopriority = (mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT) & 7;
					int const pfpriority = (pf[x] >> 5) & 3;

					// upper bit of MO priority signals special rendering and doesn't draw anything
					if (mopriority & 4)
						continue;

					// check the priority
					if (!(pf[x] & 8) || mopriority >= pfpriority)
						pf[x] = mo[x] & atari_motion_objects_device::DATA_MASK;
				}
		}

	// add the alpha on top
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// now go back and process the upper bit of MO priority
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					int const mopriority = mo[x] >> atari_motion_objects_device::PRIORITY_SHIFT;

					// upper bit of MO priority might mean palette kludges
					if (mopriority & 4)
					{
						// if bit 2 is set, start setting high palette bits
						if (mo[x] & 2)
							m_mob->apply_stain(bitmap, pf, mo, x, y);
					}
				}
		}

	return 0;
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void eprom_state::video_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	if (m_extra.found())
		m_extra->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}

void eprom_state::machine_reset()
{
	m_shared_ram[0xcc00 / 2] = 0;
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

uint8_t eprom_state::adc_r(offs_t offset)
{
	if (!m_adc.found())
		return 0xff;

	uint8_t result = m_adc->data_r();
	if (!machine().side_effects_disabled())
		m_adc->address_offset_start_w(offset, 0);
	return result;
}



/*************************************
 *
 *  Latch write handler
 *
 *************************************/

void eprom_state::eprom_latch_w(uint8_t data)
{
	if (m_extra.found())
	{
		// bit 0: reset extra CPU
		if (data & 1)
			m_extra->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		else
			m_extra->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

		// bits 1-4: screen intensity
		m_screen_intensity = (data & 0x1e) >> 1;

		// bit 5: video disable
		m_video_disable = (data & 0x20);
	}
}



/*************************************
 *
 *  Synchronization
 *
 *************************************/

template<bool maincpu> void eprom_state::sync_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	u16 oldword = m_shared_ram[0xcc00 / 2];
	COMBINE_DATA(&m_shared_ram[0xcc00 / 2]);
	u16 newword = m_shared_ram[0xcc00 / 2];

	if ((oldword & 0xff00) != (newword & 0xff00))
		(maincpu ? m_maincpu->yield() : m_extra->yield());
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void eprom_state::main_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom();
	map(0x0e0000, 0x0e03ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x160000, 0x16ffff).ram().share(m_shared_ram);
	map(0x16cc00, 0x16cc01).w(FUNC(eprom_state::sync_w<true>));
	map(0x1f0000, 0x1fffff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x26001f).portr("260010");
	map(0x260020, 0x260027).mirror(0x8).r(FUNC(eprom_state::adc_r)).umask16(0x00ff);
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_base_device::main_response_r));
	map(0x2e0000, 0x2e0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x360000, 0x360001).w(FUNC(eprom_state::video_int_ack_w));
	map(0x360011, 0x360011).w(FUNC(eprom_state::eprom_latch_w));
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_base_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_base_device::main_command_w));
	map(0x3e0000, 0x3e0fff).ram().share(m_paletteram);
	map(0x3f0000, 0x3f1fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0x3f2000, 0x3f3fff).ram().share("mob");
	map(0x3f4000, 0x3f4f7f).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0x3f4f80, 0x3f4fff).ram().share("mob:slip");
	map(0x3f5000, 0x3f7fff).ram();
	map(0x3f8000, 0x3f9fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16_ext)).share("playfield_ext");
}


void eprom_state::guts_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom();
	map(0x0e0000, 0x0e03ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x160000, 0x16ffff).ram().share(m_shared_ram);
	map(0x16cc00, 0x16cc01).w(FUNC(eprom_state::sync_w<true>));
	map(0x1f0000, 0x1fffff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x26001f).portr("260010");
	map(0x260020, 0x260027).mirror(0x8).r(FUNC(eprom_state::adc_r)).umask16(0x00ff);
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_ii_device::main_response_r));
	map(0x2e0000, 0x2e0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x360000, 0x360001).w(FUNC(eprom_state::video_int_ack_w));
//  map(0x360011, 0x360011).w(FUNC(eprom_state::eprom_latch_w));
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_ii_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_ii_device::main_command_w));
	map(0x3e0000, 0x3e0fff).ram().share(m_paletteram);
	map(0xff0000, 0xff1fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16_ext)).share("playfield_ext");
	map(0xff8000, 0xff9fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0xffa000, 0xffbfff).ram().share("mob");
	map(0xffc000, 0xffcf7f).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0xffcf80, 0xffcfff).ram().share("mob:slip");
	map(0xffd000, 0xffffff).ram();
}



/*************************************
 *
 *  Extra CPU memory handlers
 *
 *************************************/

void eprom_state::extra_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x160000, 0x16ffff).ram().share(m_shared_ram);
	map(0x16cc00, 0x16cc01).w(FUNC(eprom_state::sync_w<false>));
	map(0x260000, 0x26000f).portr("260000");
	map(0x260010, 0x26001f).portr("260010");
	map(0x260020, 0x260027).mirror(0x8).r(FUNC(eprom_state::adc_r)).umask16(0x00ff);
	map(0x260031, 0x260031).r(m_jsa, FUNC(atari_jsa_base_device::main_response_r));
	map(0x360000, 0x360001).w(FUNC(eprom_state::video_int_ack_w));
	map(0x360011, 0x360011).w(FUNC(eprom_state::eprom_latch_w));
	map(0x360020, 0x360021).w(m_jsa, FUNC(atari_jsa_base_device::sound_reset_w));
	map(0x360031, 0x360031).w(m_jsa, FUNC(atari_jsa_base_device::main_command_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( eprom )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // Input buffer full (@260030)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // Output buffer full (@360030)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", adc0808_device, eoc_r)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC0")          // ADC0 @ 0x260020
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ADC1")          // ADC1 @ 0x260022
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("ADC2")          // ADC0 @ 0x260024
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("ADC3")          // ADC1 @ 0x260026
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( klaxp )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // Input buffer full (@260030)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // Output buffer full (@360030)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( guts )
	PORT_START("260000")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // Input buffer full (@260030)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // Output buffer full (@360030)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", adc0808_device, eoc_r)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC0")          // ADC0 @ 0x260020
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ADC1")          // ADC1 @ 0x260022
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("ADC2")          //* ADC0 @ 0x260024
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("ADC3")          // ADC1 @ 0x260026
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,16) },
	8*16
};


static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static GFXDECODE_START( gfx_eprom )
	GFXDECODE_ENTRY( "spr_tiles", 0, pfmolayout,  256, 32 )
	GFXDECODE_ENTRY( "chars",     0, anlayout,      0, 64 )
GFXDECODE_END


static GFXDECODE_START( gfx_guts )
	GFXDECODE_ENTRY( "sprites", 0, pfmolayout,  256, 32 )
	GFXDECODE_ENTRY( "chars",   0, anlayout,      0, 64 )
	GFXDECODE_ENTRY( "tiles",   0, pfmolayout,  256, 32 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void eprom_state::eprom(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &eprom_state::main_map);

	M68000(config, m_extra, 14.318181_MHz_XTAL / 2);
	m_extra->set_addrmap(AS_PROGRAM, &eprom_state::extra_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	TIMER(config, "scantimer").configure_scanline(FUNC(eprom_state::scanline_update), m_screen, 0, 8);

	ADC0809(config, m_adc, 14.318181_MHz_XTAL / 16);
	m_adc->in_callback<0>().set_ioport("ADC0");
	m_adc->in_callback<1>().set_ioport("ADC1");
	m_adc->in_callback<2>().set_ioport("ADC2");
	m_adc->in_callback<3>().set_ioport("ADC3");

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_eprom);
	PALETTE(config, m_palette).set_entries(2048);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(eprom_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(eprom_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, eprom_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a SYNGEN chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(eprom_state::screen_update_eprom));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);
	m_screen->screen_vblank().append_inputline(m_extra, M68K_IRQ_4, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_I(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(1);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 0.4);
	config.device_remove("jsa:pokey");
}


void eprom_state::klaxp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &eprom_state::main_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TIMER(config, "scantimer").configure_scanline(FUNC(eprom_state::scanline_update), m_screen, 0, 8);

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_eprom);
	PALETTE(config, m_palette).set_entries(2048);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(eprom_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(eprom_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, eprom_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a SYNGEN chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(eprom_state::screen_update_eprom));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_II(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(1);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void eprom_state::guts(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &eprom_state::guts_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	TIMER(config, "scantimer").configure_scanline(FUNC(eprom_state::scanline_update), m_screen, 0, 8);

	ADC0809(config, m_adc, 14.318181_MHz_XTAL / 16);
	m_adc->in_callback<0>().set_ioport("ADC0");
	m_adc->in_callback<1>().set_ioport("ADC1");
	m_adc->in_callback<2>().set_ioport("ADC2");
	m_adc->in_callback<3>().set_ioport("ADC3");

	EEPROM_2804(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_guts);
	PALETTE(config, m_palette).set_entries(2048);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_COLS, 64, 64).set_info_callback(FUNC(eprom_state::guts_get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8, 8, TILEMAP_SCAN_ROWS, 64, 32, 0).set_info_callback(FUNC(eprom_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, eprom_state::s_guts_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// note: these parameters are from published specs, not derived
	// the board uses a SYNGEN chip to generate video signals
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(eprom_state::screen_update_guts));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_II(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_jsa->test_read_cb().set_ioport("260010").bit(1);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( eprom )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136069-3025.50a",   0x00000, 0x10000, CRC(08888dec) SHA1(a0a137828b9e1efbdbc0e5ddaf4d73d24b36948a) )
	ROM_LOAD16_BYTE( "136069-3024.40a",   0x00001, 0x10000, CRC(29cb1e97) SHA1(ccf7024dccbd61983d61450f15c805422e4eee09) )
	ROM_LOAD16_BYTE( "136069-4027.50b",   0x20000, 0x10000, CRC(702241c9) SHA1(cba27e92f64fd201c16aed6a8f2dc64c4f887e4f) )
	ROM_LOAD16_BYTE( "136069-4026.40b",   0x20001, 0x10000, CRC(fecbf9e2) SHA1(cd06bfab296e9496564fc2716b26874b55dc2188) )
	ROM_LOAD16_BYTE( "136069-4029.50d",   0x40000, 0x10000, CRC(0f2f1502) SHA1(2aa65c03d4cd94839a2c2ba338177202bc1185ee) )
	ROM_LOAD16_BYTE( "136069-4028.40d",   0x40001, 0x10000, CRC(bc6f6ae8) SHA1(43a947cf9db7cda825924e167529305f63bb2a5c) )
	ROM_LOAD16_BYTE( "136069-2033.40k",   0x60000, 0x10000, CRC(130650f6) SHA1(bea7780d54a4e1f3e93f14494c82446a4bb48e19) )
	ROM_LOAD16_BYTE( "136069-2032.50k",   0x60001, 0x10000, CRC(1da21ed8) SHA1(3b00e3cf5a25918c1f3158d8b2192158f77cb521) )

	ROM_REGION( 0x80000, "extra", 0 )
	ROM_LOAD16_BYTE( "136069-2035.10s",   0x00000, 0x10000, CRC(deff6469) SHA1(2fe4d42d60965581579e8edad49b86fbd321d1db) )
	ROM_LOAD16_BYTE( "136069-2034.10u",   0x00001, 0x10000, CRC(5d7afca2) SHA1(a37ecd2909049dd0b3ddbe602f0173c44b065f6f) )
	ROM_COPY( "maincpu", 0x60000,  0x60000, 0x20000 )

	ROM_REGION( 0x10000, "jsa:cpu", 0 )
	ROM_LOAD( "136069-1040.7b",    0x00000, 0x10000, CRC(86e93695) SHA1(63ddab02df139dd41a8260c303798b2a550b9fe6) )

	ROM_REGION( 0x100000, "spr_tiles", ROMREGION_INVERT )
	ROM_LOAD( "136069-1020.47s",   0x00000, 0x10000, CRC(0de9d98d) SHA1(c2f963a8a4573e135a2825929cbc5535ce3b0215) )
	ROM_LOAD( "136069-1013.43s",   0x10000, 0x10000, CRC(8eb106ad) SHA1(ece0ddba8fafe6e720f843c4d3f69ae654ae9d92) )
	ROM_LOAD( "136069-1018.38s",   0x20000, 0x10000, CRC(bf3d0e18) SHA1(c81dacd06ce2580e37ff480d1182ab6c7e74d600) )
	ROM_LOAD( "136069-1023.32s",   0x30000, 0x10000, CRC(48fb2e42) SHA1(480edc87f7ca547c3d8e09bf1a98e04ac464f4c6) )
	ROM_LOAD( "136069-1016.76s",   0x40000, 0x10000, CRC(602d939d) SHA1(2ce9899f4cf0786df8c5f0e8cc63ce5206ea514f) )
	ROM_LOAD( "136069-1011.70s",   0x50000, 0x10000, CRC(f6c973af) SHA1(048d5a9b89cb83186ca594e71521675248970735) )
	ROM_LOAD( "136069-1017.64s",   0x60000, 0x10000, CRC(9cd52e30) SHA1(59233a87f2b50e9390f297abe7489864222f98e2) )
	ROM_LOAD( "136069-1022.57s",   0x70000, 0x10000, CRC(4e2c2e7e) SHA1(6bf203e8c029d955634dcbaef9a6932d42035b25) )
	ROM_LOAD( "136069-1012.47u",   0x80000, 0x10000, CRC(e7edcced) SHA1(4c19ea8b15332681bfc73a3d2b063985c1bbac1d) )
	ROM_LOAD( "136069-1010.43u",   0x90000, 0x10000, CRC(9d3e144d) SHA1(7f4c7ee14d10a733f8b4169b41023bda1b5702c8) )
	ROM_LOAD( "136069-1015.38u",   0xa0000, 0x10000, CRC(23f40437) SHA1(567aa09a986dd8765c54f413f906e1cb323568c6) )
	ROM_LOAD( "136069-1021.32u",   0xb0000, 0x10000, CRC(2a47ff7b) SHA1(89935eac8fbeed87668fe1dcb4645c96a9df2c03) )
	ROM_LOAD( "136069-1008.76u",   0xc0000, 0x10000, CRC(b0cead58) SHA1(b50b0125bedc1740d02c50e0547a2d2e25b2c42e) )
	ROM_LOAD( "136069-1009.70u",   0xd0000, 0x10000, CRC(fbc3934b) SHA1(08c581359a005df4d63fa07733bb343c5ab653a9) )
	ROM_LOAD( "136069-1014.64u",   0xe0000, 0x10000, CRC(0e07493b) SHA1(c5839ac4824b6fedb5397779cd30f6b1eff962d5) )
	ROM_LOAD( "136069-1019.57u",   0xf0000, 0x10000, CRC(34f8f0ed) SHA1(9096aa2a188a15c2e78acf48d33def0c9f2a419f) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136069-1007.125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136069.100t",  0x0000, 0x0117, CRC(fd9d472e) SHA1(ba2357e355d062f3ce8bdbed59856e28130999d0) )
	ROM_LOAD( "gal16v8-136069.100v",  0x0200, 0x0117, CRC(cd472121) SHA1(634354363866af39e0d69cbe7c5e80abbd428cec) )
	ROM_LOAD( "gal16v8-136069.50f",   0x0400, 0x0117, CRC(db013b25) SHA1(8090ad125cc48b9bd3070bc38003392ef561da58) )
	ROM_LOAD( "gal16v8-136069.50p",   0x0600, 0x0117, CRC(4a765b00) SHA1(6af38b44890b630b6bb8f51d56c202b5b2558969) )
	ROM_LOAD( "gal16v8-136069.55p",   0x0800, 0x0117, CRC(48abc939) SHA1(54612ffe6fc27d0e1cb401931e6b9636cef1130e) )
	ROM_LOAD( "gal16v8-136069.70j",   0x0a00, 0x0117, CRC(3b4ebe41) SHA1(5d8e550500bddc8cc06e0240bcc81737a75bf5af) )
ROM_END


ROM_START( eprom2 )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "136069-1025.50a",   0x00000, 0x10000, CRC(b0c9a476) SHA1(6d0edeeb9458e92191f6623307eddc9b2f830d4d) )
	ROM_LOAD16_BYTE( "136069-1024.40a",   0x00001, 0x10000, CRC(4cc2c50c) SHA1(088908cc57b07d71a5d664674e38fa02c55bb4fc) )
	ROM_LOAD16_BYTE( "136069-1027.50b",   0x20000, 0x10000, CRC(84f533ea) SHA1(c1da671be5149bff26acd19b14cd18db0df695b7) )
	ROM_LOAD16_BYTE( "136069-1026.40b",   0x20001, 0x10000, CRC(506396ce) SHA1(9457d346ab3aabec17f2c9ea32b9058aabdce831) )
	ROM_LOAD16_BYTE( "136069-1029.50d",   0x40000, 0x10000, CRC(99810b9b) SHA1(f744afa559798e58b0d7ad5c7f02746e5ef94524) )
	ROM_LOAD16_BYTE( "136069-1028.40d",   0x40001, 0x10000, CRC(08ab41f2) SHA1(1801c01efbeca64c1beecc9ca31ec12e02000a6c) )
	ROM_LOAD16_BYTE( "136069-1033.40k",   0x60000, 0x10000, CRC(395fc203) SHA1(5f5ceb286f5e4efd88c9a9368b0486da9f318365) )
	ROM_LOAD16_BYTE( "136069-1032.50k",   0x60001, 0x10000, CRC(a19c8acb) SHA1(77405d1e9ca82f7967ea7e54ffa81b74d81f5b56) )
	ROM_LOAD16_BYTE( "136069-1037.50e",   0x80000, 0x10000, CRC(ad39a3dd) SHA1(00dcdcb30b7f8441df4216f9be4de15791ac5fc8) )
	ROM_LOAD16_BYTE( "136069-1036.40e",   0x80001, 0x10000, CRC(34fc8895) SHA1(0c167c3a778e064a37517b52fd7a52f16d844f77) )

	ROM_REGION( 0x80000, "extra", 0 )
	ROM_LOAD16_BYTE( "136069-1035.10s",    0x00000, 0x10000, CRC(ffeb5647) SHA1(fbd9217a96e51dd0c0cbc0ba9dfdaaa36fbc1ae9) )
	ROM_LOAD16_BYTE( "136069-1034.10u",    0x00001, 0x10000, CRC(c68f58dd) SHA1(0ec300f32e67b710ac33efb60b8eccceb43faca6) )
	ROM_COPY( "maincpu", 0x60000, 0x60000, 0x20000 )

	ROM_REGION( 0x10000, "jsa:cpu", 0 )
	ROM_LOAD( "136069-1040.7b",    0x00000, 0x10000, CRC(86e93695) SHA1(63ddab02df139dd41a8260c303798b2a550b9fe6) )

	ROM_REGION( 0x100000, "spr_tiles", ROMREGION_INVERT )
	ROM_LOAD( "136069-1020.47s",   0x00000, 0x10000, CRC(0de9d98d) SHA1(c2f963a8a4573e135a2825929cbc5535ce3b0215) )
	ROM_LOAD( "136069-1013.43s",   0x10000, 0x10000, CRC(8eb106ad) SHA1(ece0ddba8fafe6e720f843c4d3f69ae654ae9d92) )
	ROM_LOAD( "136069-1018.38s",   0x20000, 0x10000, CRC(bf3d0e18) SHA1(c81dacd06ce2580e37ff480d1182ab6c7e74d600) )
	ROM_LOAD( "136069-1023.32s",   0x30000, 0x10000, CRC(48fb2e42) SHA1(480edc87f7ca547c3d8e09bf1a98e04ac464f4c6) )
	ROM_LOAD( "136069-1016.76s",   0x40000, 0x10000, CRC(602d939d) SHA1(2ce9899f4cf0786df8c5f0e8cc63ce5206ea514f) )
	ROM_LOAD( "136069-1011.70s",   0x50000, 0x10000, CRC(f6c973af) SHA1(048d5a9b89cb83186ca594e71521675248970735) )
	ROM_LOAD( "136069-1017.64s",   0x60000, 0x10000, CRC(9cd52e30) SHA1(59233a87f2b50e9390f297abe7489864222f98e2) )
	ROM_LOAD( "136069-1022.57s",   0x70000, 0x10000, CRC(4e2c2e7e) SHA1(6bf203e8c029d955634dcbaef9a6932d42035b25) )
	ROM_LOAD( "136069-1012.47u",   0x80000, 0x10000, CRC(e7edcced) SHA1(4c19ea8b15332681bfc73a3d2b063985c1bbac1d) )
	ROM_LOAD( "136069-1010.43u",   0x90000, 0x10000, CRC(9d3e144d) SHA1(7f4c7ee14d10a733f8b4169b41023bda1b5702c8) )
	ROM_LOAD( "136069-1015.38u",   0xa0000, 0x10000, CRC(23f40437) SHA1(567aa09a986dd8765c54f413f906e1cb323568c6) )
	ROM_LOAD( "136069-1021.32u",   0xb0000, 0x10000, CRC(2a47ff7b) SHA1(89935eac8fbeed87668fe1dcb4645c96a9df2c03) )
	ROM_LOAD( "136069-1008.76u",   0xc0000, 0x10000, CRC(b0cead58) SHA1(b50b0125bedc1740d02c50e0547a2d2e25b2c42e) )
	ROM_LOAD( "136069-1009.70u",   0xd0000, 0x10000, CRC(fbc3934b) SHA1(08c581359a005df4d63fa07733bb343c5ab653a9) )
	ROM_LOAD( "136069-1014.64u",   0xe0000, 0x10000, CRC(0e07493b) SHA1(c5839ac4824b6fedb5397779cd30f6b1eff962d5) )
	ROM_LOAD( "136069-1019.57u",   0xf0000, 0x10000, CRC(34f8f0ed) SHA1(9096aa2a188a15c2e78acf48d33def0c9f2a419f) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "136069.125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "gal16v8-136069.100t",  0x0000, 0x0117, CRC(fd9d472e) SHA1(ba2357e355d062f3ce8bdbed59856e28130999d0) )
	ROM_LOAD( "gal16v8-136069.100v",  0x0200, 0x0117, CRC(cd472121) SHA1(634354363866af39e0d69cbe7c5e80abbd428cec) )
	ROM_LOAD( "gal16v8-136069.50f",   0x0400, 0x0117, CRC(db013b25) SHA1(8090ad125cc48b9bd3070bc38003392ef561da58) )
	ROM_LOAD( "gal16v8-136069.50p",   0x0600, 0x0117, CRC(4a765b00) SHA1(6af38b44890b630b6bb8f51d56c202b5b2558969) )
	ROM_LOAD( "gal16v8-136069.55p",   0x0800, 0x0117, CRC(48abc939) SHA1(54612ffe6fc27d0e1cb401931e6b9636cef1130e) )
	ROM_LOAD( "gal16v8-136069.70j",   0x0a00, 0x0117, CRC(3b4ebe41) SHA1(5d8e550500bddc8cc06e0240bcc81737a75bf5af) )
ROM_END


ROM_START( klaxp1 )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "klax_ft1.50a",   0x00000, 0x10000, CRC(87ee72d1) SHA1(39ae6f8406f0768480bcc80d395a14d9c2c65dca) )
	ROM_LOAD16_BYTE( "klax_ft1.40a",   0x00001, 0x10000, CRC(ba139fdb) SHA1(98a8ac5e0349b934f55d0d9de85abacd3fd0d77d) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 )
	ROM_LOAD( "klaxsnd.10c",  0x00000, 0x10000, CRC(744734cb) SHA1(3630428d69ddd2a4d5dd76bb4ee9485c943129e9) )

	ROM_REGION( 0x40000, "spr_tiles", ROMREGION_INVERT )
	ROM_LOAD( "klaxprot.43s",   0x00000, 0x10000, CRC(a523c966) SHA1(8e284901cd1c68b25aa9dec1c87374b93cceeb40) )
	ROM_LOAD( "klaxprot.76s",   0x10000, 0x10000, CRC(dbc678cd) SHA1(4e6db153d29300e8d5960937d3bfebbd1ae2e78a) )
	ROM_LOAD( "klaxprot.47u",   0x20000, 0x10000, CRC(af184754) SHA1(4567337e1af1f748b1663e0b4c3e8ea746aac56c) )
	ROM_LOAD( "klaxprot.76u",   0x30000, 0x10000, CRC(7a56ffab) SHA1(96c491e51931c6641e63e55da173ecd41df7c7b3) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "klax125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   // ADPCM data
	ROM_LOAD( "klaxadp0.1f", 0x00000, 0x10000, CRC(ba1e864f) SHA1(7c45e9040701b54c8be398c6e5cdf9201dc37c17) )
	ROM_LOAD( "klaxadp1.1e", 0x10000, 0x10000, CRC(dec9a5ac) SHA1(8039d946ac3613fa6193b557cc8775c81871831d) )
ROM_END


ROM_START( klaxp2 )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "klax_ft2.50a",   0x00000, 0x10000, CRC(7d401937) SHA1(8db0560528a86b9cb01c4598a49694bd44b00dba) )
	ROM_LOAD16_BYTE( "klax_ft2.40a",   0x00001, 0x10000, CRC(c5ca33a9) SHA1(c2e2948f987ba43f61c043baed06ffea8787be43) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 )
	ROM_LOAD( "klaxsnd.10c",  0x00000, 0x10000, CRC(744734cb) SHA1(3630428d69ddd2a4d5dd76bb4ee9485c943129e9) )

	ROM_REGION( 0x40000, "spr_tiles", ROMREGION_INVERT )
	ROM_LOAD( "klaxprot.43s",   0x00000, 0x10000, CRC(a523c966) SHA1(8e284901cd1c68b25aa9dec1c87374b93cceeb40) )
	ROM_LOAD( "klaxprot.76s",   0x10000, 0x10000, CRC(dbc678cd) SHA1(4e6db153d29300e8d5960937d3bfebbd1ae2e78a) )
	ROM_LOAD( "klaxprot.47u",   0x20000, 0x10000, CRC(af184754) SHA1(4567337e1af1f748b1663e0b4c3e8ea746aac56c) )
	ROM_LOAD( "klaxprot.76u",   0x30000, 0x10000, CRC(7a56ffab) SHA1(96c491e51931c6641e63e55da173ecd41df7c7b3) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "klax125d",  0x00000, 0x04000, CRC(409d818e) SHA1(63dcde3ce87c1a9d5afef8089432c499cc70f8f0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   // ADPCM data
	ROM_LOAD( "klaxadp0.1f", 0x00000, 0x10000, CRC(ba1e864f) SHA1(7c45e9040701b54c8be398c6e5cdf9201dc37c17) )
	ROM_LOAD( "klaxadp1.1e", 0x10000, 0x10000, CRC(dec9a5ac) SHA1(8039d946ac3613fa6193b557cc8775c81871831d) )
ROM_END


ROM_START( guts )
	ROM_REGION( 0xa0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guts-hi0.50a", 0x00000, 0x10000, CRC(3afca24a) SHA1(4910c958ac2124de13d4069420fb2cfd18b12cec) )
	ROM_LOAD16_BYTE( "guts-lo0.40a", 0x00001, 0x10000, CRC(ce86cf23) SHA1(28504e2e8dcf1eaa96364eed1faf00fec9e98788) )
	ROM_LOAD16_BYTE( "guts-hi1.50b", 0x20000, 0x10000, CRC(a231f65d) SHA1(9c8ccd265ed0e9f6d7181d216ed41a0c5cc0cd5f) )
	ROM_LOAD16_BYTE( "guts-lo1.40b", 0x20001, 0x10000, CRC(dbdd4910) SHA1(9ca22321398b6397902aa99a3ef46f1a78ccc438) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 )
	ROM_LOAD( "guts-snd.10c", 0x00000, 0x10000, CRC(9fe065d7) SHA1(0d202af3d6c62fdcfc3bb2ea95bbf4e37c0d43cf) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "guts-mo0.bin", 0x00000, 0x10000, CRC(b8d8d8da) SHA1(6426607402aa9f1c872290910eefc57a8dd60e17) )
	ROM_LOAD( "guts-mo1.bin", 0x10000, 0x10000, CRC(d01b5a7f) SHA1(19aead56ddb92e0b1fc78d9065b1f139fe2a5668) )
	ROM_LOAD( "guts-mo2.bin", 0x20000, 0x10000, CRC(4577b807) SHA1(df0ead177342ab360cfab9b6defb7d0129d921e4) )
	ROM_LOAD( "guts-mo3.bin", 0x30000, 0x10000, CRC(4ab03c84) SHA1(b37d4abaaa42b9b847cc928a2b40b4e58ba9887f) )
	ROM_LOAD( "guts-mo4.bin", 0x40000, 0x10000, CRC(04cae4fb) SHA1(1b2cde0a97f687f67836f9eb09a45875a81c994a) )
	ROM_LOAD( "guts-mo5.bin", 0x50000, 0x10000, CRC(c65322ec) SHA1(0e77fb3db5760e12ee7b321906c2211e60a63ea4) )
	ROM_LOAD( "guts-mo6.bin", 0x60000, 0x10000, CRC(92602a5f) SHA1(a52376d326368e989cb7c3aa3913b918b4639307) )
	ROM_LOAD( "guts-mo7.bin", 0x70000, 0x10000, CRC(71a1911d) SHA1(6fd7c8b7a340466672e9db7a44e472e8e078e469) )
	ROM_LOAD( "guts-mo8.bin", 0x80000, 0x10000, CRC(aa273234) SHA1(41c310d53b4a847323cfd1f2080653cbc19216bf) )
	ROM_LOAD( "guts-mo9.bin", 0x90000, 0x10000, CRC(e85a12ef) SHA1(cbdb19e2e075c56288b9ed6aec07f03a2f265951) )
	ROM_LOAD( "guts-moa.bin", 0xa0000, 0x10000, CRC(da1cc76f) SHA1(ba91026464fec3cd94d1625c8780d5b18ecbb6ae) )
	ROM_LOAD( "guts-mob.bin", 0xb0000, 0x10000, CRC(246e7955) SHA1(cf146be2855d0a9c28c8da926c7fa381d06d5dd4) )
	ROM_LOAD( "guts-moc.bin", 0xc0000, 0x10000, CRC(1764c272) SHA1(0682fdc20cc1cd9355a50804a3f13c913c33f52b) )
	ROM_LOAD( "guts-mod.bin", 0xd0000, 0x10000, CRC(8220f2f6) SHA1(8a2900e223c203507c11e0185c2172ddddb0f4ee) )
	ROM_LOAD( "guts-moe.bin", 0xe0000, 0x10000, CRC(ee372eac) SHA1(bb1248bb3415853e16819a7b6b64ec67f87a2c58) )
	ROM_LOAD( "guts-mof.bin", 0xf0000, 0x10000, CRC(028ec56e) SHA1(8a95cffe5c00296e4df725335046a810efab533a) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "guts-alpha.bin", 0x00000, 0x04000, CRC(ee965058) SHA1(ccd3f6550bd5b531e8dd70ca88c30cdabf30e1e9) )

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT )
	ROM_LOAD( "guts-pf0.bin", 0x00000, 0x10000, CRC(1669fdb3) SHA1(d8d27b0f5877e215bf3d5343893c979860b1b45f) )
	ROM_LOAD( "guts-pf1.bin", 0x10000, 0x10000, CRC(135c41bd) SHA1(945ad9edbfcc5fe807cb15aa72b1be9e05974cb2) )
	ROM_LOAD( "guts-pf2.bin", 0x20000, 0x10000, CRC(c0408d39) SHA1(187ecea51f607c7f2565a9853c728e80118fefb1) )
	ROM_LOAD( "guts-pf4.bin", 0x40000, 0x10000, CRC(577f25a6) SHA1(3eaa3de6aa7e5b3938d34823f2cccc60e9b211e7) )
	ROM_LOAD( "guts-pf5.bin", 0x50000, 0x10000, CRC(43cbc0e3) SHA1(51e5f90391ebb402f715f389168baf401e3c03e9) )
	ROM_LOAD( "guts-pf6.bin", 0x60000, 0x10000, CRC(03c096f4) SHA1(fc0ffd5b61ab8bda37db508ec6dc5c70e1007187) )
	ROM_LOAD( "guts-pf8.bin", 0x80000, 0x10000, CRC(2f078b09) SHA1(e9b1aba767339d4677c3366d3f2df5a8deb5105e) )
	ROM_LOAD( "guts-pf9.bin", 0x90000, 0x10000, CRC(7cb7302d) SHA1(9d6ae58f1f64d1e28634dd42daedebb39570cd95) )
	ROM_LOAD( "guts-pfa.bin", 0xa0000, 0x10000, CRC(a3919dae) SHA1(0eba64afcc35e37322f9eb3a0e254026443ce092) )
	ROM_LOAD( "guts-pfc.bin", 0xc0000, 0x10000, CRC(7c571ee8) SHA1(044744ca95b2bd7486b0b057f1d7bdbd391958ab) )
	ROM_LOAD( "guts-pfd.bin", 0xd0000, 0x10000, CRC(979af5b2) SHA1(574a41552eb641668841cf01aeed442ccd3bc8e5) )
	ROM_LOAD( "guts-pfe.bin", 0xe0000, 0x10000, CRC(bf384e4d) SHA1(c4810b5a3ee754b169efa01f06941a02b50c53a0) )

	ROM_REGION( 0x40000, "jsa:oki1", 0 )   // ADPCM data
	ROM_LOAD( "guts-adpcm0.1f", 0x00000, 0x10000, CRC(92e9c35d) SHA1(dcc724f915e113bc34f499af9fd7c8ebb6d8ba98) )
	ROM_LOAD( "guts-adpcm1.1e", 0x10000, 0x10000, CRC(0afddd3a) SHA1(e1a43825ad02325a64869ec8048c8176da01b286) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, eprom,  0,     eprom, eprom, eprom_state, empty_init, ROT0, "Atari Games", "Escape from the Planet of the Robot Monsters (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, eprom2, eprom, eprom, eprom, eprom_state, empty_init, ROT0, "Atari Games", "Escape from the Planet of the Robot Monsters (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, klaxp1, klax,  klaxp, klaxp, eprom_state, empty_init, ROT0, "Atari Games", "Klax (prototype set 1)",                               MACHINE_SUPPORTS_SAVE )
GAME( 1989, klaxp2, klax,  klaxp, klaxp, eprom_state, empty_init, ROT0, "Atari Games", "Klax (prototype set 2)",                               MACHINE_SUPPORTS_SAVE )
GAME( 1989, guts,   0,     guts,  guts,  eprom_state, empty_init, ROT0, "Atari Games", "Guts n' Glory (prototype)",                            MACHINE_SUPPORTS_SAVE )
