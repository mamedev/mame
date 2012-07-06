/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

#pragma once

#ifndef __V9938_H__
#define __V9938_H__



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_V9938_ADD(_tag, _screen, _vramsize) \
	MCFG_DEVICE_ADD(_tag, V9938, 0) \
	v9938_device::static_set_screen(*device, _screen); \
	v9938_device::static_set_vram_size(*device, _vramsize); \

#define MCFG_V9958_ADD(_tag, _screen, _vramsize) \
	MCFG_DEVICE_ADD(_tag, V9958, 0) \
	v9938_device::static_set_screen(*device, _screen); \
	v9938_device::static_set_vram_size(*device, _vramsize); \

#define MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(_func) \
	v9938_device::static_set_interrupt_callback(*device, v99x8_interrupt_delegate(_func, #_func, (device_t *)0), device->tag());

#define MCFG_V99X8_INTERRUPT_CALLBACK_DEVICE(_devname, _class, _func) \
	v9938_device::static_set_interrupt_callback(*device, v99x8_interrupt_delegate(&_class::_func, #_class "::" #_func, (_class *)0), _devname);


// init functions

#define MODEL_V9938	(0)
#define MODEL_V9958	(1)

// resolutions
#define RENDER_HIGH	(0)
#define RENDER_LOW	(1)
#define RENDER_AUTO	(2)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type V9938;
extern const device_type V9958;




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class v99x8_device;
typedef delegate<void (v99x8_device &, int)> v99x8_interrupt_delegate;


// ======================> v99x8_device

class v99x8_device : public device_t, public device_memory_interface
{
	friend PALETTE_INIT( v9958 );

protected:
    // construction/destruction
	v99x8_device(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock);

public:
	int interrupt ();
	void set_sprite_limit (int i) { m_sprite_limit = i; }
	void set_resolution (int);
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

	static void static_set_screen(device_t &device, const char *screen_name);
	static void static_set_vram_size(device_t &device, UINT32 vram_size);
	static void static_set_interrupt_callback(device_t &device, v99x8_interrupt_delegate callback, const char *device_name);

protected:
	const address_space_config		m_space_config;
	address_space*					m_vram_space;

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

	template<typename _PixelType, int _Width> void default_border(const pen_t *pens, _PixelType *ln);
	template<typename _PixelType, int _Width> void graphic7_border(const pen_t *pens, _PixelType *ln);
	template<typename _PixelType, int _Width> void graphic5_border(const pen_t *pens, _PixelType *ln);
	template<typename _PixelType, int _Width> void mode_text1(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_text2(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_multi(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic1(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic23(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic4(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic5(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic6(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_graphic7(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void mode_unknown(const pen_t *pens, _PixelType *ln, int line);
	template<typename _PixelType, int _Width> void default_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col);
	template<typename _PixelType, int _Width> void graphic5_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col);
	template<typename _PixelType, int _Width> void graphic7_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col);

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
	int	m_pal_write_first, m_cmd_write_first;
	UINT8 m_pal_write, m_cmd_write;
	UINT8 m_pal_reg[32], m_stat_reg[10], m_cont_reg[48], m_read_ahead;

	// memory
	UINT16 m_address_latch;
	int m_vram_size;

    // interrupt
    UINT8 m_int_state;
    v99x8_interrupt_delegate m_int_callback;
    const char *m_int_callback_device_name;
	int m_scanline;
    // blinking
    int m_blink, m_blink_count;
    // sprites
    int m_sprite_limit;
	// size
	int m_size, m_size_old, m_size_auto, m_size_now;
	// mouse
	UINT8 m_mx_delta, m_my_delta;
	// mouse & lightpen
	UINT8 m_button_state;
	// palette
	UINT16 m_pal_ind16[16];
	UINT16 m_pal_ind256[256];
	// render screen
	screen_device *m_screen;
	const char *m_screen_name;
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
		void (v99x8_device::*visible_16s)(const pen_t *, UINT16*, int);
		void (v99x8_device::*border_16)(const pen_t *, UINT16*);
		void (v99x8_device::*border_16s)(const pen_t *, UINT16*);
		void (v99x8_device::*sprites)(int, UINT8*);
		void (v99x8_device::*draw_sprite_16)(const pen_t *, UINT16*, UINT8*);
		void (v99x8_device::*draw_sprite_16s)(const pen_t *, UINT16*, UINT8*);
	} ;
	static const v99x8_mode s_modes[];

	static UINT16 *s_pal_indYJK;
};


class v9938_device : public v99x8_device
{
public:
    v9938_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class v9958_device : public v99x8_device
{
public:
    v9958_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

PALETTE_INIT( v9938 );
PALETTE_INIT( v9958 );


#endif
