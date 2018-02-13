// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, hap, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

****************************************************************************/
#ifndef MAME_AUDIO_TAITO_EN_H
#define MAME_AUDIO_TAITO_EN_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "machine/mc68681.h"
#include "machine/mb87078.h"
#include "machine/mb8421.h"

class taito_en_device : public device_t

{
public:
	taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE16_MEMBER( en_es5505_bank_w );
	DECLARE_WRITE8_MEMBER( en_volume_w );

	//todo: hook up cpu/es5510
	DECLARE_READ16_MEMBER( es5510_dsp_r );
	DECLARE_WRITE16_MEMBER( es5510_dsp_w );

	void en_sound_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// inherited devices/pointers
	required_device<cpu_device> m_audiocpu;
	required_device<es5505_device> m_ensoniq;
	required_device<mc68681_device> m_duart68681;
	required_device<mb87078_device> m_mb87078;

	//todo: hook up cpu/es5510
	std::unique_ptr<uint32_t[]> m_es5510_dram;
	uint16_t   m_es5510_dsp_ram[0x200];
	uint32_t   m_es5510_gpr[0xc0];
	uint32_t   m_es5510_dol_latch;
	uint32_t   m_es5510_dil_latch;
	uint32_t   m_es5510_dadr_latch;
	uint32_t   m_es5510_gpr_latch;
	uint8_t    m_es5510_ram_sel;

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);

	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);
};

DECLARE_DEVICE_TYPE(TAITO_EN, taito_en_device)

#endif // MAME_AUDIO_TAITO_EN_H
