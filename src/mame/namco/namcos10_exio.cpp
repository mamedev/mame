// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "namcos10_exio.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NAMCOS10_EXIO,      namcos10_exio_device,      "namcos10_exio",   "Namco System 10 EXIO")
DEFINE_DEVICE_TYPE(NAMCOS10_EXIO_BASE, namcos10_exio_base_device, "namcos10_exio_g", "Namco System 10 EXIO(G)")
DEFINE_DEVICE_TYPE(NAMCOS10_MGEXIO,    namcos10_mgexio_device,    "namcos10_mgexio", "Namco System 10 MGEXIO")

// EXIO(G) has the bare minimum: CPLD, audio output jacks, gun I/O, and a card edge connector for additional I/O
namcos10_exio_base_device::namcos10_exio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ident_code) :
	device_t(mconfig, type, tag, owner, clock), m_ident_code(ident_code)
{
}

namcos10_exio_base_device::namcos10_exio_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	namcos10_exio_base_device(mconfig, NAMCOS10_EXIO_BASE, tag, owner, clock, 0x32)
{
}

void namcos10_exio_base_device::device_start()
{
}

////////////////////////////////////

namcos10_exio_device::namcos10_exio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	namcos10_exio_base_device(mconfig, NAMCOS10_EXIO, tag, owner, clock, 0x30),
	m_maincpu(*this, "exio_mcu"),
	m_ram(*this, "exio_ram"),
	m_analog_cb(*this, 0)
{
}

void namcos10_exio_device::device_start()
{
	namcos10_exio_base_device::device_start();

	save_item(NAME(m_is_active));
	save_item(NAME(m_analog_idx));
	save_item(NAME(m_bus_req));
	save_item(NAME(m_ctrl));
}

void namcos10_exio_device::device_reset_after_children()
{
	namcos10_exio_base_device::device_reset_after_children();

	m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	m_is_active = false;
	m_analog_idx = 0;
	m_bus_req = 0;
	m_ctrl = 0;
}

void namcos10_exio_device::map(address_map &map)
{
	map(0x000100, 0x002fff).ram(); // TODO: Stack and such is stored here, how large should this really be?
	map(0x003000, 0x007fff).ram().mirror(0xff8000).share(m_ram);
}

void namcos10_exio_device::device_add_mconfig(machine_config &config)
{
	// TODO: tmp95c061 doesn't have a serial implementation yet so JVS communication won't work for now
	TMP95C061(config, m_maincpu, XTAL(22'118'400));
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_exio_device::map);

	m_maincpu->port1_read().set(FUNC(namcos10_exio_device::port_read<1>));
	m_maincpu->port5_read().set(FUNC(namcos10_exio_device::port_read<5>));
	m_maincpu->port7_read().set(FUNC(namcos10_exio_device::port_read<7>));
	m_maincpu->port8_read().set(FUNC(namcos10_exio_device::port_read<8>));
	m_maincpu->port9_read().set(FUNC(namcos10_exio_device::port_read<9>));
	m_maincpu->porta_read().set(FUNC(namcos10_exio_device::port_read<10>));
	m_maincpu->portb_read().set(FUNC(namcos10_exio_device::port_read<11>));
	m_maincpu->port1_write().set(FUNC(namcos10_exio_device::port_write<1>));
	m_maincpu->port2_write().set(FUNC(namcos10_exio_device::port_write<2>));
	m_maincpu->port5_write().set(FUNC(namcos10_exio_device::port_write<5>));
	m_maincpu->port6_write().set(FUNC(namcos10_exio_device::port_write<6>));
	m_maincpu->port8_write().set(FUNC(namcos10_exio_device::port_write<8>));
	m_maincpu->porta_write().set(FUNC(namcos10_exio_device::port_write<10>));
	m_maincpu->portb_write().set(FUNC(namcos10_exio_device::port_write<11>));

	m_maincpu->port7_write().set([this] (uint8_t data) {
		// The common EXIO program uploaded seems to write what analog value it wants to read here.
		// Going to the CPLD?
		m_analog_idx = data;
	});
	m_maincpu->an_read<0>().set([this] () {
		return m_analog_cb((m_analog_idx & 3) * 2);
	});
	m_maincpu->an_read<1>().set([this] () {
		return m_analog_cb((m_analog_idx & 3) * 2 + 1);
	});
}

uint16_t namcos10_exio_device::cpu_status_r()
{
	uint16_t r = m_is_active ? 1 : 0;
	return (r << 8) | r;
}

uint16_t namcos10_exio_device::ctrl_r()
{
	return m_ctrl;
}

void namcos10_exio_device::ctrl_w(uint16_t data)
{
	logerror("%s: exio_ctrl_w %04x %04x\n", machine().describe_context(), data, m_ctrl ^ data);

	if ((data & 1) && !(m_ctrl & 1)) {
		m_maincpu->reset();
	} else if (!(data & 1) && (m_ctrl & 1)) {
		m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	}

	m_ctrl = data;
}

uint16_t namcos10_exio_device::bus_req_r()
{
	return m_bus_req == 1 ? 2 : 0;
}

void namcos10_exio_device::bus_req_w(uint16_t data)
{
	m_bus_req = data;
}

void namcos10_exio_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 0))
		m_ram[offset / 2] = ((data & 0xff) << 8) | (m_ram[offset / 2] & 0xff);
	else
		m_ram[offset / 2] = (m_ram[offset / 2] & 0xff00) | (data & 0xff);
}

uint16_t namcos10_exio_device::ram_r(offs_t offset)
{
	if (BIT(offset, 0))
		return BIT(m_ram[offset / 2], 8, 8);

	return BIT(m_ram[offset / 2], 0, 8);
}

template <int Port>
void namcos10_exio_device::port_write(offs_t offset, uint8_t data)
{
	// logerror("%s: exio_port%d_write %02x\n", machine().describe_context(), Port, data);

	if (Port == 8) {
		// HACK: Simple check to just know when the CPU is alive
		m_is_active |= data != 0;
	}
}

template <int Port>
uint8_t namcos10_exio_device::port_read(offs_t offset)
{
	// logerror("%s: exio_port%d_read\n", machine().describe_context(), Port);
	return 0;
}

////////////////////////////////////

namcos10_mgexio_device::namcos10_mgexio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	namcos10_exio_base_device(mconfig, NAMCOS10_MGEXIO, tag, owner, clock, 0x33),
	m_maincpu(*this, "exio_mcu"),
	m_ram(*this, "exio_ram"),
	m_nvram(*this, "nvram"),
	m_port_read(*this, 0),
	m_port_write(*this)
{
}

void namcos10_mgexio_device::device_start()
{
	save_item(NAME(m_is_active));
	save_item(NAME(m_bus_req));
	save_item(NAME(m_ctrl));

	m_cpu_reset_timer = timer_alloc(FUNC(namcos10_mgexio_device::cpu_reset_timeout), this);

	m_nvram->set_base(m_ram, 0x8000);
}

void namcos10_mgexio_device::device_reset_after_children()
{
	namcos10_exio_base_device::device_reset_after_children();

	m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	m_is_active = false;
	m_bus_req = 0;
	m_ctrl = 0;
}

TIMER_CALLBACK_MEMBER(namcos10_mgexio_device::cpu_reset_timeout)
{
	m_maincpu->reset();
	m_is_active = true;
}

void namcos10_mgexio_device::map(address_map &map)
{
	map(0x00000, 0x7ffff).ram().share(m_ram);
}

template <int Port>
uint8_t namcos10_mgexio_device::port_r()
{
	return m_port_read[Port](0);
}

template <int Port>
void namcos10_mgexio_device::port_w(uint8_t data)
{
	m_port_write[Port](data);
}

void namcos10_mgexio_device::device_add_mconfig(machine_config &config)
{
	H83007(config, m_maincpu, 14.746_MHz_XTAL);
	m_maincpu->set_mode_a20();
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_mgexio_device::map);
	m_maincpu->read_port4().set(FUNC(namcos10_mgexio_device::port_r<0>));
	m_maincpu->write_port4().set(FUNC(namcos10_mgexio_device::port_w<0>));
	m_maincpu->read_port6().set(FUNC(namcos10_mgexio_device::port_r<1>));
	m_maincpu->write_port6().set(FUNC(namcos10_mgexio_device::port_w<1>));
	m_maincpu->read_port7().set(FUNC(namcos10_mgexio_device::port_r<2>));
	m_maincpu->read_port8().set(FUNC(namcos10_mgexio_device::port_r<3>));
	m_maincpu->write_port8().set(FUNC(namcos10_mgexio_device::port_w<3>));
	m_maincpu->read_port9().set(FUNC(namcos10_mgexio_device::port_r<4>));
	m_maincpu->write_port9().set(FUNC(namcos10_mgexio_device::port_w<4>));
	m_maincpu->read_porta().set(FUNC(namcos10_mgexio_device::port_r<5>));
	m_maincpu->write_porta().set(FUNC(namcos10_mgexio_device::port_w<5>));
	m_maincpu->read_portb().set(FUNC(namcos10_mgexio_device::port_r<6>));
	m_maincpu->write_portb().set(FUNC(namcos10_mgexio_device::port_w<6>));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

uint16_t namcos10_mgexio_device::cpu_status_r()
{
	// pacmball's code call bit 1 the "sub_cpu_enable_flag"
	return m_is_active ? 2 : 0;
}

uint16_t namcos10_mgexio_device::ctrl_r()
{
	// bit 0 being 1 makes some games check for sensor responses on the H8's ports (sekaikh, ballpom)
	// pacmball doesn't seem to care about bit 0
	// Possibly those games only want to check I/O when the CPU is known to be active?
	return m_ctrl;
}

void namcos10_mgexio_device::ctrl_w(uint16_t data)
{
	logerror("%s: exio_ctrl_w %04x %04x\n", machine().describe_context(), data, m_ctrl ^ data);

	// sekaikh and ballpom only write 3 here and that's all
	// pacmball will write 3 at the start when the CPU is to be started
	// and then flip bit 1 at the end of the main loop so it might be I/O related
	if ((data & 1) && !(m_ctrl & 1)) {
		// Timed such that there's enough delay before starting but also
		// so it doesn't wait too long into the timeout before starting.
		// So far only pacmball relies on timings to be correct to boot.
		//
		// TODO: Should this really be restarted every time or just the first time?
		// Even pacmball only wants to see the timings correct the first time
		// so it might actually just use standby mode after the initial boot.
		m_cpu_reset_timer->adjust(attotime::from_msec(40));
	} else if (!(data & 1) && (m_ctrl & 1)) {
		// Stop the CPU when bit 0 is flipped from 1 to 0
		// This happens when a sub CPU-related error occurs
		m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	}

	m_ctrl = data;
}

uint16_t namcos10_mgexio_device::bus_req_r()
{
	// Should have bit 1 set when the CPU is requested
	return m_bus_req == 1 ? 2 : 0;
}

void namcos10_mgexio_device::bus_req_w(uint16_t data)
{
	// Usage before using RAM:
	// Write 0 to bus_req_w
	// Check if unk_r has bit 0 set
	//   If set, loop until bus_req_r bit 1 is 0
	//     If bus_req_r bit 1 does not return 0 within 500 polls, "BREQ time out" and "BUS-ACK" is raised
	// (do RAM things)
	// Write 1 to bus_req_w
	//
	// bus_req_r bit 1 must be 1 for the CPU check

	// logerror("%s: bus_req_w %04x\n", machine().describe_context(), data);
	m_bus_req = data;
}

void namcos10_mgexio_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 0) == 1)
		m_ram[offset / 2] = (m_ram[offset / 2] & 0xff00) | data;
	else
		m_ram[offset / 2] = (data << 8) | (m_ram[offset / 2] & 0xff);
}

uint16_t namcos10_mgexio_device::ram_r(offs_t offset)
{
	if (BIT(offset, 0) == 1)
		return m_ram[offset / 2] & 0xff;
	else
		return (m_ram[offset / 2] >> 8) & 0xff;
}
