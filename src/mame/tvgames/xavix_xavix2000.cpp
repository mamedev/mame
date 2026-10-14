// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Driver for XaviX2000 hardware titles. Defines board configurations, inputs,
    ROM maps and registrations around the extended 6502-derived CPU. A
    decapped CPU die is marked "SSD 2000 NEC 85605-621".

    The CPU adds opcodes absent from the 1997/1998 parts. Known XaviX2000
    systems do not use the SuperXaviX bitmap modes or 16-bit ROM interface.
    Some systems in xavix.cpp may use this type; without decapping, the extra
    opcodes are the only known software-visible distinction.

    Optical sensor hardware observations:
    - Lord of the Rings and Star Wars have identical-looking 32x32 sensors;
      Dragon Quest looks similar to Lord of the Rings.
    - The sensor dies are labeled CU5501 for Lord of the Rings and CU5501A
      for Star Wars and Dragon Quest, apparently from ETOMS.

    Test-mode entry:
    - ban_omt, ban_onep: Hold Select Up and Select Down from startup, then
      soft reset and release.
    - ttv_mx: Hold Motion Up, Brake and Accelerator from startup, then soft
      reset and release.
    - tom_jump: Hold Motion Up, Dash and Brake from startup, then soft reset
      and release.

    BTANB:
    - tak_chq: Two red road-tile pixels remain visible at the right border at
      race start, also seen in hardware footage: https://www.youtube.com/watch?v=vTAt31e7X7Q

    TODO:
    - duelmast: Verify sensor timing and physical image alignment.
    - epo_sdb: Measure physical trackball scaling.
    - epo_sdb, epo_es2j and duelmast: Measure physical battery-voltage scaling.
    - epo_ebox and epo_sdb: Identify remaining status inputs and factory-test contacts.
    - Shared CPU identification: Confirm chip generations where extended
      opcodes are the only evidence; epo_ebox has independent hardware identification.

***************************************************************************/

#include "emu.h"
#include "xavix.h"

#include "cpu/m6502/xavix2000.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"

#include "softlist_dev.h"

// #define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  Shared Hardware
//**************************************************************************

void xavix_state::xavix2000(machine_config &config)
{
	XAVIX2000_SOC(config, m_soc, NTSC_MASTER_CLOCK);
	configure_board(config);
	m_palette->set_entries(512);
}


void xavix_state::xavix2000_4mb(machine_config &config)
{
	xavix2000(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_4mb_extbus_map);
}

void xavix_state::xavix2000_nv(machine_config &config)
{
	xavix2000(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}

void xavix_state::xavix2000_nv_4mb(machine_config &config)
{
	xavix2000_nv(config);
	m_soc->set_external_map(*this, &xavix_state::xavix_4mb_extbus_map);
}

//--------------------------------------------------------------------------
//  EEPROM Connections
//--------------------------------------------------------------------------

void xavix_i2c_state::xavix2000_i2c_24c02(machine_config &config)
{
	xavix2000(config);

	I2C_24C02(config, "i2cmem");
}

void xavix_i2c_state::xavix2000_i2c_24c04(machine_config &config)
{
	xavix2000(config);

	I2C_24C04(config, "i2cmem");
}

void xavix_i2c_state::xavix2000_i2c_24c04_2mb(machine_config &config)
{
	xavix2000_i2c_24c04(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_2mb_extbus_map);
}

void xavix_i2c_state::xavix2000_i2c_24c04_4mb(machine_config &config)
{
	xavix2000_i2c_24c04(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_4mb_extbus_map);
}

void xavix_i2c_state::xavix2000_i2c_24c08(machine_config &config)
{
	xavix2000(config);
	I2C_24C08(config, "i2cmem");
}

void xavix_i2c_state::xavix2000_i2c_24c08_4mb(machine_config &config)
{
	xavix2000_i2c_24c08(config);
	m_soc->set_external_map(*this, &xavix_i2c_state::xavix_4mb_extbus_map);
}

//--------------------------------------------------------------------------
//  Camera Connections
//--------------------------------------------------------------------------

void xavix2000_i2c_camera_state::configure_camera(machine_config &config, xavix_analog_image_sensor_device::target_shape shape)
{
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	m_camera->set_target_shape(shape);
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}

void xavix2000_i2c_camera_state::xavix2000_i2c_24c02_camera(machine_config &config)
{
	xavix2000_i2c_24c02(config);
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::ROD);
}

u8 xavix2000_i2c_camera_state::io1_input_r(u8 direction)
{
	return (m_in1->read() & ~0x06) | m_camera->status_r();
}

void xavix2000_i2c_camera_state::io0_output_w(u8 data, u8 direction)
{
	m_camera->setup_w(data, direction);
}

void xavix2000_i2c_camera_state::io1_output_w(u8 data, u8 direction)
{
	xavix_i2c_state::io1_output_w(data, direction);
	m_camera->control_w(data, direction);
}

//**************************************************************************
//  Standalone Input Layouts
//**************************************************************************

// MX Dirt Rebel
static INPUT_PORTS_START( ttv_mx )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Motion Up") // you tilt the device, but actual inputs are digital
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Motion Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Motion Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Motion Right")

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// TV de Asobitai IDATEN Jump: Gekisou IDATEN Battle
static INPUT_PORTS_START( tom_jump )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dash")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Jump")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Motion Up") // you tilt the device, but actual inputs are digital
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Motion Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Motion Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Motion Right")

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


//**************************************************************************
//  System Implementations
//**************************************************************************

// Super Shot! Excite Golf
static INPUT_PORTS_START( epo_golf )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_epo_golf(machine_config &config)
{
	xavix2000_i2c_24c04_4mb(config);
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::DISK);
}


// Strike Kimeruze! Excite Bowling
static INPUT_PORTS_START( epo_bowl )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_epo_bowl(machine_config &config)
{
	xavix2000_i2c_24c04_2mb(config);
	// The ball's reflective patch faces the floor console; the ROM derives the throw from its motion.
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::DISK);
}


// mini-moni. Party! Rhythm de Pyon!
static INPUT_PORTS_START( epo_mini )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	// During gameplay, the ROM also accepts O + Right to return directly to the title screen.

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_epo_mini(machine_config &config)
{
	xavix2000_i2c_24c08_4mb(config);
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock()).set_target_count(4);
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


// Star Wars: Lightsaber Battle Game (both regions)
static INPUT_PORTS_START( ttv_sw )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // Retail menu selection and confirmation use the reflective sword.

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_ttv_sw(machine_config &config)
{
	xavix2000_i2c_24c02_camera(config);
	m_camera->set_target_y_clipped(true);
}


// The Lord of the Rings: Warrior of Middle-Earth
static INPUT_PORTS_START( ttv_lotr )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // Retail controls use the reflective sword, not the factory-test contacts.

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


// Kenshin Dragon Quest: Yomigaerishi Densetsu no Ken
static INPUT_PORTS_START( drgqst )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // Pointing, slashing, guarding and near-camera magic use the sword image.

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_drgqst(machine_config &config)
{
	xavix2000_i2c_24c08(config);
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::ROD);
}


// Tottoko Hamtaro: Ham-Ham Dai Circus!
static INPUT_PORTS_START( epo_hamc )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Select Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Select Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Select Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Select Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
INPUT_PORTS_END

void xavix2000_camera_state::xavix2000_epo_hamc(machine_config &config)
{
	xavix2000_4mb(config);
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock()).set_target_count(2);
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}

u8 xavix2000_camera_state::io1_input_r(u8 direction)
{
	return (m_in1->read() & ~0x06) | m_camera->status_r();
}

void xavix2000_camera_state::io0_output_w(u8 data, u8 direction)
{
	m_camera->setup_w(data, direction);
}

void xavix2000_camera_state::io1_output_w(u8 data, u8 direction)
{
	m_camera->control_w(data, direction);
}


// Let's! TV Play Taikan Kakutou One Piece Punch Battle: Kaizoku Ou ni Kimi ga Naru!
static INPUT_PORTS_START( ban_onep )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Select Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Select Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_ban_onep(machine_config &config)
{
	xavix2000_i2c_24c04(config);
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock()).set_target_count(2);
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


// Let's! TV Play Touin Ougi Onmyou Taisenki
static INPUT_PORTS_START( ban_omt )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Select Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Select Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_ban_omt(machine_config &config)
{
	xavix2000_i2c_24c04_4mb(config);
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::DISK);
	// A compact starting target avoids continually requesting the ROM's near-camera action.
	m_camera->set_target_size_small(true);
	// Representative timing leaves time to rebuild the sprite list before the next field.
	m_camera->set_half_pixel_clocks(40);
}


// TV de Asobitai Disney Princess: Kirakira Mahou no Lesson
static INPUT_PORTS_START( tom_dpgm )
	PORT_INCLUDE(xavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_INCLUDE(xavix_soft_power_control) // The physical POWER button is known; its electrical wiring remains unverified.
INPUT_PORTS_END

void xavix2000_i2c_camera_state::xavix2000_tom_dpgm(machine_config &config)
{
	xavix2000_i2c_24c08_4mb(config);
	configure_camera(config, xavix_analog_image_sensor_device::target_shape::DISK);
	m_camera->set_target_size_small(true);
	// Provisional pixel timing keeps paired capture and game updates within one field.
	m_camera->set_half_pixel_clocks(40);
}


// K.O. Shiyouze! Excite Boxing
static INPUT_PORTS_START( epo_ebox )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("O / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("X / Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Used by service and boot paths; physical identity unknown.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("gloves", FUNC(xavix_ebox_gloves_device::data_r))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Auto-off policy input; physical source unknown.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Used as a rising-edge control; physical identity unknown.
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_hard_power_control)
	PORT_INCLUDE(xavix_reset_control)
INPUT_PORTS_END

void xavix2000_epo_ebox_state::xavix2000_epo_ebox(machine_config &config)
{
	xavix2000_nv_4mb(config);

	XAVIX_EBOX_GLOVES(config, m_gloves, peripheral_clock());
	m_gloves->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_gloves, FUNC(xavix_ebox_gloves_device::vblank_w));
}


// Super Dash Ball
static INPUT_PORTS_START( epo_sdb )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("MOUSE0X")
	// Relative scaling is provisional because the original counts per distance have not been measured.
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P1 Trackball X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_X_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE0Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("P1 Trackball Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1) PORT_CODE(MOUSECODE_Y_INDEXED(0)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_NAME("P2 Trackball X") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_X_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("P2 Trackball Y") PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(2) PORT_CODE(MOUSECODE_Y_INDEXED(1)) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(INPUT_CODE_INVALID)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Factory-test chord with no identified public controls
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Jump / Confirm") PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_PLAYER(2)
	// The documented adapter and proven idle-policy split support this identity, but a factory strap cannot be excluded.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Jump / Confirm") PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("AN7")
	// Representative differences select the ROM's three threshold bands; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix_state::xavix2000_epo_sdb(machine_config &config)
{
	xavix2000_nv_4mb(config);

	// Each ball-and-roller controller feeds two writable wrapping counters; both Y channels have reversed polarity.
	m_soc->anport().input_callback<0>().set_ioport("MOUSE0X");
	m_soc->anport().input_callback<1>().set_ioport("MOUSE0Y");
	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().input_callback<3>().set_ioport("MOUSE1Y");
	m_soc->anport().set_counter_mode(0);
	m_soc->anport().set_counter_mode(1, true);
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);
}


// Choro-Q Byun Byun Racer
static INPUT_PORTS_START( tak_chq )
	PORT_INCLUDE(xavix_i2c)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Brake") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Accelerator") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Brake") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Accelerator") PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Command") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Command") PORT_PLAYER(2)

	// The firmware consumes two's-complement offsets, with zero at wheel centre.
	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_NAME("P1 Steering Wheel") PORT_MINMAX(0x80, 0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(8) PORT_CENTERDELTA(8) PORT_PLAYER(1)
	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_NAME("P2 Steering Wheel") PORT_MINMAX(0x80, 0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(8) PORT_CENTERDELTA(8) PORT_PLAYER(2)
INPUT_PORTS_END

void xavix_i2c_state::xavix2000_tak_chq(machine_config &config)
{
	xavix2000_i2c_24c04_4mb(config);

	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().input_callback<3>().set_ioport("MOUSE1Y");
}


// Card Scan! Excite Stage Soccer Nippon Daihyou Team
static INPUT_PORTS_START( epo_es2j )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 S / Shoot") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 P / Short Pass") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 G / Ground Kick") PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 L / Long Kick") PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 S / Shoot") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 P / Short Pass") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 G / Ground Kick") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 L / Long Kick") PORT_PLAYER(2)

	PORT_MODIFY("AN7")
	// Representative values select all three native states; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1c, "Low" )
	PORT_CONFSETTING(    0x24, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void xavix2000_epo_es2j_state::xavix2000_epo_es2j(machine_config &config)
{
	xavix2000_4mb(config);

	XAVIX_EPO_ES2J_CARD_READER(config, m_card_reader, peripheral_clock());
	m_card_reader->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<2>));
}

u8 xavix2000_epo_es2j_state::io0_input_r(u8 direction)
{
	u8 result = 0;
	u8 const select = m_soc->io().data_latch<1>() & m_soc->io().direction<1>();
	if (BIT(select, 0))
		result |= m_controllers[0]->read();
	if (BIT(select, 1))
		result |= m_controllers[1]->read();
	return result;
}


// Duel Masters: Duel Station
static INPUT_PORTS_START( duelmast )
	PORT_INCLUDE(xavix)
	PORT_INCLUDE(xavix_soft_power_control)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Scan / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the imager.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))

	PORT_MODIFY("AN5")
	// Representative AN5 values select all three native states while AN7 remains at its measured full-scale reference.
	PORT_CONFNAME( 0xff, 0xff, "Battery Status" )
	PORT_CONFSETTING(    0xff, "Normal" )
	PORT_CONFSETTING(    0xe3, "Low" )
	PORT_CONFSETTING(    0xdf, "Very Low" )

	PORT_MODIFY("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void xavix2000_duelmast_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(xavix2000_duelmast_state::cart_r), FUNC(xavix2000_duelmast_state::cart_w));
	map(0x408000, 0x40ffff).ram(); // seems to expect RAM here (at least when cart is enabled)
}

void xavix2000_duelmast_state::xavix2000_duelmast(machine_config &config)
{
	xavix2000(config);

	I2C_24C04(config, "i2cmem");

	XAVIX_DUELMAST_SCANNER(config, m_scanner, peripheral_clock());
	m_soc->adc().input_callback<0>().set(m_scanner, FUNC(xavix_analog_image_sensor_device_base::pixel_r));

	XAVIX_CART_SLOT(config, m_cartslot, xavix_cart_slot_device::cart_devices, nullptr);
	// Reference data is used only by the optional manual card-selection aid.
	m_scanner->card_data_callback().set([this](offs_t offset) { return m_cartslot->has_cart() ? m_cartslot->read_cart(0x0c8000 + offset) : m_rom[0x020000 + offset]; });
	m_scanner->expanded_cards_callback().set([this]() -> int { return m_cartslot->has_cart(); });
	SOFTWARE_LIST(config, "cart_list_duelmast").set_original("duelmast_cart");
}

u8 xavix2000_duelmast_state::io1_input_r(u8 direction)
{
	return (m_in1->read() & ~0x06) | m_scanner->status_r();
}

void xavix2000_duelmast_state::io0_output_w(u8 data, u8 direction)
{
	m_scanner->setup_w(data, direction);
}

void xavix2000_duelmast_state::io1_output_w(u8 data, u8 direction)
{
	xavix_i2c_cart_state::io1_output_w(data, direction);
	m_scanner->control_w(data, direction);
}

//**************************************************************************
//  ROM Definitions
//**************************************************************************

ROM_START( epo_sdb )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("superdashball.bin", 0x000000, 0x400000, CRC(a004a764) SHA1(47a96822d4d7d6a0f6be5cd729c3747dbab65979) )
ROM_END

ROM_START( epo_ebox )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("exciteboxing.bin", 0x000000, 0x400000, CRC(e25ae4f5) SHA1(7f7b613f0ab8f43f5cad0d13de538921e77cae9c) )
ROM_END

ROM_START( epo_bowl )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bowling.bin", 0x000000, 0x200000, CRC(d34f8d9e) SHA1(ebe3792172dc43904b9226beb27f1da89d2388cc) )
ROM_END

ROM_START( epo_golf ) // GLFJ MAIN-03
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("golf.bin", 0x000000, 0x400000, CRC(d1f231cf) SHA1(9421836a6bc4af9ee1fc7a402d62b2fb4dbcdefc) )
ROM_END

ROM_START( epo_hamc ) // ET158 MB REV.0
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD( "hamster.u1", 0x000000, 0x400000, CRC(b1177813) SHA1(ed01096ebb63b72267ad7e0b2115224bbab64011) )
ROM_END

ROM_START( ban_omt ) // OMTJ MAIN-07
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("otmj.bin", 0x000000, 0x400000, CRC(1c1dc6fb) SHA1(d0cf1345b765d66ca9a0870ee6d0e3ccd84a8c0b) )
ROM_END

ROM_START( ttv_sw )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jedi.bin", 0x000000, 0x800000, CRC(51cae5fd) SHA1(1ed8d556f31b4182259ca8c766d60c824d8d9744) )
ROM_END

ROM_START( ttv_swj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lightsaber.bin", 0x000000, 0x800000, CRC(a5c22ed0) SHA1(406f0bccb01cd4a26fe4a5675d7ebecc78c58147) )
ROM_END


ROM_START( ttv_lotr )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lotr.bin", 0x000000, 0x800000, CRC(a034ecd5) SHA1(264a9d4327af0a075841ad6129db67d82cf741f1) )
ROM_END

ROM_START( ttv_mx )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "mxdirtrebel.bin", 0x000000, 0x800000, CRC(e64bf1a1) SHA1(137f97d7d857697a13e0c8984509994dc7bc5fc5) )
ROM_END

ROM_START( tom_jump )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tom_jump.bin", 0x000000, 0x800000, CRC(20bf5c17) SHA1(bca7535baa6a54ad3ee0929bd3b74a22cb5139da) )
ROM_END


ROM_START( drgqst )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "dragonquest.bin", 0x000000, 0x800000, CRC(3d24413f) SHA1(1677e81cedcf349de7bf091a232dc82c6424efba) )
ROM_END

ROM_START( epo_mini )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "minimoni.u1", 0x000000, 0x400000, CRC(2adb01ee) SHA1(987218b6799195ba15adf39885c1d177c381ec26) )
ROM_END

ROM_START( tak_chq )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "choroq.u2", 0x000000, 0x400000, CRC(ffd2eb95) SHA1(a30884da5554483ebfd0009cf5dd1768be8a99cb) )
ROM_END

ROM_START( ban_onep )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("onepiece.bin", 0x000000, 0x800000, CRC(c5cb5a5f) SHA1(db85f6cc48d77c5a4967b9b8e2999167e3dfc8c8) )
ROM_END

ROM_START( duelmast )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("duelmasters.u4", 0x000000, 0x200000, CRC(2f11fcd7) SHA1(d8849c74833e77b8b309e845523f2cdc7ac68054) )
	ROM_RELOAD(0x200000,0x200000)
	ROM_RELOAD(0x400000,0x200000)
	ROM_RELOAD(0x600000,0x200000)
ROM_END

ROM_START( tom_dpgm )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("disney.bin", 0x000000, 0x400000, CRC(1dc181b3) SHA1(fa30069d17705f27e4ff45e7f6ccf06986e138f3) )
ROM_END

ROM_START( epo_es2j ) // ES2J MAIN-01  2005 date on PCB, 2006 ingame
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("es2j.u3", 0x000000, 0x400000, CRC(840aecb1) SHA1(ad52449ffc13af5f4c67b2c3cf438e7ecd80b9fb) )
ROM_END



//**************************************************************************
//  System Registrations
//**************************************************************************

// K.O.しようぜ！エキサイトボクシング
CONS( 2002, epo_ebox, 0,      0, xavix2000_epo_ebox,         epo_ebox, xavix2000_epo_ebox_state,   empty_init, "Epoch / SSD Company Ltd.", "K.O. Shiyouze! Excite Boxing (Japan)", MACHINE_NOT_WORKING ) // doesn't use XaviX2000 extra opcodes, but had that type of CPU

// ストライクきめるぜ！ エキサイトボウリング
CONS( 2002, epo_bowl, 0,      0, xavix2000_epo_bowl,         epo_bowl, xavix2000_i2c_camera_state, empty_init, "Epoch / SSD Company Ltd.", "Strike Kimeruze! Excite Bowling (Japan)", MACHINE_NOT_WORKING )

// スーパーショット！ エキサイトゴルフ
// 2002 date on PCB, 2003 in-game.
CONS( 2003, epo_golf, 0,      0, xavix2000_epo_golf,         epo_golf, xavix2000_i2c_camera_state, empty_init, "Epoch / SSD Company Ltd.", "Super Shot! Excite Golf (Japan)", MACHINE_NOT_WORKING )

// とっとこハム太郎 ハムハム大サーカス！
CONS( 2002, epo_hamc, 0,      0, xavix2000_epo_hamc,         epo_hamc, xavix2000_camera_state,     empty_init, "Epoch / SSD Company Ltd.", "Tottoko Hamtaro: Ham-Ham Dai Circus! (Japan)", MACHINE_NOT_WORKING )

// ミニモニ。パーティ！リズムでぴょん！
CONS( 2003, epo_mini, 0,      0, xavix2000_epo_mini,         epo_mini, xavix2000_i2c_camera_state, empty_init, "Epoch / SSD Company Ltd.", "mini-moni. Party! Rhythm de Pyon! (Japan)", MACHINE_NOT_WORKING )

// カードスキャン!　エキサイトステージ サッカー日本代表チーム
CONS( 2006, epo_es2j, 0,      0, xavix2000_epo_es2j,         epo_es2j, xavix2000_epo_es2j_state,   empty_init, "Epoch / SSD Company Ltd.", "Card Scan! Excite Stage Soccer Nippon Daihyou Team (Japan)", MACHINE_NOT_WORKING )

/* Card images: https://dm.takaratomy.co.jp/card (use the complete upright front, not an artwork crop).
   Boot with an empty scanner for cover calibration, then load the card image in File Manager.
   This is a product in the Duel Masters line called Duel Station; the boot up screen calls it Duel Station, title logo is Duel Masters */
CONS( 2003, duelmast, 0,      0, xavix2000_duelmast,         duelmast, xavix2000_duelmast_state,   empty_init, "Takara / SSD Company Ltd.", "Duel Masters: Duel Station (Japan)", MACHINE_NOT_WORKING )

// スーパーダッシュボール
CONS( 2004, epo_sdb,  0,      0, xavix2000_epo_sdb,          epo_sdb,  xavix_state,                empty_init, "Epoch / SSD Company Ltd.", "Super Dash Ball (Japan)", 0 )

CONS( 2005, ttv_sw,   0,      0, xavix2000_ttv_sw,           ttv_sw,   xavix2000_i2c_camera_state, empty_init, "Hasbro / Tiger Games / SSD Company Ltd.", "Star Wars Saga Edition: Lightsaber Battle Game!", MACHINE_NOT_WORKING )
// テレビで遊び隊 STAR WARS LIGHTSABER BATTLE ～ジェダイの戦い～
CONS( 2005, ttv_swj,  ttv_sw, 0, xavix2000_ttv_sw,           ttv_sw,   xavix2000_i2c_camera_state, empty_init, "Tomy / SSD Company Ltd.", "TV de Asobitai Star Wars: Lightsaber Battle - Jedi no Tatakai (Japan)", MACHINE_NOT_WORKING )

CONS( 2005, ttv_lotr, 0,      0, xavix2000_i2c_24c02_camera, ttv_lotr, xavix2000_i2c_camera_state, empty_init, "Hasbro / Tiger Games / SSD Company Ltd.", "The Lord of the Rings: Warrior of Middle-Earth", MACHINE_NOT_WORKING )

CONS( 2005, ttv_mx,   0,      0, xavix2000_i2c_24c04,        ttv_mx,   xavix_i2c_state,            empty_init, "Hasbro / Tiger Games / SSD Company Ltd.", "MX Dirt Rebel", 0 )

// テレビで遊び隊　韋駄天翔 激走 韋駄天バトル - seems to be based on the same engine at ttv_mx and has an almost identical controller, but not exactly the same game
CONS( 2005, tom_jump, 0,      0, xavix2000_i2c_24c04,        tom_jump, xavix_i2c_state,            empty_init, "Tomy / SSD Company Ltd.", "TV de Asobitai IDATEN Jump: Gekisou IDATEN Battle (Japan)", 0 )

// 剣神ドラゴンクエスト 甦りし伝説の剣
CONS( 2003, drgqst,   0,      0, xavix2000_drgqst,           drgqst,   xavix2000_i2c_camera_state, empty_init, "Square Enix / SSD Company Ltd.", "Kenshin Dragon Quest: Yomigaerishi Densetsu no Ken (Japan)", MACHINE_NOT_WORKING )

// チョロＱビュンビュンレーサー
CONS( 2003, tak_chq,  0,      0, xavix2000_tak_chq,          tak_chq,  xavix_i2c_state,            empty_init, "Takara / SSD Company Ltd.", "Choro-Q Byun Byun Racer (Japan)", MACHINE_NOT_WORKING )

// Let’s！TV プレイ　体感格闘ワンピースパンチバトル 　～海賊王にキミがなる！～
CONS( 2004, ban_onep, 0,      0, xavix2000_ban_onep,         ban_onep, xavix2000_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Taikan Kakutou One Piece Punch Battle: Kaizoku Ou ni Kimi ga Naru! (Japan)", MACHINE_NOT_WORKING )

// Let’s！TV プレイ　闘印奥義　陰陽大戦記　～目指せ最強闘神士～ - 2004 date on PCB, 2005 in-game
CONS( 2005, ban_omt,  0,      0, xavix2000_ban_omt,          ban_omt,  xavix2000_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Touin Ougi Onmyou Taisenki: Mezase Saikyou Toushinshi (Japan)", MACHINE_NOT_WORKING )

// テレビで遊び隊 ディズニープリンセス　キラキラ魔法のレッスン
CONS( 2004, tom_dpgm, 0,      0, xavix2000_tom_dpgm,         tom_dpgm, xavix2000_i2c_camera_state, empty_init, "Tomy / SSD Company Ltd.", "TV de Asobitai Disney Princess: Kirakira Mahou no Lesson (Japan)", MACHINE_NOT_WORKING )
