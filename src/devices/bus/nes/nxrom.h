// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_MMCX_H
#define __NES_MMCX_H

#include "nes_slot.h"
#include "sound/samples.h"


// ======================> nes_nrom_device

class nes_nrom_device : public device_t,
						public device_nes_cart_interface
{
public:
	// construction/destruction
	nes_nrom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_nrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual void device_start() override { common_start(); }

	virtual void pcb_reset() override;

	void common_start();
};


// ======================> nes_nrom368_device

class nes_nrom368_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nrom368_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
};


// ======================> nes_fcbasic_device

class nes_fcbasic_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fcbasic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// emulate the additional WRAM
};


// ======================> nes_axrom_device

class nes_axrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_axrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bxrom_device

class nes_bxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bxrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_cnrom_device

class nes_cnrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cnrom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_cnrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_chr_open_bus;
};


// ======================> nes_cprom_device

class nes_cprom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cprom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gxrom_device

class nes_gxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_gxrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_uxrom_device

class nes_uxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_uxrom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_uxrom_cc_device

class nes_uxrom_cc_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_uxrom_cc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_un1rom_device

class nes_un1rom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_un1rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_nochr_device

class nes_nochr_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nochr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual DECLARE_WRITE8_MEMBER(chr_w) override;
};



// device type definition
extern const device_type NES_NROM;
extern const device_type NES_NROM368;
extern const device_type NES_FCBASIC;
extern const device_type NES_AXROM;
extern const device_type NES_BXROM;
extern const device_type NES_CNROM;
extern const device_type NES_CPROM;
extern const device_type NES_GXROM;
extern const device_type NES_UXROM;
extern const device_type NES_UXROM_CC;
extern const device_type NES_UN1ROM;
extern const device_type NES_NOCHR;

#endif
