// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************
    PINBALL
    Mr. Game 1B11188/0

    These games have a M68000 and 3x Z80, and a M114 Sound IC.
    They have a video screen upon which the scores and other info is displayed.

Status:
- motrshow, motrshowa, dakar working in the electronic sense, but not mechanically
- macattck most roms are missing
- wcup90 different hardware, not coded

How to set up the machine (motrshow, motrshowa, dakar):
- These machines need to be loaded with default settings before they can accept coins
- Press - key (minus in main keyboard)
- Press again until you see test 25 (Motor Show) or test 23 (Dakar)
- In the dipswitch menu turn off the Ram Protect switch
- Press Left shift and Right shift together (game stops responding)
- Turn the Ram Protect Switch back on
- Press F3 or reboot
- The default settings have been saved to nvram and you can insert coins
- However, the game cannot be played due to missing balls.

ToDo:
- Support for electronic volume control
- Audio rom banking
- Most sounds missing due to unemulated M114 chip
- wcup90 is different hardware and there's no schematic

*****************************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "video/resnet.h"
#include "sound/tms5220.h"
#include "sound/dac.h"
#include "machine/i8255.h"

class mrgame_state : public driver_device
{
public:
	mrgame_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
		, m_p_objectram(*this, "objectram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu1(*this, "audiocpu1")
		, m_audiocpu2(*this, "audiocpu2")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
	{ }

	DECLARE_PALETTE_INIT(mrgame);
	DECLARE_DRIVER_INIT(mrgame);
	DECLARE_WRITE8_MEMBER(ack1_w);
	DECLARE_WRITE8_MEMBER(ack2_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(row_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(triple_w);
	DECLARE_WRITE8_MEMBER(video_w);
	DECLARE_WRITE8_MEMBER(video_ctrl_w);
	DECLARE_READ8_MEMBER(col_r);
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_READ8_MEMBER(rsw_r);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	UINT32 screen_update_mrgame(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	std::unique_ptr<bitmap_ind16> m_tile_bitmap;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_p_videoram;
	required_shared_ptr<UINT8> m_p_objectram;
	required_device<gfxdecode_device> m_gfxdecode;
private:
	bool m_ack1;
	bool m_ack2;
	bool m_ackv;
	bool m_flip;
	UINT8 m_irq_state;
	UINT8 m_row_data;
	UINT8 m_sound_data;
	UINT8 m_gfx_bank;
	UINT8 m_video_data;
	UINT8 m_video_status;
	UINT8 m_video_ctrl[8];
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<m68000_device> m_maincpu;
	required_device<z80_device> m_audiocpu1;
	required_device<z80_device> m_audiocpu2;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
};


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, mrgame_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x020000, 0x02ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x030000, 0x030001) AM_READ8(rsw_r, 0xff) //RSW ACK
	AM_RANGE(0x030002, 0x030003) AM_WRITE8(sound_w, 0xff) //W SOUND
	AM_RANGE(0x030004, 0x030005) AM_WRITE8(video_w, 0xff00) //W VID
	AM_RANGE(0x030006, 0x030007) AM_WRITE8(triple_w, 0xff) //W CS
	AM_RANGE(0x030008, 0x030009) AM_WRITENOP //W DATA - lamp/sol data
	AM_RANGE(0x03000a, 0x03000b) AM_WRITE8(row_w, 0xff) //W ROW
	AM_RANGE(0x03000c, 0x03000d) AM_READ8(col_r, 0xff) //R COL
	AM_RANGE(0x03000e, 0x03000f) AM_WRITENOP //EXT ADD - lamp/sol data
ADDRESS_MAP_END

static ADDRESS_MAP_START( video_map, AS_PROGRAM, 8, mrgame_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("video", 0)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x50ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("objectram")
	AM_RANGE(0x6800, 0x6807) AM_MIRROR(0x07f8) AM_WRITE(video_ctrl_w)
	AM_RANGE(0x7000, 0x77ff) AM_READNOP //AFR - looks like a watchdog
	AM_RANGE(0x8100, 0x8103) AM_MIRROR(0x7efc) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio1_map, AS_PROGRAM, 8, mrgame_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("audio1", 0)
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio1_io, AS_IO, 8, mrgame_state )
	ADDRESS_MAP_GLOBAL_MASK(3)
	AM_RANGE(0x0000, 0x0000) AM_WRITENOP //AM_DEVWRITE("dac", dac_device, write_unsigned8) //DA1. The DC output might be an electronic volume control of the M114's output.
	AM_RANGE(0x0001, 0x0001) AM_READ(sound_r) //IN1
	AM_RANGE(0x0002, 0x0002) AM_WRITE(ack1_w) //AKL1
	AM_RANGE(0x0003, 0x0003) AM_WRITENOP //SGS pass data to M114
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio2_map, AS_PROGRAM, 8, mrgame_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("audio2", 0)
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio2_io, AS_IO, 8, mrgame_state )
	ADDRESS_MAP_GLOBAL_MASK(7)
	AM_RANGE(0x0000, 0x0000) AM_DEVWRITE("dacl", dac_device, write_unsigned8) //DA2
	AM_RANGE(0x0001, 0x0001) AM_READ(sound_r) //IN2
	AM_RANGE(0x0002, 0x0002) AM_WRITE(ack2_w) //AKL2
	AM_RANGE(0x0003, 0x0003) AM_DEVREADWRITE("tms", tms5220_device, status_r, data_w) //Speech
	AM_RANGE(0x0004, 0x0004) AM_DEVWRITE("dacr", dac_device, write_unsigned8) //DA3
ADDRESS_MAP_END

static INPUT_PORTS_START( mrgame )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "Ram Protect")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0e, 0x0e, "Country")
	PORT_DIPSETTING(    0x00, "Italy 1")
	PORT_DIPSETTING(    0x02, "Italy")
	PORT_DIPSETTING(    0x04, "Great Britain")
	PORT_DIPSETTING(    0x06, "France")
	PORT_DIPSETTING(    0x08, "Germany")
	PORT_DIPSETTING(    0x0a, "Belgium")
	PORT_DIPSETTING(    0x0c, "Yugoslavia")
	PORT_DIPSETTING(    0x0e, "U.S.A.")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Flipper") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Flipper") PORT_CODE(KEYCODE_LSHIFT)

	// These dips are only documented for Motor Show
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Test Game")
	PORT_DIPSETTING(    0x01, "Connected")
	PORT_DIPSETTING(    0x00, "Disconnected")
	PORT_DIPNAME( 0x02, 0x02, "Dragster")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPNAME( 0x04, 0x04, "F.1.")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPNAME( 0x08, 0x08, "Motocross")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Advance Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Return Test")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Factory Burn Test")
	PORT_BIT( 0xe9, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

READ8_MEMBER( mrgame_state::rsw_r )
{
	return m_io_dsw0->read() | ((UINT8)m_ack1 << 5) | ((UINT8)m_ack2 << 4);
}

// this is like a keyboard, energise a row and read the column data
READ8_MEMBER( mrgame_state::col_r )
{
	if (m_row_data == 0)
		return m_io_x0->read();
	else
	if (m_row_data == 1)
		return m_io_x1->read();
	else
	if (m_row_data == 7)
		return m_video_status;
	else

	return 0xff;
}

WRITE8_MEMBER( mrgame_state::row_w )
{
	m_row_data = data & 7;
}

READ8_MEMBER( mrgame_state::sound_r )
{
	return m_sound_data;
}

WRITE8_MEMBER( mrgame_state::sound_w )
{
	m_sound_data = data;
	m_audiocpu1->set_input_line(INPUT_LINE_NMI, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
	m_audiocpu2->set_input_line(INPUT_LINE_NMI, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

// this produces 24 outputs from three driver chips to drive lamps & solenoids
WRITE8_MEMBER( mrgame_state::triple_w )
{
	if ((data & 0x18)==0)
		m_ackv = BIT(data, 7);
}

WRITE8_MEMBER( mrgame_state::video_w )
{
	m_video_data = data;
}

WRITE8_MEMBER( mrgame_state::video_ctrl_w )
{
	m_video_ctrl[offset] = data;

	if (offset == 0)
		m_gfx_bank = (m_gfx_bank & 6) | BIT(data, 0);
	else
	if (offset == 3)
		m_gfx_bank = (m_gfx_bank & 5) | (BIT(data, 0) << 1);
	else
	if (offset == 4)
		m_gfx_bank = (m_gfx_bank & 3) | (BIT(data, 0) << 2);
	else
	if (offset == 6)
		m_flip = BIT(data, 0);
}

WRITE8_MEMBER( mrgame_state::ack1_w )
{
	m_ack1 = BIT(data, 0);
}

WRITE8_MEMBER( mrgame_state::ack2_w )
{
	m_ack2 = BIT(data, 0);
}

READ8_MEMBER( mrgame_state::porta_r )
{
	return m_video_data;
}

WRITE8_MEMBER( mrgame_state::portb_w )
{
	m_video_status = data;
	m_ackv = 0;
}

READ8_MEMBER( mrgame_state::portc_r )
{
	return m_io_dsw1->read() | ((UINT8)m_ackv << 4);
}

void mrgame_state::machine_start()
{
	m_tile_bitmap=std::make_unique<bitmap_ind16>(256,256);
}

void mrgame_state::machine_reset()
{
	m_sound_data = 0xff;
	m_irq_state = 0xff;
	m_video_data = 0;
	m_gfx_bank = 0;
	m_video_status = 0;
	m_ack1 = 0;
	m_ack2 = 0;
	m_ackv = 0;
	m_flip = 0;
	m_row_data = 0;
}

DRIVER_INIT_MEMBER( mrgame_state, mrgame )
{
}

// This pulses the IRQ pins of both audio cpus. The schematic does not
//show which 4040 output is used, so we have guessed.
TIMER_DEVICE_CALLBACK_MEMBER( mrgame_state::irq_timer )
{
	m_irq_state++;
	// pulse_line of IRQ not allowed, so trying this instead
	if (m_irq_state == 254)
	{
		m_audiocpu1->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_audiocpu2->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
	else
	if (m_irq_state == 255)
	{
		m_audiocpu1->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		m_audiocpu2->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

// layouts from pinmame
static const gfx_layout charlayout =
{
	8, 8,
	4096,
	2,
	{ 0, 0x8000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	8*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	1024,
	2,
	{ 0, 0x8000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 },
	{ 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 },
	32*8
};

static GFXDECODE_START( mrgame )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "chargen", 0, spritelayout, 0, 16 )
GFXDECODE_END

PALETTE_INIT_MEMBER( mrgame_state, mrgame)
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	UINT8 i, bit0, bit1, bit2, r, g, b;
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
	{
		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
		palette.set_pen_color(i+32, rgb_t(r, g, b));
	}
}

// most of this came from pinmame as the diagram doesn't make a lot of sense
UINT32 mrgame_state::screen_update_mrgame(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 x,y,ptr=0,col;
	INT32 scrolly[32];
	UINT16 chr;
	bool flipx,flipy;

	// text
	for (x = 0; x < 32; x++)
	{
		scrolly[x] = -m_p_objectram[ptr++];
		col = m_p_objectram[ptr++];

		for (y = 0; y < 32; y++)
		{
			chr = m_p_videoram[x+y*32] | (m_gfx_bank << 8);

			m_gfxdecode->gfx(0)->opaque(*m_tile_bitmap, m_tile_bitmap->cliprect(),
				chr,
				col,
				m_flip,0,
				x*8,y*8);
		}
	}

	// scroll each column as needed
	copyscrollbitmap(bitmap,*m_tile_bitmap,0,nullptr,32,scrolly,cliprect);


	// sprites
	for (ptr = 0x40; ptr < 0x60; ptr += 4)
	{
		x = m_p_objectram[ptr + 3] + 1;
		y = 255 - m_p_objectram[ptr];
		flipx = BIT(m_p_objectram[ptr + 1], 6);
		flipy = BIT(m_p_objectram[ptr + 1], 7);
		chr = (m_p_objectram[ptr + 1] & 0x3f) | (m_gfx_bank << 6);
		col = m_p_objectram[ptr + 2];

		if ((y > 16) && (x > 24))
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				chr,
				col,
				flipx,flipy,
				x,y-16,0);
	}

	return 0;
}

static MACHINE_CONFIG_START( mrgame, mrgame_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mrgame_state, irq1_line_hold, 183)
	MCFG_CPU_ADD("videocpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(video_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mrgame_state, nmi_line_pulse)
	MCFG_CPU_ADD("audiocpu1", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(audio1_map)
	MCFG_CPU_IO_MAP(audio1_io)
	MCFG_CPU_ADD("audiocpu2", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(audio2_map)
	MCFG_CPU_IO_MAP(audio2_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 8, 247) // If you align with X on test screen some info is chopped off
	MCFG_SCREEN_UPDATE_DRIVER(mrgame_state, screen_update_mrgame)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 64)
	MCFG_PALETTE_INIT_OWNER(mrgame_state, mrgame)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mrgame)

	/* Sound */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_DAC_ADD("dacl")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_DAC_ADD("dacr")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
	MCFG_SOUND_ADD("tms", TMS5220, 672000) // uses a RC combination. 672k copied from jedi.h
	MCFG_TMS52XX_READYQ_HANDLER(INPUTLINE("audiocpu2", Z80_INPUT_LINE_WAIT))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	/* Devices */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", mrgame_state, irq_timer, attotime::from_hz(16000)) //ugh
	MCFG_DEVICE_ADD("ppi", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(mrgame_state, porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(mrgame_state, portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(mrgame_state, portc_r))
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Dakar (06/1988)
/-------------------------------------------------------------------*/
ROM_START(dakar)
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000001, 0x8000, CRC(83183929) SHA1(977ac10a1e78c759eb0550794f2639fe0e2d1507))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000000, 0x8000, CRC(2010d28d) SHA1(d262dabd9298566df43df298cf71c974bee1434a))

	ROM_REGION(0x10000, "video", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(88a9ca81) SHA1(9660d416b2b8f1937cda7bca51bd287641c7730c))

	ROM_REGION( 0x10000, "chargen", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(3c68b448) SHA1(f416f00d2de0c71c021fec0e9702ba79b761d5e7))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(0aac43e9) SHA1(28edfeddb2d54e40425488bad37e3819e4488b0b))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(c8269b27) SHA1(daa83bfdb1e255b846bbade7f200abeaa9399c06))

	ROM_REGION(0x10000, "audio1", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(29e9417e) SHA1(24f465993da7c93d385ec453497f2af4d8abb6f4))
	ROM_LOAD("snd_ic07.rom", 0x8000, 0x8000, CRC(71ab15fe) SHA1(245842bb41410ea481539700f79c7ef94f8f8924))

	ROM_REGION(0x4000, "m114", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))

	ROM_REGION(0x10000, "audio2", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(7b2394d1) SHA1(f588f5105d75b54dd65bb6448a2d7774fb8477ec))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4039ea65) SHA1(390fce94d1e48b395157d8d9afaa485114c58d52))
ROM_END

/*-------------------------------------------------------------------
/ Motor Show (1989)
/-------------------------------------------------------------------*/
ROM_START(motrshow)
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000001, 0x8000, CRC(e862ca71) SHA1(b02e5f39f9427d58b70b7999a5ff6075beff05ae))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000000, 0x8000, CRC(c898ae25) SHA1(f0e1369284a1e0f394f1d40281fd46252016602e))

	ROM_REGION(0x10000, "video", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(1d4568e2) SHA1(bfc2bb59708ce3a09f9a1b3460ed8d5269840c97))

	ROM_REGION( 0x10000, "chargen", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(c27a4ded) SHA1(9c2c9b17f1e71afb74bdfbdcbabb99ef935d32db))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(1664ec8d) SHA1(e7b15acdac7dfc51b668e908ca95f02a2b569737))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(5b585252) SHA1(b88e56ebdce2c3a4b170aff4b05018e7c21a79b8))

	ROM_REGION(0x10000, "audio1", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(fba5a8f1) SHA1(ddf989abebe05c569c9ecdd498bd8ea409df88ac))

	ROM_REGION(0x4000, "m114", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))

	ROM_REGION(0x10000, "audio2", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(9dec153d) SHA1(8a0140257316aa19c0401456839e11b6896609b1))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4f42be6e) SHA1(684e988f413cd21c785ad5d60ef5eaddddaf72ab))
ROM_END

ROM_START(motrshowa)
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD16_BYTE("cpuic13a.rom", 0x000001, 0x8000, CRC(2dbdd9d4) SHA1(b404814a4e83ead6da3c57818ae97f23d380f9da))
	ROM_LOAD16_BYTE("cpuic14b.rom", 0x000000, 0x8000, CRC(0bd98fec) SHA1(b90a7e997db59740398003ba94a69118b1ee70af))

	ROM_REGION(0x10000, "video", 0)
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(1d4568e2) SHA1(bfc2bb59708ce3a09f9a1b3460ed8d5269840c97))

	ROM_REGION( 0x10000, "chargen", 0 )
	ROM_LOAD("vid_ic55.rom", 0x0000, 0x8000, CRC(c27a4ded) SHA1(9c2c9b17f1e71afb74bdfbdcbabb99ef935d32db))
	ROM_LOAD("vid_ic56.rom", 0x8000, 0x8000, CRC(1664ec8d) SHA1(e7b15acdac7dfc51b668e908ca95f02a2b569737))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic66.rom", 0x0000, 0x0020, CRC(5b585252) SHA1(b88e56ebdce2c3a4b170aff4b05018e7c21a79b8))

	ROM_REGION(0x10000, "audio1", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(fba5a8f1) SHA1(ddf989abebe05c569c9ecdd498bd8ea409df88ac))

	ROM_REGION(0x4000, "m114", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))

	ROM_REGION(0x10000, "audio2", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, CRC(9dec153d) SHA1(8a0140257316aa19c0401456839e11b6896609b1))
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, CRC(4f42be6e) SHA1(684e988f413cd21c785ad5d60ef5eaddddaf72ab))
ROM_END

/*-------------------------------------------------------------------
/ Mac Attack (1990)
/-------------------------------------------------------------------*/
ROM_START(macattck)
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000001, 0x8000, NO_DUMP)
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000000, 0x8000, NO_DUMP)

	ROM_REGION(0x10000, "video", 0)
	ROM_LOAD("vid_ic91.rom", 0x00000, 0x8000, CRC(42d2ba01) SHA1(c13d38c2798575760461912cef65dde57dfd938c))

	ROM_REGION( 0x30000, "chargen", 0 )
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(f6e047fb) SHA1(6be712dda60257b9e7014315c8fee19812622bf6))
	ROM_LOAD("vid_ic15.rom", 0x08000, 0x8000, CRC(405a8f54) SHA1(4d58915763db3c3be2bfc166be1a12285ff2c38b))
	ROM_LOAD("vid_ic16.rom", 0x10000, 0x8000, CRC(063ea783) SHA1(385dbfcc8ecd3a784f9a8752d00e060b48d70d6a))
	ROM_LOAD("vid_ic17.rom", 0x18000, 0x8000, CRC(9f95abf8) SHA1(d71cf36c8bf27ad41b2d3cebd0af620a34ce0062) BAD_DUMP)
	ROM_LOAD("vid_ic18.rom", 0x20000, 0x8000, CRC(83ef25f8) SHA1(bab482badb8646b099dbb197ca9af3a126b274e3))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic61.rom", 0x0000, 0x0020, CRC(538c72ae) SHA1(f704492568257fcc4a4f1189207c6fb6526eb81c) BAD_DUMP)

	ROM_REGION(0x10000, "audio1", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x4000, "m114", 0)
	ROM_LOAD("snd_ic22.rom", 0x0000, 0x4000, NO_DUMP)

	ROM_REGION(0x10000, "audio2", 0)
	ROM_LOAD("snd_ic35.rom", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("snd_ic36.rom", 0x8000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ World Cup 90 (1990)
/-------------------------------------------------------------------*/
ROM_START(wcup90)
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD16_BYTE("cpu_ic13.rom", 0x000001, 0x8000, CRC(0e2edfb0) SHA1(862fb1f6509fb1f560d0b2bb8a5764f64b259f04))
	ROM_LOAD16_BYTE("cpu_ic14.rom", 0x000000, 0x8000, CRC(fdd03165) SHA1(6dc6e68197218f8808436098c26cd04fc3215b1c))

	ROM_REGION(0x10000, "video", 0)
	ROM_LOAD("vid_ic91.rom", 0x00000, 0x8000, CRC(3287ad20) SHA1(d5a453efc7292670073f157dca04897be857b8ed))

	ROM_REGION( 0x30000, "chargen", 0 )
	ROM_LOAD("vid_ic14.rom", 0x00000, 0x8000, CRC(a101d562) SHA1(ad9ad3968f13169572ec60e22e84acf43382b51e))
	ROM_LOAD("vid_ic15.rom", 0x08000, 0x8000, CRC(40791e7a) SHA1(788760b8527df48d1825be88099491b6e94f0a19))
	ROM_LOAD("vid_ic16.rom", 0x10000, 0x8000, CRC(a7214157) SHA1(a4660180e8491a37028fec8533cf13daf839a7c4))
	ROM_LOAD("vid_ic17.rom", 0x18000, 0x8000, CRC(caf4fb04) SHA1(81784a4dc7c671090cf39cafa7d34a6b34523168))
	ROM_LOAD("vid_ic18.rom", 0x20000, 0x8000, CRC(83ad2a10) SHA1(37664e5872e6322ee6bb61ec9385876626598152))

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD("vid_ic61.rom", 0x0000, 0x0020, CRC(538c72ae) SHA1(f704492568257fcc4a4f1189207c6fb6526eb81c))

	ROM_REGION(0x10000, "audio1", 0)
	ROM_LOAD("snd_ic06.rom", 0x0000, 0x8000, CRC(19a66331) SHA1(fbd71bc378b5a04247fd1754529c66b086eb33d8))

	ROM_REGION(0x4000, "user1", 0)
	ROM_LOAD("snd_ic21.rom", 0x0000, 0x4000, CRC(e6c1098e) SHA1(06bf8917a27d5e46e4aab93e1f212918418e3a82))

	ROM_REGION(0x30000, "user2", 0)
	ROM_LOAD("snd_ic45.rom", 0x00000, 0x10000, CRC(265aa979) SHA1(9ca10c41526a2d227c21f246273ca14bec7f1bc7))
	ROM_LOAD("snd_ic46.rom", 0x10000, 0x10000, CRC(7edb321e) SHA1(b242e94c24e996d2de803d339aa9bf6e93586a4c))

	ROM_REGION(0x10000, "audio2", 0)
	ROM_LOAD("snd_ic44.rom", 0x00000, 0x8000, CRC(00946570) SHA1(83e7dd89844679571ab2a803295c8ca8941a4ac7))
ROM_END


GAME(1988,  dakar,     0,         mrgame,  mrgame, mrgame_state,  mrgame,  ROT0,  "Mr Game", "Dakar", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1989,  motrshow,  0,         mrgame,  mrgame, mrgame_state,  mrgame,  ROT0,  "Mr Game", "Motor Show (set 1)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1989,  motrshowa, motrshow,  mrgame,  mrgame, mrgame_state,  mrgame,  ROT0,  "Mr Game", "Motor Show (set 2)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )
GAME(1990,  macattck,  0,         mrgame,  mrgame, mrgame_state,  mrgame,  ROT0,  "Mr Game", "Mac Attack", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  wcup90,    0,         mrgame,  mrgame, mrgame_state,  mrgame,  ROT0,  "Mr Game", "World Cup 90", MACHINE_IS_SKELETON_MECHANICAL)
