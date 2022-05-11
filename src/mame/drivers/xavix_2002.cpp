// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************/

/* SuperXaviX (XaviX 2002 type CPU) hardware titles (3rd XaviX generation?)

  these use the SSD 2002 NEC 85054-611 type CPU
  differences include support for 16-bit ROMs, high resolution bitmap modes, interlace screen modes, extra IO

   XavixPort Golf is "SSD 2003 SuperXaviX MXIC 2003 3009" (not dumped yet, but actually marked as SuperXaviX unlike the others!)

*/

#include "emu.h"
#include "includes/xavix_2002.h"

// #define VERBOSE 1
#include "logmacro.h"

/* The 'XaviXPORT' isn't a real console, more of a TV adapter, all the actual hardware (CPU including video hw, sound hw) is in the cartridges and controllers
   and can vary between games, see notes at top of driver.

   The 'Domyos Interactive System (DiS)' released in France by Decathlon appears to be identical to XaviXPORT (but for PAL regions, and with an entirely different software range)
*/


static INPUT_PORTS_START( xavix )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("AN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN5")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MOUSE0X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE0Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1X")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("MOUSE1Y")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_i2c )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
INPUT_PORTS_END


static INPUT_PORTS_START( epo_tfit )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // select
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // back
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
INPUT_PORTS_END


static INPUT_PORTS_START( mrangbat )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("EX0") // NOT A JOYSTICK!!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY // Red/Up 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_16WAY // Red/Up 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_16WAY // Green / Circle / Right 2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_16WAY // Pink / Star / Left 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY // Blue / Square / Right 1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY // Yellow / Triangle / Left 1
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 )


	PORT_START("EX1")
	PORT_DIPNAME( 0x01, 0x00, "EX1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("EX2")
	PORT_DIPNAME( 0x01, 0x00, "EX2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_bowl )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xavix_i2c_bowl_state, camera_r)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(xavix_i2c_bowl_state, camera_r)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
INPUT_PORTS_END

static INPUT_PORTS_START( xavixp )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END


/* SuperXavix IO port handliner (per game) */

uint8_t xavix_i2c_jmat_state::read_extended_io0()
{
	LOG("%s: read_extended_io0\n", machine().describe_context());
	return 0x00;
}

uint8_t xavix_i2c_jmat_state::read_extended_io1()
{
	LOG("%s: read_extended_io1\n", machine().describe_context());

	// reads this by reading the byte, then shifting right 4 times to place value into carry flag
	return m_i2cmem->read_sda() << 3;
	//return 0x00;
}

uint8_t xavix_i2c_jmat_state::read_extended_io2()
{
	LOG("%s: read_extended_io2\n", machine().describe_context());
	return 0x00;
}

void xavix_i2c_jmat_state::write_extended_io0(uint8_t data)
{
	LOG("%s: io0_data_w %02x\n", machine().describe_context(), data);
}

void xavix_i2c_jmat_state::write_extended_io1(uint8_t data)
{
	LOG("%s: io1_data_w %02x\n", machine().describe_context(), data);

	m_i2cmem->write_sda((data & 0x08) >> 3);
	m_i2cmem->write_scl((data & 0x10) >> 4);

}

void xavix_i2c_jmat_state::write_extended_io2(uint8_t data)
{
	LOG("%s: io2_data_w %02x\n", machine().describe_context(), data);
}

READ_LINE_MEMBER(xavix_i2c_lotr_state::camera_r) // seems to be some kind of camera status bits
{
	return machine().rand();
}

READ_LINE_MEMBER(xavix_i2c_bowl_state::camera_r) // seems to be some kind of camera status bits
{
	return machine().rand();
}

void xavix_state::xavix2002(machine_config &config)
{
	xavix(config);

	XAVIX2002(config.replace(), m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &xavix_state::xavix_map);
	m_maincpu->set_addrmap(5, &xavix_state::superxavix_lowbus_map); // has extra video, io etc.
	m_maincpu->set_addrmap(6, &xavix_state::xavix_extbus_map);
	m_maincpu->set_vblank_int("screen", FUNC(xavix_state::interrupt));
	m_maincpu->set_vector_callback(FUNC(xavix_state::get_vectors));

	m_palette->set_entries(512);

	XAVIX2002IO(config, m_xavix2002io, 0);
}



void xavix_i2c_jmat_state::xavix2002_i2c_jmat(machine_config &config)
{
	xavix2002_i2c_24c08(config);

	m_xavix2002io->read_0_callback().set(FUNC(xavix_i2c_jmat_state::read_extended_io0));
	m_xavix2002io->write_0_callback().set(FUNC(xavix_i2c_jmat_state::write_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(xavix_i2c_jmat_state::read_extended_io1));
	m_xavix2002io->write_1_callback().set(FUNC(xavix_i2c_jmat_state::write_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(xavix_i2c_jmat_state::read_extended_io2));
	m_xavix2002io->write_2_callback().set(FUNC(xavix_i2c_jmat_state::write_extended_io2));
}

void xavix2002_superpttv_state::xavix2002_superpctv(machine_config& config)
{
	xavix2002(config);

	m_xavix2002io->read_0_callback().set(FUNC(xavix2002_superpttv_state::read_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(xavix2002_superpttv_state::read_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(xavix2002_superpttv_state::read_extended_io2));
}


void xavix_i2c_state::xavix2002_i2c_24c08(machine_config &config)
{
	xavix2002(config);

	I2C_24C08(config, "i2cmem", 0);
}

void xavix_i2c_state::xavix2002_i2c_24c04(machine_config &config)
{
	xavix2002(config);

	I2C_24C04(config, "i2cmem", 0);
}

void xavix_i2c_state::xavix2002_i2c_mrangbat(machine_config &config)
{
	xavix2002(config);

	I2C_24C02(config, "i2cmem", 0); // 24C02?

	m_xavix2002io->read_0_callback().set_ioport("EX0");
	m_xavix2002io->read_1_callback().set_ioport("EX1");
	m_xavix2002io->read_2_callback().set_ioport("EX2");
}


// XaviXPORT
ROM_START( xavtenni )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xavixtennis.bin", 0x000000, 0x800000, CRC(23a1d918) SHA1(2241c59e8ea8328013e55952ebf9060ea0a4675b) )
ROM_END

ROM_START( xavbaseb )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbaseball.bin", 0x000000, 0x800000, CRC(e9ed692d) SHA1(537e390e972156dc7da66ee127ae4c8052038ee5) )
ROM_END

ROM_START( xavbowl )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbowling.bin", 0x000000, 0x800000, CRC(2873460b) SHA1(ea8e2392f5a12961a23eb66dca8e07dec81ce8c8) )
ROM_END

ROM_START( xavbassf )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbassfishing.bin", 0x000000, 0x800000, CRC(09ab2f29) SHA1(616254176315d0947002e9ae5a6371a3ffa2e8eb) )

	// code for the nRF24E1s, stored in SEEPROMs.  One in the cartridge, one in the rod/reel
	ROM_REGION( 0x1001, "reel_io", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbassfishingnrf24e1reel.bin", 0x0000, 0x1001, CRC(cfbb19ae) SHA1(32464e4e4be33fdbc7768311f93ce437a316c616) )

	ROM_REGION( 0x800000, "base_io", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbassfishingnrf24e1cart.bin", 0x0000, 0x1001, CRC(62f6303e) SHA1(126b2663e252fb80948f53153e4046e63dd8be32) )
ROM_END

ROM_START( xavbox )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpboxing.bin", 0x000000, 0x800000, CRC(b61e7717) SHA1(162b9c53ac8c9d7b6972db44f7bc1cb0a7837b70) )
ROM_END

// Several of the XaviXport and DiS games are 2 glob setups (and must have some kind of banking)

ROM_START( xavjmat )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(1420640d) SHA1(dd714cd57cff885293688f74f69b5c1726e20ec0) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(52dc318c) SHA1(dc50e0747ba29cfb1048fd4a55d26870086c869b) )
ROM_END

// currently copies the wrong code into RAM to execute (due to extended ROM size, and possible banking)
// [:] ':maincpu' (00E074): rom_dmatrg_w (do DMA?) 01
// [:]   (possible DMA op SRC 00ebe2d3 DST 358a LEN 0398)
//         needs to come from 006be2d3 (so still from lower 8MB, not upper 8MB)

ROM_START( xavmusic )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(e7c8ad59) SHA1(d47fac8b480de4db88a1b306ff8830a65d1738a3) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(977c956f) SHA1(debc086d0cf6c391002ad163e7bfaa2f010cc8f5) )
ROM_END


// Domyos DiS (XaviX 2002 based titles)
ROM_START( domfitex )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000,  CRC(841fe3cd) SHA1(8678b8a0c5198b24169a84dbe3ae979bb0838f23) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(1dc844ea) SHA1(c23da9006227f7fe4982998c17759d403a47472a) )
ROM_END

ROM_START( domfitch )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(0ff2a7a6) SHA1(9b924cc4330e3f8d9204390854048fe2325bfdf7) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(284583f6) SHA1(bd2d5304f1e01eed656b5de957ec0a0330a3d969) )
ROM_END

ROM_START( domdance )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(74f9499d) SHA1(a64235075e32567cd6d2ab7b1284efcb8e7538e2) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(e437565c) SHA1(f6db219ea14404b698ca453f6e50c726b2e77abb) )
ROM_END

ROM_START( domstepc )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(cb37b5e9) SHA1(b742e3db98f36720adf5af9096c6bc235279de12) )

	ROM_REGION( 0x0800000, "biosu3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(dadaa744) SHA1(fd7ca77232a8fe228fc93b0a8a47ba3260349d90) )
ROM_END

ROM_START( mrangbat )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("powerrangerspad.bin", 0x000000, 0x400000, CRC(d3a98775) SHA1(485c66242dd0ee436a278d23005aece48d606431) )
ROM_END

ROM_START( tmy_thom )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "thomastank.bin", 0x000000, 0x800000, CRC(a52a23be) SHA1(e5b3500239d9e56eb5405f7585982959e5a162da) )
ROM_END

ROM_START( epo_tfit )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("tennisfitness.bin", 0x000000, 0x400000, CRC(cbf65bd2) SHA1(30b3da6f061b2dd91679db42a050f715901beb87) )
ROM_END

ROM_START( suprpctv )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00) // inverted line?
	ROM_LOAD("superpctv.bin", 0x200000, 0x200000, CRC(4a55a81c) SHA1(178b4b595a3aefc6d1c176031b436fc3312009e7) )
	ROM_CONTINUE(0x000000, 0x200000)
	ROM_CONTINUE(0x600000, 0x200000)
	ROM_CONTINUE(0x400000, 0x200000)
ROM_END

ROM_START( udance )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("udancerom0.bin", 0x000000, 0x800000, CRC(3066580a) SHA1(545257c75a892894faf386f4ab9a31967cdbe8ae) )

	ROM_REGION(0x800000, "biosx", ROMREGION_ERASE00)
	ROM_LOAD("udancerom1.bin", 0x000000, 0x800000, CRC(7dbaabde) SHA1(38c523dcdf8185465fc550fb9b0e8c7909f839be) )
ROM_END


CONS( 2004, xavtenni, 0, 0, xavix2002_i2c_24c04, xavix_i2c,  xavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Tennis (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, xavbaseb, 0, 0, xavix2002_i2c_24c08, xavix_i2c,  xavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Baseball (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, xavbowl,  0, 0, xavix2002_i2c_24c04, xavix_bowl, xavix_i2c_bowl_state, init_xavix, "SSD Company LTD",         "XaviX Bowling (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // has IR 'Camera'
CONS( 2004, xavbox,   0, 0, xavix2002_i2c_jmat, xavix,  xavix_i2c_jmat_state,      init_xavix, "SSD Company LTD",         "XaviX Boxing (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // has IR 'Camera'
// Bass Fishing PCB is just like Tennis except with an RF daughterboard.
CONS( 2004, xavbassf, 0, 0, xavix2002_i2c_24c08, xavix_i2c,  xavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Bass Fishing (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// TODO: check SEEPROM type and hookup, banking!
CONS( 2005, xavjmat,  0, 0, xavix2002_i2c_jmat,  xavix,      xavix_i2c_jmat_state, init_xavix, "SSD Company LTD",         "Jackie Chan J-Mat Fitness (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, xavmusic, 0, 0, xavix2002_i2c_jmat,  xavix,      xavix_i2c_jmat_state, init_xavix, "SSD Company LTD",         "XaviX Music & Circuit (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// https://arnaudmeyer.wordpress.com/domyos-interactive-system/
// Domyos Fitness Adventure
// Domyos Fitness Challenge
// Domyos Fitness Exercises
// Domyos Fit Race
// Domyos Soft Fitness
// Domyos Fitness Dance
// Domyos Fitness Play
// Domyos Fitness Training

// Domyos Bike Concept (not listed on site above)

// Has SEEPROM and an RTC.  Exercise has some leftover PC buffer stuff.  (TODO, check SEEPROM type, RTC type, banking) (both Exercises and Challenge are identical PCBs)
CONS( 2008, domfitex, 0, 0, xavix2002_i2c_jmat, xavixp, xavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Exercises (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2008, domfitch, 0, 0, xavix2002_i2c_jmat, xavixp, xavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Challenge (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, domdance, 0, 0, xavix2002_i2c_jmat, xavixp, xavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Dance (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, domstepc, 0, 0, xavix2002_i2c_jmat, xavixp, xavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Step Concept (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// some DIS games run on XaviX 2 instead, see xavix2.cpp for Domyos Fitness Adventure and Domyos Bike Concept

CONS( 2005, mrangbat, 0, 0, xavix2002_i2c_mrangbat, mrangbat,   xavix_i2c_state, init_xavix, "Bandai / SSD Company LTD", "Mahou Taiketsu Magiranger - Magimat de Dance & Battle (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2004, epo_tfit, 0, 0, xavix2002_i2c_24c04,    epo_tfit,   xavix_i2c_state, init_xavix, "Epoch / SSD Company LTD",  "Excite Sports Tennis x Fitness (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // Epoch Tennis and Fitness has 24LC04

// TODO: does it have an SEEPROM? why does it hang? full title?
CONS( 2005, tmy_thom, 0, 0, xavix2002_i2c_24c04,    xavix_i2c,  xavix_i2c_state, init_xavix, "Tomy / SSD Company LTD",   "Thomas and Friends (Tomy)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// has HT24LC16
CONS( 2008, udance,   0, 0, xavix2002, xavix, xavix_state, init_xavix, "Tiger / SSD Company LTD", "U-Dance", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// this has RAM in the usual ROM space, needs handling, also has Atmel 24LC64.
CONS( 200?, suprpctv, 0, 0, xavix2002_superpctv,    xavix,      xavix2002_superpttv_state, init_xavix, "Epoch / SSD Company LTD", "Super PC TV (Epoch)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )


