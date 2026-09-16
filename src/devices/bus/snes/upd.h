// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_UPD_H
#define MAME_BUS_SNES_UPD_H

#pragma once

#include "snes_slot.h"
#include "rom.h"
#include "rom21.h"
#include "cpu/upd7725/upd7725.h"

// ======================> sns_rom20_necdsp_device

class sns_rom20_necdsp_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom20_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sns_rom20_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

	uint32_t necdsp_prg_r(offs_t offset);
	uint16_t necdsp_data_r(offs_t offset);

	void dsp_data_map_lorom(address_map &map) ATTR_COLD;
	void dsp_prg_map_lorom(address_map &map) ATTR_COLD;

	required_device<upd7725_device> m_upd7725;

private:
	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom21_necdsp_device

class sns_rom21_necdsp_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sns_rom21_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

	uint32_t necdsp_prg_r(offs_t offset);
	uint16_t necdsp_data_r(offs_t offset);

	void dsp_data_map_hirom(address_map &map) ATTR_COLD;
	void dsp_prg_map_hirom(address_map &map) ATTR_COLD;

	required_device<upd7725_device> m_upd7725;

private:
	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom_setadsp_device

class sns_rom_setadsp_device : public sns_rom_device
{
public:
	virtual void speedup_addon_bios_access() override;

protected:
	// construction/destruction
	sns_rom_setadsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

	virtual uint32_t setadsp_prg_r(offs_t offset);
	virtual uint16_t setadsp_data_r(offs_t offset);

	void st01x_data_map(address_map &map) ATTR_COLD;
	void st01x_prg_map(address_map &map) ATTR_COLD;

	required_device<upd96050_device> m_upd96050;

private:
	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom_seta10dsp_device

class sns_rom_seta10dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta10dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// ======================> sns_rom_seta11dsp_device [Faster CPU than ST010]

class sns_rom_seta11dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta11dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP, sns_rom20_necdsp_device)
DECLARE_DEVICE_TYPE(SNS_HIROM_NECDSP, sns_rom21_necdsp_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SETA10, sns_rom_seta10dsp_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SETA11, sns_rom_seta11dsp_device)



// Devices including DSP dumps to support faulty .sfc dumps missing DSP data

class sns_rom20_necdsp1_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp1_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom20_necdsp1b_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp1b_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom20_necdsp2_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp2_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom20_necdsp3_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp3_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom20_necdsp4_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp4_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom21_necdsp1_legacy_device : public sns_rom21_necdsp_device
{
public:
	// construction/destruction
	sns_rom21_necdsp1_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom_seta10dsp_legacy_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta10dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sns_rom_seta11dsp_legacy_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta11dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP1_LEG,  sns_rom20_necdsp1_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP1B_LEG, sns_rom20_necdsp1b_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP2_LEG,  sns_rom20_necdsp2_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP3_LEG,  sns_rom20_necdsp3_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_NECDSP4_LEG,  sns_rom20_necdsp4_legacy_device)
DECLARE_DEVICE_TYPE(SNS_HIROM_NECDSP1_LEG,  sns_rom21_necdsp1_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SETA10_LEG,   sns_rom_seta10dsp_legacy_device)
DECLARE_DEVICE_TYPE(SNS_LOROM_SETA11_LEG,   sns_rom_seta11dsp_legacy_device)

#endif // MAME_BUS_SNES_UPD_H
