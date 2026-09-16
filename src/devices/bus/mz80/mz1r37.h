// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_MZ80_MZ1R37_H
#define MAME_BUS_MZ80_MZ1R37_H

#pragma once

#include "mz80_exp.h"
#include "sound/ymopl.h"

class mz1r37_device : public mz80_exp_device
{
public:
	mz1r37_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr u32 MAX_EMM_SIZE = 640 * 1024;

	std::unique_ptr<u8[]> m_emm_ram;
	uint32_t m_emm_offset = 0;

	u8 data_r(offs_t offset);
	void address_w(offs_t offset, u8 data);
	void data_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(MZ1R37, mz1r37_device)


#endif // MAME_BUS_MZ80_MZ1E35_H
