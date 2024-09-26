// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Spy Hunter(Tecfri bootleg)
single PCB with 2x Z80

significant changes compared to original HW

Very different hardware, probably bootleg despite the license text printed on the PCB, similar to '1942p' and 'spartanxtec.cpp'
PCB made by Tecfri for Recreativos Franco S.A. in Spain, has Bally Midway logo, and licensing text on the PCB.
Board is dated '85' so seems to be a low-cost rebuild? it is unclear if it made it to market.

non-interlaced

sound system appears to be the same as 'spartanxtec.cpp'

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "spyhunttec.lh"


namespace {

class spyhuntertec_state : public driver_device
{
public:
	spyhuntertec_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_analog_timer(*this, "analog_timer"),
		m_analog_input(*this, "AN.%u", 0),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_paletteram(*this, "paletteram"),
		m_spyhunt_alpharam(*this, "spyhunt_alpha"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void spyhuntertec(machine_config &config);

	void init_spyhuntertec();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<timer_device> m_analog_timer;
	required_ioport_array<2> m_analog_input;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_spyhunt_alpharam;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t m_spyhunt_sprite_color_mask = 0;
	int16_t m_spyhunt_scroll_offset = 0;
	int16_t m_spyhunt_scrollx = 0;
	int16_t m_spyhunt_scrolly = 0;

	int mcr_cocktail_flip = 0;

	tilemap_t *m_alpha_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	void spyhuntertec_paletteram_w(offs_t offset, uint8_t data);

//  uint32_t screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void spyhuntertec_port04_w(uint8_t data);
	void spyhuntertec_portf0_w(uint8_t data);

	void spyhunt_videoram_w(offs_t offset, uint8_t data);
	void spyhunt_alpharam_w(offs_t offset, uint8_t data);
	void spyhunt_scroll_value_w(offs_t offset, uint8_t data);
	void sound_irq_ack(uint8_t data);


	void ay1_porta_w(uint8_t data);
	uint8_t ay1_porta_r();

	void ay2_porta_w(uint8_t data);
	uint8_t ay2_porta_r();

	uint8_t spyhuntertec_in2_r();
	uint8_t spyhuntertec_in3_r();

	TILEMAP_MAPPER_MEMBER(spyhunt_bg_scan);
	TILE_GET_INFO_MEMBER(spyhunt_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(spyhunt_get_alpha_tile_info);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);

	TIMER_DEVICE_CALLBACK_MEMBER(analog_count_callback);
	void reset_analog_timer();

	uint8_t m_analog_select = 0;
	uint8_t m_analog_count = 0;
	void spyhuntertec_map(address_map &map) ATTR_COLD;
	void spyhuntertec_portmap(address_map &map) ATTR_COLD;
	void spyhuntertec_sound_map(address_map &map) ATTR_COLD;
	void spyhuntertec_sound_portmap(address_map &map) ATTR_COLD;
};

void spyhuntertec_state::ay1_porta_w(uint8_t data)
{
//  printf("ay1_porta_w %02x\n", data);
}

uint8_t spyhuntertec_state::ay1_porta_r()
{
//  printf("ay1_porta_r\n");
	return 0;
}

void spyhuntertec_state::reset_analog_timer()
{
	// 555 timer, period is guessed
	m_analog_timer->adjust(attotime::from_nsec(9400));
}

TIMER_DEVICE_CALLBACK_MEMBER(spyhuntertec_state::analog_count_callback)
{
	if (m_analog_count != 0)
		m_analog_count--;
	reset_analog_timer();
}

void spyhuntertec_state::ay2_porta_w(uint8_t data)
{
	// d7: latch analog counter on falling edge, d0 selects which one
	if (~data & m_analog_select & 0x80)
	{
		reset_analog_timer();
		m_analog_count = m_analog_input[data & 1]->read();
	}

	m_analog_select = data;
}

uint8_t spyhuntertec_state::ay2_porta_r()
{
// read often, even if port is set to output mode
// maybe latches something?
//  printf("ay2_porta_r\n");
	return 0x00; // not sure value matters
}

void spyhuntertec_state::spyhunt_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void spyhuntertec_state::spyhunt_alpharam_w(offs_t offset, uint8_t data)
{
	m_spyhunt_alpharam[offset] = data;
	m_alpha_tilemap->mark_tile_dirty(offset);
}


void spyhuntertec_state::spyhunt_scroll_value_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			/* low 8 bits of horizontal scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & ~0xff) | data;
			break;

		case 1:
			/* upper 3 bits of horizontal scroll and upper 1 bit of vertical scroll */
			m_spyhunt_scrollx = (m_spyhunt_scrollx & 0xff) | ((data & 0x07) << 8);
			m_spyhunt_scrolly = (m_spyhunt_scrolly & 0xff) | ((data & 0x80) << 1);
			break;

		case 2:
			/* low 8 bits of vertical scroll */
			m_spyhunt_scrolly = (m_spyhunt_scrolly & ~0xff) | data;
			break;
	}
}


void spyhuntertec_state::spyhuntertec_paletteram_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;
	offset = (offset & 0x0f) | (offset & 0x60) >> 1;

	int r = (data & 0x07) >> 0;
	int g = (data & 0x38) >> 3;
	int b = (data & 0xc0) >> 6;

	m_palette->set_pen_color(offset^0xf, rgb_t(r<<5,g<<5,b<<6));
}


TILEMAP_MAPPER_MEMBER(spyhuntertec_state::spyhunt_bg_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) | ((col & 0x3f) << 4) | ((row & 0x10) << 6);
}


TILE_GET_INFO_MEMBER(spyhuntertec_state::spyhunt_get_bg_tile_info)
{
	int data = m_videoram[tile_index];
	int code = (data & 0x3f) | ((data >> 1) & 0x40);
	tileinfo.set(0, code, 0, (data & 0x40) ? TILE_FLIPY : 0);
}


TILE_GET_INFO_MEMBER(spyhuntertec_state::spyhunt_get_alpha_tile_info)
{
	tileinfo.set(2, m_spyhunt_alpharam[tile_index], 0, 0);
}



void spyhuntertec_state::video_start()
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spyhuntertec_state::spyhunt_get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(spyhuntertec_state::spyhunt_bg_scan)),  64,16, 64,32);

	/* initialize the text tilemap */
	m_alpha_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spyhuntertec_state::spyhunt_get_alpha_tile_info)), TILEMAP_SCAN_COLS,  16,8, 32,32);
	m_alpha_tilemap->set_transparent_pen(0);
	m_alpha_tilemap->set_scrollx(0, 16);

	save_item(NAME(m_spyhunt_sprite_color_mask));
	save_item(NAME(m_spyhunt_scrollx));
	save_item(NAME(m_spyhunt_scrolly));
	save_item(NAME(m_spyhunt_scroll_offset));

	mcr_cocktail_flip = 0; // TODO: this doesn't get set anywhere, code at line 322 is effectively unreachable
}




void spyhuntertec_state::mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced)
{
	m_screen->priority().fill(1, cliprect);

	/* loop over sprite RAM */
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, sx, sy, flags;

		/* skip if zero */
		if (m_spriteram[offs] == 0)
			continue;

/*
    monoboard:
        flags.d0 -> ICG0~ -> PCG0~/PCG2~/PCG4~/PCG6~ -> bit 4 of linebuffer
        flags.d1 -> ICG1~ -> PCG1~/PCG3~/PCG5~/PCG7~ -> bit 5 of linebuffer
        flags.d2 -> IPPR  -> PPR0 /PPR1 /PPR2 /PPR3  -> bit 6 of linebuffer
        flags.d3 -> IRA15 ----------------------------> address line 15 of FG ROMs
        flags.d4 -> HFLIP
        flags.d5 -> VFLIP

*/

		/* extract the bits of information */
		flags = m_spriteram[offs + 1];
		code = m_spriteram[offs + 2] + 256 * ((flags >> 3) & 0x01);
		color = ~flags & color_mask;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		sx = (m_spriteram[offs + 3] - 3) * 2;
		sy = (241 - m_spriteram[offs]);

		if (interlaced == 1) sy *= 2;

		code ^= code_xor;

		sx += dx;
		sy += dy;

		/* sprites use color 0 for background pen and 8 for the 'under tile' pen.
		    The color 8 is used to cover over other sprites. */
		if (!mcr_cocktail_flip)
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
					screen.priority(), 0x02, 0xfeff);
		}
		else
		{
			/* first draw the sprite, visible */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x00, 0x0101);

			/* then draw the mask, behind the background but obscuring following sprites */
			m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, !flipx, !flipy, 480 - sx, 452 - sy,
					screen.priority(), 0x02, 0xfeff);
		}
	}
}


uint32_t spyhuntertec_state::screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	m_bg_tilemap->set_scrollx(0, m_spyhunt_scrollx * 2 + m_spyhunt_scroll_offset);
	m_bg_tilemap->set_scrolly(0, m_spyhunt_scrolly * 2);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */
	mcr3_update_sprites(screen, bitmap, cliprect, m_spyhunt_sprite_color_mask, 0, -12, 0, 0);

	/* render any characters on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



uint8_t spyhuntertec_state::spyhuntertec_in2_r()
{
	// it writes 04 / 14 to the sound latch (at FD00) before
	// reading bit 6 here a minimum of 32 times.
	// seems to be how it reads the analog controls? probably via sound CPU??

	/* note, these commands trigger a read from ay2_porta on the sound cpu side, followed by 2 writes

	a52a spyhuntertec_fd00_w 14
	ay2_porta_r
	ay2_porta_w 80
	ay2_porta_w 00
	a52a spyhuntertec_fd00_w 04
	ay2_porta_r
	ay2_porta_w 81
	ay2_porta_w 01
	*/

	/*

	    -- input reading code here
	    A388: 3E 14         ld   a,$14
	    A38A: 28 04         jr   z,$A390
	    A38C: DD 23         inc  ix
	    A38E: 3E 04         ld   a,$04
	    A390: CD 20 A5      call $A520 << write command to sub-cpu

	    -- delay loop / timeout loop for reading result? value of b doesn't get used in the end
	    A393: 06 1F         ld   b,$1F << loop counter
	    A395: 21 02 FC      ld   hl,$FC02
	    loopstart:
	    A398: CB 76         bit  6,(hl)
	    A39A: 28 06         jr   z,$A3A2 to dest2
	    A39C: 10 FA         djnz $A398 (to loopstart)

	    A39E: 06 0F         ld   b,$0F <
	    A3A0: 18 1E         jr   $A3C0  to dest 3

	    dest2:
	    A3A2: 06 33         ld   b,$33  << loop counter
	    loop2start:
	    A3A4: CB 76         bit  6,(hl)
	    A3A6: 20 11         jr   nz,$A3B9 (to outofloop)
	    A3A8: CB 76         bit  6,(hl)
	    A3AA: 00            nop
	    A3AB: CB 76         bit  6,(hl)
	    A3AD: 20 0A         jr   nz,$A3B9 (to outofloop)
	    A3AF: 10 F3         djnz $A3A4 (to loop2start)

	    A3B1: 00            nop
	    A3B2: 00            nop
	    A3B3: 00            nop
	    A3B4: 00            nop
	    A3B5: 00            nop
	    A3B6: 00            nop
	    A3B7: 00            nop
	    A3B8: 00            nop

	    outofloop:
	    A3B9: 78            ld   a,b
	    A3BA: FE 20         cp   $20
	    A3BC: 38 02         jr   c,$A3C0
	    A3BE: 06 1F         ld   b,$1F

	    dest3:
	    A3C0: 21 B6 A6      ld   hl,$A6B6
	    ...


	*/
//  printf("%04x spyhuntertec_in2_r\n", m_maincpu->pc());

	return (ioport("IN2")->read() & ~0x40) | ((m_analog_count == 0) ? 0x40 : 0x00);
}

uint8_t spyhuntertec_state::spyhuntertec_in3_r()
{
	uint8_t ret = ioport("IN3")->read();
//  printf("%04x spyhuntertec_in3_r\n",m_maincpu->pc());
	return ret;
}

void spyhuntertec_state::spyhuntertec_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).rom();

	map(0xa800, 0xa8ff).ram(); // the ROM is a solid fill in these areas, and they get tested as RAM, I think they moved the 'real' scroll regs here
	map(0xa900, 0xa9ff).ram();

	map(0xe000, 0xe7ff).ram().w(FUNC(spyhuntertec_state::spyhunt_videoram_w)).share("videoram");
	map(0xe800, 0xebff).mirror(0x0400).ram().w(FUNC(spyhuntertec_state::spyhunt_alpharam_w)).share("spyhunt_alpha");
	map(0xf000, 0xf7ff).ram(); //.share("nvram");
	map(0xf800, 0xf9ff).ram().share("spriteram"); // origional spriteram
	map(0xfa00, 0xfa7f).mirror(0x0180).ram().w(FUNC(spyhuntertec_state::spyhuntertec_paletteram_w)).share("paletteram");

	map(0xfc00, 0xfc00).portr("DSW0");
	map(0xfc01, 0xfc01).portr("DSW1");
	map(0xfc02, 0xfc02).r(FUNC(spyhuntertec_state::spyhuntertec_in2_r));
	map(0xfc03, 0xfc03).r(FUNC(spyhuntertec_state::spyhuntertec_in3_r));

	map(0xfd00, 0xfd00).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0xfe00, 0xffff).ram().share("spriteram2"); // actual spriteram for this hw??
}

void spyhuntertec_state::spyhuntertec_port04_w(uint8_t data)
{
}

void spyhuntertec_state::spyhuntertec_portf0_w(uint8_t data)
{
	// 0x08 on startup, then 0x03, probably CTC leftovers from the original.
	if ((data != 0x03) && (data != 0x08)) printf("spyhuntertec_portf0_w %02x\n", data);
}

void spyhuntertec_state::spyhuntertec_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x04).w(FUNC(spyhuntertec_state::spyhuntertec_port04_w));
	map(0x84, 0x86).w(FUNC(spyhuntertec_state::spyhunt_scroll_value_w));
	map(0xe0, 0xe0).nopw(); // was watchdog
//  map(0xe8, 0xe8).nopw();
	map(0xf0, 0xf0).w(FUNC(spyhuntertec_state::spyhuntertec_portf0_w));
}


void spyhuntertec_state::spyhuntertec_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x83ff).ram();

	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


void spyhuntertec_state::sound_irq_ack(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

void spyhuntertec_state::spyhuntertec_sound_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x00).w(FUNC(spyhuntertec_state::sound_irq_ack));

	map(0x0012, 0x0013).w("ay3", FUNC(ay8912_device::address_data_w));
	map(0x0012, 0x0012).r("ay3", FUNC(ay8912_device::data_r));

	map(0x0014, 0x0015).w("ay1", FUNC(ay8912_device::address_data_w));
	map(0x0014, 0x0014).r("ay1", FUNC(ay8912_device::data_r));

	map(0x0018, 0x0019).w("ay2", FUNC(ay8912_device::address_data_w)); // data written to port a
	map(0x0018, 0x0018).r("ay2", FUNC(ay8912_device::data_r)); // actually read

}



static INPUT_PORTS_START( spyhuntertec )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSW0-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW0-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW0-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW0-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW0-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW0-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW0-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW0-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Right Button / Smoke Screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  ) PORT_NAME("Center Button / Weapons Van")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Left Trigger / Missiles")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Left Button / Oil Slick")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Gear Shift") PORT_TOGGLE
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) // analog signal
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0001, 0x0001, "3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Right Trigger / Machine Guns")

	PORT_START("AN.0")
	PORT_BIT( 0xff, 0x02, IPT_PEDAL ) PORT_MINMAX(0x02,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("AN.1")
	PORT_BIT( 0x7f, 0x40, IPT_PADDLE ) PORT_MINMAX(0x30,0x50) PORT_SENSITIVITY(40) PORT_KEYDELTA(3) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout spyhuntertec_alphalayout =
{
	16,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4},
	{ 0, 0, 1, 1, 2, 2, 3, 3, 8, 8, 9, 9, 10, 10, 11, 11 },
	{ 0, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


const gfx_layout spyhuntertec_sprite_layout =
{
	32,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 6,7,  4,5,  2,3,  0,1,  14,15,  12,13,  10,11,  8,9,    22,23, 20,21,  18,19,  16,17,  30,31,  28,29,  26,27,  24,25 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32   },

	16*32
};


static const uint32_t spyhuntp_charlayout_xoffset[64] =
{
		0x0000*8,0x0000*8,   0x0000*8+1,0x0000*8+1,   0x0000*8+2,0x0000*8+2,   0x0000*8+3,0x0000*8+3,   0x0000*8+4,0x0000*8+4,   0x0000*8+5,0x0000*8+5,   0x0000*8+6,0x0000*8+6,   0x0000*8+7,0x0000*8+7,
		0x1000*8,0x1000*8,   0x1000*8+1,0x1000*8+1,   0x1000*8+2,0x1000*8+2,   0x1000*8+3,0x1000*8+3,   0x1000*8+4,0x1000*8+4,   0x1000*8+5,0x1000*8+5,   0x1000*8+6,0x1000*8+6,   0x1000*8+7,0x1000*8+7,
		0x2000*8,0x2000*8,   0x2000*8+1,0x2000*8+1,   0x2000*8+2,0x2000*8+2,   0x2000*8+3,0x2000*8+3,   0x2000*8+4,0x2000*8+4,   0x2000*8+5,0x2000*8+5,   0x2000*8+6,0x2000*8+6,   0x2000*8+7,0x2000*8+7,
		0x3000*8,0x3000*8,   0x3000*8+1,0x3000*8+1,   0x3000*8+2,0x3000*8+2,   0x3000*8+3,0x3000*8+3,   0x3000*8+4,0x3000*8+4,   0x3000*8+5,0x3000*8+5,   0x3000*8+6,0x3000*8+6,   0x3000*8+7,0x3000*8+7,
};


static const gfx_layout spyhuntertec_charlayout =
{
	64,16,
	RGN_FRAC(1,8),
	4,
	{  0*8,  0x4000*8 + 2*8, 0x4000*8 + 0*8, 2*8  },
	EXTENDED_XOFFS,
	{ 0*8,  4*8,  8*8,  12*8,    16*8,  20*8,  24*8,  28*8,     1*8,  5*8, 9*8, 13*8,    17*8,  21*8,  25*8,  29*8    },
	32*8,
	spyhuntp_charlayout_xoffset,
	nullptr
};


static GFXDECODE_START( gfx_spyhuntertec )
	GFXDECODE_ENTRY( "gfx1", 0, spyhuntertec_charlayout,  3*16, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spyhuntertec_sprite_layout,   0*16, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, spyhuntertec_alphalayout, 4*16, 1 )
GFXDECODE_END



void spyhuntertec_state::machine_start()
{
}

void spyhuntertec_state::machine_reset()
{
}




void spyhuntertec_state::spyhuntertec(machine_config &config)
{
// note: no ctc, no nvram
// 2*z80, 3*ay8912
// 2 XTALs: one 20MHz, other one near maincpu ?MHz

	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000); // NEC D780C-2 (rated 6MHz)
	m_maincpu->set_addrmap(AS_PROGRAM, &spyhuntertec_state::spyhuntertec_map);
	m_maincpu->set_addrmap(AS_IO, &spyhuntertec_state::spyhuntertec_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(spyhuntertec_state::irq0_line_hold));
	TIMER(config, m_analog_timer).configure_generic(FUNC(spyhuntertec_state::analog_count_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(30*16, 30*8);
	m_screen->set_visarea(0, 30*16-1, 0, 30*8-1);
	m_screen->set_screen_update(FUNC(spyhuntertec_state::screen_update_spyhuntertec));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spyhuntertec);
	PALETTE(config, m_palette).set_entries(64+4); // FUNC(spyhuntertec_state::spyhunt)

	Z80(config, m_audiocpu, 4000000); // SGS Z8400B1 (rated 2.5MHz?)
	m_audiocpu->set_addrmap(AS_PROGRAM, &spyhuntertec_state::spyhuntertec_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &spyhuntertec_state::spyhuntertec_sound_portmap);
	m_audiocpu->set_periodic_int(FUNC(spyhuntertec_state::irq0_line_assert), attotime::from_hz(1000));

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ay8912_device &ay1(AY8912(config, "ay1", 3000000/2)); // AY-3-8912
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);
	ay1.port_a_read_callback().set(FUNC(spyhuntertec_state::ay1_porta_r));
	ay1.port_a_write_callback().set(FUNC(spyhuntertec_state::ay1_porta_w));

	ay8912_device &ay2(AY8912(config, "ay2", 3000000/2)); // "
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
	ay2.port_a_read_callback().set(FUNC(spyhuntertec_state::ay2_porta_r));
	ay2.port_a_write_callback().set(FUNC(spyhuntertec_state::ay2_porta_w));

	AY8912(config, "ay3", 3000000/2).add_route(ALL_OUTPUTS, "mono", 0.25); // "
}




/*
  Spy Hunter (Recreativos Franco)
  Licensed by Bally Midway.
  Original PCB.
*/

ROM_START( spyhuntsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x4000, CRC(ae05b8a2) SHA1(8854368206c484b30865a4d29ac85f854599e791) )
	ROM_LOAD( "2.bin",   0x4000, 0x4000, CRC(c96f5d69) SHA1(528b1ded5d4fea008482cace48c96ec162d78bae) )
	ROM_LOAD( "3.bin",   0x8000, 0x4000, CRC(bd6c1a5c) SHA1(c47fe5daccead4a3ea326c2b0b12405e24f9e222) )
	ROM_LOAD( "4.bin",   0xc000, 0x2000, CRC(3ea6a65c) SHA1(1320ce17044307ed3c4f2459631a9aa1734f1f30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x2000, CRC(33fe2829) SHA1(e6950dbf681242bf23542ca6604e62eacb431101) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "6.bin",   0x0000, 0x200, CRC(6b76f46a) SHA1(4b398084c42a60fcfa4a9bf14f844e36a3f42723) )
	ROM_CONTINUE(0x0001, 0x200)
	ROM_CONTINUE(0x0800, 0x200)
	ROM_CONTINUE(0x0801, 0x200)
	ROM_CONTINUE(0x1000, 0x200)
	ROM_CONTINUE(0x1001, 0x200)
	ROM_CONTINUE(0x1800, 0x200)
	ROM_CONTINUE(0x1801, 0x200)
	ROM_CONTINUE(0x2000, 0x200)
	ROM_CONTINUE(0x2001, 0x200)
	ROM_CONTINUE(0x2800, 0x200)
	ROM_CONTINUE(0x2801, 0x200)
	ROM_CONTINUE(0x3000, 0x200)
	ROM_CONTINUE(0x3001, 0x200)
	ROM_CONTINUE(0x3800, 0x200)
	ROM_CONTINUE(0x3801, 0x200)
	ROM_LOAD32_BYTE( "7.bin",   0x0002, 0x200, CRC(085bd7a7) SHA1(c35c309b6c6485baec54d4434dea44abf4d48f41) )
	ROM_CONTINUE(0x0003, 0x200)
	ROM_CONTINUE(0x0802, 0x200)
	ROM_CONTINUE(0x0803, 0x200)
	ROM_CONTINUE(0x1002, 0x200)
	ROM_CONTINUE(0x1003, 0x200)
	ROM_CONTINUE(0x1802, 0x200)
	ROM_CONTINUE(0x1803, 0x200)
	ROM_CONTINUE(0x2002, 0x200)
	ROM_CONTINUE(0x2003, 0x200)
	ROM_CONTINUE(0x2802, 0x200)
	ROM_CONTINUE(0x2803, 0x200)
	ROM_CONTINUE(0x3002, 0x200)
	ROM_CONTINUE(0x3003, 0x200)
	ROM_CONTINUE(0x3802, 0x200)
	ROM_CONTINUE(0x3803, 0x200)
	ROM_LOAD32_BYTE( "8.bin",   0x4000, 0x200, CRC(e699b329) SHA1(cb4b8c7b6fa1cb1144a18f1442dc3b267c408914) )
	ROM_CONTINUE(0x4001, 0x200)
	ROM_CONTINUE(0x4800, 0x200)
	ROM_CONTINUE(0x4801, 0x200)
	ROM_CONTINUE(0x5000, 0x200)
	ROM_CONTINUE(0x5001, 0x200)
	ROM_CONTINUE(0x5800, 0x200)
	ROM_CONTINUE(0x5801, 0x200)
	ROM_CONTINUE(0x6000, 0x200)
	ROM_CONTINUE(0x6001, 0x200)
	ROM_CONTINUE(0x6800, 0x200)
	ROM_CONTINUE(0x6801, 0x200)
	ROM_CONTINUE(0x7000, 0x200)
	ROM_CONTINUE(0x7001, 0x200)
	ROM_CONTINUE(0x7800, 0x200)
	ROM_CONTINUE(0x7801, 0x200)
	ROM_LOAD32_BYTE( "9.bin",   0x4002, 0x200, CRC(6d462ec7) SHA1(0ff37f75b0eeceb86177a3f7c93834d5c0e24515) )
	ROM_CONTINUE(0x4003, 0x200)
	ROM_CONTINUE(0x4802, 0x200)
	ROM_CONTINUE(0x4803, 0x200)
	ROM_CONTINUE(0x5002, 0x200)
	ROM_CONTINUE(0x5003, 0x200)
	ROM_CONTINUE(0x5802, 0x200)
	ROM_CONTINUE(0x5803, 0x200)
	ROM_CONTINUE(0x6002, 0x200)
	ROM_CONTINUE(0x6003, 0x200)
	ROM_CONTINUE(0x6802, 0x200)
	ROM_CONTINUE(0x6803, 0x200)
	ROM_CONTINUE(0x7002, 0x200)
	ROM_CONTINUE(0x7003, 0x200)
	ROM_CONTINUE(0x7802, 0x200)
	ROM_CONTINUE(0x7803, 0x200)

	ROM_REGION( 0x10000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "10.bin",   0x00000, 0x4000, CRC(6f9fd416) SHA1(a51c86e5b22c91fc44673f53400b58af40b18065) )
	ROM_LOAD( "11.bin",   0x04000, 0x4000, CRC(75526ffe) SHA1(ff1adf6f9b6595114d0bd06b72d9eb7bbf70144d) )
	ROM_LOAD( "12.bin",   0x08000, 0x4000, CRC(82ee7a4d) SHA1(184720de76680275bf7c4a171f03a0ce771d91fc) )
	ROM_LOAD( "13.bin",   0x0c000, 0x4000, CRC(0cc592a3) SHA1(b3563bde83432cdbaedb88d4d222da30bf679b08) )

	ROM_REGION( 0x01000, "gfx3", 0 )
	ROM_LOAD( "14.bin",  0x00000, 0x1000, CRC(87a4c130) SHA1(7792afdc36b0f3bd51c387d04d38f60c85fd2e93) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.ic19", 0x0000, 0x0100, CRC(b9dc4d09) SHA1(56176e5c40e6926784cfe93b2e5241c2b46f4a38) )
	ROM_LOAD( "82s129.ic20", 0x0100, 0x0100, CRC(b9dc4d09) SHA1(56176e5c40e6926784cfe93b2e5241c2b46f4a38) )

	ROM_REGION( 0x200, "proms2", 0 )
	ROM_LOAD( "prom.1c8", 0x0000, 0x0200, CRC(ce69832d) SHA1(a663cbf762232f3ed9fc0b42a559e1ca4639589b) )

ROM_END


ROM_START( spyhuntpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x0000, 0x4000, CRC(2a2f77cb) SHA1(e1b74c951efb2a49bef0507ab3268b274515f339) )
	ROM_LOAD( "2.bin",   0x4000, 0x4000, CRC(00778aff) SHA1(7c0b24c393f841e8379d4bba57ba502e3d2512f9) )
	ROM_LOAD( "3.bin",   0x8000, 0x4000, CRC(2183b4af) SHA1(2b958afc40b26c9bc8d5254b0600426649f4ebf0) )
	ROM_LOAD( "4.bin",   0xc000, 0x2000, CRC(3ea6a65c) SHA1(1320ce17044307ed3c4f2459631a9aa1734f1f30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.bin",   0x0000, 0x2000, CRC(33fe2829) SHA1(e6950dbf681242bf23542ca6604e62eacb431101) )


	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "6.bin",   0x0000, 0x200, CRC(6b76f46a) SHA1(4b398084c42a60fcfa4a9bf14f844e36a3f42723) )
	ROM_CONTINUE(0x0001, 0x200)
	ROM_CONTINUE(0x0800, 0x200)
	ROM_CONTINUE(0x0801, 0x200)
	ROM_CONTINUE(0x1000, 0x200)
	ROM_CONTINUE(0x1001, 0x200)
	ROM_CONTINUE(0x1800, 0x200)
	ROM_CONTINUE(0x1801, 0x200)
	ROM_CONTINUE(0x2000, 0x200)
	ROM_CONTINUE(0x2001, 0x200)
	ROM_CONTINUE(0x2800, 0x200)
	ROM_CONTINUE(0x2801, 0x200)
	ROM_CONTINUE(0x3000, 0x200)
	ROM_CONTINUE(0x3001, 0x200)
	ROM_CONTINUE(0x3800, 0x200)
	ROM_CONTINUE(0x3801, 0x200)
	ROM_LOAD32_BYTE( "7.bin",   0x0002, 0x200, CRC(085bd7a7) SHA1(c35c309b6c6485baec54d4434dea44abf4d48f41) )
	ROM_CONTINUE(0x0003, 0x200)
	ROM_CONTINUE(0x0802, 0x200)
	ROM_CONTINUE(0x0803, 0x200)
	ROM_CONTINUE(0x1002, 0x200)
	ROM_CONTINUE(0x1003, 0x200)
	ROM_CONTINUE(0x1802, 0x200)
	ROM_CONTINUE(0x1803, 0x200)
	ROM_CONTINUE(0x2002, 0x200)
	ROM_CONTINUE(0x2003, 0x200)
	ROM_CONTINUE(0x2802, 0x200)
	ROM_CONTINUE(0x2803, 0x200)
	ROM_CONTINUE(0x3002, 0x200)
	ROM_CONTINUE(0x3003, 0x200)
	ROM_CONTINUE(0x3802, 0x200)
	ROM_CONTINUE(0x3803, 0x200)
	ROM_LOAD32_BYTE( "8.bin",   0x4000, 0x200, CRC(e699b329) SHA1(cb4b8c7b6fa1cb1144a18f1442dc3b267c408914) )
	ROM_CONTINUE(0x4001, 0x200)
	ROM_CONTINUE(0x4800, 0x200)
	ROM_CONTINUE(0x4801, 0x200)
	ROM_CONTINUE(0x5000, 0x200)
	ROM_CONTINUE(0x5001, 0x200)
	ROM_CONTINUE(0x5800, 0x200)
	ROM_CONTINUE(0x5801, 0x200)
	ROM_CONTINUE(0x6000, 0x200)
	ROM_CONTINUE(0x6001, 0x200)
	ROM_CONTINUE(0x6800, 0x200)
	ROM_CONTINUE(0x6801, 0x200)
	ROM_CONTINUE(0x7000, 0x200)
	ROM_CONTINUE(0x7001, 0x200)
	ROM_CONTINUE(0x7800, 0x200)
	ROM_CONTINUE(0x7801, 0x200)
	ROM_LOAD32_BYTE( "9.bin",   0x4002, 0x200, CRC(6d462ec7) SHA1(0ff37f75b0eeceb86177a3f7c93834d5c0e24515) )
	ROM_CONTINUE(0x4003, 0x200)
	ROM_CONTINUE(0x4802, 0x200)
	ROM_CONTINUE(0x4803, 0x200)
	ROM_CONTINUE(0x5002, 0x200)
	ROM_CONTINUE(0x5003, 0x200)
	ROM_CONTINUE(0x5802, 0x200)
	ROM_CONTINUE(0x5803, 0x200)
	ROM_CONTINUE(0x6002, 0x200)
	ROM_CONTINUE(0x6003, 0x200)
	ROM_CONTINUE(0x6802, 0x200)
	ROM_CONTINUE(0x6803, 0x200)
	ROM_CONTINUE(0x7002, 0x200)
	ROM_CONTINUE(0x7003, 0x200)
	ROM_CONTINUE(0x7802, 0x200)
	ROM_CONTINUE(0x7803, 0x200)

	ROM_REGION( 0x10000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "10.bin",   0x00000, 0x4000, CRC(6f9fd416) SHA1(a51c86e5b22c91fc44673f53400b58af40b18065) )
	ROM_LOAD( "11.bin",   0x04000, 0x4000, CRC(75526ffe) SHA1(ff1adf6f9b6595114d0bd06b72d9eb7bbf70144d) )
	ROM_LOAD( "12.bin",   0x08000, 0x4000, CRC(82ee7a4d) SHA1(184720de76680275bf7c4a171f03a0ce771d91fc) )
	ROM_LOAD( "13.bin",   0x0c000, 0x4000, CRC(0cc592a3) SHA1(b3563bde83432cdbaedb88d4d222da30bf679b08) )


	ROM_REGION( 0x01000, "gfx3", 0 )
	ROM_LOAD( "14.bin",  0x00000, 0x1000, CRC(87a4c130) SHA1(7792afdc36b0f3bd51c387d04d38f60c85fd2e93) )
ROM_END


void spyhuntertec_state::init_spyhuntertec()
{
	m_spyhunt_sprite_color_mask = 0x00;
	m_spyhunt_scroll_offset = 16;
}

} // Anonymous namespace


/***************************************************************************
*                              Game Drivers                                *
***************************************************************************/

//    YEAR  NAME       PARENT   MACHINE       INPUT         STATE               INIT               ROT    COMPANY                                              FULLNAME                                                             FLAGS                                        LAYOUT
GAMEL(1985, spyhuntsp, spyhunt, spyhuntertec, spyhuntertec, spyhuntertec_state, init_spyhuntertec, ROT90, "Recreativos Franco S.A. (Bally Midway license)",    "Spy Hunter (Spain, Recreativos Franco S.A., Bally Midway license)", MACHINE_SUPPORTS_SAVE,                       layout_spyhunttec )
GAMEL(1985, spyhuntpr, spyhunt, spyhuntertec, spyhuntertec, spyhuntertec_state, init_spyhuntertec, ROT90, "bootleg (Recreativos Franco S.A. license, Tecfri)", "Spy Hunter (Spain, Recreativos Franco S.A., Tecfri PCB)",           MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_spyhunttec )
