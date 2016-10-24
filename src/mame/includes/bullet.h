// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __BULLET__
#define __BULLET__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/centronics/ctronics.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"

#define Z80_TAG         "u20"
#define Z80CTC_TAG      "u1"
#define Z80DMA_TAG      "u50"
#define Z80DART_TAG     "u45"
#define Z80PIO_TAG      "z80pio"
#define MB8877_TAG      "u55"
#define CENTRONICS_TAG  "centronics"
#define SCSIBUS_TAG     "scsi"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class bullet_state : public driver_device
{
public:
	bullet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_dart(*this, Z80DART_TAG),
		m_dmac(*this, Z80DMA_TAG),
		m_fdc(*this, MB8877_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, MB8877_TAG":0"),
		m_floppy1(*this, MB8877_TAG":1"),
		m_floppy2(*this, MB8877_TAG":2"),
		m_floppy3(*this, MB8877_TAG":3"),
		m_floppy4(*this, MB8877_TAG":4"),
		m_floppy5(*this, MB8877_TAG":5"),
		m_floppy6(*this, MB8877_TAG":6"),
		m_floppy7(*this, MB8877_TAG":7"),
		m_floppy(nullptr),
		m_centronics(*this, CENTRONICS_TAG),
		m_rom(*this, Z80_TAG),
		m_sw1(*this, "SW1"),
		m_fdrdy(0),
		m_exdsk_sw(false),
		m_hdcon_sw(false)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dmac;
	required_device<mb8877_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<floppy_connector> m_floppy4;
	required_device<floppy_connector> m_floppy5;
	required_device<floppy_connector> m_floppy6;
	required_device<floppy_connector> m_floppy7;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;
	required_memory_region m_rom;
	required_ioport m_sw1;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t info_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t brom_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void brom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t win_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wstrobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exdsk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void exdma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hdcon_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void segst_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dma_mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dartardy_w(int state);
	void dartbrdy_w(int state);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);
	void write_centronics_select(int state);
	void write_centronics_fault(int state);
	void fdc_drq_w(int state);

	void update_dma_rdy();
	// memory state
	int m_segst;
	int m_brom;

	// DMA state
	uint8_t m_exdma;
	int m_buf;
	bool m_fdrdy;
	int m_dartardy;
	int m_dartbrdy;
	int m_winrdy;
	int m_exrdy1;
	int m_exrdy2;
	bool m_exdsk_sw;
	bool m_hdcon_sw;

	int m_centronics_busy;
	int m_centronics_perror;
	int m_centronics_select;
	int m_centronics_fault;

	void ctc_tick(timer_device &timer, void *ptr, int32_t param);
	void dart_rxtxca_w(int state);
	uint8_t io_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};

class bulletf_state : public bullet_state
{
public:
	bulletf_state(const machine_config &mconfig, device_type type, const char *tag) :
		bullet_state(mconfig, type, tag),
		m_floppy8(*this, MB8877_TAG":8"),
		m_floppy9(*this, MB8877_TAG":9"),
		m_scsibus(*this, SCSIBUS_TAG),
		m_scsi_data_in(*this, "scsi_data_in"),
		m_scsi_data_out(*this, "scsi_data_out"),
		m_scsi_ctrl_in(*this, "scsi_ctrl_in")
	{
	}

	required_device<floppy_connector> m_floppy8;
	required_device<floppy_connector> m_floppy9;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_ctrl_in;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xdma0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xfdc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hwsts_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t scsi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scsi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t dma_mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pio_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cstrb_w(int state);
	void req_w(int state);

	void update_dma_rdy();

	int m_rome;
	uint8_t m_xdma0;
	uint8_t m_mbank;
	int m_wack;
	int m_wrdy;
};

#endif
