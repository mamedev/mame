// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_KONAMI_HYPROLYB_H
#define MAME_KONAMI_HYPROLYB_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class hyprolyb_adpcm_device : public device_t
{
public:
	hyprolyb_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(uint8_t data);
	uint8_t busy_r();

	void msm_data_w(uint8_t data);
	uint8_t msm_vck_r();
	uint8_t ready_r();
	uint8_t data_r();

	void vck_callback( int st );
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	private:
	// internal state
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<msm5205_device> m_msm;
	uint8_t    m_adpcm_ready; // only bootlegs
	uint8_t    m_adpcm_busy;
	uint8_t    m_vck_ready;
};

DECLARE_DEVICE_TYPE(HYPROLYB_ADPCM, hyprolyb_adpcm_device)

#endif // MAME_KONAMI_HYPROLYB_H
