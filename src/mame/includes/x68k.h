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
#include "sound/2151intf.h"
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
			m_nvram(0x4000/sizeof(UINT16)),
			m_tvram(0x80000/sizeof(UINT16)),
			m_gvram(0x80000/sizeof(UINT16)),
			m_spritereg(0x8000/sizeof(UINT16), 0)
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

	std::vector<UINT16> m_nvram;
	std::vector<UINT16> m_tvram;
	std::vector<UINT16> m_gvram;
	std::vector<UINT16> m_spritereg;

	std::unique_ptr<bitmap_ind16> m_pcgbitmap;
	std::unique_ptr<bitmap_ind16> m_gfxbitmap;

	void floppy_load_unload(bool load, floppy_image_device *dev);
	int floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	struct
	{
		int sram_writeprotect;
		int monitor;
		int contrast;
		int keyctrl;
		UINT16 cputype;
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
	UINT8 m_ppi_port[3];
	int m_current_vector[8];
	UINT8 m_current_irq_line;
	unsigned int m_scanline;
	int m_led_state;
	emu_timer* m_mouse_timer;
	emu_timer* m_led_timer;
	emu_timer* m_net_timer;
	unsigned char m_scc_prev;
	UINT16 m_ppi_prev;
	int m_mfp_prev;
	emu_timer* m_scanline_timer;
	emu_timer* m_raster_irq;
	emu_timer* m_vblank_irq;
	emu_timer* m_fdc_tc;
	emu_timer* m_adpcm_timer;
	emu_timer* m_bus_error_timer;
	UINT16* m_spriteram;
	tilemap_t* m_bg0_8;
	tilemap_t* m_bg1_8;
	tilemap_t* m_bg0_16;
	tilemap_t* m_bg1_16;
	int m_sprite_shift;
	int m_oddscanline;
	bool m_is_32bit;
	DECLARE_DRIVER_INIT(x68kxvi);
	DECLARE_DRIVER_INIT(x68030);
	DECLARE_DRIVER_INIT(x68000);
	TILE_GET_INFO_MEMBER(x68k_get_bg0_tile);
	TILE_GET_INFO_MEMBER(x68k_get_bg1_tile);
	TILE_GET_INFO_MEMBER(x68k_get_bg0_tile_16);
	TILE_GET_INFO_MEMBER(x68k_get_bg1_tile_16);
	DECLARE_MACHINE_START(x68030);
	DECLARE_MACHINE_RESET(x68000);
	DECLARE_MACHINE_START(x68000);
	DECLARE_VIDEO_START(x68000);
	DECLARE_PALETTE_INIT(x68000);
	UINT32 screen_update_x68000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(x68k_led_callback);
	TIMER_CALLBACK_MEMBER(x68k_scc_ack);
	TIMER_CALLBACK_MEMBER(md_6button_port1_timeout);
	TIMER_CALLBACK_MEMBER(md_6button_port2_timeout);
	TIMER_CALLBACK_MEMBER(x68k_bus_error);
	TIMER_CALLBACK_MEMBER(x68k_net_irq);
	TIMER_CALLBACK_MEMBER(x68k_crtc_operation_end);
	TIMER_CALLBACK_MEMBER(x68k_hsync);
	TIMER_CALLBACK_MEMBER(x68k_crtc_raster_end);
	TIMER_CALLBACK_MEMBER(x68k_crtc_raster_irq);
	TIMER_CALLBACK_MEMBER(x68k_crtc_vblank_irq);
	DECLARE_READ8_MEMBER(ppi_port_a_r);
	DECLARE_READ8_MEMBER(ppi_port_b_r);
	DECLARE_READ8_MEMBER(ppi_port_c_r);
	DECLARE_WRITE8_MEMBER(ppi_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE8_MEMBER(x68k_ct_w);
	DECLARE_WRITE_LINE_MEMBER(x68k_rtc_alarm_irq);
	DECLARE_WRITE8_MEMBER(x68030_adpcm_w);
	DECLARE_WRITE_LINE_MEMBER(mfp_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(x68k_scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(x68k_scsi_drq);

	//dmac
	void dma_irq(int channel);
	DECLARE_WRITE8_MEMBER(dma_end);
	DECLARE_WRITE8_MEMBER(dma_error);

	int x68k_read_mouse();
	void x68k_set_adpcm();
	UINT8 md_3button_r(int port);
	void md_6button_init();
	UINT8 md_6button_r(int port);
	UINT8 xpd1lr_r(int port);

	DECLARE_WRITE_LINE_MEMBER(x68k_fm_irq);
	DECLARE_WRITE_LINE_MEMBER(x68k_irq2_line);

	DECLARE_WRITE16_MEMBER(x68k_scc_w);
	DECLARE_WRITE16_MEMBER(x68k_fdc_w);
	DECLARE_READ16_MEMBER(x68k_fdc_r);
	DECLARE_WRITE16_MEMBER(x68k_ioc_w);
	DECLARE_READ16_MEMBER(x68k_ioc_r);
	DECLARE_WRITE16_MEMBER(x68k_sysport_w);
	DECLARE_READ16_MEMBER(x68k_sysport_r);
	DECLARE_WRITE16_MEMBER(x68k_ppi_w);
	DECLARE_READ16_MEMBER(x68k_ppi_r);
	DECLARE_WRITE16_MEMBER(x68k_sram_w);
	DECLARE_READ16_MEMBER(x68k_sram_r);
	DECLARE_WRITE16_MEMBER(x68k_vid_w);
	DECLARE_READ16_MEMBER(x68k_vid_r);
	DECLARE_READ16_MEMBER(x68k_areaset_r);
	DECLARE_WRITE16_MEMBER(x68k_areaset_w);
	DECLARE_WRITE16_MEMBER(x68k_enh_areaset_w);
	DECLARE_READ16_MEMBER(x68k_rom0_r);
	DECLARE_WRITE16_MEMBER(x68k_rom0_w);
	DECLARE_READ16_MEMBER(x68k_emptyram_r);
	DECLARE_WRITE16_MEMBER(x68k_emptyram_w);
	DECLARE_READ16_MEMBER(x68k_exp_r);
	DECLARE_WRITE16_MEMBER(x68k_exp_w);
	DECLARE_READ16_MEMBER(x68k_scc_r);

	DECLARE_READ16_MEMBER(x68k_spritereg_r);
	DECLARE_WRITE16_MEMBER(x68k_spritereg_w);
	DECLARE_READ16_MEMBER(x68k_spriteram_r);
	DECLARE_WRITE16_MEMBER(x68k_spriteram_w);
	DECLARE_WRITE16_MEMBER(x68k_crtc_w);
	DECLARE_READ16_MEMBER(x68k_crtc_r);
	DECLARE_WRITE16_MEMBER(x68k_gvram_w);
	DECLARE_READ16_MEMBER(x68k_gvram_r);
	DECLARE_WRITE16_MEMBER(x68k_tvram_w);
	DECLARE_READ16_MEMBER(x68k_tvram_r);
	IRQ_CALLBACK_MEMBER(x68k_int_ack);

private:
	inline void x68k_plot_pixel(bitmap_rgb32 &bitmap, int x, int y, UINT32 color);
	void x68k_crtc_text_copy(int src, int dest, UINT8 planes);
	void x68k_crtc_refresh_mode();
	void x68k_draw_text(bitmap_rgb32 &bitmap, int xscr, int yscr, rectangle rect);
	bool x68k_draw_gfx_scanline(bitmap_ind16 &bitmap, rectangle cliprect, UINT8 priority);
	void x68k_draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect);
	void x68k_draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect);

public:
	bitmap_rgb32* x68k_get_gfx_page(int pri,int type);
	attotime prescale(int val);
	void mfp_trigger_irq(int irq);
	void mfp_set_timer(int timer, unsigned char data);
	void mfp_recv_data(int data);
	DECLARE_PALETTE_DECODER(GGGGGRRRRRBBBBBI);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	void set_bus_error(UINT32 address, bool write, UINT16 mem_mask);
	bool m_bus_error;
};



#endif /* X68K_H_ */
