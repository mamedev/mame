// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Common printer interfaces

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_LPRINT_H
#define MAME_BUS_SPECTRUM_LPRINT_H

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_lprint_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_lprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual void pre_data_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;

	int m_romcs;
	int m_busy;
};

class spectrum_lprint3_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_lprint3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }

	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<spectrum_expansion_slot_device> m_exp;

	int m_romcs;
	int m_busy;
};

class spectrum_kempcentrs_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_kempcentrs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_device<centronics_device> m_centronics;

	int m_busy;
};

class spectrum_kempcentre_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_kempcentre_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	spectrum_kempcentre_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual void pre_data_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;

	int m_active;
	int m_romcs;
	int m_busy;
};

class spectrum_kempcentreu_device :
	public spectrum_kempcentre_device
{
public:
	// construction/destruction
	spectrum_kempcentreu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_LPRINT, spectrum_lprint_device)
DECLARE_DEVICE_TYPE(SPECTRUM_LPRINT3, spectrum_lprint3_device)
DECLARE_DEVICE_TYPE(SPECTRUM_KEMPCENTRS, spectrum_kempcentrs_device)
DECLARE_DEVICE_TYPE(SPECTRUM_KEMPCENTREF, spectrum_kempcentre_device)
DECLARE_DEVICE_TYPE(SPECTRUM_KEMPCENTREU, spectrum_kempcentreu_device)


#endif // MAME_BUS_SPECTRUM_LPRINT_H
