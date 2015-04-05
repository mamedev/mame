// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ABC1600__
#define __ABC1600__

#include "bus/abcbus/abcbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/8530scc.h"
#include "bus/abckb/abckb.h"
#include "machine/abc1600mac.h"
#include "machine/e0516.h"
#include "machine/nmc9306.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z8536.h"
#include "video/abc1600.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MC68008P8_TAG       "3f"
#define Z8410AB1_0_TAG      "5g"
#define Z8410AB1_1_TAG      "7g"
#define Z8410AB1_2_TAG      "9g"
#define Z8470AB1_TAG        "17b"
#define Z8530B1_TAG         "2a"
#define Z8536B1_TAG         "15b"
#define SAB1797_02P_TAG     "5a"
#define FDC9229BT_TAG       "7a"
#define E050_C16PC_TAG      "13b"
#define NMC9306_TAG         "14c"
#define SCREEN_TAG          "screen"
#define BUS0I_TAG           "bus0i"
#define BUS0X_TAG           "bus0x"
#define BUS1_TAG            "bus1"
#define BUS2_TAG            "bus2"
#define RS232_A_TAG         "rs232a"
#define RS232_B_TAG         "rs232b"
#define ABC_KEYBOARD_PORT_TAG   "kb"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc1600_state

class abc1600_state : public driver_device
{
public:
	abc1600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, MC68008P8_TAG),
			m_dma0(*this, Z8410AB1_0_TAG),
			m_dma1(*this, Z8410AB1_1_TAG),
			m_dma2(*this, Z8410AB1_2_TAG),
			m_dart(*this, Z8470AB1_TAG),
			m_scc(*this, Z8530B1_TAG),
			m_cio(*this, Z8536B1_TAG),
			m_fdc(*this, SAB1797_02P_TAG),
			m_rtc(*this, E050_C16PC_TAG),
			m_nvram(*this, NMC9306_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, SAB1797_02P_TAG":0"),
			m_floppy1(*this, SAB1797_02P_TAG":1"),
			m_floppy2(*this, SAB1797_02P_TAG":2"),
			m_bus0i(*this, BUS0I_TAG),
			m_bus0x(*this, BUS0X_TAG),
			m_bus1(*this, BUS1_TAG),
			m_bus2(*this, BUS2_TAG)
	{ }

	required_device<m68000_base_device> m_maincpu;
	required_device<z80dma_device> m_dma0;
	required_device<z80dma_device> m_dma1;
	required_device<z80dma_device> m_dma2;
	required_device<z80dart_device> m_dart;
	required_device<scc8530_t> m_scc;
	required_device<z8536_device> m_cio;
	required_device<fd1797_t> m_fdc;
	required_device<e0516_device> m_rtc;
	required_device<nmc9306_device> m_nvram;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<abcbus_slot_t> m_bus0i;
	required_device<abcbus_slot_t> m_bus0x;
	required_device<abcbus_slot_t> m_bus1;
	required_device<abcbus_slot_t> m_bus2;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( bus_r );
	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_READ8_MEMBER( dart_r );
	DECLARE_WRITE8_MEMBER( dart_w );
	DECLARE_READ8_MEMBER( scc_r );
	DECLARE_WRITE8_MEMBER( scc_w );
	DECLARE_READ8_MEMBER( cio_r );
	DECLARE_WRITE8_MEMBER( cio_w );
	DECLARE_WRITE8_MEMBER( fw0_w );
	DECLARE_WRITE8_MEMBER( fw1_w );
	DECLARE_WRITE8_MEMBER( spec_contr_reg_w );

	DECLARE_WRITE_LINE_MEMBER( dbrq_w );

	DECLARE_READ8_MEMBER( cio_pa_r );
	DECLARE_READ8_MEMBER( cio_pb_r );
	DECLARE_WRITE8_MEMBER( cio_pb_w );
	DECLARE_READ8_MEMBER( cio_pc_r );
	DECLARE_WRITE8_MEMBER( cio_pc_w );

	DECLARE_WRITE_LINE_MEMBER( nmi_w );

	IRQ_CALLBACK_MEMBER( abc1600_int_ack );

	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	UINT8 read_io(offs_t offset);
	void write_io(offs_t offset, UINT8 data);
	UINT8 read_internal_io(offs_t offset);
	void write_internal_io(offs_t offset, UINT8 data);
	UINT8 read_external_io(offs_t offset);
	void write_external_io(offs_t offset, UINT8 data);

	void update_drdy0();
	void update_drdy1();
	void update_drdy2();

	// DMA
	int m_dmadis;
	int m_sysscc;
	int m_sysfs;
	UINT8 m_cause;
	int m_partst;               // parity test

	// peripherals
	int m_cs7;                  // card select address bit 7
	int m_bus0;                 // BUS 0 selected
	UINT8 m_csb;                // card select
	int m_atce;                 // V.24 channel A external clock enable
	int m_btce;                 // V.24 channel B external clock enable
};



#endif
