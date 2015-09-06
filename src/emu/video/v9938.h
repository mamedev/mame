// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Nathan Woods
/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

#pragma once

#ifndef __V9938_H__
#define __V9938_H__



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_V9938_ADD(_tag, _screen, _vramsize, _clock) \
	MCFG_DEVICE_ADD(_tag, V9938, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	v9938_device::static_set_vram_size(*device, _vramsize);
#define MCFG_V9958_ADD(_tag, _screen, _vramsize, _clock) \
	MCFG_DEVICE_ADD(_tag, V9958, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	v9938_device::static_set_vram_size(*device, _vramsize);

#define MCFG_V99X8_INTERRUPT_CALLBACK(_irq) \
	downcast<v99x8_device *>(device)->set_interrupt_callback(DEVCB_##_irq);


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type V9938;
extern const device_type V9958;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> v99x8_device

class v99x8_device :    public device_t,
						public device_memory_interface,
						public device_video_interface
{
protected:
	// construction/destruction
	v99x8_device(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock);

public:
	template<class _irq> void set_interrupt_callback(_irq irq) {
		m_int_callback.set_callback(irq);
	}
	int interrupt ();
	int get_transpen();
	bitmap_ind16 &get_bitmap() { return m_bitmap; }
	void update_mouse_state(int mx_delta, int my_delta, int button_state);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 vram_r();
	UINT8 status_r();
	void palette_w(UINT8 data);
	void vram_w(UINT8 data);
	void command_w(UINT8 data);
	void register_w(UINT8 data);

	static void static_set_vram_size(device_t &device, UINT32 vram_size);

	/* RESET pin */
	void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

protected:
	const address_space_config      m_space_config;
	address_space*                  m_vram_space;

	int m_model;

	// device overrides
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_DATA) const { return (spacenum == AS_DATA) ? &m_space_config : NULL; }

private:
	// internal helpers
	void reset_palette();
	void vram_write(int offset, int data);
	int vram_read(int offset);
	void check_int();
	void register_write(int reg, int data);

	void default_border(const pen_t *pens, UINT16 *ln);
	void graphic7_border(const pen_t *pens, UINT16 *ln);
	void graphic5_border(const pen_t *pens, UINT16 *ln);
	void mode_text1(const pen_t *pens, UINT16 *ln, int line);
	void mode_text2(const pen_t *pens, UINT16 *ln, int line);
	void mode_multi(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic1(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic23(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic4(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic5(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic6(const pen_t *pens, UINT16 *ln, int line);
	void mode_graphic7(const pen_t *pens, UINT16 *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yae(const pen_t *pens, _PixelType *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yjk(const pen_t *pens, _PixelType *ln, int line);
	void mode_unknown(const pen_t *pens, UINT16 *ln, int line);
	void default_draw_sprite(const pen_t *pens, UINT16 *ln, UINT8 *col);
	void graphic5_draw_sprite(const pen_t *pens, UINT16 *ln, UINT8 *col);
	void graphic7_draw_sprite(const pen_t *pens, UINT16 *ln, UINT8 *col);

	void sprite_mode1(int line, UINT8 *col);
	void sprite_mode2(int line, UINT8 *col);
	void set_mode();
	void refresh_16(int line);
	void refresh_line(int line);

	void interrupt_start_vblank();

	int VDPVRMP(UINT8 M, int MX, int X, int Y);

	UINT8 VDPpoint5(int MXS, int SX, int SY);
	UINT8 VDPpoint6(int MXS, int SX, int SY);
	UINT8 VDPpoint7(int MXS, int SX, int SY);
	UINT8 VDPpoint8(int MXS, int SX, int SY);

	UINT8 VDPpoint(UINT8 SM, int MXS, int SX, int SY);

	void VDPpsetlowlevel(int addr, UINT8 CL, UINT8 M, UINT8 OP);

	void VDPpset5(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset6(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset7(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);
	void VDPpset8(int MXD, int DX, int DY, UINT8 CL, UINT8 OP);

	void VDPpset(UINT8 SM, int MXD, int DX, int DY, UINT8 CL, UINT8 OP);

	int get_vdp_timing_value(const int *);

	void srch_engine();
	void line_engine();
	void lmmv_engine();
	void lmmm_engine();
	void lmcm_engine();
	void lmmc_engine();
	void hmmv_engine();
	void hmmm_engine();
	void ymmm_engine();
	void hmmc_engine();

	inline bool v9938_second_field();

	void cpu_to_vdp(UINT8 V);
	UINT8 vdp_to_cpu();
	void report_vdp_command(UINT8 Op);
	UINT8 command_unit_w(UINT8 Op);
	void update_command();

	// general
	int m_offset_x, m_offset_y, m_visible_y, m_mode;
	// palette
	int m_pal_write_first, m_cmd_write_first;
	UINT8 m_pal_write, m_cmd_write;
	UINT8 m_pal_reg[32], m_stat_reg[10], m_cont_reg[48], m_read_ahead;
	UINT8 m_v9958_sp_mode;

	// memory
	UINT16 m_address_latch;
	int m_vram_size;

	// interrupt
	UINT8 m_int_state;
	devcb_write_line   m_int_callback;
	int m_scanline;
	// blinking
	int m_blink, m_blink_count;
	// mouse
	UINT8 m_mx_delta, m_my_delta;
	// mouse & lightpen
	UINT8 m_button_state;
	// palette
	UINT16 m_pal_ind16[16];
	UINT16 m_pal_ind256[256];
	// render bitmap
	bitmap_ind16 m_bitmap;
	// Command unit
	struct {
		int SX,SY;
		int DX,DY;
		int TX,TY;
		int NX,NY;
		int MX;
		int ASX,ADX,ANX;
		UINT8 CL;
		UINT8 LO;
		UINT8 CM;
		UINT8 MXS, MXD;
	} m_mmc;
	int  m_vdp_ops_count;
	void (v99x8_device::*m_vdp_engine)();

	struct v99x8_mode
	{
		UINT8 m;
		void (v99x8_device::*visible_16)(const pen_t *, UINT16*, int);
		void (v99x8_device::*border_16)(const pen_t *, UINT16*);
		void (v99x8_device::*sprites)(int, UINT8*);
		void (v99x8_device::*draw_sprite_16)(const pen_t *, UINT16*, UINT8*);
	} ;
	static const v99x8_mode s_modes[];
	required_device<palette_device> m_palette;
protected:
	static UINT16 s_pal_indYJK[0x20000];
};


class v9938_device : public v99x8_device
{
public:
	v9938_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(v9938);
protected:
	virtual machine_config_constructor device_mconfig_additions() const;
};

class v9958_device : public v99x8_device
{
public:
	v9958_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(v9958);

protected:
	virtual machine_config_constructor device_mconfig_additions() const;
};


#endif
