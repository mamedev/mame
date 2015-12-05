// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#pragma once

#ifndef __COCO_FDC_H__
#define __COCO_FDC_H__

#include "emu.h"
#include "cococart.h"
#include "machine/msm6242.h"
#include "machine/ds1315.h"
#include "machine/wd_fdc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_rtc_type_t

enum coco_rtc_type_t
{
	RTC_DISTO   = 0x00,
	RTC_CLOUD9  = 0x01,

	RTC_NONE    = 0xFF
};

// ======================> coco_fdc_device

class coco_fdc_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		coco_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		DECLARE_FLOPPY_FORMATS(floppy_formats);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

		virtual UINT8* get_cart_base();

		virtual void update_lines();
		virtual void dskreg_w(UINT8 data);

		void set_intrq(UINT8 val) { m_intrq = val; }
		void set_drq(UINT8 val) { m_drq = val; }

		DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
		DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
protected:
		// device-level overrides
		virtual void device_start();
		virtual DECLARE_READ8_MEMBER(read);
		virtual DECLARE_WRITE8_MEMBER(write);

		coco_rtc_type_t real_time_clock();

		// internal state
		cococart_slot_device *m_owner;

		UINT8 m_dskreg;
		UINT8 m_drq : 1;
		UINT8 m_intrq : 1;

		optional_device<wd1773_t> m_wd17xx;              /* WD17xx */
		optional_device<wd2797_t> m_wd2797;              /* WD2797 */
		optional_device<ds1315_device> m_ds1315;         /* DS1315 */

		/* Disto RTC */
		optional_device<msm6242_device> m_disto_msm6242;        /* 6242 RTC on Disto interface */

		offs_t m_msm6242_rtc_address;
};


// device type definition
extern const device_type COCO_FDC;

// ======================> coco_fdc_v11_device

class coco_fdc_v11_device :
		public coco_fdc_device
{
public:
		// construction/destruction
		coco_fdc_v11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual const rom_entry *device_rom_region() const;
};


// device type definition
extern const device_type COCO_FDC_V11;

// ======================> coco3_hdb1_device

class coco3_hdb1_device :
		public coco_fdc_device
{
public:
		// construction/destruction
		coco3_hdb1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual const rom_entry *device_rom_region() const;
};


// device type definition
extern const device_type COCO3_HDB1;

// ======================> cp400_fdc_device

class cp400_fdc_device :
		public coco_fdc_device
{
public:
		// construction/destruction
		cp400_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual const rom_entry *device_rom_region() const;
};


// device type definition
extern const device_type CP400_FDC;

// ======================> dragon_fdc_device

class dragon_fdc_device :
		public coco_fdc_device
{
public:
		// construction/destruction
		dragon_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		dragon_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
		virtual void update_lines();
		virtual void dskreg_w(UINT8 data);
protected:
		// device-level overrides
		virtual void device_start();
		virtual DECLARE_READ8_MEMBER(read);
		virtual DECLARE_WRITE8_MEMBER(write);
};


// device type definition
extern const device_type DRAGON_FDC;

// ======================> sdtandy_fdc_device

class sdtandy_fdc_device :
		public dragon_fdc_device
{
public:
		// construction/destruction
		sdtandy_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual const rom_entry *device_rom_region() const;
};


// device type definition
extern const device_type SDTANDY_FDC;

#endif  /* __COCO_FDC_H__ */
