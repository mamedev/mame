// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A800_ROM_H
#define __A800_ROM_H

#include "a800_slot.h"


// ======================> a800_rom_device

class a800_rom_device : public device_t,
						public device_a800_cart_interface
{
public:
	// construction/destruction
	a800_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a800_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
};


// ======================> a800_rom_bbsb_device

class a800_rom_bbsb_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_bbsb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_80xx) override;

protected:
	int m_banks[2];
};


// ======================> a800_rom_williams_device

class a800_rom_williams_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_williams_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;

protected:
	int m_bank;
};


// ======================> a800_rom_express_device

class a800_rom_express_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_express_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;

protected:
	int m_bank;
};


// ======================> a800_rom_blizzard_device

class a800_rom_blizzard_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_blizzard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
};


// ======================> a800_rom_turbo_device

class a800_rom_turbo_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_turbo_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;

protected:
	int m_bank;
};


// ======================> a800_rom_telelink2_device

class a800_rom_telelink2_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_telelink2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_80xx) override;
	virtual DECLARE_READ8_MEMBER(read_d5xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;
};


// ======================> a800_rom_microcalc_device

class a800_rom_microcalc_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_microcalc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;

protected:
	int m_bank;
};


// ======================> xegs_rom_device

class xegs_rom_device : public a800_rom_device
{
public:
	// construction/destruction
	xegs_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) override;

protected:
	int m_bank;
};


// ======================> a5200_rom_2chips_device

class a5200_rom_2chips_device : public a800_rom_device
{
public:
	// construction/destruction
	a5200_rom_2chips_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
};


// ======================> a5200_rom_bbsb_device

class a5200_rom_bbsb_device : public a800_rom_device
{
public:
	// construction/destruction
	a5200_rom_bbsb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read_80xx) override;
	virtual DECLARE_WRITE8_MEMBER(write_80xx) override;

protected:
	int m_banks[2];
};



// device type definition
extern const device_type A800_ROM;
extern const device_type A800_ROM_BBSB;
extern const device_type A800_ROM_WILLIAMS;
extern const device_type A800_ROM_EXPRESS;
extern const device_type A800_ROM_TURBO;
extern const device_type A800_ROM_TELELINK2;
extern const device_type A800_ROM_MICROCALC;
extern const device_type XEGS_ROM;
extern const device_type A5200_ROM_2CHIPS;
extern const device_type A5200_ROM_BBSB;


#endif
