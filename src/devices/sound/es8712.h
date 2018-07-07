// license:BSD-3-Clause
// copyright-holders:Quench, David Graves, R. Belmont
/* An interface for the ES8712 ADPCM controller */

#ifndef MAME_SOUND_ES8712_H
#define MAME_SOUND_ES8712_H

#pragma once

#include "machine/74157.h"
#include "sound/msm5205.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ES8712_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ES8712, _clock)
#define MCFG_ES8712_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ES8712, _clock)
#define MCFG_ES8712_RESET_HANDLER(_devcb) \
	downcast<es8712_device &>(*device).set_reset_handler(DEVCB_##_devcb);
#define MCFG_ES8712_MSM_WRITE_CALLBACK(_devcb) \
	downcast<es8712_device &>(*device).set_msm_write_callback(DEVCB_##_devcb);
#define MCFG_ES8712_MSM_TAG(_msmtag) \
	downcast<es8712_device &>(*device).set_msm_tag(_msmtag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> es8712_device

class es8712_device : public device_t, public device_rom_interface
{
public:
	es8712_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_msm_tag(const char *tag) { m_msm.set_tag(tag); }
	template<class Object> devcb_base &set_reset_handler(Object &&cb) { return m_reset_handler.set_callback(std::forward<Object>(cb)); }
	template<class Object> devcb_base &set_msm_write_callback(Object &&cb) { return m_msm_write_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(msm_w);
	DECLARE_WRITE_LINE_MEMBER(msm_int);

	void play();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;

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
