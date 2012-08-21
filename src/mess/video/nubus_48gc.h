#pragma once

#ifndef __NUBUS_48GC_H__
#define __NUBUS_48GC_H__

#include "emu.h"
#include "machine/nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jmfb_device

class jmfb_device :
		public device_t,
		public device_nubus_card_interface
{
public:
        // construction/destruction
		jmfb_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();

        DECLARE_READ32_MEMBER(mac_48gc_r);
        DECLARE_WRITE32_MEMBER(mac_48gc_w);

public:
        UINT8  *m_vram;
        UINT32 m_mode, m_vbl_disable, m_toggle, m_stride, m_base;
        UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
        UINT32 m_registers[0x100];
        int m_xres, m_yres;
        bool m_is824;
};

class nubus_48gc_device : public jmfb_device
{
public:
    nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class nubus_824gc_device : public jmfb_device
{
public:
    nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
    virtual const rom_entry *device_rom_region() const;
};

// device type definition
extern const device_type NUBUS_48GC;
extern const device_type NUBUS_824GC;

#endif  /* __NUBUS_48GC_H__ */
