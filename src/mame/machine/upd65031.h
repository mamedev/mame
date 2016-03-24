// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
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

#define MCFG_UPD65031_KB_CALLBACK(_read) \
	devcb = &upd65031_device::set_kb_rd_callback(*device, DEVCB_##_read);

#define MCFG_UPD65031_INT_CALLBACK(_write) \
	devcb = &upd65031_device::set_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD65031_NMI_CALLBACK(_write) \
	devcb = &upd65031_device::set_nmi_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD65031_SPKR_CALLBACK(_write) \
	devcb = &upd65031_device::set_spkr_wr_callback(*device, DEVCB_##_write);

#define MCFG_UPD65031_SCR_UPDATE_CB(_class, _method) \
	upd65031_device::set_screen_update_callback(*device, upd65031_screen_update_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_UPD65031_MEM_UPDATE_CB(_class, _method) \
	upd65031_device::set_memory_update_callback(*device, upd65031_memory_update_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (bitmap_ind16 &bitmap, UINT16 sbf, UINT16 hires0, UINT16 hires1, UINT16 lores0, UINT16 lores1, int flash)> upd65031_screen_update_delegate;
typedef device_delegate<void (int bank, UINT16 page, int rams)> upd65031_memory_update_delegate;

#define UPD65031_SCREEN_UPDATE(_name) void _name(bitmap_ind16 &bitmap, UINT16 sbf, UINT16 hires0, UINT16 hires1, UINT16 lores0, UINT16 lores1, int flash)
#define UPD65031_MEMORY_UPDATE(_name) void _name(int bank, UINT16 page, int rams)


// ======================> upd65031_device

class upd65031_device : public device_t
{
public:
	// construction/destruction
	upd65031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_kb_rd_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_read_kb.set_callback(object); }
	template<class _Object> static devcb_base &set_int_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_int.set_callback(object); }
	template<class _Object> static devcb_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_nmi.set_callback(object); }
	template<class _Object> static devcb_base &set_spkr_wr_callback(device_t &device, _Object object) { return downcast<upd65031_device &>(device).m_write_spkr.set_callback(object); }

	static void set_screen_update_callback(device_t &device, upd65031_screen_update_delegate callback) { downcast<upd65031_device &>(device).m_screen_update_cb = callback; }
	static void set_memory_update_callback(device_t &device, upd65031_memory_update_delegate callback) { downcast<upd65031_device &>(device).m_out_mem_cb = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( flp_w );
	DECLARE_WRITE_LINE_MEMBER( btl_w );
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	inline void interrupt_refresh();
	inline void update_rtc_interrupt();
	inline void set_mode(int mode);
	static const device_timer_id TIMER_RTC = 0;
	static const device_timer_id TIMER_FLASH = 1;
	static const device_timer_id TIMER_SPEAKER = 2;

	devcb_read8        m_read_kb;
	devcb_write_line   m_write_int;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_spkr;

	upd65031_screen_update_delegate m_screen_update_cb;  // callback for update the LCD
	upd65031_memory_update_delegate m_out_mem_cb;        // callback for update bankswitch

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
