/*****************************************************************************
 *
 * includes/amstrad.h
 *
 ****************************************************************************/

#ifndef AMSTRAD_H_
#define AMSTRAD_H_

#include "sound/ay8910.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "imagedev/snapquik.h"
#include "machine/cpcexp.h"
#include "machine/cpc_ssa1.h"
#include "machine/cpc_rom.h"
#include "machine/mface2.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "machine/ctronics.h"

/****************************
 * Gate Array data (CPC) -
 ****************************/
struct gate_array_t
{
	bitmap_ind16	*bitmap;		/* The bitmap we work on */
	UINT8	pen_selected;		/* Pen selection */
	UINT8	mrer;				/* Mode and ROM Enable Register */
	UINT8	upper_bank;
	UINT8	romdis;  // ROMDIS signal from the expansion port

	/* input signals from CRTC */
	int		vsync;
	int		hsync;
	int		de;
	int		ma;
	int		ra;

	/* used for timing */
	int		hsync_after_vsync_counter;
	int		hsync_counter;				/* The gate array counts CRTC HSYNC pulses using an internal 6-bit counter. */

	/* used for drawing the screen */
	attotime	last_draw_time;
	int		y;
	UINT16	*draw_p;					/* Position in the bitmap where we are currently drawing */
	UINT16	colour;
	UINT16	address;
	UINT8	*mode_lookup;
	UINT8	data;
	UINT8	ticks;
	UINT8	ticks_increment;
	UINT16	line_ticks;
	UINT8	colour_ticks;
	UINT8	max_colour_ticks;
};

/****************************
 * ASIC data (CPC plus)
 ****************************/
struct asic_t
{
	UINT8	*ram;				/* pointer to RAM used for the CPC+ ASIC memory-mapped registers */
	UINT8	enabled;			/* Are CPC plus features enabled/unlocked */
	UINT8	pri;				/* Programmable raster interrupt */
	UINT8	seqptr;				/* Current position in the ASIC unlocking sequence */
	UINT8	rmr2;				/* ROM mapping register 2 */
	UINT16	split_ma_base;		/* Used to handle split screen support */
	UINT16	split_ma_started;	/* Used to handle split screen support */
	UINT16	vpos;				/* Current logical scanline */
	UINT16	h_start;			/* Position where DE became active */
	UINT16	h_end;				/* Position where DE became inactive */
	UINT8	addr_6845;			/* We need these to store a shadow copy of R1 of the mc6845 */
	UINT8	horiz_disp;
	UINT8	hscroll;
	UINT8   de_start;			/* flag to check if DE is been enabled this frame yet */

	/* DMA */
	UINT8	dma_status;
	UINT8	dma_clear;			/* Set if DMA interrupts are to be cleared automatically */
	UINT8	dma_prescaler[3];	/* DMA channel prescaler */
	UINT16	dma_repeat[3];		/* Location of the DMA channel's last repeat */
	UINT16	dma_addr[3];		/* DMA channel address */
	UINT16	dma_loopcount[3];	/* Count loops taken on this channel */
	UINT16	dma_pause[3];		/* DMA pause count */
};


class amstrad_state : public driver_device
{
public:
	amstrad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_ay(*this, "ay"),
		  m_fdc(*this, "upd765"),
		  m_crtc(*this, "mc6845"),
		  m_screen(*this, "screen"),
		  m_ppi(*this, "ppi8255"),
		  m_centronics(*this, "centronics"),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG),
		  m_exp(*this, "exp"),
		  m_rtc(*this, "rtc")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ay8912_device> m_ay;
	optional_device<upd765_family_device> m_fdc;  // not on a GX4000
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<i8255_device> m_ppi;
	optional_device<centronics_device> m_centronics;  // not on a GX4000
	optional_device<cassette_image_device> m_cassette; // not on a GX4000, (or technically, the 6128+)
	required_device<ram_device> m_ram;
	optional_device<cpc_expansion_slot_device> m_exp; // not on a GX4000
	optional_device<mc146818_device> m_rtc;  // Aleste 520EX only

	int m_system_type;
	UINT8 m_aleste_mode;
	int m_plus_irq_cause;
	gate_array_t m_gate_array;
	asic_t m_asic;
	int m_GateArray_RamConfiguration;
	unsigned char *m_AmstradCPC_RamBanks[4];
	unsigned char *m_Aleste_RamBanks[4];
	int m_aleste_active_page[4];
	unsigned char *m_Amstrad_ROM_Table[256];
	UINT8 m_ppi_port_inputs[3];
	UINT8 m_ppi_port_outputs[3];
	int m_aleste_rtc_function;
	int m_aleste_fdc_int;
	int m_prev_reg;
	UINT16 m_GateArray_render_colours[17];
	UINT8 m_mode0_lookup[256];
	UINT8 m_mode1_lookup[256];
	UINT8 m_mode2_lookup[256];
	int m_prev_data;
	int m_printer_bit8_selected;
	unsigned char m_Psg_FunctionSelected;
	int m_previous_ppi_portc_w;
	DECLARE_WRITE8_MEMBER(amstrad_plus_asic_4000_w);
	DECLARE_WRITE8_MEMBER(amstrad_plus_asic_6000_w);
	DECLARE_READ8_MEMBER(amstrad_plus_asic_4000_r);
	DECLARE_READ8_MEMBER(amstrad_plus_asic_6000_r);
	DECLARE_WRITE8_MEMBER(aleste_msx_mapper);
	DECLARE_READ8_MEMBER(amstrad_cpc_io_r);
	DECLARE_WRITE8_MEMBER(amstrad_cpc_io_w);
	DECLARE_READ8_MEMBER(amstrad_psg_porta_read);
	void amstrad_plus_seqcheck(int data);
	DECLARE_DRIVER_INIT(aleste);
	DECLARE_MACHINE_START(amstrad);
	DECLARE_MACHINE_RESET(amstrad);
	DECLARE_VIDEO_START(amstrad);
	DECLARE_PALETTE_INIT(amstrad_cpc);
	DECLARE_PALETTE_INIT(amstrad_cpc_green);
	DECLARE_MACHINE_START(plus);
	DECLARE_MACHINE_RESET(plus);
	DECLARE_PALETTE_INIT(amstrad_plus);
	DECLARE_MACHINE_START(gx4000);
	DECLARE_MACHINE_RESET(gx4000);
	DECLARE_MACHINE_START(kccomp);
	DECLARE_MACHINE_RESET(kccomp);
	DECLARE_PALETTE_INIT(kccomp);
	DECLARE_MACHINE_START(aleste);
	DECLARE_MACHINE_RESET(aleste);
	DECLARE_PALETTE_INIT(aleste);
	UINT32 screen_update_amstrad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_amstrad(screen_device &screen, bool state);
	DECLARE_INPUT_CHANGED_MEMBER(cpc_monitor_changed);
	TIMER_CALLBACK_MEMBER(amstrad_pc2_low);
	TIMER_CALLBACK_MEMBER(amstrad_video_update_timer);
	TIMER_CALLBACK_MEMBER(cb_set_resolution);
	DECLARE_WRITE_LINE_MEMBER(aleste_interrupt);
	DECLARE_WRITE_LINE_MEMBER(amstrad_hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(amstrad_plus_hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(amstrad_vsync_changed);
	DECLARE_WRITE_LINE_MEMBER(amstrad_plus_vsync_changed);
	DECLARE_WRITE_LINE_MEMBER(amstrad_de_changed);
	DECLARE_WRITE_LINE_MEMBER(amstrad_plus_de_changed);
	DECLARE_READ8_MEMBER(amstrad_ppi_porta_r);
	DECLARE_WRITE8_MEMBER(amstrad_ppi_porta_w);
	DECLARE_READ8_MEMBER(amstrad_ppi_portb_r);
	DECLARE_WRITE8_MEMBER(amstrad_ppi_portc_w);

	void aleste_interrupt(bool state);
};


/*----------- defined in machine/amstrad.c -----------*/


WRITE_LINE_DEVICE_HANDLER( cpc_irq_w );
WRITE_LINE_DEVICE_HANDLER( cpc_nmi_w );
WRITE_LINE_DEVICE_HANDLER( cpc_romdis );
WRITE_LINE_DEVICE_HANDLER( cpc_romen );

SNAPSHOT_LOAD( amstrad );

DEVICE_IMAGE_LOAD(amstrad_plus_cartridge);

extern const mc6845_interface amstrad_mc6845_intf;
extern const mc6845_interface amstrad_plus_mc6845_intf;






SLOT_INTERFACE_START(cpc_exp_cards)
	SLOT_INTERFACE("ssa1", CPC_SSA1)
	SLOT_INTERFACE("dkspeech", CPC_DKSPEECH)
	SLOT_INTERFACE("rom", CPC_ROM)
	SLOT_INTERFACE("multiface2", CPC_MFACE2)
SLOT_INTERFACE_END

SLOT_INTERFACE_START(cpcplus_exp_cards)
	SLOT_INTERFACE("ssa1", CPC_SSA1)
	SLOT_INTERFACE("dkspeech", CPC_DKSPEECH)
	SLOT_INTERFACE("rom", CPC_ROM)
SLOT_INTERFACE_END

#endif /* AMSTRAD_H_ */
