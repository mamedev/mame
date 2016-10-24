// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    williams.h

    Functions to emulate general the various Williams/Midway sound cards.

****************************************************************************/

#include "emu.h"
#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "sound/hc55516.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type WILLIAMS_NARC_SOUND;
extern const device_type WILLIAMS_CVSD_SOUND;
extern const device_type WILLIAMS_ADPCM_SOUND;



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
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reset_write(int state);

	// internal communications
	void bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void talkback_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cvsd_digit_clock_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cvsd_clock_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2151_irq_w(int state);
	void pia_irqa(int state);
	void pia_irqb(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m6809e_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<hc55516_device> m_hc55516;

	// internal state
	uint8_t m_talkback;
};


// ======================> williams_narc_sound_device

class williams_narc_sound_device :  public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	williams_narc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	uint16_t read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reset_write(int state);

	// internal communications
	void master_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slave_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void command2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t command2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void master_talkback_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void master_sync_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slave_talkback_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void slave_sync_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cvsd_digit_clock_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cvsd_clock_set_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2151_irq_w(int state);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
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
	required_device<m6809e_device> m_cpu0;
	required_device<m6809e_device> m_cpu1;
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
	void write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reset_write(int state);
	int irq_read();

	// internal communications
	void bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void oki6295_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void talkback_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2151_irq_w(int state);

protected:
	// timer IDs
	enum
	{
		TID_COMMAND,
		TID_IRQ_CLEAR
	};

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// devices
	required_device<m6809e_device> m_cpu;

	// internal state
	uint8_t m_latch;
	uint8_t m_talkback;
	uint8_t m_sound_int_state;
};
