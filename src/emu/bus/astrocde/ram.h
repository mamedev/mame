// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __ASTROCADE_RAM_H
#define __ASTROCADE_RAM_H

#include "exp.h"


// ======================> astrocade_blueram_4k_device

class astrocade_blueram_4k_device : public device_t,
								public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_blueram_4k_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	astrocade_blueram_4k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { m_ram.resize(0x1000); save_item(NAME(m_ram)); }
	virtual void device_reset() {}
	virtual ioport_constructor device_input_ports() const;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

protected:
	dynamic_buffer m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_blueram_16k_device

class astrocade_blueram_16k_device : public astrocade_blueram_4k_device
{
public:
	// construction/destruction
	astrocade_blueram_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() { m_ram.resize(0x4000); save_item(NAME(m_ram)); }
};

// ======================> astrocade_blueram_32k_device

class astrocade_blueram_32k_device : public astrocade_blueram_4k_device
{
public:
	// construction/destruction
	astrocade_blueram_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_start() { m_ram.resize(0x8000); save_item(NAME(m_ram)); }
};

// ======================> astrocade_viper_sys1_device

class astrocade_viper_sys1_device : public device_t,
								public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_viper_sys1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { m_ram.resize(0x4000); save_item(NAME(m_ram)); }
	virtual void device_reset() {}
	virtual ioport_constructor device_input_ports() const;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	dynamic_buffer m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_whiteram_device

class astrocade_whiteram_device : public device_t,
								public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_whiteram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { m_ram.resize(0x8000); save_item(NAME(m_ram)); }
	virtual void device_reset() {}
	virtual ioport_constructor device_input_ports() const;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	dynamic_buffer m_ram;
	required_ioport m_write_prot;
};

// ======================> astrocade_rl64ram_device

class astrocade_rl64ram_device : public device_t,
							public device_astrocade_card_interface
{
public:
	// construction/destruction
	astrocade_rl64ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() { m_ram.resize(0xb000); save_item(NAME(m_ram)); }
	virtual void device_reset() {}
	virtual ioport_constructor device_input_ports() const;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:
	dynamic_buffer m_ram;
	required_ioport m_write_prot;
};



// device type definition
extern const device_type ASTROCADE_BLUERAM_4K;
extern const device_type ASTROCADE_BLUERAM_16K;
extern const device_type ASTROCADE_BLUERAM_32K;
extern const device_type ASTROCADE_VIPER_SYS1;
extern const device_type ASTROCADE_WHITERAM;
extern const device_type ASTROCADE_RL64RAM;


#endif
