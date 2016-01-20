// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#ifndef __ISA_LPT_H__
#define __ISA_LPT_H__

#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_lpt_device

class isa8_lpt_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_lpt_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	bool is_primary() { return m_is_primary; }

	WRITE_LINE_MEMBER(pc_cpu_line);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	// internal state
	bool m_is_primary;
};

// device type definition
extern const device_type ISA8_LPT;

#endif /* __ISA_LPT_H__ */
