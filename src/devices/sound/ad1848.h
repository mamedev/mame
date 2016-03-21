// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef _AD1848_H_
#define _AD1848_H_

#include "emu.h"
#include "sound/dac.h"

#define MCFG_AD1848_IRQ_CALLBACK(_cb) \
	devcb = &ad1848_device::set_irq_callback(*device, DEVCB_##_cb);

#define MCFG_AD1848_DRQ_CALLBACK(_cb) \
	devcb = &ad1848_device::set_drq_callback(*device, DEVCB_##_cb);

class ad1848_device : public device_t
{
public:
	ad1848_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(dack_r);
	DECLARE_WRITE8_MEMBER(dack_w);
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<ad1848_device &>(device).m_irq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_callback(device_t &device, _Object object) { return downcast<ad1848_device &>(device).m_drq_cb.set_callback(object); }
	virtual machine_config_constructor device_mconfig_additions() const override;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	union {
		struct {
			UINT8 linp;
			UINT8 rinp;
			UINT8 laux1;
			UINT8 raux1;
			UINT8 laux2;
			UINT8 raux2;
			UINT8 lout;
			UINT8 rout;
			UINT8 dform;
			UINT8 iface;
			UINT8 pinc;
			UINT8 init;
			UINT8 misc;
			UINT8 mix;
			UINT8 ubase;
			UINT8 lbase;
		};
		UINT8 idx[15];
	} m_regs;
	UINT8 m_addr;
	UINT8 m_stat;
	UINT16 m_count;
	UINT32 m_samples;
	UINT8 m_sam_cnt;
	bool m_play, m_mce, m_trd;
	devcb_write_line m_irq_cb;
	devcb_write_line m_drq_cb;
	required_device<dac_device> m_dacl;
	required_device<dac_device> m_dacr;
	emu_timer *m_timer;
};

extern const device_type AD1848;

#endif
