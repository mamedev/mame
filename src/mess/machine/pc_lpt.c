/***************************************************************************

    IBM-PC printer interface

***************************************************************************/

#include "emu.h"
#include "pc_lpt.h"


/***************************************************************************
    CENTRONICS INTERFACE
***************************************************************************/

static const centronics_interface pc_centronics_config =
{
	DEVCB_LINE_MEMBER(pc_lpt_device, ack_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static SLOT_INTERFACE_START(pc_centronics)
	SLOT_INTERFACE("printer", CENTRONICS_PRINTER)
	SLOT_INTERFACE("covox", CENTRONICS_COVOX)
	SLOT_INTERFACE("covox_stereo", CENTRONICS_COVOX_STEREO)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( pc_lpt )
	MCFG_CENTRONICS_ADD("centronics", pc_centronics_config, pc_centronics, "printer")
MACHINE_CONFIG_END

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE_LINE_MEMBER( pc_lpt_device::ack_w )
{
	if (m_irq_enabled && m_ack == TRUE && state == FALSE)
	{
		/* pulse irq when going from high to low */
		m_out_irq_func(TRUE);
		m_out_irq_func(FALSE);
	}

	m_ack = state;
}


READ8_MEMBER( pc_lpt_device::data_r )
{
	// pull up mechanism for input lines, zeros are provided by pheripherial
	return m_data & ~m_centronics->read(space.machine().driver_data()->generic_space(), 0);
}


WRITE8_MEMBER( pc_lpt_device::data_w )
{
	m_data = data;
	m_centronics->write(space.machine().driver_data()->generic_space(), 0, data);
}


READ8_MEMBER( pc_lpt_device::status_r )
{
	UINT8 result = 0;

	result |= m_centronics->fault_r() << 3;
	result |= m_centronics->vcc_r() << 4; /* SELECT is connected to VCC */
	result |= !m_centronics->pe_r() << 5;
	result |= m_centronics->ack_r() << 6;
	result |= !m_centronics->busy_r() << 7;

	return result;
}


READ8_MEMBER( pc_lpt_device::control_r )
{
	UINT8 result = 0;

	/* return latch state */
	result |= m_strobe << 0;
	result |= m_autofd << 1;
	result |= m_init << 2;
	result |= m_select << 3;
	result |= m_irq_enabled << 4;

	return result;
}


WRITE8_MEMBER( pc_lpt_device::control_w )
{
	//  logerror("pc_lpt_control_w: 0x%02x\n", data);

	/* save to latch */
	m_strobe = BIT(data, 0);
	m_autofd = BIT(data, 1);
	m_init = BIT(data, 2);
	m_select = BIT(data, 3);
	m_irq_enabled = BIT(data, 4);

	/* output to centronics */
	m_centronics->strobe_w(m_strobe);
	m_centronics->autofeed_w(m_autofd);
	m_centronics->init_prime_w(m_init);
}


READ8_MEMBER( pc_lpt_device::read )
{
	switch (offset)
	{
	case 0: return data_r(space, 0);
	case 1: return status_r(space, 0);
	case 2: return control_r(space, 0);
	}

	/* if we reach this its an error */
	logerror("PC-LPT %s: Read from invalid offset %x\n", tag(), offset);

	return 0xff;
}


WRITE8_MEMBER( pc_lpt_device::write )
{
	switch (offset)
	{
	case 0: data_w(space, 0, data); break;
	case 1: break;
	case 2: control_w(space, 0, data); break;
	}
}

const device_type PC_LPT = &device_creator<pc_lpt_device>;

pc_lpt_device::pc_lpt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_LPT, "PC-LPT", tag, owner, clock, "pc_lpt", __FILE__),
	m_data(0),
	m_ack(0),
	m_strobe(0),
	m_autofd(0),
	m_init(0),
	m_select(0),
	m_irq_enabled(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pc_lpt_device::device_config_complete()
{
	// inherit a copy of the static data
	const pc_lpt_interface *intf = reinterpret_cast<const pc_lpt_interface *>(static_config());
	if (intf != NULL)
		*static_cast<pc_lpt_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq, 0, sizeof(m_out_irq));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc_lpt_device::device_start()
{
	/* get centronics device */
	m_centronics = subdevice<centronics_device>("centronics");
	assert(m_centronics != NULL);

	/* resolve callbacks */
	m_out_irq_func.resolve(m_out_irq, *this);

	/* register for state saving */
	save_item(NAME(m_ack));
	save_item(NAME(m_strobe));
	save_item(NAME(m_autofd));
	save_item(NAME(m_init));
	save_item(NAME(m_select));
	save_item(NAME(m_irq_enabled));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc_lpt_device::device_reset()
{
	m_strobe = TRUE;
	m_autofd = TRUE;
	m_init = FALSE;
	m_select = TRUE;
	m_irq_enabled = FALSE;
	m_data = 0xff;
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
	isa8_lpt_device *lpt  = downcast<isa8_lpt_device *>(device->owner());
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
	PORT_DIPSETTING(    0x00, "0x378" )
	PORT_DIPSETTING(    0x01, "0x278" )
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
		device_t(mconfig, ISA8_LPT, "Printer Adapter", tag, owner, clock, "isa_lpt", __FILE__),
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
		m_isa->install_device(0x0378, 0x037b, 0, 0, read8_delegate(FUNC(pc_lpt_device::read), subdevice<pc_lpt_device>("lpt")), write8_delegate(FUNC(pc_lpt_device::write), subdevice<pc_lpt_device>("lpt")));
	} else {
		m_isa->install_device(0x0278, 0x027b, 0, 0, read8_delegate(FUNC(pc_lpt_device::read), subdevice<pc_lpt_device>("lpt")), write8_delegate(FUNC(pc_lpt_device::write), subdevice<pc_lpt_device>("lpt")));
	}
}
