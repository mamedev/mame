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
#include "cpu/m68000/m68000.h"
#include "includes/amiga.h"
#include "sound/es5503.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"


class mquake_state : public amiga_state
{
public:
	mquake_state(const machine_config &mconfig, device_type type, const char *tag)
	: amiga_state(mconfig, type, tag),
	m_es5503(*this, "es5503"),
	m_es5503_rom(*this, "es5503")
	{ }

	DECLARE_DRIVER_INIT(mquake);

	DECLARE_READ8_MEMBER( es5503_sample_r );
	DECLARE_WRITE16_MEMBER( output_w );
	DECLARE_READ16_MEMBER( coin_chip_r );
	DECLARE_WRITE16_MEMBER( coin_chip_w );

private:
	required_device<es5503_device> m_es5503;
	required_region_ptr<UINT8> m_es5503_rom;
};




/*************************************
 *
 *  ES5503 access
 *
 *************************************/

READ8_MEMBER( mquake_state::es5503_sample_r )
{
	return m_es5503_rom[offset + (m_es5503->get_channel_strobe() * 0x10000)];
}

static ADDRESS_MAP_START( mquake_es5503_map, AS_0, 8, mquake_state )
	AM_RANGE(0x000000, 0x1ffff) AM_READ(es5503_sample_r)
ADDRESS_MAP_END

WRITE16_MEMBER( mquake_state::output_w )
{
	if (ACCESSING_BITS_0_7)
		logerror("%06x:output_w(%x) = %02x\n", space.device().safe_pc(), offset, data);
}


READ16_MEMBER( mquake_state::coin_chip_r )
{
	if (offset == 1)
		return ioport("COINCHIP")->read();
	logerror("%06x:coin_chip_r(%02x) & %04x\n", space.device().safe_pc(), offset, mem_mask);
	return 0xffff;
}

WRITE16_MEMBER( mquake_state::coin_chip_w )
{
	logerror("%06x:coin_chip_w(%02x) = %04x & %04x\n", space.device().safe_pc(), offset, data, mem_mask);
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

static ADDRESS_MAP_START( overlay_512kb_map, AS_PROGRAM, 16, mquake_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_MIRROR(0x180000) AM_RAM AM_SHARE("chip_ram")
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( a500_mem, AS_PROGRAM, 16, mquake_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_DEVICE("overlay", address_map_bank_device, amap16)
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0xc00000, 0xd7ffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xd80000, 0xddffff) AM_NOP
	AM_RANGE(0xde0000, 0xdeffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xdf0000, 0xdfffff) AM_READWRITE(custom_chip_r, custom_chip_w)
	AM_RANGE(0xe00000, 0xe7ffff) AM_WRITENOP AM_READ(rom_mirror_r)
	AM_RANGE(0xe80000, 0xefffff) AM_NOP // autoconfig space (installed by devices)
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("kickstart", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, mquake_state )
	AM_IMPORT_FROM(a500_mem)
	AM_RANGE(0x200000, 0x203fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x204000, 0x2041ff) AM_DEVREADWRITE8("es5503", es5503_device, read, write, 0x00ff)
	AM_RANGE(0x282000, 0x282001) AM_READ_PORT("SW.LO")
	AM_RANGE(0x282002, 0x282003) AM_READ_PORT("SW.HI")
	AM_RANGE(0x284000, 0x28400f) AM_WRITE(output_w)
	AM_RANGE(0x286000, 0x28600f) AM_READWRITE(coin_chip_r, coin_chip_w)
	AM_RANGE(0x300000, 0x3bffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xf00000, 0xfbffff) AM_ROM AM_REGION("user2", 0)           /* Custom ROM */
ADDRESS_MAP_END



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
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mquake_state,amiga_joystick_convert, (void *)0)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_1_dat")
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mquake_state,amiga_joystick_convert, (void *)1)
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

static MACHINE_CONFIG_START( mquake, mquake_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, amiga_state::CLK_7M_NTSC)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_DEVICE_ADD("overlay", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(overlay_512kb_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(22)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x200000)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_FRAGMENT_ADD(ntsc_video)

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_INIT_OWNER(mquake_state,amiga)

	MCFG_VIDEO_START_OVERRIDE(mquake_state,amiga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, amiga_state::CLK_C1_NTSC)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(3, "lspeaker", 0.50)

	MCFG_ES5503_ADD("es5503", amiga_state::CLK_7M_NTSC)       /* ES5503 is likely mono due to channel strobe used as bank select */
	MCFG_ES5503_OUTPUT_CHANNELS(1)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mquake_es5503_map)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.50)

	/* cia */
	MCFG_DEVICE_ADD("cia_0", MOS8520, amiga_state::CLK_E_NTSC)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_0_irq))
	MCFG_MOS6526_PA_INPUT_CALLBACK(IOPORT("CIA0PORTA"))
	MCFG_MOS6526_PA_OUTPUT_CALLBACK(WRITE8(amiga_state, cia_0_port_a_write))
	MCFG_DEVICE_ADD("cia_1", MOS8520, amiga_state::CLK_E_NTSC)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(amiga_state, cia_1_irq))

	/* fdc */
	MCFG_DEVICE_ADD("fdc", AMIGA_FDC, amiga_state::CLK_7M_NTSC)
	MCFG_AMIGA_FDC_INDEX_CALLBACK(DEVWRITELINE("cia_1", mos8520_device, flag_w))
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mquake )
	ROM_REGION(0x80000, "kickstart", 0)
	ROM_LOAD16_WORD_SWAP("315093-01.u2", 0x00000, 0x40000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88))
	ROM_COPY("kickstart", 0x00000, 0x40000, 0x40000)

	ROM_REGION(0xc0000, "user2", 0)
	ROM_LOAD16_BYTE( "rom0l.bin",    0x00000, 0x10000, CRC(60c35ec3) SHA1(84fe88af54903cbd46044ef52bb50e8f94a94dcd) )
	ROM_LOAD16_BYTE( "rom0h.bin",    0x00001, 0x10000, CRC(11551a68) SHA1(bc17e748cc7a4a547de230431ea08f0355c0eec8) )
	ROM_LOAD16_BYTE( "rom1l.bin",    0x20000, 0x10000, CRC(0128c423) SHA1(b0465069452bd11b67c9a2f2b9021c91788bedbb) )
	ROM_LOAD16_BYTE( "rom1h.bin",    0x20001, 0x10000, CRC(95119e65) SHA1(29f3c32ca110c9687f38fd03ccb979c1e7c7a87e) )
	ROM_LOAD16_BYTE( "rom2l.bin",    0x40000, 0x10000, CRC(f8b8624a) SHA1(cb769581f78882a950be418dd4b35bbb6fd78a34) )
	ROM_LOAD16_BYTE( "rom2h.bin",    0x40001, 0x10000, CRC(46e36e0d) SHA1(0813430137a31d5af2cadbd712a418e9ff339a21) )
	ROM_LOAD16_BYTE( "rom3l.bin",    0x60000, 0x10000, CRC(c00411a2) SHA1(960d3539914f587c2186ec6eefb81b3cdd9325a0) )
	ROM_LOAD16_BYTE( "rom3h.bin",    0x60001, 0x10000, CRC(4540c681) SHA1(cb0bc6dc506ed0c9561687964e57299a472c5cd8) )
	ROM_LOAD16_BYTE( "rom4l.bin",    0x80000, 0x10000, CRC(f48d0730) SHA1(703a8ed47f64b3824bc6e5a4c5bdb2895f8c3d37) )
	ROM_LOAD16_BYTE( "rom4h.bin",    0x80001, 0x10000, CRC(eee39fec) SHA1(713e24fa5f4ba0a8bc7bf67ed2d9e079fd3aa5d6) )
	ROM_LOAD16_BYTE( "rom5l.bin",    0xa0000, 0x10000, CRC(7b6ec532) SHA1(e19005269673134431eb55053d650f747f614b89) )
	ROM_LOAD16_BYTE( "rom5h.bin",    0xa0001, 0x10000, CRC(ed8ec9b7) SHA1(510416bc88382e7a548635dcba53a2b615272e0f) )

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

DRIVER_INIT_MEMBER( mquake_state, mquake )
{
	m_agnus_id = AGNUS_HR_NTSC;
	m_denise_id = DENISE;
}




/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1987, mquake, 0, mquake, mquake, mquake_state, mquake, 0, "Sente", "Moonquake", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
