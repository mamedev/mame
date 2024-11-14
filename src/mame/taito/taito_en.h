// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

****************************************************************************/
#ifndef MAME_TAITO_TAITO_EN_H
#define MAME_TAITO_TAITO_EN_H

#pragma once

#include "cpu/es5510/es5510.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "sound/esqpump.h"
#include "machine/mc68681.h"
#include "machine/mb87078.h"
#include "machine/mb8421.h"

class taito_en_device : public device_t, public device_mixer_interface

{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	m68000_device &audiocpu() { return *m_audiocpu; }

	void set_bank(int bank, int entry) { m_cpubank[bank]->set_entry(entry); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void en_sound_map(address_map &map) ATTR_COLD;
	void en_otis_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;

	// inherited devices/pointers
	required_device<m68000_device> m_audiocpu;
	required_device<es5505_device> m_ensoniq;
	required_device<es5510_device> m_esp;
	required_device<esq_5505_5510_pump_device> m_pump;
	required_device<mc68681_device> m_duart68681;
	required_device<mb87078_device> m_mb87078;

	required_shared_ptr<uint16_t> m_osram;
	required_shared_ptr<uint16_t> m_otisbank;
	required_region_ptr<uint16_t> m_otisrom;

	required_memory_region m_osrom;
	required_memory_bank_array<3> m_cpubank;

	uint32_t m_bankmask = 0;
	uint32_t m_old_clock = ~0;

	std::unique_ptr<offs_t[]> m_calculated_otisbank;

	IRQ_CALLBACK_MEMBER(duart_iack);
	void duart_output(uint8_t data);

	void mb87078_gain_changed(offs_t offset, uint8_t data);
	void es5505_clock_changed(u32 data);

	void en_es5505_bank_w(offs_t offset, uint16_t data);
	void en_volume_w(offs_t offset, uint8_t data);
};

DECLARE_DEVICE_TYPE(TAITO_EN, taito_en_device)

#endif // MAME_TAITO_TAITO_EN_H
