/**********************************************************************

    NEC uPD65031 'BLINK' emulation

**********************************************************************/

#pragma once

#ifndef __UPD65031__
#define __UPD65031__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD65031_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), UPD65031, _clock)   \
	MCFG_DEVICE_CONFIG(_config)

#define UPD65031_INTERFACE(name) \
	const upd65031_interface (name) =

#define MCFG_UPD65031_KB_CALLBACK(_read) \
	devcb = &upd65031_device::set_kb_rd_callback(*device, DEVCB2_##_read);

#define MCFG_UPD65031_INT_CALLBACK(_write) \
	devcb = &upd65031_device::set_int_wr_callback(*device, DEVCB2_##_write);

#define MCFG_UPD65031_NMI_CALLBACK(_write) \
	devcb = &upd65031_device::set_nmi_wr_callback(*device, DEVCB2_##_write);

#define MCFG_UPD65031_SPKR_CALLBACK(_write) \
	devcb = &upd65031_device::set_spkr_wr_callback(*device, DEVCB2_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef void (*upd65031_screen_update_func)(device_t &device, bitmap_ind16 &bitmap, UINT16 sbf, UINT16 hires0, UINT16 hires1, UINT16 lores0, UINT16 lores1, int flash);
#define UPD65031_SCREEN_UPDATE(name) void name(device_t &device, bitmap_ind16 &bitmap, UINT16 sbf, UINT16 hires0, UINT16 hires1, UINT16 lores0, UINT16 lores1, int flash)

typedef void (*upd65031_memory_update_func)(device_t &device, int bank, UINT16 page, int rams);
#define UPD65031_MEMORY_UPDATE(name) void name(device_t &device, int bank, UINT16 page, int rams)


// ======================> upd65031_interface

struct upd65031_interface
{
	upd65031_screen_update_func m_screen_update_cb;  // callback for update the LCD
	upd65031_memory_update_func m_out_mem_cb;        // callback for update bankswitch
};


// ======================> upd65031_device

class upd65031_device : public device_t,
						public upd65031_interface
{
public:
	// construction/destruction
	upd65031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_kb_rd_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_read_kb.set_callback(object); }
	template<class _Object> static devcb2_base &set_int_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_int.set_callback(object); }
	template<class _Object> static devcb2_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_nmi.set_callback(object); }
	template<class _Object> static devcb2_base &set_spkr_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_spkr.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( flp_w );
	DECLARE_WRITE_LINE_MEMBER( btl_w );
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	inline void interrupt_refresh();
	inline void update_rtc_interrupt();
	inline void set_mode(int mode);
	static const device_timer_id TIMER_RTC = 0;
	static const device_timer_id TIMER_FLASH = 1;
	static const device_timer_id TIMER_SPEAKER = 2;

	devcb2_read8        m_read_kb;
	devcb2_write_line   m_write_int;
	devcb2_write_line   m_write_nmi;
	devcb2_write_line   m_write_spkr;

	int     m_mode;
	UINT16  m_lcd_regs[5];      // LCD registers
	UINT8   m_tim[5];           // RTC registers
	UINT8   m_sr[4];            // segment registers
	UINT8   m_sta;              // interrupt status
	UINT8   m_int;              // interrupts mask
	UINT8   m_ack;              // interrupts acknowledge
	UINT8   m_tsta;             // timer interrupt status
	UINT8   m_tmk;              // timer interrupt mask
	UINT8   m_tack;             // timer interrupts acknowledge
	UINT8   m_com;              // command register
	int     m_flash;            // cursor flash
	int     m_speaker_state;    // spkr line

	// timers
	emu_timer *m_rtc_timer;
	emu_timer *m_flash_timer;
	emu_timer *m_speaker_timer;
};


// device type definition
extern const device_type UPD65031;


#endif
