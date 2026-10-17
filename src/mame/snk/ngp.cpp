// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

SNK NeoGeo Pocket driver

After setting the initial console settings power cycle the unit at least
once (in emulation), otherwise the settings will not be saved properly.


The NeoGeo Pocket (Color) contains one big chip which contains the following
components:
- Toshiba TLCS-900/H cpu core with 64KB ROM
- Z80 cpu core
- K1GE (mono)/K2GE (color) graphics chip + lcd controller
- SN76489A compatible sound chip
- DAC for additional sound
- RTC
- RAM


The following TLCS-900/H input sources are known:
- INT0 - Alarm/RTC irq
- INT4 - VBlank irq
- INT5 - Interrupt from Z80
- AN0 - Analog input which is used to determine the battery level
- NMI - Triggered by power button
- TIO - HBlank signal


The following TLCS-900/H output destination are known:
- TO3 - connected to Z80 INT pin


The cartridges
==============

The cartridges used flash chips produced by Toshiba, Sharp or Samsung. These
are the only 3 manufacturers supported by the NeoGeo pocket bios. The device
IDs supported appear to be SNK exclusive. Most likely because of the factory
blocked game data areas on these chip.

These manufacturer IDs are supported: 0x98, 0xec, 0xb0
These device IDs are supported: 0xab, 0x2c, 0x2f

There is support for 3 different sizes of flash roms: 4Mbit, 8Mbit 16Mbit. The
32Mbit games appear to be 2 16Mbit flash chips in 2 different memory regions.

The flash chips have a couple of different sized blocks. When writing to a
cartridge the neogeo pocket bios will erase the proper block and write the data
to the block.

The relation between block number and flash chip is as follows:

 # |  16Mbit (2f)  |  8Mbit (2c) |  4Mbit (ab)
---+---------------+-------------+-------------
 0 | 000000-00ffff | 00000-0ffff | 00000-0ffff
 1 | 010000-01ffff | 10000-1ffff | 10000-1ffff
 2 | 020000-02ffff | 20000-2ffff | 20000-2ffff
 3 | 030000-03ffff | 30000-3ffff | 30000-3ffff
 4 | 040000-01ffff | 40000-4ffff | 40000-4ffff
 5 | 050000-01ffff | 50000-5ffff | 50000-5ffff
 6 | 060000-01ffff | 60000-6ffff | 60000-6ffff
 7 | 070000-01ffff | 70000-7ffff | 70000-77fff
 8 | 080000-01ffff | 80000-8ffff | 78000-79fff
 9 | 090000-01ffff | 90000-9ffff | 7a000-7bfff
10 | 0a0000-01ffff | a0000-affff | 7c000-7ffff
11 | 0b0000-01ffff | b0000-bffff |
12 | 0c0000-01ffff | c0000-cffff |
13 | 0d0000-01ffff | d0000-dffff |
14 | 0e0000-01ffff | e0000-effff |
15 | 0f0000-01ffff | f0000-f7fff |
16 | 100000-10ffff | f8000-f9fff |
17 | 110000-11ffff | fa000-fbfff |
18 | 120000-12ffff | fc000-fffff |
19 | 130000-13ffff |             |
20 | 140000-14ffff |             |
21 | 150000-15ffff |             |
22 | 160000-16ffff |             |
23 | 170000-17ffff |             |
24 | 180000-18ffff |             |
25 | 190000-19ffff |             |
26 | 1a0000-1affff |             |
27 | 1b0000-1bffff |             |
28 | 1c0000-1cffff |             |
29 | 1d0000-1dffff |             |
30 | 1e0000-1effff |             |
31 | 1f0000-1f7fff |             |
32 | 1f8000-1f9fff |             |
33 | 1fa000-1fbfff |             |
34 | 1fc000-1fffff |             |

The last block is always reserved for use by the system. The Neogeo Pocket Color
bios does some tests on this last block to see if the flash functionality is
working. It does this on every boot!

The Toshiba TC58FVT004 seems to have an interface similar to what is used in
the Neogeo Pocket.


******************************************************************************/


#include "emu.h"
#include "k1ge.h"
#include "ngp_cart.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/t6w28.h"

#include "dirtc.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

constexpr uint32_t MAINRAM_SIZE = 0x3000;
constexpr uint32_t IO_REG_SIZE = 0x40;

// This encapsulates the RAM and I/O registers that are kept active when
// the system is "powered down". They are saved to an nvram file to survive
// across sessions.
// A simple driver nvram would have been saved to a file named <driver_name>.
// However, software cartridge flash contents would be written to the
// folder <driver_name>. This clashes, we work around that by using this
// ngp_sysram_device.
class ngp_sysram_device : public device_t, public device_nvram_interface
{
public:
	ngp_sysram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override ATTR_COLD;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	memory_share_creator<uint8_t> m_mainram;
	memory_share_creator<uint8_t> m_io_reg;
};


DEFINE_DEVICE_TYPE_PRIVATE(NGP_SYSRAM, device_t, ngp_sysram_device, "ngp_sysram", "SNK Neo Geo Pocket System RAM")


ngp_sysram_device::ngp_sysram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NGP_SYSRAM, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_mainram(*this, "mainram", MAINRAM_SIZE, ENDIANNESS_LITTLE)
	, m_io_reg(*this, "ioreg", IO_REG_SIZE, ENDIANNESS_LITTLE)
{
}


void ngp_sysram_device::device_start()
{
}


void ngp_sysram_device::nvram_default()
{
}


bool ngp_sysram_device::nvram_read(util::read_stream &file)
{
	uint8_t data[MAINRAM_SIZE + IO_REG_SIZE];

	auto const [err, actual] = read(file, data, sizeof(data));
	if (!err && (actual == sizeof(data)))
	{
		std::copy_n(data, MAINRAM_SIZE, &m_mainram[0]);
		std::copy_n(data + MAINRAM_SIZE, IO_REG_SIZE, &m_io_reg[0]);

		return true;
	}
	return false;
}


bool ngp_sysram_device::nvram_write(util::write_stream &file)
{
	uint8_t data[MAINRAM_SIZE + IO_REG_SIZE];

	std::copy_n(&m_mainram[0], MAINRAM_SIZE, data);
	std::copy_n(&m_io_reg[0], IO_REG_SIZE, data + MAINRAM_SIZE);

	auto const [err, actual] = write(file, data, sizeof(data));
	return !err;
}


class ngp_state : public driver_device, public device_rtc_interface
{
public:
	ngp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		device_rtc_interface(mconfig, *this),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_z80(*this, "soundcpu"),
		m_t6w28(*this, "t6w28"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot"),
		m_k1ge(*this, "k1ge"),
		m_io_controls(*this, "Controls"),
		m_io_power(*this, "Power"),
		m_io_reg(*this, "sysram:ioreg")
	{ }

	void ngp_common(machine_config &config);
	void ngp(machine_config &config);
	void ngpc(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_callback);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override ATTR_COLD;

private:
	required_device<tmp95c061_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<z80_device> m_z80;
	required_device<t6w28_device> m_t6w28;
	required_device<dac_byte_interface> m_ldac;
	required_device<dac_byte_interface> m_rdac;
	required_device<speaker_device> m_speaker;
	required_device<ngp_cart_slot_device> m_cart;
	required_device<k1ge_device> m_k1ge;
	required_ioport m_io_controls;
	required_ioport m_io_power;

	required_shared_ptr<uint8_t> m_io_reg;
	uint8_t m_old_to3 = 0;
	emu_timer* m_seconds_timer = nullptr;

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t z80_comm_r();
	void z80_comm_w(uint8_t data);
	void z80_signal_main_w(uint8_t data);

	void z80_clear_irq(uint8_t data);

	void vblank_pin_w(int state);
	void hblank_pin_w(int state);
	void tlcs900_porta(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(seconds_callback);

	void main_mem(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void z80_mem(address_map &map) ATTR_COLD;
};


TIMER_CALLBACK_MEMBER(ngp_state::seconds_callback)
{
	advance_seconds();
}


uint8_t ngp_state::io_r(offs_t offset)
{
	switch (offset)
	{
	case 0x11:  // year
		return convert_to_bcd(get_clock_register(RTC_YEAR) % 100);
	case 0x12:  // month
		return convert_to_bcd(get_clock_register(RTC_MONTH));
	case 0x13:  // day
		return convert_to_bcd(get_clock_register(RTC_DAY));
	case 0x14:  // hour
		return convert_to_bcd(get_clock_register(RTC_HOUR));
	case 0x15:  // minute
		return convert_to_bcd(get_clock_register(RTC_MINUTE));
	case 0x16:  // second
		return convert_to_bcd(get_clock_register(RTC_SECOND));
	case 0x30:  // Read controls
		return m_io_controls->read();
	case 0x31:
		return (m_io_power->read() & 0x01)
			| 0x02;  // Sub-battery OK
	}
	return m_io_reg[offset];
}


void ngp_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x11:  // year
		set_clock_register(RTC_YEAR, bcd_to_integer(data));
		break;
	case 0x12:  // month
		set_clock_register(RTC_MONTH, bcd_to_integer(data));
		break;
	case 0x13:  // day
		set_clock_register(RTC_DAY, bcd_to_integer(data));
		break;
	case 0x14:  // hour
		set_clock_register(RTC_HOUR, bcd_to_integer(data));
		break;
	case 0x15:  // minute
		set_clock_register(RTC_MINUTE, bcd_to_integer(data));
		break;
	case 0x16:  // second
		set_clock_register(RTC_SECOND, bcd_to_integer(data));
		break;
	case 0x20:  // t6w28 "right"
	case 0x21:  // t6w28 "left"
		if (m_io_reg[0x38] == 0x55 && m_io_reg[0x39] == 0xaa)
			m_t6w28->write(0, data);
		break;

	case 0x22:  // DAC right
		m_rdac->write(data);
		break;
	case 0x23:  // DAC left
		m_ldac->write(data);
		break;

	case 0x36: // 50 written when system powers down, 05 written when system starts up
	case 0x37:
		break;
	case 0x38:  // Sound enable/disable.
		switch (data)
		{
		case 0x55:  // Enabled sound
			m_t6w28->set_enable(true);
			break;
		case 0xaa:  // Disable sound
			m_t6w28->set_enable(false);
			break;
		}
		break;

	case 0x39:  // Z80 enable/disable.
		switch (data)
		{
		case 0x55:  // Enable Z80
			m_z80->resume(SUSPEND_REASON_HALT);
			m_z80->reset();
			m_z80->set_input_line(0, CLEAR_LINE);
			break;
		case 0xaa:  // Disable Z80
			m_z80->suspend(SUSPEND_REASON_HALT, 1);
			break;
		}
		break;

	case 0x3a:  // Trigger Z80 NMI
		m_z80->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		break;
	}
	m_io_reg[offset] = data;
}


void ngp_state::main_mem(address_map &map)
{
	map(0x00'0080, 0x00'00bf).rw(FUNC(ngp_state::io_r), FUNC(ngp_state::io_w)).share("sysram:ioreg");
	map(0x00'4000, 0x00'6fff).ram().share("sysram:mainram");
	map(0x00'7000, 0x00'7fff).ram().share("soundram");
	map(0x00'8000, 0x00'bfff).rw(m_k1ge, FUNC(k1ge_device::read), FUNC(k1ge_device::write));
	map(0xff'0000, 0xff'ffff).rom().region("maincpu", 0);
}


uint8_t ngp_state::z80_comm_r()
{
	return m_io_reg[0x3c];
}


void ngp_state::z80_comm_w(uint8_t data)
{
	m_io_reg[0x3c] = data;
}


void ngp_state::z80_signal_main_w(uint8_t data)
{
	m_maincpu->set_input_line(TLCS900_INT5, ASSERT_LINE);
}


void ngp_state::z80_mem(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("soundram");                    /* shared with tlcs900 */
	map(0x4000, 0x4001).w(m_t6w28, FUNC(t6w28_device::write));      /* sound chip (right, left) */
	map(0x8000, 0x8000).rw(FUNC(ngp_state::z80_comm_r), FUNC(ngp_state::z80_comm_w));  /* main-sound communication */
	map(0xc000, 0xc000).w(FUNC(ngp_state::z80_signal_main_w));               /* signal irq to main cpu */
}


void ngp_state::z80_clear_irq(uint8_t data)
{
	m_z80->set_input_line(0, CLEAR_LINE);

	// Unknown what causes the maincpu INT5 signal to be cleared. This will do for now.
	m_maincpu->set_input_line(TLCS900_INT5, CLEAR_LINE);
}


void ngp_state::z80_io(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(ngp_state::z80_clear_irq));
}


INPUT_CHANGED_MEMBER(ngp_state::power_callback)
{
	if (BIT(m_io_reg[0x33], 2))
		m_maincpu->set_input_line(TLCS900_NMI, (BIT(m_io_power->read(), 0) ? CLEAR_LINE : ASSERT_LINE));
}


static INPUT_PORTS_START(ngp)
	PORT_START("Controls")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_NAME("Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_NAME("Down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_NAME("Left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Button A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Button B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SELECT) PORT_NAME("Option")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("Power")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngp_state::power_callback), 0)
INPUT_PORTS_END


void ngp_state::vblank_pin_w(int state)
{
	m_maincpu->set_input_line(TLCS900_INT4, state ? ASSERT_LINE : CLEAR_LINE);
}


void ngp_state::hblank_pin_w(int state)
{
	m_maincpu->set_input_line(TLCS900_TIO, state ? ASSERT_LINE : CLEAR_LINE);
}


void ngp_state::tlcs900_porta(offs_t offset, uint8_t data)
{
	int to3 = BIT(data,3);

	if (to3 && !m_old_to3)
		m_z80->set_input_line(0, ASSERT_LINE);

	m_old_to3 = to3;
}


void ngp_state::machine_start()
{
	m_seconds_timer = timer_alloc(FUNC(ngp_state::seconds_callback), this);
	m_seconds_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	save_item(NAME(m_old_to3));
}


void ngp_state::machine_reset()
{
	m_old_to3 = 0;

	m_z80->suspend(SUSPEND_REASON_HALT, 1);
	m_z80->set_input_line(0, CLEAR_LINE);

	// Enforce the cpu to start executing as if the power button had just been pushed.
	m_maincpu->set_state_int(tlcs900_device::TLCS900_PC, 0xff'1800);
}


void ngp_state::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_io_reg[0x16] = convert_to_bcd(second);
	m_io_reg[0x15] = convert_to_bcd(minute);
	m_io_reg[0x14] = convert_to_bcd(hour);
	m_io_reg[0x13] = convert_to_bcd(day);
	m_io_reg[0x12] = convert_to_bcd(month);
	m_io_reg[0x11] = convert_to_bcd(year % 100);
}


void ngp_state::ngp_common(machine_config &config)
{
	NGP_SYSRAM(config, "sysram", 0U);

	TMP95C061(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_am8_16(1);
	m_maincpu->set_addrmap(AS_PROGRAM, &ngp_state::main_mem);
	m_maincpu->porta_write().set(FUNC(ngp_state::tlcs900_porta));
	m_maincpu->an_read<0>().set_constant(0x3ff); // main battery power

	Z80(config, m_z80, 6.144_MHz_XTAL/2);
	m_z80->set_addrmap(AS_PROGRAM, &ngp_state::z80_mem);
	m_z80->set_addrmap(AS_IO, &ngp_state::z80_io);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_raw(6.144_MHz_XTAL, 515, 0, 160 /*480*/, 199, 0, 152);

	// sound hardware
	SPEAKER(config, m_speaker, 2).front();

	T6W28(config, m_t6w28, 6.144_MHz_XTAL/2);
	m_t6w28->add_route(0, m_speaker, 0.50, 0);
	m_t6w28->add_route(1, m_speaker, 0.50, 1);

	DAC_8BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.25, 0); // unknown DAC
	DAC_8BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.25, 1); // unknown DAC

	NGP_CART_SLOT(config, m_cart, generic_plain_slot, "ngp_cart", "bin,ngp,npc,ngc");
	m_cart->set_address_space(m_maincpu, AS_PROGRAM);
}


void ngp_state::ngp(machine_config &config)
{
	ngp_common(config);

	K1GE(config, m_k1ge, 6.144_MHz_XTAL, m_screen);
	m_k1ge->vblank_callback().set(FUNC(ngp_state::vblank_pin_w));
	m_k1ge->hblank_callback().set(FUNC(ngp_state::hblank_pin_w));

	m_screen->set_screen_update(m_k1ge, FUNC(k1ge_device::screen_update));
	m_screen->set_palette(m_k1ge);

	SOFTWARE_LIST(config, "cart_list").set_original("ngp");
	SOFTWARE_LIST(config, "ngpc_list").set_compatible("ngpc");
}


void ngp_state::ngpc(machine_config &config)
{
	ngp_common(config);
	K2GE(config, m_k1ge, 6.144_MHz_XTAL, m_screen);
	m_k1ge->vblank_callback().set(FUNC(ngp_state::vblank_pin_w));
	m_k1ge->hblank_callback().set(FUNC(ngp_state::hblank_pin_w));

	m_screen->set_screen_update(m_k1ge, FUNC(k1ge_device::screen_update));
	m_screen->set_palette(m_k1ge);

	SOFTWARE_LIST(config, "cart_list").set_original("ngpc");
	SOFTWARE_LIST(config, "ngp_list").set_compatible("ngp");
}


ROM_START(ngp)
	ROM_REGION(0x10000, "maincpu" , 0)
	ROM_LOAD("ngp_bios.ngp", 0x0000, 0x10000, CRC(6232df8d) SHA1(2f6429b68446536d8b03f35d02f1e98beb6460a0))
ROM_END


ROM_START(ngpc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ngpcbios.rom", 0x0000, 0x10000, CRC(6eeb6f40) SHA1(edc13192054a59be49c6d55f83b70e2510968e86))
ROM_END

} // anonymous namespace


//   YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE      INIT        COMPANY  FULLNAME               FLAGS
CONS(1998, ngp,  0,      0,      ngp,     ngp,   ngp_state, empty_init, "SNK",   "NeoGeo Pocket",       MACHINE_SUPPORTS_SAVE)
CONS(1999, ngpc, ngp,    0,      ngpc,    ngp,   ngp_state, empty_init, "SNK",   "NeoGeo Pocket Color", MACHINE_SUPPORTS_SAVE)
