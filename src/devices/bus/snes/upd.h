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

	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	virtual DECLARE_READ32_MEMBER(necdsp_prg_r);
	virtual DECLARE_READ16_MEMBER(necdsp_data_r);

protected:
	sns_rom20_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	required_device<upd7725_device> m_upd7725;

	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom21_necdsp_device

class sns_rom21_necdsp_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	virtual DECLARE_READ32_MEMBER(necdsp_prg_r);
	virtual DECLARE_READ16_MEMBER(necdsp_data_r);

protected:
	sns_rom21_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	required_device<upd7725_device> m_upd7725;

	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom_setadsp_device

class sns_rom_setadsp_device : public sns_rom_device
{
public:
	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	virtual DECLARE_READ32_MEMBER(setadsp_prg_r);
	virtual DECLARE_READ16_MEMBER(setadsp_data_r);

protected:
	// construction/destruction
	sns_rom_setadsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	required_device<upd96050_device> m_upd96050;

	std::vector<uint32_t> m_dsp_prg;
	std::vector<uint16_t> m_dsp_data;
};

// ======================> sns_rom_seta10dsp_device

class sns_rom_seta10dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta10dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};

// ======================> sns_rom_seta11dsp_device [Faster CPU than ST010]

class sns_rom_seta11dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta11dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
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

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom20_necdsp1b_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp1b_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom20_necdsp2_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp2_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom20_necdsp3_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp3_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom20_necdsp4_legacy_device : public sns_rom20_necdsp_device
{
public:
	// construction/destruction
	sns_rom20_necdsp4_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom21_necdsp1_legacy_device : public sns_rom21_necdsp_device
{
public:
	// construction/destruction
	sns_rom21_necdsp1_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom_seta10dsp_legacy_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta10dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_rom_seta11dsp_legacy_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta11dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
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
