// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion 3-Link RS232 Serial Interface / Parallel Printer Interface

    Note:
    The Acorn A-Link RS232 Serial Interface has a slightly different wiring
    to the 3-Link. The A-Link will work fine with PC software, and plugged
    into a PC, but the 3-Link will not work with the Acorn software.

    The 3-Link and A-Link have been confirmed to have the same ROM.

**********************************************************************/

#include "emu.h"
#include "3link.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_3LINK_SERIAL, psion_3link_serial_device, "psion_3link_ser", "Psion 3-Link RS232 Serial Interface")
DEFINE_DEVICE_TYPE(PSION_3LINK_PARALLEL, psion_3link_parallel_device, "psion_3link_par", "Psion 3-Link Parallel Printer Interface")


//-------------------------------------------------
//  ROM( 3link_serial )
//-------------------------------------------------

ROM_START(3link_serial)
	ROM_REGION(0x20000, "rom", 0)
	ROM_LOAD("3link_v1.21f.rom", 0x0000, 0x20000, CRC(211f6218) SHA1(11cab06779ea19f109970dfa641b89f0121bfd95))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *psion_3link_serial_device::device_rom_region() const
{
	return ROM_NAME(3link_serial);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_3link_serial_device::device_add_mconfig(machine_config &config)
{
	PSION_ASIC5(config, m_asic5, 1'536'000).set_mode(psion_asic5_device::PERIPHERAL_MODE); // TODO: clock derived from host
	m_asic5->set_info_byte(0x05); // ROM + RS232
	m_asic5->readpa_handler().set([this]() { return m_rom[m_addr_latch & 0x1ffff]; });
	m_asic5->writepb_handler().set([this](uint8_t data) { m_addr_latch = (m_addr_latch & 0xffff00) | (data << 0); });
	m_asic5->writepd_handler().set([this](uint8_t data) { m_addr_latch = (m_addr_latch & 0xff00ff) | (data << 8); });
	m_asic5->writecs_handler().set([this](uint8_t data) { m_addr_latch = (m_addr_latch & 0x00ffff) | (BIT(~data, 1) << 16); });
	m_asic5->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_asic5->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_asic5->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_asic5->int_handler().set(DEVICE_SELF_OWNER, FUNC(psion_sibo_slot_device::int_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_asic5, FUNC(psion_asic5_device::write_rxd));
	m_rs232->dcd_handler().set(m_asic5, FUNC(psion_asic5_device::write_dcd));
	m_rs232->dsr_handler().set(m_asic5, FUNC(psion_asic5_device::write_dsr));
	m_rs232->cts_handler().set(m_asic5, FUNC(psion_asic5_device::write_cts));
}

void psion_3link_parallel_device::device_add_mconfig(machine_config &config)
{
	PSION_ASIC5(config, m_asic5, 1'536'000).set_mode(psion_asic5_device::PERIPHERAL_MODE); // TODO: clock derived from host
	m_asic5->set_info_byte(0x02); // Parallel
	m_asic5->readpa_handler().set("cent_status_in", FUNC(input_buffer_device::read));
	m_asic5->writepb_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_asic5->writepc_handler().set("cent_ctrl_out", FUNC(output_latch_device::write));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	centronics.ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));
	centronics.fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit6));
	centronics.perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	output_latch_device &cent_ctrl_out(OUTPUT_LATCH(config, "cent_ctrl_out"));
	cent_ctrl_out.bit_handler<4>().set("centronics", FUNC(centronics_device::write_strobe));
	cent_ctrl_out.bit_handler<5>().set("centronics", FUNC(centronics_device::write_init));
	cent_ctrl_out.bit_handler<6>().set("centronics", FUNC(centronics_device::write_autofd));
	cent_ctrl_out.bit_handler<7>().set("centronics", FUNC(centronics_device::write_select_in));

	INPUT_BUFFER(config, "cent_status_in");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_3link_device - constructor
//-------------------------------------------------

psion_3link_serial_device::psion_3link_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_3LINK_SERIAL, tag, owner, clock)
	, device_psion_sibo_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_asic5(*this, "asic5")
	, m_rs232(*this, "rs232")
{
}

psion_3link_parallel_device::psion_3link_parallel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_3LINK_PARALLEL, tag, owner, clock)
	, device_psion_sibo_interface(mconfig, *this)
	, m_asic5(*this, "asic5")
	, m_cent_ctrl_out(*this, "cent_ctrl_out")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_3link_serial_device::device_start()
{
	save_item(NAME(m_addr_latch));
}

void psion_3link_parallel_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_3link_serial_device::device_reset()
{
	m_addr_latch = 0x00;
}

void psion_3link_parallel_device::device_reset()
{
	m_cent_ctrl_out->write(0xff); // pullups
}
