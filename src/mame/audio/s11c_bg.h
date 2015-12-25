// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s11c_bg.h - Williams System 11C background sound (M68B09E + YM2151 + HC55516 + DAC)
 *
 *  Created on: 2/10/2013
 */

#ifndef S11C_BG_H_
#define S11C_BG_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "machine/6821pia.h"

#define MCFG_WMS_S11C_BG_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, S11C_BG, 0) \
	s11c_bg_device::static_set_gfxregion(*device, _region);


class s11c_bg_device : public device_t
{
public:
	// construction/destruction
	s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_device<dac_device> m_dac1;
	required_device<pia6821_device> m_pia40;
	required_memory_bank m_cpubank;
	memory_region* m_rom;

	DECLARE_WRITE8_MEMBER(pia40_pa_w);
	DECLARE_WRITE8_MEMBER(pia40_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia40_cb2_w);
	DECLARE_WRITE8_MEMBER(bg_speech_clock_w);
	DECLARE_WRITE8_MEMBER(bg_speech_digit_w);
	DECLARE_WRITE8_MEMBER(bgbank_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	void ctrl_w(UINT8 data);
	void data_w(UINT8 data);

	static void static_set_gfxregion(device_t &device, const char *tag);

protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	const char* m_regiontag;

};

extern const device_type S11C_BG;

#endif /* S11C_BG_H_ */
