// license:BSD-3-Clause
// copyright-holders:Quench, David Graves, R. Belmont
/* An interface for the ES8712 ADPCM controller */

#ifndef MAME_SOUND_ES8712_H
#define MAME_SOUND_ES8712_H

#pragma once

#include "machine/74157.h"
#include "sound/msm5205.h"
#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> es8712_device

class es8712_device : public device_t, public device_rom_interface<20> // TODO : 20 address bits?
{
public:
	es8712_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_msm_tag(T &&tag) { m_msm.set_tag(std::forward<T>(tag)); }
	auto reset_handler() { return m_reset_handler.bind(); }
	auto msm_write_handler() { return m_msm_write_cb.bind(); }

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
	void msm_w(offs_t offset, uint8_t data);
	void msm_int(int state);

	void play();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void es8712_state_save_register();

	required_device<hct157_device> m_adpcm_select;
	optional_device<msm5205_device> m_msm;

	// device callbacks
	devcb_write_line m_reset_handler;
	devcb_write8 m_msm_write_cb;

	uint8_t m_playing;          /* 1 if we're actively playing */

	uint32_t m_base_offset;     /* pointer to the base memory location */

	uint32_t m_start;           /* starting address for the next loop */
	uint32_t m_end;             /* ending address for the next loop */

	uint8_t m_adpcm_trigger;
};

DECLARE_DEVICE_TYPE(ES8712, es8712_device)

#endif // MAME_SOUND_ES8712_H
