// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Data East 32 bit ARM based lightgun games:

    Dragon Gun
    Locked 'n Loaded

    Emulation by Bryan McPhail, mish@tendril.co.uk.  Thank you to Tim,
    Avedis and Stiletto for many things including work on Fighter's
    History protection and tracking down Tattoo Assassins!

    For version information:
     Reset with Player 1 & 2 start held

    How to calibrate guns in lockload and clones:
    - keep SERVICE1 pressed (default 9) and press the test mode switch (default F2)
    - complete guns' calibration
    - exit test mode

    TODO:

    Video backgrounds(intel DVI) in dragngun*?
    reference video(dragngunj): https://youtu.be/TVc0SsTVJ94

    lockload, gunhard is a slightly different hardware
    revision: board # DE-0420-1 where the US set is DE-0359-2.
    The sound is _not_ hooked up correctly for this set.
	Music tempo is unverified (it has external timer / IRQ controller?).

    dragngun*: Sprite flickers during attract demo



Locked 'n Loaded (US)
Data East Corporation (c) 1994

DE-0359-2 PCB Layout - Same PCB as used for Dragon Gun, see comment below:

------------------------------------------------------------
|     32.220MHz   28.000MHz                 8M-7    8M-3   |
|                                          MBM-05  MBM-03  |
|         NH06-0   HuC6280A                 8M-5    8M-1   |
|           YM2151                         MBM-04  MBM-02  |
--+                                                        |
--+           MBM-07                                       |
|             MAR-07                74                     |
| M6295 M6295 MBM-06                       MBM-01  NH05-0  |
| M6295                                    MBM-00  NH04-0  |
|                                      74                  |
|J                                                         |
|A                                                         |
|M                                                         |
|M                                          2M-5    2M-4   |
|A  113                                    NH03-0  NH01-0  |
|                                  101      2M-7    2M-6   |
--+                                        NH02-0  NH00-0  |
--+ DSW1   146                                             |
|A             93C45                                       |
|U                                +-------------------------+
|X                                |         DE-0406-1       |
--|       ADC0808CCN              |       AUX PCB with      |
  --------------------------------|      Gun Connectors     |
                                  --------------------------+

2M-4 through 2M-7 are empty sockets for additional program ROMs (used by dragon Gun)
Odd numbered 8M are empty sockets
AUX edge connector is a 48 pin type similar to those used on Namco System 11, 12, etc


DE-0360-4 ROM board Layout:

------------------------------------------------------------
| CN2                   TC524256BZ-10 TC524256BZ-10  MAR-17|
|                       TC524256BZ-10 TC524256BZ-10  MAR-18|
| HM65256BLSP-10        TC524256BZ-10 TC524256BZ-10  MAR-19|
| 16 of these chips     TC524256BZ-10 TC524256BZ-10  MAR-20|
| in this area                                       MAR-21|
|                                       Intel i750   MAR-22|
|         187     23.000MHz                          MAR-23|
|MBM-08                                              MAR-24|
|MBM-09             20.0000MHz                       MAR-25|
|MBM-10                                      145     MAR-26|
|MBM-11  186                                         MAR-27|
|MBM-12                                              MAR-28|
|MBM-13                                                    |
|MBM-14 PAL16L8BCN                          Intel i750     |
|MBM-15 PAL16L8BCN                                         |
| CN1                           25.000MHz      PAL16L8BCN  |
------------------------------------------------------------

CN1 = Triple row 32 pin connector
CN2 = Dual row 32 pin connector

Locked 'n Loaded appears to be a conversion of Dragon Gun (c) 1993 as
there are 12 surface mounted GFX roms and 1 surface mounted sample rom
left over from the conversion.  The roms labeled "MAR-xx" are those
from Dragon Gun.


***************************************************************************/

#include "emu.h"

#include "deco_irq.h"
#include "deco16ic.h"
#include "deco146.h"
#include "decocrpt.h"
#include "namco_c355spr.h"

#include "cpu/arm/arm.h"
#include "cpu/h6280/h6280.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "sound/lc7535.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"

#include "speaker.h"

#include <algorithm>


namespace {

class dragngun_state : public driver_device
{
public:
	dragngun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_sprgen(*this, "c355spr")
		, m_deco_tilegen(*this, "tilegen%u", 1)
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_deco_irq(*this, "irq")
		, m_eeprom(*this, "eeprom")
		, m_ioprot(*this, "ioprot")
		, m_ym2151(*this, "ymsnd")
		, m_oki(*this, "oki%u", 1)
		, m_vol_main(*this, "vol_main")
		, m_vol_gun(*this, "vol_gun")
		, m_sprite_spriteformat(*this, "lay%u", 0)
		, m_sprite_spritetile(*this, "look%u", 0)
		, m_sprite_cliptable(*this, "spclip")
		, m_sprite_indextable(*this, "spindex")
		, m_pf_rowscroll32(*this, "pf%u_rowscroll32", 1)
		, m_paletteram(*this, "paletteram")
		, m_io_inputs(*this, "INPUTS")
		, m_io_light_x(*this, "LIGHT%u_X", 0U)
		, m_io_light_y(*this, "LIGHT%u_Y", 0U)
	{ }

	void dragngun(machine_config &config) ATTR_COLD;
	void lockload(machine_config &config) ATTR_COLD;
	void lockloadu(machine_config &config) ATTR_COLD;

	void init_dragngun() ATTR_COLD;
	void init_dragngunj() ATTR_COLD;
	void init_lockload() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(lockload_gun_trigger);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void sound_bankswitch_w(u8 data);
	void lockload_okibank_lo_w(u8 data);
	void lockload_okibank_hi_w(u8 data); // lockload
	u16 ioprot_r(offs_t offset);
	void ioprot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void volume_w(u8 data);
	void lc7535_volume_w(u8 data);
	void speaker_switch_w(u32 data);
	u32 lightgun_r();
	void lightgun_w(offs_t offset, u32 data = 0);
	void sprite_control_w(u32 data);
	void spriteram_dma_w(u32 data);
	void gun_irq_ack_w(u32 data);
	u32 unk_video_r();
	u32 lockload_gun_mirror_r(offs_t offset);

	template<int Chip> void pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void buffered_palette_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palette_dma_w(u32 data);

	int sprite_bank_callback(int sprite);
	u16 read_spritetile(int lookupram_offset);
	u16 read_spriteformat(int spriteformatram_offset, u8 attr);
	u16 read_spritetable(int offs, u8 attr);
	u16 read_spritelist(int offs);
	u16 read_cliptable(int offs, u8 attr);
	int sprite_priority_callback(int priority);

	bool sprite_mix_callback(u16 &dest, u8 &destpri, u16 colbase, u16 src, int srcpri, int pri);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECO16IC_BANK_CB_MEMBER(bank_1_callback);
	DECO16IC_BANK_CB_MEMBER(bank_2_callback);

	void expand_sprite_data() ATTR_COLD;
	void dragngun_init_common() ATTR_COLD;

	void namco_sprites(machine_config &config);

	void common_map(address_map &map) ATTR_COLD;
	void dragngun_map(address_map &map) ATTR_COLD;
	void h6280_sound_map(address_map &map) ATTR_COLD;
	void lockload_map(address_map &map) ATTR_COLD;
	void lockloadu_map(address_map &map) ATTR_COLD;
	void lockloadu_sound_map(address_map &map) ATTR_COLD;
	void z80_sound_io(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<namco_c355spr_device> m_sprgen;
	required_device_array<deco16ic_device, 2> m_deco_tilegen;
	required_device<buffered_spriteram32_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<deco_irq_device> m_deco_irq;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<deco146_device> m_ioprot;
	optional_device<ym2151_device> m_ym2151;
	optional_device_array<okim6295_device, 3> m_oki;
	optional_device<lc7535_device> m_vol_main;
	optional_device<lc7535_device> m_vol_gun;

	required_shared_ptr_array<u32, 2> m_sprite_spriteformat;
	required_shared_ptr_array<u32, 2> m_sprite_spritetile;
	required_shared_ptr<u32> m_sprite_cliptable;
	required_shared_ptr<u32> m_sprite_indextable;

	// we use the pointers below to store a 32-bit copy..
	required_shared_ptr_array<u32, 4> m_pf_rowscroll32;
	optional_shared_ptr<u32> m_paletteram;

	optional_ioport m_io_inputs;
	optional_ioport_array<2> m_io_light_x;
	optional_ioport_array<2> m_io_light_y;

	std::unique_ptr<u8[]> m_dirty_palette{};
	std::unique_ptr<u16[]> m_pf_rowscroll[4]{};

	u32 m_sprite_ctrl = 0U;
	u32 m_lightgun_port = 0;
	u32 m_oki2_bank = 0; // lockload
};


//**************************************************************************
//  PROTECTION
//**************************************************************************

u16 dragngun_state::ioprot_r(offs_t offset)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	return m_ioprot->read_data(deco146_addr, cs);
}

void dragngun_state::ioprot_w(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t real_address = 0 + (offset * 2);
	const offs_t deco146_addr = (BIT(real_address, 14, 4) << 11) | BIT(real_address, 0, 11); // NC 31-18, 13-11
	u8 cs = 0;

	m_ioprot->write_data(deco146_addr, data, mem_mask, cs);
}


//**************************************************************************
//  SOUND
//**************************************************************************

void dragngun_state::volume_w(u8 data)
{
	// TODO: Assumes linear scaling
	// values go from 0x00 (max volume) to 0xff (min volume)
	const u8 raw_vol = 0xff - data;
	const float vol_output = float(raw_vol) / 255.0f;

	m_ym2151->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[0]->set_output_gain(ALL_OUTPUTS, vol_output);
	m_oki[1]->set_output_gain(ALL_OUTPUTS, vol_output);
}

void dragngun_state::lc7535_volume_w(u8 data)
{
	m_vol_main->ce_w(BIT(data, 2));
	m_vol_main->clk_w(BIT(data, 1));
	m_vol_main->di_w(BIT(data, 0));

	m_vol_gun->ce_w(BIT(data, 2));
	m_vol_gun->clk_w(BIT(data, 1));
	m_vol_gun->di_w(BIT(data, 0));
}

void dragngun_state::speaker_switch_w(u32 data)
{
	bool gun_speaker_disabled = bool(BIT(data, 0));

	if (gun_speaker_disabled)
	{
		m_oki[2]->set_route_gain(0, m_vol_main, 0, 1.0);
		m_oki[2]->set_route_gain(0, m_vol_main, 1, 1.0);
		m_oki[2]->set_route_gain(0, m_vol_gun, 0, 0.0);
	}
	else
	{
		m_oki[2]->set_route_gain(0, m_vol_main, 0, 0.0);
		m_oki[2]->set_route_gain(0, m_vol_main, 1, 0.0);
		m_oki[2]->set_route_gain(0, m_vol_gun, 0, 1.0);
	}

	logerror("%s: Gun speaker: %s\n", machine().describe_context(), gun_speaker_disabled ? "Disabled" : "Enabled");
}

void dragngun_state::sound_bankswitch_w(u8 data)
{
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank((data >> 1) & 1);
}

void dragngun_state::lockload_okibank_lo_w(u8 data)
{
	m_oki2_bank = (m_oki2_bank & 2) | ((data >> 1) & 1);
	logerror("%s: Load OKI2 Bank Low bits: %02x, Current : %02x\n", machine().describe_context(), (data >> 1) & 1, m_oki2_bank);
	m_oki[0]->set_rom_bank((data >> 0) & 1);
	m_oki[1]->set_rom_bank(m_oki2_bank);
}

void dragngun_state::lockload_okibank_hi_w(u8 data)
{
	m_oki2_bank = (m_oki2_bank & 1) | ((data & 1) << 1); // TODO : Actually value unverified
	logerror("%s: Load OKI2 Bank Hi bits: %02x, Current : %02x\n", machine().describe_context(), ((data & 1) << 1), m_oki2_bank);
	m_oki[1]->set_rom_bank(m_oki2_bank);
}


//**************************************************************************
//  VIDEO
//**************************************************************************

void dragngun_state::video_start()
{
	for (int i = 0; i < 4; i++)
	{
		const u32 size = m_pf_rowscroll32[i].length();

		m_pf_rowscroll[i] = make_unique_clear<u16[]>(size);

		save_pointer(NAME(m_pf_rowscroll[i]), size, i);
	}
	m_dirty_palette = make_unique_clear<u8[]>(2048);

	save_item(NAME(m_sprite_ctrl));
	save_pointer(NAME(m_dirty_palette), 2048);
}

void dragngun_state::buffered_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	m_dirty_palette[offset] = 1;
}

void dragngun_state::palette_dma_w(u32 data)
{
	for (int i = 0; i < m_palette->entries(); i++)
	{
		if (m_dirty_palette[i])
		{
			m_dirty_palette[i] = 0;

			const u8 b = (m_paletteram[i] >> 16) & 0xff;
			const u8 g = (m_paletteram[i] >>  8) & 0xff;
			const u8 r = (m_paletteram[i] >>  0) & 0xff;

			m_palette->set_pen_color(i, rgb_t(r, g, b));
		}
	}
}

void dragngun_state::sprite_control_w(u32 data)
{
	m_sprite_ctrl = data;
}

void dragngun_state::spriteram_dma_w(u32 data)
{
	/* DMA spriteram to private sprite chip area, and clear cpu ram */
	m_spriteram->copy();
	memset(m_spriteram->live(), 0, 0x2000);
}

// tattass tests these as 32-bit ram, even if only 16-bits are hooked up to the tilemap chip - does it mirror parts of the dword?
template <int TileMap> void dragngun_state::pf_rowscroll_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_pf_rowscroll32[TileMap][offset]); data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf_rowscroll[TileMap][offset]); }

u32 dragngun_state::unk_video_r()
{
	return machine().rand();
}

DECO16IC_BANK_CB_MEMBER(dragngun_state::bank_1_callback)
{
	return (bank & ~0xf) << 8;
}

DECO16IC_BANK_CB_MEMBER(dragngun_state::bank_2_callback)
{
	return (bank & ~0x1f) << 7;
}

u16 dragngun_state::read_spritetile(int lookupram_offset)
{
	if (lookupram_offset & 0x2000)
		return m_sprite_spritetile[1][lookupram_offset&0x1fff];
	else
		return m_sprite_spritetile[0][lookupram_offset&0x1fff];
}

u16 dragngun_state::read_spriteformat(int spriteformatram_offset, u8 attr)
{
	if (spriteformatram_offset & 0x400)
		return m_sprite_spriteformat[1][((spriteformatram_offset & 0x1ff)<<2) + attr];
	else
		return m_sprite_spriteformat[0][((spriteformatram_offset & 0x1ff)<<2) + attr];
}

u16 dragngun_state::read_spritetable(int offs, u8 attr)
{
	return m_spriteram->buffer()[(offs << 3) + attr];
}

u16 dragngun_state::read_spritelist(int offs)
{
	return m_sprite_indextable[offs];
}

u16 dragngun_state::read_cliptable(int offs, u8 attr)
{
	return m_sprite_cliptable[(offs << 2) + attr];
}

int dragngun_state::sprite_priority_callback(int priority)
{
	/* For some reason, this bit when used in Dragon Gun causes the sprites
	   to flicker every other frame (fake transparency)

	   This would usually be a priority bit, but the flicker can't be a
	   post-process mixing effect filtering out those priorities, because then
	   it would still cut holes in sprites where it was drawn, and it doesn't.

	   Instead sprites with this priority must simple be disabled every other
	   frame.  maybe there's a register in the sprite chip to control this on
	   a per-priority level?
	*/

	if ((priority & 0x80) && (m_screen->frame_number() & 1)) // flicker
		return -1;

	priority = (priority & 0x60) >> 5;
	if (priority == 0) priority = 7;
	else if (priority == 1) priority = 7; // set to 1 to have the 'masking effect' with the dragon on the dragngun attract mode, but that breaks the player select where it needs to be 3, probably missing some bits..
	else if (priority == 2) priority = 7;
	else if (priority == 3) priority = 7;
	return priority;
}

int dragngun_state::sprite_bank_callback(int sprite)
{
	// High bits of the sprite reference into the sprite control bits for banking
	switch (sprite & 0x3000)
	{
		default:
		case 0x0000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x000f) << 12); break;
		case 0x1000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x00f0) << 8); break;
		case 0x2000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0x0f00) << 4); break;
		case 0x3000: sprite = (sprite & 0xfff) | ((m_sprite_ctrl & 0xf000) << 0); break;
	}

	// Because of the unusual interleaved rom layout, we have to mangle the bank bits
	// even further to suit our gfx decode
	switch (sprite & 0xf000)
	{
		case 0x0000: sprite = 0xc000 | (sprite & 0xfff); break;
		case 0x1000: sprite = 0xd000 | (sprite & 0xfff); break;
		case 0x2000: sprite = 0xe000 | (sprite & 0xfff); break;
		case 0x3000: sprite = 0xf000 | (sprite & 0xfff); break;

		case 0xc000: sprite = 0x0000 | (sprite & 0xfff); break;
		case 0xd000: sprite = 0x1000 | (sprite & 0xfff); break;
		case 0xe000: sprite = 0x2000 | (sprite & 0xfff); break;
		case 0xf000: sprite = 0x3000 | (sprite & 0xfff); break;
	}

	return sprite;
}

bool dragngun_state::sprite_mix_callback(u16 &dest, u8 &destpri, u16 colbase, u16 src, int srcpri, int pri)
{
	// TODO: proper priority handling
	if (srcpri >= destpri)
	{
		if ((src & 0xf) != 0xf)
		{
			dest = colbase + src;
			destpri = srcpri;
			return true;
		}
	}
	return false;
}


u32 dragngun_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x400), cliprect); // Palette index not confirmed

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0].get(), m_pf_rowscroll[1].get());
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2].get(), m_pf_rowscroll[3].get());

	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 1); // it uses pf3 in 8bpp mode instead, like captaven
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 8);

	// zooming sprite draw is very slow, and sprites are buffered.. however, one of the levels attempts to use
	// partial updates for every line, which causes things to be very slow... the sprites appear to support
	// multiple layers of alpha, so rendering to a buffer for layer isn't easy (maybe there are multiple sprite
	// chips at work?)
	//
	// really, it needs optimizing ..
	// so for now we only draw these 2 layers on the last update call
	if (cliprect.bottom() == 247)
	{
		rectangle clip(cliprect.left(), cliprect.right(), 8, 247);
		m_sprgen->clear_screen_bitmap(clip);
		m_sprgen->build_sprite_list_and_render_sprites(clip);
		m_sprgen->draw(screen, bitmap, clip);
	}

	return 0;
}


//**************************************************************************
//  INPUTS
//**************************************************************************

// TODO: probably clears both player 1 and player 2
void dragngun_state::gun_irq_ack_w(u32 data)
{
	m_deco_irq->lightgun_irq_ack_w(data);
}

// TODO: improve this, Y axis not understood at all
u32 dragngun_state::lockload_gun_mirror_r(offs_t offset)
{
//logerror("%08x:Read gun %d\n",m_maincpu->pc(),offset);

	switch (offset)
	{
		case 0:
			return ((m_io_inputs->read() & 0x30) << 5) | (m_io_light_x[0]->read()) | 0xffff800;

		case 1:
			return ((m_io_inputs->read() & 0x3000) >> 3) | (m_io_light_x[1]->read()) | 0xffff800;
	}

	return ~0;
}

u32 dragngun_state::lightgun_r()
{
	// Ports 0-3 are read, but seem unused
	switch (m_lightgun_port)
	{
		case 4: return m_io_light_x[0]->read();
		case 5: return m_io_light_x[1]->read();
		case 6: return m_io_light_y[0]->read();
		case 7: return m_io_light_y[1]->read();
	}

	//if (!machine().side_effects_disabled())
		//logerror("%s: Illegal lightgun port %d read \n", machine().describe_context(), m_lightgun_port);
	return 0;
}

void dragngun_state::lightgun_w(offs_t offset, u32 data)
{
//  logerror("%s: Lightgun port %d\n", machine().describe_context(), m_lightgun_port);
	m_lightgun_port = offset;
}

INPUT_CHANGED_MEMBER(dragngun_state::lockload_gun_trigger)
{
	switch (param)
	{
		case 0: m_deco_irq->lightgun1_trigger_w(!newval); break;
		case 1: m_deco_irq->lightgun2_trigger_w(!newval); break;
	}
}


//**************************************************************************
//  EEPROM
//**************************************************************************

u8 dragngun_state::eeprom_r()
{
	return 0xfe | m_eeprom->do_read();
}

void dragngun_state::eeprom_w(u8 data)
{
	// 76543---  unknown
	// -----2--  eeprom cs
	// ------1-  eeprom clk
	// -------0  eeprom di

	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void dragngun_state::common_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0x000000);
	map(0x100000, 0x11ffff).ram();
	map(0x120000, 0x127fff).rw(FUNC(dragngun_state::ioprot_r), FUNC(dragngun_state::ioprot_w)).umask32(0x0000ffff);
	map(0x128000, 0x12800f).m(m_deco_irq, FUNC(deco_irq_device::map)).umask32(0x000000ff);
	map(0x130000, 0x131fff).ram().w(FUNC(dragngun_state::buffered_palette_w)).share(m_paletteram);
	map(0x138008, 0x13800b).w(FUNC(dragngun_state::palette_dma_w));

	//  0x2xxxxx Namco Zooming Sprites
	map(0x204800, 0x204fff).ram().share(m_sprite_cliptable);// (all entries set to 320x256 here)
	map(0x208000, 0x208fff).ram().share(m_sprite_spriteformat[0]);
	map(0x20c000, 0x20cfff).ram().share(m_sprite_spriteformat[1]);
	map(0x210000, 0x217fff).ram().share(m_sprite_spritetile[0]);
	map(0x218000, 0x21ffff).ram().share(m_sprite_spritetile[1]);
	map(0x220000, 0x221fff).ram().share("spriteram"); // Main spriteram
	map(0x228000, 0x2283ff).ram().share(m_sprite_indextable); // sprite index (just a table 0x00-0xff here)
	map(0x230000, 0x230003).w(FUNC(dragngun_state::spriteram_dma_w));

	map(0x180000, 0x18001f).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x190000, 0x191fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x194000, 0x195fff).rw(m_deco_tilegen[0], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w));
	map(0x1a0000, 0x1a3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<0>)).share(m_pf_rowscroll32[0]);
	map(0x1a4000, 0x1a5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<1>)).share(m_pf_rowscroll32[1]);
	map(0x1c0000, 0x1c001f).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf_control_dword_r), FUNC(deco16ic_device::pf_control_dword_w));
	map(0x1d0000, 0x1d1fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf1_data_dword_r), FUNC(deco16ic_device::pf1_data_dword_w));
	map(0x1d4000, 0x1d5fff).rw(m_deco_tilegen[1], FUNC(deco16ic_device::pf2_data_dword_r), FUNC(deco16ic_device::pf2_data_dword_w)); // unused
	map(0x1e0000, 0x1e3fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<2>)).share(m_pf_rowscroll32[2]);
	map(0x1e4000, 0x1e5fff).ram().w(FUNC(dragngun_state::pf_rowscroll_w<3>)).share(m_pf_rowscroll32[3]); // unused

	map(0x300000, 0x3fffff).rom().region("maincpu", 0x100000);
	map(0x420000, 0x420000).rw(FUNC(dragngun_state::eeprom_r), FUNC(dragngun_state::eeprom_w));
	map(0x440000, 0x440003).portr("IN2");
	map(0x500000, 0x500003).w(FUNC(dragngun_state::sprite_control_w));
}

// the video drawing (especially sprite) code on this is too slow to cope with proper partial updates
// raster effects appear to need some work on it anyway?
void dragngun_state::dragngun_map(address_map &map)
{
	common_map(map);

//  map(0x01204c0, 0x01204c3).w(FUNC(dragngun_state::sound_w));

	map(0x0138000, 0x0138003).noprw(); // Palette dma complete in bit 0x8? ack?  return 0 else tight loop
//  map(0x0150000, 0x0150003).nopw(); // Unknown; Masking related?
//  map(0x0160000, 0x0160003).w(FUNC(dragngun_state::pri_w)); // priority
	map(0x0170100, 0x0170103).nopw();
	map(0x0170038, 0x017003b).nopw();
	map(0x017002C, 0x017002f).nopw();
	map(0x0170224, 0x0170227).nopw();
	map(0x0400000, 0x0400000).rw("oki3", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0410000, 0x0410000).w(FUNC(dragngun_state::lc7535_volume_w));
	map(0x0418000, 0x0418003).w(FUNC(dragngun_state::speaker_switch_w));
	map(0x0430000, 0x043001f).w(FUNC(dragngun_state::lightgun_w));
	map(0x0438000, 0x0438003).r(FUNC(dragngun_state::lightgun_r));
	// this is clearly the dvi video related area
	map(0x1000000, 0x1000007).r(FUNC(dragngun_state::unk_video_r));
	map(0x1000100, 0x1007fff).ram();
	map(0x10b0000, 0x10b01ff).ram();

	// reads from here during boss battles when the videos should be displayed at the offsets where the DVI headers are
	// as a result it ends up writing what looks like pointers to the frame data in the ram area above
	map(0x1400000, 0x1ffffff).rom().region("dvi", 0x00000);
}

void dragngun_state::lockloadu_map(address_map &map)
{
	dragngun_map(map);
	map(0x0170000, 0x0170007).r(FUNC(dragngun_state::lockload_gun_mirror_r)); // Not on Dragongun
}

void dragngun_state::lockload_map(address_map &map)
{
	common_map(map);

	map(0x138000, 0x138003).readonly().nopw(); //palette dma complete in bit 0x8? ack?  return 0 else tight loop
	map(0x170000, 0x170007).r(FUNC(dragngun_state::lockload_gun_mirror_r)); // Not on Dragongun
	map(0x178008, 0x17800f).w(FUNC(dragngun_state::gun_irq_ack_w)); // Gun read ACK's

	map(0x410000, 0x410000).w(FUNC(dragngun_state::volume_w));
}

// H6280 based sound
void dragngun_state::h6280_sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x110000, 0x110001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140000).r(m_ioprot, FUNC(deco146_device::soundlatch_r));
	map(0x1f0000, 0x1f1fff).ram();
}

// lockload needs hi bits of OKI2 bankswitching
void dragngun_state::lockloadu_sound_map(address_map &map)
{
	h6280_sound_map(map);
	map(0x150000, 0x150000).w(FUNC(dragngun_state::lockload_okibank_hi_w));
}

// Z80 based sound
void dragngun_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xb000, 0xb000).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc000, 0xc000).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd000, 0xd000).r(m_ioprot, FUNC(deco146_device::soundlatch_r));
	map(0xe000, 0xe000).w(FUNC(dragngun_state::lockload_okibank_hi_w));
}

void dragngun_state::z80_sound_io(address_map &map)
{
	map(0x0000, 0xffff).rom().region("audiocpu", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( dragngun )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // Would be a dipswitch, but only 1 present on board
	PORT_DIPNAME( 0x0100, 0x0000, "Reset" ) // Behaves like Reset
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Stage Select" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // is this actually vblank?
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) //check  //test BUTTON F2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lockload )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dragngun_state::lockload_gun_trigger), 0)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dragngun_state::lockload_gun_trigger), 1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Reload")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // Would be a dipswitch, but only 1 present on board
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Reset" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED ) //check  //test BUTTON F2

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 49, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_MINMAX(16,82)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 49, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2) PORT_MINMAX(16,82)
INPUT_PORTS_END


//**************************************************************************
//  GFXDECODE LAYOUTS
//**************************************************************************

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8    // every char takes 8 consecutive bytes
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static const gfx_layout tilelayout_8bpp =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0, 8, 0 },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	64*8
};

static GFXDECODE_START( gfx_dragngun )
	GFXDECODE_ENTRY( "tiles1", 0, charlayout,      0, 64 ) // Characters 8x8
	GFXDECODE_ENTRY( "tiles2", 0, tilelayout,      0, 64 ) // Tiles 16x16
	GFXDECODE_ENTRY( "tiles3", 0, tilelayout_8bpp, 0,  8 ) // Tiles 16x16
GFXDECODE_END


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void dragngun_state::machine_start()
{
	save_item(NAME(m_lightgun_port));
	save_item(NAME(m_oki2_bank));
}


void dragngun_state::namco_sprites(machine_config &config)
{
	NAMCO_C355SPR(config, m_sprgen);
	m_sprgen->set_tile_callback(FUNC(dragngun_state::sprite_bank_callback));
	m_sprgen->set_palette(m_palette);
	m_sprgen->set_transparent_pen(15);
	m_sprgen->set_colors(32);
	m_sprgen->set_granularity(16);
	m_sprgen->set_read_spritetile(FUNC(dragngun_state::read_spritetile));
	m_sprgen->set_read_spriteformat(FUNC(dragngun_state::read_spriteformat));
	m_sprgen->set_read_spritetable(FUNC(dragngun_state::read_spritetable));
	m_sprgen->set_read_spritelist(FUNC(dragngun_state::read_spritelist));
	m_sprgen->set_read_cliptable(FUNC(dragngun_state::read_cliptable));
	m_sprgen->set_priority_callback(FUNC(dragngun_state::sprite_priority_callback));
	m_sprgen->set_mix_callback(FUNC(dragngun_state::sprite_mix_callback));
	m_sprgen->set_device_allocates_spriteram(false);
	m_sprgen->set_alt_precision(true);
}

// DE-0359-2 + Bottom board DE-0360-4
void dragngun_state::dragngun(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::dragngun_map);

	h6280_device &audiocpu(H6280(config, m_audiocpu, 32.22_MHz_XTAL / 4 / 3)); // assume same as captaven
	audiocpu.set_addrmap(AS_PROGRAM, &dragngun_state::h6280_sound_map);
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 0); // internal sound unused
	audiocpu.add_route(ALL_OUTPUTS, "speaker", 0, 1);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline("maincpu", ARM_IRQ_LINE);

	DECO_IRQ(config, m_deco_irq);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(dragngun_state::screen_update));
	//m_screen->set_palette(m_palette);

	BUFFERED_SPRITERAM32(config, m_spriteram);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);
	m_deco_tilegen[1]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x04);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x03);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(dragngun_state::bank_2_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	namco_sprites(config);

	// I750, these aren't emulated
	//I82750PB(config, m_i82750pb, XTAL(25'000'000));
	//I82750DB(config, m_i82750db, XTAL(25'000'000));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dragngun);
	PALETTE(config, m_palette).set_entries(2048);

	DECO146PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set_inputline(m_audiocpu, 0);
	m_ioprot->set_interface_scramble_reverse();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	SPEAKER(config, "gun_speaker").front_center();

	LC7535(config, m_vol_main);
	m_vol_main->select_cb().set_constant(1);
	m_vol_main->add_route(0, "speaker", 1.0, 0);
	m_vol_main->add_route(1, "speaker", 1.0, 1);

	LC7535(config, m_vol_gun);
	m_vol_gun->select_cb().set_constant(0);
	m_vol_gun->add_route(0, "gun_speaker", 1.0);

	YM2151(config, m_ym2151, 32.22_MHz_XTAL / 9);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 1);
	m_ym2151->port_write_handler().set(FUNC(dragngun_state::sound_bankswitch_w));
	m_ym2151->add_route(0, m_vol_main, 0.42, 0);
	m_ym2151->add_route(1, m_vol_main, 0.42, 1);

	OKIM6295(config, m_oki[0], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, m_vol_main, 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, m_vol_main, 1.0, 1);

	OKIM6295(config, m_oki[1], 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, m_vol_main, 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, m_vol_main, 0.35, 1);

	OKIM6295(config, m_oki[2], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[2]->add_route(ALL_OUTPUTS, m_vol_main, 0.0, 0);
	m_oki[2]->add_route(ALL_OUTPUTS, m_vol_main, 0.0, 1);
	m_oki[2]->add_route(ALL_OUTPUTS, m_vol_gun, 1.0, 0);
}

void dragngun_state::lockloadu(machine_config &config)
{
	dragngun(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockloadu_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockloadu_sound_map);

	m_deco_irq->lightgun_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<2>));

	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);    // lockload definitely wants pf34 half width..

	m_ym2151->port_write_handler().set(FUNC(dragngun_state::lockload_okibank_lo_w));
}

// DE-0420-1 + Bottom board DE-0421-0
void dragngun_state::lockload(machine_config &config)
{
	// basic machine hardware
	ARM(config, m_maincpu, 28_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dragngun_state::lockload_map);

	INPUT_MERGER_ANY_HIGH(config, "irq_merger").output_handler().set_inputline("maincpu", ARM_IRQ_LINE);

	Z80(config, m_audiocpu, 32.22_MHz_XTAL / 9);
	m_audiocpu->set_addrmap(AS_PROGRAM, &dragngun_state::z80_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &dragngun_state::z80_sound_io);

	INPUT_MERGER_ANY_HIGH(config, "sound_irq_merger").output_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	config.set_maximum_quantum(attotime::from_hz(6000));  // to improve main<->audio comms

	DECO_IRQ(config, m_deco_irq);
	m_deco_irq->set_screen_tag(m_screen);
	m_deco_irq->lightgun1_callback().set_ioport("LIGHT0_Y");
	m_deco_irq->lightgun2_callback().set_ioport("LIGHT1_Y");
	m_deco_irq->raster2_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_deco_irq->vblank_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_deco_irq->lightgun_irq_callback().set("irq_merger", FUNC(input_merger_any_high_device::in_w<2>));

	EEPROM_93C46_16BIT(config, m_eeprom);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(28_MHz_XTAL / 4, 442, 0, 320, 274, 8, 248);
	m_screen->set_screen_update(FUNC(dragngun_state::screen_update));

	BUFFERED_SPRITERAM32(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dragngun);
	PALETTE(config, m_palette).set_entries(2048);

	DECO16IC(config, m_deco_tilegen[0]);
	m_deco_tilegen[0]->set_pf1_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf2_size(DECO_64x32);
	m_deco_tilegen[0]->set_pf1_col_bank(0x20);
	m_deco_tilegen[0]->set_pf2_col_bank(0x30);
	m_deco_tilegen[0]->set_pf1_col_mask(0x0f);
	m_deco_tilegen[0]->set_pf2_col_mask(0x0f);
	m_deco_tilegen[0]->set_bank1_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_bank2_callback(FUNC(dragngun_state::bank_1_callback));
	m_deco_tilegen[0]->set_pf12_8x8_bank(0);
	m_deco_tilegen[0]->set_pf12_16x16_bank(1);
	m_deco_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO16IC(config, m_deco_tilegen[1]);
	m_deco_tilegen[1]->set_pf1_size(DECO_32x32);
	m_deco_tilegen[1]->set_pf2_size(DECO_32x32);    // lockload definitely wants pf34 half width..
	m_deco_tilegen[1]->set_pf1_col_bank(0x04);
	m_deco_tilegen[1]->set_pf2_col_bank(0x04);
	m_deco_tilegen[1]->set_pf1_col_mask(0x03);
	m_deco_tilegen[1]->set_pf2_col_mask(0x03);
	m_deco_tilegen[1]->set_bank1_callback(FUNC(dragngun_state::bank_2_callback));
	// no bank2 callback
	m_deco_tilegen[1]->set_pf12_8x8_bank(0);
	m_deco_tilegen[1]->set_pf12_16x16_bank(2);
	m_deco_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	namco_sprites(config);

	DECO146PROT(config, m_ioprot);
	m_ioprot->port_a_cb().set_ioport("INPUTS");
	m_ioprot->port_b_cb().set_ioport("SYSTEM");
	m_ioprot->port_c_cb().set_ioport("DSW");
	m_ioprot->soundlatch_irq_cb().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<0>));
	m_ioprot->set_interface_scramble_reverse();

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, 32.22_MHz_XTAL / 9);
	m_ym2151->irq_handler().set("sound_irq_merger", FUNC(input_merger_any_high_device::in_w<1>));
	m_ym2151->port_write_handler().set(FUNC(dragngun_state::lockload_okibank_lo_w));
	m_ym2151->add_route(0, "speaker", 0.42, 0);
	m_ym2151->add_route(1, "speaker", 0.42, 1);

	OKIM6295(config, m_oki[0], 32.22_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	OKIM6295(config, m_oki[1], 32.22_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.35, 1);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( dragngun )
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "kb02.a9",  0x000000, 0x40000, CRC(4fb9cfea) SHA1(e20fbae32682fc5fdc82070d2d6c73b5b7ac13f8) )
	ROM_LOAD32_BYTE( "kb06.c9",  0x000001, 0x40000, CRC(2395efec) SHA1(3c08299a6cdeebf9d3d5d367ab435eec76986194) )
	ROM_LOAD32_BYTE( "kb00.a5",  0x000002, 0x40000, CRC(1539ff35) SHA1(6c82fe01f5ebf5cdd3a914cc823499fa6a26f9a9) )
	ROM_LOAD32_BYTE( "kb04.c5",  0x000003, 0x40000, CRC(5b5c1ec2) SHA1(3c5c02b7e432cf1861e0c8db23b302dc47774a42) )
	ROM_LOAD32_BYTE( "kb03.a10", 0x100000, 0x40000, CRC(6c6a4f42) SHA1(ae96fe81f9ba587eb3194dbffa0233413d63c4c6) )
	ROM_LOAD32_BYTE( "kb07.c10", 0x100001, 0x40000, CRC(2637e8a1) SHA1(7bcd1b1f3a4e6aaa0a3b78ca77dc666948c87547) )
	ROM_LOAD32_BYTE( "kb01.a7",  0x100002, 0x40000, CRC(d780ba8d) SHA1(0e315c718c038962b6020945b48bcc632de6f5e1) )
	ROM_LOAD32_BYTE( "kb05.c7",  0x100003, 0x40000, CRC(fbad737b) SHA1(04e16abe8c4cec4f172bea29516535511db9db90) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "kb10.n25",  0x00000,  0x10000,  CRC(ec56f560) SHA1(feb9491683ba7f1000edebb568d6b3471fcc87fb) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "kb08.a15",  0x00000,  0x10000,  CRC(8fe4e5f5) SHA1(922b94f8ce0c35e965259c11e95891ef4be913d4) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "kb09.a17",  0x00001,  0x10000,  CRC(e9dcac3f) SHA1(0621e601ffae73bbf69623042c9c8ab0526c3de6) )

	ROM_REGION( 0x120000, "tiles2", 0 )
	ROM_LOAD( "mar-00.bin",    0x000000,  0x80000,  CRC(d0491a37) SHA1(cc0ae1e9e5f42ba30159fb79bccd2e237cd037d0) ) // Encrypted tiles
	ROM_LOAD( "mar-01.bin",    0x090000,  0x80000,  CRC(d5970365) SHA1(729baf1efbef15c9f3e1d700717f5ba4f10d3014) )

	ROM_REGION( 0x400000, "tiles3", 0 )
	ROM_LOAD( "mar-02.bin",  0x000000, 0x40000,  CRC(c6cd4baf) SHA1(350286829a330b64f463d0a9cbbfdb71eecf5188) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x100000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-03.bin",  0x040000, 0x40000,  CRC(793006d7) SHA1(7d8aba2fe75917f580a3a931a7defe5939a0874e) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-04.bin",  0x080000, 0x40000,  CRC(56631a2b) SHA1(0fa3d6215df8ce923c153b96f39161ba88b2dd53) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x180000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-05.bin",  0x0c0000, 0x40000,  CRC(ac16e7ae) SHA1(dca32e0a677a99f47a7b8e8f105483c57382f218) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mar-15.bin", 0x000000, 0x100000,  CRC(ec976b20) SHA1(c120b3c56d5e02162e41dc7f726c260d0f8d2f1a) )
	ROM_LOAD32_BYTE( "mar-13.bin", 0x000001, 0x100000,  CRC(d675821c) SHA1(ff195422d0bef62d1f9c7784bba1e6b7ab5cd211) )
	ROM_LOAD32_BYTE( "mar-11.bin", 0x000002, 0x100000,  CRC(1fc638a4) SHA1(003dcfbb65a8f32a1a030502a11432287cf8b4e0) )
	ROM_LOAD32_BYTE( "mar-09.bin", 0x000003, 0x100000,  CRC(18fec9e1) SHA1(1290a9c13b4fd7d2197b39ec616206796e3a17a8) )
	ROM_LOAD32_BYTE( "mar-16.bin", 0x400000, 0x100000,  CRC(8b329bc8) SHA1(6e34eb6e2628a01a699d20a5155afb2febc31255) )
	ROM_LOAD32_BYTE( "mar-14.bin", 0x400001, 0x100000,  CRC(22d38c71) SHA1(62273665975f3e6000fa4b01755aeb70e5dd002d) )
	ROM_LOAD32_BYTE( "mar-12.bin", 0x400002, 0x100000,  CRC(4c412512) SHA1(ccd5014bc9f9648cf5fa56bb8d54fc72a7099ca3) )
	ROM_LOAD32_BYTE( "mar-10.bin", 0x400003, 0x100000,  CRC(73126fbc) SHA1(9b9c31335e4db726863b219072c83810008f88f9) )

	// this is standard DVI data, see http://www.fileformat.info/format/dvi/egff.htm
	// there are DVI headers at 0x000000, 0x580000, 0x800000, 0xB10000, 0xB80000
	ROM_REGION32_LE( 0x1000000, "dvi", 0 ) // Video data - unused for now
	ROM_LOAD32_BYTE( "mar-17.bin",  0x000003,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-20.bin",  0x000002,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-28.bin",  0x000001,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-25.bin",  0x000000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) ) // 49 I / 53 S
	ROM_LOAD32_BYTE( "mar-18.bin",  0x400003,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
	ROM_LOAD32_BYTE( "mar-21.bin",  0x400002,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
	ROM_LOAD32_BYTE( "mar-27.bin",  0x400001,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
	ROM_LOAD32_BYTE( "mar-24.bin",  0x400000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
	ROM_LOAD32_BYTE( "mar-19.bin",  0x800003,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-22.bin",  0x800002,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-26.bin",  0x800001,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-23.bin",  0x800000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) ) // 49 I / 53 S

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mar-06.n17", 0x000000, 0x80000,  CRC(3e006c6e) SHA1(55786e0fde2bf6ba9802f3f4fa8d4c21625b976a) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mar-08.n21", 0x000000, 0x80000,  CRC(b9281dfd) SHA1(449faf5d36f3b970d0a9b483e2152a5f68604a77) )

	// TODO : Japan version uses first bank of oki3, US version uses second half. it has bankswitched dynamic? or address shuffle?
	ROM_REGION(0x80000, "oki3", 0 )
	// Remove this hack if oki3 bankswitching is verified
	ROM_LOAD( "mar-07.n19", 0x40000, 0x40000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
	ROM_CONTINUE(           0x00000, 0x40000 )
	//ROM_LOAD( "mar-07.n19", 0x000000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
ROM_END

ROM_START( dragngunj )
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "ka-02.a9",  0x000000, 0x40000, CRC(402a03f9) SHA1(cdd5da9e35191bd716eb6245360702adb6078a94) )
	ROM_LOAD32_BYTE( "ka-06.c9",  0x000001, 0x40000, CRC(26822853) SHA1(8a9e61c9ac9a5aa4b21f063f700acfebac8d408b) )
	ROM_LOAD32_BYTE( "ka-00.a5",  0x000002, 0x40000, CRC(cc9e321b) SHA1(591d5f13a558960dbf286ca4541be0e42b234f2f) )
	ROM_LOAD32_BYTE( "ka-04.c5",  0x000003, 0x40000, CRC(5fd9d935) SHA1(8fd87b05325fae84860bbf1e169a3946f3197307) )
	ROM_LOAD32_BYTE( "ka-03.a10", 0x100000, 0x40000, CRC(e213c859) SHA1(aa0610427bbaa22da7f44289fdf9baf37b636710) )
	ROM_LOAD32_BYTE( "ka-07.c10", 0x100001, 0x40000, CRC(f34c54eb) SHA1(6b67cdb1d2dcc272de96292254914a212ff351cd) )
	ROM_LOAD32_BYTE( "ka-01.a7",  0x100002, 0x40000, CRC(1b52364c) SHA1(151365adc26bc7d71a4d2fc73bca598d3aa09f81) )
	ROM_LOAD32_BYTE( "ka-05.c7",  0x100003, 0x40000, CRC(4c975f52) SHA1(3c6b287c77a049e3f8822ed9d545733e8ea3357b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ka-10.n25",  0x00000,  0x10000,  CRC(ec56f560) SHA1(feb9491683ba7f1000edebb568d6b3471fcc87fb) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "ka-08.a15",  0x00000,  0x10000,  CRC(8fe4e5f5) SHA1(922b94f8ce0c35e965259c11e95891ef4be913d4) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "ka-09.a17",  0x00001,  0x10000,  CRC(e9dcac3f) SHA1(0621e601ffae73bbf69623042c9c8ab0526c3de6) )

	ROM_REGION( 0x120000, "tiles2", 0 )
	ROM_LOAD( "mar-00.bin",    0x000000,  0x80000,  CRC(d0491a37) SHA1(cc0ae1e9e5f42ba30159fb79bccd2e237cd037d0) ) // Encrypted tiles
	ROM_LOAD( "mar-01.bin",    0x090000,  0x80000,  CRC(d5970365) SHA1(729baf1efbef15c9f3e1d700717f5ba4f10d3014) )

	ROM_REGION( 0x400000, "tiles3", 0 )
	ROM_LOAD( "mar-02.bin",  0x000000, 0x40000,  CRC(c6cd4baf) SHA1(350286829a330b64f463d0a9cbbfdb71eecf5188) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x100000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-03.bin",  0x040000, 0x40000,  CRC(793006d7) SHA1(7d8aba2fe75917f580a3a931a7defe5939a0874e) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-04.bin",  0x080000, 0x40000,  CRC(56631a2b) SHA1(0fa3d6215df8ce923c153b96f39161ba88b2dd53) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x180000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 3/4
	ROM_LOAD( "mar-05.bin",  0x0c0000, 0x40000,  CRC(ac16e7ae) SHA1(dca32e0a677a99f47a7b8e8f105483c57382f218) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mar-15.bin", 0x000000, 0x100000,  CRC(ec976b20) SHA1(c120b3c56d5e02162e41dc7f726c260d0f8d2f1a) )
	ROM_LOAD32_BYTE( "mar-13.bin", 0x000001, 0x100000,  CRC(d675821c) SHA1(ff195422d0bef62d1f9c7784bba1e6b7ab5cd211) )
	ROM_LOAD32_BYTE( "mar-11.bin", 0x000002, 0x100000,  CRC(1fc638a4) SHA1(003dcfbb65a8f32a1a030502a11432287cf8b4e0) )
	ROM_LOAD32_BYTE( "mar-09.bin", 0x000003, 0x100000,  CRC(18fec9e1) SHA1(1290a9c13b4fd7d2197b39ec616206796e3a17a8) )
	ROM_LOAD32_BYTE( "mar-16.bin", 0x400000, 0x100000,  CRC(8b329bc8) SHA1(6e34eb6e2628a01a699d20a5155afb2febc31255) )
	ROM_LOAD32_BYTE( "mar-14.bin", 0x400001, 0x100000,  CRC(22d38c71) SHA1(62273665975f3e6000fa4b01755aeb70e5dd002d) )
	ROM_LOAD32_BYTE( "mar-12.bin", 0x400002, 0x100000,  CRC(4c412512) SHA1(ccd5014bc9f9648cf5fa56bb8d54fc72a7099ca3) )
	ROM_LOAD32_BYTE( "mar-10.bin", 0x400003, 0x100000,  CRC(73126fbc) SHA1(9b9c31335e4db726863b219072c83810008f88f9) )

	ROM_REGION32_LE( 0x1000000, "dvi", 0 ) // Video data - unused for now
	ROM_LOAD32_BYTE( "mar-17.bin",  0x000003,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-20.bin",  0x000002,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-28.bin",  0x000001,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-25.bin",  0x000000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) ) // 49 I / 53 S
	ROM_LOAD32_BYTE( "mar-18.bin",  0x400003,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
	ROM_LOAD32_BYTE( "mar-21.bin",  0x400002,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
	ROM_LOAD32_BYTE( "mar-27.bin",  0x400001,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
	ROM_LOAD32_BYTE( "mar-24.bin",  0x400000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
	ROM_LOAD32_BYTE( "mar-19.bin",  0x800003,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) ) // 56 V / 41 A
	ROM_LOAD32_BYTE( "mar-22.bin",  0x800002,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) ) // 44 D / 56 V
	ROM_LOAD32_BYTE( "mar-26.bin",  0x800001,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) ) // 56 V / 53 S
	ROM_LOAD32_BYTE( "mar-23.bin",  0x800000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) ) // 49 I / 53 S

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mar-06.n17", 0x000000, 0x80000,  CRC(3e006c6e) SHA1(55786e0fde2bf6ba9802f3f4fa8d4c21625b976a) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mar-08.n21", 0x000000, 0x80000,  CRC(b9281dfd) SHA1(449faf5d36f3b970d0a9b483e2152a5f68604a77) )

	ROM_REGION(0x80000, "oki3", 0 )
	ROM_LOAD( "mar-07.n19", 0x000000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
ROM_END

ROM_START( lockload ) // Board No. DE-0420-1 + Bottom board DE-0421-0 slightly different hardware, a unique PCB and not a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nl-00-1.a6", 0x000002, 0x80000, CRC(7a39bf8d) SHA1(8b1a6407bab74b3960a243a6c04c0005a82126f1) )
	ROM_LOAD32_BYTE( "nl-01-1.a8", 0x000000, 0x80000, CRC(d23afcb7) SHA1(de7b5bc936a87cc6511d588b0bf082bbf745581c) )
	ROM_LOAD32_BYTE( "nl-02-1.d6", 0x000003, 0x80000, CRC(730e0168) SHA1(fdfa0d335c03c2c528326f90948e642f9ea43150) )
	ROM_LOAD32_BYTE( "nl-03-1.d8", 0x000001, 0x80000, CRC(51a53ece) SHA1(ee2c8858844a47fa1e83c30c06d78cf49219dc33) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nm-06-.p22",  0x00000,  0x10000,  CRC(31d1c245) SHA1(326e35e7ebd8ea761d90e856c50d86512327f2a5) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nl-04-.a15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nl-05-.a17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b21",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b22",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e21",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e22",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n19",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )
ROM_END

ROM_START( gunhard ) // Board No. DE-0420-1 + Bottom board DE-0421-0 slightly different hardware, a unique PCB and not a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nf-00-1.a6", 0x000002, 0x80000, CRC(2c8045d4) SHA1(4c900951d56bd22e30905969b8eb687d9b4363bd) )
	ROM_LOAD32_BYTE( "nf-01-1.a8", 0x000000, 0x80000, CRC(6f160117) SHA1(05738f61890e9d6d2b25330958c0e7369f2ff4a6) )
	ROM_LOAD32_BYTE( "nf-02-1.d6", 0x000003, 0x80000, CRC(bd353948) SHA1(ddcc12b3d1c8919eb7eb961d61f6286e6b37a58e) )
	ROM_LOAD32_BYTE( "nf-03-1.d8", 0x000001, 0x80000, CRC(118a9a72) SHA1(e0b2fd21f477e531d6a04256767874f13e031a48) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nj-06-1.p22",  0x00000,  0x10000,  CRC(31d1c245) SHA1(326e35e7ebd8ea761d90e856c50d86512327f2a5) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nf-04-1.a15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nf-05-1.a17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b21",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b22",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e21",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e22",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n19",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )
ROM_END

ROM_START( lockloadu ) // Board No. DE-0359-2 + Bottom board DE-0360-4, a Dragongun conversion
	ROM_REGION(0x200000, "maincpu", 0 ) // ARM 32 bit code
	ROM_LOAD32_BYTE( "nh-00-0.b5", 0x000002, 0x80000, CRC(b8a57164) SHA1(b700a08db2ad1aa1bf0a32635ffbd5d3f08713ee) )
	ROM_LOAD32_BYTE( "nh-01-0.b8", 0x000000, 0x80000, CRC(e371ac50) SHA1(c448b54bc8962844b490994607b21b0c806d7714) )
	ROM_LOAD32_BYTE( "nh-02-0.d5", 0x000003, 0x80000, CRC(3e361e82) SHA1(b5445d44f2a775c141fdc561d5489234c39445a4) )
	ROM_LOAD32_BYTE( "nh-03-0.d8", 0x000001, 0x80000, CRC(d08ee9c3) SHA1(9a85710a11940df047e83e8d5977a23d6c67d665) )

	ROM_REGION(0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "nh-06-0.n25",  0x00000,  0x10000,  CRC(7a1af51d) SHA1(54e6b16d3f5b787d3c6eb7203d8854e6e0fb9803) )

	ROM_REGION( 0x020000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "nh-04-0.b15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) // Encrypted tiles
	ROM_LOAD16_BYTE( "nh-05-0.b17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) // Encrypted tiles
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "tiles3", 0 )
	ROM_LOAD( "mbm-02.b23",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x200000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x400000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x600000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x040000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x240000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x440000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x640000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-03.b26",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x280000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x480000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x680000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x0c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x2c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x4c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x6c0000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-04.e23",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x300000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x500000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x700000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x140000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x340000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x540000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x740000, 0x40000 ) // 3/4
	ROM_LOAD( "mbm-05.e26",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) // Encrypted tiles 0/4
	ROM_CONTINUE(            0x380000, 0x40000 ) // 2 bpp per 0x40000 chunk, 1/4
	ROM_CONTINUE(            0x580000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x780000, 0x40000 ) // 3/4
	ROM_CONTINUE(            0x1c0000, 0x40000 ) // Next block 2bpp 0/4
	ROM_CONTINUE(            0x3c0000, 0x40000 ) // 1/4
	ROM_CONTINUE(            0x5c0000, 0x40000 ) // 2/4
	ROM_CONTINUE(            0x7c0000, 0x40000 ) // 3/4

	ROM_REGION( 0x800000*2, "c355spr", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000000, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000001, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000002, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000003, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400000, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400001, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400002, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400003, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )

	ROM_REGION32_LE( 0x1000000, "dvi", ROMREGION_ERASE00 ) // Video data - same as Dragongun, probably leftover from a conversion
//  ROM_LOAD( "mar-17.bin",  0x00000,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) )
//  ROM_LOAD( "mar-18.bin",  0x00000,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
//  ROM_LOAD( "mar-19.bin",  0x00000,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) )
//  ROM_LOAD( "mar-20.bin",  0x00000,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) )
//  ROM_LOAD( "mar-21.bin",  0x00000,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
//  ROM_LOAD( "mar-22.bin",  0x00000,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) )
//  ROM_LOAD( "mar-23.bin",  0x00000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) )
//  ROM_LOAD( "mar-24.bin",  0x00000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
//  ROM_LOAD( "mar-25.bin",  0x00000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) )
//  ROM_LOAD( "mar-26.bin",  0x00000,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) )
//  ROM_LOAD( "mar-27.bin",  0x00000,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
//  ROM_LOAD( "mar-28.bin",  0x00000,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) )

	// not sure why the IC positions are swapped compared to Dragon Gun
	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbm-07.n21",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x100000, "oki2", ROMREGION_ERASE00 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )

	ROM_REGION(0x80000, "oki3", ROMREGION_ERASE00 )
	ROM_LOAD( "mar-07.n19",  0x00000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )  // same as dragngun, unused
ROM_END


//**************************************************************************
//  INITIALIZER
//**************************************************************************

//extern void process_dvi_data(device_t *device,u8* dvi_data, int offset, int regionsize);

void dragngun_state::expand_sprite_data()
{
	u8 *rom = memregion("c355spr")->base();
	size_t len = memregion("c355spr")->bytes();

	for (int i = 0; i < len / 2; i++)
	{
		u8 temp = rom[i];
		rom[i + len / 2] = temp & 0x0f;
		rom[i] = (temp >> 4) & 0x0f;
	}
}

void dragngun_state::dragngun_init_common()
{
	const u8 *SRC_RAM = memregion("tiles1")->base();
	u8 *DST_RAM = memregion("tiles2")->base();

	deco74_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
	deco74_decrypt_gfx(machine(), "tiles3");

	std::copy(&SRC_RAM[0x00000], &SRC_RAM[0x10000], &DST_RAM[0x080000]);
	std::copy(&SRC_RAM[0x10000], &SRC_RAM[0x20000], &DST_RAM[0x110000]);

	expand_sprite_data();

#if 0
	{
		u8 *ROM = memregion("dvi")->base();

		auto fp = fopen("video.dvi", "w+b");
		if (fp)
		{
			fwrite(ROM, 0xc00000, 1, fp);
			fclose(fp);
		}
	}
#endif

	// there are DVI headers at 0x000000, 0x580000, 0x800000, 0xB10000, 0xB80000
//  process_dvi_data(this,memregion("dvi")->base(),0x000000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0x580000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0x800000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0xB10000, 0x1000000);
//  process_dvi_data(this,memregion("dvi")->base(),0xB80000, 0x1000000);
}

void dragngun_state::init_dragngun()
{
	dragngun_init_common();

	u32 *ROM = (u32 *)memregion("maincpu")->base();
	ROM[0x01b32c/4] = 0xe1a00000; // bl $ee000: NOP test switch lock
}

void dragngun_state::init_dragngunj()
{
	dragngun_init_common();

	u32 *ROM = (u32 *)memregion("maincpu")->base();
	ROM[0x01a1b4/4] = 0xe1a00000; // bl $ee000: NOP test switch lock
}

void dragngun_state::init_lockload()
{
//  u32 *ROM = (u32 *)memregion("maincpu")->base();

	deco74_decrypt_gfx(machine(), "tiles1");
	deco74_decrypt_gfx(machine(), "tiles2");
	deco74_decrypt_gfx(machine(), "tiles3");

//  ROM[0x1fe3c0/4] = 0xe1a00000;//  NOP test switch lock
//  ROM[0x1fe3cc/4] = 0xe1a00000;//  NOP test switch lock
//  ROM[0x1fe40c/4] = 0xe1a00000;//  NOP test switch lock

	expand_sprite_data();
}

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    MACHINE    INPUT     CLASS           INIT            ROT   COMPANY                  FULLNAME                                              FLAGS
// LTEST-02 Version 0.00, 1992-12-08 17:00
GAME( 1992, dragngun,   0,        dragngun,  dragngun, dragngun_state, init_dragngun,  ROT0, "Data East Corporation", "Dragon Gun (US, LTEST-02 Version 0.00, 1992-12-08)",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // OKI3 Bankswitching aren't verified
// Master Version 0.00 1992-11-24 09:00
GAME( 1992, dragngunj,  dragngun, dragngun,  dragngun, dragngun_state, init_dragngunj, ROT0, "Data East Corporation", "Dragon Gun (Japan, Master Version 0.00, 1992-11-24)",                           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ""

// Master-1 Version 0.0 1994-11-17 19:00
GAME( 1994, lockload,   0,        lockload,  lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Locked 'n Loaded (World, Master-1 Version 0.0, 1994-11-17)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // hangs during attract mode if let running for a while without coining up; shooting in the lower corners during calibration of player's 2 gun hangs the game
// Master-1 Version 0.0 1994-11-17 18:00
GAME( 1994, gunhard,    lockload, lockload,  lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Gun Hard (Japan, Master-1 Version 0.0, 1994-11-17)",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // "
// Master-0 Version 0.0 1994-11-09 23:30
GAME( 1994, lockloadu,  lockload, lockloadu, lockload, dragngun_state, init_lockload,  ROT0, "Data East Corporation", "Locked 'n Loaded (US Master-0 Version 0.0, 1994-11-09, Dragon Gun conversion)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // "
