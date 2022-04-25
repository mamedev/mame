// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/**************************************************************************************

    Sente Super System

    Preliminary driver by Mariusz Wojcieszek


    Notes:

    It's possible that the Moonquake set is actually a hardware diagnostic program
    rather than the game itself. The origin of the set and the state of the PCB
    from which it was dumped are unknown.

    * There are no references to Moonquake within the entire ROM data.

    * No code paths lead out of the test mode.

    * The non-test-mode data starts at 0xf03c3e. Curiously, there are no FF values
    within the data and large sections are repeated (see 0xf20000, 0xf40092, 0xf60023
    and 0xfa0046). Initially thought to be encrypted/compressed data, it may instead
    be randomly-generated data for testing the ROM banks.

    * ROMs 5L/5H are not present in a photo of a known-working PCB.

    * The ES5503 ROMs only contain speech for the sound bank tests.

    * The external interrupt (INT6) is related to the ES5503 but appears to be unused
    by the diagnostic program.

    * The internal program of the I/O MCU (68705) is undumped.

**************************************************************************************/


#include "emu.h"
#include "includes/amiga.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"
#include "sound/es5503.h"
#include "speaker.h"


class mquake_state : public amiga_state
{
public:
	mquake_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag)
		, m_es5503(*this, "es5503")
		, m_es5503_rom(*this, "es5503")
	{ }

	void mquake(machine_config &config);

	void init_mquake();

private:
	uint8_t es5503_sample_r(offs_t offset);
	void output_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t coin_chip_r(offs_t offset, uint16_t mem_mask = ~0);
	void coin_chip_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void a500_mem(address_map &map);
	void main_map(address_map &map);
	void mquake_es5503_map(address_map &map);
	void overlay_512kb_map(address_map &map);

	required_device<es5503_device> m_es5503;
	required_region_ptr<uint8_t> m_es5503_rom;
};




/*************************************
 *
 *  ES5503 access
 *
 *************************************/

uint8_t mquake_state::es5503_sample_r(offs_t offset)
{
	return m_es5503_rom[offset + (m_es5503->get_channel_strobe() * 0x10000)];
}

void mquake_state::mquake_es5503_map(address_map &map)
{
	map(0x000000, 0x1ffff).r(FUNC(mquake_state::es5503_sample_r));
}

void mquake_state::output_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		logerror("%06x:output_w(%x) = %02x\n", m_maincpu->pc(), offset, data);
}


uint16_t mquake_state::coin_chip_r(offs_t offset, uint16_t mem_mask)
{
	if (offset == 1)
		return ioport("COINCHIP")->read();
	logerror("%06x:coin_chip_r(%02x) & %04x\n", m_maincpu->pc(), offset, mem_mask);
	return 0xffff;
}

void mquake_state::coin_chip_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%06x:coin_chip_w(%02x) = %04x & %04x\n", m_maincpu->pc(), offset, data, mem_mask);
}

// inputs at 282000, 282002 (full word)
// outputs at 284000, 284002, 284004, 284006, 284008, 28400a, 28400c, 28400e (0=off, FF=on in LSB)
// coin chip I/O: read from 286002 (LSB), write to 28600A (low 4 bits)
//     write to 286008 (LSB) = F = reset?
// NVRAM at 200000-203FFF (both MSB and LSB)



/*************************************
 *
 *  Memory map
 *
 *************************************/

void mquake_state::overlay_512kb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).mirror(0x180000).ram().share("chip_ram");
	map(0x200000, 0x27ffff).rom().region("kickstart", 0);
}

void mquake_state::a500_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).m(m_overlay, FUNC(address_map_bank_device::amap16));
	map(0xa00000, 0xbfffff).rw(FUNC(mquake_state::cia_r), FUNC(mquake_state::cia_w));
	map(0xc00000, 0xd7ffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xd80000, 0xddffff).noprw();
	map(0xde0000, 0xdeffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xdf0000, 0xdfffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xe00000, 0xe7ffff).nopw().r(FUNC(mquake_state::rom_mirror_r));
	map(0xe80000, 0xefffff).noprw(); // autoconfig space (installed by devices)
	map(0xf80000, 0xffffff).rom().region("kickstart", 0);
}

void mquake_state::main_map(address_map &map)
{
	a500_mem(map);
	map(0x200000, 0x203fff).ram().share("nvram");
	map(0x204000, 0x2041ff).rw(m_es5503, FUNC(es5503_device::read), FUNC(es5503_device::write)).umask16(0x00ff);
	map(0x282000, 0x282001).portr("SW.LO");
	map(0x282002, 0x282003).portr("SW.HI");
	map(0x284000, 0x28400f).w(FUNC(mquake_state::output_w));
	map(0x286000, 0x28600f).rw(FUNC(mquake_state::coin_chip_r), FUNC(mquake_state::coin_chip_w));
	map(0x300000, 0x3bffff).rom().region("user2", 0);
	map(0xf00000, 0xfbffff).rom().region("user2", 0);           /* Custom ROM */
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mquake )
	PORT_START("CIA0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)         /* JS0SW */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)         /* JS1SW */

	PORT_START("joy_0_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(mquake_state, amiga_joystick_convert<0>)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_1_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(mquake_state, amiga_joystick_convert<1>)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("p1_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("COINCHIP")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW.LO")
	PORT_DIPNAME( 0x0001, 0x0001, "SW3.1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "SW3.2" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "SW3.3" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "SW3.4" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "SW2.1" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "SW2.2" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "SW2.3" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "SW2.4" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "SW1.1" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "SW1.2" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "SW1.3" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "SW1.4" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "I/O SW00" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "I/O SW01" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "I/O SW02" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SW.HI")
	PORT_DIPNAME( 0x0001, 0x0001, "SW3.5" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "SW3.6" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "SW3.7" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "SW3.8" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "SW2.5" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "SW2.6" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "SW2.7" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "SW2.8" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "SW1.5" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "SW1.6" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "SW1.7" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "SW1.8" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "I/O SW20" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "I/O SW21" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "I/O SW22" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "I/O SW23" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END




/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mquake_state::mquake(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, amiga_state::CLK_7M_NTSC);
	m_maincpu->set_addrmap(AS_PROGRAM, &mquake_state::main_map);

	ADDRESS_MAP_BANK(config, m_overlay).set_map(&mquake_state::overlay_512kb_map).set_options(ENDIANNESS_BIG, 16, 22, 0x200000);
	ADDRESS_MAP_BANK(config, m_chipset).set_map(&mquake_state::ocs_map).set_options(ENDIANNESS_BIG, 16, 9, 0x200);

	AMIGA_COPPER(config, m_copper, amiga_state::CLK_7M_NTSC);
	m_copper->set_host_cpu_tag(m_maincpu);
	m_copper->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_copper->set_ecs_mode(false);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	ntsc_video(config);

	PALETTE(config, m_palette, FUNC(mquake_state::amiga_palette), 4096);

	MCFG_VIDEO_START_OVERRIDE(mquake_state,amiga)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	PAULA_8364(config, m_paula, amiga_state::CLK_C1_NTSC);
	m_paula->add_route(0, "lspeaker", 0.50);
	m_paula->add_route(1, "rspeaker", 0.50);
	m_paula->add_route(2, "rspeaker", 0.50);
	m_paula->add_route(3, "lspeaker", 0.50);
	m_paula->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_paula->int_cb().set(FUNC(amiga_state::paula_int_w));

	ES5503(config, m_es5503, amiga_state::CLK_7M_NTSC); /* ES5503 is likely mono due to channel strobe used as bank select */
	m_es5503->set_channels(1);
	m_es5503->set_addrmap(0, &mquake_state::mquake_es5503_map);
	m_es5503->add_route(0, "lspeaker", 0.50);
	m_es5503->add_route(0, "rspeaker", 0.50);

	/* cia */
	MOS8520(config, m_cia_0, amiga_state::CLK_E_NTSC);
	m_cia_0->irq_wr_callback().set(FUNC(amiga_state::cia_0_irq));
	m_cia_0->pa_rd_callback().set_ioport("CIA0PORTA");
	m_cia_0->pa_wr_callback().set(FUNC(amiga_state::cia_0_port_a_write));
	MOS8520(config, m_cia_1, amiga_state::CLK_E_NTSC);
	m_cia_1->irq_wr_callback().set(FUNC(amiga_state::cia_1_irq));

	/* fdc */
	AMIGA_FDC(config, m_fdc, amiga_state::CLK_7M_NTSC);
	m_fdc->index_callback().set("cia_1", FUNC(mos8520_device::flag_w));
	m_fdc->read_dma_callback().set(FUNC(amiga_state::chip_ram_r));
	m_fdc->write_dma_callback().set(FUNC(amiga_state::chip_ram_w));
	m_fdc->dskblk_callback().set(FUNC(amiga_state::fdc_dskblk_w));
	m_fdc->dsksyn_callback().set(FUNC(amiga_state::fdc_dsksyn_w));
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mquake )
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROM_LOAD16_WORD("315093-01.u2", 0x00000, 0x40000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88))
	ROM_COPY("kickstart", 0x00000, 0x40000, 0x40000)

	ROM_REGION16_BE(0xc0000, "user2", 0)
	ROM_LOAD16_BYTE( "rom0l.bin",    0x00001, 0x10000, CRC(60c35ec3) SHA1(84fe88af54903cbd46044ef52bb50e8f94a94dcd) )
	ROM_LOAD16_BYTE( "rom0h.bin",    0x00000, 0x10000, CRC(11551a68) SHA1(bc17e748cc7a4a547de230431ea08f0355c0eec8) )
	ROM_LOAD16_BYTE( "rom1l.bin",    0x20001, 0x10000, CRC(0128c423) SHA1(b0465069452bd11b67c9a2f2b9021c91788bedbb) )
	ROM_LOAD16_BYTE( "rom1h.bin",    0x20000, 0x10000, CRC(95119e65) SHA1(29f3c32ca110c9687f38fd03ccb979c1e7c7a87e) )
	ROM_LOAD16_BYTE( "rom2l.bin",    0x40001, 0x10000, CRC(f8b8624a) SHA1(cb769581f78882a950be418dd4b35bbb6fd78a34) )
	ROM_LOAD16_BYTE( "rom2h.bin",    0x40000, 0x10000, CRC(46e36e0d) SHA1(0813430137a31d5af2cadbd712a418e9ff339a21) )
	ROM_LOAD16_BYTE( "rom3l.bin",    0x60001, 0x10000, CRC(c00411a2) SHA1(960d3539914f587c2186ec6eefb81b3cdd9325a0) )
	ROM_LOAD16_BYTE( "rom3h.bin",    0x60000, 0x10000, CRC(4540c681) SHA1(cb0bc6dc506ed0c9561687964e57299a472c5cd8) )
	ROM_LOAD16_BYTE( "rom4l.bin",    0x80001, 0x10000, CRC(f48d0730) SHA1(703a8ed47f64b3824bc6e5a4c5bdb2895f8c3d37) )
	ROM_LOAD16_BYTE( "rom4h.bin",    0x80000, 0x10000, CRC(eee39fec) SHA1(713e24fa5f4ba0a8bc7bf67ed2d9e079fd3aa5d6) )
	ROM_LOAD16_BYTE( "rom5l.bin",    0xa0001, 0x10000, CRC(7b6ec532) SHA1(e19005269673134431eb55053d650f747f614b89) )
	ROM_LOAD16_BYTE( "rom5h.bin",    0xa0000, 0x10000, CRC(ed8ec9b7) SHA1(510416bc88382e7a548635dcba53a2b615272e0f) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705.bin", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION(0x040000, "es5503", 0)
	ROM_LOAD( "qrom0.bin",    0x000000, 0x010000, CRC(753e29b4) SHA1(4c7ccff02d310c7c669aa170e8efb6f2cb996432) )
	ROM_LOAD( "qrom1.bin",    0x010000, 0x010000, CRC(e9e15629) SHA1(a0aa60357a13703f69a2a13e83f2187c9a1f63c1) )
	ROM_LOAD( "qrom2.bin",    0x020000, 0x010000, CRC(837294f7) SHA1(99e383998105a63896096629a51b3a0e9eb16b17) )
	ROM_LOAD( "qrom3.bin",    0x030000, 0x010000, CRC(530fd1a9) SHA1(e3e5969f0880de0a6cdb443a82b85d34ab8ff4f8) )
ROM_END




/*************************************
 *
 *  Driver init
 *
 *************************************/

void mquake_state::init_mquake()
{
	m_agnus_id = AGNUS_HR_NTSC;
	m_denise_id = DENISE;
}




/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1987, mquake, 0, mquake, mquake, mquake_state, init_mquake, 0, "Sente", "Moonquake", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
