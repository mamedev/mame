// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rom.h
 * Amstrad CPC mountable ROM image device
 *
 */

#ifndef CPC_ROM_H_
#define CPC_ROM_H_

#include "emu.h"
#include "cpcexp.h"

/*** ROM image device ***/

// ======================> rom_image_device

class rom_image_device :    public device_t,
							public device_image_interface
{
public:
	// construction/destruction
	rom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~rom_image_device();

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t image_type() const override { return IO_ROM; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "cpc_rom"; }
	virtual const char *file_extensions() const override { return "rom,bin"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	UINT8* base() { return m_base; }

protected:
	// device-level overrides
	virtual void device_config_complete() override { update_names(); }
	virtual void device_start() override;

private:
	UINT8* m_base;
};


// device type definition
extern const device_type ROMSLOT;


#define MCFG_ROMSLOT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ROMSLOT, 0)

/*** ROM box device ***/

class cpc_rom_device  : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	UINT8* base(UINT8 slot) { if(slot >=1 && slot <= 8) return m_rom[slot]->base(); else return nullptr; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	//cpc_expansion_slot_device *m_slot;

	rom_image_device* m_rom[8];
};

// device type definition
extern const device_type CPC_ROM;


#endif
