// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    Custom Computer Systems C1000 IDE Adapter

    A whole bunch of TTL, an option ROM, and an IDE port.

***************************************************************************/

#ifndef MAME_BUS_MCA_C1000_H
#define MAME_BUS_MCA_C1000_H

#pragma once

#include "mca.h"
#include "machine/idectrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_com_device

class mca16_c1000_ide_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_c1000_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t get_card_id() override { return 0x6213; }

protected:
	mca16_c1000_ide_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

	uint8_t alt_r(offs_t offset);
	void alt_w(offs_t offset, uint8_t data);

	virtual void pos_w(offs_t offset, uint8_t data) override;

	virtual void unmap() override;
	virtual void remap() override {};

    virtual const tiny_rom_entry *device_rom_region() const override;
    
private:
    DECLARE_WRITE_LINE_MEMBER(ide_interrupt);

    void update_pos_data_1(uint8_t data) override;
    void update_pos_data_2(uint8_t data) override;
    void update_pos_data_3(uint8_t data) override;

	void map(address_map &map);    

	offs_t rom_base();

    bool m_is_primary;
	uint8_t m_is_mapped;

    required_device<ata_interface_device> m_ata;

    offs_t  m_cur_io_start, m_cur_io_end;
    uint8_t m_cur_irq;
    uint16_t m_ata_data_latch;
    bool    m_is_latched;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_C1000_IDE, mca16_c1000_ide_device)

#endif