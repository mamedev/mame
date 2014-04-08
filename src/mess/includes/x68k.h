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
	};

	x68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_okim6258(*this, "okim6258"),
			m_hd63450(*this, "hd63450"),
			m_ram(*this, RAM_TAG),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_mfpdev(*this, MC68901_TAG),
			m_rtc(*this, RP5C15_TAG),
			m_nvram16(*this, "nvram16"),
			m_nvram32(*this, "nvram32"),
			m_gvram16(*this, "gvram16"),
			m_tvram16(*this, "tvram16"),
			m_gvram32(*this, "gvram32"),
			m_tvram32(*this, "tvram32") { }

	required_device<m68000_base_device> m_maincpu;
	required_device<okim6258_device> m_okim6258;
	required_device<hd63450_device> m_hd63450;
	required_device<ram_device> m_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<mc68901_device> m_mfpdev;
	required_device<rp5c15_device> m_rtc;

	optional_shared_ptr<UINT16> m_nvram16;
	optional_shared_ptr<UINT32> m_nvram32;

	optional_shared_ptr<UINT16> m_gvram16;
	optional_shared_ptr<UINT16> m_tvram16;
	optional_shared_ptr<UINT32> m_gvram32;
	optional_shared_ptr<UINT32> m_tvram32;

	DECLARE_WRITE_LINE_MEMBER( mfp_tbo_w );

	void floppy_load_unload();
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
		upd72065_device *fdc;
		floppy_image_device *floppy[4];
		int led_ctrl[4];
		int led_eject[4];
		int eject[4];
		int motor[4];
		int selected_drive;
		int drq_state;
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
		unsigned short text_pal[0x100];
		unsigned short gfx_pal[0x100];
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
	UINT16* m_sram;
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
	UINT16* m_spriteram;
	UINT16* m_spritereg;
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
	UINT32 screen_update_x68000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);
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
	DECLARE_READ8_MEMBER(fdc_read_byte);
	DECLARE_WRITE8_MEMBER(fdc_write_byte);

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
	DECLARE_WRITE16_MEMBER(x68k_fm_w);
	DECLARE_READ16_MEMBER(x68k_fm_r);
	DECLARE_WRITE16_MEMBER(x68k_ioc_w);
	DECLARE_READ16_MEMBER(x68k_ioc_r);
	DECLARE_WRITE16_MEMBER(x68k_sysport_w);
	DECLARE_READ16_MEMBER(x68k_sysport_r);
	DECLARE_WRITE16_MEMBER(x68k_ppi_w);
	DECLARE_READ16_MEMBER(x68k_ppi_r);
	DECLARE_WRITE16_MEMBER(x68k_sram_w);
	DECLARE_READ16_MEMBER(x68k_sram_r);
	DECLARE_READ32_MEMBER(x68k_sram32_r);
	DECLARE_WRITE32_MEMBER(x68k_sram32_w);
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
	DECLARE_WRITE32_MEMBER(x68k_gvram32_w);
	DECLARE_READ32_MEMBER(x68k_gvram32_r);
	DECLARE_WRITE32_MEMBER(x68k_tvram32_w);
	DECLARE_READ32_MEMBER(x68k_tvram32_r);
	IRQ_CALLBACK_MEMBER(x68k_int_ack);

private:
	inline void x68k_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	void x68k_crtc_text_copy(int src, int dest);
	void x68k_crtc_refresh_mode();
	void x68k_draw_text(bitmap_ind16 &bitmap, int xscr, int yscr, rectangle rect);
	void x68k_draw_gfx_scanline(bitmap_ind16 &bitmap, rectangle cliprect, UINT8 priority);
	void x68k_draw_gfx(bitmap_ind16 &bitmap,rectangle cliprect);
	void x68k_draw_sprites(bitmap_ind16 &bitmap, int priority, rectangle cliprect);

public:
	bitmap_ind16* x68k_get_gfx_page(int pri,int type);
	attotime prescale(int val);
	void mfp_trigger_irq(int irq);
	void mfp_set_timer(int timer, unsigned char data);
	void mfp_recv_data(int data);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	void set_bus_error(UINT32 address, bool write, UINT16 mem_mask);
	bool m_bus_error;
};



#endif /* X68K_H_ */
