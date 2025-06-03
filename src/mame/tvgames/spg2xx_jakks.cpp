// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Mortal Kombat - attract demo almost always picked Johnny Cage vs. Johhny Cage until some unknown factor starts to randomize it
//               - Scorpion's 'get over here' sounds don't decode well

#include "emu.h"
#include "spg2xx.h"
#include "machine/nvram.h"


namespace {

class jakks_state : public spg2xx_game_state
{
public:
	jakks_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void base_config(machine_config& config);
	void spg2xx_jakks(machine_config& config);
	void mk(machine_config& config);
	void spg2xx_dpma(machine_config& config);
	void jakks_mpaco(machine_config& config);
	void jakks_mpac(machine_config& config);
	void jakks_rapm(machine_config& config);
	void jakks_sesa(machine_config& config);
	void spg2xx_hmbb(machine_config& config);
	void spg2xx_wof2(machine_config& config);
	
private:
	void mem_map_2m_mkram(address_map &map) ATTR_COLD;
	void mem_map_hmbb(address_map &map) ATTR_COLD;
	void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	u16 wof2_wheel_r()
	{
		// bits 0x0003 are input?
		// read in the 4096hz timer interrupt
		// but the timer interrupt causes glitches on the wheel spin screen
		// is there an interrupt priority issue?
		u16 ret = 0x0003;
		logerror("%s: wof2_wheel_r returning %04x\n", machine().describe_context(), ret);
		return ret;
	}
};


void jakks_state::portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(mem_mask, 1))
		if (m_i2cmem) m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		if (m_i2cmem) m_i2cmem->write_sda(BIT(data, 0));
}

static INPUT_PORTS_START( batman )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("X Button")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_jakks )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED ) // Button 3 if used
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED ) // Button 4 if used
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_dond )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )        
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )     
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("No Deal Button")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Deal Button")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spg2xx_jpdy )
	PORT_INCLUDE( spg2xx_jakks )

	PORT_MODIFY("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("P1 Answer")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("P2 Answer")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(3) PORT_NAME("P3 Answer")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
INPUT_PORTS_END

static INPUT_PORTS_START( jak_spd3 )
	PORT_INCLUDE( spg2xx_jakks )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("C Button")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Menu / Pause")

INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_avtr )
	PORT_INCLUDE( spg2xx_jakks )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_xmen )
	PORT_INCLUDE( spg2xx_jakks )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("X Button")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_supm )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x01ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_spdv )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("B Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_dpma )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mk )
	PORT_START("P1")
	PORT_BIT( 0x001f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(1) PORT_NAME("Pause / Menu")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(1) PORT_NAME("Block (alt)") // which one of these is actually connected to the button?
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(1) PORT_NAME("Block")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("High Kick")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("High Punch")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("Low Kick")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("Low Punch")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")

	PORT_START("P3") // In addition to the "M/T" pad documented below, PCB also has "P/N" (PAL / NTSC) pad (not read?) and a "F/S" pad (also not read?)
	PORT_BIT( 0x0fff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x1000, 0x1000, "Blood" ) // see code at 05EC30 - "M/T" (Mature / Teen?) pad on PCB, set at factory
	PORT_CONFSETTING(      0x0000, "Disabled" )
	PORT_CONFSETTING(      0x1000, "Enabled" )
	PORT_BIT( 0x6000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_CONFNAME( 0x8000, 0x8000, "Link State" ) // see code at 05EA54
	PORT_CONFSETTING(      0x8000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_mpaco )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range) (note, range is different to GKR unit)
	PORT_BIT(0x03ff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x03ff)
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_sesa )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED ) 
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Red Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Blue Button")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) // no SEEPROM on this
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // PAL/NTSC flag, set to PAL
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jak_mpac )
	PORT_INCLUDE( jak_mpaco )

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( jak_spac )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jak_rapm )
	PORT_INCLUDE( jak_spac )

	PORT_START("DIALX") // for Pole Position, joystick can be twisted like a dial/wheel (limited?) (check range)
	PORT_BIT(0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_MINMAX(0x00,0x0fff)
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_pacg )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_gdg )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("Joypad Up")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("Joypad Down")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("Joypad Left")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("Joypad Right")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("A Button")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )  
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED ) // no SEEPROM on this
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_ntoonsc )
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu / Pause")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x0006, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PAL/NTSC flag, set to NTSC (unverified here)
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spg2xx_1vs )
	PORT_INCLUDE( spg2xx_jakks )
	PORT_MODIFY("P1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("C Button")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Menu / Pause")
INPUT_PORTS_END

void jakks_state::base_config(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	I2C_24C04(config, m_i2cmem, 0);
}

void jakks_state::spg2xx_jakks(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
}

void jakks_state::spg2xx_dpma(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
}

void jakks_state::jakks_sesa(machine_config& config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	spg2xx_base(config);

	m_maincpu->set_pal(true);
	m_screen->set_refresh_hz(50);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));
	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m);
}



void jakks_state::mem_map_2m_mkram(address_map &map)
{
	map(0x000000, 0x1fffff).bankr("cartbank");
	map(0x3e0000, 0x3fffff).ram().share("nvram"); // backed up by the CR2032
}

void jakks_state::mem_map_hmbb(address_map &map)
{
	map(0x000000, 0x3fffff).bankr("cartbank");
	map(0x3e0000, 0x3fffff).ram();
}

void jakks_state::mk(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_2m_mkram);

	m_maincpu->porta_in().set_ioport("P1");
	//m_maincpu->portb_in().set(FUNC(jakks_state::base_portb_r));
	m_maincpu->portc_in().set_ioport("P3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);
}

void jakks_state::spg2xx_hmbb(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_hmbb);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	I2C_24C16(config, m_i2cmem, 0);
}


void jakks_state::jakks_mpaco(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(jakks_state::base_porta_r));
	m_maincpu->portc_in().set_ioport("P3");
	m_maincpu->portc_out().set(FUNC(jakks_state::portc_w));

	m_maincpu->set_addrmap(AS_PROGRAM, &jakks_state::mem_map_1m);
	m_maincpu->adc_in<0>().set_ioport("DIALX");
}


void jakks_state::jakks_mpac(machine_config &config)
{
	jakks_mpaco(config);
	I2C_24C04(config, m_i2cmem, 0); 
	m_maincpu->adc_in<0>().set_ioport("DIALX");
}

void jakks_state::jakks_rapm(machine_config &config)
{
	spg2xx_jakks(config);
	m_maincpu->adc_in<0>().set_ioport("DIALX");
}

void jakks_state::spg2xx_wof2(machine_config &config)
{
	spg2xx_jakks(config);
	m_maincpu->portb_in().set(FUNC(jakks_state::wof2_wheel_r));
}

ROM_START( jak_batm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "batman.bin", 0x000000, 0x400000, CRC(46f848e5) SHA1(5875d57bb3fe0cac5d20e626e4f82a0e5f9bb94c) )
ROM_END

ROM_START( jak_wall )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkwalle.u3", 0x000000, 0x400000, CRC(bf9b0a7a) SHA1(d998d940c8a80b32d535120e58fddb3e414ef455) )
ROM_END

ROM_START( jak_potc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "pirates.u5", 0x000000, 0x400000, CRC(935fe66c) SHA1(8b5b11c61b7f32c313aa46e33a1c918ed82f7916) )
ROM_END

ROM_START( jak_cind )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkscinderella.u5", 0x000000, 0x200000, CRC(73008a51) SHA1(89168bc2c64836daa341a6fbacb1fa63c2fef14b) )
ROM_END

ROM_START( jak_slpb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_sleepingbeauty.u5", 0x000000, 0x200000, CRC(e5b20a73) SHA1(3c305c4b9265d9bbf090805daaf26ad43af54389) )
ROM_END

ROM_START( jak_hm1m )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkshmoiam.u4", 0x000000, 0x400000, CRC(38f0ec0c) SHA1(15458aa3de77776a9e3419073a8af7f92e403b6e) )
ROM_END

ROM_START( jak_avtr )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksavatar.u2", 0x000000, 0x400000, CRC(e6f3fd64) SHA1(fe364eab4fb8d729a9c1c6a640779e54ad34fb24) )
ROM_END

ROM_START( jak_supm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "superman.u3", 0x000000, 0x400000, CRC(626bdd85) SHA1(605b3193c17f606d2de5689f045b50ac0b7ff024) )
ROM_END

ROM_START( jak_sbjd )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "spongebobjelly.bin", 0x000000, 0x400000, CRC(804fbd87) SHA1(519aa7fada993837cb57fce26a1d721547af1861) )
ROM_END

ROM_START( jak_mk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// Sources indicate this should use a 6MB ROM.  The ROM here dosen't end on a blank fill and even the ROM checksum listed in the header seems to be about 50% off.
	// However no content actually seems to be missing, so are sources claiming a 6MB ROM just incorrect, with the checksum also being misleading?
	ROM_LOAD16_WORD_SWAP( "jakmk.bin", 0x000000, 0x400000, CRC(b7d7683e) SHA1(e54a020ee746d240267ef78bed7aea744351b421) )
ROM_END

ROM_START( jak_mpacw )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wirelessnamco.bin", 0x000000, 0x200000, CRC(78a318ca) SHA1(3c2601cbb023edb6a1f3d4bce686e0be1ef63eee) )
ROM_END

ROM_START( jak_mpacq )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_qvc7in1.u2", 0x000000, 0x200000, CRC(a0a132a0) SHA1(acb5a98dfb597151015929bdd5b6eada1764e2fe) )
ROM_END

ROM_START( jak_mpaco )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_mspacmanrev1.u1", 0x000000, 0x100000, CRC(de4c2262) SHA1(2a5bacdcd10ccfdb1ae792d0aaaf2d38ec936f94) )
ROM_END

ROM_START( jak_rapm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_retroarcade.u3", 0x000000, 0x400000, CRC(f2dcb1c8) SHA1(2361b8598279bc6642ca997bbf2072a9e6ac045e) )
ROM_END

ROM_START( jak_pacg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspacmangold.u2", 0x000000, 0x200000, CRC(59904862) SHA1(f86a8847742fa918bdfa0fa927ec6e1f573ce31c) )
ROM_END

ROM_START( jak_spac ) // this is actually built as a gamekey, with the header text in it, sources suggest it was also going to be released as one
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkssuperpacman.u2", 0x000000, 0x100000, CRC(490db7cc) SHA1(9b599f28b9fa8e4a502ae7a167c410d4738b8e7a) )
ROM_END

ROM_START( jak_spd3 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "spiderman3.u4", 0x000000, 0x200000, CRC(87019271) SHA1(80d126af970236a1cecf7ade49f916caf8f67ceb) )
ROM_END

ROM_START( jak_powr ) // shows Game-Key screen but was never produced with a GK slot
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "powerrangers.u2", 0x000000, 0x200000, CRC(859c6cff) SHA1(12bb08657e333c2644d707deead8cd3e34a140b2) )
ROM_END

ROM_START( jak_pix )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspixar.bin", 0x000000, 0x200000, CRC(ec110f2b) SHA1(a57e1d45cfb537173f94d1a95323183a62976bb4) )
ROM_END

ROM_START( jak_shrk )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkshrek.u2", 0x000000, 0x400000, CRC(4abb910d) SHA1(e663abd2ba6eacca2f4a046d8b8ebac2cf3fd36a) )
ROM_END

ROM_START( jak_marv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakmarvelheroes.u4", 0x000000, 0x200000, CRC(e63259a3) SHA1(8745f3071ba460c8ba4a5bd376a72154b2edd8bd) )
ROM_END

ROM_START( jak_spdv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkspiderweb.u2", 0x000000, 0x200000, CRC(408c94bc) SHA1(350f7b84abf3d0d56f081647b3a228505751ff70) )
ROM_END

ROM_START( jak_dpma )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks_magicaladventures.u2", 0x000000, 0x200000, CRC(3c3fdf54) SHA1(9847412a0ee21819cc714ca1a2dd519edb892c95) )
ROM_END

ROM_START( jak_hsm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkshighschoolmusical.u3", 0x000000, 0x400000, CRC(ad83549a) SHA1(1d198209055e418402c9f3f389bccde156ca5a43) )
ROM_END

ROM_START( jak_ctah )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkscheetahgirls.u4", 0x000000, 0x400000, CRC(cab222c2) SHA1(2fda5599d86850ffaaa35824c7788d153bd1d615) )
ROM_END

ROM_START( jak_dwa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdorawa.u3", 0x000000, 0x200000, CRC(fd519d3a) SHA1(a68a0dca5b722d83258c452a1c9243d4e9cb9de0) )
ROM_END

ROM_START( jak_5thg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakks5thgrader.u2", 0x000000, 0x200000, CRC(d460d360) SHA1(b6547a5ba93b40dcd7c993c4dfc917c1f415f4f0) )
ROM_END

ROM_START( jak_gdg )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksgodiegogo.u4", 0x000000, 0x200000, CRC(eaffbf9f) SHA1(7a415b817c8fed44569e1b66a42e80de9a81d4de) )
ROM_END

ROM_START( jak_sesa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkssesamestreet.u2", 0x000000, 0x400000, CRC(bae5f43a) SHA1(24e7875cea236e51c10013a33cc41f3c1816514c) )
ROM_END

ROM_START( jak_ntsc )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkssummercamp.bin", 0x000000, 0x200000, CRC(4b6711a0) SHA1(694eb97be72233adb5de322dfc00633c4db79113) )
ROM_END

ROM_START( jak_wof2 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkswheeloffortun2nd.u4", 0x000000, 0x200000, CRC(9eff79a8) SHA1(342b736aba13705d63f49f58bb3a2ef2505620a4) )
ROM_END

ROM_START( jak_dond )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksdond.u4", 0x000000, 0x200000, CRC(0ae0706f) SHA1(6144190d126b36378c05b6e0a633ab2b53b3fa39) )
ROM_END

ROM_START( jak_1vs )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	// byte 0xb5a27 originally gave 0xff with consistent reads (but bytesum was wrong)
	// with different settings gave 0xfe and consistent reads (bytesum in header matches)
	// ingame checksum screen seems to be broken (gives black screen after calculating)
	ROM_LOAD16_WORD_SWAP( "jakks1vs100.u3", 0x000000, 0x200000, CRC(4a8cdedf) SHA1(e6d035262d9b022e9cd19f2322ba6ef1a1db7b38) )
ROM_END

ROM_START( jak_jpdy )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakksjeopardy.u2", 0x000000, 0x200000, CRC(6ccf00b7) SHA1(51e6fe60f3169e52d03f46469923253231a7262e) )
ROM_END

ROM_START( jak_hmbb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkhmbobw.bin", 0x000000, 0x800000, CRC(4e72cbf9) SHA1(efcec76da39c8373e8ef769c4fa0a35d379896d8) )
ROM_END

ROM_START( jak_xmen )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "jakkxmen.u3", 0x000000, 0x400000, CRC(f4383dca) SHA1(9850782e498ab0036850aa1f8926299c658a6099) )
ROM_END

ROM_START( jak_xmenp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "xmen.bin", 0x000000, 0x400000, CRC(1fa271e0) SHA1(c32652e9eddf82ab496e3609f8fa444e447fb509) )
ROM_END

ROM_START( jak_dwmn )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dreamworks.bin", 0x000000, 0x400000, CRC(3ae9f786) SHA1(46451be3af459fbdb75d1155b3817543afe183d5) )
ROM_END

ROM_START( jak_dwmno )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "dw_spg300_test.bin", 0x000000, 0x400000, CRC(1ca2817b) SHA1(39ae519457c102c4420fae3699b2db0557ef1cf5) )
ROM_END

} // anonymous namespace


// Pre-GameKey units

CONS( 2004, jak_batm, 0, 0, spg2xx_jakks,  batman,        jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "The Batman (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

// this is an older unit than the jak_mpac Game Key Ready set and features no GameKey branding
CONS( 2004, jak_mpacw, 0,         0, jakks_mpac, jak_mpac,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 7-in-1 (JAKKS Pacific TV Game) (wireless, 18 AUG 2004 A)", MACHINE_IMPERFECT_SOUND )
CONS( 2004, jak_mpacq, jak_mpacw, 0, jakks_mpac, jak_mpac,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 7-in-1 (JAKKS Pacific TV Game) (QVC version, 12 JUL 2004 A)", MACHINE_IMPERFECT_SOUND )

CONS( 2004, jak_mpaco, jak_mpac,  0, jakks_mpaco, jak_mpaco,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd",      "Ms. Pac-Man Collection 5-in-1 (JAKKS Pacific TV Game) (01 APR 2004 A)", MACHINE_IMPERFECT_SOUND )

// you could link 2 pads of this together for 2 player mode as you could with WWE (feature not emulated)
CONS( 2004, jak_mk,   0, 0, mk,     mk,     jakks_state, empty_init, "JAKKS Pacific Inc / Digital Eclipse", "Mortal Kombat (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

// Post-GameKey units (all of these still have GameKey references in the code even if the physical connector was no longer present on the PCB)

// This was available in 2 different case styles, initially an underwater / jellyfish themed one, then later
// reissued in a 'SpongeBob head' style case reminiscent of the undumpable 2003 SpongeBob plug and play but
// with 2 buttons in the top left corner instead of 1
//
// The software on both versions of Jellyfish Dodge is believed to be the same, the build date can be seen in
// the 'hidden' test mode.
//
// A further updated version of this, adapted for touch controls, was released as a 'TV Touch' unit, see
// spg2xx_jakks_tvtouch.cpp
CONS( 2007, jak_sbjd, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "SpongeBob SquarePants Jellyfish Dodge (JAKKS Pacific TV Game) (Apr 5 2007)", MACHINE_IMPERFECT_SOUND )

CONS( 2008, jak_wall, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Wall-E (JAKKS Pacific TV Game) (Dec 18 2007 11:34:25)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_potc, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Pirates of the Caribbean - Islands of Fortune (JAKKS Pacific TV Game) (Jun 1 2007 12:34:28)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_supm, 0, 0, spg2xx_jakks,  jak_supm,      jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Superman in Super Villain Showdown (JAKKS Pacific TV Game) (26 Jan 2006 A)", MACHINE_IMPERFECT_SOUND ) // has AT24C04

CONS( 2006, jak_spdv, 0, 0, spg2xx_jakks,  spg2xx_spdv,   jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "The Amazing Spider-Man in Villain Round-Up (JAKKS Pacific TV Game) (24 Apr 2006 A)", MACHINE_IMPERFECT_SOUND )

// the physical 2nd edition does't have a dpad, you need to use the wheel to navigate (although the inputs still work in emulation)
// test mode button code is unknown (changed from usual HotGen code due to lack of dpad) routine is at 0xd5fd
CONS( 2007, jak_wof2, 0, 0, spg2xx_wof2,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",      "Wheel of Fortune - 2nd Edition (JAKKS Pacific TV Game) (Mar 15 2007 PAK2)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_pacg, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Arcade Gold featuring Pac-Man (20 APR 2007 A SKU O)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_spac, 0, 0, spg2xx_jakks,  jak_mpac,      jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Super Pac-Man Collection (26 JAN 2006 A SKU L)", MACHINE_IMPERFECT_SOUND )

CONS( 2008, jak_rapm, 0, 0, jakks_rapm,    jak_rapm,      jakks_state, empty_init, "JAKKS Pacific Inc / Namco / HotGen Ltd", "Retro Arcade featuring Pac-Man (20 AUG 2008 A SKU N)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_slpb, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",  "Sleeping Beauty - Tales of Enchantment (JAKKS Pacific TV Game) (Sep 17 2007 14:45:02)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_hm1m, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",  "Hannah Montana - One in a Million (JAKKS Pacific TV Game) (Aug 13 2007 15:42:29)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_avtr, 0, 0, spg2xx_jakks,  spg2xx_avtr,   jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",  "Avatar: The Last Airbender - Book One Challenges (JAKKS Pacific TV Game) (06 Jun 2006 A)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_hmbb, 0, 0, spg2xx_hmbb,   spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",  "Hannah Montana - Best of Both Worlds (JAKKS Pacific TV Game) (Aug 17 2007 22:47:47)", MACHINE_IMPERFECT_SOUND )

// from a PAL unit, and seems to have timing issues on the audio (speech cutting off / starting before previous has finished) when using an NTSC machine config, so maybe the NTSC ROM is different?
// test mode combination isn't the usual HotGen one, but can be accessed by setting a breakpoint at 0xa6ba and setting r2 to 0x0a - TODO: figure out combination so version can be checked against an NTSC unit.
CONS( 2006, jak_sesa, 0, 0, jakks_sesa,    spg2xx_sesa,   jakks_state, empty_init, "JAKKS Pacific Inc / HotGen Ltd",  "Sesame Street Beat (JAKKS Pacific TV Game) (Aug 23 2006 19:12:03, PAL/UK)", MACHINE_IMPERFECT_SOUND )


CONS( 2005, jak_powr, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Power Rangers S.P.D. (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_pix,  0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Disney Pixar Classics (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

// menu sounds don't work until you go into a game (work after a reset, bad default initializations?)
CONS( 2006, jak_shrk, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Dreamworks Shrek / Over The Hedge (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_marv, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Marvel Heroes: Ultimate Action (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_ntsc, 0, 0, spg2xx_jakks,  spg2xx_ntoonsc,jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Nicktoons - Summer Camp (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_dpma, 0, 0, spg2xx_dpma,  spg2xx_dpma,    jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Disney Princess Magical Adventures (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_hsm,  0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "High School Musical (JAKKS Pacific TV Game) (Dec 19 2007 17:08:20)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_ctah, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "The Cheetah Girls - Passport to Fame (JAKKS Pacific TV Game) (Aug 1 2007 10:32:50)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_spd3, 0, 0, spg2xx_jakks,  jak_spd3,      jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Spider-Man 3 (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_cind, 0, 0, spg2xx_jakks,  spg2xx_pacg,   jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Cinderella - Once Upon a Midnight (JAKKS Pacific TV Game) (Aug 29 2007 11:15:55)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_dwa,  0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Dora the Explorer - Dora's World Adventure! (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_5thg, 0, 0, spg2xx_jakks,  spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Handheld Games",  "Are You Smarter than a 5th Grader? (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_jpdy, 0, 0, spg2xx_jakks,  spg2xx_jpdy,   jakks_state, empty_init, "JAKKS Pacific Inc / 5000ft, Inc",  "Jeopardy! (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_gdg,  0, 0, spg2xx_dpma,   spg2xx_gdg,    jakks_state, empty_init, "JAKKS Pacific Inc / 1st Playable Productions",  "Go Diego Go! (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2006, jak_dond, 0, 0, spg2xx_jakks,  jak_dond,      jakks_state, init_crc,   "JAKKS Pacific Inc / Pronto Games",    "Deal or No Deal (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2007, jak_1vs,  0, 0, spg2xx_jakks,  spg2xx_1vs,    jakks_state, init_crc,   "JAKKS Pacific Inc / Pronto Games",    "1 Vs 100 (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )

CONS( 2005, jak_xmen, 0,        0, spg2xx_jakks,  spg2xx_xmen,  jakks_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",     "X-Men - Mutant Reign (JAKKS Pacific TV Game)", MACHINE_IMPERFECT_SOUND )
CONS( 2005, jak_xmenp,jak_xmen, 0, spg2xx_jakks,  spg2xx_xmen,  jakks_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",     "X-Men - Mutant Reign (JAKKS Pacific TV Game, prototype)", MACHINE_IMPERFECT_SOUND )

// Dreamworks Movie Night was never manufactured
CONS( 2006, jak_dwmn, 0,        0, spg2xx_jakks, spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",      "Dreamworks Movie Night (JAKKS Pacific TV Game, Oct 18 2006, prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2006, jak_dwmno,jak_dwmn, 0, spg2xx_jakks, spg2xx_jakks,  jakks_state, empty_init, "JAKKS Pacific Inc / Amaze Entertainment",      "Dreamworks Movie Night (JAKKS Pacific TV Game, Apr 24 2006, test program)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
