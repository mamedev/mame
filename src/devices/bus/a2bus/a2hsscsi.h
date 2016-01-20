// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2hsscsi.h

    Implementation of the Apple II High Speed SCSI Card

*********************************************************************/

#ifndef __A2BUS_HSSCSI__
#define __A2BUS_HSSCSI__

#include "emu.h"
#include "a2bus.h"
#include "machine/ncr5380n.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_hsscsi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_hsscsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_hsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	required_device<ncr5380n_device> m_ncr5380;
	required_device<nscsi_bus_device> m_scsibus;

	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual void write_cnxx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;
	virtual void write_c800(address_space &space, UINT16 offset, UINT8 data) override;

private:
	UINT8 *m_rom;
	UINT8 m_ram[8192];  // 8 banks of 1024 bytes
	int m_rambank, m_rombank;
	UINT8 m_drq;
	UINT8 m_bank;
	bool m_816block;
	UINT8 m_c0ne, m_c0nf;
};

// device type definition
extern const device_type A2BUS_HSSCSI;

#endif /* __A2BUS_HSSCSI__ */
