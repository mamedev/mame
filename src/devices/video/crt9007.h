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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CRT9007_CHARACTER_WIDTH(_value) \
	crt9007_device::static_set_character_width(*device, _value);

#define MCFG_CRT9007_INT_CALLBACK(_write) \
	devcb = &crt9007_device::set_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_DMAR_CALLBACK(_write) \
	devcb = &crt9007_device::set_dmar_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_VS_CALLBACK(_write) \
	devcb = &crt9007_device::set_vs_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_HS_CALLBACK(_write) \
	devcb = &crt9007_device::set_hs_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_VLT_CALLBACK(_write) \
	devcb = &crt9007_device::set_vlt_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_CURS_CALLBACK(_write) \
	devcb = &crt9007_device::set_curs_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_DRB_CALLBACK(_write) \
	devcb = &crt9007_device::set_drb_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_WBEN_CALLBACK(_write) \
	devcb = &crt9007_device::set_wben_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_CBLANK_CALLBACK(_write) \
	devcb = &crt9007_device::set_cblank_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_SLG_CALLBACK(_write) \
	devcb = &crt9007_device::set_slg_wr_callback(*device, DEVCB_##_write);

#define MCFG_CRT9007_SLD_CALLBACK(_write) \
	devcb = &crt9007_device::set_sld_wr_callback(*device, DEVCB_##_write);



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

	static void static_set_character_width(device_t &device, int value) { downcast<crt9007_device &>(device).m_hpixels_per_column = value; }

	template <class Object> static devcb_base &set_int_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_int.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_dmar_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_dmar.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_vs_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_vs.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_hs_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_hs.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_vlt_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_vlt.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_curs_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_curs.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_drb_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_drb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_wben_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_wben.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_cblank_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_cblank.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_slg_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_slg.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_sld_wr_callback(device_t &device, Object &&cb) { return downcast<crt9007_device &>(device).m_write_sld.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( ack_w );
	DECLARE_WRITE_LINE_MEMBER( lpstb_w );

	void set_character_width(int value);

	void crt9007(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	enum
	{
		TIMER_HSYNC,
		TIMER_VSYNC,
		TIMER_VLT,
		TIMER_CURS,
		TIMER_DRB,
		TIMER_DMA
	};

	inline uint8_t readbyte(offs_t address);

	inline void trigger_interrupt(int line);
	inline void update_cblank_line();
	inline void update_hsync_timer(int state);
	inline void update_vsync_timer(int state);
	inline void update_vlt_timer(int state);
	inline void update_curs_timer(int state);
	inline void update_drb_timer(int state);
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

	int m_disp;
	int m_hpixels_per_column;

	// runtime variables, do not state save
	int m_vsync_start;
	int m_vsync_end;
	int m_hsync_start;
	int m_hsync_end;
	int m_vlt_start;
	int m_vlt_end;
	int m_vlt_bottom;
	int m_drb_bottom;
	int m_hs;
	int m_vs;
	int m_cblank;
	int m_vlt;
	int m_drb;
	//int m_wben;
	//int m_slg;
	//int m_sld;
	int m_lpstb;

	// DMA
	int m_dmar;
	int m_ack;
	int m_dma_count;
	int m_dma_burst;
	int m_dma_delay;

	// timers
	emu_timer *m_vsync_timer;
	emu_timer *m_hsync_timer;
	emu_timer *m_vlt_timer;
	emu_timer *m_curs_timer;
	emu_timer *m_drb_timer;
	emu_timer *m_dma_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(CRT9007, crt9007_device)

#endif // MAME_VIDEO_CRT9007_H
