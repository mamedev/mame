// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef PC_T1T_H
#define PC_T1T_H

#include "video/mc6845.h"
#include "machine/ram.h"
#include "machine/bankdev.h"

#define T1000_SCREEN_NAME   "screen"
#define T1000_MC6845_NAME   "mc6845_t1000"

// used in tandy1000hx; used in pcjr???
struct reg
{
		reg()
		{
			index = 0;
			memset(&data, 0, sizeof(data));
		}

	uint8_t index;
	uint8_t data[0x20];
	/* see vgadoc
	   0 mode control 1
	   1 palette mask
	   2 border color
	   3 mode control 2
	   4 reset
	   0x10-0x1f palette registers
	*/
};

class pc_t1t_device :  public device_t,
								public device_video_interface
{
public:
	// construction/destruction
	pc_t1t_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	void palette_init_pcjr(palette_device &palette);

	void t1000_de_changed(int state);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	virtual MC6845_UPDATE_ROW( crtc_update_row );
	MC6845_UPDATE_ROW( t1000_text_inten_update_row );
	MC6845_UPDATE_ROW( t1000_text_blink_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_4bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_2bpp_tga_update_row );
	MC6845_UPDATE_ROW( t1000_gfx_1bpp_update_row );

protected:
	required_device<mc6845_device> m_mc6845;
	uint8_t m_mode_control, m_color_select;
	uint8_t m_status;

	struct reg m_reg;

	uint16_t m_bank;

	int m_pc_framecnt;

	uint8_t *m_displayram;

	uint8_t  *m_chr_gen;
	uint8_t  m_chr_size;
	uint16_t m_ra_offset;

	uint8_t   m_address_data_ff;

	int     m_update_row_type;
	uint8_t   m_display_enable;
	uint8_t   m_vsync;
	uint8_t   m_palette_base;

	int mode_control_r(void);
	void color_select_w(int data);
	int color_select_r(void);
	int status_r(void);
	void lightpen_strobe_w(int data);
	void vga_index_w(int data);
	int vga_data_r(void);
	int bank_r(void);

	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_vram;
};

class pcvideo_t1000_device :  public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_t1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void t1000_vsync_changed(int state);
	void disable_w(int state);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	void mode_switch( void );
	void vga_data_w(int data);
	void bank_w(int data);
	void mode_control_w(int data);
	bool m_disable;
};

extern const device_type PCVIDEO_T1000;

#define MCFG_PCVIDEO_T1000_ADD(_tag) \
		MCFG_DEVICE_ADD(_tag, PCVIDEO_T1000, 0)

class pcvideo_pcjr_device :  public pc_t1t_device
{
public:
	// construction/destruction
	pcvideo_pcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pcjr_vsync_changed(int state);

	uint8_t   *m_jxkanji;

	virtual MC6845_UPDATE_ROW( crtc_update_row ) override;
	MC6845_UPDATE_ROW( pcjx_text_update_row );
	MC6845_UPDATE_ROW( pcjr_gfx_2bpp_high_update_row );

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	void pc_pcjr_mode_switch();
	void pc_pcjr_vga_data_w(int data);
	void pc_pcjr_bank_w(int data);
	void pc_pcjx_bank_w(int data);
};

extern const device_type PCVIDEO_PCJR;

#define MCFG_PCVIDEO_PCJR_ADD(_tag) \
		MCFG_DEVICE_ADD(_tag, PCVIDEO_PCJR, 0)


#endif /* PC_T1T_H */
