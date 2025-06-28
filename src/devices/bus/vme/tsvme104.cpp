// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tekmis TSVME104/105
 *
 * This board is one of several from a PRISME MS2i computer, which was used to
 * acquire data from an electron microscope, and interface it to a Pericolor
 * image processing unit. The entire system was marketed by EDAX.
 *
 * The case has the following ports labeled on the rear:
 *
 *  P1 Console - serial
 *  P2 PERICOLOR - serial
 *  P3 Adapt. 1/0
 *  P4 Analog. 1/0
 *  P5 Binary channels        (not connected)
 *  P6 Motor stage            (not connected)
 *  P7 EDS                    (not connected)
 *  P8 SEM - serial           (not connected)
 *  P9 PERICOLOR - parallel
 * P10 Extens. - parallel
 * P11                        (not connected)
 * P12                        (not connected)
 *
 * Sources:
 *  - none
 *
 * TODO:
 *  - skeleton only
 *
 */

#include "emu.h"
#include "tsvme104.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_TSVME104, vme_tsvme104_card_device, "tsvme104", "Tekmis TSVME104")

vme_tsvme104_card_device::vme_tsvme104_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_TSVME104, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_ptm(*this, "ptm")
	, m_duart(*this, "duart%u", 0U)
	, m_serial(*this, "serial%u", 0U)
	, m_boot(*this, "boot")
{
}

ROM_START(tsvme104)
	ROM_REGION16_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "109", "109: PRISME VERSION NUMBER")
	ROMX_LOAD("abx_0.ic51", 0x0000, 0x10000, CRC(b09f887e) SHA1(901d2e59d4b3c825f15e832b20cc7492ab8687e6), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("abx_1.ic49", 0x0001, 0x10000, CRC(2603b5c2) SHA1(3b9834731f2b1a47f8c840c5e47d9fc4a8769d42), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(tsvme104)
INPUT_PORTS_END

const tiny_rom_entry *vme_tsvme104_card_device::device_rom_region() const
{
	return ROM_NAME(tsvme104);
}

ioport_constructor vme_tsvme104_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tsvme104);
}

void vme_tsvme104_card_device::device_start()
{
}

void vme_tsvme104_card_device::device_reset()
{
	m_boot.select(0);

	m_boot_tap = m_cpu->space(AS_PROGRAM).install_write_tap(0x00'0000, 0x1f'ffff, "boot_w",
		[this](offs_t offset, u16 &data, u16 mem_mask)
		{
			m_boot.disable();
			m_boot_tap.remove();
		}, &m_boot_tap);
}

void vme_tsvme104_card_device::device_add_mconfig(machine_config &config)
{
	// TODO: board has 25MHz and 16MHz crystals

	M68010(config, m_cpu, 25_MHz_XTAL / 2); // MC68010R12
	m_cpu->set_addrmap(AS_PROGRAM, &vme_tsvme104_card_device::cpu_mem);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_tsvme104_card_device::cpu_int);

	PTM6840(config, m_ptm, 0); // ST EF68B40P

	MC68681(config, m_duart[0], 3.6864_MHz_XTAL);
	m_duart[0]->irq_cb().set_inputline(m_cpu, INPUT_LINE_IRQ3);

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	m_duart[0]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	MC68681(config, m_duart[1], 3.6864_MHz_XTAL);
	m_duart[1]->irq_cb().set_inputline(m_cpu, INPUT_LINE_IRQ4);

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);

	m_duart[1]->a_tx_cb().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_serial[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_serial[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));

	// TODO: SCB68155CAN40 asynchronous interrupt handler for VMEbus
	// TODO: Tomson Semiconducteurs TSVME 180: NCR/5385E + NCR 8310 SCSI daughter-board
	// TODO: reset/abort push buttons
	// TODO: fail/halt/busy led's
}

void vme_tsvme104_card_device::cpu_mem(address_map &map)
{
	map(0x00'0000, 0x07'ffff).ram();

	map(0x00'0000, 0x01'ffff).view(m_boot);
	m_boot[0](0x00'0000, 0x01'ffff).rom().region("eprom", 0);

	//map(0x70'8000, 0x70'81ff).ram(); // 16bit r/w - host interface?

	map(0xf8'0000, 0xf9'ffff).rom().region("eprom", 0);

	map(0xfe'0200, 0xfe'020f).umask16(0x00ff).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xfe'0400, 0xfe'041f).umask16(0x00ff).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xfe'0600, 0xfe'061f).umask16(0x00ff).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write));

	//map(0xfe'0800, 0xfe'08ff); // 8bit device?
	//map(0xfe'0e00, 0xfe'0eff); // 8bit device?
}

void vme_tsvme104_card_device::cpu_int(address_map &map)
{
	map(0xff'fff3, 0xff'fff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xff'fff5, 0xff'fff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xff'fff7, 0xff'fff7).lr8(NAME([this]() { return m_duart[0]->get_irq_vector(); }));
	map(0xff'fff9, 0xff'fff9).lr8(NAME([this]() { return m_duart[1]->get_irq_vector(); }));
	map(0xff'fffb, 0xff'fffb).lr8(NAME([]() { return m68000_base_device::autovector(5); }));
	map(0xff'fffd, 0xff'fffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xff'ffff, 0xff'ffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}
