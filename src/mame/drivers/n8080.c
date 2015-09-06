// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 hardware

    - Space Fever
    - Space Fever High Splitter (aka SF-Hisplitter)
    - Space Launcher
    - Sheriff / Bandido / Western Gun 2
    - Helifire

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/n8080.h"

#define MASTER_CLOCK    XTAL_20_16MHz


WRITE8_MEMBER(n8080_state::n8080_shift_bits_w)
{
	m_shift_bits = data & 7;
}
WRITE8_MEMBER(n8080_state::n8080_shift_data_w)
{
	m_shift_data = (m_shift_data >> 8) | (data << 8);
}


READ8_MEMBER(n8080_state::n8080_shift_r)
{
	return m_shift_data >> (8 - m_shift_bits);
}

static ADDRESS_MAP_START( main_cpu_map, AS_PROGRAM, 8, n8080_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( helifire_main_cpu_map, AS_PROGRAM, 8, n8080_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("colorram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, n8080_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ(n8080_shift_r)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN3")

	AM_RANGE(0x02, 0x02) AM_WRITE(n8080_shift_bits_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(n8080_shift_data_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(n8080_sound_1_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(n8080_sound_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(n8080_video_control_w)
ADDRESS_MAP_END


/* Input ports */

static INPUT_PORTS_START( spacefev )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game A") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game B") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game C") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* enables diagnostic ROM at $1c00 */

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("IN3")

INPUT_PORTS_END


static INPUT_PORTS_START( highsplt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game A") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game B") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game C") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* enables diagnostic ROM at $2000 */

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("IN3")

INPUT_PORTS_END


static INPUT_PORTS_START( spacelnc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game A") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game B") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game C") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* enables diagnostic ROM at $2000 */

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "3000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "8000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("IN3")

INPUT_PORTS_END


static INPUT_PORTS_START( sheriff )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP1 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP2 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP3 enables diagnostic ROM at $2400 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )    // Switches 3-7 are UNUSED
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ))   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
INPUT_PORTS_END


static INPUT_PORTS_START( bandido )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP1 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP2 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP3 enables diagnostic ROM at $2400 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown )) /* don't know if this is used */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END


static INPUT_PORTS_START( westgun2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) /* enables diagnostic ROM at $2400 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")

INPUT_PORTS_END


static INPUT_PORTS_START( helifire )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP1 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP2 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) /* EXP3 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x04, "6000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x0c, "10000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))

	/* potentiometers */
	PORT_START("POT0")
	PORT_DIPNAME( 0xff, 0x50, "VR1 sun brightness" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x70, "70" )

	PORT_START("POT1")
	PORT_DIPNAME( 0xff, 0x00, "VR2 sea brightness" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x70, "70" )
INPUT_PORTS_END


/* Interrupts */

TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::rst1_tick)
{
	int state = m_inte ? ASSERT_LINE : CLEAR_LINE;

	/* V7 = 1, V6 = 0 */
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, state, 0xcf);
}

TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::rst2_tick)
{
	int state = m_inte ? ASSERT_LINE : CLEAR_LINE;

	/* vblank */
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, state, 0xd7);
}

WRITE_LINE_MEMBER(n8080_state::n8080_inte_callback)
{
	m_inte = state;
}

WRITE8_MEMBER(n8080_state::n8080_status_callback)
{
	if (data & I8085_STATUS_INTA)
	{
		/* interrupt acknowledge */
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

void n8080_state::machine_start()
{
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_bits));
	save_item(NAME(m_inte));
}

MACHINE_RESET_MEMBER(n8080_state,n8080)
{
	m_shift_data = 0;
	m_shift_bits = 0;
	m_inte = 0;
}

MACHINE_RESET_MEMBER(n8080_state,spacefev)
{
	MACHINE_RESET_CALL_MEMBER(n8080);

	m_spacefev_red_screen = 0;
	m_spacefev_red_cannon = 0;
}

MACHINE_RESET_MEMBER(n8080_state,sheriff)
{
	MACHINE_RESET_CALL_MEMBER(n8080);

	m_sheriff_color_mode = 0;
	m_sheriff_color_data = 0;
}

MACHINE_RESET_MEMBER(n8080_state,helifire)
{
	MACHINE_RESET_CALL_MEMBER(n8080);

	m_helifire_mv = 0;
	m_helifire_sc = 0;
	m_helifire_flash = 0;
}


static MACHINE_CONFIG_START( spacefev, n8080_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, MASTER_CLOCK / 10)
	MCFG_I8085A_STATUS(WRITE8(n8080_state,n8080_status_callback))
	MCFG_I8085A_INTE(WRITELINE(n8080_state,n8080_inte_callback))
	MCFG_CPU_PROGRAM_MAP(main_cpu_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_MACHINE_RESET_OVERRIDE(n8080_state,spacefev)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 16, 239)
	MCFG_SCREEN_UPDATE_DRIVER(n8080_state, screen_update_spacefev)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(n8080_state,n8080)
	MCFG_VIDEO_START_OVERRIDE(n8080_state,spacefev)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst1", n8080_state, rst1_tick, "screen", 128, 256)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst2", n8080_state, rst2_tick, "screen", 240, 256)

	/* sound hardware */
	MCFG_FRAGMENT_ADD( spacefev_sound )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( sheriff, n8080_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, MASTER_CLOCK / 10)
	MCFG_I8085A_STATUS(WRITE8(n8080_state,n8080_status_callback))
	MCFG_I8085A_INTE(WRITELINE(n8080_state,n8080_inte_callback))
	MCFG_CPU_PROGRAM_MAP(main_cpu_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_MACHINE_RESET_OVERRIDE(n8080_state,sheriff)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 16, 239)
	MCFG_SCREEN_UPDATE_DRIVER(n8080_state, screen_update_sheriff)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(n8080_state,n8080)
	MCFG_VIDEO_START_OVERRIDE(n8080_state,sheriff)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst1", n8080_state, rst1_tick, "screen", 128, 256)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst2", n8080_state, rst2_tick, "screen", 240, 256)

	/* sound hardware */
	MCFG_FRAGMENT_ADD( sheriff_sound )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( westgun2, sheriff )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", I8080, XTAL_19_968MHz / 10)
	MCFG_I8085A_STATUS(WRITE8(n8080_state,n8080_status_callback))
	MCFG_I8085A_INTE(WRITELINE(n8080_state,n8080_inte_callback))
	MCFG_CPU_PROGRAM_MAP(main_cpu_map)
	MCFG_CPU_IO_MAP(main_io_map)

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( helifire, n8080_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, MASTER_CLOCK / 10)
	MCFG_I8085A_STATUS(WRITE8(n8080_state,n8080_status_callback))
	MCFG_I8085A_INTE(WRITELINE(n8080_state,n8080_inte_callback))
	MCFG_CPU_PROGRAM_MAP(helifire_main_cpu_map)
	MCFG_CPU_IO_MAP(main_io_map)

	MCFG_MACHINE_RESET_OVERRIDE(n8080_state,helifire)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 16, 239)
	MCFG_SCREEN_UPDATE_DRIVER(n8080_state, screen_update_helifire)
	MCFG_SCREEN_VBLANK_DRIVER(n8080_state, screen_eof_helifire)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8 + 0x400)
	MCFG_PALETTE_INIT_OWNER(n8080_state,helifire)
	MCFG_VIDEO_START_OVERRIDE(n8080_state,helifire)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst1", n8080_state, rst1_tick, "screen", 128, 256)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("rst2", n8080_state, rst2_tick, "screen", 240, 256)

	/* sound hardware */
	MCFG_FRAGMENT_ADD( helifire_sound )
MACHINE_CONFIG_END


/*
Space Fever (3 sets, Space Fever?, High Splitter?, Space Launcher?)
Nintendo, 1979

Note: These are all simple ROM swaps on a standard b/w Space Fever PCB.


PCB Layouts
-----------

Top Board (Sound PCB)

TSF-SOU
|----------------------------------------------------|
|                                 VR3    VR2    VR1  |
|  8035                    74123                     |
|                 74275                              |
|  6MHz                                              |
|                          74123         SN76477     |
|  SF_SOUND.IC2   74275                              |
|                                                    |
|                          7405                      |
|                                                    |
|----------------------------------------------------|
Notes:
      All IC's shown.
      There is no AMP on the PCB, sound amplification is done via a small external AMP board.
      ROM IC2 is a 2708 EPROM.
      VR1: master volume
      VR2: shoot volume
      VR3: music volume
      8035 clocks: pins 2 and 3 measure 6.000MHz
                   pin 9 measures 399.256kHz
                   pin 12 measures 200.0kHz
                   pin 13 measures 105.0kHz
                   pin 21 measures 399.4Khz
                   pin 22 measures 400.0kHz
                   pin 23 measures 399.3kHz
                   pin 24 measures 399.3kHz
                   pin 39 measures 61.5627Hz


Middle board
------------

TSF-I/O  PI-500803
|----------------------------------------------------|
|                                                    |
|     VR1                                            |
|                     20.160MHz                      |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                     DSW1(8)                        |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|----------------------------------------------------|
Notes:
      VR1: adjusts brightness
      Board contains mostly logic ICs (not shown)
      Video output is b/w, the harness is wired to a JAMMA fingerboard but only blue is used.


Bottom board
------------

TSF-CPU  PI-500802
|----------------------------------------------------|
|                                                    |
|                                                    |
|                                                    |
|             SF_F1.F1  SF_G1.G1  SF_H1.H1  SF_I1.I1 |
|                                                    |
|   8080                                             |
|                                                    |
|             SF_F2.F2  SF_G2.G2  SF_H2.H2  SF_I2.I2 |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|     4116  4116  4116  4116  4116  4116  4116  4116 |
|----------------------------------------------------|
Notes:
      All ROMs are 2708, 1K x8
      4116: 2K x8 DRAM
      8080 clock: 2.0160MHz (20.160 / 10)
      Sync: no V reading, H is 15.57kHz

      Set 1 is on the PCB and is complete.
      Some ROMs in set1 match the current sfeverbw set.

      The other two sets were supplied as just EPROMs.
      Set2 (maybe High Splitter) is missing the ROM at location I2. Might be missing, or maybe
      just the program is smaller and the extra ROM was not required.
*/

ROM_START( spacefev )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ro-.bin",  0x0000, 0x0400, CRC(35f295bd) SHA1(34d1df25fcdea598ca1191cecc2125e6f63dbce3) ) // "F1??"
	ROM_LOAD( "f2-ro-.bin",  0x0400, 0x0400, CRC(0c633f4c) SHA1(a551ddbf21670fb1f000404b92da87a97f7ba157) ) // "F2??"
	ROM_LOAD( "g1-ro-.bin",  0x0800, 0x0400, CRC(f3d851cb) SHA1(535c52a56e54a064aa3d1c48a129f714234a1007) ) // "G1??"
	ROM_LOAD( "g2-ro-.bin",  0x0c00, 0x0400, CRC(1faef63a) SHA1(68e1bfc45587bfb1ee2eb477b60efd4f69dffd2c) ) // "G2??"
	ROM_LOAD( "h1-ro-.bin",  0x1000, 0x0400, CRC(b365389d) SHA1(e681f2c5e37cc07912915ef74184ff9336309de3) ) // "H1??"
	ROM_LOAD( "h2-ro-.bin",  0x1400, 0x0400, CRC(a163e800) SHA1(e8817f3e17f099a0dc66213d2d3d3fdeb117b10e) ) // "H2??"
	ROM_LOAD( "i1-ro-p.bin", 0x1800, 0x0400, CRC(756b5582) SHA1(b7f3d218b7f4267ce6128624306396bcacb9b44e) ) // "I1??P"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
	ROM_END

ROM_START( spacefevo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ro-.bin",  0x0000, 0x0400, CRC(35f295bd) SHA1(34d1df25fcdea598ca1191cecc2125e6f63dbce3) ) // "F1??"
	ROM_LOAD( "f2-ro-.bin",  0x0400, 0x0400, CRC(0c633f4c) SHA1(a551ddbf21670fb1f000404b92da87a97f7ba157) ) // "F2??"
	ROM_LOAD( "g1-ro-.bin",  0x0800, 0x0400, CRC(f3d851cb) SHA1(535c52a56e54a064aa3d1c48a129f714234a1007) ) // "G1??"
	ROM_LOAD( "g2-ro-.bin",  0x0c00, 0x0400, CRC(1faef63a) SHA1(68e1bfc45587bfb1ee2eb477b60efd4f69dffd2c) ) // "G2??"
	ROM_LOAD( "h1-ro-.bin",  0x1000, 0x0400, CRC(b365389d) SHA1(e681f2c5e37cc07912915ef74184ff9336309de3) ) // "H1??"
	ROM_LOAD( "h2-ro-.bin",  0x1400, 0x0400, CRC(a163e800) SHA1(e8817f3e17f099a0dc66213d2d3d3fdeb117b10e) ) // "H2??"
	ROM_LOAD( "i1-ro-.bin",  0x1800, 0x0400, CRC(00027be2) SHA1(551a779a2e5a6455b7a348d246731c094e0ec709) ) // "I1??"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
	ROM_END

ROM_START( spacefevo2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-i-.bin",   0x0000, 0x0400, CRC(7fa305e8) SHA1(cda9fc9c76f57800de25ddf65f69fef19fd28481) ) // "F1?C"
	ROM_LOAD( "f2-i-.bin",   0x0400, 0x0400, CRC(7c1429aa) SHA1(8d8e0a4fc09fb1ecbfb86c67c20000ef30ab3fac) ) // "F2?C"
	ROM_LOAD( "g1-i-.bin",   0x0800, 0x0400, CRC(75f6efc1) SHA1(286bc75e35e8ad6277e9db7377e90731b9c2ec97) ) // "G1?C"
	ROM_LOAD( "g2-i-.bin",   0x0c00, 0x0400, CRC(fb6bcf4a) SHA1(3edea04d67c2f3b1a6a73adadea83ddda0be3842) ) // "G2?C"
	ROM_LOAD( "h1-i-.bin",   0x1000, 0x0400, CRC(3beef037) SHA1(4bcc157e7d721b3a9e16e7a2efa807303d4be8ac) ) // "H1?C"
	ROM_LOAD( "h2-i-.bin",   0x1400, 0x0400, CRC(bddbc94f) SHA1(f90cbc3cd0f695cbb9ae03b608f4bf5a4a000c64) ) // "H2?C"
	ROM_LOAD( "i1-i-.bin",   0x1800, 0x0400, CRC(437786c5) SHA1(2ccdb0d48dbbfe47ae82e970ca37970602405cf6) ) // "I1?C"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highsplt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "hs.h1",       0x1000, 0x0400, CRC(eefb4273) SHA1(853a62976a406516f10ac68dc2859399b8b7aae8) )
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "hs.i1",       0x1800, 0x0400, CRC(41e18df9) SHA1(2212c836313775e7c507a875672c0b3635825e02) )
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss4.bin",     0x0000, 0x0400, CRC(939e01d4) SHA1(7c9ccd24e5da03831cd0aa821da17e3b81cd8381) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highsplta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "h1-ha-.bin",  0x1000, 0x0400, CRC(b0505da3) SHA1(f7b1f3a6dd06ff0cdeb6b13c948b7a262592514a) ) // "H1?n"
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "i1-ha-.bin",  0x1800, 0x0400, CRC(aa36b25d) SHA1(28f555aab27b206a8c6f550b6caa938cece6e204) ) // "I1?n"
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss4.bin",     0x0000, 0x0400, CRC(939e01d4) SHA1(7c9ccd24e5da03831cd0aa821da17e3b81cd8381) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highspltb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "h1-ha-.bin",  0x1000, 0x0400, CRC(b0505da3) SHA1(f7b1f3a6dd06ff0cdeb6b13c948b7a262592514a) ) // "H1?n"
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "i1-ha-.bin",  0x1800, 0x0400, CRC(aa36b25d) SHA1(28f555aab27b206a8c6f550b6caa938cece6e204) ) // "I1?n"
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "ss4.ic2",     0x0000, 0x0400, CRC(ce95dc5f) SHA1(20f7b8c565c408439dcfae240b7d1aa42c29651b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( spacelnc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sl.f1",    0x0000, 0x0400, CRC(6ad59e40) SHA1(d416f7e6f5f55178df5c390548cd299650853022) )
	ROM_LOAD( "sl.f2",    0x0400, 0x0400, CRC(2de568e2) SHA1(f13740d3d9bf7434b7760e9286ef6e2ede40845f) )
	ROM_LOAD( "sl.g1",    0x0800, 0x0400, CRC(06d0ab36) SHA1(bf063100b065dbf511d6f32da169fb461568d15d) )
	ROM_LOAD( "sl.g2",    0x0c00, 0x0400, CRC(73ac4fe6) SHA1(7fa8c09692446bdf804900158e040f0b875a2e32) )
	ROM_LOAD( "sl.h1",    0x1000, 0x0400, CRC(7f42a94b) SHA1(ad85706de5e3f952b12756275be1ea1276a10666) )
	ROM_LOAD( "sl.h2",    0x1400, 0x0400, CRC(04b7a5f9) SHA1(589b0a0c8dcb1300623fe8478f1d7173b2bc575f) )
	ROM_LOAD( "sl.i1",    0x1800, 0x0400, CRC(d30007a3) SHA1(9e5905df8f7822385daef159a07f0e8257cb862a) )
	ROM_LOAD( "sl.i2",    0x1c00, 0x0400, CRC(640ffd2f) SHA1(65c21396c39dc99ec263f66f400a8e4c7712b20a) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "sl.snd",   0x0000, 0x0400, CRC(8e1ff929) SHA1(5c7da97b05fb8fff242158978199f5d35b234426) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sf.prm",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) )
ROM_END

ROM_START( sheriff )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sh.f1",    0x0000, 0x0400, CRC(e79df6e8) SHA1(908176de9bfc3d48e2da9af6ba7ebdee698ec2de) )
	ROM_LOAD( "sh.f2",    0x0400, 0x0400, CRC(da67721a) SHA1(ee6a5fb98da1d1fcfad0ef27af300473a637f578) )
	ROM_LOAD( "sh.g1",    0x0800, 0x0400, CRC(3fb7888e) SHA1(2c2d6b27d577d5ccf759e451e53c2e3314af40f6) )
	ROM_LOAD( "sh.g2",    0x0c00, 0x0400, CRC(585fcfee) SHA1(82f2abc14f893c092b80da45fc297fa5fb0890b5) )
	ROM_LOAD( "sh.h1",    0x1000, 0x0400, CRC(e59eab52) SHA1(aa87710237dd48d1831f1b307d547b1b0707cd4e) )
	ROM_LOAD( "sh.h2",    0x1400, 0x0400, CRC(79e69a6a) SHA1(1780ce77d7d9ddbf4aceabe0fcf079339837bbe1) )
	ROM_LOAD( "sh.i1",    0x1800, 0x0400, CRC(dda7d1e8) SHA1(bd2a7388e81c71922b2e97d68be71359a75e8d37) )
	ROM_LOAD( "sh.i2",    0x1c00, 0x0400, CRC(5c5f3f86) SHA1(25c64ccb7d0e136f67d6e1da7927ae6d89e0ceb9) )
	ROM_LOAD( "sh.j1",    0x2000, 0x0400, CRC(0aa8b79a) SHA1(aed139e8c8ba912823c57fe4cc7231b2d638f479) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "sh.snd",   0x0000, 0x0400, CRC(75731745) SHA1(538a63c9c60f1886fca4caf3eb1e0bada2d3f162) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) )
ROM_END

ROM_START( bandido )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sh-a.f1",  0x0000, 0x0400, CRC(aec94829) SHA1(aa6d241670ea061bac4a71dff82dfa832095eae6) )
	ROM_LOAD( "sh.f2",    0x0400, 0x0400, CRC(da67721a) SHA1(ee6a5fb98da1d1fcfad0ef27af300473a637f578) )
	ROM_LOAD( "sh.g1",    0x0800, 0x0400, CRC(3fb7888e) SHA1(2c2d6b27d577d5ccf759e451e53c2e3314af40f6) )
	ROM_LOAD( "sh.g2",    0x0c00, 0x0400, CRC(585fcfee) SHA1(82f2abc14f893c092b80da45fc297fa5fb0890b5) )
	ROM_LOAD( "sh-a.h1",  0x1000, 0x0400, CRC(5cb63677) SHA1(59a8e5f8b134bf44d3e5a1105a9346f0c5f9378e) )
	ROM_LOAD( "sh.h2",    0x1400, 0x0400, CRC(79e69a6a) SHA1(1780ce77d7d9ddbf4aceabe0fcf079339837bbe1) )
	ROM_LOAD( "sh.i1",    0x1800, 0x0400, CRC(dda7d1e8) SHA1(bd2a7388e81c71922b2e97d68be71359a75e8d37) )
	ROM_LOAD( "sh.i2",    0x1c00, 0x0400, CRC(5c5f3f86) SHA1(25c64ccb7d0e136f67d6e1da7927ae6d89e0ceb9) )
	ROM_LOAD( "sh.j1",    0x2000, 0x0400, CRC(0aa8b79a) SHA1(aed139e8c8ba912823c57fe4cc7231b2d638f479) )
	ROM_LOAD( "sh-a.j2",  0x2400, 0x0400, CRC(a10b848a) SHA1(c045f1f6a11cbf49a1bae06c701b659d587292a3) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "sh.snd",   0x0000, 0x0400, CRC(75731745) SHA1(538a63c9c60f1886fca4caf3eb1e0bada2d3f162) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) )
ROM_END

ROM_START( westgun2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rf01.ic36",    0x0000, 0x0800, CRC(7eabc538) SHA1(8f0540a5cb391b83aafc0365b44686628eae5c77) )
	ROM_LOAD( "rf02.ic35",    0x0800, 0x0800, CRC(3344d6a5) SHA1(ea2a8413401b53c9d1b9c653ac3a98855a35cce6) )
	ROM_LOAD( "rf03.ic34",    0x1000, 0x0800, CRC(d4bb08fd) SHA1(92c0821f259037b193658997289b6b41c6f67215) )
	ROM_LOAD( "rf04.ic33",    0x1800, 0x0800, CRC(60b71f0d) SHA1(10650426972afb0ccb964548a52879ed3f0b316a) )
	ROM_LOAD( "rf05.ic32",    0x2000, 0x0800, CRC(81e650fb) SHA1(e600567125294d1411fcad3a015edb98cee36ff8) )

	ROM_REGION( 0x0800, "audiocpu", 0 )
	ROM_LOAD( "rf06.ic35",   0x0000, 0x0800, CRC(4eafe957) SHA1(78e03402219c0ad814f63ae507eadc636d95f755) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) ) // rf07 not dumped, taken from parent
ROM_END

ROM_START( helifire )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "hf.f1",    0x0000, 0x0400, CRC(032f89ca) SHA1(63b0310875ed78a6385e44eea781ddcc4a63557c) )
	ROM_LOAD( "hf.f2",    0x0400, 0x0400, CRC(2774e70f) SHA1(98d845e80db61799493dbebe8db801567277432c) )
	ROM_LOAD( "hf.g1",    0x0800, 0x0400, CRC(b5ad6e8a) SHA1(1eb4931e85bd6a559e85a2b978d383216d3988a7) )
	ROM_LOAD( "hf.g2",    0x0c00, 0x0400, CRC(5e015bf4) SHA1(60f5a9707c8655e54a8381afd764856fb25c29f1) )
	ROM_LOAD( "hf.h1",    0x1000, 0x0400, CRC(23bb4e5a) SHA1(b59bc0adff3635aca1def2b1997f7edc6ca7e8ee) )
	ROM_LOAD( "hf.h2",    0x1400, 0x0400, CRC(358227c6) SHA1(d7bd678ef1737edc6aa609e43e3ae96a8d61dc15) )
	ROM_LOAD( "hf.i1",    0x1800, 0x0400, CRC(0c679f44) SHA1(cbe31dbe5f2c5f11a637cb3bde4e059c310d0e76) )
	ROM_LOAD( "hf.i2",    0x1c00, 0x0400, CRC(d8b7a398) SHA1(3ddfeac39147d5df6096f525f7ef67abef32a28b) )
	ROM_LOAD( "hf.j1",    0x2000, 0x0400, CRC(98ef24db) SHA1(70ad8dd6e1e8f4bf4ce431737ca1856eecc03d53) )
	ROM_LOAD( "hf.j2",    0x2400, 0x0400, CRC(5e2b5877) SHA1(f7c747e8a1d9fe2dda71ee6304636cf3cdf727a7) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "hf.snd",   0x0000, 0x0400, CRC(9d77a31f) SHA1(36db9b5087b6661de88042854874bc247c92d985) )
ROM_END

ROM_START( helifirea )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "hf-a.f1",  0x0000, 0x0400, CRC(92c9d6c1) SHA1(860a7b3980e9e11d48769fad347c965e04ed3f89) )
	ROM_LOAD( "hf-a.f2",  0x0400, 0x0400, CRC(a264dde8) SHA1(48f972ad5af6c2ab61117f60d9244df6df6d313c) )
	ROM_LOAD( "hf.g1",    0x0800, 0x0400, CRC(b5ad6e8a) SHA1(1eb4931e85bd6a559e85a2b978d383216d3988a7) )
	ROM_LOAD( "hf-a.g2",  0x0c00, 0x0400, CRC(a987ebcd) SHA1(46726293c308c18b28941809419ba4c2ffc8084f) )
	ROM_LOAD( "hf-a.h1",  0x1000, 0x0400, CRC(25abcaf0) SHA1(a14c795de1fc283405f71bb83f4ac5c98fd406cb) )
	ROM_LOAD( "hf.h2",    0x1400, 0x0400, CRC(358227c6) SHA1(d7bd678ef1737edc6aa609e43e3ae96a8d61dc15) )
	ROM_LOAD( "hf.i1",    0x1800, 0x0400, CRC(0c679f44) SHA1(cbe31dbe5f2c5f11a637cb3bde4e059c310d0e76) )
	ROM_LOAD( "hf-a.i2",  0x1c00, 0x0400, CRC(296610fd) SHA1(f1ab379983e45f3cd718dd82962c609297b4dcb8) )
	ROM_LOAD( "hf.j1",    0x2000, 0x0400, CRC(98ef24db) SHA1(70ad8dd6e1e8f4bf4ce431737ca1856eecc03d53) )
	ROM_LOAD( "hf.j2",    0x2400, 0x0400, CRC(5e2b5877) SHA1(f7c747e8a1d9fe2dda71ee6304636cf3cdf727a7) )

	ROM_REGION( 0x0400, "audiocpu", 0 )
	ROM_LOAD( "hf.snd",   0x0000, 0x0400, CRC(9d77a31f) SHA1(36db9b5087b6661de88042854874bc247c92d985) )
ROM_END


GAME( 1979, spacefev,   0,        spacefev, spacefev, driver_device, 0, ROT270, "Nintendo", "Space Fever (New Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacefevo,  spacefev, spacefev, spacefev, driver_device, 0, ROT270, "Nintendo", "Space Fever (Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacefevo2, spacefev, spacefev, spacefev, driver_device, 0, ROT270, "Nintendo", "Space Fever (Older Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, highsplt,   0,        spacefev, highsplt, driver_device, 0, ROT270, "Nintendo", "Space Fever High Splitter (set 1)", MACHINE_SUPPORTS_SAVE ) // known as "SF-Hisplitter" on its flyer
GAME( 1979, highsplta,  highsplt, spacefev, highsplt, driver_device, 0, ROT270, "Nintendo", "Space Fever High Splitter (set 2)", MACHINE_SUPPORTS_SAVE ) // known as "SF-Hisplitter" on its flyer
GAME( 1979, highspltb,  highsplt, spacefev, highsplt, driver_device, 0, ROT270, "Nintendo", "Space Fever High Splitter (alt Sound)", MACHINE_SUPPORTS_SAVE ) // known as "SF-Hisplitter" on its flyer
GAME( 1979, spacelnc,   0,        spacefev, spacelnc, driver_device, 0, ROT270, "Nintendo", "Space Launcher", MACHINE_SUPPORTS_SAVE )
GAME( 1979, sheriff,    0,        sheriff,  sheriff, driver_device,  0, ROT270, "Nintendo", "Sheriff", MACHINE_SUPPORTS_SAVE )
GAME( 1980, bandido,    sheriff,  sheriff,  bandido, driver_device,  0, ROT270, "Nintendo (Exidy license)", "Bandido", MACHINE_SUPPORTS_SAVE )
GAME( 1980, westgun2,   sheriff,  westgun2, westgun2, driver_device, 0, ROT270, "Nintendo (Taito Corporation license)", "Western Gun Part II", MACHINE_SUPPORTS_SAVE ) // official Taito PCBs, but title/copyright not shown
GAME( 1980, helifire,   0,        helifire, helifire, driver_device, 0, ROT270, "Nintendo", "HeliFire (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, helifirea,  helifire, helifire, helifire, driver_device, 0, ROT270, "Nintendo", "HeliFire (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
