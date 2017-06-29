// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Intel INTELLEC® 4/MOD 40

 Set the terminal for 110 1/8/N/2 to talk to the monitor.
 It only likes to see uppercase letters, digits, comman and carriage return.
 */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs40/mcs40.h"


namespace {

class intellec4_40_state : public driver_device
{
public:
	intellec4_40_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "maincpu")
		, m_tty(*this, "tty")
		, m_mon_rom(*this, "monitor", 0x0400)
		, m_ram(*this, "ram")
		, m_banks(*this, "bank%u", 0)
		, m_mode_sw(*this, "MODE")
		, m_single_step_timer(nullptr)
		, m_stp_ack(false), m_single_step(false)
		, m_ram_page(0U), m_ram_data(0U), m_ram_write(false)
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

	DECLARE_INPUT_CHANGED_MEMBER(stop_sw);
	DECLARE_INPUT_CHANGED_MEMBER(single_step_sw);

protected:
	virtual void driver_start() override;

private:
	TIMER_CALLBACK_MEMBER(single_step_expired);

	bool get_stop_sw() { return BIT(~m_mode_sw->read(), 0); }

	required_device<cpu_device>         m_cpu;
	required_device<rs232_port_device>  m_tty;
	required_region_ptr<u8>             m_mon_rom;
	required_shared_ptr<u8>             m_ram;
	required_memory_bank_array<4>       m_banks;

	required_ioport                     m_mode_sw;

	emu_timer                           *m_single_step_timer;

	bool    m_stp_ack, m_single_step;
	u8      m_ram_page, m_ram_data;
	bool    m_ram_write;
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
		m_cpu->set_input_line(I4040_STP_LINE, get_stop_sw() ? ASSERT_LINE : CLEAR_LINE);
	}
	m_stp_ack = !state;
}


INPUT_CHANGED_MEMBER(intellec4_40_state::stop_sw)
{
	m_cpu->set_input_line(I4040_STP_LINE, (newval || m_single_step) ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(intellec4_40_state::single_step_sw)
{
	// (re-)triggers the single-step monostable
	if (newval && !oldval)
	{
		// 9602 with Rx = 20kΩ, Cx = 0.005µF
		// K * Rx(kΩ) * (1 + (1 / Rx(kΩ))) = 0.34 * 20 * 5000 * (1 + (1 / 20)) = 35700ns
		m_single_step_timer->adjust(attotime::from_nsec(35700));
		m_single_step = true;
		m_cpu->set_input_line(I4040_STP_LINE, CLEAR_LINE);
	}
}


void intellec4_40_state::driver_start()
{
	for (unsigned i = 0; m_banks.size() > i; ++i)
	{
		m_banks[i]->configure_entry(0, &m_mon_rom[i << 8]);
		m_banks[i]->configure_entry(1, &m_ram[i << 8]);
		m_banks[i]->set_entry(0);
	}

	m_single_step_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(intellec4_40_state::single_step_expired), this));

	save_item(NAME(m_stp_ack));
	save_item(NAME(m_single_step));
	save_item(NAME(m_ram_page));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_ram_write));
}


TIMER_CALLBACK_MEMBER(intellec4_40_state::single_step_expired)
{
	m_single_step = false;
	m_cpu->set_input_line(I4040_STP_LINE, get_stop_sw() ? ASSERT_LINE : CLEAR_LINE);
}


ADDRESS_MAP_START(mod40_program, AS_PROGRAM, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x01ff) AM_READWRITE(pm_read, pm_write)
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_data, AS_DATA, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_RAM // 4 * 4002
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_io, AS_IO, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_MIRROR(0x0700) AM_READ(rom0_in)
	AM_RANGE(0x00e0, 0x00ef) AM_MIRROR(0x0700) AM_READWRITE(rome_in, rome_out)
	AM_RANGE(0x00f0, 0x00ff) AM_MIRROR(0x0700) AM_READWRITE(romf_in, romf_out)
	AM_RANGE(0x1000, 0x103f) AM_MIRROR(0x0800) AM_WRITE(ram0_out)
	AM_RANGE(0x1040, 0x107f) AM_MIRROR(0x0800) AM_WRITE(ram1_out)
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_opcodes, AS_DECRYPTED_OPCODES, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_READ_BANK("bank0")
	AM_RANGE(0x0100, 0x01ff) AM_READ_BANK("bank1")
	AM_RANGE(0x0200, 0x02ff) AM_READ_BANK("bank2")
	AM_RANGE(0x0300, 0x03ff) AM_READ_BANK("bank3")
	AM_RANGE(0x0000, 0x0fff) AM_READONLY AM_SHARE("ram")
ADDRESS_MAP_END


MACHINE_CONFIG_START(intlc440)
	MCFG_CPU_ADD("maincpu", I4040, 5185000./7)
	MCFG_CPU_PROGRAM_MAP(mod40_program)
	MCFG_CPU_DATA_MAP(mod40_data)
	MCFG_CPU_IO_MAP(mod40_io)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(mod40_opcodes)
	MCFG_I4040_STP_ACK_CB(WRITELINE(intellec4_40_state, stp_ack))

	MCFG_RS232_PORT_ADD("tty", default_rs232_devices, "terminal")
MACHINE_CONFIG_END


INPUT_PORTS_START(intlc440)
	PORT_START("MODE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_TOGGLE PORT_NAME("STOP")        PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, stop_sw,        0)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 )             PORT_NAME("SINGLE STEP") PORT_CHANGED_MEMBER(DEVICE_SELF, intellec4_40_state, single_step_sw, 0)
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
