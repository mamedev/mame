// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*******************************************************************************

   Run and Gun / Slam Dunk
   (c) 1993 Konami

   Driver by R. Belmont.

   This hardware uses the 55673 sprite chip like PreGX and System GX, but in a 4 bit
   per pixel layout.  There is also an all-TTL front overlay tilemap and a rotating
   scaling background done with the PSAC2 ('936).

   Status: Front tilemap should be complete, sprites are mostly correct, controls
   should be fine.

   Known Issues:
   - CRTC and video registers needs syncronization with current video draw state,
     it's very noticeable if for example scroll values are in very different states
     between screens.
   - Current draw state could be improved optimization-wise (for example by supporting
     it in the core in some way).
   - sprite palettes are not entirely right (fixed?)
   - sound volume mixing, handtune with set_gain() with m_k054539 devices.
     Also notice that "volume" in sound options is for k054539_1 (SFX)

*******************************************************************************/

#include "emu.h"

#include "k053246_k053247_k055673.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k053252.h"
#include "machine/k054321.h"
#include "sound/k054539.h"
#include "video/k053936.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "rungun_dual.lh"


namespace {

class rungun_state : public driver_device
{
public:
	rungun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_k054539(*this, "k054539_%u", 0),
		m_k053936(*this, "k053936"),
		m_k055673(*this, "k055673"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_palette2(*this, "palette2"),
		m_screen(*this, "screen"),
		m_k054321(*this, "k054321"),
		m_sysreg(*this, "sysreg"),
		m_bank2(*this, "bank2"),
		m_spriteram_bank(*this, "spriteram_bank"),
		m_p_inputs(*this, "P%u", 1U),
		m_dsw(*this, "DSW"),
		m_system(*this, "SYSTEM"),
		m_eepromout(*this, "EEPROMOUT")
	{ }

	void rng(machine_config &config);
	void rng_dual(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device_array<k054539_device, 2> m_k054539;
	required_device<k053936_device> m_k053936;
	required_device<k055673_device> m_k055673;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_palette2;
	required_device<screen_device> m_screen;
	required_device<k054321_device> m_k054321;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_sysreg;

	required_memory_bank m_bank2;
	required_memory_bank m_spriteram_bank;

	required_ioport_array<4> m_p_inputs;
	required_ioport m_dsw;
	required_ioport m_system;
	required_ioport m_eepromout;

	/* video-related */
	tilemap_t   *m_ttl_tilemap[2]{};
	tilemap_t   *m_936_tilemap[2]{};
	std::unique_ptr<uint16_t[]> m_psac2_vram;
	std::unique_ptr<uint16_t[]> m_ttl_vram;
	std::unique_ptr<uint16_t[]> m_pal_ram;
	uint8_t     m_current_display_bank = 0;
	int         m_ttl_gfx_index = 0;
	int         m_sprite_colorbase = 0;

	uint8_t     *m_roz_rom = nullptr;
	uint8_t     m_roz_rombase = 0;

	/* sound */
	uint8_t     m_sound_ctrl = 0;
	uint8_t     m_sound_nmi_clk = 0;

	bool        m_video_priority_mode = false;
	std::unique_ptr<uint16_t[]> m_banked_ram;
	bool        m_single_screen_mode = false;
	uint8_t     m_video_mux_bank = 0;

	uint16_t sysregs_r(offs_t offset, uint16_t mem_mask = ~0);
	void sysregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_ctrl_w(uint8_t data);
	uint16_t ttl_ram_r(offs_t offset);
	void ttl_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t psac2_videoram_r(offs_t offset);
	void psac2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t k53936_rom_r(offs_t offset);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rng_936_tile_info);
	void k054539_nmi_gen(int state);
	uint16_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	K055673_CB_MEMBER(sprite_callback);

	uint32_t screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint32_t screen_update_rng_dual_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rng_dual_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	bitmap_ind16 m_rng_dual_demultiplex_left_temp;
	bitmap_ind16 m_rng_dual_demultiplex_right_temp;
	void sprite_dma_trigger(void);

	INTERRUPT_GEN_MEMBER(rng_interrupt);

	void rungun_map(address_map &map) ATTR_COLD;
	void rungun_sound_map(address_map &map) ATTR_COLD;
};


uint16_t rungun_state::sysregs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
		case 0x00/2:
			return (m_p_inputs[0]->read() | m_p_inputs[2]->read() << 8);

		case 0x02/2:
			return (m_p_inputs[1]->read() | m_p_inputs[3]->read() << 8);


		case 0x04/2:
			/*
			    bit0-7: coin mechs and services
			    bit8 : freeze
			    bit9 : screen output select
			*/
			{
				uint8_t field_bit = m_screen->frame_number() & 1;
				if (m_single_screen_mode)
					field_bit = 1;
				return (m_system->read() & 0xfdff) | (field_bit << 9);
			}

		case 0x06/2:
			if (ACCESSING_BITS_0_7)
			{
				data = m_dsw->read();
			}
			return ((m_sysreg[0x06 / 2] & 0xff00) | data);
	}

	return m_sysreg[offset];
}

void rungun_state::sysregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_sysreg + offset);

	switch (offset)
	{
		case 0x08/2:
			/*
			    bit0  : eeprom_di_write
			    bit1  : eeprom_cs_write
			    bit2  : eeprom_clk_write
			    bit3  : coin counter #1
			    bit4  : coin counter #2 (when coin slot "common" is selected)
			    bit7  : set before massive memory writes (video chip select?)
			    bit10 : IRQ5 ACK
			    bit12 : if set, forces screen output to 1 monitor.
			    bit14 : (0) sprite on top of PSAC2 layer (1) other way around (title screen)
			*/
			if (ACCESSING_BITS_0_7)
			{
				m_spriteram_bank->set_entry((data & 0x80) >> 7);
				m_video_mux_bank = ((data & 0x80) >> 7) ^ 1;
				m_eepromout->write(data, 0xff);

				machine().bookkeeping().coin_counter_w(0, data & 0x08);
				machine().bookkeeping().coin_counter_w(1, data & 0x10);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_single_screen_mode = (data & 0x1000) == 0x1000;
				m_video_priority_mode = (data & 0x4000) == 0x4000;
				if (!(data & 0x400)) // actually a 0 -> 1 transition
					m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
			}
			break;

		case 0x0c/2:
			/*
			    bit 0  : also enables IRQ???
			    bit 1  : disable PSAC2 input?
			    bit 2  : OBJCHA
			    bit 3  : enable IRQ 5
			    bit 7-4: base address for 53936 ROM readback.
			*/
			m_k055673->k053246_set_objcha_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
			m_roz_rombase = (data & 0xf0) >> 4;
			break;
	}
}

void rungun_state::sound_irq_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		m_soundcpu->set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(rungun_state::rng_interrupt)
{
	// send to sprite device current state (i.e. bread & butter sprite DMA)
	// TODO: firing this in screen update causes sprites to desync badly ...
	sprite_dma_trigger();

	if (m_sysreg[0x0c / 2] & 0x09)
		device.execute().set_input_line(M68K_IRQ_5, ASSERT_LINE);
}

uint8_t rungun_state::k53936_rom_r(offs_t offset)
{
	// TODO: odd addresses returns ...?
	uint32_t rom_addr = offset;
	rom_addr += m_roz_rombase * 0x20000;
	return m_roz_rom[rom_addr];
}

uint16_t rungun_state::palette_r(offs_t offset)
{
	return m_pal_ram[offset + m_video_mux_bank*0x800/2];
}

void rungun_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t addr = offset + m_video_mux_bank*0x800/2;
	COMBINE_DATA(&m_pal_ram[addr]);

	uint8_t r = m_pal_ram[addr] & 0x1f;
	uint8_t g = (m_pal_ram[addr] & 0x3e0) >> 5;
	uint8_t b = (m_pal_ram[addr] & 0x7e00) >> 10;

	palette_device &cur_paldevice = m_video_mux_bank == 0 ? *m_palette : *m_palette2;
	cur_paldevice.set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

void rungun_state::rungun_map(address_map &map)
{
	map(0x000000, 0x2fffff).rom();                                         // main program + data
	map(0x300000, 0x3007ff).rw(FUNC(rungun_state::palette_r), FUNC(rungun_state::palette_w));
	map(0x380000, 0x39ffff).ram();                                         // work RAM
	map(0x400000, 0x43ffff).r(FUNC(rungun_state::k53936_rom_r)).umask16(0x00ff);               // '936 ROM readback window
	map(0x480000, 0x48001f).rw(FUNC(rungun_state::sysregs_r), FUNC(rungun_state::sysregs_w)).share("sysreg");
	map(0x4c0000, 0x4c001f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write)).umask16(0x00ff); // CCU (for scanline and vblank polling)
	map(0x540000, 0x540001).w(FUNC(rungun_state::sound_irq_w));
	map(0x580000, 0x58001f).m(m_k054321, FUNC(k054321_device::main_map)).umask16(0xff00);
	map(0x5c0000, 0x5c000f).r(m_k055673, FUNC(k055673_device::k055673_rom_word_r));                       // 246A ROM readback window
	map(0x5c0010, 0x5c001f).w(m_k055673, FUNC(k055673_device::k055673_reg_word_w));
	map(0x600000, 0x601fff).bankrw("spriteram_bank");                                                // OBJ RAM
	map(0x640000, 0x640007).w(m_k055673, FUNC(k055673_device::k053246_w));                      // '246A registers
	map(0x680000, 0x68001f).w(m_k053936, FUNC(k053936_device::ctrl_w));          // '936 registers
	map(0x6c0000, 0x6cffff).rw(FUNC(rungun_state::psac2_videoram_r), FUNC(rungun_state::psac2_videoram_w)); // PSAC2 ('936) RAM (34v + 35v)
	map(0x700000, 0x7007ff).rw(m_k053936, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w));          // PSAC "Line RAM"
	map(0x740000, 0x741fff).rw(FUNC(rungun_state::ttl_ram_r), FUNC(rungun_state::ttl_ram_w));     // text plane RAM
	map(0x7c0000, 0x7c0001).nopw();                                    // watchdog
}


/**********************************************************************************/

/* TTL text plane stuff */
TILE_GET_INFO_MEMBER(rungun_state::ttl_get_tile_info)
{
	uint32_t const base_addr = uintptr_t(tilemap.user_data());
	auto const lvram = util::little_endian_cast<uint8_t const>(m_ttl_vram.get()) + base_addr;

	int const attr = (lvram[tile_index << 2] & 0xf0) >> 4;
	int const code = ((lvram[tile_index << 2] & 0x0f) << 8) | lvram[(tile_index << 2) + 2];

	tileinfo.set(m_ttl_gfx_index, code, attr, 0);
}

K055673_CB_MEMBER(rungun_state::sprite_callback)
{
	*color = m_sprite_colorbase | (*color & 0x001f);
}

uint16_t rungun_state::ttl_ram_r(offs_t offset)
{
	return m_ttl_vram[offset + (m_video_mux_bank*0x1000)];
}

void rungun_state::ttl_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ttl_vram[offset + (m_video_mux_bank*0x1000)]);
	m_ttl_tilemap[m_video_mux_bank]->mark_tile_dirty(offset / 2);
}

/* 53936 (PSAC2) rotation/zoom plane */
uint16_t rungun_state::psac2_videoram_r(offs_t offset)
{
	return m_psac2_vram[offset+(m_video_mux_bank*0x80000)];
}

void rungun_state::psac2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_psac2_vram[offset+(m_video_mux_bank*0x80000)]);
	m_936_tilemap[m_video_mux_bank]->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(rungun_state::get_rng_936_tile_info)
{
	uint32_t base_addr = (uintptr_t)tilemap.user_data();
	int tileno, colour, flipx;

	tileno = m_psac2_vram[tile_index * 2 + 1 + base_addr] & 0x3fff;
	flipx = (m_psac2_vram[tile_index * 2 + 1 + base_addr] & 0xc000) >> 14;
	colour = 0x10 + (m_psac2_vram[tile_index * 2 + base_addr] & 0x000f);

	tileinfo.set(0, tileno, colour, TILE_FLIPYX(flipx));
}


void rungun_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	int gfx_index;

	m_ttl_vram = std::make_unique<uint16_t[]>(0x1000*2);
	m_psac2_vram = std::make_unique<uint16_t[]>(0x80000*2);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("gfx3")->base(), 0, m_palette->entries() / 16, 0));
	m_ttl_gfx_index = gfx_index;

	// create the tilemaps
	for (uint32_t screen_num = 0; screen_num < 2; screen_num++)
	{
		m_ttl_tilemap[screen_num] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rungun_state::ttl_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
		m_ttl_tilemap[screen_num]->set_user_data((void *)(uintptr_t)(screen_num * 0x2000));
		m_ttl_tilemap[screen_num]->set_transparent_pen(0);

		m_936_tilemap[screen_num] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rungun_state::get_rng_936_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
		m_936_tilemap[screen_num]->set_user_data((void *)(uintptr_t)(screen_num * 0x80000));
		m_936_tilemap[screen_num]->set_transparent_pen(0);

	}
	m_sprite_colorbase = 0x20;

	m_screen->register_screen_bitmap(m_rng_dual_demultiplex_left_temp);
	m_screen->register_screen_bitmap(m_rng_dual_demultiplex_right_temp);
}

uint32_t rungun_state::screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);
	m_current_display_bank = m_screen->frame_number() & 1;
	if (m_single_screen_mode)
		m_current_display_bank = 0;

	if (!m_video_priority_mode)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_936_tilemap[m_current_display_bank], 0, 0, 1);
		m_k055673->k053247_sprites_draw(bitmap, cliprect);
	}
	else
	{
		m_k055673->k053247_sprites_draw(bitmap, cliprect);
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_936_tilemap[m_current_display_bank], 0, 0, 1);
	}

	m_ttl_tilemap[m_current_display_bank]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// the 60hz signal gets split between 2 screens
uint32_t rungun_state::screen_update_rng_dual_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int m_current_display_bank = m_screen->frame_number() & 1;

	if (!m_current_display_bank)
		screen_update_rng(screen, m_rng_dual_demultiplex_left_temp, cliprect);
	else
		screen_update_rng(screen, m_rng_dual_demultiplex_right_temp, cliprect);

	copybitmap( bitmap, m_rng_dual_demultiplex_left_temp, 0, 0, 0, 0, cliprect);
	return 0;
}

// this depends upon the first screen being updated, and the bitmap being copied to the temp bitmap
uint32_t rungun_state::screen_update_rng_dual_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, m_rng_dual_demultiplex_right_temp, 0, 0, 0, 0, cliprect);
	return 0;
}

void rungun_state::sprite_dma_trigger(void)
{
	uint32_t src_address;

	if (m_single_screen_mode)
		src_address = 1*0x2000;
	else
		src_address = m_current_display_bank*0x2000;

	// TODO: size could be programmable somehow.
	for (int i = 0; i < 0x1000; i += 2)
		m_k055673->k053247_word_w(i / 2, m_banked_ram[(i + src_address) / 2]);
}


/**********************************************************************************/

void rungun_state::sound_ctrl_w(uint8_t data)
{
	/*
	    .... xxxx - Z80 ROM bank
	    ...x .... - NMI enable/acknowledge
	    xx.. .... - BLT2/1 (?)
	*/

	m_bank2->set_entry(data & 0x07);

	if (!(data & 0x10))
		m_soundcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_sound_ctrl = data;
}

void rungun_state::k054539_nmi_gen(int state)
{
	if (m_sound_ctrl & 0x10)
	{
		// Trigger an /NMI on the rising edge
		if (!m_sound_nmi_clk && state)
		{
			m_soundcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		}
	}

	m_sound_nmi_clk = state;
}

/* sound (this should be split into audio/xexex.cpp or pregx.cpp or so someday) */

void rungun_state::rungun_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank2");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe22f).rw(m_k054539[0], FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xe230, 0xe3ff).ram();
	map(0xe400, 0xe62f).rw(m_k054539[1], FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xe630, 0xe7ff).ram();
	map(0xf000, 0xf003).m(m_k054321, FUNC(k054321_device::sound_map));
	map(0xf800, 0xf800).w(FUNC(rungun_state::sound_ctrl_w));
	map(0xfff0, 0xfff3).nopw();
}


static INPUT_PORTS_START( rng )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_DIPNAME( 0x0100, 0x0000, "Freeze" )
	PORT_DIPSETTING( 0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Field Bit (DEBUG)" )
	PORT_DIPSETTING( 0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x0200, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_DIPNAME( 0x04, 0x04, "Bit2 (Unknown)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, "Monitors" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPNAME( 0x20, 0x00, "Number of players" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0x40, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x40, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x80, 0x80, "Bit7 (Unknown)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))

	PORT_START("P1")
	KONAMI8_B123_START(1)

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("P3")
	KONAMI8_B123_START(3)

	PORT_START("P4")
	KONAMI8_B123_START(4)
INPUT_PORTS_END

static INPUT_PORTS_START( rng_dual )
	PORT_INCLUDE(rng)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, "Monitors" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPNAME( 0x20, 0x20, "Number of players" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( rng_nodip )
	PORT_INCLUDE(rng)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/**********************************************************************************/

static GFXDECODE_START( gfx_rungun )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_msb, 0x0000, 64 )
GFXDECODE_END


void rungun_state::machine_start()
{
	uint8_t *ROM = memregion("soundcpu")->base();

	m_roz_rom = memregion("gfx1")->base();
	m_bank2->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	m_banked_ram = make_unique_clear<uint16_t[]>(0x2000);
	m_pal_ram = make_unique_clear<uint16_t[]>(0x800);
	m_spriteram_bank->configure_entries(0, 2, &m_banked_ram[0], 0x2000);

	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_nmi_clk));
}

void rungun_state::machine_reset()
{
	memset(m_sysreg, 0, 0x20);
	m_sound_ctrl = 0;
}

void rungun_state::rng(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rungun_state::rungun_map);
	m_maincpu->set_vblank_int("screen", FUNC(rungun_state::rng_interrupt));

	Z80(config, m_soundcpu, 8000000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &rungun_state::rungun_sound_map);

	config.set_maximum_quantum(attotime::from_hz(6000)); // higher if sound stutters

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rungun);

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_refresh_hz(59.185606);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(88, 88+416-1, 24, 24+224-1);
	m_screen->set_screen_update(FUNC(rungun_state::screen_update_rng));
	m_screen->set_palette(m_palette);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	K053936(config, m_k053936, 0);
	m_k053936->set_offsets(34, 9);

	K055673(config, m_k055673, 0);
	m_k055673->set_sprite_callback(FUNC(rungun_state::sprite_callback));
	m_k055673->set_config(K055673_LAYOUT_RNG, -8, 15);
	m_k055673->set_palette(m_palette);
	m_k055673->set_screen(m_screen);

	K053252(config, m_k053252, 16000000/2);
	m_k053252->set_offsets(9*8, 24);
	m_k053252->set_screen("screen");

	PALETTE(config, m_palette2).set_format(palette_device::xBGR_555, 1024);
	m_palette2->enable_shadows();
	m_palette2->enable_hilights();

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	K054321(config, m_k054321, "speaker");

	// SFX
	K054539(config, m_k054539[0], 18.432_MHz_XTAL);
	m_k054539[0]->set_device_rom_tag("k054539");
	m_k054539[0]->timer_handler().set(FUNC(rungun_state::k054539_nmi_gen));
	m_k054539[0]->add_route(0, "speaker", 1.0, 1);
	m_k054539[0]->add_route(1, "speaker", 1.0, 0);

	// BGM, volumes handtuned to make SFXs audible (still not 100% right tho)
	K054539(config, m_k054539[1], 18.432_MHz_XTAL);
	m_k054539[1]->set_device_rom_tag("k054539");
	m_k054539[1]->add_route(0, "speaker", 0.6, 0);
	m_k054539[1]->add_route(1, "speaker", 0.6, 1);
}

// for dual-screen output Run and Gun requires the video de-multiplexer board connected to the Jamma output, this gives you 2 Jamma connectors, one for each screen.
// this means when operated as a single dedicated cabinet the game runs at 60fps, and has smoother animations than when operated as a twin setup where each
// screen only gets an update every other frame.
void rungun_state::rng_dual(machine_config &config)
{
	rng(config);

	m_screen->set_screen_update(FUNC(rungun_state::screen_update_rng_dual_left));

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER));
	screen2.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen2.set_refresh_hz(59.185606);
	screen2.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen2.set_size(64*8, 32*8);
	screen2.set_visarea(88, 88+416-1, 24, 24+224-1);
	screen2.set_screen_update(FUNC(rungun_state::screen_update_rng_dual_right));
	screen2.set_palette(m_palette2);

	m_k053252->set_slave_screen("screen2");
}


// Older non-US 53936/A13 roms were all returning bad from the mask ROM check. Using the US ROM on non-US reports good therefore I guess that data matches for that
// across all sets.

ROM_START( rungun )
	/* main program Europe Version AA  1993, 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247eaa03.bin", 0x000000, 0x80000, CRC(f5c91ec0) SHA1(298926ea30472fa8d2c0578dfeaf9a93509747ef) )
	ROM_LOAD16_BYTE( "247eaa04.bin", 0x000001, 0x80000, CRC(0e62471f) SHA1(2861b7a4e78ff371358d318a1b13a6488c0ac364) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05",  0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungun.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END

ROM_START( rungund ) // same as above set, but with demux adapter connected
	/* main program Europe Version AA  1993, 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247eaa03.bin", 0x000000, 0x80000, CRC(f5c91ec0) SHA1(298926ea30472fa8d2c0578dfeaf9a93509747ef) )
	ROM_LOAD16_BYTE( "247eaa04.bin", 0x000001, 0x80000, CRC(0e62471f) SHA1(2861b7a4e78ff371358d318a1b13a6488c0ac364) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05",  0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungun.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END

ROM_START( runguna )
	/* main program Europe Version AA 1993, 10.4 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247eaa03.rom", 0x000000, 0x80000, CRC(fec3e1d6) SHA1(cd89dc32ad06308134d277f343a7e8b5fe381f69) )
	ROM_LOAD16_BYTE( "247eaa04.rom", 0x000001, 0x80000, CRC(1b556af9) SHA1(c8351ebd595307d561d089c66cd6ed7f6111d996) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("1.13g",  0x000000, 0x20000, CRC(c0b35df9) SHA1(a0c73d993eb32bd0cd192351b5f86794efd91949) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "runguna.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END


ROM_START( rungunad ) // same as above set, but with demux adapter connected
	/* main program Europe Version AA 1993, 10.4 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247eaa03.rom", 0x000000, 0x80000, CRC(fec3e1d6) SHA1(cd89dc32ad06308134d277f343a7e8b5fe381f69) )
	ROM_LOAD16_BYTE( "247eaa04.rom", 0x000001, 0x80000, CRC(1b556af9) SHA1(c8351ebd595307d561d089c66cd6ed7f6111d996) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("1.13g",  0x000000, 0x20000, CRC(c0b35df9) SHA1(a0c73d993eb32bd0cd192351b5f86794efd91949) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "runguna.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END

// This set fails the rom checks on 18n,16n and 21n even on real hardware but is clearly a different code revision to the above sets.
// The rom at 21N is the same between all sets so it failing makes very little sense.
// The date code places this at month before the other EAA sets, so maybe it's a prototype and the checksums in the ROM hadn't
// been finalized yet.

ROM_START( rungunb )
	/* main program Europe Version AA 1993, 9.10 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "4.18n", 0x000000, 0x80000, CRC(d6515edb) SHA1(4c30c5df231945027a7d3c54e250b0a246ae3b17))
	ROM_LOAD16_BYTE( "5.16n", 0x000001, 0x80000, CRC(f2f03eec) SHA1(081fd43b83e148694d34349b826bd02e0a1f85c9))

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("1.13g",  0x000000, 0x20000, CRC(c0b35df9) SHA1(a0c73d993eb32bd0cd192351b5f86794efd91949) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "runguna.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END


ROM_START( rungunbd ) // same as above set, but with demux adapter connected
	/* main program Europe Version AA 1993, 9.10 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "4.18n", 0x000000, 0x80000, CRC(d6515edb) SHA1(4c30c5df231945027a7d3c54e250b0a246ae3b17))
	ROM_LOAD16_BYTE( "5.16n", 0x000001, 0x80000, CRC(f2f03eec) SHA1(081fd43b83e148694d34349b826bd02e0a1f85c9))

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("1.13g",  0x000000, 0x20000, CRC(c0b35df9) SHA1(a0c73d993eb32bd0cd192351b5f86794efd91949) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "runguna.nv", 0x0000, 0x080, CRC(7bbf0e3c) SHA1(0fd3c9400e9b97a06517e0c8620f773a383100fd) )
ROM_END

ROM_START( rungunuba )
	/* main program US Version BA 1993 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247uba03.bin", 0x000000, 0x80000, CRC(c24d7500) SHA1(38e6ae9fc00bf8f85549be4733992336c46fe1f3) )
	ROM_LOAD16_BYTE( "247uba04.bin", 0x000001, 0x80000, CRC(3f255a4a) SHA1(3a4d50ecec8546933ad8dabe21682ba0951eaad0) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05", 0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(        0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungunua.nv", 0x0000, 0x080, CRC(9890d304) SHA1(c94a77d1d45e372350456cf8eaa7e7ebd3cdbb84) )
ROM_END


ROM_START( rungunubad )  // same as above set, but with demux adapter connected
	/* main program US Version BA 1993 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247uba03.bin", 0x000000, 0x80000, CRC(c24d7500) SHA1(38e6ae9fc00bf8f85549be4733992336c46fe1f3) )
	ROM_LOAD16_BYTE( "247uba04.bin", 0x000001, 0x80000, CRC(3f255a4a) SHA1(3a4d50ecec8546933ad8dabe21682ba0951eaad0) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05", 0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(        0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungunua.nv", 0x0000, 0x080, CRC(9890d304) SHA1(c94a77d1d45e372350456cf8eaa7e7ebd3cdbb84) )
ROM_END

ROM_START( slmdunkj )
	/* main program Japan Version AA 1993 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247jaa03.bin", 0x000000, 0x20000, CRC(87572078) SHA1(cfa784eb40ed8b3bda9d57abb6022bbe92056206) )
	ROM_LOAD16_BYTE( "247jaa04.bin", 0x000001, 0x20000, CRC(aa105e00) SHA1(617ac14535048b6e0da43cc98c4b67c8e306bef1) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05",  0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "slmdunkj.nv", 0x0000, 0x080, CRC(531d27bd) SHA1(42251272691c66c1f89f99e6e5e2f300c1a7d69d) )
ROM_END

ROM_START( slmdunkjd ) // same as above set, but with demux adapter connected
	/* main program Japan Version AA 1993 10.8 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247jaa03.bin", 0x000000, 0x20000, CRC(87572078) SHA1(cfa784eb40ed8b3bda9d57abb6022bbe92056206) )
	ROM_LOAD16_BYTE( "247jaa04.bin", 0x000001, 0x20000, CRC(aa105e00) SHA1(617ac14535048b6e0da43cc98c4b67c8e306bef1) )

	/* data (Guru 1 megabyte redump) */
	ROM_LOAD16_BYTE( "247b01.23n", 0x200000, 0x80000, CRC(2d774f27) SHA1(c48de9cb9daba25603b8278e672f269807aa0b20) )
	ROM_CONTINUE(                  0x100000, 0x80000)
	ROM_LOAD16_BYTE( "247b02.21n", 0x200001, 0x80000, CRC(d088c9de) SHA1(19d7ad4120f7cfed9cae862bb0c799fdad7ab15c) )
	ROM_CONTINUE(                  0x100001, 0x80000)

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05",  0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(         0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	//ROM_LOAD( "247-a13", 0x000000, 0x200000, BAD_DUMP CRC(cc194089) SHA1(b5af94f5f583d282ac1499b371bbaac8b2fedc03) )
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "slmdunkj.nv", 0x0000, 0x080, CRC(531d27bd) SHA1(42251272691c66c1f89f99e6e5e2f300c1a7d69d) )
ROM_END



ROM_START( rungunuabd ) // dual cabinet setup ONLY
	/* main program US Version AB 1993 10.12 */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247uab03.bin", 0x000000, 0x80000, CRC(f259fd11) SHA1(60381a3fa7f78022dcb3e2f3d13ea32a10e4e36e) )
	ROM_LOAD16_BYTE( "247uab04.bin", 0x000001, 0x80000, CRC(b918cf5a) SHA1(4314c611ef600ec081f409c78218de1639f8b463) )

	/* data */
	ROM_LOAD16_BYTE( "247a01", 0x100000, 0x80000, CRC(8341cf7d) SHA1(372c147c4a5d54aed2a16b0ed258247e65dda563) )
	ROM_LOAD16_BYTE( "247a02", 0x100001, 0x80000, CRC(f5ef3f45) SHA1(2e1d8f672c130dbfac4365dc1301b47beee10161) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05", 0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(        0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungunu.nv", 0x0000, 0x080, CRC(d501f579) SHA1(9e01d9a6a8cdc782dd2a92fbf2295e8df732f892) )
ROM_END


ROM_START( rungunuaad ) // dual cabinet setup ONLY
	/* main program US Version AB 1993  9.10 (note program ROMs have UA A labels, but it shows VER.UAB on-screen) */
	ROM_REGION( 0x300000, "maincpu", 0)
	ROM_LOAD16_BYTE( "247uaa03.bin", 0x000000, 0x80000, CRC(a05f4cd0) SHA1(1ec8941293a173c659b8503837617ce098390ccd) )
	ROM_LOAD16_BYTE( "247uaa04.bin", 0x000001, 0x80000, CRC(ebb11bef) SHA1(587c97659fa59c3895886a7b98cd9c91b21f0ed4) )

	/* data */
	ROM_LOAD16_BYTE( "247a01", 0x100000, 0x80000, CRC(8341cf7d) SHA1(372c147c4a5d54aed2a16b0ed258247e65dda563) )
	ROM_LOAD16_BYTE( "247a02", 0x100001, 0x80000, CRC(f5ef3f45) SHA1(2e1d8f672c130dbfac4365dc1301b47beee10161) )

	/* sound program */
	ROM_REGION( 0x030000, "soundcpu", 0 )
	ROM_LOAD("247a05", 0x000000, 0x20000, CRC(64e85430) SHA1(542919c3be257c8f118fc21d3835d7b6426a22ed) )
	ROM_RELOAD(        0x010000, 0x20000 )

	/* '936 tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "247a13", 0x000000, 0x200000, CRC(c5a8ef29) SHA1(23938b8093bc0b9eef91f6d38127ca7acbdc06a6) )

	/* sprites */
	ROM_REGION( 0x800000, "k055673", 0)
	ROM_LOAD64_WORD( "247-a11", 0x000000, 0x200000, CRC(c3f60854) SHA1(cbee7178ab9e5aa6a5aeed0511e370e29001fb01) )  // 5y
	ROM_LOAD64_WORD( "247-a08", 0x000002, 0x200000, CRC(3e315eef) SHA1(898bc4d5ad244e5f91cbc87820b5d0be99ef6662) )  // 2u
	ROM_LOAD64_WORD( "247-a09", 0x000004, 0x200000, CRC(5ca7bc06) SHA1(83c793c68227399f93bd1ed167dc9ed2aaac4167) )  // 2y
	ROM_LOAD64_WORD( "247-a10", 0x000006, 0x200000, CRC(a5ccd243) SHA1(860b88ade1a69f8b6c5b8206424814b386343571) )  // 5u

	/* TTL text plane ("fix layer") */
	ROM_REGION( 0x20000, "gfx3", 0)
	ROM_LOAD( "247-a12", 0x000000, 0x20000, CRC(57a8d26e) SHA1(0431d10b76d77c26a1f6f2b55d9dbcfa959e1cd0) )

	/* sound data */
	ROM_REGION( 0x400000, "k054539", 0)
	ROM_LOAD( "247-a06", 0x000000, 0x200000, CRC(b8b2a67e) SHA1(a873d32f4b178c714743664fa53c0dca29cb3ce4) )
	ROM_LOAD( "247-a07", 0x200000, 0x200000, CRC(0108142d) SHA1(4dc6a36d976dad9c0da5a5b1f01f2eb3b369c99d) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "rungunu.nv", 0x0000, 0x080, CRC(d501f579) SHA1(9e01d9a6a8cdc782dd2a92fbf2295e8df732f892) )
ROM_END

} // anonymous namespace


// these sets operate as single screen / dual screen depending on if you have the video de-multiplexer plugged in, and the dipswitch set to 1 or 2 monitors

// the 2nd letter of the code indicates the cabinet type, this is why the selectable (single/dual) screen version of Run and Gun for the USA is 'UBA' because the first release there 'UAA' was dual screen only.
// it appears that all other regions were switchable from the first release, so use the 'A' code.

// these are running WITHOUT the demux adapter, on a single screen
GAME( 1993, rungun,   0,      rng, rng, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 10.8)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1993, runguna,  rungun, rng, rng, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 10.4)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1993, rungunb,  rungun, rng, rng, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 9.10, prototype?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1993, rungunuba,rungun, rng, rng, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver UBA 1993 10.8)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1993, slmdunkj, rungun, rng, rng, rungun_state, empty_init, ROT0, "Konami", "Slam Dunk (ver JAA 1993 10.8)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )

// these sets have the demux adapter connected, and output to 2 screens (as the adapter represents a physical hardware difference, albeit a minor one, use clone sets)
GAMEL( 1993, rungund,   rungun, rng_dual, rng_dual, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 10.8) (dual screen with demux adapter)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
GAMEL( 1993, rungunad,  rungun, rng_dual, rng_dual, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 10.4) (dual screen with demux adapter)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
GAMEL( 1993, rungunbd,  rungun, rng_dual, rng_dual, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver EAA 1993 9.10, prototype?) (dual screen with demux adapter)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
GAMEL( 1993, rungunubad,rungun, rng_dual, rng_dual, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver UBA 1993 10.8) (dual screen with demux adapter)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
GAMEL( 1993, slmdunkjd, rungun, rng_dual, rng_dual, rungun_state, empty_init, ROT0, "Konami", "Slam Dunk (ver JAA 1993 10.8) (dual screen with demux adapter)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )

// these sets have no DIP switches to select single screen mode (they're not even displayed in test menu) they're twin cabinet ONLY
GAMEL( 1993, rungunuabd,rungun, rng_dual, rng_nodip, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver UAB 1993 10.12, dedicated twin cabinet)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
GAMEL( 1993, rungunuaad,rungun, rng_dual, rng_nodip, rungun_state, empty_init, ROT0, "Konami", "Run and Gun (ver UAB 1993  9.10, dedicated twin cabinet)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_rungun_dual )
