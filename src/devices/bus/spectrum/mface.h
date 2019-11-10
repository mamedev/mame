// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Romantic Robot Multiface One/128/3

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_MFACE_H
#define MAME_BUS_SPECTRUM_MFACE_H

#include "exp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_mface_base_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_mface_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_INPUT_CHANGED_MEMBER(magic_button);

protected:
	spectrum_mface_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	required_memory_region m_rom;
	required_device<spectrum_expansion_slot_device> m_exp;

	uint8_t m_ram[8 * 1024];
	int m_romcs;
};

class spectrum_mface1_device : public spectrum_mface_base_device
{
public:
	spectrum_mface1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	
	DECLARE_INPUT_CHANGED_MEMBER(magic_button) override;

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	
private:
	required_ioport m_joy;
	required_ioport m_hwconfig;
};

class spectrum_mface128_device : public spectrum_mface_base_device
{
public:
	spectrum_mface128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
};

class spectrum_mface3_device : public spectrum_mface_base_device
{
public:
	spectrum_mface3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
};

class spectrum_mprint_device : public spectrum_mface_base_device
{
public:
	spectrum_mprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1, spectrum_mface1_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE128, spectrum_mface128_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE3, spectrum_mface3_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MPRINT, spectrum_mprint_device)



#endif // MAME_BUS_SPECTRUM_MFACE_H
