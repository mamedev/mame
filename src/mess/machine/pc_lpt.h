/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#ifndef __PC_LPT_H__
#define __PC_LPT_H__

#include "isa.h"
#include "bus/centronics/ctronics.h"
#include "bus/centronics/covox.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pc_lpt_interface
{
	devcb_write_line m_out_irq;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class pc_lpt_device : public device_t,
								public pc_lpt_interface
{
public:
	pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc_lpt_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( control_r );
	DECLARE_WRITE8_MEMBER( control_w );

	DECLARE_WRITE_LINE_MEMBER( ack_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	// internal state
	centronics_device *m_centronics;

	devcb_resolved_write_line m_out_irq_func;

	UINT8 m_data;

	int m_ack;

	/* control latch */
	int m_strobe;
	int m_autofd;
	int m_init;
	int m_select;
	int m_irq_enabled;
};

extern const device_type PC_LPT;


#define MCFG_PC_LPT_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, PC_LPT, 0) \
	MCFG_DEVICE_CONFIG(_intf)

#define MCFG_PC_LPT_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_lpt_device

class isa8_lpt_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual ioport_constructor device_input_ports() const;

		bool is_primary() { return m_is_primary; }
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
private:
		// internal state
		bool m_is_primary;
};


// device type definition
extern const device_type ISA8_LPT;

#endif /* __PC_LPT__ */
