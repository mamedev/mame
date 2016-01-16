// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __KC_RAM_H__
#define __KC_RAM_H__

#include "emu.h"
#include "kc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> kc_m011_device

class kc_m011_device :
		public device_t,
		public device_kcexp_interface
{
public:
	// construction/destruction
	kc_m011_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	kc_m011_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0xf6; }
	virtual void control_w(UINT8 data) override;
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;
	virtual DECLARE_WRITE_LINE_MEMBER( mei_w ) override;

protected:
	kcexp_slot_device *m_slot;

	// internal state
	int     m_mei;          // module enable line
	UINT8 * m_ram;
	UINT8   m_enabled;
	UINT8   m_write_enabled;
	UINT16  m_base;
	UINT8   m_segment;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const { return 0x10000; }
};


// ======================> kc_m022_device

class kc_m022_device :
		public kc_m011_device
{
public:
	// construction/destruction
	kc_m022_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0xf4; }
	virtual void read(offs_t offset, UINT8 &ata) override;
	virtual void write(offs_t offset, UINT8 data) override;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const override { return 0x4000; }
};


// ======================> kc_m032_device

class kc_m032_device :
		public kc_m011_device
{
public:
	// construction/destruction
	kc_m032_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override;

	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0x79; }
	virtual void control_w(UINT8 data) override;
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const override { return 0x40000; }
};


// ======================> kc_m034_device

class kc_m034_device :
		public kc_m011_device
{
public:
	// construction/destruction
	kc_m034_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override;

	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0x7a; }
	virtual void control_w(UINT8 data) override;
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const override { return 0x80000; }
};


// ======================> kc_m035_device

class kc_m035_device :
		public kc_m011_device
{
public:
	// construction/destruction
	kc_m035_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0x7b; }
	virtual void control_w(UINT8 data) override;
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const override { return 0x100000; }
};


// ======================> kc_m036_device

class kc_m036_device :
		public kc_m011_device
{
public:
	// construction/destruction
	kc_m036_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override;

	// kcexp_interface overrides
	virtual UINT8 module_id_r() override { return 0x78; }
	virtual void control_w(UINT8 data) override;
	virtual void read(offs_t offset, UINT8 &data) override;
	virtual void write(offs_t offset, UINT8 data) override;

private:
	// internal helpers
	virtual UINT32 get_ram_size() const override { return 0x20000; }
};

// device type definition
extern const device_type KC_M011;
extern const device_type KC_M022;
extern const device_type KC_M032;
extern const device_type KC_M034;
extern const device_type KC_M035;
extern const device_type KC_M036;

#endif  /* __KC_RAM_H__ */
