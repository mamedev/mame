// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Driver for SuperXaviX systems, including XaviXPORT, Domyos and TV-PC.
    Defines board ROM maps, cartridges, inputs, EEPROM/RTC connections and
    external-memory adapters around the shared SoC devices.

    A decapped CPU die is marked "SSD 2002 NEC 85054-611". Hardware additions
    include 16-bit ROM support, higher-resolution bitmap output, interlaced
    screen modes and additional PIO. An undumped XaviXPORT Golf unit reportedly
    contains a die marked "SSD 2003 SuperXaviX MXIC 2003 3009".

    XaviXPORT is a TV adapter; the CPU, video, sound and controller hardware
    reside in its cartridges and controllers and can vary between games.

    The Domyos Interactive System (DiS), released in France by Decathlon,
    appears to use the same platform for PAL regions with a different
    software range.

    XaviXPORT and Domyos cartridge hardware observations:

    - XaviX Tennis and XaviX Baseball are the simplest: just the CPU, x16 ROM
      and 24C08 SEEPROM. Bowling and Boxing also have 4 IR LEDs and a 32x32 sensor.
      XaviX Fishing has ROM, 24C08 and a 24-pin daughterboard with a Nordic
      nRF24E1 2.4GHz 8051-based SoC.

    - The remaining cartridges described below include an S35390 I2C clock
      chip with crystal and battery backup, and have two x16 ROM chips, using
      a 5-pin single-gate inverter to create complementary /OE signals.

    - J-MAT, Fitness Exercise and Fitness Challenge have a 24CS64 SEEPROM.

    - Fitness Dance has an Atmel H93864C (maybe SEEPROM?), a Microchip
      dsPIC33FJ12GP202 and two JRC 2740 dual op amps.

    - Music and Circuit has a 24CS64, two UTC324 quad op amps, a 74HC14,
      a 74HCT04, and an 8-pin SOIC labeled 61545, which is likely an M61545
      dual electronic volume control.

    Optical sensor hardware observations:
    - The 32x32 sensors appear to be from ETOMS. Most of the PCBs are marked
      CIS and connect to the main PCB with an 18-20 wire ribbon cable.
    - Bowling and Boxing look identical; both sensor dies are labeled CU5501A.

    Test-mode entry:
    - xavtenni, xavtennij, xavbaseb, xavbasebj, xavgolf, xavgolfj, xavbowl,
      xavbowlj, xavbassf, xavbox, xavboxj, epo_tfit: Hold Console Up and
      Console Down from startup, then soft reset and release.
    - anpanmdx: Hold Blue Rock / Shokupanman, Left Footprint / Baikinman and
      Green Paper / Cream Panda from startup and soft reset about 5.7 seconds
      later, then release.
    - apmj2009: Hold Baikinman / Left Footprint, Melonpanna / Pink Triangle and
      Shokupanman / Red Square from startup and soft reset about 7.7 seconds
      later, then release.
    - ban_ordj: Hold Triangle Up and Triangle Down from startup, then soft
      reset and release.
    - epo_doka: Hold Back and Pad 1 / Secret Gadget from startup, then soft
      reset and release.
    - ban_bkgj: Hold Rock / Confirm and Paper / Previous from startup and soft
      reset about 6.5 seconds later, then release. Leave Scissors / Next
      unpressed.

    TODO:
    - Shared IR controllers (e.g. xavtenni/xavtennij, xavbaseb/xavbasebj,
      epo_tfit and anpanmdx): Measure transmitter timing, repetition and
      physical motion thresholds.
    - ban_ordj, xavcheck, ban_bkgj and ndpmj: Identify remaining receiver/factory
      status wiring. ban_bkgj: Measure the physical power-cutoff delay.
    - domdance: Dump the dsPIC firmware; onset detection and level scaling
      are approximate.
    - doradraw/hkdraw: Confirm the provisional MBM29DL161TE flash identification.
    - udance: Measure sensor-side tracking, including overlapping reflectors.
    - xavgolf/xavgolfj and xavbassf: Verify physical swing/reel polarity.
    - xavbaseb/xavbasebj, anpanmdx, ban_ordj and ban_bkgj: Identify the remaining
      auxiliary/factory report meanings and physical accessory behaviour.
    - Shared battery monitoring (e.g. doradraw/hkdraw, udance and epo_ntpj):
      Measure physical voltage scaling.

***************************************************************************/

/***************************************************************************

    Notes from Video Game Console Library (thanks Guru!):
    (** this source is not authoritative: XaviX Tennis appears to use Super XaviX,
        whose emulated CPU is 6502-derived rather than 65816; see the other driver notes)

    XaviXPORT arrived on the scene with 3 game titles (XaviX Tennis, XaviX Bowling and XaviX Baseball) using their
    original XaviX Multiprocessor.  This proprietary chip is reported to contain an 8-bit high speed central processing
    unit (6502) at 21 MHz, picture processor, sound processor, DMA controller, 1K bytes high speed RAM, universal timer,
    AD/Converter and I/O device control.  Each cartridge comes with a wireless peripheral to be used with the game (Baseball Bat,
    Tennis Racquet, etc.) that requires "AA" batteries.  The XaviXPORT system retailed for $79.99 USD with the cartridges
    retailing for $49.99 USD.

    The following year at CES 2005, SSD COMPANY LIMITED introduced two new XaviXPORT titles (XaviX Golf and XaviX Bass Fishing) each
    containing the upgraded "Super XaviX".  This new chip is said to sport a 16-bit high central processing unit (65816) at 43 MHz.
    SSD COMPANY LIMITED is already working on their next chip called "XaviX II" that is said to be a 32-bit RISC processor
    with 3D capabilities.

***************************************************************************/

#include "emu.h"
#include "xavix_superxavix.h"

#include "xavix_io_step_mat.h"

#include "bus/xavix/domyos_step.h"
#include "cpu/m6502/xavix2000.h"

#include "softlist_dev.h"

#include <algorithm>
#include <cmath>

#include "epo_ntpj.lh"

#define LOG_PIO (1U << 1)
#define LOG_ANALYSER (1U << 2)
#define VERBOSE 0
#include "logmacro.h"

//**************************************************************************
//  Shared Hardware
//**************************************************************************

void superxavix_state::mapped_program_map(address_map &map)
{
	map(0x000000, 0xffffff).r(FUNC(superxavix_state::mapped_code_r));
	map(0x000000, 0x003fff).w(FUNC(superxavix_state::debug_mem_w));
}

void superxavix_power_state::configure_board(machine_config &config)
{
	superxavix_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_power_state::io1_power_off_w));
}

void superxavix_i2c_power_state::configure_board(machine_config &config)
{
	superxavix_i2c_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_i2c_power_state::io1_power_off_w));
}

void superxavix_state::superxavix(machine_config &config)
{
	m_soc = SUPERXAVIX_SOC(config, m_super_soc, NTSC_MASTER_CLOCK);
	configure_board(config);
	m_super_soc->set_graphics_read_callback(*this, FUNC(superxavix_state::read_graphics_byte));
	m_super_soc->set_bitmap_read_callback(*this, FUNC(superxavix_state::read_bitmap_byte));

	m_palette->set_entries(512);

	superxavix_video_device::configure_screen(*m_screen, m_soc->clock(), video_standard::NTSC);

	m_super_soc->plotter().memory_read_callback().set(*this, FUNC(superxavix_state::read_plotter_byte));
	m_super_soc->plotter().memory_write_callback().set(*this, FUNC(superxavix_state::write_plotter_byte));

	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_state::pio_input_r<0>));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_state::pio_output_w<0>));
	m_super_soc->pio().input_callback<1>().set(*this, FUNC(superxavix_state::pio_input_r<1>));
	m_super_soc->pio().output_callback<1>().set(*this, FUNC(superxavix_state::pio_output_w<1>));
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_state::pio_input_r<2>));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_state::pio_output_w<2>));
	m_super_soc->pio().input_mask_callback<0>().set(*this, FUNC(superxavix_state::pio_input_mask_r<0>));
	m_super_soc->pio().input_mask_callback<1>().set(*this, FUNC(superxavix_state::pio_input_mask_r<1>));
	m_super_soc->pio().input_mask_callback<2>().set(*this, FUNC(superxavix_state::pio_input_mask_r<2>));
	m_super_soc->pio().legacy_input_mask_callback().set(*this, FUNC(superxavix_state::legacy_input_mask_r));
}

void superxavix_state::superxavix_4mb(machine_config &config)
{
	superxavix(config);
	m_soc->set_external_map(*this, &superxavix_state::xavix_4mb_extbus_map);
}

//-------------------------------------------------
//  Memory Access
//-------------------------------------------------

void superxavix_state::configure_mapped_memory()
{
	m_soc->set_program_map(*this, &superxavix_state::mapped_program_map);
	// Tile planes, bitstreams and bitmaps use the program view; sound and DMA retain their data view.
	m_soc->set_video_read_callback(*this, FUNC(superxavix_state::read_program_byte));
	m_super_soc->set_bitmap_read_callback(*this, FUNC(superxavix_state::read_program_byte));
}

u8 superxavix_state::mapped_code_r(offs_t offset)
{
	// High banks fetch externally throughout the window; low banks retain internal RAM/registers.
	if ((offset & 0x800000) || (offset & 0x8000))
		return m_soc->cpu().space(xavix_device::AS_EXTERNAL_BUS).read_byte(offset & 0x7fffff);
	else
		return m_code_lowbus.read_byte(offset & 0x7fff);
}

u8 superxavix_state::read_program_byte(u32 address)
{
	return m_soc->cpu().space(AS_PROGRAM).read_byte(address);
}

u8 superxavix_state::read_graphics_byte(u32 address)
{
	// anpanmdx uses this address form for its title-screen bitmap scroller.
	if (m_extra && BIT(address, 25))
	{
		address &= 0x1ffffff;
		address ^= 0x0600000;
	}

	// The next address bit selects the additional ROM for video fetches.
	if ((address & 0x1000000) && m_extra)
		return m_extra[address & 0x7fffff];

	return read_banked_data_bypass(address);
}

u8 superxavix_state::read_bitmap_byte(u32 address)
{
	return m_extra ? read_graphics_byte(address) : read_banked_data_bypass(address);
}

u8 superxavix_state::read_plotter_byte(u32 address)
{
	if (m_extra && BIT(address, 24))
		return m_extra[address & 0x7fffff];

	return m_soc->cpu().space(xavix_device::AS_EXTERNAL_BUS).read_byte(address & 0x7fffff);
}

void superxavix_state::write_plotter_byte(u32 address, u8 data)
{
	m_soc->cpu().space(xavix_device::AS_EXTERNAL_BUS).write_byte(address & 0x7fffff, data);
}

//-------------------------------------------------
//  PIO Connections
//-------------------------------------------------

template <unsigned Port>
u8 superxavix_state::pio_input_r(offs_t offset, u8 mem_mask)
{
	static_assert(Port < 3);
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PIO, "%s: pio%u_input_r (mask %02x)\n", machine().describe_context(), Port, mem_mask);
	return m_pio_inputs[Port]->read();
}

u8 superxavix_state::connected_inputs(ioport_port &port)
{
	u8 mask = 0;
	for (ioport_field const &field : port.fields())
		if (field.type() != IPT_UNUSED && field.type() != IPT_UNKNOWN)
			mask |= field.mask();
	return mask;
}

template <unsigned Port>
u8 superxavix_state::pio_input_mask_r()
{
	return connected_inputs(*m_pio_inputs[Port]) | m_pio_connected_inputs[Port];
}

u8 superxavix_state::legacy_input_mask_r(offs_t port)
{
	return connected_inputs(port ? *m_in1 : *m_in0) | m_legacy_connected_inputs[port];
}

template <unsigned Port>
void superxavix_state::pio_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	static_assert(Port < 3);
	LOGMASKED(LOG_PIO, "%s: pio%u_output_w %02x (mask %02x)\n", machine().describe_context(), Port, data, mem_mask);
}

//-------------------------------------------------
//  EEPROM Connections
//-------------------------------------------------

void superxavix_i2c_state::superxavix_i2c_24c02(machine_config &config)
{
	superxavix(config);

	I2C_24C02(config, "i2cmem");
}

void superxavix_i2c_state::superxavix_i2c_24c02_4mb(machine_config &config)
{
	superxavix_i2c_24c02(config);
	m_soc->set_external_map(*this, &superxavix_i2c_state::xavix_4mb_extbus_map);
}

void superxavix_i2c_state::superxavix_i2c_24c04(machine_config &config)
{
	superxavix(config);

	I2C_24C04(config, "i2cmem");
}

void superxavix_i2c_state::superxavix_i2c_24c04_4mb(machine_config &config)
{
	superxavix_i2c_24c04(config);
	m_soc->set_external_map(*this, &superxavix_i2c_state::xavix_4mb_extbus_map);
}

void superxavix_i2c_state::superxavix_i2c_24c08(machine_config &config)
{
	superxavix(config);

	I2C_24C08(config, "i2cmem");
}

void superxavix_i2c_state::superxavix_i2c_24c16(machine_config &config)
{
	superxavix(config);

	I2C_24C16(config, "i2cmem");
}

void superxavix_i2c_state::io1_output_w(u8 data, u8 direction)
{
	m_i2cmem->write_sda(BIT(data | ~direction, 3));
	m_i2cmem->write_scl(BIT(data | ~direction, 4));
}

void superxavix_i2c_pio1_state::superxavix_i2c_24c64(machine_config &config)
{
	superxavix(config);

	I2C_24C64(config, "i2cmem");
	configure_pio();
}

void superxavix_i2c_pio1_state::configure_pio()
{
	m_pio_connected_inputs[1] |= 0x08;
	m_super_soc->pio().set_legacy_output_mask<1>(0);
	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_input_r<0>));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_output_w<0>));
	m_super_soc->pio().input_callback<1>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_input_r<1>));
	m_super_soc->pio().output_callback<1>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_output_w<1>));
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_input_r<2>));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_output_w<2>));
}

template <unsigned Port>
u8 superxavix_i2c_pio1_state::pio_input_r(offs_t offset, u8 mem_mask)
{
	static_assert(Port < 3);
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_PIO, "%s: pio%u_input_r\n", machine().describe_context(), Port);

	u8 ret = 0x00;

	if constexpr (Port == 1)
	{
		// reads this by reading the byte, then shifting right 4 times to place value into carry flag
		if (!(mem_mask & 0x08))
			ret |= m_i2cmem->read_sda() << 3;
	}

	return ret;
}

template <unsigned Port>
void superxavix_i2c_pio1_state::pio_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	static_assert(Port < 3);
	LOGMASKED(LOG_PIO, "%s: pio%u_output_w %02x\n", machine().describe_context(), Port, data);

	if constexpr (Port == 1)
	{
		m_i2cmem->write_sda(BIT(mem_mask, 3) ? BIT(data, 3) : 1);
		m_i2cmem->write_scl(BIT(mem_mask, 4) ? BIT(data, 4) : 0);
	}
}

//-------------------------------------------------
//  RTC Connections
//-------------------------------------------------

template <unsigned Port>
void superxavix_i2c_pio1_state::configure_rtc_pio(machine_config &config)
{
	static_assert(Port < 2);
	m_pio_connected_inputs[Port] |= Port ? 0x80 : 0x08;
	m_super_soc->pio().set_legacy_output_mask<Port>(0);
	S35390A(config, m_rtc, 32.768_kHz_XTAL);
	m_super_soc->pio().input_callback<Port>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_rtc_input_r<Port>));
	m_super_soc->pio().output_callback<Port>().set(*this, FUNC(superxavix_i2c_pio1_state::pio_rtc_output_w<Port>));
}

template <unsigned Port>
u8 superxavix_i2c_pio1_state::pio_rtc_input_r(offs_t offset, u8 mem_mask)
{
	constexpr unsigned SDA_BIT = Port ? 7 : 3;
	u8 ret = pio_input_r<Port>(offset, mem_mask);

	if (!BIT(mem_mask, SDA_BIT))
		ret |= m_rtc->sda_r() << SDA_BIT;

	return ret;
}

template <unsigned Port>
void superxavix_i2c_pio1_state::pio_rtc_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	constexpr unsigned SDA_BIT = Port ? 7 : 3;
	constexpr unsigned SCL_BIT = Port ? 6 : 4;
	if constexpr (Port == 1)
		pio_output_w<Port>(offset, data, mem_mask);

	// The bit-banged bus can lower SCL and release SDA in the same PIO write.
	m_rtc->scl_w(BIT(data | ~mem_mask, SCL_BIT));
	m_rtc->sda_w(BIT(data | ~mem_mask, SDA_BIT));
}

//-------------------------------------------------
//  Camera Connections
//-------------------------------------------------

xavix_analog_image_sensor_device &superxavix_i2c_camera_state::configure_dual_reflector_camera(machine_config &config)
{
	auto &camera = XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	camera.set_target_count(2);
	// Representative LOW time allows ADC priming; HIGH is shorter than the ROM's four-poll frame-gap test.
	camera.set_pixel_clocks(64, 32);
	camera.set_frame_gap_pixel_high(true);
	m_soc->adc().input_callback<0>().set(camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
	return camera;
}

u8 superxavix_i2c_camera_state::io1_input_r(u8 direction)
{
	return (m_in1->read() & ~0x06) | m_camera->status_r();
}

void superxavix_i2c_camera_state::io0_output_w(u8 data, u8 direction)
{
	m_camera->setup_w(data, direction);
}

void superxavix_i2c_camera_state::io1_output_w(u8 data, u8 direction)
{
	superxavix_i2c_state::io1_output_w(data, direction);
	m_camera->control_w(data, direction);
}

//**************************************************************************
//  Shared Controls
//**************************************************************************

static INPUT_PORTS_START( superxavix )
	PORT_INCLUDE(xavix)

	PORT_START("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("PIO2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( superxavix_i2c )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))
INPUT_PORTS_END

static INPUT_PORTS_START( superxavix_battery_status )
	PORT_MODIFY("AN7")
	// Representative AN5-AN7 differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1a, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )
INPUT_PORTS_END

static INPUT_PORTS_START( superxavix_demo_mode )
	PORT_MODIFY("PIO2")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x00, "Demo Mode" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xavixport_panel )
	PORT_START("PANEL")
	// Independent panel buttons permit opposing directions, including the Up+Down test-mode chord.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Console Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_16WAY PORT_NAME("Console Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_16WAY PORT_NAME("Console Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_16WAY PORT_NAME("Console Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Console Cancel") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Console Enter") PORT_CODE(KEYCODE_ENTER)

	PORT_INCLUDE(xavix_hard_power_control)

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(input_seq::end_code)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(superxavix_state::xavixport_reset_button), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( xavixport_ir )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

//**************************************************************************
//  Standalone Input Layouts
//**************************************************************************

// Let's! TV Play Futari wa PreCure Max Heart: Mat de Dance Max Heart ni Odotchaou
static INPUT_PORTS_START( maxheart )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	// The 16-way inputs allow independent Left/Right floor switches to be pressed together.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Confirm") PORT_CODE(KEYCODE_W) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Confirm") PORT_CODE(KEYCODE_E) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_16WAY PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_16WAY PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_16WAY PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_16WAY PORT_CODE(KEYCODE_A)

	PORT_MODIFY("PIO0")
	// The game reads the same mat switches through PIO0 for inactivity tracking.
	// The legacy and PIO input registers read the same physical port.
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(superxavix_state::primary_controls_r))
INPUT_PORTS_END


// Let's! TV Play Disney Characters: Oto! Iro! Ton-Ton! Miracle Parade
static INPUT_PORTS_START( ban_dn1j )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Green")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Blue")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Yellow")
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(superxavix_demo_mode)
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


// Doko Demo Doraemon Nihon Ryokou Game DX Taikan! Doko Dora Grand Prix!
static INPUT_PORTS_START( epo_doka )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN0")
	// The four numbered pads double as Player 1-4 buttons in multiplayer minigames.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pad 2 / Roulette")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pad 1 / Secret Gadget")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Back")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pad 3 / Check Map")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Pad 4 / Check Hint")

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_MODIFY("AN5")
	// Representative differences select the ROM's warning rates; physical battery-voltage scaling is unknown.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1b, "Low" )
	PORT_CONFSETTING(    0x28, "Very Low" )
INPUT_PORTS_END

//**************************************************************************
//  System Implementations
//**************************************************************************

//-------------------------------------------------
//  Systems Sharing XaviXPORT Controls
//-------------------------------------------------

xavixport_io_device &superxavix_state::configure_xavixport_io(machine_config &config)
{
	m_pio_connected_inputs[2] |= 0xa4;
	xavixport_io_device &xavixport_io(XAVIXPORT_IO(config, "xavixport_io", 0));
	xavixport_io.panel_callback().set_ioport("PANEL");
	m_super_soc->pio().input_callback<2>().set(xavixport_io, FUNC(xavixport_io_device::pio_r)).mask(0xa0);
	m_super_soc->pio().output_callback<2>().set(xavixport_io, FUNC(xavixport_io_device::pio_w));

	return xavixport_io;
}

INPUT_CHANGED_MEMBER(superxavix_state::xavixport_reset_button)
{
	if (newval)
	{
		m_soc->cpu_reset_w(ASSERT_LINE);
	}
	else
	{
		// The PORT reset line restarts the cartridge SoC while its external controls and non-volatile storage remain powered.
		m_soc->reset();
		m_soc->cpu_reset_w(CLEAR_LINE);
	}
}

// IO0 serial panel for early Tennis and Baseball
void superxavix_io0_panel_state::machine_start()
{
	superxavix_i2c_state::machine_start();
	m_legacy_connected_inputs[0] |= 0x01;

	save_item(NAME(m_panel_shift));
	save_item(NAME(m_panel_selected));
	save_item(NAME(m_panel_clock));
}

void superxavix_io0_panel_state::machine_reset()
{
	superxavix_i2c_state::machine_reset();

	m_panel_shift = 0x3f;
	m_panel_selected = false;
	m_panel_clock = false;
}

u8 superxavix_io0_panel_state::io0_input_r(u8 direction)
{
	u8 result = superxavix_i2c_state::io0_input_r(direction) | 0x01;
	if (m_panel_selected)
		result = (result & ~0x01) | BIT(m_panel_shift, 5);

	return result;
}

void superxavix_io0_panel_state::io0_output_w(u8 data, u8 direction)
{
	u8 const output = data & direction;
	bool const selected = BIT(output, 1);
	bool const clock = BIT(output, 2);

	if (selected && !m_panel_selected)
	{
		u8 const panel = m_panel->read();
		u8 const pressed = panel >> 2;
		m_panel_shift = ~pressed & 0x3f;
	}
	else if (selected && m_panel_selected && m_panel_clock && !clock)
	{
		// The physical shift edge is undocumented; the firmware completes each pulse before its next sample.
		m_panel_shift = ((m_panel_shift << 1) | 1) & 0x3f;
	}

	m_panel_selected = selected;
	m_panel_clock = clock;
}


// XaviX Tennis
static INPUT_PORTS_START( xavixport_tennis )
	PORT_INCLUDE(xavixport_ir)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_ir_motion_device::data_r))
INPUT_PORTS_END

void superxavix_xavtenni_state::superxavix_xavtenni(machine_config &config)
{
	superxavix_i2c_24c08(config);

	configure_ir(config, m_controllers, xavix_ir_motion_device::controller_type::XAVIX_RACKETS);
}


// XaviX Baseball
static INPUT_PORTS_START( xavixport_baseball )
	PORT_INCLUDE(xavixport_ir)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("controllers", FUNC(xavix_baseball_controllers_device::data_r))
INPUT_PORTS_END

void superxavix_xavbaseb_state::superxavix_xavbaseb(machine_config &config)
{
	superxavix_i2c_24c08(config);

	XAVIX_BASEBALL_CONTROLLERS(config, m_controllers, peripheral_clock());
	m_controllers->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_controllers, FUNC(xavix_baseball_controllers_device::vblank_w));
}


// XaviX Golf
static INPUT_PORTS_START( xavixport_golf )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("swing_sensor", FUNC(xavix_golf_swing_device::data_r))
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

void superxavix_xavgolf_state::superxavix_xavgolf(machine_config &config)
{
	superxavix_i2c_24c08(config);

	configure_xavixport_io(config);

	XAVIX_GOLF_SWING(config, m_swing_sensor, peripheral_clock());
	m_swing_sensor->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_swing_sensor, FUNC(xavix_golf_swing_device::vblank_w));
}


// XaviX Bass Fishing
static INPUT_PORTS_START( xavixport_bassfishing )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) // Serial data comes from the cartridge's radio MCU.

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // Radio MCU handshake.

	PORT_INCLUDE(xavixport_panel)

	PORT_MODIFY("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(input_seq::end_code)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(superxavix_xavbassf_state::fishing_reset_button), 0)
INPUT_PORTS_END

void superxavix_xavbassf_state::superxavix_xavbassf(machine_config &config)
{
	superxavix_i2c_24c08(config);

	configure_xavixport_io(config);

	// TODO: Identify the fitted EEPROM; the dump and SPI protocol are 25320-compatible.
	EEPROM_25320(config, m_fishing_cart_eeprom);
	NRF24E1(config, m_fishing_cart_mcu, 16_MHz_XTAL);
	m_fishing_cart_mcu->set_eeprom_region("^fishing_cart_eeprom");
	m_fishing_cart_mcu->eeprom_read_cb().set([this] (offs_t offset) -> u8 { return m_fishing_cart_eeprom->internal_read(offset); });
	m_fishing_cart_mcu->port_in_cb<0>().set(FUNC(superxavix_xavbassf_state::fishing_cart_mcu_port_r));
	m_fishing_cart_mcu->gpio_out_cb<0>().set(FUNC(superxavix_xavbassf_state::fishing_cart_mcu_port_w));
	m_fishing_cart_mcu->spi_clock_cb().set(m_fishing_cart_eeprom, FUNC(eeprom_spi_base_device::sck_w));
	m_fishing_cart_mcu->spi_mosi_cb().set(m_fishing_cart_eeprom, FUNC(eeprom_spi_base_device::si_w));
	m_fishing_cart_mcu->spi_miso_cb().set(m_fishing_cart_eeprom, FUNC(eeprom_spi_base_device::so_r));

	XAVIX_BASSFISHING(config, m_fishing_tackle, 0);

	m_fishing_cart_mcu->radio().set_peer(m_fishing_tackle->radio());
	m_fishing_tackle->radio().set_peer(m_fishing_cart_mcu->radio());

	// Allow the console and cartridge MCU to observe each other's GPIO handshakes.
	config.set_maximum_quantum(attotime::from_usec(10));
}

void superxavix_xavbassf_state::machine_start()
{
	superxavix_i2c_state::machine_start();
	save_item(NAME(m_fishing_host_data));
	save_item(NAME(m_fishing_host_direction));
	save_item(NAME(m_fishing_cart_mcu_pins));
}

void superxavix_xavbassf_state::machine_reset()
{
	superxavix_i2c_state::machine_reset();
	m_fishing_host_data = 0;
	m_fishing_host_direction = 0;
}

INPUT_CHANGED_MEMBER(superxavix_xavbassf_state::fishing_reset_button)
{
	superxavix_state::xavixport_reset_button(field, param, oldval, newval);
	// Releasing the console's GPIO does not reset the separately powered radio bridge.
	if (!newval)
		io0_output_w(0, 0);
}

u8 superxavix_xavbassf_state::io0_input_r(u8 direction)
{
	return (superxavix_i2c_state::io0_input_r(direction) & 0x7f) | (BIT(m_fishing_cart_mcu_pins, 1) << 7);
}

u8 superxavix_xavbassf_state::io1_input_r(u8 direction)
{
	return (superxavix_i2c_state::io1_input_r(direction) & ~0x04) | (m_fishing_cart_mcu_pins & 0x04);
}

void superxavix_xavbassf_state::io0_output_w(u8 data, u8 direction)
{
	superxavix_i2c_state::io0_output_w(data, direction);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superxavix_xavbassf_state::fishing_host_sync), this), (u16(direction) << 8) | data);
}

u8 superxavix_xavbassf_state::fishing_cart_mcu_port_r()
{
	u8 const pins = m_fishing_host_data | ~m_fishing_host_direction;
	// P0.1 is bidirectional data; P0.3 receives the console's clock.
	return 0xf5 | (BIT(pins, 7) << 1) | (BIT(pins, 5) << 3);
}

void superxavix_xavbassf_state::fishing_cart_mcu_port_w(offs_t offset, u8 data, u8 mem_mask)
{
	u8 const pins = data | ~mem_mask;
	m_fishing_cart_eeprom->cs_w(BIT(pins, 0));
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superxavix_xavbassf_state::fishing_cart_mcu_sync), this), pins);
}

TIMER_CALLBACK_MEMBER(superxavix_xavbassf_state::fishing_host_sync)
{
	m_fishing_host_data = param;
	m_fishing_host_direction = param >> 8;
}

TIMER_CALLBACK_MEMBER(superxavix_xavbassf_state::fishing_cart_mcu_sync)
{
	if (BIT(m_fishing_cart_mcu_pins, 2) && !BIT(param, 2))
		m_soc->io().event_w<2>(1);
	m_fishing_cart_mcu_pins = param;
}


// XaviX Bowling
static INPUT_PORTS_START( xavix_bowl )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

void superxavix_i2c_camera_state::superxavix_xavbowl(machine_config &config)
{
	// The 24C08 is documented for xavbowlj and assumed to match xavbowl.
	superxavix_i2c_24c08(config);
	configure_xavixport_io(config);
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	// The ROM derives the throw from successive images of the passive ball reflector.
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


// XaviX Boxing / Jackie Chan PowerBoxing
static INPUT_PORTS_START( xavix_box )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

void superxavix_i2c_pio1_state::superxavix_xavbox(machine_config &config)
{
	superxavix_i2c_24c08(config);

	configure_pio();
	configure_xavixport_io(config);
}

void superxavix_xavbox_state::superxavix_xavbox(machine_config &config)
{
	superxavix_i2c_pio1_state::superxavix_xavbox(config);
	// The sensor uses legacy GPIO while the EEPROM remains on PIO1 bits 3/4.
	m_super_soc->pio().set_legacy_output_mask<1>(0x21);
	XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock()).set_target_count(2);
	m_camera->set_frame_gap_pixel_high(true);
	// Representative pixel pulses must be shorter than the ROM's five-poll gap detector.
	m_camera->set_half_pixel_clocks(40);
	m_soc->adc().input_callback<0>().set(m_camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}

u8 superxavix_xavbox_state::io1_input_r(u8 direction)
{
	return (m_in1->read() & ~0x06) | m_camera->status_r();
}

void superxavix_xavbox_state::io0_output_w(u8 data, u8 direction)
{
	m_camera->setup_w(data, direction);
}

void superxavix_xavbox_state::io1_output_w(u8 data, u8 direction)
{
	m_camera->control_w(data, direction);
}


// Jackie Chan J-Mat Fitness / XaviX Aerostep J-Mat
static INPUT_PORTS_START( xavix_jmat )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("mat", FUNC(xavix_ir_step_mat_device::data_r))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

void superxavix_xavjmat_state::configure_mat(machine_config &config, xavix_ir_step_mat_device::protocol type)
{
	superxavix_i2c_24c64(config);
	configure_rtc_pio<0>(config);
	configure_xavixport_io(config);

	XAVIX_IR_STEP_MAT(config, m_mat, peripheral_clock()).set_protocol(type);
	m_mat->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_mat, FUNC(xavix_ir_step_mat_device::vblank_w));
}

void superxavix_xavjmat_state::superxavix_xavjmat(machine_config &config)
{
	configure_mat(config, xavix_ir_step_mat_device::protocol::JMAT);
}

void superxavix_xavjmat_state::superxavix_xavaero(machine_config &config)
{
	configure_mat(config, xavix_ir_step_mat_device::protocol::AEROSTEP);
}


// XaviX Music & Circuit / Aerostep MC / PowerKIDS
// Shared step-sensor inputs; Check & Balance also uses the XaviX step layout.
static INPUT_PORTS_START( xavix_step )
	PORT_INCLUDE(superxavix)
	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

microphone_device &superxavix_i2c_pio1_state::configure_stereo_music_input(machine_config &config)
{
	microphone_device &music_input(MICROPHONE(config, "music_input", 2));
	music_input.map_on_request_only(0).map_on_request_only(1);
	return music_input;
}

xavixport_io_device &superxavix_i2c_pio1_state::configure_xavix_step_mat(machine_config &config)
{
	xavixport_io_device &xavixport_io(configure_xavixport_io(config));
	xavix_step_mat_device &mat(XAVIX_STEP_MAT(config, "mat"));
	xavixport_io.accessory_callback().set(mat, FUNC(xavix_step_mat_device::report_r));
	return xavixport_io;
}

void superxavix_i2c_pio1_state::configure_xavix_step_sensor(machine_config &config)
{
	superxavix_i2c_24c64(config);
	configure_rtc_pio<1>(config);
	configure_xavix_step_mat(config);
}

microphone_device &superxavix_i2c_pio1_state::configure_xavix_music_input(machine_config &config)
{
	configure_xavix_step_sensor(config);
	microphone_device &music_input(configure_stereo_music_input(config));

	// The mono ADC0 tap ahead of music attenuation is inferred; the board routing needs confirmation.
	m_soc->adc().set_audio_channel(0);
	music_input.add_route(0, m_soc->adc(), 0.5);
	music_input.add_route(1, m_soc->adc(), 0.5);
	return music_input;
}

void superxavix_xavmusic_state::superxavix_xavmusic(machine_config &config)
{
	microphone_device &music_input(configure_xavix_music_input(config));
	M61545(config, m_music_volume, 0);
	music_input.add_route(0, m_music_volume, 1.0, 0);
	music_input.add_route(1, m_music_volume, 1.0, 1);
	m_music_volume->add_route(0, "speaker", 1.0, 0);
	m_music_volume->add_route(1, "speaker", 1.0, 1);
	m_super_soc->pio().set_legacy_output_mask<0>(0);
	m_super_soc->pio().output_callback<0>().append(*this, FUNC(superxavix_xavmusic_state::pio0_volume_w));
}

void superxavix_xavmusicj_state::superxavix_xavmusicj(machine_config &config)
{
	microphone_device &music_input(configure_xavix_music_input(config));
	// The 11-bit serial words match M62429; confirm the fitted part on this board revision.
	M62429(config, m_music_volume, 0);
	music_input.add_route(0, m_music_volume, 1.0, 0);
	music_input.add_route(1, m_music_volume, 1.0, 1);
	m_music_volume->add_route(0, "speaker", 1.0, 0);
	m_music_volume->add_route(1, "speaker", 1.0, 1);
	m_super_soc->pio().set_legacy_output_mask<0>(0);
	m_super_soc->pio().output_callback<0>().append(*this, FUNC(superxavix_xavmusicj_state::pio0_volume_w));
}

void superxavix_xavmusic_state::pio0_volume_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_music_volume->data_w(BIT(data | ~mem_mask, 5));
	m_music_volume->clock_w(BIT(data | ~mem_mask, 6));
}

void superxavix_xavmusicj_state::pio0_volume_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_music_volume->data_w(BIT(data | ~mem_mask, 5));
	m_music_volume->clock_w(BIT(data | ~mem_mask, 6));
}

void superxavix_i2c_pio1_state::superxavix_xavpowrk(machine_config &config)
{
	configure_xavix_step_sensor(config);
}


// XaviX Aero Check & Balance
static INPUT_PORTS_START( xavix_check )
	PORT_INCLUDE(xavix_step)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void superxavix_i2c_pio1_state::superxavix_xavcheck(machine_config &config)
{
	superxavix_i2c_24c64(config);
	configure_rtc_pio<0>(config);
	xavixport_io_device &xavixport_io(configure_xavix_step_mat(config));
	// PIO2.7's electrical source is unknown; Check & Balance treats it as receiver present.
	xavixport_io.set_receiver_present(true);
}

static INPUT_PORTS_START( xavixp )
	PORT_INCLUDE(superxavix)
INPUT_PORTS_END


// Domyos Fitness Exercises / Fitness Challenge / Fitness Dance / Step Concept
static INPUT_PORTS_START( domyos_step )
	PORT_INCLUDE(xavixp)
	PORT_INCLUDE(xavixport_panel)
INPUT_PORTS_END

void superxavix_i2c_pio1_state::superxavix_domfitex(machine_config &config)
{
	superxavix_i2c_24c64(config);
	configure_rtc_pio<1>(config);
	m_soc->set_video_standard(video_standard::PAL);
	m_soc->set_clock(PAL_MASTER_CLOCK);
	superxavix_video_device::configure_screen(*m_screen, m_soc->clock(), video_standard::PAL);

	xavixport_io_device &xavixport_io(configure_xavixport_io(config));
	domyos_step_port_device &step_port(DOMYOS_STEP_PORT(config, "step"));
	xavixport_io.accessory_callback().set(step_port, FUNC(domyos_step_port_device::report_r));
	m_super_soc->pio().input_callback<2>().append(step_port, FUNC(domyos_step_port_device::pio_r)).mask(0x04);
	m_super_soc->pio().output_callback<2>().append(step_port, FUNC(domyos_step_port_device::pio_w));
	step_port.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<1>));
}

// Stand-in for domdance's undumped dsPIC33FJ12GP202, based on the I/O protocol
// reverse-engineered from the host ROM; the audio analysis is approximate.
void superxavix_domdance_state::superxavix_domdance(machine_config &config)
{
	superxavix_domfitex(config);
	microphone_device &music_input(configure_stereo_music_input(config));
	music_input.add_route(0, "speaker", 1.0, 0);
	music_input.add_route(1, "speaker", 1.0, 1);
	music_input.add_route(0, *this, 1.0, 0);
	music_input.add_route(1, *this, 1.0, 1);
	m_pio_connected_inputs[0] |= 0x80;
	m_super_soc->pio().set_legacy_output_mask<0>(0);
	m_super_soc->pio().input_callback<0>().set(FUNC(superxavix_domdance_state::analyser_data_r));
	m_super_soc->pio().output_callback<0>().set(FUNC(superxavix_domdance_state::analyser_clock_w));
	m_super_soc->pio().output_callback<2>().append(FUNC(superxavix_domdance_state::analyser_select_w));
}

void superxavix_domdance_state::machine_start()
{
	superxavix_i2c_pio1_state::machine_start();
	// This rate belongs to the approximate analyser, not the undumped dsPIC's ADC.
	m_analyser_stream = stream_alloc(2, 0, 48'000);
	save_item(NAME(m_analyser_shift));
	save_item(NAME(m_analyser_bits));
	save_item(NAME(m_analyser_selected));
	save_item(NAME(m_analyser_clock));
	save_item(NAME(m_analyser_write));
	save_item(NAME(m_analyser_data));
	save_item(NAME(m_analyser_running));
	save_item(NAME(m_energy_sum));
	save_item(NAME(m_energy_average));
	save_item(NAME(m_energy_samples));
	save_item(NAME(m_music_level));
	save_item(NAME(m_beat_hold));
	save_item(NAME(m_beat_delay));
}

void superxavix_domdance_state::machine_reset()
{
	superxavix_i2c_pio1_state::machine_reset();
	m_analyser_shift = 0;
	m_analyser_bits = 0;
	m_analyser_selected = false;
	m_analyser_clock = false;
	m_analyser_write = false;
	m_analyser_data = false;
	m_analyser_running = false;
	m_energy_sum = 0;
	m_energy_average = 0;
	m_energy_samples = 0;
	m_music_level = 0;
	m_beat_hold = 0;
	m_beat_delay = 0;
}

u8 superxavix_domdance_state::analyser_data_r()
{
	return m_analyser_data ? 0x80 : 0;
}

void superxavix_domdance_state::analyser_select_w(offs_t offset, u8 data, u8 mem_mask)
{
	bool const selected = BIT(data & mem_mask, 0);
	if (selected != m_analyser_selected)
	{
		m_analyser_selected = selected;
		m_analyser_bits = 0;
		m_analyser_shift = 0;
		m_analyser_data = false;
	}
}

void superxavix_domdance_state::analyser_clock_w(offs_t offset, u8 data, u8 mem_mask)
{
	bool const clock = BIT(data & mem_mask, 6);
	if (m_analyser_selected && clock && !m_analyser_clock)
	{
		if (!m_analyser_bits)
		{
			// The first clock selects an eight-bit command or a 24-bit report, both MSB first.
			m_analyser_write = BIT(data & mem_mask, 7);
			if (!m_analyser_write)
			{
				m_analyser_stream->update();
				// Only the onset bit and level byte have identified consumers; other fields are unknown.
				m_analyser_shift = (u32(m_beat_hold != 0) << 16) | (u32(m_music_level) << 8);
			}
			m_analyser_bits = 1;
		}
		else if (m_analyser_write && m_analyser_bits <= 8)
		{
			m_analyser_shift = (m_analyser_shift << 1) | BIT(data & mem_mask, 7);
			if (++m_analyser_bits == 9)
			{
				LOGMASKED(LOG_ANALYSER, "Music analyser command %02x\n", m_analyser_shift);
				// Startup sends 0x40; the diagnostic's preceding 0x00 has no established effect.
				if (m_analyser_shift == 0x40)
				{
					m_analyser_stream->update();
					m_analyser_running = true;
				}
			}
		}
		else if (!m_analyser_write && m_analyser_bits <= 24)
		{
			m_analyser_data = BIT(m_analyser_shift, 23);
			m_analyser_shift = (m_analyser_shift << 1) & 0xffffff;
			m_analyser_bits++;
		}
		// Ignore the trailing clock; retain the last report bit until deselected.
	}
	m_analyser_clock = clock;
}

void superxavix_domdance_state::sound_stream_update(sound_stream &stream)
{
	if (!m_analyser_running)
		return;

	// Approximate the undumped firmware with 10 ms energy windows and a 150 ms onset holdoff.
	for (int sample = 0; sample < stream.samples(); sample++)
	{
		float const left = stream.get(0, sample);
		float const right = stream.get(1, sample);
		m_energy_sum += (left * left + right * right) * 0.5f;
		if (++m_energy_samples == 480)
		{
			float const energy = m_energy_sum / m_energy_samples;
			// The level report is eight bits; the gameplay meter clamps it to 55.
			m_music_level = u8(std::min(255.0f, std::sqrt(energy) * 255.0f));
			if (m_beat_hold)
				m_beat_hold--;
			if (m_beat_delay)
				m_beat_delay--;
			if (!m_beat_delay && energy > std::max(0.0004f, m_energy_average * 2.0f))
			{
				m_beat_hold = 4;
				m_beat_delay = 15;
			}
			m_energy_average += (energy - m_energy_average) * 0.1f;
			m_energy_sum = 0;
			m_energy_samples = 0;
		}
	}
}

//-------------------------------------------------
//  Systems Sharing IR Reports
//-------------------------------------------------

// TV de Asobitai Ryuusei no Rockman: Denpa Henkan! On Air!
static INPUT_PORTS_START( tmy_rkmj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("arm_controller", FUNC(xavix_rockman_arm_device::data_r))
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0xbf, IP_ACTIVE_HIGH, IPT_UNUSED )
	// PIO2.6 is polled by an otherwise unidentified factory/service path.
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(xavix_soft_power_control)
INPUT_PORTS_END

void superxavix_tmy_rkmj_state::superxavix_tmy_rkmj(machine_config &config)
{
	superxavix_i2c_24c04(config);

	XAVIX_ROCKMAN_ARM(config, m_arm_controller, peripheral_clock());
	m_arm_controller->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_arm_controller, FUNC(xavix_rockman_arm_device::vblank_w));
}


// Excite Sports Tennis x Fitness
static INPUT_PORTS_START( epo_tfit )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Console Select / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Console Back / Cancel")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Console Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Console Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Console Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Console Right")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rackets", FUNC(xavix_tfit_rackets_device::data_r))
	PORT_BIT( 0xd6, IP_ACTIVE_HIGH, IPT_UNUSED )
	// The physical identity follows from the documented AC option and the ROM's battery/long-runtime selection.
	PORT_CONFNAME( 0x20, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x20, "AC Adapter" )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_reset_control)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_epo_tfit_state::superxavix_epo_tfit(machine_config &config)
{
	superxavix_i2c_24c04_4mb(config);

	XAVIX_TFIT_RACKETS(config, m_rackets, peripheral_clock());
	m_rackets->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_rackets, FUNC(xavix_tfit_rackets_device::vblank_w));
}


// Let's! TV Play Narikiri Taikan Boukenger: Hashire! Ute! Mission Start!!
static INPUT_PORTS_START( ban_bkgj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Rock / Confirm")
	// Mat contacts are independent; PORT_16WAY preserves simultaneous presses of the left and right zones.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Scissors / Next")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Paper / Previous")
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("survi_buster", FUNC(xavix_survi_buster_device::data_r))
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	// Bit 6 selects unidentified factory/setup behaviour rather than a public control.
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The optional AC adapter is documented, but no firmware-visible source selector has been identified.
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_ban_bkgj_state::superxavix_ban_bkgj(machine_config &config)
{
	superxavix_i2c_24c04_4mb(config);

	XAVIX_SURVI_BUSTER(config, m_survi_buster, peripheral_clock());
	m_survi_buster->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_survi_buster, FUNC(xavix_survi_buster_device::vblank_w));
}

void superxavix_ban_bkgj_state::machine_start()
{
	superxavix_i2c_state::machine_start();

	m_power_off_timer = timer_alloc(FUNC(superxavix_ban_bkgj_state::power_off_expired), this);
}

void superxavix_ban_bkgj_state::machine_reset()
{
	superxavix_i2c_state::machine_reset();

	m_power_off_timer->adjust(attotime::never);
}

void superxavix_ban_bkgj_state::io1_output_w(u8 data, u8 direction)
{
	superxavix_i2c_state::io1_output_w(data, direction);

	if (BIT(direction, 7))
	{
		// The receiver clocks IO1.7 briefly, while software shutdown holds it high; the physical cutoff delay is unknown.
		m_power_off_timer->adjust(BIT(data, 7) ? attotime::from_msec(100) : attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(superxavix_ban_bkgj_state::power_off_expired)
{
	machine().schedule_exit();
}


// Let's! TV Play Ongeki Battle! Kamen Rider Hibiki: Kimero! Ikki Kasei no Kata
static INPUT_PORTS_START( ban_ordj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Circle / Confirm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cross / Cancel")
	// The two triangular buttons are proven opposites; their individual up/down orientation is inferred.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_NAME("Triangle Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_NAME("Triangle Down")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("sticks", FUNC(xavix_hibiki_sticks_device::data_r))
	PORT_BIT( 0x16, IP_ACTIVE_HIGH, IPT_UNUSED )
	// This status input can bypass calibration, but its board-level source is unknown.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The optional AC adapter is documented, but no firmware-visible source selector has been identified.
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_ban_ordj_state::superxavix_ban_ordj(machine_config &config)
{
	superxavix_i2c_24c04(config);

	XAVIX_HIBIKI_STICKS(config, m_sticks, peripheral_clock());
	m_sticks->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_sticks, FUNC(xavix_hibiki_sticks_device::vblank_w));
}


// Anpanman Kazoku de! Ikunou Mat DX
static INPUT_PORTS_START( anpanmdx )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	// Spatial mat bindings also keep its contacts separate from the wireless-stick activation buttons.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Blue Rock / Shokupanman") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red Scissors / Melonpanna") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Green Paper / Cream Panda") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Orange Left Star / Akachanman") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Purple Right Star / Dokinchan") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Left Footprint / Baikinman") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Right Footprint / Anpanman") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("sticks", FUNC(xavix_anpanman_sticks_device::data_r))
	PORT_BIT( 0xf6, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_anpanmdx_state::superxavix_anpanmdx(machine_config &config)
{
	superxavix_i2c_24c08(config);

	XAVIX_ANPANMAN_STICKS(config, m_sticks, peripheral_clock());
	m_sticks->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));
	m_screen->screen_vblank().append(m_sticks, FUNC(xavix_anpanman_sticks_device::vblank_w));
}


// Anpanman Pyon-Pyon Ikunou Mat
static INPUT_PORTS_START( apmj2009 )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	// Four cells lack unique screen captions; their identities follow the manual and the ROM's ordered 3-3-2 grouping.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Anpanman / Center") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Shokupanman / Red Square") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Melonpanna / Pink Triangle") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Currypanman / Blue Circle") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Cream Panda / Green Star") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Horrorman / Purple Star") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Baikinman / Left Footprint") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Dokinchan / Right Footprint") PORT_CODE(KEYCODE_3_PAD)

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	// The sole PIO0 read checks a factory signature ($46), not a normal control.
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(superxavix_demo_mode)
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

//-------------------------------------------------
//  Other Systems
//-------------------------------------------------

// Ishikawa Ryou Excite Golf
static INPUT_PORTS_START( epo_rgfj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_MODIFY("PIO2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Circle / Confirm") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cross / Cancel") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	// The documented power sources select 15-minute or 17-hour inactivity limits.
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_i2c_camera_state::superxavix_epo_rgfj(machine_config &config)
{
	superxavix_i2c_24c08(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_i2c_camera_state::io1_power_off_w));
	auto &camera = XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	// Representative pixel timing separates the ROM's five-poll frame test from ordinary pixels.
	camera.set_half_pixel_clocks(40);
	camera.set_frame_gap_pixel_high(true);
	// TODO: Verify the club's optical geometry and the physical event source.
	camera.set_target_shape(xavix_analog_image_sensor_device::target_shape::ROD);
	// Acquisition starts in the frame gap, before the ROM masks video interrupts to read the next images.
	camera.set_event_source(xavix_analog_image_sensor_device_base::event_source::FRAME_END);
	camera.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<1>));
	m_soc->adc().input_callback<0>().set(camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


// Taikan Cast Off Kamen Rider Kabuto / Taikan Taitoku Kekkaishi
static INPUT_PORTS_START( ban_reflective_camera )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	// Both consoles accept an optional AC adapter, but no firmware-visible source selector has been identified.
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END


void superxavix_i2c_camera_state::superxavix_ban_reflective_camera(machine_config &config)
{
	superxavix_i2c_24c02(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_i2c_camera_state::io1_power_off_w));
	auto &camera = XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	camera.set_target_count(2);
	camera.set_controller_type(xavix_analog_image_sensor_device::controller_type::REFLECTORS);
	// TODO: Measure exposure timing and event wiring; exposure-register units are unknown.
	camera.set_exposure_period(attotime::from_usec(REFLECTIVE_EXPOSURE_USEC));
	camera.set_pixel_clocks(41, 15);
	camera.set_event_source(xavix_analog_image_sensor_device_base::event_source::EXPOSURE_START);
	camera.event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<1>));
	m_soc->adc().input_callback<0>().set(camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


void superxavix_i2c_camera_state::superxavix_ban_utmj(machine_config &config)
{
	superxavix_ban_reflective_camera(config);
	// Two glove reflectors and the ankle band contribute independently to the image.
	m_camera->set_target_count(3);
}


// Let's! TV Play Narikiri Fight Ultraman: Ute! Hissatsu Kousen!!
static INPUT_PORTS_START( ban_ult )
	PORT_INCLUDE(superxavix_i2c)
	PORT_INCLUDE(xavix_hard_power_control)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_DIPNAME( 0x80, 0x00, "Demo Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE(superxavix_battery_status)
INPUT_PORTS_END

void superxavix_i2c_camera_state::superxavix_ban_ult(machine_config &config)
{
	superxavix_i2c_24c02(config);
	auto &camera = XAVIX_ANALOG_IMAGE_SENSOR(config, m_camera, peripheral_clock());
	camera.set_target_count(3);
	// Representative pixel timing separates the ROM's five-poll frame test from ordinary pixels.
	camera.set_half_pixel_clocks(40);
	camera.set_frame_gap_pixel_high(true);
	m_soc->adc().input_callback<0>().set(camera, FUNC(xavix_analog_image_sensor_device_base::pixel_r));
}


// Taikan Daikaijuu Battle: Ayatsure! Ultra Daikaijuu!
static INPUT_PORTS_START( ban_um2j )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_DIPNAME( 0x80, 0x00, "Demo Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_i2c_camera_state::superxavix_ban_um2j(machine_config &config)
{
	superxavix_i2c_24c04(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_i2c_camera_state::io1_power_off_w));
	configure_dual_reflector_camera(config).set_controller_type(xavix_analog_image_sensor_device::controller_type::REFLECTORS);
}


// Gekiwaza Shuutoku Gekiranger: Super Gekiranger e no Michi
static INPUT_PORTS_START( ban_gkrj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_DIPNAME( 0x80, 0x00, "Demo Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE(superxavix_battery_status)
INPUT_PORTS_END

void superxavix_i2c_camera_state::superxavix_ban_gkrj(machine_config &config)
{
	superxavix_i2c_24c02_4mb(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_i2c_camera_state::io1_power_off_w));
	configure_dual_reflector_camera(config);
}


// Yes! PreCure 5: Issho ni Henshin! Metamorphose!
static INPUT_PORTS_START( ban_pr2j )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED ) // Frame and pixel status come from the camera.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", FUNC(i2cmem_device::read_sda))

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	// The printed panel keys are independent contacts, including simultaneous Up/Down.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Circle / Confirm") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cross / Cancel") PORT_CODE(KEYCODE_C)
	PORT_DIPNAME( 0x80, 0x00, "Demo Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_ban_pr2j_state::superxavix_ban_pr2j(machine_config &config)
{
	superxavix_i2c_24c04(config);
	configure_dual_reflector_camera(config).set_controller_type(xavix_analog_image_sensor_device::controller_type::ARM_COVERS);
}

void superxavix_ban_pr2j_state::io1_output_w(u8 data, u8 direction)
{
	// EEPROM uses bits 7/6; bits 3/4 belong to the optional accessory link.
	m_i2cmem->write_scl(BIT(data | ~direction, 6));
	m_i2cmem->write_sda(BIT(data | ~direction, 7));
	m_camera->control_w(data, direction);
}


// U-Dance
static INPUT_PORTS_START( udance )
	PORT_INCLUDE(superxavix_i2c)
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_udance_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("bios", 0);
	map(0x400000, 0x7fffff).r(FUNC(superxavix_udance_state::rom_window_r));
}

void superxavix_udance_state::superxavix_udance(machine_config &config)
{
	superxavix_i2c_24c16(config);
	m_pio_connected_inputs[0] |= 0x10;
	m_super_soc->pio().set_legacy_output_mask<0>(0xbf);

	XAVIX_SERIAL_TRACKING_SENSOR(config, m_camera);

	m_soc->set_external_map(*this, &superxavix_udance_state::xavix_extbus_map);
	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_udance_state::pio0_input_r));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_udance_state::pio0_output_w));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_udance_state::rom_bank_w));
}

void superxavix_udance_state::machine_start()
{
	superxavix_i2c_state::machine_start();

	save_item(NAME(m_rom_bank));
}

void superxavix_udance_state::machine_reset()
{
	superxavix_i2c_state::machine_reset();

	m_rom_bank = 0;
}

u8 superxavix_udance_state::rom_window_r(offs_t offset)
{
	offset &= 0x3fffff;

	// PIO2 bits 5 and 6 select the 4 MiB ROM bank in the upper half of the external bus.
	switch (m_rom_bank)
	{
	case 0x40:
		return m_extra[offset];

	case 0x60:
		return m_extra[0x400000 + offset];

	default:
		return m_rom[0x400000 + offset];
	}
}

u8 superxavix_udance_state::read_banked_data_bypass(u32 address)
{
	if ((address >> 16) >= 0x80 || (address & 0xffff) >= 0x8000)
	{
		address &= 0x7fffff;
		return address < 0x400000 ? m_rom[address] : rom_window_r(address - 0x400000);
	}

	return m_soc->read_low_bus_bypass(address);
}

u8 superxavix_udance_state::pio0_input_r(offs_t offset, u8 mem_mask)
{
	u8 result = superxavix_state::pio_input_r<0>(offset, mem_mask) & ~0x10;

	if (!(mem_mask & 0x10))
		result |= m_camera->data_r() << 4;

	return result;
}

void superxavix_udance_state::pio0_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	if (mem_mask & 0x40)
		m_camera->clock_w(BIT(data, 6));
}

void superxavix_udance_state::rom_bank_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_rom_bank = ((mem_mask & 0x60) == 0x60) ? (data & 0x60) : 0;
}


// TV de Asobitai Soreike Thomas: Sodor Tou no Nakamatachi
static INPUT_PORTS_START( tmy_thom )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Talk")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Camera / Viewpoint")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Whistle")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0xfd, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Drive Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(1)

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

u8 superxavix_tmy_thom_state::io0_input_r(u8 direction)
{
	// The wheel exposes one encoder phase, mirrored to IO0.1 by the firmware; direction is not observable.
	return (superxavix_i2c_power_state::io0_input_r(direction) & ~0x01) | BIT(m_wheel->read(), 0);
}


// Let's! TV Play Mahou Taiketsu Magiranger: Magimat de Dance & Battle
static INPUT_PORTS_START( mrangbat )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	// Each player's three floor pads double as menu selection and rock-paper-scissors controls.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Red Circle / Rock / Confirm") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Red Circle / Rock / Confirm") PORT_PLAYER(2)
	// Independent floor contacts can be held together; directional defaults retain menu navigation.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Green Diamond / Scissors / Select Right") PORT_PLAYER(2) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Pink Star / Paper / Select Left") PORT_PLAYER(2) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Blue Square / Paper / Select Right") PORT_PLAYER(1) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Yellow Triangle / Scissors / Select Left") PORT_PLAYER(1) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE(superxavix_demo_mode)
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_i2c_state::superxavix_mrangbat(machine_config &config)
{
	superxavix(config);

	m_soc->set_external_map(*this, &superxavix_i2c_state::xavix_4mb_extbus_map);

	I2C_24C02(config, "i2cmem"); // 24C02?

	m_super_soc->pio().input_callback<0>().set_ioport("PIO0");
	m_super_soc->pio().input_callback<1>().set_ioport("PIO1");
	m_super_soc->pio().input_callback<2>().set_ioport("PIO2");
}


// Dontore da bei: Donbei-kun to Training
static INPUT_PORTS_START( ndpbj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("PIO2")
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PANEL")
	// The base has four independent arrow buttons and separate Cancel/Enter buttons.
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter / Confirm") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

void superxavix_ndpbj_state::superxavix_ndpbj(machine_config &config)
{
	superxavix_i2c_24c08(config);

	// The base panel uses the same five-byte serial report as XaviXPORT.
	XAVIXPORT_IO(config, m_panel_io, 0);
	m_panel_io->panel_callback().set_ioport("PANEL");
	m_pio_connected_inputs[2] |= 0x20;
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_ndpbj_state::pio2_input_r));
	m_super_soc->pio().output_callback<2>().append(m_panel_io, FUNC(xavixport_io_device::pio_w));

	XAVIX_DIGITAL_IMAGE_SENSOR(config, m_camera);
	m_pio_connected_inputs[0] |= 0xff;
	m_super_soc->pio().set_legacy_output_mask<0>(0);
	m_super_soc->pio().input_callback<0>().set(m_camera, FUNC(xavix_digital_image_sensor_device::data_r));
	m_super_soc->pio().output_callback<0>().set(m_camera, FUNC(xavix_digital_image_sensor_device::data_w));
	m_super_soc->pio().output_callback<2>().append(m_camera, FUNC(xavix_digital_image_sensor_device::strobe_w));
}

u8 superxavix_ndpbj_state::pio2_input_r(offs_t offset, u8 mem_mask)
{
	return (superxavix_state::pio_input_r<2>(offset, mem_mask) & ~0x20) | (m_panel_io->pio_r(offset, mem_mask) & 0x20);
}


// Dontore da bei 2: Ouchi de, Minna de Fitness!
static INPUT_PORTS_START( ndpmj )
	PORT_INCLUDE(superxavix_i2c)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xf7, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	// Mat contacts are independent; PORT_16WAY preserves simultaneous presses of the two Select pads.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Far Left / Back") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Centre Left / Select / Previous") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Centre Right / Select / Next") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Far Right / Confirm") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The optional AC adapter is documented, but no firmware-visible source selector has been identified.
	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_ndpmj_state::superxavix_ndpmj(machine_config &config)
{
	superxavix_i2c_24c16(config);
	m_pio_connected_inputs[1] |= 0x80;
	m_super_soc->pio().set_legacy_output_mask<1>(0x3f);

	S35390A(config, m_rtc, 32.768_kHz_XTAL); // U6

	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_ndpmj_state::pio_input_r<0>));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_ndpmj_state::pio_output_w<0>));
	m_super_soc->pio().input_callback<1>().set(*this, FUNC(superxavix_ndpmj_state::pio1_rtc_input_r));
	m_super_soc->pio().output_callback<1>().set(*this, FUNC(superxavix_ndpmj_state::pio1_rtc_output_w));
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_ndpmj_state::pio_input_r<2>));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_ndpmj_state::pio_output_w<2>));
}

u8 superxavix_ndpmj_state::pio1_rtc_input_r(offs_t offset, u8 mem_mask)
{
	u8 ret = 0x00;

	if (!(mem_mask & 0x80))
		ret |= m_rtc->sda_r() << 7;

	return ret;
}

void superxavix_ndpmj_state::pio1_rtc_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	// The bit-banged bus can lower SCL and release SDA in the same PIO write.
	m_rtc->scl_w(BIT(data | ~mem_mask, 6));
	m_rtc->sda_w(BIT(data | ~mem_mask, 7));
}


// Hello Kitty: TV de Asobou! Manabou! Card de Omamagoto AIUEO Zukan
static INPUT_PORTS_START( epo_kabj )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	// The legends reflect each key's Latin/numeric layer as well as its kana layer.
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"K / \u3042") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // K / あ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"V / \u3044") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // V / い
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u3046") // う
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"10 / \u3048") // 10 / え
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"20 / \u304a") // 20 / お
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"J / \u304b") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // J / か
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"U / \u304d") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // U / き
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u304f") // く

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"9 / \u3051") PORT_CODE(KEYCODE_9) PORT_CHAR('9') // 9 / け
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"19 / \u3053") // 19 / こ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"I / \u3055") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // I / さ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T / \u3057") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // T / し
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u3059") // す
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"8 / \u305b") PORT_CODE(KEYCODE_8) PORT_CHAR('8') // 8 / せ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"18 / \u305d") // 18 / そ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"H / \u305f") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // H / た

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"S / \u3061") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // S / ち
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u3064") // つ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"7 / \u3066") PORT_CODE(KEYCODE_7) PORT_CHAR('7') // 7 / て
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"17 / \u3068") // 17 / と
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"G / \u306a") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // G / な
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"R / \u306b") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // R / に
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u306c") // ぬ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"6 / \u306d") PORT_CODE(KEYCODE_6) PORT_CHAR('6') // 6 / ね

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"16 / \u306e") // 16 / の
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"F / \u306f") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // F / は
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Q / \u3072") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // Q / ひ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u3075") // ふ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"5 / \u3078") PORT_CODE(KEYCODE_5) PORT_CHAR('5') // 5 / へ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"15 / \u307b") // 15 / ほ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"E / \u307e") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // E / ま
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P / \u307f") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // P / み

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u3080") // む
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"4 / \u3081") PORT_CODE(KEYCODE_4) PORT_CHAR('4') // 4 / め
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"14 / \u3082") // 14 / も
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"D / \u3084") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // D / や
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"O / \u309b") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // O / ゛
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Z / \u3086") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // Z / ゆ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"3 / \u309c") PORT_CODE(KEYCODE_3) PORT_CHAR('3') // 3 / ゜
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"13 / \u3088") // 13 / よ

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"C / \u3089") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // C / ら
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"N / \u308a") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // N / り
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y / \u308b") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // Y / る
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"2 / \u308c") PORT_CODE(KEYCODE_2) PORT_CHAR('2') // 2 / れ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"12 / \u308d") // 12 / ろ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"B / \u308f") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // B / わ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"M / \u3092") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // M / を
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"X / \u3093") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // X / ん

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"1 / \u30fc") PORT_CODE(KEYCODE_1) PORT_CHAR('1') // 1 / ー
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Forward / Space (\u3059\u3059\u3080)") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') // Forward / Space (すすむ)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"A / \u3083") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // A / ゃ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"L / \u3085") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // L / ゅ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"W / \u3087") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // W / ょ
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"0 / \u3063") PORT_CODE(KEYCODE_0) PORT_CHAR('0') // 0 / っ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Back / Delete (\u3082\u3069\u308b)") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) // Back / Delete (もどる)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Stroke Order (\u304b\u304d\u3058\u3085\u3093)") PORT_CODE(KEYCODE_1_PAD) // Stroke Order (かきじゅん)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Illustration (\u30a4\u30e9\u30b9\u30c8)") PORT_CODE(KEYCODE_2_PAD) // Illustration (イラスト)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Menu (\u30e1\u30cb\u30e5\u30fc)") PORT_CODE(KEYCODE_3_PAD) // Menu (メニュー)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Hint (\u30d2\u30f3\u30c8)") PORT_CODE(KEYCODE_4_PAD) // Hint (ヒント)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Action (\u30a2\u30af\u30b7\u30e7\u30f3)") PORT_CODE(KEYCODE_5_PAD) // Action (アクション)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Confirm (Red)")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel / Delete (Pink)")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Strawberry Right") PORT_2WAY

	PORT_MODIFY("PIO2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Strawberry Left") PORT_2WAY
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_MODIFY("AN7")
	// Representative AN5-AN7 differences select the ROM's slowly updated warning levels.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1a, "Low" )
	PORT_CONFSETTING(    0x26, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_epo_kabj_state::superxavix_epo_kabj(machine_config &config)
{
	superxavix_state::superxavix(config);
	m_legacy_connected_inputs[0] = 0xff;
	m_legacy_connected_inputs[1] = 0xf7;

	XAVIX_EPO_KABJ_CARD_READER(config, m_card_reader);
	m_card_reader->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<3>));
}

u16 superxavix_epo_kabj_state::keyboard_levels_r()
{
	std::array<u16, 12> contacts{};
	for (unsigned key = 0; key != KEY_CONTACTS.size(); ++key)
	{
		if (BIT(m_keyboard[key / 8]->read(), key % 8))
		{
			auto const [first, second] = KEY_CONTACTS[key];
			contacts[first] |= u16(1) << second;
			contacts[second] |= u16(1) << first;
		}
	}

	u16 levels = m_soc->io().data_latch<0>() & m_soc->io().direction<0>();
	u8 const io1_drive = m_soc->io().data_latch<1>() & m_soc->io().direction<1>();
	levels |= u16(io1_drive & 0x07) << 8;
	levels |= u16(BIT(io1_drive, 4)) << 11;
	return propagate_passive_levels<u16>(levels, contacts);
}

u8 superxavix_epo_kabj_state::io0_input_r(u8)
{
	return keyboard_levels_r();
}

u8 superxavix_epo_kabj_state::io1_input_r(u8)
{
	u16 const levels = keyboard_levels_r();
	// Self-contained activities read the three direct panel buttons through original GPIO as well as PIO.
	return u8((levels >> 8) & 0x07) | (BIT(levels, 11) << 4) | (m_panel_controls->read() & 0xe0);
}


// Doraemon Ugoku! Oekaki
static INPUT_PORTS_START( doradraw )
	PORT_INCLUDE(superxavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pen Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Cancel (B)")

	PORT_MODIFY("PIO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Action (A)") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_MODIFY("MOUSE1X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(80) PORT_KEYDELTA(40) PORT_PLAYER(1)

	PORT_MODIFY("MOUSE1Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(80) PORT_KEYDELTA(40) PORT_PLAYER(1)

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_doradraw_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("bios", 0x000000);
	map(0x400000, 0x4fffff).ram().share("bitmap_buffer"); // reads/writes here
	// Drawing RAM is also selected by data banks 0xe0-0xef.
	map(0x600000, 0x6fffff).mirror(0x800000).ram().share("bitmap_buffer2");
	// CPU data banks 0x80-0xbf mirror the lower half of U2.
	map(0x800000, 0xbfffff).rom().region("bios", 0x000000);
	map(DATA_FLASH_BASE, DATA_FLASH_BASE + DATA_FLASH_SIZE - 1).r(FUNC(superxavix_doradraw_state::data_flash_r));
}

void superxavix_doradraw_state::superxavix_doradraw(machine_config &config)
{
	superxavix(config);
	configure_mapped_memory();
	// The optical pen uses writable signed counters, with vertical polarity opposite MAME's mouse Y.
	m_soc->anport().input_callback<2>().set_ioport("MOUSE1X");
	m_soc->anport().input_callback<3>().set_ioport("MOUSE1Y");
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);
	m_soc->set_data_address_mask(0xffffff);
	// U7 is a 2 MiB, 16-bit flash using AMD/Fujitsu commands and the
	// MBM29DL161TE sector layout. TODO: Verify the exact part from hardware.
	FUJITSU_29DL161TE_16BIT(config, m_data_flash);
	m_super_soc->plotter().set_flush_callback(FUNC(superxavix_doradraw_state::flush_plotter_write));
}

void superxavix_doradraw_state::machine_start()
{
	superxavix_state::machine_start();

	save_item(NAME(m_flash_word_address));
	save_item(NAME(m_flash_word_data));
	save_item(NAME(m_flash_byte_mask));
}

void superxavix_doradraw_state::machine_reset()
{
	superxavix_state::machine_reset();

	m_flash_word_address = 0;
	m_flash_word_data = 0;
	m_flash_byte_mask = 0;
}

u8 superxavix_doradraw_state::read_plotter_byte(u32 address)
{
	if ((address - DATA_FLASH_BASE) < DATA_FLASH_SIZE)
		return data_flash_r(address - DATA_FLASH_BASE);

	return superxavix_state::read_plotter_byte(address);
}

u8 superxavix_doradraw_state::read_banked_data_bypass(u32 address)
{
	// U7 and drawing RAM remain on the external bus for full-address fetches.
	if ((address - DATA_FLASH_BASE) < 0x300000)
		return m_soc->cpu().space(xavix_device::AS_EXTERNAL_BUS).read_byte(address);

	return superxavix_state::read_banked_data_bypass(address);
}

u8 superxavix_doradraw_state::read_graphics_byte(u32 address)
{
	// Graphics address bit 24 selects the upper half of U2.
	if (BIT(address, 24))
		return m_rom[(address & 0x3fffff) | 0x400000];

	return superxavix_state::read_graphics_byte(address);
}

u8 superxavix_doradraw_state::data_flash_r(offs_t offset)
{
	u16 const data = m_data_flash->read(offset >> 1);
	return BIT(offset, 0) ? data >> 8 : data;
}

void superxavix_doradraw_state::write_plotter_byte(u32 address, u8 data)
{
	if ((address - DATA_FLASH_BASE) >= DATA_FLASH_SIZE)
	{
		flush_plotter_write();
		superxavix_state::write_plotter_byte(address, data);
		return;
	}

	u32 const offset = address - DATA_FLASH_BASE;
	u32 const word_address = offset >> 1;
	if (!m_flash_byte_mask || (m_flash_word_address != word_address))
	{
		flush_plotter_write();
		m_flash_word_address = word_address;
		m_flash_word_data = m_data_flash->read(word_address);
	}

	if (BIT(offset, 0))
	{
		m_flash_word_data = (m_flash_word_data & 0x00ff) | (u16(data) << 8);
		m_flash_byte_mask |= 0x02;
	}
	else
	{
		m_flash_word_data = (m_flash_word_data & 0xff00) | data;
		m_flash_byte_mask |= 0x01;
	}

	if (m_flash_byte_mask == 0x03)
		flush_plotter_write();
}

void superxavix_doradraw_state::flush_plotter_write()
{
	if (m_flash_byte_mask)
	{
		m_data_flash->write(m_flash_word_address, m_flash_word_data);
		m_flash_byte_mask = 0;
	}
}

//-------------------------------------------------
//  Shared TV-PC Implementation
//-------------------------------------------------

// Shared TV-PC controller connection and keyboard

static INPUT_PORTS_START( tvpc_keyboard )
	// Keypad defaults avoid MAME's F-key actions on this appliance keyboard.
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Super") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Input Mode (\u5165\u529b\u30e2\u30fc\u30c9)") PORT_CODE(KEYCODE_1_PAD) // Input Mode (入力モード)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Character Type (\u3072\u3089\u304c\u306a/\u30ab\u30bf\u30ab\u30ca/\u82f1\u6570)") PORT_CODE(KEYCODE_2_PAD) // Character Type (ひらがな/カタカナ/英数)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Back (\u3082\u3069\u308b)") PORT_CODE(KEYCODE_3_PAD) // Back (もどる)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Symbol (\u304d\u3054\u3046)") PORT_CODE(KEYCODE_4_PAD) // Symbol (きごう)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Emoji (\u3048\u3082\u3058)") PORT_CODE(KEYCODE_5_PAD) // Emoji (えもじ)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Back Space") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mail") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"0 \u308f") PORT_CODE(KEYCODE_0) PORT_CHAR('0') // 0 わ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"1 \u306c") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // 1 ぬ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"2 \u3075") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') // 2 ふ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"3 \u3042") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // 3 あ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"4 \u3046") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 4 う
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"5 \u3048") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // 5 え
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"6 \u304a") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') // 6 お
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"7 \u3084") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') // 7 や

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"8 \u3086") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') // 8 ゆ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"9 \u3088") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') // 9 よ
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8": \u3051") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*') // : け
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"; \u308c") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+') // ; れ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8", \u306d") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // , ね
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"- \u307b") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') // - ほ
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8". \u308b") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // . る
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"/ \u3081") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // / め

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"@ \u309b") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`') // @ ゛
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"A \u3061") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // A ち
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"B \u3053") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // B こ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"C \u305d") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // C そ
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"D \u3057") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // D し
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"E \u3044") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // E い
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"F \u306f") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // F は
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"G \u304d") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // G き

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"H \u304f") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // H く
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"I \u306b") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // I に
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"J \u307e") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // J ま
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"K \u306e") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // K の
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"L \u308a") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // L り
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"M \u3082") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // M も
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"N \u307f") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // N み
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"O \u3089") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // O ら

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"P \u305b") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // P せ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Q \u305f") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // Q た
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"R \u3059") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // R す
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"S \u3068") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // S と
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"T \u304b") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // T か
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"U \u306a") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // U な
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"V \u3072") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // V ひ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"W \u3066") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // W て

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"X \u3055") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // X さ
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Y \u3093") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // Y ん
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"Z \u3064") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // Z つ
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"[ \u309c") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') // [ ゜
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"¥ \u30fc") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(U'¥') PORT_CHAR('|') // ¥ ー
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"] \u3080") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') // ] む
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"^ \u3078") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~') // ^ へ
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\\ \u308d") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('_') // \ ろ

	PORT_START("RIGHT_SHIFT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

void superxavix_tvpc_state::configure_tvpc_controller(machine_config &config)
{
	// PIO mouse-button inputs also serve cartridge reads through the legacy GPIO aliases.
	m_pio_connected_inputs[0] |= 0x90;
	m_super_soc->pio().set_legacy_output_mask<0>(0);
	m_super_soc->pio().set_legacy_output_mask<1>(0);
	XAVIX_TVPC_CONTROLLER(config, m_tvpc);
	m_tvpc->set_input_source(*this);
	m_tvpc->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<2>));
	m_screen->screen_vblank().append(m_tvpc, FUNC(xavix_tvpc_controller_device::vblank_w));
}

u8 superxavix_tvpc_state::tvpc_pio0_input_r(offs_t offset, u8 mem_mask)
{
	u8 result = m_tvpc_pio[0]->read() & ~0x90;

	if (m_tvpc->data_r())
		result |= 0x80;
	if (BIT(mouse_buttons_r(), 0))
		result |= 0x10;

	return result;
}

void superxavix_tvpc_state::tvpc_pio0_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_tvpc->host_w(data, mem_mask);
}


// Super TV-PC
static INPUT_PORTS_START( suprtvpc )
	PORT_INCLUDE(superxavix)
	PORT_INCLUDE(tvpc_keyboard)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CODE(JOYCODE_X_RIGHT_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CODE(JOYCODE_X_LEFT_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CODE(JOYCODE_Y_DOWN_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CODE(JOYCODE_Y_UP_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Attack") PORT_CODE(JOYCODE_BUTTON1_INDEXED(0)) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CODE(JOYCODE_X_RIGHT_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CODE(JOYCODE_X_LEFT_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CODE(JOYCODE_Y_DOWN_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CODE(JOYCODE_Y_UP_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Attack") PORT_CODE(JOYCODE_BUTTON1_INDEXED(1)) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PIO2")
	// PIO2.1 selects the firmware's battery or AC-adapter inactivity timeout.
	PORT_CONFNAME( 0x02, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x02, "AC Adapter" )

	PORT_MODIFY("AN0")
	// Representative differences select the ROM's hysteretic battery classifier.
	PORT_CONFNAME( 0xff, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Normal" )
	PORT_CONFSETTING(    0x1a, "Low" )
	PORT_CONFSETTING(    0x20, "Very Low" )

	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_suprtvpc_state::xavix_extbus_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("bios", 0x000000);
	map(0x000000, 0x3fffff).r(FUNC(superxavix_suprtvpc_state::cartridge_rom_r));
	// PIO2 bit 2 selects either upper 2 MiB ROM block, including drawing templates
	// below 0x500000 and the loading-screen/desktop bitmaps used by suprtvpchk/suprtvpcdo.
	map(0x400000, 0x5fffff).bankr(m_rombank);
	map(0x600000, 0x67ffff).ram().share("bitmap_buffer"); // reads/writes here
}

void superxavix_suprtvpc_state::superxavix_suprtvpc(machine_config &config)
{
	superxavix(config);
	configure_mapped_memory();
	m_pio_connected_inputs[1] |= 0x08;
	m_pio_connected_inputs[2] |= 0x61;
	I2C_24C64(config, m_i2cmem);
	SUPRTVPC_EXP_PORT(config, m_exp);
	TVPC_MOUSE_PORT(config, m_mouse[0]).set_default_option("mouse");
	TVPC_MOUSE_PORT(config, m_mouse[1]).set_mouse_index(1);
	configure_tvpc_controller(config);

	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_suprtvpc_state::tvpc_pio0_input_r));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_suprtvpc_state::pio0_output_w));
	m_super_soc->pio().input_callback<1>().set(*this, FUNC(superxavix_suprtvpc_state::pio1_input_r));
	m_super_soc->pio().output_callback<1>().set(*this, FUNC(superxavix_suprtvpc_state::pio1_output_w));
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_suprtvpc_state::pio2_input_r));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_suprtvpc_state::pio2_output_w));

	// Both one-button mice use writable relative counters; vertical polarity is opposite MAME's mouse Y.
	m_soc->anport().input_callback<0>().set(m_mouse[0], FUNC(tvpc_mouse_port_device::x_r));
	m_soc->anport().input_callback<1>().set(m_mouse[0], FUNC(tvpc_mouse_port_device::y_r));
	m_soc->anport().input_callback<2>().set(m_mouse[1], FUNC(tvpc_mouse_port_device::x_r));
	m_soc->anport().input_callback<3>().set(m_mouse[1], FUNC(tvpc_mouse_port_device::y_r));
	m_soc->anport().set_counter_mode(0);
	m_soc->anport().set_counter_mode(1, true);
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);

	SUPRTVPC_CART_SLOT(config, m_cart, suprtvpc_cart_slot_device::cart_devices, "rom");
	m_cart->ir_rx_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<0>));

	SOFTWARE_LIST(config, "cart_list").set_original("suprtvpc_cart");
}

void superxavix_suprtvpc_state::machine_start()
{
	superxavix_state::machine_start();

	m_rombank->configure_entries(0, 2, &m_rom[0x400000], 0x200000);
	if (memory_region *const cart_region = m_cart->rom_region())
		m_super_soc->decode_cartridge_graphics(*cart_region);
}

void superxavix_suprtvpc_state::machine_reset()
{
	superxavix_state::machine_reset();

	m_rombank->set_entry(0);
}

void superxavix_suprtvpc_state::pio0_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	tvpc_pio0_output_w(offset, data, mem_mask);
	m_exp->data_w(BIT(data & mem_mask, 5));
}

u8 superxavix_suprtvpc_state::cartridge_rom_r(offs_t offset)
{
	u32 const size = m_cart->rom_size();
	return size ? m_cart->rom_base()[offset & (size - 1)] : m_rom[offset];
}

u8 superxavix_suprtvpc_state::read_banked_data_bypass(u32 address)
{
	if ((address & 0x800000) || ((address & 0xffff) >= 0x8000))
	{
		offs_t const offset = address & (m_rom.bytes() - 1);
		return offset < 0x400000 ? cartridge_rom_r(offset) : m_rom[offset];
	}
	return m_soc->read_low_bus_bypass(address);
}

u8 superxavix_suprtvpc_state::mouse_buttons_r()
{
	return (m_mouse[0]->button_r() << 1) | m_mouse[1]->button_r();
}

u8 superxavix_suprtvpc_state::pio1_input_r(offs_t offset, u8 mem_mask)
{
	u8 result = m_tvpc_pio[1]->read() & ~0x08;

	if (BIT(mouse_buttons_r(), 1))
		result |= 0x08;

	return result;
}

void superxavix_suprtvpc_state::pio1_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	LOGMASKED(LOG_PIO, "%s: pio1_output_w %02x\n", machine().describe_context(), data);
	m_exp->clock_w(BIT(data & mem_mask, 1));
}

u8 superxavix_suprtvpc_state::pio2_input_r(offs_t offset, u8 mem_mask)
{
	// Motherboard and cartridge I²C use SDA5/SCL4 and SDA6/SCL7 respectively.
	return (m_tvpc_pio[2]->read() & ~0x61) | (m_cart->sda_r() << 6) | (m_i2cmem->read_sda() << 5) | m_exp->ready_r();
}

void superxavix_suprtvpc_state::pio2_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_i2cmem->write_scl(BIT(data | ~mem_mask, 4));
	m_i2cmem->write_sda(BIT(data | ~mem_mask, 5));
	m_cart->scl_w(BIT(data | ~mem_mask, 7));
	m_cart->sda_w(BIT(data | ~mem_mask, 6));
	m_cart->ir_tx_w(BIT(data | ~mem_mask, 0));

	if (BIT(mem_mask, 2))
		m_rombank->set_entry(BIT(data, 2));
}


// Hello Kitty Piano PC
static INPUT_PORTS_START( epo_ntpj )
	PORT_INCLUDE(superxavix)
	PORT_INCLUDE(tvpc_keyboard)

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Karaoke") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Red Ribbon Right") PORT_CODE(JOYCODE_X_RIGHT_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Red Ribbon Left") PORT_CODE(JOYCODE_X_LEFT_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Red Ribbon Down") PORT_CODE(JOYCODE_Y_DOWN_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Red Ribbon Up") PORT_CODE(JOYCODE_Y_UP_SWITCH_INDEXED(0)) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Red Ribbon Button") PORT_CODE(JOYCODE_BUTTON1_INDEXED(0)) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Blue Ribbon Right") PORT_CODE(JOYCODE_X_RIGHT_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Blue Ribbon Left") PORT_CODE(JOYCODE_X_LEFT_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Blue Ribbon Down") PORT_CODE(JOYCODE_Y_DOWN_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Blue Ribbon Up") PORT_CODE(JOYCODE_Y_UP_SWITCH_INDEXED(1)) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Blue Ribbon Button") PORT_CODE(JOYCODE_BUTTON1_INDEXED(1)) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	// Piano keys omit PC-key defaults to avoid also typing on the built-in keyboard.
	// The controller scans three bytes LSB-first; fields are named by physical pitch.
	PORT_START("PIANO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#3")

	PORT_START("PIANO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A4")

	PORT_START("PIANO2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C5")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F3")

	PORT_MODIFY("PIO2")
	// PIO2.7 selects the Piano PC's battery or AC-adapter inactivity timeout.
	PORT_CONFNAME( 0x80, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "Batteries" )
	PORT_CONFSETTING(    0x80, "AC Adapter" )

	PORT_INCLUDE(superxavix_battery_status)
	PORT_INCLUDE(xavix_hard_power_control)
INPUT_PORTS_END

void superxavix_epo_ntpj_state::configure_board(machine_config &config)
{
	superxavix_tvpc_state::configure_board(config);
	m_soc->io().output_callback<1>().append(*this, FUNC(superxavix_epo_ntpj_state::io1_power_off_w));
}

void superxavix_epo_ntpj_state::superxavix_epo_ntpj(machine_config &config)
{
	superxavix(config);
	configure_mapped_memory();
	m_pio_connected_inputs[1] |= 0x01;
	m_pio_connected_inputs[2] |= 0x20;
	I2C_24C64(config, m_i2cmem);
	configure_tvpc_controller(config);
	XAVIX_PIANO_CONTROLLER(config, m_piano);
	m_piano->set_input_source(*this);
	m_piano->event_callback().set(m_soc->io(), FUNC(xavix_io_device::event_w<1>));

	TVPC_MOUSE_PORT(config, m_mouse).set_default_option("mic_mouse");
	m_soc->adc().set_audio_channel(0);
	m_mouse->add_route(0, m_soc->adc(), 1.0);

	// The Piano PC's only mouse is wired to the second writable counter pair.
	m_soc->anport().input_callback<2>().set(m_mouse, FUNC(tvpc_mouse_port_device::x_r));
	m_soc->anport().input_callback<3>().set(m_mouse, FUNC(tvpc_mouse_port_device::y_r));
	m_soc->anport().set_counter_mode(2);
	m_soc->anport().set_counter_mode(3, true);

	m_super_soc->pio().input_callback<0>().set(*this, FUNC(superxavix_epo_ntpj_state::tvpc_pio0_input_r));
	m_super_soc->pio().input_callback<1>().set(*this, FUNC(superxavix_epo_ntpj_state::pio1_input_r));
	m_super_soc->pio().input_callback<2>().set(*this, FUNC(superxavix_epo_ntpj_state::pio2_input_r));
	m_super_soc->pio().output_callback<0>().set(*this, FUNC(superxavix_epo_ntpj_state::pio0_output_w));
	m_super_soc->pio().output_callback<1>().set(*this, FUNC(superxavix_epo_ntpj_state::pio1_output_w));
	m_super_soc->pio().output_callback<2>().set(*this, FUNC(superxavix_epo_ntpj_state::pio2_output_w));

	config.set_default_layout(layout_epo_ntpj);
}

u8 superxavix_epo_ntpj_state::mouse_buttons_r()
{
	return m_mouse->button_r();
}

u8 superxavix_epo_ntpj_state::pio1_input_r(offs_t offset, u8 mem_mask)
{
	u8 result = m_tvpc_pio[1]->read() & ~0x01;
	if (m_piano->data_r())
		result |= 0x01;

	return result;
}

void superxavix_epo_ntpj_state::pio0_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	tvpc_pio0_output_w(offset, data, mem_mask);
	m_piano->host_w(data, mem_mask);
}

u8 superxavix_epo_ntpj_state::pio2_input_r(offs_t offset, u8 mem_mask)
{
	return (m_tvpc_pio[2]->read() & ~0x20) | (m_i2cmem->read_sda() << 5);
}

void superxavix_epo_ntpj_state::pio2_output_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_piano->pio2_w(data, mem_mask);
	// The IS24C64 shares PIO2 with the guide-light columns, using SCL4 and SDA5.
	m_i2cmem->write_scl(BIT(data | ~mem_mask, 4));
	m_i2cmem->write_sda(BIT(data | ~mem_mask, 5));
}

//**************************************************************************
//  ROM Definitions
//**************************************************************************

// XaviXPORT
ROM_START( xavtenni )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xavixtennis.bin", 0x000000, 0x800000, CRC(23a1d918) SHA1(2241c59e8ea8328013e55952ebf9060ea0a4675b) )
ROM_END

ROM_START( xavtennij )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xav_tennis_j.bin", 0x000000, 0x800000, CRC(a8d1fa9a) SHA1(abfa32b2a17e01a4fe7944151efb1898f1867107) )
ROM_END

ROM_START( xavbaseb )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbaseball.bin", 0x000000, 0x800000, CRC(e9ed692d) SHA1(537e390e972156dc7da66ee127ae4c8052038ee5) )
ROM_END

ROM_START( xavbasebj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xav_baseball_j.u1", 0x000000, 0x800000, CRC(792a2798) SHA1(9f0d4693dc6c3b062893d9badfcadc1f272d2817) )
ROM_END

ROM_START( xavgolf )
	ROM_REGION( 0x800000, "temp", ROMREGION_ERASE00 )
	ROM_LOAD( "pglfa.u5", 0x000000, 0x800000, CRC(781399d6) SHA1(4797a978f00fc10a34a79a9a09a8355eb92283f2) )

	// The ROM image is split between the base and extra Super XaviX address spaces.
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_COPY( "temp", 0x000000, 0x000000, 0x400000 )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_COPY( "temp", 0x400000, 0x000000, 0x400000 )
ROM_END

ROM_START( xavgolfj )
	ROM_REGION( 0x800000, "temp", ROMREGION_ERASE00 )
	ROM_LOAD( "pglfj.u5", 0x000000, 0x800000, CRC(d3f23cfd) SHA1(ce2d204e9abeb82a5793a613429ead009989ecd4) )

	// The ROM image is split between the base and extra Super XaviX address spaces.
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_COPY( "temp", 0x000000, 0x000000, 0x400000 )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_COPY( "temp", 0x400000, 0x000000, 0x400000 )
ROM_END

ROM_START( xavbowl )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbowling.bin", 0x000000, 0x800000, CRC(2873460b) SHA1(ea8e2392f5a12961a23eb66dca8e07dec81ce8c8) )
ROM_END

ROM_START( xavbowlj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "bowlingcard.u1", 0x000000, 0x800000, CRC(1b83b04f) SHA1(3609d71a7d92629487c2a32003a1259e74dabf0a) )
ROM_END

ROM_START( xavbassf )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbassfishing.bin", 0x000000, 0x800000, CRC(09ab2f29) SHA1(616254176315d0947002e9ae5a6371a3ffa2e8eb) )

	// nRF24E1 firmware stored in the cartridge and tackle SPI EEPROMs.
	// Each dump contains an unexplained extra byte after the 4 KiB EEPROM.
	ROM_REGION( 0x1000, "fishing_cart_eeprom", 0 )
	ROM_LOAD( "xpbassfishingnrf24e1cart.bin", 0x0000, 0x1000, CRC(cfbb19ae) SHA1(32464e4e4be33fdbc7768311f93ce437a316c616) )
	ROM_IGNORE( 1 )

	ROM_REGION( 0x1000, "fishing_tackle:eeprom", 0 )
	ROM_LOAD( "xpbassfishingnrf24e1reel.bin", 0x0000, 0x1000, CRC(62f6303e) SHA1(126b2663e252fb80948f53153e4046e63dd8be32) )
	ROM_IGNORE( 1 )
ROM_END

ROM_START( xavbox )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xpboxing.bin", 0x000000, 0x800000, CRC(b61e7717) SHA1(162b9c53ac8c9d7b6972db44f7bc1cb0a7837b70) )
ROM_END

ROM_START( xavboxj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jboxj.u2", 0x000000, 0x800000, CRC(4545c35e) SHA1(ac3245d704b5bb6200808798d9d5711c336d68b3) )
ROM_END

// Several of the XaviXport and DiS games are 2 glob setups (and must have some kind of banking)

ROM_START( xavjmat )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(1420640d) SHA1(dd714cd57cff885293688f74f69b5c1726e20ec0) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(52dc318c) SHA1(dc50e0747ba29cfb1048fd4a55d26870086c869b) )

	ROM_REGION( 0x2000, "i2cmem", ROMREGION_ERASE00 ) // provide valid defaults
	ROM_LOAD( "i2cmem", 0x0000, 0x2000, CRC(03de8161) SHA1(f1c9b8e8016eb9da956b3af4466cc35ae8c60c50) )
ROM_END

ROM_START( xavaero )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "aerostep.u2", 0x0000000, 0x0800000, CRC(7fce9cc1) SHA1(460bcef8a23d792941108e5da8c0d669a546b94c) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "aerostep.u3", 0x0000000, 0x0800000, CRC(ed9ca4ee) SHA1(4d90300880b02ac275e0cb502de16ae6f132aa2b) )

	ROM_REGION( 0x2000, "i2cmem", ROMREGION_ERASE00 ) // provide valid defaults
	ROM_LOAD( "i2cmem", 0x0000, 0x2000, CRC(03de8161) SHA1(f1c9b8e8016eb9da956b3af4466cc35ae8c60c50) )
ROM_END

ROM_START( xavmusic )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(e7c8ad59) SHA1(d47fac8b480de4db88a1b306ff8830a65d1738a3) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(977c956f) SHA1(debc086d0cf6c391002ad163e7bfaa2f010cc8f5) )
ROM_END

ROM_START( xavmusicj )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pascj.u2", 0x0000000, 0x0800000, CRC(d483fbf2) SHA1(b347eac56ee182d3c72a362d2cb16985ca3caa15) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "pascj.u3", 0x0000000, 0x0800000, CRC(464107dc) SHA1(183aa242ec46799bfe48aacfaed8e58ad2d642b2) )
ROM_END

ROM_START( xavcheck )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jmscj_1.u2", 0x0000000, 0x0800000, CRC(65c62d04) SHA1(eb3616576b5523b337b58b72514508bb484f9d41) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "jmscj_2.u3", 0x0000000, 0x0800000, CRC(c6ff7906) SHA1(0a5541eedea485aa86093e1bfcc996f503cc6dd7) )
ROM_END

ROM_START( xavpkjr )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tc58fvm6b5b.u2", 0x0000000, 0x0800000, CRC(82a32ede) SHA1(a555662b05925126ebc94ae6e4e13c1cfc3c86e0) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "tc58fvm6b5b.u3", 0x0000000, 0x0800000, CRC(8b836b5e) SHA1(f922a99423751cadb328c9a5ad1979658fe20496) )

	ROM_REGION( 0x2000, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "s-24cs64a.u1", 0x0000, 0x2000, CRC(3a7637f7) SHA1(df5b5903900ae8488a44e3449dd2757d1dc35bc2) )
ROM_END

ROM_START( xavpowrk )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "tc58fvm6b5b.u2", 0x0000000, 0x0800000, CRC(5fb55dba) SHA1(0c4d370d258a15712dd323739fe23106d00bc04a) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "tc58fvm6b5b.u3", 0x0000000, 0x0800000, CRC(4d8dd940) SHA1(5816ecf14771afb5d41e18042cd72ef5cdcfb95f) )

	ROM_REGION( 0x2000, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "s-24cs64a.u1", 0x0000, 0x2000, CRC(23b77570) SHA1(9e1d5b266a884791bcc56d2700b6cbb162d20d53) )
ROM_END


// Domyos DiS (Super XaviX titles)
ROM_START( domfitex )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000,  CRC(841fe3cd) SHA1(8678b8a0c5198b24169a84dbe3ae979bb0838f23) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(1dc844ea) SHA1(c23da9006227f7fe4982998c17759d403a47472a) )
ROM_END

ROM_START( domfitch )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(0ff2a7a6) SHA1(9b924cc4330e3f8d9204390854048fe2325bfdf7) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(284583f6) SHA1(bd2d5304f1e01eed656b5de957ec0a0330a3d969) )
ROM_END

ROM_START( domdance )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(74f9499d) SHA1(a64235075e32567cd6d2ab7b1284efcb8e7538e2) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
	ROM_LOAD( "u3", 0x0000000, 0x0800000, CRC(e437565c) SHA1(f6db219ea14404b698ca453f6e50c726b2e77abb) )
ROM_END

ROM_START( domstepc )
	ROM_REGION( 0x0800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "u2", 0x0000000, 0x0800000, CRC(cb37b5e9) SHA1(b742e3db98f36720adf5af9096c6bc235279de12) )

	ROM_REGION( 0x0800000, "extra", ROMREGION_ERASE00 )
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

ROM_START( ban_um2j )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "um2j.u1", 0x000000, 0x800000, CRC(88ea8e50) SHA1(b7507985a05335b6c07584bbb9ec2bd7f65ffe19) )
ROM_END

ROM_START( ban_dn1j )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "dn1j.u2", 0x000000, 0x800000, CRC(0a0cef0f) SHA1(c83a2635a969b3c686dbc599a37f8b7496b0c6a1) )
ROM_END

ROM_START( epo_kabj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "kabj.u2", 0x000000, 0x800000, CRC(264655ec) SHA1(11a25c7b7e3ffd3c4bc0476c64d5d68754757f40) )
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

ROM_START( ban_gkrj )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("gkrj.u2", 0x000000, 0x400000, CRC(d9ffe41a) SHA1(18583e1b5d9eb89e0364bd84b14f89bbe9640b19) )
ROM_END

ROM_START( ban_pr2j )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("pr2j.u2", 0x000000, 0x800000, CRC(e46bf811) SHA1(4b88a7a7001e99be526e889d0b81a43be0b1e464)  )
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

	ROM_REGION(0x800000, "extra", ROMREGION_ERASE00)
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

ROM_START( suprtvpcln ) // from the 'Super TV-PC Link' branded unit which was bundled with a link card, updated copyright date compared to suprtvpc set at least
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00) // inverted line?
	ROM_LOAD("stvpc.u3", 0x200000, 0x200000, CRC(55d872ca) SHA1(7d0b4b527e95f65df5d70dcc303c81c7d893c638) )
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

	ROM_REGION(0x200000, "data", ROMREGION_ERASE00) // 16-bit writable data flash
	ROM_LOAD("dmdj.u7", 0x000000, 0x200000, CRC(0e6392f9) SHA1(30fa3d3451b37d663e124c7d1d52c7e30284d2fb) )
ROM_END

ROM_START( hkdraw )
	ROM_REGION(0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("dmdj.u2", 0x000000, 0x800000, CRC(aa9dcbed) SHA1(153bc2bf99f68f40ff342859f323d2071e7979dc) )

	ROM_REGION(0x200000, "data", ROMREGION_ERASE00) // 16-bit writable data flash
	ROM_LOAD("dmdj.u7", 0x000000, 0x200000, CRC(320098cb) SHA1(07c73fa7406e44eafbfdcbe70857e61f18c0472d) )
ROM_END

ROM_START( ndpbj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("ndpbj.u2", 0x000000, 0x800000, CRC(80cb5cbb) SHA1(cd424c4fbea8e9e47d165c4c8be52755fc7c2d98) )

	ROM_REGION( 0x400, "i2cmem", ROMREGION_ERASE00)
	ROM_LOAD("s-24cs08a.u6", 0x000, 0x400, CRC(a22db408) SHA1(f8d925c75054a961930af12869e3002bb9c4600b) )
ROM_END

ROM_START( ndpmj )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("ndpmj.u3", 0x000000, 0x800000, CRC(a8132d93) SHA1(2bcbf497e3fa7c7f44569f337cb8babff80c9338) )

	ROM_REGION( 0x800, "i2cmem", ROMREGION_ERASE00)
	ROM_LOAD("s-24cs16a.u5", 0x000, 0x800, CRC(5d2dd322) SHA1(c0af08a382c4dbefe290fef34cfd8345e904bd74) )
ROM_END


//**************************************************************************
//  System Registrations
//**************************************************************************

CONS( 2004, xavtenni,   0,        0, superxavix_xavtenni,              xavixport_tennis,      superxavix_xavtenni_state,   empty_init, "SSD Company Ltd.", "XaviX Tennis (XaviXPORT)", MACHINE_NOT_WORKING )
// ザビックス テニス
CONS( 2005, xavtennij,  xavtenni, 0, superxavix_xavtenni,              xavixport_tennis,      superxavix_xavtenni_state,   empty_init, "SSD Company Ltd.", "XaviX Tennis (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
CONS( 2004, xavbaseb,   0,        0, superxavix_xavbaseb,              xavixport_baseball,    superxavix_xavbaseb_state,   empty_init, "SSD Company Ltd.", "XaviX Baseball (XaviXPORT)", MACHINE_NOT_WORKING )
// ザビックス ベースボール
CONS( 2005, xavbasebj,  xavbaseb, 0, superxavix_xavbaseb,              xavixport_baseball,    superxavix_xavbaseb_state,   empty_init, "SSD Company Ltd.", "XaviX Baseball (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
CONS( 2005, xavgolf,    0,        0, superxavix_xavgolf,               xavixport_golf,        superxavix_xavgolf_state,    empty_init, "SSD Company Ltd.", "XaviX Golf (XaviXPORT)", 0 )
// ザビックス ゴルフ
CONS( 2005, xavgolfj,   xavgolf,  0, superxavix_xavgolf,               xavixport_golf,        superxavix_xavgolf_state,    empty_init, "SSD Company Ltd.", "XaviX Golf (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
// Both Bowling ROMs make zero-count startup palette uploads that write to unmapped 0x6bff.
CONS( 2004, xavbowl,    0,        0, superxavix_xavbowl,               xavix_bowl,            superxavix_i2c_camera_state, empty_init, "SSD Company Ltd.", "XaviX Bowling (XaviXPORT)", MACHINE_NOT_WORKING ) // has IR 'Camera'
// ザビックス ボウリング
CONS( 2005, xavbowlj,   xavbowl,  0, superxavix_xavbowl,               xavix_bowl,            superxavix_i2c_camera_state, empty_init, "SSD Company Ltd.", "XaviX Bowling (XaviXPORT, PT2-BWL-11, Japan)", MACHINE_NOT_WORKING ) // ^^
CONS( 2004, xavbox,     0,        0, superxavix_xavbox,                xavix_box,             superxavix_xavbox_state,     empty_init, "SSD Company Ltd.", "XaviX Boxing / Jackie Chan PowerBoxing (XaviXPORT)", MACHINE_NOT_WORKING ) // has IR 'Camera'
// ザビックス パワーボクシング
CONS( 2004, xavboxj,    xavbox,   0, superxavix_xavbox,                xavix_box,             superxavix_xavbox_state,     empty_init, "SSD Company Ltd.", "XaviX Boxing / Jackie Chan PowerBoxing (XaviXPORT, Japan)", MACHINE_NOT_WORKING ) // has IR 'Camera'
// Bass Fishing PCB is just like Tennis except with an RF daughterboard.
CONS( 2004, xavbassf,   0,        0, superxavix_xavbassf,              xavixport_bassfishing, superxavix_xavbassf_state,   empty_init, "SSD Company Ltd.", "XaviX Bass Fishing (XaviXPORT)", MACHINE_NOT_WORKING )

// These titles use a 24C64-compatible SEEPROM.
CONS( 2005, xavjmat,    0,        0, superxavix_xavjmat,               xavix_jmat,            superxavix_xavjmat_state,    empty_init, "SSD Company Ltd.", "Jackie Chan J-Mat Fitness (XaviXPORT)", MACHINE_NOT_WORKING )
// ザビックス エアロステップ
CONS( 2005, xavaero,    xavjmat,  0, superxavix_xavaero,               xavix_jmat,            superxavix_xavjmat_state,    empty_init, "SSD Company Ltd.", "XaviX Aerostep J-Mat (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
CONS( 2007, xavmusic,   0,        0, superxavix_xavmusic,              xavix_step,            superxavix_xavmusic_state,   empty_init, "SSD Company Ltd.", "XaviX Music & Circuit (XaviXPORT)", MACHINE_NOT_WORKING )
// ザビックスエアロ エアロステップ ミュージック&サーキット
CONS( 2007, xavmusicj,  xavmusic, 0, superxavix_xavmusicj,             xavix_step,            superxavix_xavmusicj_state,  empty_init, "SSD Company Ltd.", "XaviX Aerostep MC (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
// ザビックスエアロ チェック&バランス
CONS( 2006, xavcheck,   0,        0, superxavix_xavcheck,              xavix_check,           superxavix_i2c_pio1_state,   empty_init, "SSD Company Ltd.", "XaviX Aero Check & Balance (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
// XaviX パワーキッズ ジュニア カラダでのばす才能開発
CONS( 2007, xavpkjr,    0,        0, superxavix_xavpowrk,              xavix_step,            superxavix_i2c_pio1_state,   empty_init, "SSD Company Ltd.", "PowerKIDS Jr. (XaviXPORT, Japan)", MACHINE_NOT_WORKING )
// XaviX パワーキッズ カラダでのばす才能開発
CONS( 2007, xavpowrk,   0,        0, superxavix_xavpowrk,              xavix_step,            superxavix_i2c_pio1_state,   empty_init, "SSD Company Ltd.", "PowerKIDS (XaviXPORT, Japan)", MACHINE_NOT_WORKING )

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

// These titles use a 24C64-compatible SEEPROM and an S-35390A RTC.
// Exercises has some leftover PC buffer stuff.  Exercises and Challenge use identical PCBs.
CONS( 2008, domfitex,   0,        0, superxavix_domfitex,              domyos_step,           superxavix_i2c_pio1_state,   empty_init, "Decathlon / SSD Company Ltd.", "Domyos Fitness Exercises (Domyos Interactive System)", MACHINE_NOT_WORKING )
CONS( 2008, domfitch,   0,        0, superxavix_domfitex,              domyos_step,           superxavix_i2c_pio1_state,   empty_init, "Decathlon / SSD Company Ltd.", "Domyos Fitness Challenge (Domyos Interactive System)", MACHINE_NOT_WORKING )
CONS( 2007, domdance,   0,        0, superxavix_domdance,              domyos_step,           superxavix_domdance_state,   empty_init, "Decathlon / SSD Company Ltd.", "Domyos Fitness Dance (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
CONS( 2007, domstepc,   0,        0, superxavix_domfitex,              domyos_step,           superxavix_i2c_pio1_state,   empty_init, "Decathlon / SSD Company Ltd.", "Domyos Step Concept (Domyos Interactive System)", MACHINE_NOT_WORKING )

// some DIS games run on XaviX 2 instead, see xavix2.cpp for Domyos Fitness Adventure and Domyos Bike Concept

// Let's!TVプレイ 魔法戦隊マジレンジャー マジマットでダンス＆バトル
CONS( 2005, mrangbat,   0,        0, superxavix_mrangbat,              mrangbat,              superxavix_i2c_state,        empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Mahou Taiketsu Magiranger: Magimat de Dance & Battle (Japan)", 0 )

// エキサイトスポーツ　テニス×フィットネス
CONS( 2004, epo_tfit,   0,        0, superxavix_epo_tfit,              epo_tfit,              superxavix_epo_tfit_state,   empty_init, "Epoch / SSD Company Ltd.", "Excite Sports Tennis x Fitness (Japan)", MACHINE_NOT_WORKING )

// 石川遼 エキサイトゴルフ
CONS( 2010, epo_rgfj,   0,        0, superxavix_epo_rgfj,              epo_rgfj,              superxavix_i2c_camera_state, empty_init, "Epoch / SSD Company Ltd.", "Ishikawa Ryou Excite Golf (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ ふたりはプリキュアMax Heart マットでダンス Max Heartにおどっちゃおう
CONS( 2004, maxheart,   0,        0, superxavix_i2c_24c04_4mb,         maxheart,              superxavix_i2c_state,        empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Futari wa PreCure Max Heart: Mat de Dance Max Heart ni Odotchaou (Japan)", MACHINE_NOT_WORKING )

// どこでもドラえもん 日本旅行ゲームDX体感！どこドラグランプリ！
CONS( 2004, epo_doka,   0,        0, superxavix_4mb,                   epo_doka,              superxavix_state,            empty_init, "Epoch / SSD Company Ltd.", "Doko Demo Doraemon Nihon Ryokou Game DX Taikan! Doko Dora Grand Prix! (Japan)", MACHINE_NOT_WORKING )

// Hello Kitty テレビであそぼう!まなぼう! カードでおままごと あいうえお図鑑
CONS( 2006, epo_kabj,   0,        0, superxavix_epo_kabj,              epo_kabj,              superxavix_epo_kabj_state,   empty_init, "Epoch / SSD Company Ltd.", "Hello Kitty: TV de Asobou! Manabou! Card de Omamagoto AIUEO Zukan (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ なりきり体感 ボウケンジャー 走れ！撃て！ミッションスタート！！
CONS( 2006, ban_bkgj,   0,        0, superxavix_ban_bkgj,              ban_bkgj,              superxavix_ban_bkgj_state,   empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Narikiri Taikan Boukenger: Hashire! Ute! Mission Start!! (Japan)", MACHINE_NOT_WORKING )

// Let's!TV プレイ 体感キャストオフ 仮面ライダーカブト クロックアップ＆ライダーキック!!
CONS( 2006, ban_utmj,   0,        0, superxavix_ban_utmj,              ban_reflective_camera, superxavix_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Taikan Cast Off: Kamen Rider Kabuto Clock Up & Rider Kick!! (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ なりきりファイト ウルトラマン 撃て！必殺光線！！
CONS( 2006, ban_ult,    0,        0, superxavix_ban_ult,               ban_ult,               superxavix_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Narikiri Fight Ultraman: Ute! Hissatsu Kousen!! (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ 体感大怪獣バトル あやつれ!ウルトラ大怪獣!
CONS( 2007, ban_um2j,   0,        0, superxavix_ban_um2j,              ban_um2j,              superxavix_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Taikan Daikaijuu Battle: Ayatsure! Ultra Daikaijuu! (Japan)", MACHINE_NOT_WORKING )

// Let’s!TVプレイ ゲキワザ習得 ゲキレンジャー スーパーゲキレンジャーへの道
CONS( 2007, ban_gkrj,   0,        0, superxavix_ban_gkrj,              ban_gkrj,              superxavix_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Gekiwaza Shuutoku Gekiranger: Super Gekiranger e no Michi (Japan)", MACHINE_NOT_WORKING )

// Yes!プリキュア5 いっしょに変身!メタモルフォーゼ!
CONS( 2007, ban_pr2j,   0,        0, superxavix_ban_pr2j,              ban_pr2j,              superxavix_ban_pr2j_state,   empty_init, "Bandai / SSD Company Ltd.", "Yes! PreCure 5: Issho ni Henshin! Metamorphose! (Japan)", MACHINE_NOT_WORKING )

// テレビで遊び隊 それいけトーマス ソドー島のなかまたち
CONS( 2005, tmy_thom,   0,        0, superxavix_i2c_24c04,             tmy_thom,              superxavix_tmy_thom_state,   empty_init, "Tomy / SSD Company Ltd.", "TV de Asobitai Soreike Thomas: Sodor Tou no Nakamatachi / Thomas & Friends on the Island of Sodor (Japan)", MACHINE_NOT_WORKING )

// Ｌｅｔ’ｓ！ ＴＶプレイ 体感体得 結界師 方囲！定礎！結！滅！
CONS( 2007, ban_kksj,   0,        0, superxavix_ban_reflective_camera, ban_reflective_camera, superxavix_i2c_camera_state, empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Taikan Taitoku Kekkaishi: Houi! Jouso! Ketsu! Metsu! (Japan)", MACHINE_NOT_WORKING )

// テレビで遊び隊 流星のロックマン 電波変換！オン・エア！
CONS( 2007, tmy_rkmj,   0,        0, superxavix_tmy_rkmj,              tmy_rkmj,              superxavix_tmy_rkmj_state,   empty_init, "Takara Tomy / Capcom / SSD Company Ltd.", "TV de Asobitai Ryuusei no Rockman: Denpa Henkan! On Air! (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ 音撃バトル！仮面ライダー響鬼 決めろ！一気火勢の型
CONS( 2005, ban_ordj,   0,        0, superxavix_ban_ordj,              ban_ordj,              superxavix_ban_ordj_state,   empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Ongeki Battle! Kamen Rider Hibiki: Kimero! Ikki Kasei no Kata (Japan)", MACHINE_NOT_WORKING )

// Let's!TVプレイ ディズニーキャラクターズ オト！イロ！トン・トン！ミラクルパレード
CONS( 2007, ban_dn1j,   0,        0, superxavix_i2c_24c04,             ban_dn1j,              superxavix_i2c_power_state,  empty_init, "Bandai / SSD Company Ltd.", "Let's! TV Play Disney Characters: Oto! Iro! Ton-Ton! Miracle Parade (Japan)", MACHINE_NOT_WORKING )

// アンパンマン かぞくで!育脳マットDX
CONS( 2011, anpanmdx,   0,        0, superxavix_anpanmdx,              anpanmdx,              superxavix_anpanmdx_state,   empty_init, "JoyPalette / Toho / SSD Company Ltd.", "Anpanman Kazoku de! Ikunou Mat DX (Japan)", MACHINE_NOT_WORKING )

// アンパンマン ぴょんぴょん育脳マット
CONS( 2009, apmj2009,   0,        0, superxavix_i2c_24c16,             apmj2009,              superxavix_i2c_power_state,  empty_init, "Toho / SSD Company Ltd.", "Anpanman Pyon-Pyon Ikunou Mat (Japan)", MACHINE_NOT_WORKING )

// has HT24LC16
CONS( 2008, udance,     0,        0, superxavix_udance,                udance,                superxavix_udance_state,     empty_init, "Hasbro / Tiger Electronics / Creativity / Medialink / SSD Company Ltd.", "U-Dance", 0 )

// these have RAM in the usual ROM space & also have an Atmel 24LC64,
CONS( 2004, suprtvpc,   0,        0, superxavix_suprtvpc,              suprtvpc,              superxavix_suprtvpc_state,   empty_init, "Epoch / SSD Company Ltd.", "Super TV-PC (Japan)", MACHINE_NOT_WORKING )
CONS( 2006, suprtvpchk, suprtvpc, 0, superxavix_suprtvpc,              suprtvpc,              superxavix_suprtvpc_state,   empty_init, "Epoch / SSD Company Ltd.", "Super TV-PC - Hello Kitty (Japan)", MACHINE_NOT_WORKING )
CONS( 2006, suprtvpcdo, suprtvpc, 0, superxavix_suprtvpc,              suprtvpc,              superxavix_suprtvpc_state,   empty_init, "Epoch / SSD Company Ltd.", "Super TV-PC - Doraemon (Japan)", MACHINE_NOT_WORKING )
CONS( 2006, suprtvpcln, suprtvpc, 0, superxavix_suprtvpc,              suprtvpc,              superxavix_suprtvpc_state,   empty_init, "Epoch / SSD Company Ltd.", "Super TV-PC - Link (Japan)", MACHINE_NOT_WORKING )

// Hello Kitty ピアノパソコン
// similar to Super TV-PC but with additional built in piano
CONS( 2008, epo_ntpj,   0,        0, superxavix_epo_ntpj,              epo_ntpj,              superxavix_epo_ntpj_state,   empty_init, "Epoch / SSD Company Ltd.", "Hello Kitty Piano PC (Japan)", MACHINE_NOT_WORKING )

// ドラえもん うごく！おえかき エポック社
CONS( 2007, doradraw,   0,        0, superxavix_doradraw,              doradraw,              superxavix_doradraw_state,   empty_init, "Epoch / SSD Company Ltd.", "Doraemon Ugoku! Oekaki (Japan)", 0 )

// ハローキティ うごく！おえかき エポック社
CONS( 2007, hkdraw,     0,        0, superxavix_doradraw,              doradraw,              superxavix_doradraw_state,   empty_init, "Epoch / SSD Company Ltd.", "Hello Kitty Ugoku! Oekaki (Japan)", 0 )

// どんトレだ兵衛～どん兵衛くんとトレーニング
// Camera exposure response remains unverified.
CONS( 2007, ndpbj,      0,        0, superxavix_ndpbj,                 ndpbj,                 superxavix_ndpbj_state,      empty_init, "Nissin / SSD Company Ltd.", "Dontore da bei: Donbei-kun to Training (Japan)", MACHINE_NOT_WORKING )

// どんトレだ兵衛2～おうちで、みんなでフィットネス!～
CONS( 2008, ndpmj,      0,        0, superxavix_ndpmj,                 ndpmj,                 superxavix_ndpmj_state,      empty_init, "Nissin / SSD Company Ltd.", "Dontore da bei 2: Ouchi de, Minna de Fitness! (Japan)", 0 )
