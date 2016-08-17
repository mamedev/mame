// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SCV_ROM_H
#define __SCV_ROM_H

#include "slot.h"


// ======================> scv_rom8_device

class scv_rom8_device : public device_t,
						public device_scv_cart_interface
{
public:
	// construction/destruction
	scv_rom8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	scv_rom8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
};

// ======================> scv_rom16_device

class scv_rom16_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
};


// ======================> scv_rom32_device

class scv_rom32_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
};


// ======================> scv_rom32ram8_device

class scv_rom32ram8_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom32ram8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

private:
	UINT8 m_ram_enabled;
};


// ======================> scv_rom64_device

class scv_rom64_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

private:
	UINT8 m_bank_base;
};


// ======================> scv_rom128_device

class scv_rom128_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

private:
	UINT8 m_bank_base;
};


// ======================> scv_rom128ram4_device

class scv_rom128ram4_device : public scv_rom8_device
{
public:
	// construction/destruction
	scv_rom128ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

private:
	UINT8 m_bank_base, m_ram_enabled;
};



// device type definition
extern const device_type SCV_ROM8K;
extern const device_type SCV_ROM16K;
extern const device_type SCV_ROM32K;
extern const device_type SCV_ROM32K_RAM8K;
extern const device_type SCV_ROM64K;
extern const device_type SCV_ROM128K;
extern const device_type SCV_ROM128K_RAM4K;



#endif
