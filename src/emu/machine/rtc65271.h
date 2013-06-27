/*
    rtc65271.h: include file for rtc65271.c
*/

#ifndef __RTC65271_H__
#define __RTC65271_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC65271_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, RTC65271, 0) \
	MCFG_DEVICE_CONFIG(_config)
// ======================> rtc65271_interface

struct rtc65271_interface
{
	devcb_write_line    m_interrupt_cb;
};

// ======================> rtc65271_device

class rtc65271_device : public device_t,
						public device_nvram_interface,
						public rtc65271_interface
{
public:
	// construction/destruction
	rtc65271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
public:
	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_READ8_MEMBER( xram_r );
	DECLARE_WRITE8_MEMBER( rtc_w );
	DECLARE_WRITE8_MEMBER( xram_w );
private:
	UINT8 read(int xramsel, offs_t offset);
	void write(int xramsel, offs_t offset, UINT8 data);
	void field_interrupts();

	static TIMER_CALLBACK( rtc_SQW_callback );
	static TIMER_CALLBACK( rtc_begin_update_callback );
	static TIMER_CALLBACK( rtc_end_update_callback );

	void rtc_SQW_cb();
	void rtc_begin_update_cb();
	void rtc_end_update_cb();
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
	devcb_resolved_write_line m_interrupt_func;
};

// device type definition
extern const device_type RTC65271;

#endif
