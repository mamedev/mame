// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.h - Williams WPC pinball sound
 *
 *  Created on: 4/10/2013
 */

#ifndef WPCSND_H_
#define WPCSND_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/hc55516.h"
#include "sound/dac.h"

#define MCFG_WMS_WPC_SOUND_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, WPCSND, 0) \
	wpcsnd_device::static_set_gfxregion(*device, _region);

#define MCFG_WPC_SOUND_REPLY_CALLBACK(_reply) \
	downcast<wpcsnd_device *>(device)->set_reply_callback(DEVCB_##_reply);

class wpcsnd_device : public device_t
{
public:
	// construction/destruction
	wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_device<dac_device> m_dac;
	required_memory_bank m_cpubank;
	required_memory_bank m_fixedbank;
	memory_region* m_rom;

	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_WRITE8_MEMBER(volume_w);

	void ctrl_w(UINT8 data);
	void data_w(UINT8 data);
	UINT8 ctrl_r();
	UINT8 data_r();

	static void static_set_gfxregion(device_t &device, const char *tag);

	// callbacks
	template<class _reply> void set_reply_callback(_reply reply) { m_reply_cb.set_callback(reply); }

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	const char* m_regiontag;
	UINT8 m_latch;
	UINT8 m_reply;
	bool m_reply_available;

	// callback
	devcb_write_line m_reply_cb;

};

extern const device_type WPCSND;

#endif /* WPCSND_H_ */
