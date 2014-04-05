// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21056-00 Xebec Interface Host Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __LUXOR_55_21056__
#define __LUXOR_55_21056__

#include "emu.h"
#include "abcbus.h"
#include "bus/scsi/s1410.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/scsibus.h"
#include "machine/scsicb.h"
#include "machine/scsihd.h"
#include "machine/z80dma.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_55_21056_device

class luxor_55_21056_device :  public device_t,
								public device_abcbus_card_interface
{
public:
	// construction/destruction
	luxor_55_21056_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( sasi_status_r );
	DECLARE_WRITE8_MEMBER( stat_w );
	DECLARE_READ8_MEMBER( out_r );
	DECLARE_WRITE8_MEMBER( inp_w );
	DECLARE_READ8_MEMBER( sasi_data_r );
	DECLARE_WRITE8_MEMBER( sasi_data_w );
	DECLARE_READ8_MEMBER( rdy_reset_r );
	DECLARE_WRITE8_MEMBER( rdy_reset_w );
	DECLARE_READ8_MEMBER( sasi_sel_r );
	DECLARE_WRITE8_MEMBER( sasi_sel_w );
	DECLARE_READ8_MEMBER( sasi_rst_r );
	DECLARE_WRITE8_MEMBER( sasi_rst_w );

	DECLARE_READ8_MEMBER( memory_read_byte );
	DECLARE_WRITE8_MEMBER( memory_write_byte );
	DECLARE_READ8_MEMBER( io_read_byte );
	DECLARE_WRITE8_MEMBER( io_write_byte );

	DECLARE_WRITE_LINE_MEMBER( sasi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( sasi_io_w );
	DECLARE_WRITE_LINE_MEMBER( sasi_req_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);
	virtual UINT8 abcbus_inp();
	virtual void abcbus_out(UINT8 data);
	virtual UINT8 abcbus_stat();
	virtual void abcbus_c1(UINT8 data);
	virtual void abcbus_c3(UINT8 data);

private:
	void set_rdy(int state);

	required_device<cpu_device> m_maincpu;
	required_device<z80dma_device> m_dma;
	required_device<scsicb_device> m_sasibus;
	required_ioport m_s1;

	int m_cs;
	int m_rdy;
	int m_req;

	UINT8 m_inp;
	UINT8 m_out;
	UINT8 m_stat;
	UINT8 m_sasi_data;
};


// device type definition
extern const device_type LUXOR_55_21056;



#endif
