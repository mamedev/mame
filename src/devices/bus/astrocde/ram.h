// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ASTROCADE_RAM_H
#define MAME_BUS_ASTROCADE_RAM_H

#pragma once

#include "exp.h"


// ======================> astrocade_blueram_4k_device

class astrocade_blueram_4k_device : public device_t, public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_blueram_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	astrocade_blueram_4k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override { m_ram.resize(0x1000); save_item(NAME(m_ram)); }
	virtual void device_reset() override { }

	std::vector<uint8_t> m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_blueram_16k_device

class astrocade_blueram_16k_device : public astrocade_blueram_4k_device
{
public:
	// construction/destruction
	astrocade_blueram_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override { m_ram.resize(0x4000); save_item(NAME(m_ram)); }
};

// ======================> astrocade_blueram_32k_device

class astrocade_blueram_32k_device : public astrocade_blueram_4k_device
{
public:
	// construction/destruction
	astrocade_blueram_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override { m_ram.resize(0x8000); save_item(NAME(m_ram)); }
};

// ======================> astrocade_viper_sys1_device

class astrocade_viper_sys1_device : public device_t, public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_viper_sys1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override { m_ram.resize(0x4000); save_item(NAME(m_ram)); }
	virtual void device_reset() override { }

private:
	std::vector<uint8_t> m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_whiteram_device

class astrocade_whiteram_device : public device_t, public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_whiteram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override { m_ram.resize(0x8000); save_item(NAME(m_ram)); }
	virtual void device_reset() override { }

private:
	std::vector<uint8_t> m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_rl64ram_device

class astrocade_rl64ram_device : public device_t, public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_rl64ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override { m_ram.resize(0xb000); save_item(NAME(m_ram)); }
	virtual void device_reset() override { }

private:
	std::vector<uint8_t> m_ram;
	required_ioport m_write_prot;
};



// device type definition
DECLARE_DEVICE_TYPE(ASTROCADE_BLUERAM_4K,  astrocade_blueram_4k_device)
DECLARE_DEVICE_TYPE(ASTROCADE_BLUERAM_16K, astrocade_blueram_16k_device)
DECLARE_DEVICE_TYPE(ASTROCADE_BLUERAM_32K, astrocade_blueram_32k_device)
DECLARE_DEVICE_TYPE(ASTROCADE_VIPER_SYS1,  astrocade_viper_sys1_device)
DECLARE_DEVICE_TYPE(ASTROCADE_WHITERAM,    astrocade_whiteram_device)
DECLARE_DEVICE_TYPE(ASTROCADE_RL64RAM,     astrocade_rl64ram_device)


#endif // MAME_BUS_ASTROCADE_RAM_H
