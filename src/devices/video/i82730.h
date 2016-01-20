// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 82730

    Text Coprocessor

***************************************************************************/

#pragma once

#ifndef __I82730_H__
#define __I82730_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I82730_ADD(_tag, _cpu_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, I82730, _clock) \
	i82730_device::set_cpu_tag(*device, owner, _cpu_tag);

#define MCFG_I82730_SINT_HANDLER(_devcb) \
	devcb = &i82730_device::set_sint_handler(*device, DEVCB_##_devcb);

#define MCFG_I82730_UPDATE_ROW_CB(_class, _method) \
	i82730_device::set_update_row_callback(*device, i82730_update_row_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (bitmap_rgb32 &bitmap, UINT16 *data, UINT8 lc, UINT16 y, int x_count)> i82730_update_row_delegate;

#define I82730_UPDATE_ROW(name) \
	void name(bitmap_rgb32 &bitmap, UINT16 *data, UINT8 lc, UINT16 y, int x_count)


// ======================> i82730_device

class i82730_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	i82730_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _Object> static devcb_base &set_sint_handler(device_t &device, _Object object)
		{ return downcast<i82730_device &>(device).m_sint_handler.set_callback(object); }

	// inline configuration
	static void set_cpu_tag(device_t &device, device_t *owner, std::string tag);
	static void set_update_row_callback(device_t &device, i82730_update_row_delegate callback) { downcast<i82730_device &>(device).m_update_row_cb = callback; }

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(ca_w);
	DECLARE_WRITE_LINE_MEMBER(irst_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// status
	enum
	{
		DUR  = 0x001,  // data underrun
		LPU  = 0x002,  // light pen update
		DBOR = 0x004,  // data buffer overrun
		EONF = 0x008,  // end of n frames
		FDE  = 0x010,  // frame data error
		RCC  = 0x020,  // reserved channel command executed
		RDC  = 0x040,  // reserved data stream command executed
		DIP  = 0x080,  // display in progress
		VDIP = 0x100   // virtual display in progress
	};

	static const char* m_command_names[];

	bool sysbus_16bit() { return BIT(m_sysbus, 0); }

	UINT8 read_byte(offs_t address);
	UINT16 read_word(offs_t address);
	void write_byte(offs_t address, UINT8 data);
	void write_word(offs_t address, UINT16 data);

	void update_interrupts();
	void mode_set();
	void execute_command();
	void load_row();

	TIMER_CALLBACK_MEMBER(row_update);

	devcb_write_line m_sint_handler;
	i82730_update_row_delegate m_update_row_cb;

	std::string m_cpu_tag;
	address_space *m_program;

	emu_timer *m_row_timer;

	bitmap_rgb32 m_bitmap;

	bool m_initialized;
	bool m_mode_set;

	int m_ca;

	// internal registers
	UINT8 m_sysbus;
	UINT32 m_ibp;
	UINT32 m_cbp;
	UINT16 m_intmask;
	UINT16 m_status;

	int m_list_switch;
	int m_auto_line_feed;
	UINT8 m_max_dma_count;

	UINT32 m_lptr;
	UINT32 m_sptr;

	int m_dma_burst_space;
	int m_dma_burst_length;

	// display parameters
	int m_hfldstrt;
	int m_margin;
	int m_lpr;
	UINT16 m_field_attribute_mask;
	int m_vsyncstp;
	int m_vfldstrt;
	int m_vfldstp;

	int m_frame_int_count;

	// row buffers
	struct row_buffer
	{
		UINT16 data[200];
		int count;
	};

	row_buffer m_row[2];
	int m_row_index;
};

// device type definition
extern const device_type I82730;

#endif // __I82730_H__
