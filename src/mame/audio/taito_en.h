// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

****************************************************************************/
#ifndef MAME_AUDIO_TAITO_EN_H
#define MAME_AUDIO_TAITO_EN_H

#pragma once

#include "cpu/es5510/es5510.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "sound/esqpump.h"
#include "machine/mc68681.h"
#include "machine/mb87078.h"
#include "machine/mb8421.h"

class taito_en_device : public device_t

{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE16_MEMBER( en_es5505_bank_w );
	DECLARE_WRITE8_MEMBER( en_volume_w );

	void set_bank(int bank, int entry) { m_cpubank[bank]->set_entry(entry); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void en_sound_map(address_map &map);
	void fc7_map(address_map &map);

	// inherited devices/pointers
	required_device<cpu_device> m_audiocpu;
	required_device<es5505_device> m_ensoniq;
	required_device<es5510_device> m_esp;
	required_device<esq_5505_5510_pump_device> m_pump;
	required_device<mc68681_device> m_duart68681;
	required_device<mb87078_device> m_mb87078;

	required_shared_ptr<uint16_t> m_osram;

	required_memory_region m_osrom;
	required_memory_bank_array<3> m_cpubank;

	uint32_t m_bankmask;

	IRQ_CALLBACK_MEMBER(duart_iack);
	DECLARE_WRITE8_MEMBER(duart_output);

	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);
	void es5505_clock_changed(u32 data);
};

DECLARE_DEVICE_TYPE(TAITO_EN, taito_en_device)

#endif // MAME_AUDIO_TAITO_EN_H
