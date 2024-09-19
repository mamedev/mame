// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Note: even after the GameKey port was physically removed and the PCBs redesigned, many of the test modes still show the value read from the port (and many games still show the Game Key Ready splash screen on startup)

#include "emu.h"
#include "spg2xx.h"

#include "bus/jakks_gamekey/slot.h"
#include "softlist_dev.h"


namespace {

class jakks_gkr_state : public spg2xx_game_state
{
public:
	jakks_gkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag),
		m_porta_key_mode(false),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr),
		m_analog_x(*this, "JOYX"),
		m_analog_y(*this, "JOYY")
	{ }

	void jakks_gkr(machine_config &config);
	void jakks_gkr_i2c(machine_config &config);
	void jakks_gkr_1m_i2c(machine_config &config);
	void jakks_gkr_2m_i2c(machine_config &config);
	void jakks_gkr_nk(machine_config &config);
	void jakks_gkr_nk_i2c(machine_config &config);
	void jakks_gkr_dy(machine_config &config);
	void jakks_gkr_dy_i2c(machine_config &config);
	void jakks_gkr_dp_i2c(machine_config &config);
	void jakks_gkr_sw_i2c(machine_config &config);
	void jakks_gkr_nm_i2c(machine_config &config);
	void jakks_gkr_cc_i2c(machine_config &config);
	void jakks_gkr_wf_i2c(machine_config &config);
	void jakks_gkr_mv_i2c(machine_config &config);
	void jakks_gkr_wp(machine_config &config);
	void jakks_gkr_cb(machine_config &config);

	int i2c_gkr_r();

protected:
	[[maybe_unused]] uint16_t jakks_porta_r();
	[[maybe_unused]] void jakks_porta_w(uint16_t data);
	void jakks_portb_w(uint16_t data);

private:
	virtual void machine_start() override ATTR_COLD;

	uint16_t joy_x_read();
	uint16_t joy_y_read();

	uint16_t jakks_porta_key_io_r();

	void gkr_portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void jakks_porta_key_io_w(uint16_t data);
	bool m_porta_key_mode;

	optional_device<jakks_gamekey_slot_device> m_cart;
	memory_region *m_cart_region;

	optional_ioport m_analog_x;
	optional_ioport m_analog_y;
};

int jakks_gkr_state::i2c_gkr_r()
{
	if (m_cart && m_cart->exists())
	{
		return m_cart->read_cart_seeprom();
	}
	else
	{
		return m_i2cmem->read_sda();
	}
}

void jakks_gkr_state::gkr_portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_cart && m_cart->exists())
	{
		m_cart->write_cart_seeprom(offset, data, mem_mask);
	}
	else
	{
		if (m_i2cmem)
		{
			if (BIT(mem_mask, 1))
				m_i2cmem->write_scl(BIT(data, 1));
			if (BIT(mem_mask, 0))
				m_i2cmem->write_sda(BIT(data, 0));
		}
	}
}

uint16_t jakks_gkr_state::jakks_porta_r()
{
	//logerror("%s: jakks_porta_r\n", machine().describe_context());
	return m_io_p1->read();
}

void jakks_gkr_state::jakks_porta_w(uint16_t data)
{
	//logerror("%s: jakks_porta_w %04x\n", machine().describe_context(), data);
}

void jakks_gkr_state::jakks_portb_w(uint16_t data)
{
	//logerror("%s: jakks_portb_w %04x\n", machine().describe_context(), data);
}

uint16_t jakks_gkr_state::joy_x_read()
{
	const uint16_t data = m_analog_x->read();
	return data > 0x0fff ? 0x0fff : data;
}

uint16_t jakks_gkr_state::joy_y_read()
{
	const uint16_t data = m_analog_y->read();
	return data > 0x0fff ? 0x0fff : data;
}

uint16_t jakks_gkr_state::jakks_porta_key_io_r()
{
	//logerror("%s: jakks_porta_key_io_r\n", machine().describe_context());
	if (m_porta_key_mode == false)
	{
		return m_io_p1->read();
	}
	else
	{
		/* masks with 0xf, inverts, and combines it with a previous read (when data written to jakks_porta_key_io_w was 0x0000) and expects result to be 0x0000
		   could just expect data written to be returned, but that would be a strange check.
		   all systems seem to respond to the same result, so how is the per-system lock implemented? */
		return (m_io_p1->read() & 0xfff0) | 0x000f;
	}
}

void jakks_gkr_state::jakks_porta_key_io_w(uint16_t data)
{
	logerror("%s: jakks_porta_key_io_w %04x\n", machine().describe_context(), data);
	// only seen 0xffff and 0x0000 written here.. writes 0xffff before the 2nd part of the port a gamekey check read.
	if (data == 0xffff)
	{
		m_porta_key_mode = true;
	}
	else
	{
		m_porta_key_mode = false;
	}
}


static INPUT_PORTS_START( jak_sith_i2c )
	PORT_START("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0xf3df, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("JOYX")
	PORT_BIT(0x0fff, 0x0800, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x0000,0x0fff)

	PORT_START("JOYY")
	PORT_BIT(0x0fff, 0x0800, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x0000,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_pooh )
	PORT_START("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Menu / Pause")
	PORT_BIT( 0xf7df, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xfff7, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)

	PORT_START("JOYX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("JOYY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_care )
	PORT_START("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Menu / Pause")
	PORT_BIT( 0xf7df, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0xfff7, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)

	PORT_START("JOYX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("JOYY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_nm_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range)
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_cc_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("D")

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_wf_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
	PORT_BIT( 0xfff6, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC

	/* on real unit you can spin the wheel (and must make sure it completes a full circle, or you lose your turn) instead of pressing 'B' for a random spin but where does it map? (it can be tested in secret test mode)
	PORT_START("DIALX")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)

	PORT_START("DIALY")
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
	*/
INPUT_PORTS_END

static INPUT_PORTS_START( jak_gkr )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (not verified for all games, state can be seen in secret test menu of many tho)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) // this causes WWE to think the unit is a '2nd Controller' and tells you to plug the 1st one in.
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_sdoo_i2c ) // GameKeyReady units had 2 main buttons, later releases reduced that to 1 button (as the internal games don't require 2 and no GameKeys were released)
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED ) // debug input, skips levels!
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED ) // must be low or other inputs don't work?
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r) // is this correct? doesn't seem to work
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_gkr_i2c )
	PORT_INCLUDE(jak_gkr)

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
INPUT_PORTS_END

static INPUT_PORTS_START( jak_dpr_i2c )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Start / Menu / Pause")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jakks_gkr_state, i2c_gkr_r)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff6, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void jakks_gkr_state::machine_start()
{
	spg2xx_game_state::machine_start();

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(JAKKSSLOT_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cart_region->bytes() + 0x7fffff) / 0x800000, m_cart_region->base(), 0x800000);
		m_bank->set_entry(0);
	}
}

void jakks_gkr_state::jakks_gkr(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_gkr_state::jakks_porta_key_io_r));
	m_maincpu->porta_out().set(FUNC(jakks_gkr_state::jakks_porta_key_io_w));
	m_maincpu->portb_out().set(FUNC(jakks_gkr_state::jakks_portb_w));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_gkr_state::gkr_portc_w));

	JAKKS_GAMEKEY_SLOT(config, m_cart, 0, jakks_gamekey, nullptr);
}

void jakks_gkr_state::jakks_gkr_i2c(machine_config &config)
{
	jakks_gkr(config);
	I2C_24C16(config, m_i2cmem, 0); // ?
}


void jakks_gkr_state::jakks_gkr_1m_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
}

void jakks_gkr_state::jakks_gkr_2m_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_2m);
}

void jakks_gkr_state::jakks_gkr_nk(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_nk").set_original("jakks_gamekey_nk");
}

void jakks_gkr_state::jakks_gkr_nk_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_nk").set_original("jakks_gamekey_nk");
}

void jakks_gkr_state::jakks_gkr_dy(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_dy").set_original("jakks_gamekey_dy");
}

void jakks_gkr_state::jakks_gkr_dy_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_dy").set_original("jakks_gamekey_dy");
}

void jakks_gkr_state::jakks_gkr_mv_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_mv").set_original("jakks_gamekey_mv");
}


void jakks_gkr_state::jakks_gkr_dp_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	SOFTWARE_LIST(config, "jakks_gamekey_dp").set_original("jakks_gamekey_dp");
}

void jakks_gkr_state::jakks_gkr_sw_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_2m);
	m_maincpu->adc_in<0>().set(FUNC(jakks_gkr_state::joy_x_read));
	m_maincpu->adc_in<1>().set(FUNC(jakks_gkr_state::joy_x_read));
	m_maincpu->adc_in<2>().set(FUNC(jakks_gkr_state::joy_y_read));
	m_maincpu->adc_in<3>().set(FUNC(jakks_gkr_state::joy_y_read));
	SOFTWARE_LIST(config, "jakks_gamekey_sw").set_original("jakks_gamekey_sw");
}

void jakks_gkr_state::jakks_gkr_wp(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_maincpu->adc_in<0>().set_ioport("JOYX");
	m_maincpu->adc_in<2>().set_ioport("JOYY");
	//SOFTWARE_LIST(config, "jakks_gamekey_wp").set_original("jakks_gamekey_wp"); // NO KEYS RELEASED

	m_maincpu->set_force_no_drc(true); // the Light Tag game seems to hang maybe once every 7 times with the DRC, appears more stable without (could just be chance tho)
}

void jakks_gkr_state::jakks_gkr_cb(machine_config &config)
{
	jakks_gkr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_maincpu->adc_in<0>().set_ioport("JOYX");
	m_maincpu->adc_in<2>().set_ioport("JOYY");
	//SOFTWARE_LIST(config, "jakks_gamekey_cb").set_original("jakks_gamekey_cb"); // NO KEYS RELEASED
}

void jakks_gkr_state::jakks_gkr_nm_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	m_maincpu->adc_in<0>().set_ioport("DIALX");
	SOFTWARE_LIST(config, "jakks_gamekey_nm").set_original("jakks_gamekey_nm");
}

void jakks_gkr_state::jakks_gkr_cc_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	// shows 'E0' in gamekey test menu on real HW (maybe related to value key needs to return if one existed)
	//SOFTWARE_LIST(config, "jakks_gamekey_cc").set_original("jakks_gamekey_cc"); // no game keys were released
}

void jakks_gkr_state::jakks_gkr_wf_i2c(machine_config &config)
{
	jakks_gkr_i2c(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_gkr_state::mem_map_1m);
	//m_maincpu->adc_in<0>().set_ioport("DIALX"); // wheel does not seem to map here
	//m_maincpu->adc_in<1>().set_ioport("DIALY");
	//SOFTWARE_LIST(config, "jakks_gamekey_wf").set_original("jakks_gamekey_wf"); // no game keys were released
}

ROM_START( jak_wwe )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkswwegkr.bin", 0x000000, 0x200000, CRC(b078a812) SHA1(7d97c0e2171b3fd91b280480c9ffd5651828195a) )
ROM_END

ROM_START( jak_fan4 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksffgkr.bin", 0x000000, 0x200000, CRC(8755a1f7) SHA1(7214da15fe61881da27b81575fbdb54cc0f1d6aa) )
ROM_END

ROM_START( jak_just )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksjlagkr.bin", 0x000000, 0x200000, CRC(182989f0) SHA1(799229c537d6fe629ba9e1e4051d1bb9ca445d44) )
ROM_END

ROM_START( jak_dora )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdoragkr.bin", 0x000000, 0x200000, CRC(bcaa132d) SHA1(3894b980fbc4144731b2a7a94acebb29e30de67c) )
ROM_END

ROM_START( jak_nick )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksnicktoonsgkr.bin", 0x000000, 0x200000, CRC(4dec1656) SHA1(b3002ab15e75068102f4955a3f0c52fb6d5cda56) )
ROM_END

ROM_START( jak_sbfc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksspongebobgkr.bin", 0x000000, 0x200000, CRC(9871303c) SHA1(78bc2687e1514094db8bb875e1117df3fcb3d201) )
ROM_END

ROM_START( jak_dorr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdora2gkr.bin", 0x000000, 0x200000, CRC(6c09bcd9) SHA1(4bcad79658832f319d16b4f63257e127f6862d79) )
ROM_END


ROM_START( jak_spdm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksspidermangkr.bin", 0x000000, 0x200000, CRC(1b2ee700) SHA1(30ea69c489e1238b004f473f972b682e35573138) )
ROM_END

ROM_START( jak_pooh )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspoohgkr.bin", 0x000000, 0x200000, CRC(0d97df55) SHA1(f108621a83c7b2263dd1531d82311627c3a02002) )
ROM_END

ROM_START( jak_care )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "carebeargkr.bin", 0x000000, 0x200000, CRC(e6096eb7) SHA1(92ee1a6df374f8b355ba2280dc43d764f6f69dfe) )
ROM_END

ROM_START( jak_wof )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkswheeloffortunegkr.bin", 0x000000, 0x200000, CRC(6a879620) SHA1(95478764a61741569041c2299528f6464651d593) )
ROM_END

ROM_START( jak_disn )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "disneygkr.bin", 0x000000, 0x100000,  CRC(7a5ebcd7) SHA1(9add8c2a6e3f0409c8957a2ba2d054fd2c4c39c1) )
ROM_END

ROM_START( jak_disf )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "disneyfriendsgkr.bin", 0x000000, 0x200000, CRC(77bca50b) SHA1(6e0f4fd229ee11eac721b5dbe79cf9002d3dbd64) )
ROM_END

ROM_START( jak_dpr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdisneyprincessgkr.bin", 0x000000, 0x200000, CRC(e26003ce) SHA1(ee15243281df6f09b96185c34582d7091604c954) )
ROM_END

ROM_START( jak_dprs )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "disneyprincess2gkr.bin", 0x000000, 0x200000, CRC(b670bdde) SHA1(c33ce7ada72a0c44bc881b5792cd33a9f2f0fb08) )
ROM_END

ROM_START( jak_mpac )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksmspacmangkr.bin", 0x000000, 0x100000, CRC(cab40f77) SHA1(30731acc461150d96aafa7a0451cfb1a25264678) )
ROM_END

ROM_START( jak_sdoo )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksscoobydoogkr.bin", 0x000000, 0x400000, CRC(61062ce5) SHA1(9d21767fd855385ef83e4209c429ecd4bf7e5384) )
ROM_END

ROM_START( jak_dwmn )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dreamworks.bin", 0x000000, 0x400000, CRC(3ae9f786) SHA1(46451be3af459fbdb75d1155b3817543afe183d5) )
ROM_END

ROM_START( jak_dwmno )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dw_spg300_test.bin", 0x000000, 0x400000, CRC(1ca2817b) SHA1(39ae519457c102c4420fae3699b2db0557ef1cf5) )
ROM_END

ROM_START( jak_xmenp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "xmen.bin", 0x000000, 0x400000, CRC(1fa271e0) SHA1(c32652e9eddf82ab496e3609f8fa444e447fb509) )
ROM_END


ROM_START( jak_dbz )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdragonballzgkr.bin", 0x000000, 0x200000, CRC(d52c3b20) SHA1(fd5ce41c143cad9bca3372054f4ff98b52c33874) )
ROM_END

ROM_START( jak_sith )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksstarwarsgkr.bin", 0x000000, 0x200000, CRC(932cde19) SHA1(b88b748c235e9eeeda574e4d5b4077ae9da6fbd0) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END

ROM_START( jak_sithp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "starwars_prototype.bin", 0x000000, 0x400000, CRC(796b7d90) SHA1(9bcb9899dcaae57288316fe60e7724512e80c905) )
ROM_END

ROM_START( jak_swot )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "starwars_originaltrilogy_prototype.bin", 0x000000, 0x400000, CRC(3dda7aff) SHA1(970044a6b2f14863353e559f5d2a4e928c8de439) )
ROM_END

ROM_START( jak_capc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "capcomgkr.bin", 0x000000, 0x200000, CRC(6d47cce4) SHA1(263926a991d55459aa3cee90049d2202c1e3a70e) )
ROM_END

} // anonymous namespace


// 'Game-Key Ready' JAKKS games (these can also take per-game specific expansion cartridges, although not all games had them released)
// Some of these were available in versions without Game-Key ports, it is unconfirmed if code was the same unless otherwise stated
// For units released AFTER the GameKey promotion was cancelled it appears the code is the same as the PCB inside is the same, just the external port closed off, earlier units might be different hardware in some cases.
// units released BEFORE the GameKey support were sometimes different hardware, eg. the Spider-Man and Disney units were SPG110 based
CONS( 2005, jak_wwe,   0,        0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "WWE (JAKKS Pacific TV Game, Game-Key Ready)",            MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // WW (no game-keys released)
CONS( 2005, jak_fan4,  0,        0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse",         "Fantastic Four (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // F4 (no game-keys released)
CONS( 2005, jak_just,  0,        0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Taniko",                  "Justice League (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // DC (no game-keys released)
CONS( 2005, jak_dora,  0,        0, jakks_gkr_nk,     jak_gkr,      jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dora the Explorer - Nursery Rhyme Adventure (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys (same as Nicktoons & Spongebob) (3 released) - The upper part of this one is pink/purple.
CONS( 2005, jak_dorr,  0,        0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dora the Explorer - Race to Play Park (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys (same as Nicktoons & Spongebob) (3 released) - The upper part of this one is blue
CONS( 2004, jak_nick,  0,        0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Nicktoons (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys
CONS( 2005, jak_sbfc,  0,        0, jakks_gkr_nk_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "SpongeBob SquarePants - The Fry Cook Games (JAKKS Pacific TV Game, Game-Key Ready) (AUG 18 2005 21:31:56)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NK keys
CONS( 2005, jak_sdoo,  0,        0, jakks_gkr_2m_i2c, jak_sdoo_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Jolliford Management",    "Scooby-Doo! and the Mystery of the Castle (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) //  SD (no game-keys released)  (was dumped from a later unit with GameKey port missing, but internal PCB still supported it, code likely the same)
CONS( 2005, jak_disn,  0,        0, jakks_gkr_dy,     jak_gkr,      jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "Disney (JAKKS Pacific TV Game, Game-Key Ready) (08 FEB 2005 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DY keys (3 released)
CONS( 2005, jak_disf,  0,        0, jakks_gkr_dy_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "Disney Friends (JAKKS Pacific TV Game, Game-Key Ready) (17 MAY 2005 A)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DY keys (3 released)
CONS( 2005, jak_dpr,   0,        0, jakks_gkr_dp_i2c, jak_dpr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / 5000ft, Inc",             "Disney Princess (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DP keys (1 key released)
CONS( 2005, jak_dprs,  0,        0, jakks_gkr_dp_i2c, jak_dpr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / 5000ft, Inc",             "Disney Princesses (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses DP keys (1 key released) (unit looks identical to above, including just having 'Disney Princess' logo, but this one has the 'board game' as a frontend and a slightly different on-screen title)
// Some versions of the 'Revenge of the Sith' box art show 'Classic Battles' below the Star Wars logo
CONS( 2005, jak_sith,  0,        0, jakks_gkr_sw_i2c, jak_sith_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Griptonite Games",        "Star Wars - Revenge of the Sith (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses SW keys (1 released)
CONS( 2005, jak_sithp, jak_sith, 0, jakks_gkr_sw_i2c, jak_sith_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Griptonite Games",        "Star Wars - Revenge of the Sith (JAKKS Pacific TV Game, Game-Key Ready, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // ^
// release version of Star Wars Original Trilogy not dumped yet
CONS( 2006, jak_swot,  0,        0, jakks_gkr_sw_i2c, jak_sith_i2c, jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",     "Star Wars - Original Trilogy (JAKKS Pacific TV Game, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // was designed with SW keys in mind, but retail lacked the port
CONS( 2005, jak_dbz,   0,        0, jakks_gkr_1m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Handheld Games",          "Dragon Ball Z (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // DB (no game-keys released, 1 in development but cancelled)
CONS( 2005, jak_mpac,  0,        0, jakks_gkr_nm_i2c, jak_nm_i2c,   jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man 5-in-1 (Ms. Pac-Man, Pole Position, Galaga, Xevious, Mappy) (JAKKS Pacific TV Game, Game-Key Ready) (07 FEB 2005 A SKU F)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses NM (3 keys available [Dig Dug, New Rally-X], [Rally-X, Pac-Man, Bosconian], [Pac-Man, Bosconian])
CONS( 2005, jak_capc,  0,        0, jakks_gkr_cc_i2c, jak_cc_i2c,   jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Capcom / HotGen Ltd",     "Capcom 3-in-1 (1942, Commando, Ghosts'n Goblins) (JAKKS Pacific TV Game, Game-Key Ready) (29 MAR 2005 B)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses CC keys (no game-keys released)
CONS( 2005, jak_wof,   0,        0, jakks_gkr_wf_i2c, jak_wf_i2c,   jakks_gkr_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",              "Wheel of Fortune (JAKKS Pacific TV Game, Game-Key Ready) (Jul 11 2005 ORIG)",  MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // uses WF keys (no game-keys released)  analog wheel not emulated
// There is a 'Second Edition' version of Wheel of Fortune with a Gold case, GameKey port removed, and a '2' over the usual Game Key Ready logo, internals are different too, not Game-Key Ready
CONS( 2004, jak_spdm,  0,        0, jakks_gkr_mv_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse",         "Spider-Man (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) //  MV (1 key available)
CONS( 2005, jak_pooh,  0,        0, jakks_gkr_wp,     jak_pooh,     jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Backbone Entertainment",  "Winnie the Pooh - Piglet's Special Day (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // WP (no game-keys released)
CONS( 2005, jak_care,  0,        0, jakks_gkr_cb,     jak_care,     jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Backbone Entertainment",  "Care Bears TV Games (JAKKS Pacific TV Game, Game-Key Ready)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // CB (no game-keys released)

// release version of X-Men is not dumped yet
CONS( 2005, jak_xmenp, 0,        0, jakks_gkr_2m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",     "X-Men - Mutant Reign (JAKKS Pacific TV Game, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Dreamworks Movie Night was never manufactured
CONS( 2006, jak_dwmn, 0,        0, jakks_gkr_2m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",      "Dreamworks Movie Night (JAKKS Pacific TV Game, Oct 18 2006, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, jak_dwmno,jak_dwmn, 0, jakks_gkr_2m_i2c, jak_gkr_i2c,  jakks_gkr_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",      "Dreamworks Movie Night (JAKKS Pacific TV Game, Apr 24 2006, test program)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Some versions of the Shrek - Over the Hedge unit show the GameKey logo on startup (others don't) there is no evidence to suggest it was ever released with a GameKey port tho, and the internal PCB has no place for one on the versions we've seen (which show the logo)
