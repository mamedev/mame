// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************/

/* SuperXaviX (XaviX 2002 type CPU) hardware titles (3rd XaviX generation?)

  these use the SSD 2002 NEC 85054-611 type CPU
  differences include support for 16-bit ROMs, high resolution bitmap modes, interlace screen modes, extra IO

   XavixPort Golf is "SSD 2003 SuperXaviX MXIC 2003 3009" (not dumped yet, but actually marked as SuperXaviX unlike the others!)

*/

#include "emu.h"
#include "xavix_2002.h"

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

	PORT_START("EX0")
	PORT_DIPNAME( 0x01, 0x00, "EX0" )
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

	PORT_START("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_i2c )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
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
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
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

	PORT_MODIFY("EX0") // NOT A JOYSTICK!!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_16WAY // Red/Up 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_16WAY // Red/Up 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_16WAY // Green / Circle / Right 2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_16WAY // Pink / Star / Left 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY // Blue / Square / Right 1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY // Yellow / Triangle / Left 1
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_bowl )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(superxavix_i2c_bowl_state::unknown_random_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(superxavix_i2c_bowl_state::unknown_random_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( ban_ult )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(superxavix_i2c_bowl_state::unknown_random_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(superxavix_i2c_bowl_state::unknown_random_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))

	PORT_MODIFY("EX2")
	PORT_DIPNAME( 0x80, 0x80, "Demo Mode" ) // bypasses calibration screen
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xavixp )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( ban_ordj )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) // something input related, having it high allows bypass of calibration screen
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ban_dn1j )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( anpanmdx )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Back
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // Start ('Paper')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // Left in Menu ('Red Star')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // Right in Menu ('Blue Star')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )
INPUT_PORTS_END

static INPUT_PORTS_START( suprtvpc )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("MOUSE0X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)
INPUT_PORTS_END

/* SuperXavix IO port handlers (per game) */

uint8_t superxavix_i2c_jmat_state::read_extended_io0(offs_t offset, uint8_t mem_mask)
{
	LOG("%s: read_extended_io0\n", machine().describe_context());
	return 0x00;
}

uint8_t superxavix_i2c_jmat_state::read_extended_io1(offs_t offset, uint8_t mem_mask)
{
	LOG("%s: read_extended_io1\n", machine().describe_context());

	uint8_t ret = 0x00;

	// reads this by reading the byte, then shifting right 4 times to place value into carry flag
	if (!(mem_mask & 0x08))
		ret |= m_i2cmem->read_sda() << 3;

	return ret;
}

uint8_t superxavix_i2c_jmat_state::read_extended_io2(offs_t offset, uint8_t mem_mask)
{
	LOG("%s: read_extended_io2\n", machine().describe_context());
	return 0x00;
}

void superxavix_i2c_jmat_state::write_extended_io0(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOG("%s: io0_data_w %02x\n", machine().describe_context(), data);
}

void superxavix_i2c_jmat_state::write_extended_io1(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOG("%s: io1_data_w %02x\n", machine().describe_context(), data);

	if (mem_mask & 0x08)
		m_i2cmem->write_sda((data & 0x08) >> 3);

	if (mem_mask & 0x10)
		m_i2cmem->write_scl((data & 0x10) >> 4);

}

void superxavix_i2c_jmat_state::write_extended_io2(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOG("%s: io2_data_w %02x\n", machine().describe_context(), data);
}

void superxavix_i2c_state::write_io1(uint8_t data, uint8_t direction)
{
	m_i2cmem->write_sda(BIT(data | ~direction, 3));
	m_i2cmem->write_scl(BIT(data | ~direction, 4));
}

void superxavix_state::xavix2002(machine_config &config)
{
	xavix(config);

	XAVIX2002(config.replace(), m_maincpu, MAIN_CLOCK * 2);
	set_xavix_cpumaps(config);
	m_maincpu->set_addrmap(5, &superxavix_state::superxavix_lowbus_map); // has extra video, io etc.

	m_palette->set_entries(512);

	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 64*8-1, 2*8, 30*8-1);

	XAVIX2002IO(config, m_xavix2002io, 0);

	m_xavix2002io->read_0_callback().set(FUNC(superxavix_state::superxavix_read_extended_io0));
	m_xavix2002io->write_0_callback().set(FUNC(superxavix_state::superxavix_write_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(superxavix_state::superxavix_read_extended_io1));
	m_xavix2002io->write_1_callback().set(FUNC(superxavix_state::superxavix_write_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(superxavix_state::superxavix_read_extended_io2));
	m_xavix2002io->write_2_callback().set(FUNC(superxavix_state::superxavix_write_extended_io2));
}

void superxavix_state::xavix2002_4mb(machine_config &config)
{
	xavix2002(config);
	m_maincpu->set_addrmap(6, &superxavix_state::xavix_4mb_extbus_map);
}

void superxavix_i2c_jmat_state::superxavix_i2c_jmat(machine_config &config)
{
	superxavix_i2c_24c08(config);

	m_xavix2002io->read_0_callback().set(FUNC(superxavix_i2c_jmat_state::read_extended_io0));
	m_xavix2002io->write_0_callback().set(FUNC(superxavix_i2c_jmat_state::write_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(superxavix_i2c_jmat_state::read_extended_io1));
	m_xavix2002io->write_1_callback().set(FUNC(superxavix_i2c_jmat_state::write_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(superxavix_i2c_jmat_state::read_extended_io2));
	m_xavix2002io->write_2_callback().set(FUNC(superxavix_i2c_jmat_state::write_extended_io2));
}

DEVICE_IMAGE_LOAD_MEMBER(superxavix_super_tv_pc_state::cart_load)
{
	u64 length;
	memory_region *cart_region = nullptr;

	if (m_cart->loaded_through_softlist())
	{
		cart_region = m_cart->memregion("prg");
		if (!cart_region)
			return std::make_pair(image_error::BADSOFTWARE, "Software list item is missing 'prg' region");
		length = cart_region->bytes();
	}
	else
	{
		length = m_cart->length();
	}

	if (!length)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
	else if (length > 0x40'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must be no larger than 4 MiB (0x400000 bytes)");
	else if (length & (length - 1))
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges size must be a power of two"); // to simplify copying into BIOS region

	if (!m_cart->loaded_through_softlist())
	{
		cart_region = machine().memory().region_alloc(m_cart->subtag("prg"), length, 1, ENDIANNESS_LITTLE);
		if (!cart_region)
			return std::make_pair(std::errc::not_enough_memory, std::string());

		if (m_cart->fread(cart_region->base(), length) != length)
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
	}

	// driver requires ROM code to be in a memory region, so need to copy (can't install in address space)
	memory_region *const bios_region = memregion("bios");
	for (offs_t base = 0; base < 0x40'0000; base += length)
		memcpy(bios_region->base() + base, cart_region->base(), length);

	return std::make_pair(std::error_condition(), std::string());
}

void superxavix_super_tv_pc_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("bios", 0x000000);
	map(0x500000, 0x5fffff).bankr("rombank"); // needed for suprtvpchk and suprtvpcdo to read bitmaps for loading screen and desktop
	map(0x600000, 0x67ffff).ram().share("bitmap_buffer"); // reads/writes here
}

void superxavix_super_tv_pc_state::machine_reset()
{
	superxavix_state::machine_reset();

	m_rombank->configure_entry(0, memregion("bios")->base() + 0x500000);
	m_rombank->configure_entry(1, memregion("bios")->base() + 0x700000);
	m_rombank->set_entry(0);
}


void superxavix_super_tv_pc_state::superxavix_super_tv_pc(machine_config& config)
{
	xavix2002(config);

	m_xavix2002io->read_0_callback().set(FUNC(superxavix_super_tv_pc_state::read_extended_io0));
	m_xavix2002io->write_0_callback().set(FUNC(superxavix_super_tv_pc_state::write_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(superxavix_super_tv_pc_state::read_extended_io1));
	m_xavix2002io->write_1_callback().set(FUNC(superxavix_super_tv_pc_state::write_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(superxavix_super_tv_pc_state::read_extended_io2));
	m_xavix2002io->write_2_callback().set(FUNC(superxavix_super_tv_pc_state::write_extended_io2));

	m_anport->read_0_callback().set(FUNC(superxavix_super_tv_pc_state::stvpc_anport0_r));
	m_anport->read_1_callback().set(FUNC(superxavix_super_tv_pc_state::stvpc_anport1_r));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "super_tv_pc_cart");
	m_cart->set_width(GENERIC_ROM8_WIDTH);
	m_cart->set_device_load(FUNC(superxavix_super_tv_pc_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("super_tv_pc_cart");
}

void superxavix_piano_pc_state::superxavix_piano_pc(machine_config &config)
{
	xavix2002(config);

	m_anport->read_0_callback().set(FUNC(superxavix_piano_pc_state::piano_pc_anport0_r));
	m_anport->read_1_callback().set(FUNC(superxavix_piano_pc_state::piano_pc_anport1_r));

	m_xavix2002io->read_0_callback().set(FUNC(superxavix_piano_pc_state::read_extended_io0));
	m_xavix2002io->read_1_callback().set(FUNC(superxavix_piano_pc_state::read_extended_io1));
	m_xavix2002io->read_2_callback().set(FUNC(superxavix_piano_pc_state::read_extended_io2));
}


void superxavix_doradraw_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("bios", 0x000000);
	map(0x400000, 0x4fffff).ram().share("bitmap_buffer"); // reads/writes here
	map(0x600000, 0x6fffff).ram().share("bitmap_buffer2"); // reads/writes here
}


void superxavix_doradraw_state::superxavix_doradraw(machine_config& config)
{
	xavix2002(config);
}

void superxavix_i2c_state::superxavix_i2c_24c16(machine_config &config)
{
	xavix2002(config);

	I2C_24C16(config, "i2cmem", 0);
}

void superxavix_i2c_state::superxavix_i2c_24c08(machine_config &config)
{
	xavix2002(config);

	I2C_24C08(config, "i2cmem", 0);
}

void superxavix_i2c_state::superxavix_i2c_24c04(machine_config &config)
{
	xavix2002(config);

	I2C_24C04(config, "i2cmem", 0);
}

void superxavix_i2c_state::superxavix_i2c_24c04_4mb(machine_config &config)
{
	superxavix_i2c_24c04(config);
	m_maincpu->set_addrmap(6, &superxavix_i2c_state::xavix_4mb_extbus_map);
}

void superxavix_i2c_state::superxavix_i2c_24c02(machine_config &config)
{
	xavix2002(config);

	I2C_24C02(config, "i2cmem", 0);
}


void superxavix_i2c_state::superxavix_i2c_mrangbat(machine_config &config)
{
	xavix2002(config);

	m_maincpu->set_addrmap(6, &superxavix_i2c_state::xavix_4mb_extbus_map);

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

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(52dc318c) SHA1(dc50e0747ba29cfb1048fd4a55d26870086c869b) )
ROM_END

ROM_START( xavaero )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "aerostep.u2", 0x0000000, 0x0800000, CRC(7fce9cc1) SHA1(460bcef8a23d792941108e5da8c0d669a546b94c) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "aerostep.u3", 0x0000000, 0x0800000, CRC(ed9ca4ee) SHA1(4d90300880b02ac275e0cb502de16ae6f132aa2b) )
ROM_END


// currently copies the wrong code into RAM to execute (due to extended ROM size, and possible banking)
// [:] ':maincpu' (00E074): rom_dmatrg_w (do DMA?) 01
// [:]   (possible DMA op SRC 00ebe2d3 DST 358a LEN 0398)
//         needs to come from 006be2d3 (so still from lower 8MB, not upper 8MB)

ROM_START( xavmusic )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(e7c8ad59) SHA1(d47fac8b480de4db88a1b306ff8830a65d1738a3) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(977c956f) SHA1(debc086d0cf6c391002ad163e7bfaa2f010cc8f5) )
ROM_END


// Domyos DiS (XaviX 2002 based titles)
ROM_START( domfitex )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000,  CRC(841fe3cd) SHA1(8678b8a0c5198b24169a84dbe3ae979bb0838f23) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(1dc844ea) SHA1(c23da9006227f7fe4982998c17759d403a47472a) )
ROM_END

ROM_START( domfitch )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(0ff2a7a6) SHA1(9b924cc4330e3f8d9204390854048fe2325bfdf7) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(284583f6) SHA1(bd2d5304f1e01eed656b5de957ec0a0330a3d969) )
ROM_END

ROM_START( domdance )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(74f9499d) SHA1(a64235075e32567cd6d2ab7b1284efcb8e7538e2) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(e437565c) SHA1(f6db219ea14404b698ca453f6e50c726b2e77abb) )
ROM_END

ROM_START( domstepc )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(cb37b5e9) SHA1(b742e3db98f36720adf5af9096c6bc235279de12) )

	ROM_REGION( 0x0800000, "extra_u3", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(dadaa744) SHA1(fd7ca77232a8fe228fc93b0a8a47ba3260349d90) )
ROM_END

ROM_START( anpanmdx )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "apmj.u3", 0x0000000, 0x0800000, CRC(41348086) SHA1(63bbf6128901c1518f537766a40e162b2616d00c) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "am2j.u7", 0x0000000, 0x0800000, CRC(ff653a6b) SHA1(ece11198a06f9cddfae7f8c7e038675010869723) )
ROM_END

ROM_START( apmj2009 )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "apmj.u3", 0x0000000, 0x0800000, CRC(5fab9492) SHA1(aa588e5333bdf81daf3b5868e00783d76a42e80e) )
ROM_END

ROM_START( mrangbat )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("powerrangerspad.bin", 0x000000, 0x400000, CRC(d3a98775) SHA1(485c66242dd0ee436a278d23005aece48d606431) )
ROM_END

ROM_START( tmy_thom )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "thomastank.bin", 0x000000, 0x800000, CRC(a52a23be) SHA1(e5b3500239d9e56eb5405f7585982959e5a162da) )
ROM_END

ROM_START( ban_kksj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "kksj.u1", 0x000000, 0x800000, CRC(8071dc36) SHA1(46f41d4185a115b27c685d1eabcd554b3c5a64b7) )
ROM_END

ROM_START( tmy_rkmj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "rkmj.u1", 0x000000, 0x800000, CRC(80e70625) SHA1(500e287671a0822b736ed05704090d90187602ac) )
ROM_END

ROM_START( ban_ordj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ordj.u2", 0x000000, 0x800000, CRC(78fbb00f) SHA1(797b5495e292c36c003300ed18547e5643056149) )
ROM_END

ROM_START( ban_dn1j )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "dn1j.u2", 0x000000, 0x800000, CRC(0a0cef0f) SHA1(c83a2635a969b3c686dbc599a37f8b7496b0c6a1) )
ROM_END


ROM_START( epo_tfit )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("tennisfitness.bin", 0x000000, 0x400000, CRC(cbf65bd2) SHA1(30b3da6f061b2dd91679db42a050f715901beb87) )
ROM_END

ROM_START( maxheart )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("mgrj.u2", 0x000000, 0x400000, CRC(447c25e6) SHA1(9cc65088512218f43d66b332de7a862d95c1c353) )
ROM_END

ROM_START( epo_doka )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("doka.u1", 0x000000, 0x400000, CRC(853266d2) SHA1(d4121b89ee464088951898282404e5a2b788dd69) )
ROM_END

ROM_START( ban_utmj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("utmj.u7", 0x000000, 0x800000, CRC(0ac2bcd9) SHA1(ca7c82e2015c86bb37bd66016c33343d174e9965) )

	// SEEPROM is HT24LC02 at u3
ROM_END

ROM_START( ban_ult )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("ultraman.u1", 0x000000, 0x800000,CRC(bc2a94fb) SHA1(4dc81089ac2afc1c9496a49ffd778213bb4a12bd) )
ROM_END

ROM_START( ban_bkgj )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bkgj.u2", 0x000000, 0x400000, CRC(a59ce23c) SHA1(d2a6be9e46f3cfc3cf798bf1f76732eee909c93b) )

	// SEEPROM is a S-24CS04A at u4
ROM_END

ROM_START( epo_rgfj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	// gave consistent reads 5 times, then started not, should be good, but there is the potential for the ROM to have already been failing
	ROM_LOAD("rgfj.u1", 0x000000, 0x800000, CRC(96c9563a) SHA1(36b9dd3e5dcc8099787b25d28143997f61273234) )
ROM_END

ROM_START( udance )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("udancerom0.bin", 0x000000, 0x800000, CRC(3066580a) SHA1(545257c75a892894faf386f4ab9a31967cdbe8ae) )

	ROM_REGION(0x800000, "biosx", ROMREGION_ERASE00)
	ROM_LOAD("udancerom1.bin", 0x000000, 0x800000, CRC(7dbaabde) SHA1(38c523dcdf8185465fc550fb9b0e8c7909f839be) )
ROM_END

ROM_START( suprtvpc )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00) // inverted line?
	ROM_LOAD("supertvpc_dogs.u4", 0x200000, 0x200000, CRC(ab326e6d) SHA1(e22205f6ff4c8cc46538d78e27535be63acea42a) )
	ROM_CONTINUE(0x000000, 0x200000)
	ROM_CONTINUE(0x600000, 0x200000)
	ROM_CONTINUE(0x400000, 0x200000)
ROM_END

ROM_START( suprtvpchk )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00) // inverted line?
	ROM_LOAD("superpctv.bin", 0x200000, 0x200000, CRC(4a55a81c) SHA1(178b4b595a3aefc6d1c176031b436fc3312009e7) )
	ROM_CONTINUE(0x000000, 0x200000)
	ROM_CONTINUE(0x600000, 0x200000)
	ROM_CONTINUE(0x400000, 0x200000)
ROM_END

ROM_START( suprtvpcdo )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00) // inverted line?
	ROM_LOAD("supertvpc_doreamon.u4", 0x200000, 0x200000, CRC(8e7039dc) SHA1(44ffecc8195614e56c289a028c2140c24ad74171) )
	ROM_CONTINUE(0x000000, 0x200000)
	ROM_CONTINUE(0x600000, 0x200000)
	ROM_CONTINUE(0x400000, 0x200000)
ROM_END

ROM_START( epo_ntpj )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("ntpj.u6", 0x000000, 0x800000, CRC(6ce02166) SHA1(21c2ed48014e66123bb9968648984f82de361e2a) )

	// uses IS24C64 EEPROM

	// there is extra hardware for the Piano side of things which may or may not have ROM data in it
ROM_END

ROM_START( doradraw )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("dmdj.u2", 0x000000, 0x800000, CRC(b3ca50ab) SHA1(9e6d28c1e170d3556e3c4ddcefb4cb51fd100df5) )

	ROM_REGION(0x200000, "data", ROMREGION_ERASE00) // banked or extended video bus?
	ROM_LOAD("dmdj.u7", 0x000000, 0x200000, CRC(0e6392f9) SHA1(30fa3d3451b37d663e124c7d1d52c7e30284d2fb) )
ROM_END

ROM_START( ndpbj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("ndpbj.u2", 0x000000, 0x800000, CRC(80cb5cbb) SHA1(cd424c4fbea8e9e47d165c4c8be52755fc7c2d98) )

	ROM_REGION( 0x400, "i2cmem", ROMREGION_ERASE00)
	ROM_LOAD("s-24cs08a.u6", 0x000, 0x400, CRC(a22db408) SHA1(f8d925c75054a961930af12869e3002bb9c4600b) )
ROM_END


void superxavix_super_tv_pc_state::init_stvpc()
{
	init_xavix();
	m_disable_memory_bypass = true;
}

void superxavix_i2c_jmat_state::init_xavmusic()
{
	init_xavix();
	// is sprite yflip broken on (some?) revisions of SuperXaviX hardware, or is there a CPU bug causing this
	m_disable_sprite_yflip = true;
}

void superxavix_piano_pc_state::init_piano_pc()
{
	init_xavix();
	m_disable_memory_bypass = true;
}

void superxavix_state::init_epo_doka()
{
	init_xavix();
	m_disable_tile_regs_flip = true;
}

void superxavix_doradraw_state::init_doradraw()
{
	init_xavix();
	m_disable_memory_bypass = true;
}


CONS( 2004, xavtenni, 0, 0, superxavix_i2c_24c04, xavix_i2c,  superxavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Tennis (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, xavbaseb, 0, 0, superxavix_i2c_24c08, xavix_i2c,  superxavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Baseball (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2004, xavbowl,  0, 0, superxavix_i2c_24c04, xavix_bowl, superxavix_i2c_bowl_state, init_xavix, "SSD Company LTD",         "XaviX Bowling (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // has IR 'Camera'
CONS( 2004, xavbox,   0, 0, superxavix_i2c_jmat, xavix,  superxavix_i2c_jmat_state,      init_xavix, "SSD Company LTD",         "XaviX Boxing (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // has IR 'Camera'
// Bass Fishing PCB is just like Tennis except with an RF daughterboard.
CONS( 2004, xavbassf, 0, 0, superxavix_i2c_24c08, xavix_i2c,  superxavix_i2c_state,      init_xavix, "SSD Company LTD",         "XaviX Bass Fishing (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// TODO: check SEEPROM type and hookup, banking!
CONS( 2005, xavjmat,  0, 0, superxavix_i2c_jmat,  xavix,      superxavix_i2c_jmat_state, init_xavix, "SSD Company LTD",         "Jackie Chan J-Mat Fitness (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2005, xavaero,  0, 0, superxavix_i2c_jmat,  xavix,      superxavix_i2c_jmat_state, init_xavix, "SSD Company LTD",         "XaviX Aerostep (XaviXPORT, Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, xavmusic, 0, 0, superxavix_i2c_jmat,  xavix,      superxavix_i2c_jmat_state, init_xavmusic, "SSD Company LTD",         "XaviX Music & Circuit (XaviXPORT)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

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
CONS( 2008, domfitex, 0, 0, superxavix_i2c_jmat, xavixp, superxavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Exercises (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2008, domfitch, 0, 0, superxavix_i2c_jmat, xavixp, superxavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Challenge (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, domdance, 0, 0, superxavix_i2c_jmat, xavixp, superxavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Fitness Dance (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, domstepc, 0, 0, superxavix_i2c_jmat, xavixp, superxavix_i2c_jmat_state, init_xavix, "Decathlon / SSD Company LTD", "Domyos Step Concept (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// some DIS games run on XaviX 2 instead, see xavix2.cpp for Domyos Fitness Adventure and Domyos Bike Concept

// Let's!TVプレイ 魔法戦隊マジレンジャー マジマットでダンス＆バトル
CONS( 2005, mrangbat, 0, 0, superxavix_i2c_mrangbat, mrangbat,   superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD", "Let's! TV Play Mahou Taiketsu Magiranger - Magimat de Dance & Battle (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// エキサイトスポーツ　テニス×フィットネス
CONS( 2004, epo_tfit, 0, 0, superxavix_i2c_24c04_4mb,    epo_tfit,   superxavix_i2c_state, init_xavix, "Epoch / SSD Company LTD",  "Excite Sports Tennis x Fitness (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// 石川遼 エキサイトゴルフ
CONS( 2010, epo_rgfj, 0, 0, superxavix_i2c_24c08,        xavix_i2c,  superxavix_i2c_state, init_xavix, "Epoch / SSD Company LTD", "Ishikawa Ryou Excite Golf (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// Let's!TVプレイ ふたりはプリキュアMaxHeart マットでダンス MaxHeartにおどっちゃおう
CONS( 2004, maxheart, 0, 0, superxavix_i2c_24c04_4mb,    xavix_i2c,   superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",  "Let's! TV Play Futari wa PreCure MaxHeart Mat de Dance MaxHeart ni Odotchaou (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// どこでもドラえもん 日本旅行ゲームDX体感！どこドラグランプリ！
CONS( 2004, epo_doka, 0, 0, xavix2002_4mb,               xavix,      superxavix_state,     init_epo_doka, "Epoch / SSD Company LTD",  "Doko Demo Doraemon Nihon Ryokou Game DX Taikan! Doko Dora Grand Prix! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// Let's!TVプレイ なりきり体感 ボウケンジャー 走れ！撃て！ミッションスタート！！
CONS( 2006, ban_bkgj, 0, 0, superxavix_i2c_24c04_4mb,xavix_i2c,  superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",  "Let's! TV Play Narikiri Taikan Boukenger Hashire! Ute! Mission Start!! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// Let's!TV プレイ 体感キャストオフ 仮面ライダーカブト クロックアップ＆ライダーキック
CONS( 2006, ban_utmj, 0, 0, superxavix_i2c_24c02,    xavix_i2c,  superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",  "Let's! TV Play Taikan Cast Off - Kamen Rider Kabuto Clock Up & Rider Kick!! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// Let's!TVプレイ なりきりファイト ウルトラマン 撃て！必殺光線！！
CONS( 2006, ban_ult, 0, 0, superxavix_i2c_24c02,    ban_ult,  superxavix_i2c_bowl_state, init_no_timer, "Bandai / SSD Company LTD",  "Let's! TV Play Narikiri Fight Ultraman - Ute! Hissatsu Kousen!! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

// それいけトーマス ソドー島のなかまたち
CONS( 2005, tmy_thom, 0, 0, superxavix_i2c_24c04,    xavix_i2c,  superxavix_i2c_state, init_xavix, "Tomy / SSD Company LTD",   "Soreike Thomas - Sodor Tou no Nakamatachi / Thomas & Friends on the Island of Sodor (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// Ｌｅｔ’ｓ！ ＴＶプレイ 体感体得 結界師 方囲！定礎！結！滅！
CONS( 2007, ban_kksj, 0, 0, superxavix_i2c_24c02,    xavix_i2c,  superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",   "Let's! TV Play Taikan Taitoku Kekkaishi: Houi! Jouso! Ketsu! Metsu! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// 流星のロックマン 電波変換！オン・エア！
CONS( 2007, tmy_rkmj, 0, 0, superxavix_i2c_24c04,    xavix_i2c,  superxavix_i2c_state, init_xavix, "Takara Tomy / Capcom / SSD Company LTD",   "Ryuusei no Rockman: Denpa Henkan! On Air! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// Let's!TVプレイ 音撃バトル！仮面ライダー響鬼 決めろ！一気火勢の型
CONS( 2005, ban_ordj, 0, 0, superxavix_i2c_24c04,    ban_ordj,   superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",   "Let's! TV Play Ongeki Battle! Kamen Rider Hibiki: Kimero! Ikki Kasei no Kata (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// ディズニーキャラクターズ オト！イロ！トン・トン！ミラクルパレード
CONS( 2007, ban_dn1j, 0, 0, superxavix_i2c_24c04,    ban_dn1j,   superxavix_i2c_state, init_xavix, "Bandai / SSD Company LTD",   "Let's! TV Play Disney Characters Oto! Iro! Ton-Ton! Miracle Parade (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// アンパンマン かぞくで!育脳マットDX
CONS( 2011, anpanmdx, 0, 0, superxavix_i2c_24c08,    anpanmdx,   superxavix_i2c_state, init_xavix, "JoyPalette / SSD Company LTD",   "Anpanman Kazoku de! Ikunou Mat DX (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// アンパンマン ぴょんぴょん育脳マット
CONS( 2009, apmj2009, 0, 0, superxavix_i2c_24c16,    xavix_i2c,  superxavix_i2c_state, init_xavix, "JoyPalette / SSD Company LTD",   "Anpanman Pyon-Pyon Ikunou Mat (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// has HT24LC16
CONS( 2008, udance,   0, 0, xavix2002, xavix, superxavix_state, init_xavix, "Tiger / SSD Company LTD", "U-Dance", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// these have RAM in the usual ROM space & also have an Atmel 24LC64,
CONS( 2004, suprtvpc,    0,        0, superxavix_super_tv_pc,    suprtvpc,      superxavix_super_tv_pc_state, init_stvpc, "Epoch / SSD Company LTD", "Super TV-PC (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2006, suprtvpchk,  suprtvpc, 0, superxavix_super_tv_pc,    suprtvpc,      superxavix_super_tv_pc_state, init_stvpc, "Epoch / SSD Company LTD", "Super TV-PC - Hello Kitty (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2006, suprtvpcdo,  suprtvpc, 0, superxavix_super_tv_pc,    suprtvpc,      superxavix_super_tv_pc_state, init_stvpc, "Epoch / SSD Company LTD", "Super TV-PC - Doraemon (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// similar to Super TV-PC but with additional built in piano
CONS( 2008, epo_ntpj,  0, 0, superxavix_piano_pc, suprtvpc, superxavix_piano_pc_state, init_piano_pc, "Epoch / SSD Company LTD", "Hello Kitty Piano PC (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// ドラえもん うごく！おえかき エポック社
CONS( 2007, doradraw,  0, 0, superxavix_doradraw,    xavix,      superxavix_doradraw_state, init_doradraw, "Epoch / SSD Company LTD", "Doraemon Ugoku! Oekaki (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// どんトレだ兵衛～どん兵衛くんとトレーニング
// doesn't boot, has a camera with a large number of connections going to it, probably wants comms to work with it?
CONS( 2007, ndpbj, 0, 0, superxavix_i2c_24c08,    xavix_i2c,  superxavix_i2c_state, init_xavix, "Nissin / SSD Company LTD",   "Dontore da bei - Donbei-kun to Training (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
