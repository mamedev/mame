// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __TANDY2K__
#define __TANDY2K__

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "formats/tandy2k_dsk.h"
#include "imagedev/harddriv.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/tandy2kb.h"
#include "machine/upd765.h"
#include "sound/speaker.h"
#include "video/crt9007.h"
#include "video/crt9021.h"
#include "video/crt9212.h"

#define SCREEN_TAG      "screen"
#define I80186_TAG      "u76"
#define I8255A_TAG      "u75"
#define I8251A_TAG      "u41"
#define I8253_TAG       "u40"
#define I8259A_0_TAG    "u42"
#define I8259A_1_TAG    "u43"
#define I8272A_TAG      "u121"
#define CRT9007_TAG     "u16"
#define CRT9212_0_TAG   "u55"
#define CRT9212_1_TAG   "u15"
#define CRT9021B_TAG    "u14"
#define WD1010_TAG      "u18"
#define WD1100_11_TAG   "u12"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"

class tandy2k_state : public driver_device
{
public:
	tandy2k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I80186_TAG),
		m_uart(*this, I8251A_TAG),
		m_i8255a(*this, I8255A_TAG),
		m_pit(*this, I8253_TAG),
		m_fdc(*this, I8272A_TAG),
		m_pic0(*this, I8259A_0_TAG),
		m_pic1(*this, I8259A_1_TAG),
		m_vpac(*this, CRT9007_TAG),
		m_drb0(*this, CRT9212_0_TAG),
		m_drb1(*this, CRT9212_1_TAG),
		m_vac(*this, CRT9021B_TAG),
		m_palette(*this, "palette"),
		m_timer_vidldsh(*this, "vidldsh"),
		m_centronics(*this, CENTRONICS_TAG),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, I8272A_TAG ":0:525qd"),
		m_floppy1(*this, I8272A_TAG ":1:525qd"),
		m_rs232(*this, RS232_TAG),
		m_kb(*this, TANDY2K_KEYBOARD_TAG),
		m_hires_ram(*this, "hires_ram"),
		m_char_ram(*this, "char_ram"),
		m_dma_mux(0),
		m_kbdclk(0),
		m_kbddat(0),
		m_kbdin(0),
		m_extclk(0),
		m_rxrdy(0),
		m_txrdy(0),
		m_pb_sel(0),
		m_vram_base(0),
		m_vidouts(0),
		m_clkspd(-1),
		m_clkcnt(-1),
		m_blc(0),
		m_bkc(0),
		m_cblank(0),
		m_dblc(0),
		m_dbkc(0),
		m_dblank(0),
		m_slg(0),
		m_sld(0),
		m_cgra(0),
		m_vidla(0),
		m_outspkr(0),
		m_spkrdata(0),
		m_centronics_ack(0),
		m_centronics_fault(0),
		m_centronics_select(0),
		m_centronics_perror(0),
		m_centronics_busy(0)
	{
		for (int i = 0; i < 4; i++)
		{
			m_busdmarq[i] = CLEAR_LINE;
		}
	}

	required_device<i80186_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<i8255_device> m_i8255a;
	required_device<pit8253_device> m_pit;
	required_device<i8272a_device> m_fdc;
	required_device<pic8259_device> m_pic0;
	required_device<pic8259_device> m_pic1;
	required_device<crt9007_t> m_vpac;
	required_device<crt9212_t> m_drb0;
	required_device<crt9212_t> m_drb1;
	required_device<crt9021_t> m_vac;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_timer_vidldsh;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<rs232_port_device> m_rs232;
	required_device<tandy2k_keyboard_device> m_kb;
	required_shared_ptr<UINT16> m_hires_ram;
	optional_shared_ptr<UINT8> m_char_ram;

	virtual void machine_start();
	virtual void device_reset_after_children();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void update_drq();
	void dma_request(int line, int state);
	void speaker_update();

	DECLARE_READ8_MEMBER( char_ram_r );
	DECLARE_WRITE8_MEMBER( char_ram_w );
	DECLARE_READ8_MEMBER( videoram_r );
	DECLARE_READ8_MEMBER( enable_r );
	DECLARE_WRITE8_MEMBER( enable_w );
	DECLARE_WRITE8_MEMBER( dma_mux_w );
	DECLARE_READ8_MEMBER( kbint_clr_r );
	DECLARE_READ16_MEMBER( vpac_r );
	DECLARE_WRITE16_MEMBER( vpac_w );
	DECLARE_READ8_MEMBER( fldtc_r );
	DECLARE_WRITE8_MEMBER( fldtc_w );
	DECLARE_WRITE8_MEMBER( addr_ctrl_w );
	DECLARE_WRITE_LINE_MEMBER( rxrdy_w );
	DECLARE_WRITE_LINE_MEMBER( txrdy_w );
	DECLARE_WRITE_LINE_MEMBER( outspkr_w );
	DECLARE_WRITE_LINE_MEMBER( intbrclk_w );
	DECLARE_WRITE_LINE_MEMBER( rfrqpulse_w );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_vlt_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_drb_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_wben_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_cblank_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_slg_w );
	DECLARE_WRITE_LINE_MEMBER( vpac_sld_w );
	DECLARE_WRITE8_MEMBER( vidla_w );
	DECLARE_WRITE8_MEMBER( drb_attr_w );
	DECLARE_WRITE_LINE_MEMBER( kbdclk_w );
	DECLARE_WRITE_LINE_MEMBER( kbddat_w );
	DECLARE_READ8_MEMBER( irq_callback );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_hdl_w );
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_select);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_fault);
	CRT9021_DRAW_CHARACTER_MEMBER( vac_draw_character );
	TIMER_DEVICE_CALLBACK_MEMBER( vidldsh_tick );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	enum
	{
		LPINEN = 0,
		KBDINEN,
		PORTINEN
	};

	/* DMA state */
	UINT8 m_dma_mux;
	int m_busdmarq[4];

	/* keyboard state */
	int m_kbdclk;
	int m_kbddat;
	UINT8 m_kbdin;

	/* serial state */
	int m_extclk;
	int m_rxrdy;
	int m_txrdy;

	/* PPI state */
	int m_pb_sel;

	/* video state */
	UINT8 m_vram_base;
	int m_vidouts;
	int m_clkspd;
	int m_clkcnt;
	int m_blc;
	int m_bkc;
	int m_cblank;
	UINT8 m_dblc;
	UINT8 m_dbkc;
	UINT8 m_dblank;
	int m_slg;
	int m_sld;
	UINT8 m_cgra;
	UINT8 m_vidla;

	/* sound state */
	int m_outspkr;
	int m_spkrdata;

	int m_centronics_ack;
	int m_centronics_fault;
	int m_centronics_select;
	int m_centronics_perror;
	int m_centronics_busy;
};

#endif
