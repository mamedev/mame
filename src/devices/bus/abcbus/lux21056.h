// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21056-00 Xebec Interface Host Adapter emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_LUX21056_H
#define MAME_BUS_ABCBUS_LUX21056_H

#pragma once

#include "abcbus.h"
#include "bus/scsi/scsi.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
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
	luxor_55_21056_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;

private:
	void set_rdy(int state);

	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	void write_sasi_req(int state);
	void write_sasi_io(int state);
	void write_sasi_cd(int state);
	void write_sasi_msg(int state);
	void write_sasi_bsy(int state);

	uint8_t sasi_status_r();
	void stat_w(uint8_t data);
	uint8_t out_r();
	void inp_w(uint8_t data);
	uint8_t sasi_data_r();
	void sasi_data_w(uint8_t data);
	uint8_t rdy_reset_r();
	void rdy_reset_w(uint8_t data);
	uint8_t sasi_sel_r();
	void sasi_sel_w(uint8_t data);
	uint8_t sasi_rst_r();
	void sasi_rst_w(uint8_t data);

	void luxor_55_21056_io(address_map &map) ATTR_COLD;
	void luxor_55_21056_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80dma_device> m_dma;
	required_device<scsi_port_device> m_sasibus;
	required_device<output_latch_device> m_sasi_data_out;
	required_device<input_buffer_device> m_sasi_data_in;
	required_ioport m_s1;

	int m_cs;
	int m_rdy;
	int m_req;
	int m_sasi_req;
	int m_sasi_io;
	int m_sasi_cd;
	int m_sasi_msg;
	int m_sasi_bsy;

	uint8_t m_inp;
	uint8_t m_out;
	uint8_t m_stat;
	uint8_t m_sasi_data;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_55_21056, luxor_55_21056_device)

#endif // MAME_BUS_ABCBUS_LUX21056_H
