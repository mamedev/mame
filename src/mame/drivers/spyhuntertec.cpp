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
#include "sound/ay8910.h"
#include "spyhunttec.lh"

class spyhuntertec_state : public driver_device
{
public:
	spyhuntertec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_analog_timer(*this, "analog_timer"),
		m_analog_input(*this, "AN"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_paletteram(*this, "paletteram"),
		m_spyhunt_alpharam(*this, "spyhunt_alpha"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<timer_device> m_analog_timer;
	required_ioport_array<2> m_analog_input;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spyhunt_alpharam;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);



	UINT8 m_spyhunt_sprite_color_mask;
	INT16 m_spyhunt_scroll_offset;
	INT16 m_spyhunt_scrollx;
	INT16 m_spyhunt_scrolly;

	int mcr_cocktail_flip;

	tilemap_t *m_alpha_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(spyhuntertec_paletteram_w);
	DECLARE_DRIVER_INIT(spyhuntertec);
//  DECLARE_VIDEO_START(spyhuntertec);
//  UINT32 screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(spyhuntertec_port04_w);
	DECLARE_WRITE8_MEMBER(spyhuntertec_fd00_w);
	DECLARE_WRITE8_MEMBER(spyhuntertec_portf0_w);

	DECLARE_WRITE8_MEMBER(spyhunt_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_alpharam_w);
	DECLARE_WRITE8_MEMBER(spyhunt_scroll_value_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack);


	DECLARE_WRITE8_MEMBER(ay1_porta_w);
	DECLARE_READ8_MEMBER(ay1_porta_r);

	DECLARE_WRITE8_MEMBER(ay2_porta_w);
	DECLARE_READ8_MEMBER(ay2_porta_r);

	DECLARE_READ8_MEMBER(spyhuntertec_in2_r);
	DECLARE_READ8_MEMBER(spyhuntertec_in3_r);

	TILEMAP_MAPPER_MEMBER(spyhunt_bg_scan);
	TILE_GET_INFO_MEMBER(spyhunt_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(spyhunt_get_alpha_tile_info);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);

	TIMER_DEVICE_CALLBACK_MEMBER(analog_count_callback);
	void reset_analog_timer();

	UINT8 m_analog_select;
	UINT8 m_analog_count;
};

WRITE8_MEMBER(spyhuntertec_state::ay1_porta_w)
{
//  printf("ay1_porta_w %02x\n", data);
}

READ8_MEMBER(spyhuntertec_state::ay1_porta_r)
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

WRITE8_MEMBER(spyhuntertec_state::ay2_porta_w)
{
	// d7: latch analog counter on falling edge, d0 selects which one
	if (~data & m_analog_select & 0x80)
	{
		reset_analog_timer();
		m_analog_count = m_analog_input[data & 1]->read();
	}

	m_analog_select = data;
}

READ8_MEMBER(spyhuntertec_state::ay2_porta_r)
{
// read often, even if port is set to output mode
// maybe latches something?
//  printf("ay2_porta_r\n");
	return 0x00; // not sure value matters
}

WRITE8_MEMBER(spyhuntertec_state::spyhunt_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(spyhuntertec_state::spyhunt_alpharam_w)
{
	m_spyhunt_alpharam[offset] = data;
	m_alpha_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(spyhuntertec_state::spyhunt_scroll_value_w)
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


WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_paletteram_w)
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
	UINT8 *videoram = m_videoram;
	int data = videoram[tile_index];
	int code = (data & 0x3f) | ((data >> 1) & 0x40);
	SET_TILE_INFO_MEMBER(0, code, 0, (data & 0x40) ? TILE_FLIPY : 0);
}


TILE_GET_INFO_MEMBER(spyhuntertec_state::spyhunt_get_alpha_tile_info)
{
	SET_TILE_INFO_MEMBER(2, m_spyhunt_alpharam[tile_index], 0, 0);
}



void spyhuntertec_state::video_start()
{
	/* initialize the background tilemap */
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spyhuntertec_state::spyhunt_get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(spyhuntertec_state::spyhunt_bg_scan),this),  64,16, 64,32);

	/* initialize the text tilemap */
	m_alpha_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(spyhuntertec_state::spyhunt_get_alpha_tile_info),this), TILEMAP_SCAN_COLS,  16,8, 32,32);
	m_alpha_tilemap->set_transparent_pen(0);
	m_alpha_tilemap->set_scrollx(0, 16);

	save_item(NAME(m_spyhunt_sprite_color_mask));
	save_item(NAME(m_spyhunt_scrollx));
	save_item(NAME(m_spyhunt_scrolly));
	save_item(NAME(m_spyhunt_scroll_offset));
}




void spyhuntertec_state::mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	m_screen->priority().fill(1, cliprect);

	/* loop over sprite RAM */
	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, sx, sy, flags;

		/* skip if zero */
		if (spriteram[offs] == 0)
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
		flags = spriteram[offs + 1];
		code = spriteram[offs + 2] + 256 * ((flags >> 3) & 0x01);
		color = ~flags & color_mask;
		flipx = flags & 0x10;
		flipy = flags & 0x20;
		sx = (spriteram[offs + 3] - 3) * 2;
		sy = (241 - spriteram[offs]);

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


UINT32 spyhuntertec_state::screen_update_spyhuntertec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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



WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_fd00_w)
{
//  printf("%04x spyhuntertec_fd00_w %02x\n", space.device().safe_pc(), data);
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(spyhuntertec_state::spyhuntertec_in2_r)
{
	// it writes 04 / 14 to the sound latch (spyhuntertec_fd00_w) before
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
//  printf("%04x spyhuntertec_in2_r\n", space.device().safe_pc());
	
	return (ioport("IN2")->read() & ~0x40) | ((m_analog_count == 0) ? 0x40 : 0x00);
}

READ8_MEMBER(spyhuntertec_state::spyhuntertec_in3_r)
{
	UINT8 ret = ioport("IN3")->read();
//  printf("%04x spyhuntertec_in3_r\n", space.device().safe_pc());
	return ret;
}

static ADDRESS_MAP_START( spyhuntertec_map, AS_PROGRAM, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xa800, 0xa8ff) AM_RAM // the ROM is a solid fill in these areas, and they get tested as RAM, I think they moved the 'real' scroll regs here
	AM_RANGE(0xa900, 0xa9ff) AM_RAM

	AM_RANGE(0x0000, 0xdfff) AM_ROM

	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(spyhunt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xebff) AM_MIRROR(0x0400) AM_RAM_WRITE(spyhunt_alpharam_w) AM_SHARE("spyhunt_alpha")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM //AM_SHARE("nvram")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM AM_SHARE("spriteram") // origional spriteram
	AM_RANGE(0xfa00, 0xfa7f) AM_MIRROR(0x0180) AM_RAM_WRITE(spyhuntertec_paletteram_w) AM_SHARE("paletteram")

	AM_RANGE(0xfc00, 0xfc00) AM_READ_PORT("DSW0")
	AM_RANGE(0xfc01, 0xfc01) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc02, 0xfc02) AM_READ(spyhuntertec_in2_r)
	AM_RANGE(0xfc03, 0xfc03) AM_READ(spyhuntertec_in3_r)

	AM_RANGE(0xfd00, 0xfd00) AM_WRITE( spyhuntertec_fd00_w )

	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram2") // actual spriteram for this hw??
ADDRESS_MAP_END

WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_port04_w)
{
}

WRITE8_MEMBER(spyhuntertec_state::spyhuntertec_portf0_w)
{
	// 0x08 on startup, then 0x03, probably CTC leftovers from the original.
	if ((data != 0x03) && (data != 0x08)) printf("spyhuntertec_portf0_w %02x\n", data);
}

static ADDRESS_MAP_START( spyhuntertec_portmap, AS_IO, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_WRITE(spyhuntertec_port04_w)
	AM_RANGE(0x84, 0x86) AM_WRITE(spyhunt_scroll_value_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITENOP // was watchdog
//  AM_RANGE(0xe8, 0xe8) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_WRITE( spyhuntertec_portf0_w )
ADDRESS_MAP_END


static ADDRESS_MAP_START( spyhuntertec_sound_map, AS_PROGRAM, 8, spyhuntertec_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM

	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


WRITE8_MEMBER(spyhuntertec_state::sound_irq_ack)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

static ADDRESS_MAP_START( spyhuntertec_sound_portmap, AS_IO, 8, spyhuntertec_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0x00, 0x00) AM_WRITE(sound_irq_ack)

	AM_RANGE(0x0012, 0x0013) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
	AM_RANGE(0x0012, 0x0012) AM_DEVREAD("ay3", ay8910_device, data_r)

	AM_RANGE(0x0014, 0x0015) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x0014, 0x0014) AM_DEVREAD("ay1", ay8910_device, data_r)

	AM_RANGE(0x0018, 0x0019) AM_DEVWRITE("ay2", ay8910_device, address_data_w) // data written to port a
	AM_RANGE(0x0018, 0x0018) AM_DEVREAD("ay2", ay8910_device, data_r) // actually read

ADDRESS_MAP_END



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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // analog signal
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


static const UINT32 spyhuntp_charlayout_xoffset[64] =
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


static GFXDECODE_START( spyhuntertec )
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




static MACHINE_CONFIG_START( spyhuntertec, spyhuntertec_state )

// note: no ctc, no nvram
// 2*z80, 3*ay8912
// 2 XTALs: one 20MHz, other one near maincpu ?MHz

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000 ) // NEC D780C-2 (rated 6MHz)
	MCFG_CPU_PROGRAM_MAP(spyhuntertec_map)
	MCFG_CPU_IO_MAP(spyhuntertec_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spyhuntertec_state, irq0_line_hold)
	MCFG_TIMER_DRIVER_ADD("analog_timer", spyhuntertec_state, analog_count_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(30*16, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 30*16-1, 0, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(spyhuntertec_state, screen_update_spyhuntertec)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spyhuntertec)
	MCFG_PALETTE_ADD("palette", 64+4)

//  MCFG_PALETTE_INIT_OWNER(spyhuntertec_state,spyhunt)


	MCFG_CPU_ADD("audiocpu", Z80, 4000000 ) // SGS Z8400B1 (rated 2.5MHz?)
	MCFG_CPU_PROGRAM_MAP(spyhuntertec_sound_map)
	MCFG_CPU_IO_MAP(spyhuntertec_sound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(spyhuntertec_state, irq0_line_assert, 1000)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8912, 3000000/2) // AY-3-8912
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_AY8910_PORT_A_READ_CB(READ8(spyhuntertec_state, ay1_porta_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(spyhuntertec_state, ay1_porta_w))

	MCFG_SOUND_ADD("ay2", AY8912, 3000000/2) // "
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_AY8910_PORT_A_READ_CB(READ8(spyhuntertec_state, ay2_porta_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(spyhuntertec_state, ay2_porta_w))

	MCFG_SOUND_ADD("ay3", AY8912, 3000000/2) // "
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END




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

DRIVER_INIT_MEMBER(spyhuntertec_state,spyhuntertec)
{
	m_spyhunt_sprite_color_mask = 0x00;
	m_spyhunt_scroll_offset = 16;
}


GAMEL(1983, spyhuntpr,spyhunt,  spyhuntertec, spyhuntertec,spyhuntertec_state,  spyhuntertec,ROT90, "bootleg (Recreativos Franco S.A. license, Tecfri)", "Spy Hunter (Spain, Recreativos Franco S.A., Tecfri PCB)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_spyhunttec )
