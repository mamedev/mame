// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.h - Williams WPC pinball sound
 *
 *  Created on: 4/10/2013
 */

#ifndef WPCSND_H_
#define WPCSND_H_

#include "cpu/m6809/m6809.h"
#include "sound/ym2151.h"
#include "sound/hc55516.h"


#define MCFG_WPC_ROM_REGION(_region) \
	wpcsnd_device::static_set_romregion(*device, _region);

#define MCFG_WPC_SOUND_REPLY_CALLBACK(_reply) \
	downcast<wpcsnd_device *>(device)->set_reply_callback(DEVCB_##_reply);


class wpcsnd_device : public device_t,
	public device_mixer_interface
{
public:
	// construction/destruction
	wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_memory_bank m_cpubank;
	required_memory_bank m_fixedbank;
	required_memory_region m_rom;

	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_WRITE8_MEMBER(volume_w);

	void ctrl_w(uint8_t data);
	void data_w(uint8_t data);
	uint8_t ctrl_r();
	uint8_t data_r();

	static void static_set_romregion(device_t &device, const char *tag);

	// callbacks
	template<class _reply> void set_reply_callback(_reply reply) { m_reply_cb.set_callback(reply); }

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	uint8_t m_latch;
	uint8_t m_reply;
	bool m_reply_available;

	// callback
	devcb_write_line m_reply_cb;
};

extern const device_type WPCSND;

#endif /* WPCSND_H_ */
