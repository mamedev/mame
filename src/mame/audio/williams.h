// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    williams.h

    Functions to emulate general the various Williams/Midway sound cards.

****************************************************************************/

#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "sound/hc55516.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(WILLIAMS_CVSD_SOUND, williams_cvsd_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_NARC_SOUND, williams_narc_sound_device)
DECLARE_DEVICE_TYPE(WILLIAMS_ADPCM_SOUND, williams_adpcm_sound_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> williams_cvsd_sound_device

class williams_cvsd_sound_device :  public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_cvsd_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(cvsd_digit_clock_clear_w);
	DECLARE_WRITE8_MEMBER(cvsd_clock_set_w);

	void williams_cvsd_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<mc6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc55516;

	// internal state
	uint8_t m_talkback;

	DECLARE_WRITE8_MEMBER(talkback_w);
};


// ======================> williams_narc_sound_device

class williams_narc_sound_device :  public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_narc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);

	// internal communications
	DECLARE_WRITE8_MEMBER(master_bank_select_w);
	DECLARE_WRITE8_MEMBER(slave_bank_select_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(command2_w);
	DECLARE_READ8_MEMBER(command2_r);
	DECLARE_WRITE8_MEMBER(master_talkback_w);
	DECLARE_WRITE8_MEMBER(master_sync_w);
	DECLARE_WRITE8_MEMBER(slave_talkback_w);
	DECLARE_WRITE8_MEMBER(slave_sync_w);
	DECLARE_WRITE8_MEMBER(cvsd_digit_clock_clear_w);
	DECLARE_WRITE8_MEMBER(cvsd_clock_set_w);

	void williams_narc_master_map(address_map &map);
	void williams_narc_slave_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// timer IDs
	enum
	{
		TID_MASTER_COMMAND,
		TID_SLAVE_COMMAND,
		TID_SYNC_CLEAR
	};

	// devices
	required_device<mc6809e_device> m_cpu0;
	required_device<mc6809e_device> m_cpu1;
	required_device<hc55516_device> m_hc55516;

	// internal state
	uint8_t m_latch;
	uint8_t m_latch2;
	uint8_t m_talkback;
	uint8_t m_audio_sync;
	uint8_t m_sound_int_state;
};


// ======================> williams_adpcm_sound_device

class williams_adpcm_sound_device : public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_adpcm_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(reset_write);
	DECLARE_READ_LINE_MEMBER(irq_read);

	// internal communications
	DECLARE_WRITE8_MEMBER(bank_select_w);
	DECLARE_WRITE8_MEMBER(oki6295_bank_select_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(talkback_w);

	void williams_adpcm_map(address_map &map);
	void williams_adpcm_oki_map(address_map &map);
protected:
	// timer IDs
	enum
	{
		TID_COMMAND,
		TID_IRQ_CLEAR
	};

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<mc6809e_device> m_cpu;

	// internal state
	uint8_t m_latch;
	uint8_t m_talkback;
	uint8_t m_sound_int_state;
};
