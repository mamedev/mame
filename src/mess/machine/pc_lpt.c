/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#include "emu.h"
#include "pc_lpt.h"
#include "machine/ctronics.h"
#include "cntr_covox.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static WRITE_LINE_DEVICE_HANDLER( pc_lpt_ack_w );


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pc_lpt_state pc_lpt_state;
struct _pc_lpt_state
{
	centronics_device *centronics;

	devcb_resolved_write_line out_irq_func;

	UINT8 data;

	int ack;

	/* control latch */
	int strobe;
	int autofd;
	int init;
	int select;
	int irq_enabled;
};


/***************************************************************************
    CENTRONICS INTERFACE
***************************************************************************/

static const centronics_interface pc_centronics_config =
{
	DEVCB_LINE(pc_lpt_ack_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static SLOT_INTERFACE_START(pc_centronics)
	SLOT_INTERFACE("printer", CENTRONICS_PRINTER)
	SLOT_INTERFACE("covox", CENTRONICS_COVOX)
	SLOT_INTERFACE("covox_stereo", CENTRONICS_COVOX_STEREO)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( pc_lpt )
	MCFG_CENTRONICS_ADD("centronics", pc_centronics_config, pc_centronics, "printer", NULL)
MACHINE_CONFIG_END


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE pc_lpt_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PC_LPT);

	return (pc_lpt_state *)downcast<pc_lpt_device *>(device)->token();
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static DEVICE_START( pc_lpt )
{
	pc_lpt_state *lpt = get_safe_token(device);
	const pc_lpt_interface *intf = (const pc_lpt_interface *)device->static_config();
	/* validate some basic stuff */
	assert(device->static_config() != NULL);

	/* get centronics device */
	lpt->centronics = device->subdevice<centronics_device>("centronics");
	assert(lpt->centronics != NULL);

	/* resolve callbacks */
	lpt->out_irq_func.resolve(intf->out_irq_func, *device);

	/* register for state saving */
	device->save_item(NAME(lpt->ack));
	device->save_item(NAME(lpt->strobe));
	device->save_item(NAME(lpt->autofd));
	device->save_item(NAME(lpt->init));
	device->save_item(NAME(lpt->select));
	device->save_item(NAME(lpt->irq_enabled));
}

static DEVICE_RESET( pc_lpt )
{
	pc_lpt_state *lpt = get_safe_token(device);

	lpt->strobe = TRUE;
	lpt->autofd = TRUE;
	lpt->init = FALSE;
	lpt->select = TRUE;
	lpt->irq_enabled = FALSE;
	lpt->data = 0xff;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static WRITE_LINE_DEVICE_HANDLER( pc_lpt_ack_w )
{
	pc_lpt_state *lpt = get_safe_token(device->owner());

	if (lpt->irq_enabled && lpt->ack == TRUE && state == FALSE)
	{
		/* pulse irq when going from high to low */
		lpt->out_irq_func(TRUE);
		lpt->out_irq_func(FALSE);
	}

	lpt->ack = state;
}


READ8_DEVICE_HANDLER( pc_lpt_data_r )
{
	pc_lpt_state *lpt = get_safe_token(device);
	// pull up mechanism for input lines, zeros are provided by pheripherial
	return lpt->data & ~lpt->centronics->read(*device->machine().memory().first_space(), 0);
}


WRITE8_DEVICE_HANDLER( pc_lpt_data_w )
{
	pc_lpt_state *lpt = get_safe_token(device);
	lpt->data = data;
	lpt->centronics->write(*device->machine().memory().first_space(), 0, data);
}


READ8_DEVICE_HANDLER( pc_lpt_status_r )
{
	pc_lpt_state *lpt = get_safe_token(device);
	UINT8 result = 0;

	result |= lpt->centronics->fault_r() << 3;
	result |= lpt->centronics->vcc_r() << 4; /* SELECT is connected to VCC */
	result |= !lpt->centronics->pe_r() << 5;
	result |= lpt->centronics->ack_r() << 6;
	result |= !lpt->centronics->busy_r() << 7;

	return result;
}


READ8_DEVICE_HANDLER( pc_lpt_control_r )
{
	pc_lpt_state *lpt = get_safe_token(device);
	UINT8 result = 0;

	/* return latch state */
	result |= lpt->strobe << 0;
	result |= lpt->autofd << 1;
	result |= lpt->init << 2;
	result |= lpt->select << 3;
	result |= lpt->irq_enabled << 4;

	return result;
}


WRITE8_DEVICE_HANDLER( pc_lpt_control_w )
{
	pc_lpt_state *lpt = get_safe_token(device);

//  logerror("pc_lpt_control_w: 0x%02x\n", data);

	/* save to latch */
	lpt->strobe = BIT(data, 0);
	lpt->autofd = BIT(data, 1);
	lpt->init = BIT(data, 2);
	lpt->select = BIT(data, 3);
	lpt->irq_enabled = BIT(data, 4);

	/* output to centronics */
	lpt->centronics->strobe_w(lpt->strobe);
	lpt->centronics->autofeed_w(lpt->autofd);
	lpt->centronics->init_prime_w(lpt->init);
}


READ8_DEVICE_HANDLER( pc_lpt_r )
{
	switch (offset)
	{
	case 0: return pc_lpt_data_r(device, 0);
	case 1: return pc_lpt_status_r(device, 0);
	case 2: return pc_lpt_control_r(device, 0);
	}

	/* if we reach this its an error */
	logerror("PC-LPT %s: Read from invalid offset %x\n", device->tag(), offset);

	return 0xff;
}


WRITE8_DEVICE_HANDLER( pc_lpt_w )
{
	switch (offset)
	{
	case 0: pc_lpt_data_w(device, 0, data); break;
	case 1: break;
	case 2:	pc_lpt_control_w(device, 0, data); break;
	}
}

const device_type PC_LPT = &device_creator<pc_lpt_device>;

pc_lpt_device::pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_LPT, "PC-LPT", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(pc_lpt_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pc_lpt_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc_lpt_device::device_start()
{
	DEVICE_START_NAME( pc_lpt )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc_lpt_device::device_reset()
{
	DEVICE_RESET_NAME( pc_lpt )(this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor pc_lpt_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc_lpt  );
}



static WRITE_LINE_DEVICE_HANDLER(pc_cpu_line)
{
	isa8_lpt_device	*lpt  = downcast<isa8_lpt_device *>(device->owner());
	if (lpt->is_primary())
		lpt->m_isa->irq7_w(state);
	else
		lpt->m_isa->irq5_w(state);
}
static const pc_lpt_interface pc_lpt_config =
{
	DEVCB_LINE(pc_cpu_line)
};

static MACHINE_CONFIG_FRAGMENT( lpt_config )
	MCFG_PC_LPT_ADD("lpt", pc_lpt_config)
MACHINE_CONFIG_END

static INPUT_PORTS_START( lpt_dsw )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Base address")
	PORT_DIPSETTING(	0x00, "0x378" )
	PORT_DIPSETTING(	0x01, "0x278" )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_LPT = &device_creator<isa8_lpt_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_lpt_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lpt_config );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_lpt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( lpt_dsw );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_lpt_device - constructor
//-------------------------------------------------

isa8_lpt_device::isa8_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_LPT, "Printer Adapter", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_lpt_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_lpt_device::device_reset()
{
	m_is_primary = (ioport("DSW")->read() & 1) ? false : true;
	if (m_is_primary) {
		m_isa->install_device(subdevice("lpt"), 0x0378, 0x037b, 0, 0, FUNC(pc_lpt_r), FUNC(pc_lpt_w) );
	} else {
		m_isa->install_device(subdevice("lpt"), 0x0278, 0x027b, 0, 0, FUNC(pc_lpt_r), FUNC(pc_lpt_w) );
	}
}
