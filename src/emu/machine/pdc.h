// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 Peripheral device controller emulation

**********************************************************************/

#pragma once

#ifndef __R9751_PDC_H__
#define __R9751_PDC_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
#include "formats/pc_dsk.h"
#include "machine/hdc92x4.h"
#include "imagedev/mfmhd.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PDC_TAG           "pdc"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pdc_device

class pdc_device :  public device_t
{
public:
        // construction/destruction
//        pdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
        // optional information overrides
        virtual machine_config_constructor device_mconfig_additions() const;
//        virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;
//        DECLARE_WRITE_LINE_MEMBER( via0_irq_w );
//        virtual DECLARE_READ8_MEMBER( via0_pa_r );
//        DECLARE_WRITE8_MEMBER( via0_pa_w );
//        DECLARE_READ8_MEMBER( via0_pb_r );
//        DECLARE_WRITE8_MEMBER( via0_pb_w );
//        DECLARE_WRITE_LINE_MEMBER( via0_ca2_w );
//        DECLARE_WRITE_LINE_MEMBER( via1_irq_w );
//        DECLARE_READ8_MEMBER( via1_pb_r );
//        DECLARE_WRITE8_MEMBER( via1_pb_w );
//        DECLARE_WRITE_LINE_MEMBER( atn_w );
//        DECLARE_WRITE_LINE_MEMBER( byte_w );

//	DECLARE_READ8_MEMBER(memory_read_byte);
//	DECLARE_WRITE8_MEMBER(memory_write_byte);

	DECLARE_WRITE_LINE_MEMBER(i8237_hreq_w);
	DECLARE_WRITE_LINE_MEMBER(i8237_eop_w);
	DECLARE_READ8_MEMBER(i8237_dma_mem_r);
	DECLARE_WRITE8_MEMBER(i8237_dma_mem_w);
	DECLARE_READ8_MEMBER(i8237_fdc_dma_r);
	DECLARE_WRITE8_MEMBER(i8237_fdc_dma_w);

	DECLARE_WRITE_LINE_MEMBER(hdd_irq);

	DECLARE_WRITE8_MEMBER(p38_w);
	DECLARE_READ8_MEMBER(p38_r);

        DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();

        // device_cbm_iec_interface overrides
//        virtual void cbm_iec_atn(int state);
//        virtual void cbm_iec_reset(int state);

        // device_c64_floppy_parallel_interface overrides
//        virtual void parallel_data_w(UINT8 data);
//        virtual void parallel_strobe_w(int state);

//        enum
//        {
//                LED_POWER = 0,
//                LED_ACT
//        };

//        inline void set_iec_data();

        required_device<cpu_device> m_pdccpu;
//        required_device<via6522_device> m_via0;
//        required_device<via6522_device> m_via1;
//        required_device<c64h156_device> m_ga;
//        required_device<floppy_image_device> m_fdc;
	required_device<am9517a_device> m_dma8237;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	optional_device<hdc9224_device> m_hdc9224;
	mfm_harddisk_device*    m_harddisk;
//        required_ioport m_address;
	required_shared_ptr<UINT8> m_pdc_ram;
        // IEC bus
//        int m_data_out;                         // serial data out

        // interrupts
//        int m_via0_irq;                         // VIA #0 interrupt request
//        int m_via1_irq;                         // VIA #1 interrupt request
	UINT8 reg_p38;
};



extern const device_type PDC;

#endif
