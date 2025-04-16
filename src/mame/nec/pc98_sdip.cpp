// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-9801 S[oftware]DIP interface

TODO:
- Discards saved settings;

**************************************************************************************************/

#include "emu.h"
#include "pc98_sdip.h"

DEFINE_DEVICE_TYPE(PC98_SDIP, pc98_sdip_device, "pc98_sdip", "NEC PC-98 SDIP device")

pc98_sdip_device::pc98_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC98_SDIP, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void pc98_sdip_device::device_start()
{
	save_pointer(NAME(m_sdip_ram), m_sdip_size);
	save_item(NAME(m_bank));
}


void pc98_sdip_device::device_reset()
{
	m_bank = 0;
}


void pc98_sdip_device::nvram_default()
{
	std::fill(std::begin(m_sdip_ram), std::end(m_sdip_ram), 0xff);
}

bool pc98_sdip_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual_size] = util::read(file, m_sdip_ram, m_sdip_size);
	return !err && (actual_size == m_sdip_size);
}

bool pc98_sdip_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual_size] = util::write(file, m_sdip_ram, m_sdip_size);
	return !err;
}


template<unsigned port> u8 pc98_sdip_device::read(offs_t offset)
{
	u8 sdip_offset = port + (m_bank * 12);

	return m_sdip_ram[sdip_offset];
}

template<unsigned port> void pc98_sdip_device::write(offs_t offset, u8 data)
{
	u8 sdip_offset = port + (m_bank * 12);

	m_sdip_ram[sdip_offset] = data;
}

void pc98_sdip_device::bank_w(offs_t offset, u8 data)
{
	// TODO: depending on model type this is hooked up differently
	// (or be not hooked up at all like in 9801US case)
	m_bank = !!(BIT(data, 6));
}

template u8 pc98_sdip_device::read<0>(offs_t offset);
template u8 pc98_sdip_device::read<1>(offs_t offset);
template u8 pc98_sdip_device::read<2>(offs_t offset);
template u8 pc98_sdip_device::read<3>(offs_t offset);
template u8 pc98_sdip_device::read<4>(offs_t offset);
template u8 pc98_sdip_device::read<5>(offs_t offset);
template u8 pc98_sdip_device::read<6>(offs_t offset);
template u8 pc98_sdip_device::read<7>(offs_t offset);
template u8 pc98_sdip_device::read<8>(offs_t offset);
template u8 pc98_sdip_device::read<9>(offs_t offset);
template u8 pc98_sdip_device::read<10>(offs_t offset);
template u8 pc98_sdip_device::read<11>(offs_t offset);

template void pc98_sdip_device::write<0>(offs_t offset, u8 data);
template void pc98_sdip_device::write<1>(offs_t offset, u8 data);
template void pc98_sdip_device::write<2>(offs_t offset, u8 data);
template void pc98_sdip_device::write<3>(offs_t offset, u8 data);
template void pc98_sdip_device::write<4>(offs_t offset, u8 data);
template void pc98_sdip_device::write<5>(offs_t offset, u8 data);
template void pc98_sdip_device::write<6>(offs_t offset, u8 data);
template void pc98_sdip_device::write<7>(offs_t offset, u8 data);
template void pc98_sdip_device::write<8>(offs_t offset, u8 data);
template void pc98_sdip_device::write<9>(offs_t offset, u8 data);
template void pc98_sdip_device::write<10>(offs_t offset, u8 data);
template void pc98_sdip_device::write<11>(offs_t offset, u8 data);
