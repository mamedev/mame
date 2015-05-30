// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __NEWBRAIN__
#define __NEWBRAIN__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/cop400/cop400.h"
#include "imagedev/cassette.h"
#include "machine/6850acia.h"
#include "machine/adc0808.h"
#include "machine/z80ctc.h"
#include "machine/rescap.h"
#include "machine/ram.h"
#include "machine/upd765.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "409"
#define COP420_TAG      "419"
#define MC6850_TAG      "459"
#define ADC0809_TAG     "427"
#define DAC0808_TAG     "461"
#define Z80CTC_TAG      "458"
#define FDC_Z80_TAG     "416"
#define UPD765_TAG      "418"

#define NEWBRAIN_EIM_RAM_SIZE           0x10000

#define NEWBRAIN_ENRG1_CLK              0x01
#define NEWBRAIN_ENRG1_TVP              0x04
#define NEWBRAIN_ENRG1_CTS              0x10
#define NEWBRAIN_ENRG1_DO               0x20
#define NEWBRAIN_ENRG1_PO               0x80
#define NEWBRAIN_ENRG1_UST_BIT_1_MASK   0x30
#define NEWBRAIN_ENRG1_UST_BIT_0_MASK   0xc0

#define NEWBRAIN_ENRG2_USERP            0x01
#define NEWBRAIN_ENRG2_ANP              0x02
#define NEWBRAIN_ENRG2_MLTMD            0x04
#define NEWBRAIN_ENRG2_MSPD             0x08
#define NEWBRAIN_ENRG2_ENOR             0x10
#define NEWBRAIN_ENRG2_ANSW             0x20
#define NEWBRAIN_ENRG2_ENOT             0x40
#define NEWBRAIN_ENRG2_CENTRONICS_OUT   0x80

#define NEWBRAIN_VIDEO_RV               0x01
#define NEWBRAIN_VIDEO_FS               0x02
#define NEWBRAIN_VIDEO_32_40            0x04
#define NEWBRAIN_VIDEO_UCR              0x08
#define NEWBRAIN_VIDEO_80L              0x40

class newbrain_state : public driver_device
{
public:
	newbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_copcpu(*this, COP420_TAG),
		m_palette(*this, "palette"),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2"),
		m_rom(*this, Z80_TAG),
		m_eim_rom(*this, "eim"),
		m_char_rom(*this, "chargen"),
		m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_y8(*this, "Y8"),
		m_y9(*this, "Y9"),
		m_y10(*this, "Y10"),
		m_y11(*this, "Y11"),
		m_y12(*this, "Y12"),
		m_y13(*this, "Y13"),
		m_y14(*this, "Y14"),
		m_y15(*this, "Y15")
	{
	}

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( enrg1_w );
	DECLARE_WRITE8_MEMBER( a_enrg1_w );
	DECLARE_READ8_MEMBER( ust_r );
	DECLARE_READ8_MEMBER( a_ust_r );
	DECLARE_READ8_MEMBER( user_r );
	DECLARE_WRITE8_MEMBER( user_w );
	DECLARE_READ8_MEMBER( clclk_r );
	DECLARE_WRITE8_MEMBER( clclk_w );
	DECLARE_READ8_MEMBER( clusr_r );
	DECLARE_WRITE8_MEMBER( clusr_w );
	DECLARE_READ8_MEMBER( cop_l_r );
	DECLARE_WRITE8_MEMBER( cop_l_w );
	DECLARE_WRITE8_MEMBER( cop_g_w );
	DECLARE_READ8_MEMBER( cop_g_r );
	DECLARE_WRITE8_MEMBER( cop_d_w );
	DECLARE_READ8_MEMBER( cop_in_r );
	DECLARE_WRITE_LINE_MEMBER( cop_sk_w );
	DECLARE_READ_LINE_MEMBER( cop_si_r );
	DECLARE_WRITE_LINE_MEMBER( cop_so_w );
	DECLARE_READ8_MEMBER( tvl_r );
	DECLARE_WRITE8_MEMBER( tvl_w );
	DECLARE_WRITE8_MEMBER( tvctl_w );
	DECLARE_READ8_MEMBER( cop_r );
	DECLARE_WRITE8_MEMBER( cop_w );

	INTERRUPT_GEN_MEMBER(newbrain_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cop_regint_tick);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

	enum
	{
		TIMER_ID_RESET,
		TIMER_ID_PWRUP
	};

	void check_interrupt();
	void bankswitch();
	void tvram_w(UINT8 data, int a6);
	inline int get_reset_t();
	inline int get_pwrup_t();
	void screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_copcpu;
	required_device<palette_device> m_palette;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_memory_region m_rom;
	optional_memory_region m_eim_rom;
	required_memory_region m_char_rom;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;
	required_ioport m_y12;
	required_ioport m_y13;
	required_ioport m_y14;
	required_ioport m_y15;

	// processor state
	int m_pwrup;            // power up
	int m_userint;          // user interrupt
	int m_userint0;         // parallel port interrupt
	int m_clkint;           // clock interrupt
	int m_aciaint;          // ACIA interrupt
	int m_copint;           // COP interrupt
	int m_anint;            // A/DC interrupt
	int m_bee;              // identity
	UINT8 m_enrg1;          // enable register 1
	UINT8 m_enrg2;          // enable register 2
	int m_acia_txd;         // ACIA transmit

	// COP420 state
	UINT8 m_cop_bus;        // data bus
	int m_cop_so;           // serial out
	int m_cop_tdo;          // tape data output
	int m_cop_tdi;          // tape data input
	int m_cop_rd;           // memory read
	int m_cop_wr;           // memory write
	int m_cop_access;       // COP access

	// keyboard state
	ioport_port* m_key_row[16];
	int m_keylatch;         // keyboard row
	int m_keydata;          // keyboard column

	// video state
	int m_segment_data[16]; // VF segment data
	int m_tvcnsl;           // TV console required
	int m_tvctl;            // TV control register
	UINT16 m_tvram;         // TV start address

	// user bus state
	UINT8 m_user;

	// devices
	UINT8 m_copdata;
	int m_copstate;
	int m_copbytes;
	int m_copregint;
};

class newbrain_eim_state : public newbrain_state
{
public:
	newbrain_eim_state(const machine_config &mconfig, device_type type, const char *tag)
		: newbrain_state(mconfig, type, tag),
			m_fdccpu(*this, FDC_Z80_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_acia(*this, MC6850_TAG),
			m_fdc(*this, UPD765_TAG),
			m_floppy(*this, UPD765_TAG ":0:525dd"),
			m_eim_ram(*this, "eim_ram")
	{ }

	required_device<cpu_device> m_fdccpu;
	required_device<z80ctc_device> m_ctc;
	required_device<acia6850_device> m_acia;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_image_device> m_floppy;
	optional_shared_ptr<UINT8> m_eim_ram;

	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( fdc_auxiliary_w );
	DECLARE_READ8_MEMBER( fdc_control_r );
	DECLARE_READ8_MEMBER( ust2_r );
	DECLARE_WRITE8_MEMBER( enrg2_w );
	DECLARE_WRITE8_MEMBER( pr_w );
	DECLARE_READ8_MEMBER( user_r );
	DECLARE_WRITE8_MEMBER( user_w );
	DECLARE_READ8_MEMBER( anout_r );
	DECLARE_WRITE8_MEMBER( anout_w );
	DECLARE_READ8_MEMBER( anin_r );
	DECLARE_WRITE8_MEMBER( anio_w );
	DECLARE_READ8_MEMBER( st0_r );
	DECLARE_READ8_MEMBER( st1_r );
	DECLARE_READ8_MEMBER( st2_r );
	DECLARE_READ8_MEMBER( usbs_r );
	DECLARE_WRITE8_MEMBER( usbs_w );
	DECLARE_WRITE8_MEMBER( paging_w );
	DECLARE_WRITE_LINE_MEMBER( acia_tx );
	DECLARE_WRITE_LINE_MEMBER( acia_interrupt );
	DECLARE_WRITE_LINE_MEMBER( fdc_interrupt );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	DECLARE_WRITE_LINE_MEMBER( adc_eoc_w );

	ADC0808_ANALOG_READ_CB(adc_vref_pos_r);
	ADC0808_ANALOG_READ_CB(adc_vref_neg_r);
	ADC0808_ANALOG_READ_CB(adc_input_r);

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_c2_tick);

	void bankswitch();

	// paging state
	int m_paging;           // paging enabled
	int m_mpm;              // multi paging mode ?
	int m_a16;              // address line 16
	UINT8 m_pr[16];         // expansion interface paging register

	// floppy state
	int m_fdc_int;          // interrupt
	int m_fdc_att;          // attention
};

// ---------- defined in video/newbrain.c ----------

MACHINE_CONFIG_EXTERN( newbrain_video );

#endif
