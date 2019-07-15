// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Chameleon 24

    driver by Mariusz Wojcieszek
    uses NES emulaton by Brad Olivier

    Notes:
    - NES hardware is probably implemented on FPGA
    - Atmel mcu probably controls coins and timer - since these are not emulated
      game is marked as 'not working'
    - 72-in-1 mapper (found on NES pirate carts) is used for bank switching
    - code at 0x0f8000 in 24-2.u2 contains English version of menu, code at 0x0fc000 contains
    other version (Asian language), is this controlled by mcu?

PCB is small and newly manufactured. There's 24 games which can be chosen
from a text menu after coin-up.
The games appear to be old NES games (i.e. very poor quality for an arcade product)
Screenshots can be found here....
http://www.coinopexpress.com/products/pcbs/pcb/Chameleon_24_2839.html

PCB Layout
----------


|------------------------------------|
|       LM380    --------            |
|                |NTA0002|           |
|                |(QFP80)|   24-1.U1 |
|                --------            |
|   2003        -----------          |
|              |LATTICE  |           |
|      DSW1    |PLSI 1016|           |
|J             |(PLCC44) |  24-2.U2  |
|A    AT89C51  -----------           |
|M                                   |
|M    SW1   21.4771MHz               |
|A                                   |
| GW6582  LS02                       |
|          |-----------| 4040        |
|  74HC245 |Phillps    | 4040        |
|          |SAA71111AH2|             |
|          |20505650   |             |
|          |bP0219     | 24-3.U3     |
| 24.576MHz|-----------|             |
|             (QFP64)                |
|------------------------------------|

Notes:
       All components are listed.
       DSW1 has 2 switches only
       SW1 is a push button switch
       U1 is 27C040 EPROM
       U2 is 27C080 EPROM
       U3 is 27C512 EPROM
*/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "video/ppu2c0x.h"
#include "screen.h"
#include "speaker.h"


class cham24_state : public driver_device
{
public:
	cham24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu") { }

	required_device<n2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;

	std::unique_ptr<uint8_t[]> m_nt_ram;
	uint8_t* m_nt_page[4];
	uint32_t m_in_0;
	uint32_t m_in_1;
	uint32_t m_in_0_shift;
	uint32_t m_in_1_shift;
	DECLARE_WRITE8_MEMBER(nt_w);
	DECLARE_READ8_MEMBER(nt_r);
	DECLARE_WRITE8_MEMBER(sprite_dma_w);
	DECLARE_READ8_MEMBER(cham24_IN0_r);
	DECLARE_WRITE8_MEMBER(cham24_IN0_w);
	DECLARE_READ8_MEMBER(cham24_IN1_r);
	DECLARE_WRITE8_MEMBER(cham24_mapper_w);
	void init_cham24();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void cham24_set_mirroring( int mirroring );
	void cham24(machine_config &config);
	void cham24_map(address_map &map);
};



void cham24_state::cham24_set_mirroring( int mirroring )
{
	switch(mirroring)
	{
	case PPU_MIRROR_LOW:
		m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram.get();
		break;
	case PPU_MIRROR_HIGH:
		m_nt_page[0] = m_nt_page[1] = m_nt_page[2] = m_nt_page[3] = m_nt_ram.get() + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		m_nt_page[0] = m_nt_ram.get();
		m_nt_page[1] = m_nt_ram.get();
		m_nt_page[2] = m_nt_ram.get() + 0x400;
		m_nt_page[3] = m_nt_ram.get() + 0x400;
		break;
	case PPU_MIRROR_VERT:
		m_nt_page[0] = m_nt_ram.get();
		m_nt_page[1] = m_nt_ram.get() + 0x400;
		m_nt_page[2] = m_nt_ram.get();
		m_nt_page[3] = m_nt_ram.get() + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		m_nt_page[0] = m_nt_ram.get();
		m_nt_page[1] = m_nt_ram.get() + 0x400;
		m_nt_page[2] = m_nt_ram.get() + 0x800;
		m_nt_page[3] = m_nt_ram.get() + 0xc00;
		break;
	}
}

WRITE8_MEMBER(cham24_state::nt_w)
{
	int page = ((offset & 0xc00) >> 10);
	m_nt_page[page][offset & 0x3ff] = data;
}

READ8_MEMBER(cham24_state::nt_r)
{
	int page = ((offset & 0xc00) >> 10);
	return m_nt_page[page][offset & 0x3ff];

}

WRITE8_MEMBER(cham24_state::sprite_dma_w)
{
	int source = (data & 7);
	m_ppu->spriteram_dma(space, source);
}

READ8_MEMBER(cham24_state::cham24_IN0_r)
{
	return ((m_in_0 >> m_in_0_shift++) & 0x01) | 0x40;
}

WRITE8_MEMBER(cham24_state::cham24_IN0_w)
{
	if (data & 0xfe)
	{
		//logerror("Unhandled cham24_IN0_w write: data = %02X\n", data);
	}

	if (data & 0x01)
	{
		return;
	}

	m_in_0_shift = 0;
	m_in_1_shift = 0;

	m_in_0 = ioport("P1")->read();
	m_in_1 = ioport("P2")->read();

}

READ8_MEMBER(cham24_state::cham24_IN1_r)
{
	return ((m_in_1 >> m_in_1_shift++) & 0x01) | 0x40;
}

WRITE8_MEMBER(cham24_state::cham24_mapper_w)
{
	uint32_t gfx_bank = offset & 0x3f;
	uint32_t prg_16k_bank_page = (offset >> 6) & 0x01;
	uint32_t prg_bank = (offset >> 7) & 0x1f;
	uint32_t prg_bank_page_size = (offset >> 12) & 0x01;
	uint32_t gfx_mirroring = (offset >> 13) & 0x01;

	uint8_t* dst = memregion("maincpu")->base();
	uint8_t* src = memregion("user1")->base();

	// switch PPU VROM bank
	membank("bank1")->set_base(memregion("gfx1")->base() + (0x2000 * gfx_bank));

	// set gfx mirroring
	cham24_set_mirroring(gfx_mirroring != 0 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

	// switch PRG bank
	if (prg_bank_page_size == 0)
	{
		// 32K
		memcpy(&dst[0x8000], &src[prg_bank * 0x8000], 0x8000);
	}
	else
	{
		if (prg_16k_bank_page == 1)
		{
			// upper half of 32K page
			memcpy(&dst[0x8000], &src[(prg_bank * 0x8000) + 0x4000], 0x4000);
			memcpy(&dst[0xC000], &src[(prg_bank * 0x8000) + 0x4000], 0x4000);
		}
		else
		{
			// lower half of 32K page
			memcpy(&dst[0x8000], &src[(prg_bank * 0x8000)], 0x4000);
			memcpy(&dst[0xC000], &src[(prg_bank * 0x8000)], 0x4000);
		}
	}
}

void cham24_state::cham24_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); /* NES RAM */
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(FUNC(cham24_state::sprite_dma_w));
	map(0x4016, 0x4016).rw(FUNC(cham24_state::cham24_IN0_r), FUNC(cham24_state::cham24_IN0_w));            /* IN0 - input port 1 */
	map(0x4017, 0x4017).r(FUNC(cham24_state::cham24_IN1_r));    /* IN1 - input port 2 / PSG second control register */
	map(0x8000, 0xffff).rom().w(FUNC(cham24_state::cham24_mapper_w));
}

static INPUT_PORTS_START( cham24 )
	PORT_START("P1") /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    /* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2") /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)    /* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END

void cham24_state::video_start()
{
}


void cham24_state::machine_start()
{
	/* need nametable ram, though. I doubt this uses more than 2k, but it starts up configured for 4 */
	m_nt_ram = std::make_unique<uint8_t[]>(0x1000);
}

void cham24_state::machine_reset()
{
	/* switch PRG rom */
	uint8_t* dst = memregion("maincpu")->base();
	uint8_t* src = memregion("user1")->base();

	memcpy(&dst[0x8000], &src[0x0f8000], 0x4000);
	memcpy(&dst[0xc000], &src[0x0f8000], 0x4000);

	/* uses 8K swapping, all ROM!*/
	m_ppu->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, "bank1");
	membank("bank1")->set_base(memregion("gfx1")->base());

	m_nt_page[0] = m_nt_ram.get();
	m_nt_page[1] = m_nt_ram.get() + 0x400;
	m_nt_page[2] = m_nt_ram.get() + 0x800;
	m_nt_page[3] = m_nt_ram.get() + 0xc00;

	/* and read/write handlers */
	m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff,read8_delegate(FUNC(cham24_state::nt_r), this), write8_delegate(FUNC(cham24_state::nt_w), this));
}

void cham24_state::init_cham24()
{
}

void cham24_state::cham24(machine_config &config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &cham24_state::cham24_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 262);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update("ppu", FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( cham24 )
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x100000, "user1", 0)
	ROM_LOAD( "24-2.u2", 0x000000, 0x100000, CRC(686e9d05) SHA1(a55b9850a4b47f1b4495710e71534ca0287b05ee) )

	ROM_REGION(0x080000, "gfx1", 0)
	ROM_LOAD( "24-1.u1", 0x000000, 0x080000, CRC(43c43d58) SHA1(3171befaca28acc80fb70226748d9abde76a1b56) )

	ROM_REGION(0x10000, "user2", 0)
	ROM_LOAD( "24-3.u3", 0x0000, 0x10000, CRC(e97955fa) SHA1(6d686c5d0967c9c2f40dbd8e6a0c0907606f2c7d) ) // unknown rom
ROM_END

GAME( 2002, cham24, 0, cham24, cham24, cham24_state, init_cham24, ROT0, "bootleg", "Chameleon 24", MACHINE_NOT_WORKING )
