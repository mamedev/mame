// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21046-11/-21/-41 5.25"/8" Controller Card emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_LUX21046_H
#define MAME_BUS_ABCBUS_LUX21046_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/floppy.h"
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
	luxor_55_21046_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	luxor_55_21046_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual int abcbus_csb() override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual void abcbus_c4(uint8_t data) override;

	static void floppy_formats(format_registration &fr);

	required_device_array<floppy_connector, 2> m_floppy;

private:
	void dma_int_w(int state);

	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	void fdc_intrq_w(int state);

	uint8_t out_r();
	void inp_w(uint8_t data);
	void _4b_w(uint8_t data);
	void _9b_w(uint8_t data);
	void _8a_w(uint8_t data);
	uint8_t _9a_r(offs_t offset);

	void luxor_55_21046_io(address_map &map) ATTR_COLD;
	void luxor_55_21046_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80dma_device> m_dma;
	required_device<fd1793_device> m_fdc;
	floppy_image_device *m_selected_floppy;
	required_ioport m_sw1;
	required_ioport m_sw2;
	required_ioport m_sw3;

	bool m_cs;                  // card selected
	uint8_t m_status;           // ABC BUS status
	uint8_t m_out;              // ABC BUS data in
	uint8_t m_inp;              // ABC BUS data out
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

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> abc832_device

class abc832_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> abc834_device

class abc834_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc834_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> abc838_device

class abc838_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc838_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> abc850_floppy_device

class abc850_floppy_device :  public luxor_55_21046_device
{
public:
	// construction/destruction
	abc850_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(LUXOR_55_21046, luxor_55_21046_device)
DECLARE_DEVICE_TYPE(ABC830,         abc830_device)
DECLARE_DEVICE_TYPE(ABC832,         abc832_device)
DECLARE_DEVICE_TYPE(ABC834,         abc834_device)
DECLARE_DEVICE_TYPE(ABC838,         abc838_device)
DECLARE_DEVICE_TYPE(ABC850_FLOPPY,  abc850_floppy_device)



#endif // MAME_BUS_ABCBUS_LUX21046_H
