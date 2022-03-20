// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Romantic Robot Multiface One/128/3

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_MFACE_H
#define MAME_BUS_SPECTRUM_MFACE_H

#include "exp.h"
#include "bus/centronics/ctronics.h"

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

	virtual DECLARE_INPUT_CHANGED_MEMBER(magic_button) { m_slot->nmi_w(newval ? CLEAR_LINE : ASSERT_LINE); }

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
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual uint8_t iorq_r(offs_t offset) override { return m_exp->iorq_r(offset); }
	virtual void iorq_w(offs_t offset, uint8_t data) override { m_exp->iorq_w(offset, data); }

	void nmi(line_state state) { m_slot->nmi_w(state); m_nmi_pending = state; }

	required_memory_region m_rom;
	required_device<spectrum_expansion_slot_device> m_exp;

	int m_romcs;
	int m_nmi_pending;
	std::unique_ptr<uint8_t[]> m_ram;
};

class spectrum_mface1v2_device : public spectrum_mface_base_device
{
public:
	spectrum_mface1v2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(magic_button) override;

protected:
	spectrum_mface1v2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	required_ioport m_joy;
	required_ioport m_hwconfig;
};

class spectrum_mface1v1_device : public spectrum_mface1v2_device
{
public:
	spectrum_mface1v1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
};

class spectrum_mface1v3_device : public spectrum_mface1v2_device
{
public:
	spectrum_mface1v3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	spectrum_mface1v3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
};

class spectrum_mface1_device : public spectrum_mface1v3_device
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
};

class spectrum_mface128_base_device : public spectrum_mface_base_device
{
public:
	spectrum_mface128_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(magic_button) override;

protected:
	spectrum_mface128_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_hidden;
};

class spectrum_mface128v1_device : public spectrum_mface128_base_device
{
public:
	spectrum_mface128v1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	spectrum_mface128v1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	int m_d3_ff;  // no initial state
};

class spectrum_mface128_device : public spectrum_mface128v1_device
{
public:
	spectrum_mface128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
};

class spectrum_mface3_device : public spectrum_mface128_base_device
{
public:
	spectrum_mface3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(magic_button) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	uint8_t m_reg_file[4];  // no initial state
	int m_disable;
};

class spectrum_mprint_device : public spectrum_mface128_base_device
{
public:
	spectrum_mprint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_device<centronics_device> m_centronics;

	int m_busy;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1V1, spectrum_mface1v1_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1V2, spectrum_mface1v2_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1V3, spectrum_mface1v3_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE1, spectrum_mface1_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE128V1, spectrum_mface128v1_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE128, spectrum_mface128_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MFACE3, spectrum_mface3_device)
DECLARE_DEVICE_TYPE(SPECTRUM_MPRINT, spectrum_mprint_device)



#endif // MAME_BUS_SPECTRUM_MFACE_H
