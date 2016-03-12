// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Round" hardware

    driver by Aaron Giles

    Games supported:
        * Off the Wall (1991) [2 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/offtwall.h"



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void offtwall_state::update_interrupts()
{
	m_maincpu->set_input_line(4, m_scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(6, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

MACHINE_RESET_MEMBER(offtwall_state,offtwall)
{
	atarigen_state::machine_reset();
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

WRITE16_MEMBER(offtwall_state::io_latch_w)
{
	/* lower byte */
	if (ACCESSING_BITS_0_7)
	{
		/* bit 4 resets the sound CPU */
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
		if (!(data & 0x10))
			m_jsa->reset();
	}

	logerror("sound control = %04X\n", data);
}



/*************************************
 *
 *  SLOOP workarounds
 *
 *************************************/


/*-------------------------------------------------------------------------

    Bankswitching

    Like the slapstic, the SoS bankswitches memory using A13 and A14.
    Unlike the slapstic, the exact addresses to trigger the bankswitch
    are unknown.

    Fortunately, Off the Wall uses a common routine for the important
    bankswitching. The playfield data is stored in the banked area of
    ROM, and by comparing the playfields to a real system, a mechanism
    to bankswitch at the appropriate time was discovered. Fortunately,
    it's really basic.

    OtW looks up the address to read playfield data from a table which
    is 58 words long. Word 0 assumes the bank is 0, word 1 assumes the
    bank is 1, etc. So we just trigger off of the table read and cause
    the bank to switch then.

    In addition, there is code which checksums longs from $40000 down to
    $3e000. The code wants that checksum to be $aaaa5555, but there is
    no obvious way for this to happen. To work around this, we watch for
    the final read from $3e000 and tweak the value such that the checksum
    will come out the $aaaa5555 magically.

-------------------------------------------------------------------------*/



READ16_MEMBER(offtwall_state::bankswitch_r)
{
	/* this is the table lookup; the bank is determined by the address that was requested */
	m_bank_offset = (offset & 3) * 0x1000;
	logerror("Bankswitch index %d -> %04X\n", offset, m_bank_offset);

	return m_bankswitch_base[offset];
}


READ16_MEMBER(offtwall_state::bankrom_r)
{
	/* this is the banked ROM read */
	logerror("%06X: %04X\n", space.device().safe_pcbase(), offset);

	/* if the values are $3e000 or $3e002 are being read by code just below the
	    ROM bank area, we need to return the correct value to give the proper checksum */
	if ((offset == 0x3000 || offset == 0x3001) && space.device().safe_pcbase() > 0x37000)
	{
		UINT32 checksum = (space.read_word(0x3fd210) << 16) | space.read_word(0x3fd212);
		UINT32 us = 0xaaaa5555 - checksum;
		if (offset == 0x3001)
			return us & 0xffff;
		else
			return us >> 16;
	}

	return m_bankrom_base[(m_bank_offset + offset) & 0x3fff];
}



/*-------------------------------------------------------------------------

    Sprite Cache

    Somewhere in the code, if all the hardware tests are met properly,
    some additional dummy sprites are added to the sprite cache before
    they are copied to sprite RAM. The sprite RAM copy routine computes
    the total width of all sprites as they are copied and if the total
    width is less than or equal to 38, it adds a "HARDWARE ERROR" sprite
    to the end.

    Here we detect the read of the sprite count from within the copy
    routine, and add some dummy sprites to the cache ourself if there
    isn't enough total width.

-------------------------------------------------------------------------*/


READ16_MEMBER(offtwall_state::spritecache_count_r)
{
	int prevpc = space.device().safe_pcbase();

	/* if this read is coming from $99f8 or $9992, it's in the sprite copy loop */
	if (prevpc == 0x99f8 || prevpc == 0x9992)
	{
		UINT16 *data = &m_spritecache_count[-0x100];
		int oldword = m_spritecache_count[0];
		int count = oldword >> 8;
		int i, width = 0;

		/* compute the current total width */
		for (i = 0; i < count; i++)
			width += 1 + ((data[i * 4 + 1] >> 4) & 7);

		/* if we're less than 39, keep adding dummy sprites until we hit it */
		if (width <= 38)
		{
			while (width <= 38)
			{
				data[count * 4 + 0] = (42 * 8) << 7;
				data[count * 4 + 1] = ((30 * 8) << 7) | (7 << 4);
				data[count * 4 + 2] = 0;
				width += 8;
				count++;
			}

			/* update the final count in memory */
			m_spritecache_count[0] = (count << 8) | (oldword & 0xff);
		}
	}

	/* and then read the data */
	return m_spritecache_count[offset];
}



/*-------------------------------------------------------------------------

    Unknown Verify

    In several places, the value 1 is stored to the byte at $3fdf1e. A
    fairly complex subroutine is called, and then $3fdf1e is checked to
    see if it was set to zero. If it was, "HARDWARE ERROR" is displayed.

    To avoid this, we just return 1 when this value is read within the
    range of PCs where it is tested.

-------------------------------------------------------------------------*/



READ16_MEMBER(offtwall_state::unknown_verify_r)
{
	int prevpc = space.device().safe_pcbase();
	if (prevpc < 0x5c5e || prevpc > 0xc432)
		return m_unknown_verify_base[offset];
	else
		return m_unknown_verify_base[offset] | 0x100;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, offtwall_state )
	AM_RANGE(0x000000, 0x037fff) AM_ROM
	AM_RANGE(0x038000, 0x03ffff) AM_READ(bankrom_r) AM_REGION("maincpu", 0x38000) AM_SHARE("bankrom_base")
	AM_RANGE(0x120000, 0x120fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0x00ff)
	AM_RANGE(0x260000, 0x260001) AM_READ_PORT("260000")
	AM_RANGE(0x260002, 0x260003) AM_READ_PORT("260002")
	AM_RANGE(0x260010, 0x260011) AM_READ_PORT("260010")
	AM_RANGE(0x260012, 0x260013) AM_READ_PORT("260012")
	AM_RANGE(0x260020, 0x260021) AM_READ_PORT("260020")
	AM_RANGE(0x260022, 0x260023) AM_READ_PORT("260022")
	AM_RANGE(0x260024, 0x260025) AM_READ_PORT("260024")
	AM_RANGE(0x260030, 0x260031) AM_DEVREAD8("jsa", atari_jsa_iii_device, main_response_r, 0x00ff)
	AM_RANGE(0x260040, 0x260041) AM_DEVWRITE8("jsa", atari_jsa_iii_device, main_command_w, 0x00ff)
	AM_RANGE(0x260050, 0x260051) AM_WRITE(io_latch_w)
	AM_RANGE(0x260060, 0x260061) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0x2a0000, 0x2a0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x3effc0, 0x3effff) AM_DEVREADWRITE("vad", atari_vad_device, control_read, control_write)
	AM_RANGE(0x3f4000, 0x3f5eff) AM_RAM_DEVWRITE("vad", atari_vad_device, playfield_latched_msb_w) AM_SHARE("vad:playfield")
	AM_RANGE(0x3f5f00, 0x3f5f7f) AM_RAM AM_SHARE("vad:eof")
	AM_RANGE(0x3f5f80, 0x3f5fff) AM_RAM AM_SHARE("vad:mob:slip")
	AM_RANGE(0x3f6000, 0x3f7fff) AM_RAM_DEVWRITE("vad", atari_vad_device, playfield_upper_w) AM_SHARE("vad:playfield_ext")
	AM_RANGE(0x3f8000, 0x3fcfff) AM_RAM
	AM_RANGE(0x3fd000, 0x3fd7ff) AM_RAM AM_SHARE("vad:mob")
	AM_RANGE(0x3fd800, 0x3fffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( offtwall )
	PORT_START("260000")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260002")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)

	PORT_START("260010")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0000, "Whirly-gigs" )   /* this is official Atari terminology! */
	PORT_DIPSETTING(      0x0002, "Joysticks" )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )   /* tested at a454 */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )   /* tested at a466 */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")  /* tested before writing to 260040 */
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260012")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260020")
	PORT_BIT( 0xff, 0, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260022")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260024")
	PORT_BIT( 0xff, 0, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static GFXDECODE_START( offtwall )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  256, 32 )      /* sprites & playfield */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( offtwall, offtwall_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_MACHINE_RESET_OVERRIDE(offtwall_state,offtwall)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", offtwall)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)

	MCFG_ATARI_VAD_ADD("vad", "screen", WRITELINE(atarigen_state, scanline_int_write_line))
	MCFG_ATARI_VAD_PLAYFIELD(offtwall_state, "gfxdecode", get_playfield_tile_info)
	MCFG_ATARI_VAD_MOB(offtwall_state::s_mob_config, "gfxdecode")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(offtwall_state, screen_update_offtwall)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(offtwall_state,offtwall)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_III_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("260010", 6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_DEVICE_REMOVE("jsa:oki1")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( offtwall )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "136090-2012.17e", 0x00000, 0x20000, CRC(d08d81eb) SHA1(5a72aa2e4fc6455b94aa59a7719d0ddc8bcc80f2) )
	ROM_LOAD16_BYTE( "136090-2013.17j", 0x00001, 0x20000, CRC(61c2553d) SHA1(343d39f9b75fd236e9769ec21ab65310f85e31ca) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136090-1020.12c", 0x00000, 0x10000, CRC(488112a5) SHA1(55e84855daacfa303d1031de8c9adb992a846e21) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136090-1014.14s", 0x000000, 0x20000, CRC(4d64507e) SHA1(cb2ac41aecd2702cd57c746a6f5986cd753bc29e) )
	ROM_LOAD( "136090-1016.14p", 0x020000, 0x20000, CRC(f5454f3a) SHA1(87d82bd227f7fcfd13b6f4ad88a573d1b96a4fc1) )
	ROM_LOAD( "136090-1018.14m", 0x040000, 0x20000, CRC(17864231) SHA1(22f93fcb5d413281157ab8545647f3713f98c135) )
	ROM_LOAD( "136090-1015.18s", 0x060000, 0x20000, CRC(271f7856) SHA1(928bc5e7dc589ceb5f55e536b5a05c3866116a24) )
	ROM_LOAD( "136090-1017.18p", 0x080000, 0x20000, CRC(7f7f8012) SHA1(1123ea3c6cd2c73617a87d6a5bbb26fca8941af3) )
	ROM_LOAD( "136090-1019.18m", 0x0a0000, 0x20000, CRC(9efe511b) SHA1(db1f1d8792bf497bc9ad652b0b7d78c3abf0e817) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "offtwall-eeprom.17l", 0x0000, 0x800, CRC(5eaf2d5b) SHA1(934a76a23960e6ed2cc33c359f9735caee762145) )

	ROM_REGION(0x022f, "jsa:plds", 0)
	ROM_LOAD("136085-1038.17c.bin", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */
	ROM_LOAD("136085-1039.20c.bin", 0x0118, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */

	ROM_REGION(0x2591, "main:plds", 0)
	ROM_LOAD("136090-1001.14l.bin", 0x0000, 0x201d, NO_DUMP ) /* GAL6001-35P is read protected */
	ROM_LOAD("136090-1002.11r.bin", 0x201e, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */
	ROM_LOAD("136090-1003.15f.bin", 0x2135, 0x0117, CRC(5e723b46) SHA1(e686920d0af342e33f836fec15b6e8b5ef1b8be5)) /* GAL16V8A-25LP */
	ROM_LOAD("136090-1005.5n.bin",  0x224c, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */
	ROM_LOAD("136090-1006.5f.bin",  0x2363, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */
	ROM_LOAD("136090-1007.3f.bin",  0x247a, 0x0117, NO_DUMP ) /* GAL16V8A-25LP is read protected */
ROM_END


ROM_START( offtwallc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 4*64k for 68000 code */
	ROM_LOAD16_BYTE( "090-2612.rom", 0x00000, 0x20000, CRC(fc891a3f) SHA1(027815a20fbc6c0c9242768581b97362b39941c2) )
	ROM_LOAD16_BYTE( "090-2613.rom", 0x00001, 0x20000, CRC(805d79d4) SHA1(943ec9f408ba875bdf1794ce7d24803043480401) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136090-1020.12c", 0x00000, 0x10000, CRC(488112a5) SHA1(55e84855daacfa303d1031de8c9adb992a846e21) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "090-1614.rom", 0x000000, 0x20000, CRC(307ed447) SHA1(acee15e58cd8def8e52a7586aa14240e1f8be319) )
	ROM_LOAD( "090-1616.rom", 0x020000, 0x20000, CRC(a5bd3d9b) SHA1(756d96eac2398dc68679b7641acbf0e79204eebb) )
	ROM_LOAD( "090-1618.rom", 0x040000, 0x20000, CRC(c7d9df5d) SHA1(d5e5fbb7faf42d865862b9ac60f94d20820b00f3) )
	ROM_LOAD( "090-1615.rom", 0x060000, 0x20000, CRC(ac3642c7) SHA1(bb57e039c113c4ce5702983c8e01dbe286d7b58e) )
	ROM_LOAD( "090-1617.rom", 0x080000, 0x20000, CRC(15208a89) SHA1(124484ab54959a1e6d9022a4f3ee4288a79c768b) )
	ROM_LOAD( "090-1619.rom", 0x0a0000, 0x20000, CRC(8a5d79b3) SHA1(0a202d20e6c86989ce2223e10eadf9009dd6ca8e) )

	ROM_REGION( 0x800, "eeprom:eeprom", 0 )
	ROM_LOAD( "offtwall-eeprom.17l", 0x0000, 0x800, CRC(5eaf2d5b) SHA1(934a76a23960e6ed2cc33c359f9735caee762145) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(offtwall_state,offtwall)
{
	/* install son-of-slapstic workarounds */
	m_spritecache_count = m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fde42, 0x3fde43, read16_delegate(FUNC(offtwall_state::spritecache_count_r),this));
	m_bankswitch_base = m_maincpu->space(AS_PROGRAM).install_read_handler(0x037ec2, 0x037f39, read16_delegate(FUNC(offtwall_state::bankswitch_r),this));
	m_unknown_verify_base = m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fdf1e, 0x3fdf1f, read16_delegate(FUNC(offtwall_state::unknown_verify_r),this));
}


DRIVER_INIT_MEMBER(offtwall_state,offtwalc)
{
	/* install son-of-slapstic workarounds */
	m_spritecache_count = m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fde42, 0x3fde43, read16_delegate(FUNC(offtwall_state::spritecache_count_r),this));
	m_bankswitch_base = m_maincpu->space(AS_PROGRAM).install_read_handler(0x037eca, 0x037f43, read16_delegate(FUNC(offtwall_state::bankswitch_r),this));
	m_unknown_verify_base = m_maincpu->space(AS_PROGRAM).install_read_handler(0x3fdf24, 0x3fdf25, read16_delegate(FUNC(offtwall_state::unknown_verify_r),this));
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, offtwall, 0,        offtwall, offtwall, offtwall_state, offtwall, ROT0, "Atari Games", "Off the Wall (2/3-player upright)", 0 )
GAME( 1991, offtwallc,offtwall, offtwall, offtwall, offtwall_state, offtwalc, ROT0, "Atari Games", "Off the Wall (2-player cocktail)", 0 )
