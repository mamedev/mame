// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl
/*****************************************************************************
 *
 * includes/x68k.h
 *
 * Sharp X68000
 *
 ****************************************************************************/

#ifndef X68K_H_
#define X68K_H_

#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"
#include "machine/rp5c15.h"
#include "machine/upd765.h"
#include "sound/okim6258.h"
#include "machine/ram.h"
#include "machine/8530scc.h"
#include "sound/ym2151.h"
#include "machine/i8255.h"

#define MC68901_TAG     "mc68901"
#define RP5C15_TAG      "rp5c15"

#define GFX16     0
#define GFX256    1
#define GFX65536  2

class x68k_state : public driver_device
{
public:
	enum
	{
		TIMER_X68K_LED,
		TIMER_X68K_SCC_ACK,
		TIMER_MD_6BUTTON_PORT1_TIMEOUT,
		TIMER_MD_6BUTTON_PORT2_TIMEOUT,
		TIMER_X68K_BUS_ERROR,
		TIMER_X68K_NET_IRQ,
		TIMER_X68K_CRTC_OPERATION_END,
		TIMER_X68K_HSYNC,
		TIMER_X68K_CRTC_RASTER_END,
		TIMER_X68K_CRTC_RASTER_IRQ,
		TIMER_X68K_CRTC_VBLANK_IRQ,
		TIMER_X68K_FDC_TC,
		TIMER_X68K_ADPCM
	};

	x68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_okim6258(*this, "okim6258"),
			m_hd63450(*this, "hd63450"),
			m_ram(*this, RAM_TAG),
			m_gfxdecode(*this, "gfxdecode"),
			m_gfxpalette(*this, "gfxpalette"),
			m_pcgpalette(*this, "pcgpalette"),
			m_mfpdev(*this, MC68901_TAG),
			m_rtc(*this, RP5C15_TAG),
			m_scc(*this, "scc"),
			m_ym2151(*this, "ym2151"),
			m_ppi(*this, "ppi8255"),
			m_screen(*this, "screen"),
			m_upd72065(*this, "upd72065"),
			m_options(*this, "options"),
			m_mouse1(*this, "mouse1"),
			m_mouse2(*this, "mouse2"),
			m_mouse3(*this, "mouse3"),
			m_xpd1lr(*this, "xpd1lr"),
			m_ctrltype(*this, "ctrltype"),
			m_joy1(*this, "joy1"),
			m_joy2(*this, "joy2"),
			m_md3b(*this, "md3b"),
			m_md6b(*this, "md6b"),
			m_md6b_extra(*this, "md6b_extra"),
			m_nvram(0x4000/sizeof(uint16_t)),
			m_tvram(0x80000/sizeof(uint16_t)),
			m_gvram(0x80000/sizeof(uint16_t)),
			m_spritereg(0x8000/sizeof(uint16_t), 0)
	{ }

	required_device<m68000_base_device> m_maincpu;
	required_device<okim6258_device> m_okim6258;
	required_device<hd63450_device> m_hd63450;
	required_device<ram_device> m_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_gfxpalette;
	required_device<palette_device> m_pcgpalette;
	required_device<mc68901_device> m_mfpdev;
	required_device<rp5c15_device> m_rtc;
	required_device<scc8530_t> m_scc;
	required_device<ym2151_device> m_ym2151;
	required_device<i8255_device> m_ppi;
	required_device<screen_device> m_screen;
	required_device<upd72065_device> m_upd72065;

	required_ioport m_options;
	required_ioport m_mouse1;
	required_ioport m_mouse2;
	required_ioport m_mouse3;
	required_ioport m_xpd1lr;
	required_ioport m_ctrltype;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_md3b;
	required_ioport m_md6b;
	required_ioport m_md6b_extra;

	std::vector<uint16_t> m_nvram;
	std::vector<uint16_t> m_tvram;
	std::vector<uint16_t> m_gvram;
	std::vector<uint16_t> m_spritereg;

	bitmap_ind16 m_pcgbitmap;
	bitmap_ind16 m_gfxbitmap;
	bitmap_ind16 m_special;

	void floppy_load_unload(bool load, floppy_image_device *dev);
	image_init_result floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	struct
	{
		int sram_writeprotect;
		int monitor;
		int contrast;
		int keyctrl;
		uint16_t cputype;
	} m_sysport;
	struct
	{
		floppy_image_device *floppy[4];
		int led_ctrl[4];
		int led_eject[4];
		int eject[4];
		int motor;
		int control_drives;
		int select_drive;
	} m_fdc;
	struct
	{
		int ioc7;  // "Function B operation of joystick # one option"
		int ioc6;  // "Function A operation of joystick # one option"
		int joy1_enable;  // IOC4
		int joy2_enable;  // IOC5
	} m_joy;
	struct
	{
		int rate;  // ADPCM sample rate
		int pan;  // ADPCM output switch
		int clock;  // ADPCM clock speed
	} m_adpcm;
	struct
	{
		unsigned short reg[24];  // registers
		int operation;  // operation port (0xe80481)
		int vblank;  // 1 if in VBlank
		int hblank;  // 1 if in HBlank
		int htotal;  // Horizontal Total (in characters)
		int vtotal;  // Vertical Total
		int hbegin;  // Horizontal Begin
		int vbegin;  // Vertical Begin
		int hend;    // Horizontal End
		int vend;    // Vertical End
		int hsync_end;  // Horizontal Sync End
		int vsync_end;  // Vertical Sync End
		int hsyncadjust;  // Horizontal Sync Adjustment
		float hmultiple;  // Horizontal pixel multiplier
		float vmultiple;  // Vertical scanline multiplier (x2 for doublescan modes)
		int height;
		int width;
		int visible_height;
		int visible_width;
		int hshift;
		int vshift;
		int video_width;  // horizontal total (in pixels)
		int video_height; // vertical total
		int bg_visible_height;
		int bg_visible_width;
		int bg_hshift;
		int bg_vshift;
		int bg_hvres;  // bits 0,1 = H-Res, bits 2,3 = V-Res, bit 4 = L/H Freq (0=15.98kHz, 1=31.5kHz)
		int bg_double;  // 1 if PCG is to be doubled.
		int interlace;  // 1024 vertical resolution is interlaced
	} m_crtc;  // CRTC
	struct
	{   // video controller at 0xe82000
		unsigned short reg[3];
		int text_pri;
		int sprite_pri;
		int gfx_pri;
		int gfxlayer_pri[4];  // block displayed for each priority level
		int tile8_dirty[1024];
		int tile16_dirty[256];
	} m_video;
	struct
	{
		int irqstatus;
		int fdcvector;
		int fddvector;
		int hdcvector;
		int prnvector;
	} m_ioc;
	struct
	{
		int inputtype;  // determines which input is to be received
		int irqactive;  // non-zero if IRQ is being serviced
		char last_mouse_x;  // previous mouse x-axis value
		char last_mouse_y;  // previous mouse y-axis value
		int bufferempty;  // non-zero if buffer is empty
	} m_mouse;
	struct
	{
		// port A
		int mux1;  // multiplexer value
		int seq1;  // part of 6-button input sequence.
		emu_timer* io_timeout1;
		// port B
		int mux2;  // multiplexer value
		int seq2;  // part of 6-button input sequence.
		emu_timer* io_timeout2;
	} m_mdctrl;
	uint8_t m_ppi_port[3];
	int m_current_vector[8];
	uint8_t m_current_irq_line;
	unsigned int m_scanline;
	int m_led_state;
	emu_timer* m_mouse_timer;
	emu_timer* m_led_timer;
	emu_timer* m_net_timer;
	unsigned char m_scc_prev;
	uint16_t m_ppi_prev;
	int m_mfp_prev;
	emu_timer* m_scanline_timer;
	emu_timer* m_raster_irq;
	emu_timer* m_vblank_irq;
	emu_timer* m_fdc_tc;
	emu_timer* m_adpcm_timer;
	emu_timer* m_bus_error_timer;
	uint16_t* m_spriteram;
	tilemap_t* m_bg0_8;
	tilemap_t* m_bg1_8;
	tilemap_t* m_bg0_16;
	tilemap_t* m_bg1_16;
	int m_sprite_shift;
	int m_oddscanline;
	bool m_is_32bit;
	void init_x68kxvi();
	void init_x68030();
	void init_x68000();
	void x68k_get_bg0_tile(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void x68k_get_bg1_tile(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void x68k_get_bg0_tile_16(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void x68k_get_bg1_tile_16(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_x68030();
	void machine_reset_x68000();
	void machine_start_x68000();
	void video_start_x68000();
	void palette_init_x68000(palette_device &palette);
	uint32_t screen_update_x68000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void x68k_led_callback(void *ptr, int32_t param);
	void x68k_scc_ack(void *ptr, int32_t param);
	void md_6button_port1_timeout(void *ptr, int32_t param);
	void md_6button_port2_timeout(void *ptr, int32_t param);
	void x68k_bus_error(void *ptr, int32_t param);
	void x68k_net_irq(void *ptr, int32_t param);
	void x68k_crtc_operation_end(void *ptr, int32_t param);
	void x68k_hsync(void *ptr, int32_t param);
	void x68k_crtc_raster_end(void *ptr, int32_t param);
	void x68k_crtc_raster_irq(void *ptr, int32_t param);
	void x68k_crtc_vblank_irq(void *ptr, int32_t param);
	uint8_t ppi_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppi_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ppi_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ppi_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fdc_irq(int state);
	void x68k_ct_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void x68030_adpcm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mfp_irq_callback(int state);
	void x68k_scsi_irq(int state);
	void x68k_scsi_drq(int state);

	//dmac
	void dma_irq(int channel);
	void dma_end(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dma_error(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	int x68k_read_mouse();
	void x68k_set_adpcm();
	uint8_t md_3button_r(int port);
	void md_6button_init();
	uint8_t md_6button_r(int port);
	uint8_t xpd1lr_r(int port);

	void x68k_fm_irq(int state);
	void x68k_irq2_line(int state);

	void x68k_scc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void x68k_fdc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_fdc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_ioc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_ioc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_sysport_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_sysport_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_ppi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_ppi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_sram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_sram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_vid_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_vid_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t x68k_areaset_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_areaset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void x68k_enh_areaset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_rom0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_rom0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_emptyram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_emptyram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_exp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_exp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_scc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint16_t x68k_spritereg_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_spritereg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_spriteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_spriteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void x68k_crtc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_crtc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_gvram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_gvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void x68k_tvram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t x68k_tvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	int x68k_int_ack(device_t &device, int irqline);

private:
	inline void x68k_plot_pixel(bitmap_rgb32 &bitmap, int x, int y, uint32_t color);
	void x68k_crtc_text_copy(int src, int dest, uint8_t planes);
	void x68k_crtc_refresh_mode();
	void x68k_draw_text(bitmap_rgb32 &bitmap, int xscr, int yscr, rectangle rect);
	bool x68k_draw_gfx_scanline(bitmap_ind16 &bitmap, rectangle cliprect, uint8_t priority);
	void x68k_draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect);
	void x68k_draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect);

public:
	static rgb_t GGGGGRRRRRBBBBBI_decoder(uint32_t raw);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void set_bus_error(uint32_t address, bool write, uint16_t mem_mask);
	bool m_bus_error;
};



#endif /* X68K_H_ */
