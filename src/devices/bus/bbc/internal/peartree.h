// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Peartree MRxx00 ROM/RAM Boards

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Peartree_ROMExpansion.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_PEARTREE_H
#define MAME_BUS_BBC_INTERNAL_PEARTREE_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_mr3000_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_mr3000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_mr3000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	optional_device_array<bbc_romslot_device, 16> m_rom;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

	uint8_t m_romsel;
};


class bbc_mr4200_device : public bbc_mr3000_device
{
public:
	// construction/destruction
	bbc_mr4200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport m_wp;

	virtual void paged_w(offs_t offset, uint8_t data) override;
};


class bbc_mr4300_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_mr4300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_mr4300_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	optional_device_array<bbc_romslot_device, 16> m_rom;
	required_ioport m_wp;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

	uint8_t m_romsel;
};


class bbc_mr4800_device : public bbc_mr4300_device
{
public:
	// construction/destruction
	bbc_mr4800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_MR3000, bbc_mr3000_device);
DECLARE_DEVICE_TYPE(BBC_MR4200, bbc_mr4200_device);
DECLARE_DEVICE_TYPE(BBC_MR4300, bbc_mr4300_device);
DECLARE_DEVICE_TYPE(BBC_MR4800, bbc_mr4800_device);


#endif /* MAME_BUS_BBC_INTERNAL_PEARTREE_H */
