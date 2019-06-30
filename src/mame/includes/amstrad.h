// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Barry Rodewald
/*****************************************************************************
 *
 * includes/amstrad.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_AMSTRAD_H
#define MAME_INCLUDES_AMSTRAD_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "imagedev/floppy.h"
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
#include "emupal.h"
#include "screen.h"


/****************************
 * Gate Array data (CPC) -
 ****************************/
struct gate_array_t
{
	std::unique_ptr<bitmap_ind16>    bitmap;        /* The bitmap we work on */
	uint8_t   pen_selected;       /* Pen selection */
	uint8_t   mrer;               /* Mode and ROM Enable Register */
	uint8_t   upper_bank;
	uint8_t   romdis;  // ROMDIS signal from the expansion port

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
	uint16_t  *draw_p;                    /* Position in the bitmap where we are currently drawing */
	uint16_t  colour;
	uint16_t  address;
	uint8_t   *mode_lookup;
	uint8_t   data;
	uint8_t   ticks;
	uint8_t   ticks_increment;
	uint16_t  line_ticks;
	uint8_t   colour_ticks;
	uint8_t   max_colour_ticks;
};

/****************************
 * ASIC data (CPC plus)
 ****************************/
struct asic_t
{
	uint8_t   *ram;               /* pointer to RAM used for the CPC+ ASIC memory-mapped registers */
	uint8_t   enabled;            /* Are CPC plus features enabled/unlocked */
	uint8_t   pri;                /* Programmable raster interrupt */
	uint8_t   seqptr;             /* Current position in the ASIC unlocking sequence */
	uint8_t   rmr2;               /* ROM mapping register 2 */
	uint16_t  split_ma_base;      /* Used to handle split screen support */
	uint16_t  split_ma_started;   /* Used to handle split screen support */
	uint16_t  vpos;               /* Current logical scanline */
	uint16_t  h_start;            /* Position where DE became active */
	uint16_t  h_end;              /* Position where DE became inactive */
	uint8_t   addr_6845;          /* We need these to store a shadow copy of R1 of the mc6845 */
	uint8_t   horiz_disp;
	uint8_t   hscroll;
	uint8_t   de_start;           /* flag to check if DE is been enabled this frame yet */
	bool    hsync_first_tick;   /* flag to check in first CRTC tick, used for knowing when to cover left side of screen to cover horizontal softscroll mess */
	uint8_t   hsync_tick_count;

	/* DMA */
	uint8_t   dma_status;
	uint8_t   dma_clear;          /* Set if DMA interrupts are to be cleared automatically */
	uint8_t   dma_prescaler[3];   /* DMA channel prescaler */
	uint16_t  dma_repeat[3];      /* Location of the DMA channel's last repeat */
	uint16_t  dma_addr[3];        /* DMA channel address */
	uint16_t  dma_loopcount[3];   /* Count loops taken on this channel */
	uint16_t  dma_pause[3];       /* DMA pause count */
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

	amstrad_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ay(*this, "ay"),
		m_fdc(*this, "upd765"),
		m_floppy(*this, "upd765:%u", 0U),
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
		m_banks(*this, "bank%u", 1U),
		m_io_kbrow(*this, "kbrow.%u", 0U),
		m_io_analog(*this, "analog.%u", 0U),
		m_io_mouse(*this,"mouse_input%u", 1U),
		m_io_solder_links(*this, "solder_links"),
		m_io_green_display(*this, "green_display"),
		m_io_ctrltype(*this,"controller_type"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	required_device<z80_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	optional_device<upd765_family_device> m_fdc;  // not on a GX4000
	optional_device_array<floppy_connector, 2> m_floppy;
	required_device<mc6845_device> m_crtc;
	required_device<i8255_device> m_ppi;
	optional_device<centronics_device> m_centronics;  // not on a GX4000
	optional_device<cassette_image_device> m_cassette; // not on a GX4000, (or technically, the 6128+)
	optional_device<generic_slot_device> m_cart;  // only on 664+, 6128+ and GX4000
	required_device<ram_device> m_ram;
	optional_device<cpc_expansion_slot_device> m_exp; // not on a GX4000
	optional_device<mc146818_device> m_rtc;  // Aleste 520EX only

	int m_system_type;
	uint8_t m_aleste_mode;
	int m_plus_irq_cause;
	gate_array_t m_gate_array;
	asic_t m_asic;
	int m_GateArray_RamConfiguration;
	unsigned char *m_AmstradCPC_RamBanks[4];
	unsigned char *m_Aleste_RamBanks[4];
	int m_aleste_active_page[4];
	unsigned char *m_Amstrad_ROM_Table[256];
	uint8_t m_ppi_port_inputs[3];
	uint8_t m_ppi_port_outputs[3];
	int m_aleste_rtc_function;
	int m_prev_reg;
	uint16_t m_GateArray_render_colours[17];
	uint8_t m_mode0_lookup[256];
	uint8_t m_mode1_lookup[256];
	uint8_t m_mode2_lookup[256];
	int m_prev_data;
	int m_printer_bit8_selected;
	unsigned char m_Psg_FunctionSelected;
	int m_previous_ppi_portc_w;
	uint8_t m_amx_mouse_data;
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
	void amstrad_cpc_palette(palette_device &palette) const;
	void amstrad_cpc_green_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(plus);
	DECLARE_MACHINE_RESET(plus);
	void amstrad_plus_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(gx4000);
	DECLARE_MACHINE_RESET(gx4000);
	DECLARE_MACHINE_START(kccomp);
	DECLARE_MACHINE_RESET(kccomp);
	void kccomp_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(aleste);
	DECLARE_MACHINE_RESET(aleste);
	void aleste_palette(palette_device &palette) const;
	uint32_t screen_update_amstrad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_amstrad);
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
	DECLARE_WRITE8_MEMBER(rom_select);

	DECLARE_FLOPPY_FORMATS( aleste_floppy_formats );

	IRQ_CALLBACK_MEMBER(amstrad_cpu_acknowledge_int);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( amstrad_plus_cartridge );

	void amstrad_handle_snapshot(unsigned char *pSnapshot);
	void amstrad_rethinkMemory();
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

	void cpcplus_cartslot(machine_config &config);
	void amstrad_base(machine_config &config);
	void cpc664(machine_config &config);
	void cpcplus(machine_config &config);
	void gx4000(machine_config &config);
	void cpc6128(machine_config &config);
	void aleste(machine_config &config);
	void kccomp(machine_config &config);
	void cpc464(machine_config &config);
	void amstrad_io(address_map &map);
	void amstrad_mem(address_map &map);
protected:
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_user1;
	required_memory_bank_array<16> m_banks;
	optional_ioport_array<11> m_io_kbrow;
	optional_ioport_array<4> m_io_analog;
	optional_ioport_array<3> m_io_mouse;
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
	void amstrad_vh_update_colour(int PenIndex, uint16_t hw_colour_index);
	void aleste_vh_update_colour(int PenIndex, uint16_t hw_colour_index);
	void amstrad_gate_array_get_video_data();
	void amstrad_update_video();
	void amstrad_plus_gate_array_get_video_data();
	void amstrad_plus_update_video();
	void amstrad_plus_update_video_sprites();
	void amstrad_setLowerRom();
	void amstrad_setUpperRom();
	void AmstradCPC_GA_SetRamConfiguration();
	void AmstradCPC_PALWrite(int data);
	void amstrad_GateArray_write(uint8_t dataToGateArray);
	void amstrad_reset_machine();
	void kccomp_reset_machine();
	void update_psg();
	void amstrad_common_init();
	void enumerate_roms();
	static uint8_t kccomp_get_colour_element(int colour_value);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	int m_centronics_busy;
	uint8_t m_last_write;
};


/*----------- defined in machine/amstrad.c -----------*/


void cpc_exp_cards(device_slot_interface &device);
void cpcplus_exp_cards(device_slot_interface &device);

#endif // MAME_INCLUDES_AMSTRAD_H
