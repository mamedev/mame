// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Intel INTELLEC® 4/MOD 40

 Set the terminal for 110 1/8/N/2 to talk to the monitor.
 It only likes to see uppercase letters, digits, comma, and carriage return.
 Paper tape reader run/stop is sent to RTS on the serial port.
 */
#include "emu.h"

#include "bus/intellec4/intellec4.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs40/mcs40.h"
#include "machine/bankdev.h"


namespace {

class intellec4_40_state : public driver_device
{
public:
	intellec4_40_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "maincpu")
		, m_program_banks(*this, "prgbank")
		, m_io_banks(*this, "iobank")
		, m_bus(*this, "bus")
		, m_tty(*this, "tty")
		, m_ram(*this, "ram")
		, m_sw_mode(*this, "MODE")
	{
	}

	DECLARE_READ8_MEMBER(pm_read);
	DECLARE_WRITE8_MEMBER(pm_write);

	DECLARE_READ8_MEMBER(rom0_in);
	DECLARE_READ8_MEMBER(rome_in);
	DECLARE_READ8_MEMBER(romf_in);

	DECLARE_WRITE8_MEMBER(rome_out);
	DECLARE_WRITE8_MEMBER(romf_out);

	DECLARE_WRITE8_MEMBER(ram0_out);
	DECLARE_WRITE8_MEMBER(ram1_out);

	DECLARE_WRITE_LINE_MEMBER(stp_ack);

	// universal slot outputs
	DECLARE_WRITE_LINE_MEMBER(bus_stop);
	DECLARE_WRITE_LINE_MEMBER(bus_test);
	DECLARE_WRITE_LINE_MEMBER(bus_reset_4002);
	DECLARE_WRITE_LINE_MEMBER(bus_user_reset);

	// edge-sensitive front-panel switches
	DECLARE_INPUT_CHANGED_MEMBER(sw_stop);
	DECLARE_INPUT_CHANGED_MEMBER(sw_single_step);
	DECLARE_INPUT_CHANGED_MEMBER(sw_reset);
	DECLARE_INPUT_CHANGED_MEMBER(sw_mon);
	DECLARE_INPUT_CHANGED_MEMBER(sw_ram);
	DECLARE_INPUT_CHANGED_MEMBER(sw_prom);

protected:
	virtual void driver_start() override;
	virtual void driver_reset() override;

private:
	enum
	{
		BANK_PRG_MON = 0,
		BANK_PRG_RAM,
		BANK_PRG_PROM,
		BANK_PRG_NONE,

		BANK_IO_MON = 0,
		BANK_IO_NEITHER,
		BANK_IO_PROM,

		BIT_SW_STOP = 0,
		BIT_SW_SINGLE_STEP,
		BIT_SW_RESET,
		BIT_SW_MON,
		BIT_SW_RAM,
		BIT_SW_PROM
	};
	enum : ioport_value
	{
		MASK_SW_STOP        = 1U << BIT_SW_STOP,
		MASK_SW_SINGLE_STEP = 1U << BIT_SW_SINGLE_STEP,
		MASK_SW_RESET       = 1U << BIT_SW_RESET,
		MASK_SW_MON         = 1U << BIT_SW_MON,
		MASK_SW_RAM         = 1U << BIT_SW_RAM,
		MASK_SW_PROM        = 1U << BIT_SW_PROM
	};

	TIMER_CALLBACK_MEMBER(single_step_expired);
	TIMER_CALLBACK_MEMBER(reset_expired);

	void trigger_reset();

	required_device<cpu_device>                         m_cpu;
	required_device<address_map_bank_device>            m_program_banks, m_io_banks;
	required_device<bus::intellec4::univ_bus_device>    m_bus;
	required_device<rs232_port_device>                  m_tty;

	required_shared_ptr<u8>         m_ram;

	required_ioport                 m_sw_mode;

	emu_timer   *m_single_step_timer = nullptr;
	emu_timer   *m_reset_timer = nullptr;

	// program memory access
	u8      m_ram_page = false, m_ram_data = false;
	bool    m_ram_write = false;

	// control board state
	bool    m_stp_ack = false, m_single_step = false;
	bool    m_ff_mon = true, m_ff_ram = false, m_ff_prom = false;

	// current state of signals from bus
	bool    m_bus_stop = false, m_bus_reset_4002 = false, m_bus_user_reset = false;

	// current state of mode switches
	bool    m_sw_stop = false;
	bool    m_sw_reset = false;
	bool    m_sw_mon = false, m_sw_ram = false, m_sw_prom = false;
};


READ8_MEMBER(intellec4_40_state::pm_read)
{
	if (!machine().side_effect_disabled())
	{
		// always causes data to be latched
		u16 const addr((u16(m_ram_page) << 8) | ((offset >> 1) & 0x00ffU));
		m_ram_data = m_ram[addr];
	}

	// the C outputs of the 4289 are always high for RPM/WPM so it's equivalent to romf_in
	return m_ram_data & 0x0fU;
}

WRITE8_MEMBER(intellec4_40_state::pm_write)
{
	// always causes data to be latched
	u16 const addr((u16(m_ram_page) << 8) | ((offset >> 1) & 0x00ffU));
	m_ram_data = m_ram[addr];
	if (m_ram_write)
	{
		bool const first(BIT(offset, 0));
		m_ram[addr] = (m_ram_data & (first ? 0x0fU : 0xf0U)) | ((data & 0x0fU) << (first ? 4 : 0));
	}
}

READ8_MEMBER(intellec4_40_state::rom0_in)
{
	// bit 0 of this port is ANDed with the TTY input
	return m_tty->rxd_r() ? 0x00U : 0x01U;
}

READ8_MEMBER(intellec4_40_state::rome_in)
{
	// upper nybble of RAM data latch
	return (m_ram_data >> 4) & 0x0fU;
}

READ8_MEMBER(intellec4_40_state::romf_in)
{
	// lower nybble of RAM data latch
	return m_ram_data & 0x0fU;
}

WRITE8_MEMBER(intellec4_40_state::rome_out)
{
	// bit 0 of this port enables program memory write
	m_ram_write = BIT(data, 0);
}

WRITE8_MEMBER(intellec4_40_state::romf_out)
{
	// sets the program memory page for read/write operations
	m_ram_page = data & 0x0fU;
}

WRITE8_MEMBER(intellec4_40_state::ram0_out)
{
	// bit 0 of this port controls the TTY current loop
	m_tty->write_txd(BIT(data, 0));
}

WRITE8_MEMBER(intellec4_40_state::ram1_out)
{
	// bit 0 of this port controls the paper tape motor (0 = stop, 1 = run)
	m_tty->write_rts(BIT(~data, 0));
}

WRITE_LINE_MEMBER(intellec4_40_state::stp_ack)
{
	// resets the single-step monostable
	if (m_stp_ack && state)
	{
		m_single_step_timer->reset();
		m_single_step = false;
		if (m_sw_stop)
		{
			m_bus->stop_in(0);
			if (!m_bus_stop)
				m_cpu->set_input_line(I4040_STP_LINE, ASSERT_LINE);
		}
	}
	m_stp_ack = 0 == state;
	machine().output().set_value("led_status_run",  !m_stp_ack);
	m_bus->stop_acknowledge_in(state);
}


WRITE_LINE_MEMBER(intellec4_40_state::bus_stop)
{
	// will not allow the CPU to step/run
	if (m_single_step || !m_sw_stop)
		m_cpu->set_input_line(I4040_STP_LINE, state ? CLEAR_LINE : ASSERT_LINE);
	m_bus_stop = 0 == state;
}

WRITE_LINE_MEMBER(intellec4_40_state::bus_test)
{
	m_cpu->set_input_line(I4040_TEST_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

WRITE_LINE_MEMBER(intellec4_40_state::bus_reset_4002)
{
	// FIXME: this will clear 4002 RAMs (data space)
}

WRITE_LINE_MEMBER(intellec4_40_state::bus_user_reset)
{
	if (!state)
		trigger_reset();
	m_bus_user_reset = 0 == state;
}


INPUT_CHANGED_MEMBER(intellec4_40_state::sw_stop)
{
	// overridden by the single-step monostable and the bus stop signal
	if (!m_single_step && !m_bus_stop)
	{
		m_cpu->set_input_line(I4040_STP_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
		m_bus->stop_in(newval ? 1 : 0);
	}
	m_sw_stop = !bool(newval);
}

INPUT_CHANGED_MEMBER(intellec4_40_state::sw_single_step)
{
	// (re-)triggers the single-step monostable
	if (m_stp_ack && newval && !oldval)
	{
		// 9602 with Rx = 20kΩ, Cx = 0.005µF
		// K * Rx(kΩ) * Cx(pF) * (1 + (1 / Rx(kΩ))) = 0.34 * 20 * 5000 * (1 + (1 / 20)) = 35700ns
		m_single_step_timer->adjust(attotime::from_nsec(35700));
		m_single_step = true;
		if (m_sw_stop)
		{
			m_bus->stop_in(1);
			if (!m_bus_stop)
				m_cpu->set_input_line(I4040_STP_LINE, CLEAR_LINE);
		}
	}
}

INPUT_CHANGED_MEMBER(intellec4_40_state::sw_reset)
{
	if (!newval && oldval)
		trigger_reset();
	m_sw_reset = !bool(newval);
}

INPUT_CHANGED_MEMBER(intellec4_40_state::sw_mon)
{
	if (oldval && !newval)
	{
		if (!m_sw_ram && !m_sw_prom)
		{
			if (!m_ff_mon)
			{
				m_program_banks->set_bank(BANK_PRG_MON);
				m_io_banks->set_bank(BANK_IO_MON);
				machine().output().set_value("led_mode_mon", m_ff_mon = true);
			}
			trigger_reset();
		}
		else
		{
			m_program_banks->set_bank(BANK_PRG_NONE);
			m_io_banks->set_bank(BANK_IO_NEITHER);
		}
		if (m_ff_ram)
			machine().output().set_value("led_mode_ram", m_ff_ram = false);
		if (m_ff_prom)
			machine().output().set_value("led_mode_prom", m_ff_prom = false);
	}
	m_sw_mon = !bool(newval);
}

INPUT_CHANGED_MEMBER(intellec4_40_state::sw_ram)
{
	if (oldval && !newval)
	{
		if (!m_sw_mon && !m_sw_prom)
		{
			if (!m_ff_ram)
			{
				m_program_banks->set_bank(BANK_PRG_RAM);
				m_io_banks->set_bank(BANK_IO_NEITHER);
				machine().output().set_value("led_mode_ram", m_ff_ram = true);
			}
			trigger_reset();
		}
		else
		{
			m_program_banks->set_bank(BANK_PRG_NONE);
			m_io_banks->set_bank(BANK_IO_NEITHER);
		}
		if (m_ff_mon)
			machine().output().set_value("led_mode_mon", m_ff_mon = false);
		if (m_ff_prom)
			machine().output().set_value("led_mode_prom", m_ff_prom = false);
	}
	m_sw_ram = !bool(newval);
}

INPUT_CHANGED_MEMBER(intellec4_40_state::sw_prom)
{
	if (oldval && !newval)
	{
		if (!m_sw_mon && !m_sw_ram)
		{
			if (!m_ff_prom)
			{
				m_program_banks->set_bank(BANK_PRG_PROM);
				m_io_banks->set_bank(BANK_IO_PROM);
				machine().output().set_value("led_mode_prom", m_ff_prom = true);
			}
			trigger_reset();
		}
		else
		{
			m_program_banks->set_bank(BANK_PRG_NONE);
			m_io_banks->set_bank(BANK_IO_NEITHER);
		}
		if (m_ff_mon)
			machine().output().set_value("led_mode_mon", m_ff_mon = false);
		if (m_ff_ram)
			machine().output().set_value("led_mode_ram", m_ff_ram = false);
	}
	m_sw_prom = !bool(newval);
}


void intellec4_40_state::driver_start()
{
	m_single_step_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(intellec4_40_state::single_step_expired), this));
	m_reset_timer       = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(intellec4_40_state::reset_expired), this));

	save_item(NAME(m_ram_page));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_ram_write));

	save_item(NAME(m_stp_ack));
	save_item(NAME(m_single_step));
	save_item(NAME(m_ff_mon));
	save_item(NAME(m_ff_ram));
	save_item(NAME(m_ff_prom));

	save_item(NAME(m_bus_stop));
	save_item(NAME(m_bus_reset_4002));
	save_item(NAME(m_bus_user_reset));

	save_item(NAME(m_sw_stop));
	save_item(NAME(m_sw_reset));
	save_item(NAME(m_sw_mon));
	save_item(NAME(m_sw_ram));

	m_stp_ack = m_single_step = false;
	m_ff_mon = true;
	m_ff_ram = m_ff_prom = false;
}

void intellec4_40_state::driver_reset()
{
	// set stuff according to initial state of front panel
	ioport_value const sw_mode(m_sw_mode->read());
	m_sw_stop   = BIT(~sw_mode, BIT_SW_STOP);
	m_sw_reset  = BIT(~sw_mode, BIT_SW_RESET);
	m_sw_mon    = BIT(~sw_mode, BIT_SW_MON);
	m_sw_ram    = BIT(~sw_mode, BIT_SW_RAM);
	m_sw_prom   = BIT(~sw_mode, BIT_SW_PROM);
	m_ff_mon = m_ff_mon && !m_sw_ram && !m_sw_prom;
	m_ff_ram = m_ff_ram && !m_sw_mon && !m_sw_prom;
	m_ff_prom = m_ff_prom && !m_sw_mon && !m_sw_ram;

	// ensure we're consistent with the state of the bus
	m_bus_stop          = 0 == m_bus->stop_out();
	m_bus_reset_4002    = 0 == m_bus->reset_4002_out();
	m_bus_user_reset    = 0 == m_bus->user_reset_out();

	// ensure device inputs are all in the correct state
	m_cpu->set_input_line(I4040_TEST_LINE, m_bus->test_out() ? CLEAR_LINE : ASSERT_LINE);
	m_cpu->set_input_line(I4040_STP_LINE, ((m_sw_stop && !m_single_step) || m_bus_stop) ? ASSERT_LINE : CLEAR_LINE);
	m_bus->test_in(1);
	m_bus->stop_in((m_sw_stop && !m_single_step) ? 0 : 1);

	// set front panel LEDs
	machine().output().set_value("led_status_run",  !m_stp_ack);
	machine().output().set_value("led_mode_mon",     m_ff_mon);
	machine().output().set_value("led_mode_ram",     m_ff_ram);
	machine().output().set_value("led_mode_prom",    m_ff_prom);
}


TIMER_CALLBACK_MEMBER(intellec4_40_state::single_step_expired)
{
	m_single_step = false;
	if (m_sw_stop)
	{
		m_bus->stop_in(0);
		if (!m_bus_stop)
			m_cpu->set_input_line(I4040_STP_LINE, ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(intellec4_40_state::reset_expired)
{
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	m_bus->cpu_reset_in(1);
	// FIXME: can cause 4002 reset as well
}

void intellec4_40_state::trigger_reset()
{
	// (re-)trigger the reset monostable
	if (!m_sw_reset && !m_sw_mon && !m_sw_ram && !m_sw_prom && !m_bus_user_reset)
	{
		// 9602 with Rx = 27kΩ, Cx = 0.1µF
		// K * Rx(kΩ) * Cx(pF) * (1 + (1 / Rx(kΩ))) = 0.34 * 27 * 100000 * (1 + (1 / 27)) = 952000ns
		m_reset_timer->adjust(attotime::from_usec(952));
		m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_bus->cpu_reset_in(0);
		// FIXME: can cause 4002 reset as well
	}
}


ADDRESS_MAP_START(mod40_program_banks, AS_OPCODES, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW

	// 0x0000...0x0fff MON
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("monitor", 0x0000)

	// 0x1000...0x1fff RAM
	AM_RANGE(0x1000, 0x1fff) AM_READONLY AM_SHARE("ram")

	// 0x2000...0x2fff PROM

	// 0x3000...0x3fff unmapped in case someone presses two mode switches at once
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_io_banks, AS_PROGRAM, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW

	// 0x0000...0x0fff ROM ports - MON
	AM_RANGE(0x0000, 0x000f) AM_MIRROR(0x6700) AM_READ(rom0_in)
	AM_RANGE(0x00e0, 0x00ef) AM_MIRROR(0x6700) AM_READWRITE(rome_in, rome_out)
	AM_RANGE(0x00f0, 0x00ff) AM_MIRROR(0x6700) AM_READWRITE(romf_in, romf_out)

	// 0x1000...0x1fff RAM ports - MON
	AM_RANGE(0x1000, 0x103f) AM_MIRROR(0x6800) AM_WRITE(ram0_out)
	AM_RANGE(0x1040, 0x107f) AM_MIRROR(0x6800) AM_WRITE(ram1_out)

	// 0x2000...0x2fff ROM ports - neither

	// 0x3000...0x3fff RAM ports - neither

	// 0x4000...0x4fff ROM ports - PROM

	// 0x5000...0x5fff RAM ports - PROM

	// 0x6000...0x7fff unused
ADDRESS_MAP_END


ADDRESS_MAP_START(mod40_program, AS_PROGRAM, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(pm_read, pm_write)
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_data, AS_DATA, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x00ff) AM_RAM // 4 * 4002
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_io, AS_IO, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x1fff) AM_DEVICE("iobank", address_map_bank_device, amap8)
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_opcodes, AS_OPCODES, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x0fff) AM_DEVICE("prgbank", address_map_bank_device, amap8)
ADDRESS_MAP_END


MACHINE_CONFIG_START(intlc440)
	MCFG_CPU_ADD("maincpu", I4040, 5185000. / 7)
	MCFG_CPU_PROGRAM_MAP(mod40_program)
	MCFG_CPU_DATA_MAP(mod40_data)
	MCFG_CPU_IO_MAP(mod40_io)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(mod40_opcodes)
	MCFG_I4040_SYNC_CB(DEVWRITELINE("bus", bus::intellec4::univ_bus_device, sync_in))
	MCFG_I4040_STP_ACK_CB(WRITELINE(intellec4_40_state, stp_ack))

	MCFG_DEVICE_ADD("prgbank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(mod40_program_banks)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(14)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)

	MCFG_DEVICE_ADD("iobank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(mod40_io_banks)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(15)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_RS232_PORT_ADD("tty", default_rs232_devices, "terminal")

	MCFG_DEVICE_ADD("bus", INTELLEC4_UNIV_BUS, 518000. / 7)
	MCFG_INTELLEC4_UNIV_BUS_STOP_CB(WRITELINE(intellec4_40_state, bus_stop))
	MCFG_INTELLEC4_UNIV_BUS_TEST_CB(WRITELINE(intellec4_40_state, bus_test))
	MCFG_INTELLEC4_UNIV_BUS_RESET_4002_CB(WRITELINE(intellec4_40_state, bus_reset_4002))
	MCFG_INTELLEC4_UNIV_BUS_USER_RESET_CB(WRITELINE(intellec4_40_state, bus_user_reset))
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j7",  518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j8",  518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j9",  518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j10", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j11", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j12", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j13", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j14", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j15", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j16", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j17", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j18", 518000. / 7, intellec4_univ_cards, nullptr)
	MCFG_INTELLEC4_UNIV_SLOT_ADD("bus", "j19", 518000. / 7, intellec4_univ_cards, nullptr)
MACHINE_CONFIG_END


INPUT_PORTS_START(intlc440)
	PORT_START("MODE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("STOP")        PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_stop,        0)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 )             PORT_NAME("SINGLE STEP") PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_single_step, 0)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON3 )             PORT_NAME("RESET")       PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_reset,       0)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4 )             PORT_NAME("MON")         PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_mon,         0)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON5 )             PORT_NAME("RAM")         PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_ram,         0)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON6 )             PORT_NAME("PROM")        PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, sw_prom,        0)
INPUT_PORTS_END


ROM_START(intlc440)
	ROM_REGION(0x0400, "monitor", 0) // 4 * 1702A
	ROM_DEFAULT_BIOS("v2.1")
	ROM_SYSTEM_BIOS(0, "v2.1", "MON 4 V2.1")
	ROMX_LOAD("mon_4-000-v_2.1.a1",      0x0000, 0x0100, CRC(8d1f56ff) SHA1(96bc19be9be4e92195fad82d7a3cadb763ab6e3f), ROM_BIOS(1))
	ROMX_LOAD("mon_4-100-v_2.1.a2",      0x0100, 0x0100, CRC(66562a4f) SHA1(040749c45e95dfc39b3397d0c31c8b4c11f0a5fc), ROM_BIOS(1))
	ROMX_LOAD("mon_4-200-v_2.1.a3",      0x0200, 0x0100, CRC(fe039c68) SHA1(1801cfcc7514412865c0fdc7d1800fcf583a2d2a), ROM_BIOS(1))
	ROMX_LOAD("mon_4-300-v_2.1.a4",      0x0300, 0x0100, CRC(3724d5af) SHA1(b764b3bb3541fbda875f7a7655f46aa54b332631), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2.1_1200", "MON 4 V2.1 1200 Baud hack")
	ROMX_LOAD("mon_4-000-v_2.1.a1",      0x0000, 0x0100, CRC(8d1f56ff) SHA1(96bc19be9be4e92195fad82d7a3cadb763ab6e3f), ROM_BIOS(2))
	ROMX_LOAD("i40_mon-1.a2",            0x0100, 0x0100, CRC(cd9fecd6) SHA1(9c4fb85118c881687fd4b324e5089df05d1e63d1), ROM_BIOS(2))
	ROMX_LOAD("i40_mon-2.a3",            0x0200, 0x0100, CRC(037de128) SHA1(3694636e1f4e23688b36ea9ee755a0c5888f4328), ROM_BIOS(2))
	ROMX_LOAD("1200_baud-i40_mon-f3.a4", 0x0300, 0x0100, CRC(f3198d79) SHA1(b7903073b69f487b6f78842c08694f12225d85f0), ROM_BIOS(2))
ROM_END

} // anonymous namespace

//    YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     STATE               INIT  COMPANY  FULLNAME            FLAGS
COMP( 1974?, intlc440, 0,      0,      intlc440, intlc440, intellec4_40_state, 0,    "Intel", "INTELLEC 4/MOD40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
