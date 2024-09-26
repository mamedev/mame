// license: BSD-3-Clause
// copyright-holders: Angelo Salese
#ifndef MAME_HITACHI_MBS1_MMU_H
#define MAME_HITACHI_MBS1_MMU_H

#pragma once


class mbs1_mmu_device : public device_t, public device_memory_interface
{
public:
	mbs1_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~mbs1_mmu_device();

	// configuration helpers
	template <typename... T> mbs1_mmu_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 bank_r(offs_t offset);
	void bank_w(offs_t offset, u8 data);

	void init_banks(bool system_type, bool user_mode);

protected:
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	const address_space_config m_space_config;
	memory_access<20, 0, 0, ENDIANNESS_BIG>::specific m_space;

	u8 m_bank_latch[16];
};

DECLARE_DEVICE_TYPE(MBS1_MMU, mbs1_mmu_device)

#endif // MAME_MBS1_MMU_H
