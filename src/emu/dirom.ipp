// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::device_rom_interface(const machine_config &mconfig, device_t &device) :
	device_memory_interface(mconfig, device),
	m_rom_tag(device.basetag()),
	m_rom_config("rom", Endian, 8 << DataWidth, AddrWidth, AddrShift),
	m_bank(nullptr),
	m_cur_bank(-1)
{
}

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
void device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::override_address_width(u8 width)
{
	// cach does not need level match, only specific does at this point
	//  if(emu::detail::handler_entry_dispatch_level(AddrWidth) != emu::detail::handler_entry_dispatch_level(width))
	//      emu_fatalerror("%s: Widths %d and %d are incompatible", device().tag(), width, AddrWidth);

	m_rom_config.m_addr_width = width;
}

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
device_memory_interface::space_config_vector device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_rom_config)
	};
}

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
void device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::set_rom_bank(int bank)
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

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
void device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::interface_post_load()
{
	if(m_bank)
		m_bank->set_entry(m_cur_bank);
}

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
void device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::set_rom(const void *base, u32 size)
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

template<int AddrWidth, int DataWidth, int AddrShift, endianness_t Endian>
void device_rom_interface<AddrWidth, DataWidth, AddrShift, Endian>::interface_pre_start()
{
	if(!has_space(0))
		return;

	space().cache(m_rom_cache);

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
