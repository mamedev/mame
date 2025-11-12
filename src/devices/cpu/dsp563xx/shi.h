// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_DSP563XX_SHI_H
#define MAME_CPU_DSP563XX_SHI_H

#pragma once

// forward declaration
class dsp563xx_device;

class dsp5636x_shi_device : public device_t
{
public:
	// device type constructor
	dsp5636x_shi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// I/O register map
	void map(address_map &map);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// I/O register handlers
	u32 hckr_r();
	void hckr_w(u32 data);
	u32 hcsr_r();
	void hcsr_w(u32 data);
	u32 hsar_r();
	void hsar_w(u32 data);
	void htx_w(u32 data);
	u32 hrx_r();

	// reference to CPU
	required_device<dsp563xx_device> m_cpu;

	// internal state
	u32 m_iosr;
	u32 m_hsar;
	u32 m_hckr;
	u32 m_hcsr;
	u32 m_hrx[10];
	u32 m_htx;
};

// device type declaration
DECLARE_DEVICE_TYPE(DSP5636X_SHI, dsp5636x_shi_device)

#endif // MAME_CPU_DSP563XX_SHI_H
