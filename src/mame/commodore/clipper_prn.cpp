// license:BSD-3-Clause
// copyright-holders:Curt Coder

#include "emu.h"
#include "clipper_prn.h"

DEFINE_DEVICE_TYPE(CLIPPER_PRN, clipper_prn_device, "clipper_prn", "PDC Clipper thermal printer")

ROM_START( clipper_prn )
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "thdr5.bin", 0x0000, 0x1000, CRC(b4296e62) SHA1(4b6edadbb810c409ece77d5834568fcc2e0bbd61) )
ROM_END

clipper_prn_device::clipper_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CLIPPER_PRN, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_ppi(*this, "ppi"),
	m_ack_cb(*this),
	m_ram(*this, "ram", 0x800, ENDIANNESS_LITTLE),
	m_p1(0xff),
	m_p2(0xff),
	m_data(0xff),
	m_pc(0xff)
{
}

const tiny_rom_entry *clipper_prn_device::device_rom_region() const
{
	return ROM_NAME( clipper_prn );
}

void clipper_prn_device::program_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("mcu", 0);
}

void clipper_prn_device::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(clipper_prn_device::io_r), FUNC(clipper_prn_device::io_w));
}

void clipper_prn_device::device_add_mconfig(machine_config &config)
{
	I8039(config, m_mcu, 6'000'000);
	m_mcu->set_addrmap(AS_PROGRAM, &clipper_prn_device::program_map);
	m_mcu->set_addrmap(AS_IO, &clipper_prn_device::io_map);
	m_mcu->p1_out_cb().set(FUNC(clipper_prn_device::p1_w));
	m_mcu->p2_in_cb().set(FUNC(clipper_prn_device::p2_r));
	m_mcu->p2_out_cb().set(FUNC(clipper_prn_device::p2_w));

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(clipper_prn_device::data_r));
	m_ppi->in_pc_callback().set_constant(0xff);
	m_ppi->out_pc_callback().set(FUNC(clipper_prn_device::pc_w));
}

uint8_t clipper_prn_device::io_r(offs_t offset)
{
	if (BIT(m_p1, 4))
		return m_ram[((m_p2 & 0x07) << 8) | offset];

	return m_ppi->read((m_p2 >> 4) & 0x03);
}

void clipper_prn_device::io_w(offs_t offset, uint8_t data)
{
	if (BIT(m_p1, 4))
		m_ram[((m_p2 & 0x07) << 8) | offset] = data;
	else
		m_ppi->write((m_p2 >> 4) & 0x03, data);
}

uint8_t clipper_prn_device::p2_r()
{
	return 0x7f | (BIT(m_pc, 3) << 7);
}

void clipper_prn_device::p1_w(uint8_t data)
{
	m_p1 = data;
}

void clipper_prn_device::p2_w(uint8_t data)
{
	m_p2 = data;
}

uint8_t clipper_prn_device::data_r()
{
	return m_data;
}

void clipper_prn_device::pc_w(uint8_t data)
{
	if (BIT(m_pc ^ data, 1))
		m_ack_cb(!BIT(data, 1));

	m_pc = data;
}

void clipper_prn_device::device_start()
{
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_data));
	save_item(NAME(m_pc));
}
