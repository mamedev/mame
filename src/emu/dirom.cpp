#include "emu.h"


device_rom_interface::device_rom_interface(const machine_config &mconfig, device_t &device, UINT8 addrwidth, endianness_t endian, UINT8 datawidth) :
    device_memory_interface(mconfig, device),
    m_rom_config("rom", endian, datawidth, addrwidth)
{
}

device_rom_interface::~device_rom_interface()
{
}

const address_space_config *device_rom_interface::memory_space_config(address_spacenum spacenum) const
{
    return spacenum ? nullptr : &m_rom_config;
}

void device_rom_interface::set_rom_bank(int bank)
{
	if(!m_bank)
		emu_fatalerror("%s: device_rom_interface::set_rom_bank called without banking setup", device().tag());

	m_cur_bank = bank;
	m_bank->set_entry(bank);
}

void device_rom_interface::reset_bank()
{
	if(m_bank)
		m_bank->set_entry(m_cur_bank);
}

void device_rom_interface::set_rom(const void *base, UINT32 size)
{
	UINT32 mend = m_rom_config.addr_width() == 32 ? 0xffffffff : (1 << m_rom_config.addr_width()) - 1;
	UINT32 rend = size-1;
	UINT32 banks = (rend+1) / (mend+1);
	if(banks < 1)
		banks = 1;

	if(rend >= mend) {
		space().install_read_bank(0, mend, device().tag());
		m_bank = device().machine().memory().banks().find(device().tag())->second.get();
		m_bank->configure_entries(0, banks, const_cast<void *>(base), mend+1);

	} else {
		// Round up to the nearest power-of-two-minus-one
		UINT32 rmask = rend;
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
	m_rom_direct = &space().direct();
	m_bank = nullptr;
	m_cur_bank = -1;
	device().save_item(NAME(m_cur_bank));
	device().machine().save().register_postload(save_prepost_delegate(FUNC(device_rom_interface::reset_bank), this));

	if(!has_configured_map(0)) {
		memory_region *reg = device().memregion(DEVICE_SELF);
		if(reg)
			set_rom(reg->base(), reg->bytes());
		else {
			UINT32 end = m_rom_config.addr_width() == 32 ? 0xffffffff : (1 << m_rom_config.addr_width()) - 1;
			space().unmap_read(0, end);
		}
	}
}

