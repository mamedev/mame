// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
HotBlock board

Tetris with naughty bits

        ||||||||||||||||
+-------++++++++++++++++-------+
|                              |
|  YM2149 TESTSW               |
|                              |
|    62256 62256   6116 6116   |
|                              |
|    24mhz  TPC1020AFN 24c04a  |
|                              |
|                     PAL      |
| P8088-1 IC4 IC5 62256 62256  |
|                              |
+------------------------------+

330ohm resistor packs for colours


--

there are a variety of test modes which can be obtained
by resetting while holding down player 2 buttons

eeprom / backup data not hooked up ( 24c04a on port4 )

most sources say this is a game by Nics but I believe Nics
to be a company from Korea, this game is quite clearly a
Spanish game, we know for a fact that NIX are from Spain
so it could be by them instead



*/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/bankdev.h"
//#include "machine/i2cmem.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class hotblock_state : public driver_device
{
public:
	hotblock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_video_bank(*this, "video_bank")/*,
		m_i2cmem(*this, "i2cmem")*/,
		m_vram(*this, "vram")
	{ }

	void hotblock(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_video_bank;
//  required_device<i2cmem_device> m_i2cmem;

	/* misc */
	int      m_port0;
	int      m_port4;

	/* memory */
	required_shared_ptr<uint8_t> m_vram;

	u8 port4_r();
	void port4_w(u8 data);
	void port0_w(u8 data);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void banked_video_map(address_map &map);
	void hotblock_io(address_map &map);
	void hotblock_map(address_map &map);
};


/* port 4 is some kind of eeprom / storage .. used to store the scores */
u8 hotblock_state::port4_r()
{
//  osd_printf_debug("port4_r\n");
	//logerror("trying to read port 4 at maincpu pc %08x\n", m_maincpu->pc());
	//return (m_i2cmem->read_sda() & 1);
	return 0x00;
}


void hotblock_state::port4_w(u8 data)
{
	//logerror("trying to write port 4 in %02x at maincpu pc %08x\n", data, m_maincpu->pc());
	m_port4 = data;
}


void hotblock_state::port0_w(u8 data)
{
	m_port0 = data;
	m_video_bank->set_bank(m_port0 & ~0x40);
}

void hotblock_state::hotblock_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).m(m_video_bank, FUNC(address_map_bank_device::amap8));
	map(0x20000, 0xfffff).rom();
}

void hotblock_state::hotblock_io(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(hotblock_state::port0_w));
	map(0x0004, 0x0004).rw(FUNC(hotblock_state::port4_r), FUNC(hotblock_state::port4_w));
	map(0x8000, 0x8001).w("aysnd", FUNC(ym2149_device::address_data_w));
	map(0x8001, 0x8001).r("aysnd", FUNC(ym2149_device::data_r));
}

/* right?, anything else?? */
void hotblock_state::banked_video_map(address_map &map)
{
	map(0x000000, 0x00ffff).mirror(0x9f0000).ram().share("vram"); // port 0 = 88 c8
	map(0x200000, 0x2001ff).mirror(0x9f0000).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // port 0 = a8 e8 -- palette
}


void hotblock_state::video_start()
{
	save_item(NAME(m_port0));
	save_item(NAME(m_port4)); //stored but not read for now
}

uint32_t hotblock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (~m_port0 & 0x40)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int count = (y * 320) + cliprect.left();
		for(int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			bitmap.pix16(y, x) = m_vram[count++];
		}
	}

	return 0;
}


static INPUT_PORTS_START( hotblock )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // unused?

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // used to get test mode
INPUT_PORTS_END


void hotblock_state::hotblock(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 24_MHz_XTAL / 3); // Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &hotblock_state::hotblock_map);
	m_maincpu->set_addrmap(AS_IO, &hotblock_state::hotblock_io);

//  I2CMEM(config, m_i2cmem, 0).set_page_size(16).set_data_size(0x200); // 24C04
	ADDRESS_MAP_BANK(config, m_video_bank).set_map(&hotblock_state::banked_video_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x10000);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 3, 512, 0, 320, 312, 0, 200); // 15.625 kHz horizontal???
	screen.set_screen_update(FUNC(hotblock_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI); // right?

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200/2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 24_MHz_XTAL / 24));
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( hotblock )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "hotblk5.ic4", 0x000000, 0x080000, CRC(5f90f776) SHA1(5ca74714a7d264b4fafaad07dc11e57308828d30) )
	ROM_LOAD( "hotblk6.ic5", 0x080000, 0x080000, CRC(3176d231) SHA1(ac22fd0e9820c6714f51a3d8315eb5d43ef91eeb) )
ROM_END

GAME( 1993, hotblock, 0, hotblock, hotblock, hotblock_state, empty_init, ROT0, "NIX?", "Hot Blocks - Tetrix II", MACHINE_SUPPORTS_SAVE )
