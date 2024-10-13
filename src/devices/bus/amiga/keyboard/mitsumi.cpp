// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    mitsumi.cpp

    Fifteen rows by six columns, plus seven dedicated meta key inputs.

    Newer external keyboards and the Amiga 500 keyboard have an external
    watchdog circuit built around a 74LS123.  Older external keyboards
    and the Amiga 600 keyboard lack this.  If CNTR is tied low, the
    program expects the watchdog to be present.

    Note that the Ctrl-Amiga-Amiga detection circuit is not present in
    external "big box" keyboards.  It's implemented in the base class
    here for convenience.  In the Amiga 600, it's possible for the MCU
    to read or drive the input to the reset pulse generator via PA7
    (assuming R624 has a low enough resistance), but this isn't used by
    the program.

    The Amiga 500 and Amiga 600 keyboards generate a reset pulse on a
    dedicated line using an NE555 timer.  These systems won't generate a
    system reset in response to the KCLK line being held low for an
    extended period.  No reset warning is generated before the reset
    pulse on these systems.

    The Amiga 500 keyboard MCU is in the keyboard assembly.  The MCU
    reset signal is generated using logic in the keyboard assembly.  The
    main system reset signal is not used.

    The NE555 used to generate the reset pulse on Amiga 600 keyboards
    also provides power-on reset, and its output is combined with a
    power good signal on Rev 2C and later machines.  The Amiga 600
    keyboard MCU is on the system board and is reset by the main system
    reset signal.

    The Amiga 1000 keyboard uses an NE556 to generate a power-on reset
    sigal for the MCU.  There is no watchdog circuit.  The CNTR pin is
    unconnected, relying on the internal pull-up.

    Newer external keyboards have a 74HC00 in addition to the 74LS123.
    The exact purpose of this chip is unknown.

    We need better photos of the Hi-Tek version of the Amiga 2000
    keyboard to identify the chips (40-pin MCU, two 14-pin DIPs, one
    8-pin DIP).

    Known Amiga 500 keyboard part numbers:
    * 312502-01 U.S./Canada
    * 312502-02 Germany/Austria
    * 312502-03 France/Belgium
    * 312502-04 Italy
    * 312502-05 Sweden/Finland
    * 312502-06 Spain
    * 312502-07 Denmark
    * 312502-08 Switzerland
    * 312502-09 Norway
    * 312502-12 UK

    Known Amiga 1000 keyboard part numbers:
    * 327063-01 U.S./Canada
    * 327063-02 UK
    * 327063-03 Germany
    * 327063-04 France
    * 327063-05 Italy

    Known Amiga 2000 keyboard part numbers:
    * 312716-02 U.S./Canada

    Known Amiga CDTV keyboard part numbers:
    * 364351-01 U.S./Canada

***************************************************************************/

#include "emu.h"
#include "mitsumi.h"

#include "matrix.h"

#include "cpu/m6502/m6500_1.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"


namespace {

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START(keyboard_old)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD("6500-1", 0x0000, 0x0800, CRC(4a3fc332) SHA1(83b21d0c8b93fc9b9b3b287fde4ec8f3badac5a2) BAD_DUMP) // using newer program until we get a dump
ROM_END

ROM_START(keyboard_new)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD("6570-036", 0x0000, 0x0800, CRC(4a3fc332) SHA1(83b21d0c8b93fc9b9b3b287fde4ec8f3badac5a2))
ROM_END


//**************************************************************************
//  KEYBOARD BASE CLASSES
//**************************************************************************

class mitsumi_keyboard_base : public device_t, public device_amiga_keyboard_interface
{
public:
	virtual void kdat_w(int state) override
	{
		m_kdat_in = state ? 0x01U : 0x00U;
		m_mcu->pa_w(m_meta->read());
	}

	int kdat_r()
	{
		return m_kdat_in ^ 0x01U;
	}

	ioport_value cols_r()
	{
		ioport_value result(0xffU);
		for (unsigned i = 0U; m_rows.size() > i; ++i)
		{
			if (!BIT(m_row_drive, m_rows.size() - 1 - i))
				result &= m_rows[i]->read();
		}
		return (result >> 2) ^ 0x3fU;
	}

	int reset_r()
	{
		return m_ctrl_a_a;
	}

	DECLARE_INPUT_CHANGED_MEMBER(check_ctrl_a_a)
	{
		u8 const state((m_meta->read() & 0x4cU) ? 0x00 : 0x01);
		if (state != m_ctrl_a_a)
		{
			m_ctrl_a_a = state;
			ctrl_a_a_changed(bool(state));
		}
	}

protected:
	mitsumi_keyboard_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_amiga_keyboard_interface(mconfig, *this)
		, m_mcu{ *this, "mcu" }
		, m_rows{ *this, "ROW%u", 0U }
		, m_meta{ *this, "META" }
	{
	}

	virtual tiny_rom_entry const *device_rom_region() const override
	{
		return ROM_NAME(keyboard_new);
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		M6500_1(config, m_mcu, 3_MHz_XTAL);
		m_mcu->pa_in_cb().set_ioport("COLS");
		m_mcu->pb_in_cb().set_ioport("META");
		m_mcu->pa_out_cb().set([this] (u8 data) { m_host->kdat_w(data); }).bit(0);
		m_mcu->pa_out_cb().append([this] (u8 data) { m_host->kclk_w(data); }).bit(1);
		m_mcu->pb_out_cb().set_output("led_kbd_caps").bit(7);
		m_mcu->pc_out_cb().set([this] (u8 data) { m_row_drive = (m_row_drive & 0xff00U) | u16(data); });
		m_mcu->pd_out_cb().set([this] (u8 data) { m_row_drive = (m_row_drive & 0x80ffU) | (u16(data) << 8); }).mask(0x7f);
	}

	virtual void device_start() override
	{
		save_item(NAME(m_row_drive));
		save_item(NAME(m_kdat_in));
		save_item(NAME(m_ctrl_a_a));
	}

	virtual void ctrl_a_a_changed(bool state)
	{
	}

	required_device<m6500_1_device> m_mcu;

private:
	required_ioport_array<15>   m_rows;
	required_ioport             m_meta;

	u16 m_row_drive = 0xffffU;
	u8  m_kdat_in = 0x01;
	u8  m_ctrl_a_a = 0x00;
};


class mitsumi_watchdog_keyboard_base : public mitsumi_keyboard_base
{
protected:
	using mitsumi_keyboard_base::mitsumi_keyboard_base;

	virtual void device_add_mconfig(machine_config &config) override
	{
		mitsumi_keyboard_base::device_add_mconfig(config);

		m_mcu->pd_out_cb().append(FUNC(mitsumi_watchdog_keyboard_base::pd7_w)).bit(7);
	}

	virtual void device_start() override
	{
		mitsumi_keyboard_base::device_start();

		m_wd_timeout = timer_alloc(FUNC(mitsumi_watchdog_keyboard_base::wd_timeout), this);
		m_wd_pulse = timer_alloc(FUNC(mitsumi_watchdog_keyboard_base::wd_pulse), this);

		m_pd7 = 0x01U;

		save_item(NAME(m_pd7));
	}

	virtual void device_reset() override
	{
		mitsumi_keyboard_base::device_reset();

		m_mcu->cntr_w(0);
	}

	virtual void watchdog_changed(bool state)
	{
		m_mcu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
	}

private:
	void pd7_w(int state)
	{
		if (bool(state) != bool(m_pd7))
		{
			m_pd7 = state ? 0x01U : 0x00U;
			if (!state)
			{
				// 74LS123 with Rt = 120kΩ and Cext = 1µF
				// t = K * Rt * Cext * (1 + (0.7 / Rt)) = 0.28 * 120k * 1µ * (1 + (0.7 / 120k)) ≈ 33.6ms
				m_wd_timeout->adjust(attotime::from_usec(33600));
			}
		}
	}

	TIMER_CALLBACK_MEMBER(wd_timeout)
	{
		// 74LS123 with Rt = 10kΩ and Cext = 100nF
		// t = K * Rt * Cext * (1 + (0.7 / Rt)) = 0.28 * 10k * 100n * (1 + (0.7 / 10k)) ≈ 280µs
		m_wd_pulse->adjust(attotime::from_usec(280));
		watchdog_changed(true);
	}

	TIMER_CALLBACK_MEMBER(wd_pulse)
	{
		watchdog_changed(false);
	}

	emu_timer *m_wd_timeout = nullptr;
	emu_timer *m_wd_pulse = nullptr;

	u8 m_pd7 = 0x01U;
};


class a500_keyboard_base : public mitsumi_watchdog_keyboard_base
{
protected:
	a500_keyboard_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, type, tag, owner, clock)
		, m_reset_merger(*this, "reset")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		mitsumi_watchdog_keyboard_base::device_add_mconfig(config);

		INPUT_MERGER_ANY_HIGH(config, m_reset_merger).output_handler().set_inputline(m_mcu, INPUT_LINE_RESET);
	}

	virtual void device_start() override
	{
		mitsumi_watchdog_keyboard_base::device_start();

		m_reset_pulse = timer_alloc(FUNC(a500_keyboard_base::reset_pulse), this);

		m_reset_active = 0U;

		save_item(NAME(m_reset_active));
	}

	virtual void ctrl_a_a_changed(bool state) override
	{
		if (state && !m_reset_active)
		{
			m_reset_active = 1U;
			m_host->krst_w(0);
			m_reset_merger->in_w<0>(1);
			m_reset_pulse->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(47), CAP_U(10)));
		}
	}

	virtual void watchdog_changed(bool state) override
	{
		m_reset_merger->in_w<1>(state ? 1 : 0);
	}

private:
	TIMER_CALLBACK_MEMBER(reset_pulse)
	{
		m_reset_active = 0U;
		m_host->krst_w(1);
		m_reset_merger->in_w<0>(0);
	}

	required_device<input_merger_device> m_reset_merger;

	emu_timer *m_reset_pulse = nullptr;
	u8 m_reset_active = 0U;
};


class a600_keyboard_base : public mitsumi_keyboard_base
{
protected:
	a600_keyboard_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
		: mitsumi_keyboard_base(mconfig, type, tag, owner, clock)
		, m_reset_merger(*this, "reset")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		mitsumi_keyboard_base::device_add_mconfig(config);

		m_mcu->pa_out_cb().append(m_reset_merger, FUNC(input_merger_device::in_w<0>)).bit(7);

		INPUT_MERGER_ANY_LOW(config, m_reset_merger).output_handler().set(FUNC(a600_keyboard_base::reset_trigger));
	}

	virtual void device_start() override
	{
		mitsumi_keyboard_base::device_start();

		m_reset_pulse = timer_alloc(FUNC(a600_keyboard_base::reset_pulse), this);

		m_reset_trigger = 0U;
		m_reset_active = 0U;

		save_item(NAME(m_reset_trigger));
		save_item(NAME(m_reset_active));
	}

	virtual void device_reset() override
	{
		mitsumi_keyboard_base::device_reset();

		m_mcu->cntr_w(1);
	}

	virtual void ctrl_a_a_changed(bool state) override
	{
		m_reset_merger->in_w<1>(state ? 0 : 1);
	}

private:
	void reset_trigger(int state)
	{
		if (bool(state) != bool(m_reset_trigger))
		{
			m_reset_trigger = state ? 1U : 0U;
			if (state & !m_reset_active)
			{
				m_reset_active = 1U;
				m_host->krst_w(0);
				m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_reset_pulse->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(47), CAP_U(10)));
			}
		}
	}

	TIMER_CALLBACK_MEMBER(reset_pulse)
	{
		m_reset_active = 0U;
		m_host->krst_w(1);
		m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}

	required_device<input_merger_device> m_reset_merger;

	emu_timer *m_reset_pulse = nullptr;

	u8 m_reset_trigger = 0U, m_reset_active = 0U;
};


class a1000_keyboard_base : public mitsumi_keyboard_base
{
protected:
	using mitsumi_keyboard_base::mitsumi_keyboard_base;

	virtual tiny_rom_entry const *device_rom_region() const override
	{
		return ROM_NAME(keyboard_old);
	}
};


//**************************************************************************
//  COMMON PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START(fullsize_cols)
	PORT_START("COLS")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(mitsumi_keyboard_base, cols_r)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(mitsumi_keyboard_base, kdat_r)
INPUT_PORTS_END

INPUT_PORTS_START(compact_cols)
	PORT_START("COLS")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(mitsumi_keyboard_base, reset_r)
	PORT_BIT(0x7c, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(mitsumi_keyboard_base, cols_r)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(mitsumi_keyboard_base, kdat_r)
INPUT_PORTS_END

INPUT_PORTS_START(mitsumi_meta)
	PORT_START("META")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)        PORT_CHAR(UCHAR_MAMEKEY(LWIN))       PORT_NAME("Left Amiga")   PORT_CHANGED_MEMBER(DEVICE_SELF, mitsumi_keyboard_base, check_ctrl_a_a, 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(LALT))       PORT_NAME("Left Alt")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)             PORT_NAME("Left Shift")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))   PORT_NAME("Ctrl")         PORT_CHANGED_MEMBER(DEVICE_SELF, mitsumi_keyboard_base, check_ctrl_a_a, 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)        PORT_CHAR(UCHAR_MAMEKEY(RWIN))       PORT_NAME("Right Amiga")  PORT_CHANGED_MEMBER(DEVICE_SELF, mitsumi_keyboard_base, check_ctrl_a_a, 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_SHIFT_2)             PORT_NAME("Right Alt")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))     PORT_NAME("Right Shift")
INPUT_PORTS_END


//**************************************************************************
//  A1000 KEYBOARD PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START(a1000_us)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_us(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(a1000_de)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_de(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(a1000_fr)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_fr(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(a1000_it)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_it(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(a1000_se)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_se(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);

	// Commodore shipped A1000 U.S./Canada layout with stickers on key caps for Sweden/Finland
	PORT_MODIFY("ROW0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("ROW12")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("ROW13")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('|')
INPUT_PORTS_END

INPUT_PORTS_START(a1000_dk)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_dk(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);

	// Commodore shipped A1000 U.S./Canada layout with stickers on key caps for Denmark
	PORT_MODIFY("ROW0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("ROW12")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("ROW13")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('|')
INPUT_PORTS_END

INPUT_PORTS_START(a1000_gb)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_gb(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_a1000_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END


//**************************************************************************
//  FULL SIZE KEYBOARD PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START(fullsize_us)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_us(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_de)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_de(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_fr)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_fr(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_it)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_it(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_se)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_se(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_es)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_es(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_dk)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_dk(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_ch)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_ch(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_no)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_no(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(fullsize_gb)
	PORT_INCLUDE(fullsize_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_gb(owner, portlist, errorbuf);
INPUT_PORTS_END


//**************************************************************************
//  COMPACT KEYBOARD PORT DEFINITIONS
//**************************************************************************

INPUT_PORTS_START(compact_us)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_us(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_de)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_de(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_fr)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_fr(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_it)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_it(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_se)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_se(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_es)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_es(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_dk)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_dk(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_ch)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_ch(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_no)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_no(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END

INPUT_PORTS_START(compact_gb)
	PORT_INCLUDE(compact_cols)
	PORT_INCLUDE(mitsumi_meta)
	bus::amiga::keyboard::construct_ioport_matrix_gb(owner, portlist, errorbuf);
	bus::amiga::keyboard::construct_ioport_remove_keypad(owner, portlist, errorbuf);
INPUT_PORTS_END


//**************************************************************************
//  A500 KEYBOARD CLASSES
//**************************************************************************

class a500_keyboard_us : public a500_keyboard_base
{
public:
	a500_keyboard_us(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_US, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_us); }
};

class a500_keyboard_de : public a500_keyboard_base
{
public:
	a500_keyboard_de(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_DE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_de); }
};

class a500_keyboard_fr : public a500_keyboard_base
{
public:
	a500_keyboard_fr(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_FR, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_fr); }
};

class a500_keyboard_it : public a500_keyboard_base
{
public:
	a500_keyboard_it(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_IT, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_it); }
};

class a500_keyboard_se : public a500_keyboard_base
{
public:
	a500_keyboard_se(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_SE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_se); }
};

class a500_keyboard_es : public a500_keyboard_base
{
public:
	a500_keyboard_es(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_ES, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_es); }
};

class a500_keyboard_dk : public a500_keyboard_base
{
public:
	a500_keyboard_dk(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_DK, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_dk); }
};

class a500_keyboard_ch : public a500_keyboard_base
{
public:
	a500_keyboard_ch(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_CH, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_ch); }
};

class a500_keyboard_no : public a500_keyboard_base
{
public:
	a500_keyboard_no(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_NO, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_no); }
};

class a500_keyboard_gb : public a500_keyboard_base
{
public:
	a500_keyboard_gb(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a500_keyboard_base(mconfig, A500_KBD_GB, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_gb); }
};


//**************************************************************************
//  A600 KEYBOARD CLASSES
//**************************************************************************

class a600_keyboard_us : public a600_keyboard_base
{
public:
	a600_keyboard_us(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_US, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_us); }
};

class a600_keyboard_de : public a600_keyboard_base
{
public:
	a600_keyboard_de(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_DE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_de); }
};

class a600_keyboard_fr : public a600_keyboard_base
{
public:
	a600_keyboard_fr(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_FR, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_fr); }
};

class a600_keyboard_it : public a600_keyboard_base
{
public:
	a600_keyboard_it(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_IT, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_it); }
};

class a600_keyboard_se : public a600_keyboard_base
{
public:
	a600_keyboard_se(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_SE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_se); }
};

class a600_keyboard_es : public a600_keyboard_base
{
public:
	a600_keyboard_es(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_ES, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_es); }
};

class a600_keyboard_dk : public a600_keyboard_base
{
public:
	a600_keyboard_dk(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_DK, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_dk); }
};

class a600_keyboard_ch : public a600_keyboard_base
{
public:
	a600_keyboard_ch(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_CH, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_ch); }
};

class a600_keyboard_no : public a600_keyboard_base
{
public:
	a600_keyboard_no(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_NO, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_no); }
};

class a600_keyboard_gb : public a600_keyboard_base
{
public:
	a600_keyboard_gb(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a600_keyboard_base(mconfig, A600_KBD_GB, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(compact_gb); }
};


//**************************************************************************
//  A1000 KEYBOARD CLASSES
//**************************************************************************

class a1000_keyboard_us : public a1000_keyboard_base
{
public:
	a1000_keyboard_us(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_US, tag, owner, clock)
	{ }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_us); }
};

class a1000_keyboard_de : public a1000_keyboard_base
{
public:
	a1000_keyboard_de(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_DE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_de); }
};

class a1000_keyboard_fr : public a1000_keyboard_base
{
public:
	a1000_keyboard_fr(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_FR, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_fr); }
};

class a1000_keyboard_it : public a1000_keyboard_base
{
public:
	a1000_keyboard_it(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_IT, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_it); }
};

class a1000_keyboard_se : public a1000_keyboard_base
{
public:
	a1000_keyboard_se(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_SE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_se); }
};

class a1000_keyboard_dk : public a1000_keyboard_base
{
public:
	a1000_keyboard_dk(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_DK, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_dk); }
};

class a1000_keyboard_gb : public a1000_keyboard_base
{
public:
	a1000_keyboard_gb(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: a1000_keyboard_base(mconfig, A1000_KBD_GB, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A1000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(a1000_gb); }
};


//**************************************************************************
//  A2000/A3000/A4000/CDTV KEYBOARD CLASSES
//**************************************************************************

class a2000_keyboard_us : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_us(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_US, tag, owner, clock)
	{ }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_us); }
};

class a2000_keyboard_de : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_de(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_DE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_de); }
};

class a2000_keyboard_fr : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_fr(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_FR, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_fr); }
};

class a2000_keyboard_it : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_it(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_IT, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_it); }
};

class a2000_keyboard_se : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_se(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_SE, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_se); }
};

class a2000_keyboard_es : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_es(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_ES, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_es); }
};

class a2000_keyboard_dk : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_dk(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_DK, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_dk); }
};

class a2000_keyboard_ch : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_ch(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_CH, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_ch); }
};

class a2000_keyboard_no : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_no(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_NO, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_no); }
};

class a2000_keyboard_gb : public mitsumi_watchdog_keyboard_base
{
public:
	a2000_keyboard_gb(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: mitsumi_watchdog_keyboard_base(mconfig, A2000_KBD_GB, tag, owner, clock)
	{ }

	static auto parent_rom_device_type() { return &A2000_KBD_US; }

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(fullsize_gb); }
};

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_US, device_amiga_keyboard_interface, a500_keyboard_us, "a500kbd_us", "Amiga 500 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_DE, device_amiga_keyboard_interface, a500_keyboard_de, "a500kbd_de", "Amiga 500 Keyboard (Germany/Austria)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_FR, device_amiga_keyboard_interface, a500_keyboard_fr, "a500kbd_fr", "Amiga 500 Keyboard (France/Belgium)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_IT, device_amiga_keyboard_interface, a500_keyboard_it, "a500kbd_it", "Amiga 500 Keyboard (Italy)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_SE, device_amiga_keyboard_interface, a500_keyboard_se, "a500kbd_se", "Amiga 500 Keyboard (Sweden/Finland)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_ES, device_amiga_keyboard_interface, a500_keyboard_es, "a500kbd_es", "Amiga 500 Keyboard (Spain)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_DK, device_amiga_keyboard_interface, a500_keyboard_dk, "a500kbd_dk", "Amiga 500 Keyboard (Denmark)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_CH, device_amiga_keyboard_interface, a500_keyboard_ch, "a500kbd_ch", "Amiga 500 Keyboard (Switzerland)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_NO, device_amiga_keyboard_interface, a500_keyboard_no, "a500kbd_no", "Amiga 500 Keyboard (Norway)")
DEFINE_DEVICE_TYPE_PRIVATE(A500_KBD_GB, device_amiga_keyboard_interface, a500_keyboard_gb, "a500kbd_gb", "Amiga 500 Keyboard (UK)")

DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_US, device_amiga_keyboard_interface, a600_keyboard_us, "a600kbd_us", "Amiga 600 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_DE, device_amiga_keyboard_interface, a600_keyboard_de, "a600kbd_de", "Amiga 600 Keyboard (Germany/Austria)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_FR, device_amiga_keyboard_interface, a600_keyboard_fr, "a600kbd_fr", "Amiga 600 Keyboard (France/Belgium)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_IT, device_amiga_keyboard_interface, a600_keyboard_it, "a600kbd_it", "Amiga 600 Keyboard (Italy)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_SE, device_amiga_keyboard_interface, a600_keyboard_se, "a600kbd_se", "Amiga 600 Keyboard (Sweden/Finland)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_ES, device_amiga_keyboard_interface, a600_keyboard_es, "a600kbd_es", "Amiga 600 Keyboard (Spain)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_DK, device_amiga_keyboard_interface, a600_keyboard_dk, "a600kbd_dk", "Amiga 600 Keyboard (Denmark)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_CH, device_amiga_keyboard_interface, a600_keyboard_ch, "a600kbd_ch", "Amiga 600 Keyboard (Switzerland)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_NO, device_amiga_keyboard_interface, a600_keyboard_no, "a600kbd_no", "Amiga 600 Keyboard (Norway)")
DEFINE_DEVICE_TYPE_PRIVATE(A600_KBD_GB, device_amiga_keyboard_interface, a600_keyboard_gb, "a600kbd_gb", "Amiga 600 Keyboard (UK)")

DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_US, device_amiga_keyboard_interface, a1000_keyboard_us, "a1000kbd_us", "Amiga 1000 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_DE, device_amiga_keyboard_interface, a1000_keyboard_de, "a1000kbd_de", "Amiga 1000 Keyboard (Germany/Austria)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_FR, device_amiga_keyboard_interface, a1000_keyboard_fr, "a1000kbd_fr", "Amiga 1000 Keyboard (France/Belgium)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_IT, device_amiga_keyboard_interface, a1000_keyboard_it, "a1000kbd_it", "Amiga 1000 Keyboard (Italy)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_SE, device_amiga_keyboard_interface, a1000_keyboard_se, "a1000kbd_se", "Amiga 1000 Keyboard (Sweden/Finland)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_DK, device_amiga_keyboard_interface, a1000_keyboard_dk, "a1000kbd_dk", "Amiga 1000 Keyboard (Denmark)")
DEFINE_DEVICE_TYPE_PRIVATE(A1000_KBD_GB, device_amiga_keyboard_interface, a1000_keyboard_gb, "a1000kbd_gb", "Amiga 1000 Keyboard (UK)")

DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_US, device_amiga_keyboard_interface, a2000_keyboard_us, "a2000kbd_us", "Amiga 2000/3000/4000 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_DE, device_amiga_keyboard_interface, a2000_keyboard_de, "a2000kbd_de", "Amiga 2000/3000/4000 Keyboard (Germany/Austria)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_FR, device_amiga_keyboard_interface, a2000_keyboard_fr, "a2000kbd_fr", "Amiga 2000/3000/4000 Keyboard (France/Belgium)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_IT, device_amiga_keyboard_interface, a2000_keyboard_it, "a2000kbd_it", "Amiga 2000/3000/4000 Keyboard (Italy)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_SE, device_amiga_keyboard_interface, a2000_keyboard_se, "a2000kbd_se", "Amiga 2000/3000/4000 Keyboard (Sweden/Finland)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_ES, device_amiga_keyboard_interface, a2000_keyboard_es, "a2000kbd_es", "Amiga 2000/3000/4000 Keyboard (Spain)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_DK, device_amiga_keyboard_interface, a2000_keyboard_dk, "a2000kbd_dk", "Amiga 2000/3000/4000 Keyboard (Denmark)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_CH, device_amiga_keyboard_interface, a2000_keyboard_ch, "a2000kbd_ch", "Amiga 2000/3000/4000 Keyboard (Switzerland)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_NO, device_amiga_keyboard_interface, a2000_keyboard_no, "a2000kbd_no", "Amiga 2000/3000/4000 Keyboard (Norway)")
DEFINE_DEVICE_TYPE_PRIVATE(A2000_KBD_GB, device_amiga_keyboard_interface, a2000_keyboard_gb, "a2000kbd_gb", "Amiga 2000/3000/4000 Keyboard (UK)")
