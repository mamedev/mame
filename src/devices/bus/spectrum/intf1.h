// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ZX Interface 1

**********************************************************************/


#ifndef MAME_BUS_SPECTRUM_INTF1_H
#define MAME_BUS_SPECTRUM_INTF1_H

#include "exp.h"
#include "bus/rs232/rs232.h"
#include "imagedev/microdrv.h"
#include "softlist.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_intf1_device:
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_intf1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK | feature::LAN; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual void post_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }

private:
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<rs232_port_device> m_rs232;
	required_device<microdrive_image_device> m_mdv1;
	required_device<microdrive_image_device> m_mdv2;
	required_memory_region m_rom;

	int m_romcs;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_INTF1, spectrum_intf1_device)


#endif /* MAME_BUS_SPECTRUM_INTF1_H */
