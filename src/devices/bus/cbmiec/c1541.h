// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1540/1541/1541C/1541-II Single Disk Drive emulation

**********************************************************************/

#pragma once

#ifndef __C1541__
#define __C1541__

#include "emu.h"
#include "cbmiec.h"
#include "bus/c64/bn1541.h"
#include "cpu/m6502/m6502.h"
#include "machine/64h156.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/latch.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define C1541_TAG           "c1541"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_base_t

class c1541_base_t :  public device_t,
						public device_cbm_iec_interface,
						public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c1541_base_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( via0_irq_w );
	virtual DECLARE_READ8_MEMBER( via0_pa_r );
	DECLARE_WRITE8_MEMBER( via0_pa_w );
	DECLARE_READ8_MEMBER( via0_pb_r );
	DECLARE_WRITE8_MEMBER( via0_pb_w );
	DECLARE_WRITE_LINE_MEMBER( via0_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
	DECLARE_READ8_MEMBER( via1_pb_r );
	DECLARE_WRITE8_MEMBER( via1_pb_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( byte_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
	virtual void cbm_iec_reset(int state) override;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(UINT8 data) override;
	virtual void parallel_strobe_w(int state) override;

	enum
	{
		LED_POWER = 0,
		LED_ACT
	};

	inline void set_iec_data();

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<c64h156_device> m_ga;
	required_device<floppy_image_device> m_floppy;
	required_ioport m_address;

	// IEC bus
	int m_data_out;                         // serial data out

	// interrupts
	int m_via0_irq;                         // VIA #0 interrupt request
	int m_via1_irq;                         // VIA #1 interrupt request
};


// ======================> c1540_t

class c1540_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1540_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> c1541_t

class c1541_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> c1541c_t

class c1541c_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541c_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	virtual DECLARE_READ8_MEMBER( via0_pa_r );
};


// ======================> c1541ii_t

class c1541ii_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541ii_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> sx1541_t

class sx1541_t :  public c1541_base_t
{
public:
	// construction/destruction
	sx1541_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> fsd1_t

class fsd1_t :  public c1541_base_t
{
public:
	// construction/destruction
	fsd1_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> fsd2_t

class fsd2_t :  public c1541_base_t
{
public:
	// construction/destruction
	fsd2_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

	// device-level overrides
	virtual void device_start() override;
};


// ======================> csd1_t

class csd1_t :  public c1541_base_t
{
public:
	// construction/destruction
	csd1_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// ======================> c1541_dolphin_dos_t

class c1541_dolphin_dos_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541_dolphin_dos_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> c1541_professional_dos_v1_t

class c1541_professional_dos_v1_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541_professional_dos_v1_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> c1541_prologic_dos_classic_t

class c1541_prologic_dos_classic_t :  public c1541_base_t
{
public:
	// construction/destruction
	c1541_prologic_dos_classic_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );

	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_WRITE8_MEMBER( pia_pb_w );

protected:
	required_device<pia6821_device> m_pia;
	required_device<output_latch_device> m_cent_data_out;
	required_memory_region m_mmu_rom;
};


// ======================> indus_gt_t

class indus_gt_t :  public c1541_base_t
{
public:
	// construction/destruction
	indus_gt_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
};


// device type definition
extern const device_type C1540;
extern const device_type C1541;
extern const device_type C1541C;
extern const device_type C1541II;
extern const device_type SX1541;
extern const device_type FSD1;
extern const device_type FSD2;
extern const device_type CSD1;
extern const device_type C1541_DOLPHIN_DOS;
extern const device_type C1541_PROFESSIONAL_DOS_V1;
extern const device_type C1541_PROLOGIC_DOS_CLASSIC;
extern const device_type INDUS_GT;



#endif
