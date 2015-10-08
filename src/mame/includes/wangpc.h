// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __WANGPC__
#define __WANGPC__

#include "emu.h"
#include "bus/wangpc/wangpc.h"
#include "cpu/i86/i86.h"
#include "formats/pc_dsk.h"
#include "machine/am9517a.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/im6402.h"
#include "machine/mc2661.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/wangpckb.h"

#define I8086_TAG       "i8086"
#define AM9517A_TAG     "am9517a"
#define I8259A_TAG      "i8259"
#define I8255A_TAG      "i8255a"
#define I8253_TAG       "i8253"
#define IM6402_TAG      "im6402"
#define SCN2661_TAG     "scn2661"
#define UPD765_TAG      "upd765"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"
#define WANGPC_KEYBOARD_TAG "wangpckb"

class wangpc_state : public driver_device
{
public:
	// constructor
	wangpc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8086_TAG),
		m_dmac(*this, AM9517A_TAG),
		m_pic(*this, I8259A_TAG),
		m_ppi(*this, I8255A_TAG),
		m_pit(*this, I8253_TAG),
		m_uart(*this, IM6402_TAG),
		m_epci(*this, SCN2661_TAG),
		m_fdc(*this, UPD765_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, UPD765_TAG ":0:525dd"),
		m_floppy1(*this, UPD765_TAG ":1:525dd"),
		m_centronics(*this, CENTRONICS_TAG),
		m_cent_data_in(*this, "cent_data_in"),
		m_cent_data_out(*this, "cent_data_out"),
		m_bus(*this, WANGPC_BUS_TAG),
		m_sw(*this, "SW"),
		m_timer2_irq(1),
		m_centronics_ack(1),
		m_dav(1),
		m_dma_eop(1),
		m_uart_dr(0),
		m_uart_tbre(0),
		m_fpu_irq(0),
		m_bus_irq2(0),
		m_enable_eop(0),
		m_disable_dreq2(0),
		m_fdc_drq(0),
		m_fdc_dd0(0),
		m_fdc_dd1(0),
		m_fdc_tc(0),
		m_ds1(false),
		m_ds2(false)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<i8255_device> m_ppi;
	required_device<pit8253_device> m_pit;
	required_device<im6402_device> m_uart;
	required_device<mc2661_device> m_epci;
	required_device<upd765a_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	required_device<wangpcbus_device> m_bus;
	required_ioport m_sw;

	virtual void machine_start();
	virtual void machine_reset();

	void select_drive();
	void check_level1_interrupts();
	void check_level2_interrupts();
	void update_fdc_drq();
	void update_fdc_tc();

	DECLARE_WRITE8_MEMBER( fdc_ctrl_w );
	DECLARE_READ8_MEMBER( deselect_drive1_r );
	DECLARE_WRITE8_MEMBER( deselect_drive1_w );
	DECLARE_READ8_MEMBER( select_drive1_r );
	DECLARE_WRITE8_MEMBER( select_drive1_w );
	DECLARE_READ8_MEMBER( deselect_drive2_r );
	DECLARE_WRITE8_MEMBER( deselect_drive2_w );
	DECLARE_READ8_MEMBER( select_drive2_r );
	DECLARE_WRITE8_MEMBER( select_drive2_w );
	DECLARE_READ8_MEMBER( motor1_off_r );
	DECLARE_WRITE8_MEMBER( motor1_off_w );
	DECLARE_READ8_MEMBER( motor1_on_r );
	DECLARE_WRITE8_MEMBER( motor1_on_w );
	DECLARE_READ8_MEMBER( motor2_off_r );
	DECLARE_WRITE8_MEMBER( motor2_off_w );
	DECLARE_READ8_MEMBER( motor2_on_r );
	DECLARE_WRITE8_MEMBER( motor2_on_w );
	DECLARE_READ8_MEMBER( fdc_reset_r );
	DECLARE_WRITE8_MEMBER( fdc_reset_w );
	DECLARE_READ8_MEMBER( fdc_tc_r );
	DECLARE_WRITE8_MEMBER( fdc_tc_w );
	DECLARE_WRITE8_MEMBER( dma_page_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( timer0_irq_clr_w );
	DECLARE_READ8_MEMBER( timer2_irq_clr_r );
	DECLARE_WRITE8_MEMBER( nmi_mask_w );
	DECLARE_READ8_MEMBER( led_on_r );
	DECLARE_WRITE8_MEMBER( fpu_mask_w );
	DECLARE_READ8_MEMBER( dma_eop_clr_r );
	DECLARE_WRITE8_MEMBER( uart_tbre_clr_w );
	DECLARE_READ8_MEMBER( uart_r );
	DECLARE_WRITE8_MEMBER( uart_w );
	DECLARE_READ8_MEMBER( centronics_r );
	DECLARE_WRITE8_MEMBER( centronics_w );
	DECLARE_READ8_MEMBER( busy_clr_r );
	DECLARE_WRITE8_MEMBER( acknlg_clr_w );
	DECLARE_READ8_MEMBER( led_off_r );
	DECLARE_WRITE8_MEMBER( parity_nmi_clr_w );
	DECLARE_READ8_MEMBER( option_id_r );

	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	DECLARE_WRITE_LINE_MEMBER( eop_w );
	DECLARE_READ8_MEMBER( memr_r );
	DECLARE_WRITE8_MEMBER( memw_w );
	DECLARE_READ8_MEMBER( ior2_r );
	DECLARE_WRITE8_MEMBER( iow2_w );
	DECLARE_WRITE_LINE_MEMBER( dack0_w );
	DECLARE_WRITE_LINE_MEMBER( dack1_w );
	DECLARE_WRITE_LINE_MEMBER( dack2_w );
	DECLARE_WRITE_LINE_MEMBER( dack3_w );
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_READ8_MEMBER( ppi_pc_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_WRITE_LINE_MEMBER( pit0_w );
	DECLARE_WRITE_LINE_MEMBER( pit2_w );
	DECLARE_WRITE_LINE_MEMBER( uart_dr_w );
	DECLARE_WRITE_LINE_MEMBER( uart_tbre_w );
	DECLARE_WRITE_LINE_MEMBER( epci_irq_w );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_ack );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_fault );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_perror );
	DECLARE_WRITE_LINE_MEMBER( bus_irq2_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE_LINE_MEMBER( fdc_irq );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq );

	int on_disk0_load(floppy_image_device *image);
	void on_disk0_unload(floppy_image_device *image);
	int on_disk1_load(floppy_image_device *image);
	void on_disk1_unload(floppy_image_device *image);

	UINT8 m_dma_page[4];
	int m_dack;

	int m_timer2_irq;
	int m_centronics_ack;
	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_dav;
	int m_dma_eop;
	int m_uart_dr;
	int m_uart_tbre;
	int m_fpu_irq;
	int m_bus_irq2;

	int m_enable_eop;
	int m_disable_dreq2;
	int m_fdc_drq;
	int m_fdc_dd0;
	int m_fdc_dd1;
	int m_fdc_tc;
	int m_ds1;
	int m_ds2;

	int m_led[6];
};


#endif
