// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, R. Belmont
/*
    rtc65271.h: include file for rtc65271.c
*/

#ifndef __RTC65271_H__
#define __RTC65271_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC65271_INTERRUPT_CB(_devcb) \
	devcb = &rtc65271_device::set_interrupt_callback(*device, DEVCB_##_devcb);


// ======================> rtc65271_device

class rtc65271_device : public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	rtc65271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start() override;
	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
public:

	template<class _Object> static devcb_base &set_interrupt_callback(device_t &device, _Object object) { return downcast<rtc65271_device &>(device).m_interrupt_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_READ8_MEMBER( xram_r );
	DECLARE_WRITE8_MEMBER( rtc_w );
	DECLARE_WRITE8_MEMBER( xram_w );
private:
	UINT8 read(int xramsel, offs_t offset);
	void write(int xramsel, offs_t offset, UINT8 data);
	void field_interrupts();

	TIMER_CALLBACK_MEMBER(rtc_SQW_cb);
	TIMER_CALLBACK_MEMBER(rtc_begin_update_cb);
	TIMER_CALLBACK_MEMBER(rtc_end_update_cb);
	/* 64 8-bit registers (10 clock registers, 4 control/status registers, and
	50 bytes of user RAM) */
	UINT8 m_regs[64];
	UINT8 m_cur_reg;

	/* extended RAM: 4kbytes of battery-backed RAM (in pages of 32 bytes) */
	UINT8 m_xram[4096];
	UINT8 m_cur_xram_page;

	/* update timer: called every second */
	emu_timer *m_update_timer;

	/* SQW timer: called every periodic clock half-period */
	emu_timer *m_SQW_timer;
	UINT8 m_SQW_internal_state;

	/* callback called when interrupt pin state changes (may be NULL) */
	devcb_write_line    m_interrupt_cb;
};

// device type definition
extern const device_type RTC65271;

#endif
