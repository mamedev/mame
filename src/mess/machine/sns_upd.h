#ifndef __SNS_UPD_H
#define __SNS_UPD_H

#include "machine/sns_slot.h"
#include "machine/sns_rom.h"
#include "machine/sns_rom21.h"
#include "cpu/upd7725/upd7725.h"

// ======================> sns_rom_necdsp_device

class sns_rom20_necdsp_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom20_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "sns_rom_necdsp"; }
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<upd7725_device> m_upd7725;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	virtual DECLARE_READ32_MEMBER(necdsp_prg_r);
	virtual DECLARE_READ16_MEMBER(necdsp_data_r);
};

// ======================> sns_rom21_necdsp_device

class sns_rom21_necdsp_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "sns_rom21_necdsp"; }
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<upd7725_device> m_upd7725;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	virtual DECLARE_READ32_MEMBER(necdsp_prg_r);
	virtual DECLARE_READ16_MEMBER(necdsp_data_r);
};

// ======================> sns_rom_setadsp_device

class sns_rom_setadsp_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_setadsp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "sns_rom_setadsp"; }

	required_device<upd96050_device> m_upd96050;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	virtual DECLARE_READ32_MEMBER(setadsp_prg_r);
	virtual DECLARE_READ16_MEMBER(setadsp_data_r);
};

// ======================> sns_rom_seta10_device

class sns_rom_seta10dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta10dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "sns_rom_seta10"; }
	virtual machine_config_constructor device_mconfig_additions() const;
};

// ======================> sns_rom_seta11_device [Faster CPU than ST010]

class sns_rom_seta11dsp_device : public sns_rom_setadsp_device
{
public:
	// construction/destruction
	sns_rom_seta11dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "sns_rom_seta11"; }
	virtual machine_config_constructor device_mconfig_additions() const;
};


// device type definition
extern const device_type SNS_LOROM_NECDSP;
extern const device_type SNS_HIROM_NECDSP;
extern const device_type SNS_LOROM_SETA10;
extern const device_type SNS_LOROM_SETA11;

#endif
