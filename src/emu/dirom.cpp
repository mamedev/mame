#include "emu.h"


device_rom_interface::device_rom_interface(const machine_config &mconfig, device_t &device, u8 addrwidth, endianness_t endian, u8 datawidth) :
	device_memory_interface(mconfig, device),
	m_rom_tag(device.basetag()),
	m_rom_config("rom", endian, datawidth, addrwidth)
{
}

device_rom_interface::~device_rom_interface()
{
}

void device_rom_interface::static_set_device_rom_tag(device_t &device, const char *tag)
{
	device_rom_interface *romintf;
	if (!device.interface(romintf))
		throw emu_fatalerror("MCFG_DEVICE_ROM called on device '%s' with no ROM interface\n", device.tag());

	romintf->m_rom_tag = tag;
}

std::vector<std::pair<int, const address_space_config *>> device_rom_interface::memory_space_config() const
{
	return std::vector<std::pair<int, const address_space_config *>> {
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

	m_cur_bank = bank;
	m_bank->set_entry(bank);
	rom_bank_updated();
}

void device_rom_interface::reset_bank()
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
	m_rom_direct = &space().direct();
	m_bank = nullptr;
	m_cur_bank = -1;
	device().save_item(NAME(m_cur_bank));
	device().save_item(NAME(m_bank_count));
	device().machine().save().register_postload(save_prepost_delegate(FUNC(device_rom_interface::reset_bank), this));

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
