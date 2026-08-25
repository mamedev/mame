// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1541-compatible clone Disk Drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_C1541_CLONES_H
#define MAME_BUS_CBMIEC_C1541_CLONES_H

#pragma once

#include "c1541.h"
#include "machine/6821pia.h"
#include "machine/output_latch.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fsd1_device

class fsd1_device :  public c1541_device_base
{
public:
	// construction/destruction
	fsd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> fsd2_device

class fsd2_device :  public c1541_device_base
{
public:
	// construction/destruction
	fsd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> csd1_device

class csd1_device :  public c1541_device_base
{
public:
	// construction/destruction
	csd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> c1541_dolphin_dos_device

class c1541_dolphin_dos_device : public c1541_device_base
{
public:
	// construction/destruction
	c1541_dolphin_dos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void c1541dd_mem(address_map &map) ATTR_COLD;
};


// ======================> c1541_professional_dos_v1_device

class c1541_professional_dos_v1_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_professional_dos_v1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void c1541pd_mem(address_map &map) ATTR_COLD;
};


// ======================> c1541_turbotrans_device

class c1541_turbotrans_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> c1541_prologic_dos_classic_device

class c1541_prologic_dos_classic_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pia6821_device> m_pia;
	required_device<output_latch_device> m_cent_data_out;
	required_memory_region m_mmu_rom;

	uint8_t pia_r(offs_t offset);
	void pia_w(offs_t offset, uint8_t data);
	void pia_pa_w(uint8_t data);
	uint8_t pia_pb_r();
	void pia_pb_w(uint8_t data);
	uint8_t read();
	void write(uint8_t data);

	void c1541pdc_mem(address_map &map) ATTR_COLD;
};


// ======================> indus_gt_device

class indus_gt_device :  public c1541_device_base
{
public:
	// construction/destruction
	indus_gt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> technica_device

class technica_device :  public c1541_device_base
{
public:
	// construction/destruction
	technica_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> blue_chip_device

class blue_chip_device :  public c1541_device_base
{
public:
	// construction/destruction
	blue_chip_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> commander_c2_device

class commander_c2_device :  public c1541_device_base
{
public:
	// construction/destruction
	commander_c2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> enhancer_2000_device

class enhancer_2000_device :  public c1541_device_base
{
public:
	// construction/destruction
	enhancer_2000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> fd148_device

class fd148_device :  public c1541_device_base
{
public:
	// construction/destruction
	fd148_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> msd_sd1_device

class msd_sd1_device :  public c1541_device_base
{
public:
	// construction/destruction
	msd_sd1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> msd_sd2_device

class msd_sd2_device :  public c1541_device_base
{
public:
	// construction/destruction
	msd_sd2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(FSD1,                       fsd1_device)
DECLARE_DEVICE_TYPE(FSD2,                       fsd2_device)
DECLARE_DEVICE_TYPE(CSD1,                       csd1_device)
DECLARE_DEVICE_TYPE(C1541_DOLPHIN_DOS,          c1541_dolphin_dos_device)
DECLARE_DEVICE_TYPE(C1541_TURBOTRANS,           c1541_turbotrans_device)
DECLARE_DEVICE_TYPE(C1541_PROFESSIONAL_DOS_V1,  c1541_professional_dos_v1_device)
DECLARE_DEVICE_TYPE(C1541_PROLOGIC_DOS_CLASSIC, c1541_prologic_dos_classic_device)
DECLARE_DEVICE_TYPE(INDUS_GT,                   indus_gt_device)
DECLARE_DEVICE_TYPE(TECHNICA,                   technica_device)
DECLARE_DEVICE_TYPE(BLUE_CHIP,                  blue_chip_device)
DECLARE_DEVICE_TYPE(COMMANDER_C2,               commander_c2_device)
DECLARE_DEVICE_TYPE(ENHANCER_2000,              enhancer_2000_device)
DECLARE_DEVICE_TYPE(FD148,                      fd148_device)
DECLARE_DEVICE_TYPE(MSD_SD1,                    msd_sd1_device)
DECLARE_DEVICE_TYPE(MSD_SD2,                    msd_sd2_device)


#endif // MAME_BUS_CBMIEC_C1541_CLONES_H
