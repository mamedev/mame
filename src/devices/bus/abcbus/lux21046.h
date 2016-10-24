// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21046-11/-21/-41 5.25"/8" Controller Card emulation

*********************************************************************/

#pragma once

#ifndef __LUXOR_55_21046__
#define __LUXOR_55_21046__

#include "emu.h"
#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/abc800_dsk.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ADDRESS_ABC832          44
#define ADDRESS_ABC830          45
#define ADDRESS_ABC838          46


#define DRIVE_TEAC_FD55F        0x01
#define DRIVE_BASF_6138         0x02
#define DRIVE_MICROPOLIS_1015F  0x03
#define DRIVE_BASF_6118         0x04
#define DRIVE_MICROPOLIS_1115F  0x05
#define DRIVE_BASF_6106_08      0x08
#define DRIVE_MPI_51            0x09
#define DRIVE_BASF_6105         0x0e
#define DRIVE_BASF_6106         0x0f



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> luxor_55_21046_device

class luxor_55_21046_device :  public device_t,
								public device_abcbus_card_interface
{
public:
	// construction/destruction
	luxor_55_21046_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	luxor_55_21046_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	uint8_t out_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void inp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _4b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _9b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _8a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t _9a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void dma_int_w(int state);

	uint8_t memory_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void memory_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t io_read_byte(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void io_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void fdc_intrq_w(int state);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual int abcbus_csb() override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80dma_device> m_dma;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_ioport m_sw1;
	required_ioport m_sw2;
	required_ioport m_sw3;

	bool m_cs;                  // card selected
	uint8_t m_status;             // ABC BUS status
	uint8_t m_out;                // ABC BUS data in
	uint8_t m_inp;                // ABC BUS data out
	bool m_fdc_irq;             // FDC interrupt
	int m_dma_irq;              // DMA interrupt
	int m_busy;                 // busy bit
	int m_force_busy;           // force busy bit
};


// ======================> abc830_device

class abc830_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc830_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> abc832_device

class abc832_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> abc834_device

class abc834_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc834_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> abc838_device

class abc838_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc838_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// ======================> abc850_floppy_device

class abc850_floppy_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc850_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type LUXOR_55_21046;
extern const device_type ABC830;
extern const device_type ABC832;
extern const device_type ABC834;
extern const device_type ABC838;
extern const device_type ABC850_FLOPPY;



#endif
