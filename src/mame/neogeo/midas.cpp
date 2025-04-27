// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                        -= Andamiro's Midas hardware =-

                                            driver by Luca Elia

    a reengineered Neo-Geo, with a few differences: no Z80, better sound chip, serial eeprom and 256 color tiles.
    Plus a PIC12C508A microcontroller, probably for the protection checks (I've patched them out for now).

    Hardware description:

    http://web.archive.org/web/20041018094226/http://www.andamiro.com/kor/business/hard_05.html

    CPU         MC68000

    VRAM        256kbyte (4Display/Access bank)

    PaletteRAM  96kbyte

    Display     320(x)*224(y)

    Sprite      16(x)*240(y(max))*380(max) (96 sprite/line(max))
                128 level y-axis scaling (or line control effect)
                16 level x-axis scale-down x,y fip
                255color/sprite(of 256 palette set)

    Text        8dot*8dot, 40(x)*28(y),
                255color/text(of 16 palette set)

    Color       32640 of 24bit True Color
                (255 colors/sprite)

    Sound       8 channel 44.1KHz(max) stereo
                4bit ADPCM, 8bit PCM, 16bit PCM

    Controller  4 direction,
                6 button joystick * 2 player (max. 4 playersupport)
                light-gun*2 player
                trackball*2 player

    Maximum ROM 2Gbit

    size  220(x)mm * 210(y)mm


    Notes:

    - hammer: keep test button pressed during boot for hardware tests

*************************************************************************************************************/

#include "emu.h"

#include "neogeo_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/ymz280b.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class midas_state : public driver_device
{
public:
	midas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_prize(*this, "prize%u", 1),
		m_ticket(*this, "ticket"),
		m_zoomram(*this, "zoomtable"),
		m_zoomtable(*this, "spritegen:zoomy")
	{ }

	void livequiz(machine_config &config);

	void init_livequiz();

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t ret_ffff();
	void gfxregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void livequiz_coin_w(uint8_t data);
	void eeprom_w(uint8_t data);
	void zoomtable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void screen_vblank(int state);

	void livequiz_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<neosprite_midas_device> m_sprgen;
	required_device<screen_device> m_screen;
	optional_device_array<ticket_dispenser_device, 2> m_prize;
	optional_device<ticket_dispenser_device> m_ticket;
	required_shared_ptr<uint16_t> m_zoomram;
	required_region_ptr<uint8_t> m_zoomtable;

};

class hammer_state : public midas_state
{
public:
	hammer_state(const machine_config &mconfig, device_type type, const char *tag) :
		midas_state(mconfig, type, tag),
		m_io_hammer(*this, "HAMMER"),
		m_io_sensorx(*this, "SENSORX"),
		m_io_sensory(*this, "SENSORY")
	{ }

	void hammer(machine_config &config);

private:
	uint16_t sensor_r();
	void coin_w(uint8_t data);
	void motor_w(uint8_t data);

	void hammer_map(address_map &map) ATTR_COLD;

	required_ioport m_io_hammer;
	required_ioport m_io_sensorx;
	required_ioport m_io_sensory;
};




void midas_state::video_start()
{
}

uint32_t midas_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// fill with background color first
	bitmap.fill(0x0, cliprect);

	m_sprgen->draw_sprites(bitmap, cliprect.min_y);

	m_sprgen->draw_fixed_layer(bitmap, cliprect.min_y);

	return 0;
}

void midas_state::eeprom_w(uint8_t data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 2));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t midas_state::ret_ffff()
{
	return 0xffff;
}

void midas_state::gfxregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* accessing the LSB only is not mapped */
	if (mem_mask != 0x00ff)
	{
		/* accessing the MSB only stores same data in MSB and LSB */
		if (mem_mask == 0xff00)
			data = (data & 0xff00) | (data >> 8);

		switch (offset)
		{
		case 0x00: m_sprgen->set_videoram_offset(data); break;
		case 0x01: m_sprgen->set_videoram_data(data); break;
		case 0x02: m_sprgen->set_videoram_modulo(data); break;
		}
	}
}

void midas_state::zoomtable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_zoomram[offset]);

	if (ACCESSING_BITS_0_7)
	{
		m_zoomtable[offset] = data & 0xff;
	}
}
/***************************************************************************************
                                       Live Quiz Show
***************************************************************************************/

void midas_state::livequiz_coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
#ifdef MAME_DEBUG
//  popmessage("coin %04X", data);
#endif
}

void midas_state::livequiz_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x900000, 0x900001).portr("DSW_PLAYER1");
	map(0x920000, 0x920001).portr("SERVICE");
	map(0x940000, 0x940001).portr("PLAYER2");
	map(0x980000, 0x980001).portr("START");

	map(0x980001, 0x980001).w(FUNC(midas_state::livequiz_coin_w));

	map(0x9a0001, 0x9a0001).w(FUNC(midas_state::eeprom_w));

	map(0x9c0000, 0x9c0005).w(FUNC(midas_state::gfxregs_w));
	map(0x9c000c, 0x9c000d).nopw();    // IRQ Ack, temporary

	map(0xa00000, 0xa3ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa40000, 0xa7ffff).ram();

	map(0xb00000, 0xb00001).r(FUNC(midas_state::ret_ffff));
	map(0xb20000, 0xb20001).r(FUNC(midas_state::ret_ffff));
	map(0xb40000, 0xb40001).r(FUNC(midas_state::ret_ffff));
	map(0xb60000, 0xb60001).r(FUNC(midas_state::ret_ffff));

	map(0xb80008, 0xb8000b).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);

	map(0xba0000, 0xba0001).portr("START3");
	map(0xbc0000, 0xbc0001).portr("PLAYER3");

	map(0xd00000, 0xd1ffff).ram().w(FUNC(midas_state::zoomtable_w)).share("zoomtable"); // zoom table?

	map(0xe00000, 0xe3ffff).ram();
}

/***************************************************************************************
                                          Hammer
***************************************************************************************/

uint16_t hammer_state::sensor_r()
{
	if (m_io_hammer->read() & 0x80)
		return 0xffff;

	return (m_io_sensory->read() << 8) | m_io_sensorx->read();
}

void hammer_state::coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
#ifdef MAME_DEBUG
//  popmessage("coin %04X", data);
#endif
}

void hammer_state::motor_w(uint8_t data)
{
	m_prize[0]->motor_w(BIT(data, 0));
	m_prize[1]->motor_w(BIT(data, 1));
	m_ticket->motor_w(BIT(data, 4));
	// BIT(data, 7) ?
#ifdef MAME_DEBUG
//  popmessage("motor %04X", data);
#endif
}

void hammer_state::hammer_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x900000, 0x900001).portr("DSW");
	map(0x920000, 0x920001).portr("SERVICE");
	map(0x940000, 0x940001).portr("IN0");
	map(0x980000, 0x980001).portr("TILT");

	map(0x980001, 0x980001).w(FUNC(hammer_state::coin_w));

	map(0x9a0001, 0x9a0001).w(FUNC(hammer_state::eeprom_w));

	map(0x9c0000, 0x9c0005).w(FUNC(hammer_state::gfxregs_w));
	map(0x9c000c, 0x9c000d).nopw();    // IRQ Ack, temporary

	map(0xa00000, 0xa3ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa40000, 0xa7ffff).ram();

	map(0xb00000, 0xb00001).r(FUNC(hammer_state::ret_ffff));
	map(0xb20000, 0xb20001).r(FUNC(hammer_state::ret_ffff));
	map(0xb40000, 0xb40001).r(FUNC(hammer_state::ret_ffff));
	map(0xb60000, 0xb60001).r(FUNC(hammer_state::ret_ffff));

	map(0xb80008, 0xb8000b).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);

	map(0xba0000, 0xba0001).portr("IN1");
	map(0xbc0000, 0xbc0001).portr("HAMMER");

	map(0xbc0003, 0xbc0003).w(FUNC(hammer_state::motor_w));

	map(0xbc0004, 0xbc0005).r(FUNC(hammer_state::sensor_r));

	map(0xd00000, 0xd1ffff).ram().w(FUNC(hammer_state::zoomtable_w)).share("zoomtable"); // zoom table?

	map(0xe00000, 0xe3ffff).ram();
}


static const gfx_layout layout16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 56,48,40,32,24,16,8,0 },
	{ 16*64+7,16*64+6,16*64+5,16*64+4,16*64+3,16*64+2,16*64+1,16*64, 7,6,5,4,3,2,1,0 },
	{ STEP16(0, 64) },
	16*128
};

static const gfx_layout layout8x8x8_2 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 8,9,10,11, 0,1,2,3 },
	{ (32*2+1)*4, 32*2*4, (48*2+1)*4, 48*2*4, (0+1)*4, 0*4, (16*2+1)*4, 16*2*4 },
	{ 0*8*2, 1*8*2, 2*8*2, 3*8*2, 4*8*2, 5*8*2, 6*8*2, 7*8*2 },
	32*8*2
};

static GFXDECODE_START( gfx_midas )
	GFXDECODE_ENTRY( "sprites", 0, layout16x16x8, 0, 0x100 )
	GFXDECODE_ENTRY( "tiles",   0, layout8x8x8_2, 0,  0x80 )
GFXDECODE_END


static INPUT_PORTS_START( livequiz )

	PORT_START("DSW_PLAYER1")   // 900000
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")   // 920000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1   )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)) // EEPROM
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040,   IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYER2")   // 940000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START") // 980000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START3")    // ba0000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PLAYER3")   // bc0000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( hammer )

	PORT_START("DSW")   // 900000
	PORT_DIPNAME( 0x01, 0x01, "Debug Mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Game Mode" )
	PORT_DIPSETTING(    0x06, "Prize Game" )
	PORT_DIPSETTING(    0x00, "Ticket Game 1" ) // not in manual, does not work (it requires a toggleable ticket dispenser)
	PORT_DIPSETTING(    0x04, "Ticket Game 2" )
	PORT_DIPSETTING(    0x02, "Generic Game" )
	PORT_DIPNAME( 0x08, 0x08, "Warning Sound" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")   // 920000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN2     )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_SERVICE1  )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM   ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN   )
	PORT_SERVICE_NO_TOGGLE( 0x0040,   IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN   )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")   // 940000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TILT")  // 980000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_TILT    )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // ba0000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("HAMMER")    // bc0000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("prize1", FUNC(ticket_dispenser_device::line_r)) // prize 1 sensor ("tejisw 1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("prize2", FUNC(ticket_dispenser_device::line_r)) // prize 2 sensor ("tejisw 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(5) PORT_NAME( "Hammer" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SENSORX")
	PORT_BIT( 0xff, 0x20, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 0x3f+1) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)

	PORT_START("SENSORY")
	PORT_BIT( 0xff, 0x18, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 0x2f+1) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)

INPUT_PORTS_END

void midas_state::machine_start()
{
	m_sprgen->set_pens(m_palette->pens());
	m_sprgen->set_sprite_region(memregion("sprites")->base(), memregion("sprites")->bytes());
	m_sprgen->set_fixed_regions(memregion("tiles")->base(), memregion("tiles")->bytes(), memregion("tiles"));
	m_sprgen->set_fixed_layer_source(0); // temporary: ensure banking is disabled
}

void midas_state::machine_reset()
{
}

void midas_state::screen_vblank(int state)
{
	if (state) m_sprgen->buffer_vram();
}



void midas_state::livequiz(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &midas_state::livequiz_map);
	m_maincpu->set_vblank_int("screen", FUNC(midas_state::irq1_line_hold));

	pic16c56_device &pic1(PIC16C56(config, "pic1", XTAL(24'000'000) / 6)); // !! PIC12C508 !! unknown MHz
	pic1.set_disable(); // Currently not hooked up

	pic16c56_device &pic2(PIC16C56(config, "pic2", XTAL(24'000'000) / 6)); // !! PIC12C508 !! unknown MHz
	pic2.set_disable(); // Currently not hooked up

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, NEOGEO_HTOTAL, NEOGEO_HBEND, NEOGEO_HBSTART, NEOGEO_VTOTAL, NEOGEO_VBEND, NEOGEO_VBSTART);
	m_screen->set_screen_update(FUNC(midas_state::screen_update));
	m_screen->screen_vblank().set(FUNC(midas_state::screen_vblank));

	NEOGEO_SPRITE_MIDAS(config, m_sprgen, 0).set_screen(m_screen);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_midas);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 0x10000);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400)));
	ymz.add_route(0, "speaker", 0.80, 0);
	ymz.add_route(1, "speaker", 0.80, 1);
}

void hammer_state::hammer(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hammer_state::hammer_map);
	m_maincpu->set_vblank_int("screen", FUNC(hammer_state::irq1_line_hold));

	at89c52_device &mcu(AT89C52(config, "mcu", XTAL(24'000'000) / 2)); // on top board, unknown MHz
	mcu.set_disable(); // Currently not hooked up

	EEPROM_93C46_16BIT(config, m_eeprom);

	TICKET_DISPENSER(config, m_prize[0], attotime::from_msec(1000*5));
	TICKET_DISPENSER(config, m_prize[1], attotime::from_msec(1000*5));
	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(200));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000) / 4, NEOGEO_HTOTAL, NEOGEO_HBEND, NEOGEO_HBSTART, NEOGEO_VTOTAL, NEOGEO_VBEND, NEOGEO_VBSTART);
	m_screen->set_screen_update(FUNC(hammer_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hammer_state::screen_vblank));

	NEOGEO_SPRITE_MIDAS(config, m_sprgen, 0).set_screen(m_screen);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_midas);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 0x10000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center(); // stereo outputs aren't exists?

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400)));
	ymz.add_route(ALL_OUTPUTS, "mono", 0.80);
}


/***************************************************************************************

Live Quiz Show
1999, Andamiro Entertainment Co. Ltd.

Main Board
----------

MIDAS
|----------------------------------------------------------|
|TDA1519A                        |-----ROM-BOARD(ABOVE)----|
|                  558   YAC516  |                         |
|    VOL     558                 |    YMZ280B              |
|SP1               558   12C508A |                         |
|                                |                         |
|         |---------|            |               16.9344MHz|
|         |         |    PAL     |    |--------|           |
|         | MIDAS-2 |            |    |TMP     |   KM681000|
|J        |         |            |    |68HC000 |           |
|A        |         |            |    |        |   KM681000|
|M        |---------|    KM681000|    |--------|           |
|M                               |                 KM681000|
|A                       KM681000|                         |
| DSW(8)                         |                         |
|                        KM681000|                         |
|                                |                         |
|         |---------|            |    |---------|          |
|PUSH_BTN |         |            |    |         |          |
|         | MIDAS-1 |            |    | MIDAS-3 |          |
|93C46    |         |            |    |         |          |
|CN1      |         |            |    |         |          |
|CN2      |---------|            |    |---------|          |
|  KM681000 341256               |                         |
|CN3                             |                    24MHz|
|  KM681000 341256               |-------------------------|
|----------------------------------------------------------|
Notes:
      TMP68HC000   - Toshiba TMP68HC000 CPU clock - 12MHz [24/2] (PLCC68)
      YMZ280 clock - 16.9344MHz
      341256       - NKK N341256SJ-16 32k x8 SRAM (SOJ28)
      KM681000     - Samsung KM681000 128k x8 SRAM (SOP32)
      SP1          - 3 pin connector for stereo sound output
      CN1/2/3      - Connectors for extra controls
      MIDAS-1/2/3  - Custom chips, probably rebadged FPGAs (QFP208)
      12C508A      - Microchip PIC12C508A Microcontroller (DIP8)
      VSync        - 60Hz
      HSync        - 15.21kHz

ROM Board
---------

MIDAS
|-------------------------|
|                         |
|     27C4096.U23         |
|                         |
|  *U21        *U22       |
|                         |
|                         |
|  *U26        *U27       |
|                         |
|                         |
|   U19         U20       |
|                     CN15|
|                     CN13|
|  *U17        *U18       |
|                     CN12|
|                         |
|  *U24        *U25       |
|                         |
|                         |
|   U15         U16       |
|                         |
|                         |
|   U1         *U7        |
|                         |
|12C508A                  |
|   U5         *U6        |
|-------------------------|
Notes:
      * Not populated
      CN15/13/12 - Connectors for extra controls
      12C508A    - Microchip PIC12C508A Microcontroller (DIP8)
      U23        - 27C4096 EPROM
      All other ROMs are MX29F1610 (SOP44)

***************************************************************************************/

ROM_START( livequiz )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "flash.u1", 0x000000, 0x200000, CRC(8ec44493) SHA1(a987886cb87ac0a744f01f2e4a7cc6d12efeaa04) )

	ROM_REGION( 0x000800, "pic1", 0 )
	ROM_LOAD( "main_pic12c508a.u27", 0x000000, 0x000400, CRC(a84f0a7e) SHA1(fb27c05fb27b98ca603697e1be214dc6c8d5f884) )

	ROM_REGION( 0x000800, "pic2", 0 )
	ROM_LOAD( "sub_pic12c508a.u4",   0x000000, 0x000400, CRC(e52ebdc4) SHA1(0f3af66b5ea184e49188e74a873699324a3930f1) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD64_WORD( "flash.u15", 0x000000, 0x200000, CRC(d6eb56f1) SHA1(52d67bb25dd968c79eccb05159a578516b27e557) )
	ROM_LOAD64_WORD( "flash.u19", 0x000002, 0x200000, CRC(daa81532) SHA1(9e66bb4639b92c3d76b7918535f55883f22f24b2) )
	ROM_LOAD64_WORD( "flash.u16", 0x000004, 0x200000, CRC(4c9fd873) SHA1(6e185304ce29771265d3c48b0ef0e840d8bed02d) )
	ROM_LOAD64_WORD( "flash.u20", 0x000006, 0x200000, CRC(b540a8c7) SHA1(25b9b30c7d5ff1e410ea30580017e45590542561) )

	ROM_REGION( 0x080000, "tiles", 0 )
	ROM_LOAD( "27c4096.u23", 0x000000, 0x080000, CRC(25121de8) SHA1(edf24d87551639b871baf3243b452a4e2ba84107) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "flash.u5", 0x000000, 0x200000, CRC(dc062792) SHA1(ec415c918c47ce9d181f014cde317af5717600e4) )

	ROM_REGION( 0x10000, "spritegen:zoomy", ROMREGION_ERASE00 )
	/* uploaded */
ROM_END

void midas_state::init_livequiz()
{
	uint16_t *rom = (uint16_t *) memregion("maincpu")->base();

	// PROTECTION CHECKS
	rom[0x13345a/2] = 0x4e75;
}

/***************************************************************************************

Hammer
Andamiro 2000

PCB Layout
----------

|------------------------------|-------------------|
|TDA1519 17358 YAC516 YMZ280B  |        CN1        |
|   17558    16.9344MHz   ATMEL_ATF1500            |
|  VOL   17358                 |                   |
|                              |   S0.U25          |
|                        MC68HC000CFN16            |
|TD62064                       |   S1.U26          |
|              MIDAS-2         |                   |
| DSW(8)                       |   P.U22    A0L.U44|
|J                             |  LP621024         |
|A                 LP621024    |            A0H.U46|
|M                             |  LP621024         |
|M                 LP621024    |            A1L.U48|
|A                             |  LP621024         |
|                  LP621024    |            A1H.U50|
|                              |                   |
|                              |            A2L.U45|
|                              |                   |
| TESTSW                       |            A2H.U47|
| 93C46        MIDAS-1     MIDAS-3                 |
| LP621024            28MHz    |            A3L.U49|
|           HM62H256           |                   |
| LP621024          HM62H256   |            A3H.U51|
|------------------------------|-------------------|
Notes:
      68000 @ 14MHz [28/2]
      YMZ280B @ 16.9344MHz
      CN1 - connector for top board

Top board
---------

HAMMER TOP PCB VER1.2
AMO30803
|------------------------------------------|
|CN19 CN14 CN15 CN13 CN22 CN23 CN20 CN21   |
|                                          |
|                                          |
|                                 17558    |
|   TD62064  TD62064              17393    |
|                                          |
|                            24MHz      CN2|
|                     AT89C52     17558    |
|PAL                              17393 CN1|
|                                          |
|                             MC7805       |
|------------------------------------------|

***************************************************************************************/

ROM_START( hammer )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p.u22", 0x000000, 0x200000, CRC(687f1596) SHA1(3dc5fb0af1e8c4f3a42ce4aad39635b1111831d8) )

	ROM_REGION( 0x002000, "mcu", 0 )
	ROM_LOAD( "hammer_at89c52", 0x000000, 0x002000, NO_DUMP )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD64_WORD( "a0l.u44", 0x000000, 0x200000, CRC(b9cafd81) SHA1(24698970d1aea0907e2963c872ce61077f44c3af) )
	ROM_LOAD64_WORD( "a0h.u46", 0x800000, 0x200000, CRC(f60f188b) SHA1(486f26c473b46efb402662b2f374e361cd7b4284) )

	ROM_LOAD64_WORD( "a1l.u48", 0x000002, 0x200000, CRC(82129cf9) SHA1(6d68e943854bc9e8ea555bf03107dc9e836ca4d9) )
	ROM_LOAD64_WORD( "a1h.u50", 0x800002, 0x200000, CRC(76897c90) SHA1(aded60d3db834598cd54ad9140eee7be4129cb27) )

	ROM_LOAD64_WORD( "a2l.u45", 0x000004, 0x200000, CRC(d8086ee5) SHA1(9d5f2b3a0f903a69cfd1108ddf5ea61b571c3fe3) )
	ROM_LOAD64_WORD( "a2h.u47", 0x800004, 0x200000, CRC(a64aa2df) SHA1(7e4eb049cd6a5971a455488a484f225763921614) )

	ROM_LOAD64_WORD( "a3l.u49", 0x000006, 0x200000, CRC(4e83cf00) SHA1(e66a0b4eae0f46bf36126be3795cfac3ad3d4282) )
	ROM_LOAD64_WORD( "a3h.u51", 0x800006, 0x200000, CRC(834de39f) SHA1(6e9867180ca20e64f60bad5cad82674ce8f45b7b) )

	ROM_REGION( 0x080000, "tiles", ROMREGION_ERASE00 )
	// Use the tiles rom from livequiz (not present in this set) to show some debug text
//  ROM_LOAD( "27c4096.u23", 0x000000, 0x080000, CRC(25121de8) SHA1(edf24d87551639b871baf3243b452a4e2ba84107) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "s0.u25", 0x000000, 0x200000, CRC(c049a3e0) SHA1(0c7016c3128c170a84ad3f92fad1165775210e3d) )
	ROM_LOAD( "s1.u26", 0x200000, 0x200000, CRC(9cc4b3ec) SHA1(b91a8747074a1032eb7f70a015d394fe8e896d7e) )

	ROM_REGION( 0x10000, "spritegen:zoomy", ROMREGION_ERASE00 )
	/* uploaded */
ROM_END

} // anonymous namespace


GAME( 1999, livequiz, 0, livequiz, livequiz, midas_state,  init_livequiz, ROT0, "Andamiro", "Live Quiz Show", 0 )
GAME( 2000, hammer,   0, hammer,   hammer,   hammer_state, empty_init,    ROT0, "Andamiro", "Hammer",         0 )
