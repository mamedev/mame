// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Cirrus Logic GD542x/3x video chipsets

*/

#include "emu.h"
#include "video/pc_vga.h"

MACHINE_CONFIG_EXTERN( pcvideo_cirrus_gd5428 );
MACHINE_CONFIG_EXTERN( pcvideo_cirrus_gd5430 );

class cirrus_gd5428_device :  public svga_device
{
public:
	// construction/destruction
	cirrus_gd5428_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	cirrus_gd5428_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual READ8_MEMBER(port_03c0_r);
	virtual WRITE8_MEMBER(port_03c0_w);
	virtual READ8_MEMBER(port_03b0_r);
	virtual WRITE8_MEMBER(port_03b0_w);
	virtual READ8_MEMBER(port_03d0_r);
	virtual WRITE8_MEMBER(port_03d0_w);
	virtual READ8_MEMBER(mem_r);
	virtual WRITE8_MEMBER(mem_w);

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual UINT16 offset();

	UINT8 m_chip_id;

	UINT8 gc_mode_ext;
	UINT8 gc_bank_0;
	UINT8 gc_bank_1;
	bool gc_locked;
	UINT8 m_lock_reg;
	UINT8 m_gr10;  // high byte of background colour (in 15/16bpp)
	UINT8 m_gr11;  // high byte of foreground colour (in 15/16bpp)

	UINT8 m_cr19;
	UINT8 m_cr1a;
	UINT8 m_cr1b;

	// hardware cursor
	UINT16 m_cursor_x;
	UINT16 m_cursor_y;
	UINT16 m_cursor_addr;
	UINT8 m_cursor_attr;
	bool m_ext_palette_enabled;
	struct { UINT8 red, green, blue; } m_ext_palette[16];  // extra palette, colour 0 is cursor background, colour 15 is cursor foreground, colour 2 is overscan border colour

	// BitBLT engine
	UINT8 m_blt_status;
	UINT8 m_blt_rop;
	UINT8 m_blt_mode;
	UINT32 m_blt_source;
	UINT32 m_blt_dest;
	UINT16 m_blt_source_pitch;
	UINT16 m_blt_dest_pitch;
	UINT16 m_blt_height;
	UINT16 m_blt_width;
	UINT32 m_blt_source_current;
	UINT32 m_blt_dest_current;
	UINT16 m_blt_trans_colour;
	UINT16 m_blt_trans_colour_mask;

	bool m_blt_system_transfer;  // blit from system memory
	UINT8 m_blt_system_count;
	UINT32 m_blt_system_buffer;
	UINT16 m_blt_pixel_count;
	UINT16 m_blt_scan_count;

	UINT8 m_scratchpad1;
	UINT8 m_scratchpad2;
	UINT8 m_scratchpad3;
	UINT8 m_vclk_num[4];
	UINT8 m_vclk_denom[4];

	inline UINT8 cirrus_vga_latch_write(int offs, UINT8 data);
private:
	void cirrus_define_video_mode();
	UINT8 cirrus_seq_reg_read(UINT8 index);
	void cirrus_seq_reg_write(UINT8 index, UINT8 data);
	UINT8 cirrus_gc_reg_read(UINT8 index);
	void cirrus_gc_reg_write(UINT8 index, UINT8 data);
	UINT8 cirrus_crtc_reg_read(UINT8 index);
	void cirrus_crtc_reg_write(UINT8 index, UINT8 data);

	void start_bitblt();
	void start_reverse_bitblt();
	void start_system_bitblt();
	void blit_dword();
	void blit_byte();  // used for colour expanded system-to-vram bitblts
	void copy_pixel(UINT8 src, UINT8 dst);
};

class cirrus_gd5430_device :  public cirrus_gd5428_device
{
public:
	cirrus_gd5430_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void device_start();
};

// device type definition
extern const device_type CIRRUS_GD5428;
extern const device_type CIRRUS_GD5430;
