// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9007 CRT Video Processor and Controller (VPAC) emulation

**********************************************************************
                            _____   _____
                   VA2   1 |*    \_/     | 40  GND
                  VA10   2 |             | 39  VA9
                   VA3   3 |             | 38  VA1
                  VA11   4 |             | 37  VA8
                  VA12   5 |             | 36  VA0
                   VA4   6 |             | 35  CBLANK
                  VA13   7 |             | 34  CURS
                   VA5   8 |             | 33  ACK/_TSC
                   VA6   9 |             | 32  _CSYNC/LPSTB
                   VA7  10 |   CRT9007   | 31  SLD/SL0
                   VLT  11 |             | 30  _SLG/SL1
                   _VS  12 |             | 29  WBEN/SL2/_CSYNC
                   _HS  13 |             | 28  DMAR/SL3/VBLANK
                 _CCLK  14 |             | 27  INT
                  _DRB  15 |             | 26  _RST
                   VD7  16 |             | 25  _CS
                   VD6  17 |             | 24  VD0
                   VD5  18 |             | 23  VD1
                   VD4  19 |             | 22  VD2
                   VD3  20 |_____________| 21  +5V

**********************************************************************/

#ifndef MAME_VIDEO_CRT9007_H
#define MAME_VIDEO_CRT9007_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> crt9007_device

class crt9007_device :  public device_t,
					public device_memory_interface,
					public device_video_interface
{
public:
	// construction/destruction
	crt9007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_write_int.bind(); }
	auto dmar_callback() { return m_write_dmar.bind(); }
	auto vs_callback() { return m_write_vs.bind(); }
	auto hs_callback() { return m_write_hs.bind(); }
	auto vlt_callback() { return m_write_vlt.bind(); }
	auto curs_callback() { return m_write_curs.bind(); }
	auto drb_callback() { return m_write_drb.bind(); }
	auto wben_callback() { return m_write_wben.bind(); }
	auto cblank_callback() { return m_write_cblank.bind(); }
	auto slg_callback() { return m_write_slg.bind(); }
	auto sld_callback() { return m_write_sld.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void set_character_width(unsigned value);

	void ack_w(int state);
	void lpstb_w(int state);

	// cursor location
	bool cursor_active(unsigned x, unsigned y);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(hsync_update);
	TIMER_CALLBACK_MEMBER(vsync_update);
	TIMER_CALLBACK_MEMBER(vlt_update);
	TIMER_CALLBACK_MEMBER(cursor_update);
	TIMER_CALLBACK_MEMBER(drb_update);
	TIMER_CALLBACK_MEMBER(dma_update);
	TIMER_CALLBACK_MEMBER(frame_update);

private:
	void crt9007(address_map &map) ATTR_COLD;

	inline uint8_t readbyte(offs_t address);

	inline void trigger_interrupt(int line);
	inline void update_cblank_line();
	inline void update_hsync_timer(bool state);
	inline void update_vsync_timer(bool state);
	inline void update_vlt_timer(bool state);
	inline void update_curs_timer(bool state);
	inline void update_drb_timer(bool state);
	inline void update_dma_timer();

	inline void recompute_parameters();

	// address space configurations
	const address_space_config      m_space_config;

	devcb_write_line   m_write_int;
	devcb_write_line   m_write_dmar;
	devcb_write_line   m_write_hs;
	devcb_write_line   m_write_vs;
	devcb_write_line   m_write_vlt;
	devcb_write_line   m_write_curs;
	devcb_write_line   m_write_drb;
	devcb_write_line   m_write_wben;
	devcb_write_line   m_write_cblank;
	devcb_write_line   m_write_slg;
	devcb_write_line   m_write_sld;

	// registers
	uint8_t m_reg[0x3d];
	uint8_t m_status;

	uint8_t m_hpixels_per_column;

	// booleans
	bool m_disp;
	bool m_hs;
	bool m_vs;
	bool m_cblank;
	bool m_vlt;
	bool m_drb;
	bool m_lpstb;

	// runtime variables, do not state save
	int m_vsync_start;
	int m_vsync_end;
	int m_hsync_start;
	int m_hsync_end;
	int m_vlt_start;
	int m_vlt_end;
	int m_vlt_bottom;
	int m_drb_bottom;
	//int m_wben;
	//int m_slg;
	//int m_sld;

	// DMA
	bool m_dmar;
	bool m_ack;
	uint16_t m_dma_count;
	uint16_t m_dma_burst;
	uint8_t m_dma_delay;

	// timers
	emu_timer *m_vsync_timer;
	emu_timer *m_hsync_timer;
	emu_timer *m_vlt_timer;
	emu_timer *m_curs_timer;
	emu_timer *m_drb_timer;
	emu_timer *m_dma_timer;
	emu_timer *m_frame_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(CRT9007, crt9007_device)

#endif // MAME_VIDEO_CRT9007_H
