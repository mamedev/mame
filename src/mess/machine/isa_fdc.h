/**********************************************************************

    ISA 8 bit Floppy Disk Controller

**********************************************************************/
#pragma once

#ifndef ISA_FDC_H
#define ISA_FDC_H

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_fdc_device

class isa8_fdc_device :
		public device_t,
		public device_isa8_card_interface
{
public:
        // construction/destruction
        isa8_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		isa8_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
		virtual void device_config_complete() { m_shortname = "isa_fdc"; }
private:
        // internal state
public:
		virtual UINT8 dack_r(int line);
		virtual void dack_w(int line,UINT8 data);
		virtual void eop_w(int state);

		int status_register_a;
		int status_register_b;
		int digital_output_register;
		int tape_drive_register;
		int data_rate_register;
		int digital_input_register;
		int configuration_control_register;

		/* stored tc state - state present at pins */
		int tc_state;
		/* stored dma drq state */
		int dma_state;
		/* stored int state */
		int int_state;

		required_device<device_t> m_upd765;
};


// device type definition
extern const device_type ISA8_FDC;

// ======================> isa8_fdc_smc_device

class isa8_fdc_smc_device :
		public isa8_fdc_device
{
public:
    // construction/destruction
    isa8_fdc_smc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_config_complete() { m_shortname = "isa_fdc_smc"; }
};


// device type definition
extern const device_type ISA8_FDC_SMC;

#endif  /* ISA_FDC_H */
