/*****************************************************************************
 *
 * includes/x68k.h
 *
 * Sharp X68000
 *
 ****************************************************************************/

#ifndef X68K_H_
#define X68K_H_

#include "machine/rp5c15.h"

#define MC68901_TAG		"mc68901"
#define RP5C15_TAG		"rp5c15"

#define GFX16     0
#define GFX256    1
#define GFX65536  2

enum
{
	MFP_IRQ_GPIP0 = 0,
	MFP_IRQ_GPIP1,
	MFP_IRQ_GPIP2,
	MFP_IRQ_GPIP3,
	MFP_IRQ_TIMERD,
	MFP_IRQ_TIMERC,
	MFP_IRQ_GPIP4,
	MFP_IRQ_GPIP5,
	MFP_IRQ_TIMERB,
	MFP_IRQ_TX_ERROR,
	MFP_IRQ_TX_EMPTY,
	MFP_IRQ_RX_ERROR,
	MFP_IRQ_RX_FULL,
	MFP_IRQ_TIMERA,
	MFP_IRQ_GPIP6,
	MFP_IRQ_GPIP7
};  // MC68901 IRQ priority levels

class x68k_state : public driver_device
{
public:
	x68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_mfpdev(*this, MC68901_TAG),
		  m_rtc(*this, RP5C15_TAG),
		  m_nvram16(*this, "nvram16"),
		  m_nvram32(*this, "nvram32"),
		  m_gvram16(*this, "gvram16"),
		  m_tvram16(*this, "tvram16"),
		  m_gvram32(*this, "gvram32"),
		  m_tvram32(*this, "tvram32")
	{ }

	required_device<mc68901_device> m_mfpdev;
	required_device<rp5c15_device> m_rtc;

	optional_shared_ptr<UINT16>	m_nvram16;
	optional_shared_ptr<UINT32>	m_nvram32;

	optional_shared_ptr<UINT16> m_gvram16;
	optional_shared_ptr<UINT16> m_tvram16;
	optional_shared_ptr<UINT32> m_gvram32;
	optional_shared_ptr<UINT32> m_tvram32;

	DECLARE_WRITE_LINE_MEMBER( mfp_tdo_w );
	DECLARE_READ8_MEMBER( mfp_gpio_r );

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
		int led_ctrl[4];
		int led_eject[4];
		int eject[4];
		int motor[4];
		int media_density[4];
		int disk_inserted[4];
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
		int gpdr;  // [0]  GPIP data register.  Typically all inputs.
		int aer;   // [1]  GPIP active edge register.  Determines on which transition an IRQ is triggered.  0 = 1->0
		int ddr;   // [2]  GPIP data direction register.  Determines which GPIP bits are inputs (0) or outputs (1)
		int iera;  // [3]  Interrupt enable register A.
		int ierb;  // [4]  Interrupt enable register B.
		int ipra;  // [5]  Interrupt pending register A.
		int iprb;  // [6]  Interrupt pending register B.
		int isra;  // [7]  Interrupt in-service register A.
		int isrb;  // [8]  Interrupt in-service register B.
		int imra;  // [9]  Interrupt mask register A.
		int imrb;  // [10] Interrupt mask register B.
		int vr;    // [11] Vector register
		int tacr;  // [12] Timer A control register
		int tbcr;  // [13] Timer B control register
		int tcdcr; // [14] Timer C & D control register
		int tadr;  // [15] Timer A data register
		int tbdr;  // [16] Timer B data register
		int tcdr;  // [17] Timer C data register
		int tddr;  // [18] Timer D data register
		int scr;   // [19] Synchronous character register
		int ucr;   // [20] USART control register
		int rsr;   // [21] Receiver status register
		int tsr;   // [22] Transmitter status register
		int udr;   // [23] USART data register
		struct
		{
			int counter;
			int prescaler;
		} timer[4];
		struct
		{
			unsigned char recv_buffer;
			unsigned char send_buffer;
			int recv_enable;
			int send_enable;
		} usart;
		int vector;
		int irqline;
		int eoi_mode;
		int current_irq;
		unsigned char gpio;
	} m_mfp;  // MC68901 Multifunction Peripheral (4MHz)
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
		int delay;  // keypress delay after initial press
		int repeat; // keypress repeat rate
		int enabled;  // keyboard enabled?
		unsigned char led_status;  // keyboard LED status
		unsigned char buffer[16];
		int headpos;  // scancodes are added here
		int tailpos;  // scancodes are read from here
		int keynum;  // number of scancodes in buffer
		int keytime[0x80];  // time until next keypress
		int keyon[0x80];  // is 1 if key is pressed, used to determine if the key state has changed from 1 to 0
		int last_pressed;  // last key pressed, for repeat key handling
	} m_keyboard;
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
	emu_timer* m_kb_timer;
	emu_timer* m_mouse_timer;
	emu_timer* m_led_timer;
	emu_timer* m_net_timer;
	unsigned char m_scc_prev;
	UINT16 m_ppi_prev;
	int m_mfp_prev;
	emu_timer* m_scanline_timer;
	emu_timer* m_raster_irq;
	emu_timer* m_vblank_irq;
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
};


/*----------- defined in drivers/x68k.c -----------*/

#ifdef UNUSED_FUNCTION
void mfp_trigger_irq(int);
TIMER_CALLBACK(mfp_timer_a_callback);
TIMER_CALLBACK(mfp_timer_b_callback);
TIMER_CALLBACK(mfp_timer_c_callback);
TIMER_CALLBACK(mfp_timer_d_callback);
#endif

/*----------- defined in video/x68k.c -----------*/

TIMER_CALLBACK(x68k_crtc_raster_irq);
TIMER_CALLBACK(x68k_crtc_vblank_irq);
TIMER_CALLBACK(x68k_hsync);


DECLARE_READ16_HANDLER( x68k_spritereg_r );
DECLARE_WRITE16_HANDLER( x68k_spritereg_w );
DECLARE_READ16_HANDLER( x68k_spriteram_r );
DECLARE_WRITE16_HANDLER( x68k_spriteram_w );
DECLARE_WRITE16_HANDLER( x68k_crtc_w );
DECLARE_READ16_HANDLER( x68k_crtc_r );
DECLARE_WRITE16_HANDLER( x68k_gvram_w );
DECLARE_READ16_HANDLER( x68k_gvram_r );
DECLARE_WRITE16_HANDLER( x68k_tvram_w );
DECLARE_READ16_HANDLER( x68k_tvram_r );
DECLARE_WRITE32_HANDLER( x68k_gvram32_w );
DECLARE_READ32_HANDLER( x68k_gvram32_r );
DECLARE_WRITE32_HANDLER( x68k_tvram32_w );
DECLARE_READ32_HANDLER( x68k_tvram32_r );


#endif /* X68K_H_ */
