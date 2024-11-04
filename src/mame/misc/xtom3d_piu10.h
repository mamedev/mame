// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_MISC_XTOM3D_PIU10_H
#define MAME_MISC_XTOM3D_PIU10_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/cat702.h"
#include "machine/intelfsh.h"
#include "sound/dac3350a.h"
#include "sound/mas3507d.h"


class isa16_piu10 : public device_t, public device_isa16_card_interface
{
public:
	isa16_piu10(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void add_route(T &&... args) { m_dac3350a.lookup()->add_route(std::forward<T>(args)...); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void remap(int space_id, offs_t start, offs_t end) override;

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

	void mas3507d_mpeg_frame_sync(int state);
	void mas3507d_demand(int state);

	required_device<cat702_piu_device> m_cat702;
	required_device<dac3350a_device> m_dac3350a;
	required_device<mas3507d_device> m_mas3507d;
	required_device<macronix_29f1610mc_16bit_device> m_flash;

	uint32_t m_addr;
	uint16_t m_dest;
	bool m_flash_unlock;

	uint8_t m_mp3_mpeg_frame_sync, m_mp3_demand;

	uint8_t m_cat702_data;
};

DECLARE_DEVICE_TYPE(ISA16_PIU10, isa16_piu10)

#endif // MAME_MISC_XTOM3D_PIU10_H
