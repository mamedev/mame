// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    Cumulus CuRAM-16 Plus Memory Multifunction Adapter

***************************************************************************/

#ifndef MAME_BUS_MCA_CURAM_H
#define MAME_BUS_MCA_CURAM_H

#pragma once

#include "mca.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_curam16_plus_device

class mca16_curam16_plus_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_curam16_plus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    virtual uint16_t get_card_id() override { return 0x6025; }

	virtual void remap() override;
	virtual void unmap() override;
	
	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

	virtual uint8_t pos_r(offs_t offset) override;
	virtual void pos_w(offs_t offset, uint8_t data) override;

    virtual void update_pos_data_1(uint8_t data) override;
    virtual void update_pos_data_2(uint8_t data) override;
    virtual void update_pos_data_3(uint8_t data) override;
	virtual void update_pos_data_4(uint8_t data) override;

	uint8_t ems_200_r(offs_t offset);
	uint8_t ems_4200_r(offs_t offset);
	uint8_t ems_8200_r(offs_t offset);
	uint8_t ems_c200_r(offs_t offset);

	void ems_200_w(offs_t offset, uint8_t data);
	void ems_4200_w(offs_t offset, uint8_t data);
	void ems_8200_w(offs_t offset, uint8_t data);
	void ems_c200_w(offs_t offset, uint8_t data);

	uint8_t ems_r(offs_t offset);
	void ems_w(offs_t offset, uint8_t data);

protected:
	mca16_curam16_plus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual bool map_has_changed() override;
	
	required_device<ram_device> m_ram;
private:
    offs_t  calculate_xms_start_address();
    offs_t  calculate_xms_end_address();

	offs_t	calculate_ems_ports();

	bool 	m_is_mapped;

    offs_t  m_xms_start, m_xms_end;

	uint8_t m_ems_regs_200h[16];
	uint8_t m_ems_regs_4200h[16];
	uint8_t m_ems_regs_8200h[16];
	uint8_t m_ems_regs_c200h[16];
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_CURAM16_PLUS, mca16_curam16_plus_device)

#endif