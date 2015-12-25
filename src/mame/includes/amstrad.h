// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Barry Rodewald
/*****************************************************************************
 *
 * includes/amstrad.h
 *
 ****************************************************************************/

#ifndef AMSTRAD_H_
#define AMSTRAD_H_

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "imagedev/snapquik.h"
#include "bus/cpc/cpcexp.h"
#include "bus/cpc/ddi1.h"
#include "bus/cpc/cpc_ssa1.h"
#include "bus/cpc/cpc_rom.h"
#include "bus/cpc/mface2.h"
#include "bus/cpc/cpc_pds.h"
#include "bus/cpc/cpc_rs232.h"
#include "bus/cpc/symbfac2.h"
#include "bus/cpc/amdrum.h"
#include "bus/cpc/playcity.h"
#include "bus/cpc/smartwatch.h"
#include "bus/cpc/brunword4.h"
#include "bus/cpc/hd20.h"
#include "bus/cpc/magicsound.h"
#include "bus/cpc/doubler.h"
#include "bus/cpc/transtape.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "bus/centronics/comxpl80.h"
#include "bus/centronics/epson_ex800.h"
#include "bus/centronics/epson_lx800.h"
#include "bus/centronics/epson_lx810l.h"
#include "bus/centronics/printer.h"
#include "bus/centronics/digiblst.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


/****************************
 * Gate Array data (CPC) -
 ****************************/
struct gate_array_t
{
	std::unique_ptr<bitmap_ind16>    bitmap;        /* The bitmap we work on */
	UINT8   pen_selected;       /* Pen selection */
	UINT8   mrer;               /* Mode and ROM Enable Register */
	UINT8   upper_bank;
	UINT8   romdis;  // ROMDIS signal from the expansion port

	/* input signals from CRTC */
	int     vsync;
	int     hsync;
	int     de;
	int     ma;
	int     ra;

	/* used for timing */
	int     hsync_after_vsync_counter;
	int     hsync_counter;              /* The gate array counts CRTC HSYNC pulses using an internal 6-bit counter. */

	/* used for drawing the screen */
	attotime    last_draw_time;
	int     y;
	UINT16  *draw_p;                    /* Position in the bitmap where we are currently drawing */
	UINT16  colour;
	UINT16  address;
	UINT8   *mode_lookup;
	UINT8   data;
	UINT8   ticks;
	UINT8   ticks_increment;
	UINT16  line_ticks;
	UINT8   colour_ticks;
	UINT8   max_colour_ticks;
};

/****************************
 * ASIC data (CPC plus)
 ****************************/
struct asic_t
{
	UINT8   *ram;               /* pointer to RAM used for the CPC+ ASIC memory-mapped registers */
	UINT8   enabled;            /* Are CPC plus features enabled/unlocked */
	UINT8   pri;                /* Programmable raster interrupt */
	UINT8   seqptr;             /* Current position in the ASIC unlocking sequence */
	UINT8   rmr2;               /* ROM mapping register 2 */
	UINT16  split_ma_base;      /* Used to handle split screen support */
	UINT16  split_ma_started;   /* Used to handle split screen support */
	UINT16  vpos;               /* Current logical scanline */
	UINT16  h_start;            /* Position where DE became active */
	UINT16  h_end;              /* Position where DE became inactive */
	UINT8   addr_6845;          /* We need these to store a shadow copy of R1 of the mc6845 */
	UINT8   horiz_disp;
	UINT8   hscroll;
	UINT8   de_start;           /* flag to check if DE is been enabled this frame yet */
	bool    hsync_first_tick;   /* flag to check in first CRTC tick, used for knowing when to cover left side of screen to cover horizontal softscroll mess */
	UINT8   hsync_tick_count;

	/* DMA */
	UINT8   dma_status;
	UINT8   dma_clear;          /* Set if DMA interrupts are to be cleared automatically */
	UINT8   dma_prescaler[3];   /* DMA channel prescaler */
	UINT16  dma_repeat[3];      /* Location of the DMA channel's last repeat */
	UINT16  dma_addr[3];        /* DMA channel address */
	UINT16  dma_loopcount[3];   /* Count loops taken on this channel */
	UINT16  dma_pause[3];       /* DMA pause count */
};


class amstrad_state : public driver_device
{
public:
	enum
	{
		TIMER_PC2_LOW,
		TIMER_VIDEO_UPDATE,
		TIMER_SET_RESOLUTION
	};

	amstrad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay(*this, "ay"),
		m_fdc(*this, "upd765"),
		m_crtc(*this, "mc6845"),
		m_ppi(*this, "ppi8255"),
		m_centronics(*this, "centronics"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_ram(*this, RAM_TAG),
		m_exp(*this, "exp"),
		m_rtc(*this, "rtc"),
		m_region_maincpu(*this, "maincpu"),
		m_region_user1(*this, "user1"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_bank8(*this, "bank8"),
		m_bank9(*this, "bank9"),
		m_bank10(*this, "bank10"),
		m_bank11(*this, "bank11"),
		m_bank12(*this, "bank12"),
		m_bank13(*this, "bank13"),
		m_bank14(*this, "bank14"),
		m_bank15(*this, "bank15"),
		m_bank16(*this, "bank16"),
		m_io_kbrow(*this, "kbrow"),
		m_io_analog(*this, "analog"),
		m_io_mouse1(*this,"mouse_input1"),
		m_io_mouse2(*this,"mouse_input2"),
		m_io_mouse3(*this,"mouse_input3"),
		m_io_solder_links(*this, "solder_links"),
		m_io_green_display(*this, "green_display"),
		m_io_ctrltype(*this,"controller_type"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<z80_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	optional_device<upd765_family_device> m_fdc;  // not on a GX4000
	required_device<mc6845_device> m_crtc;
	required_device<i8255_device> m_ppi;
	optional_device<centronics_device> m_centronics;  // not on a GX4000
	optional_device<cassette_image_device> m_cassette; // not on a GX4000, (or technically, the 6128+)
	optional_device<generic_slot_device> m_cart;  // only on 664+, 6128+ and GX4000
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
	int m_prev_reg;
	UINT16 m_GateArray_render_colours[17];
	UINT8 m_mode0_lookup[256];
	UINT8 m_mode1_lookup[256];
	UINT8 m_mode2_lookup[256];
	int m_prev_data;
	int m_printer_bit8_selected;
	unsigned char m_Psg_FunctionSelected;
	int m_previous_ppi_portc_w;
	UINT8 m_amx_mouse_data;
	DECLARE_WRITE8_MEMBER(amstrad_plus_asic_4000_w);
	DECLARE_WRITE8_MEMBER(amstrad_plus_asic_6000_w);
	DECLARE_READ8_MEMBER(amstrad_plus_asic_4000_r);
	DECLARE_READ8_MEMBER(amstrad_plus_asic_6000_r);
	DECLARE_WRITE8_MEMBER(aleste_msx_mapper);
	DECLARE_READ8_MEMBER(amstrad_cpc_io_r);
	DECLARE_WRITE8_MEMBER(amstrad_cpc_io_w);
	DECLARE_READ8_MEMBER(amstrad_psg_porta_read);
	void amstrad_plus_seqcheck(int data);
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

	DECLARE_WRITE_LINE_MEMBER( cpc_romdis );
	DECLARE_WRITE_LINE_MEMBER( cpc_romen );
	DECLARE_WRITE8_MEMBER(rom_select);

	DECLARE_FLOPPY_FORMATS( aleste_floppy_formats );

	IRQ_CALLBACK_MEMBER(amstrad_cpu_acknowledge_int);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( amstrad_plus_cartridge );

	void amstrad_handle_snapshot(unsigned char *pSnapshot);
	void amstrad_rethinkMemory();
	DECLARE_SNAPSHOT_LOAD_MEMBER( amstrad );

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

protected:
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_user1;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;
	required_memory_bank m_bank6;
	required_memory_bank m_bank7;
	required_memory_bank m_bank8;
	required_memory_bank m_bank9;
	required_memory_bank m_bank10;
	required_memory_bank m_bank11;
	required_memory_bank m_bank12;
	required_memory_bank m_bank13;
	required_memory_bank m_bank14;
	required_memory_bank m_bank15;
	required_memory_bank m_bank16;
	optional_ioport_array<11> m_io_kbrow;
	optional_ioport_array<4> m_io_analog;
	optional_ioport m_io_mouse1;
	optional_ioport m_io_mouse2;
	optional_ioport m_io_mouse3;
	required_ioport m_io_solder_links;
	required_ioport m_io_green_display;
	optional_ioport m_io_ctrltype;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	memory_region *m_region_cart;

	void amstrad_init_lookups();
	void amstrad_vh_update_mode();
	void amstrad_plus_dma_parse(int channel);
	void amstrad_plus_handle_dma();
	void amstrad_vh_update_colour(int PenIndex, UINT16 hw_colour_index);
	void aleste_vh_update_colour(int PenIndex, UINT16 hw_colour_index);
	void amstrad_gate_array_get_video_data();
	void amstrad_update_video();
	void amstrad_plus_gate_array_get_video_data();
	void amstrad_plus_update_video();
	void amstrad_plus_update_video_sprites();
	void amstrad_setLowerRom();
	void amstrad_setUpperRom();
	void AmstradCPC_GA_SetRamConfiguration();
	void AmstradCPC_PALWrite(int data);
	void amstrad_GateArray_write(UINT8 dataToGateArray);
	void amstrad_reset_machine();
	void kccomp_reset_machine();
	void update_psg();
	void amstrad_common_init();
	void enumerate_roms();
	unsigned char kccomp_get_colour_element(int colour_value);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	int m_centronics_busy;
	UINT8 m_last_write;
};


/*----------- defined in machine/amstrad.c -----------*/


SLOT_INTERFACE_EXTERN(cpc_exp_cards);
SLOT_INTERFACE_EXTERN(cpcplus_exp_cards);

#endif /* AMSTRAD_H_ */
