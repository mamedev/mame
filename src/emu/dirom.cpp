// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"


device_rom_interface::device_rom_interface(const machine_config &mconfig, device_t &device, u8 addrwidth, endianness_t endian, u8 datawidth) :
	device_memory_interface(mconfig, device),
	m_rom_tag(device.basetag()),
	m_rom_config("rom", endian, datawidth, addrwidth),
	m_bank(nullptr),
	m_cur_bank(-1)
{
}

device_rom_interface::~device_rom_interface()
{
}

device_memory_interface::space_config_vector device_rom_interface::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_rom_config)
	};
}

void device_rom_interface::rom_bank_updated()
{
}

void device_rom_interface::set_rom_bank(int bank)
{
	if(!m_bank)
		emu_fatalerror("%s: device_rom_interface::set_rom_bank called without banking setup", device().tag());

	if(bank >= m_bank_count) {
		device().logerror("Warning: requested bank %x higher than actual bank count %x\n", bank, m_bank_count);
		bank = bank % m_bank_count;
	}

	if (m_cur_bank != bank) {
		m_cur_bank = bank;
		m_bank->set_entry(bank);
		rom_bank_updated();
	}
}

void device_rom_interface::interface_post_load()
{
	if(m_bank)
		m_bank->set_entry(m_cur_bank);
}

void device_rom_interface::set_rom(const void *base, u32 size)
{
	u32 mend = m_rom_config.addr_width() == 32 ? 0xffffffff : (1 << m_rom_config.addr_width()) - 1;
	u32 rend = size-1;
	m_bank_count = mend == 0xffffffff ? 1 : (rend+1) / (mend+1);
	if(m_bank_count < 1)
		m_bank_count = 1;

	if(rend >= mend) {
		space().install_read_bank(0, mend, device().tag());
		m_bank = device().machine().memory().banks().find(device().tag())->second.get();
		m_bank->configure_entries(0, m_bank_count, const_cast<void *>(base), mend+1);
		m_cur_bank = 0;

	} else {
		// Round up to the nearest power-of-two-minus-one
		u32 rmask = rend;
		rmask |= rmask >> 1;
		rmask |= rmask >> 2;
		rmask |= rmask >> 4;
		rmask |= rmask >> 8;
		rmask |= rmask >> 16;
		if(rmask != rend)
			space().unmap_read(0, mend);
		// Mirror over the high bits.  mend and rmask are both
		// powers-of-two-minus-one, so the xor works
		space().install_rom(0, rend, mend ^ rmask, const_cast<void *>(base));
	}
}

void device_rom_interface::interface_pre_start()
{
	if(!has_space(0))
		return;

	switch(space().data_width()) {
	case  8:
		if(space().endianness() == ENDIANNESS_LITTLE) {
			auto cache = space().cache<0, 0, ENDIANNESS_LITTLE>();
			m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
			m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
			m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
			m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
		} else {
			auto cache = space().cache<0, 0, ENDIANNESS_BIG>();
			m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
			m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
			m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
			m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
		}
		break;

	case 16:
		switch(space().addr_shift()) {
		case  3:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<1, 3, ENDIANNESS_LITTLE>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<1, 3, ENDIANNESS_BIG>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case  0:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<1, 0, ENDIANNESS_LITTLE>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<1, 0, ENDIANNESS_BIG>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -1:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<1, -1, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<1, -1, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		}
		break;

	case 32:
		switch(space().addr_shift()) {
		case  0:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<2, 0, ENDIANNESS_LITTLE>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<2, 0, ENDIANNESS_BIG>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -1:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<2, -1, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<2, -1, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -2:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<2, -2, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<2, -2, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		}
		break;

	case 64:
		switch(space().addr_shift()) {
		case  0:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<3, 0, ENDIANNESS_LITTLE>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<3, 0, ENDIANNESS_BIG>();
				m_r8  = [cache] (offs_t byteaddress) -> u8  { return cache->read_byte(byteaddress); };
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -1:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<3, -1, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<3, -1, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = [cache] (offs_t byteaddress) -> u16 { return cache->read_word(byteaddress); };
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -2:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<3, -2, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<3, -2, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = [cache] (offs_t byteaddress) -> u32 { return cache->read_dword(byteaddress); };
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		case -3:
			if(space().endianness() == ENDIANNESS_LITTLE) {
				auto cache = space().cache<3, -3, ENDIANNESS_LITTLE>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = nullptr;
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			} else {
				auto cache = space().cache<3, -3, ENDIANNESS_BIG>();
				m_r8  = nullptr;
				m_r16 = nullptr;
				m_r32 = nullptr;
				m_r64 = [cache] (offs_t byteaddress) -> u64 { return cache->read_qword(byteaddress); };
			}
			break;
		}
		break;
	}

	device().save_item(NAME(m_cur_bank));
	device().save_item(NAME(m_bank_count));

	if(!has_configured_map(0)) {
		memory_region *reg = device().owner()->memregion(m_rom_tag);
		if(reg)
			set_rom(reg->base(), reg->bytes());
		else {
			device().logerror("ROM region '%s' not found\n", m_rom_tag);
			u32 end = m_rom_config.addr_width() == 32 ? 0xffffffff : (1 << m_rom_config.addr_width()) - 1;
			space().unmap_read(0, end);
		}
	}
}
