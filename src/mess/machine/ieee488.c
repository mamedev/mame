/**********************************************************************

    IEEE-488.1 General Purpose Interface Bus emulation
    (aka HP-IB, GPIB, CBM IEEE)

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "ieee488.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "EOI", "DAV", "NRFD", "NDAC", "IFC", "SRQ", "ATN", "REN" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IEEE488 = &device_creator<ieee488_device>;
const device_type IEEE488_SLOT = &device_creator<ieee488_slot_device>;



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ieee488_interface - constructor
//-------------------------------------------------

device_ieee488_interface::device_ieee488_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_ieee488_interface - destructor
//-------------------------------------------------

device_ieee488_interface::~device_ieee488_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_slot_device - constructor
//-------------------------------------------------

ieee488_slot_device::ieee488_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, IEEE488_SLOT, "IEEE-488 slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_set_slot -
//-------------------------------------------------

void ieee488_slot_device::static_set_slot(device_t &device, int address)
{
	ieee488_slot_device &ieee488_card = dynamic_cast<ieee488_slot_device &>(device);
	ieee488_card.m_address = address;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_slot_device::device_start()
{
	ieee488_device* bus = owner()->subdevice<ieee488_device>(IEEE488_TAG);
	device_ieee488_interface *dev = dynamic_cast<device_ieee488_interface *>(get_card_device());
	if (dev) bus->add_device(get_card_device(), m_address);
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ieee488_device::device_config_complete()
{
	// inherit a copy of the static data
	const ieee488_interface *intf = reinterpret_cast<const ieee488_interface *>(static_config());
	if (intf != NULL)
		*static_cast<ieee488_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_eoi_cb, 0, sizeof(m_out_eoi_cb));
		memset(&m_out_dav_cb, 0, sizeof(m_out_dav_cb));
		memset(&m_out_nrfd_cb, 0, sizeof(m_out_nrfd_cb));
		memset(&m_out_ndac_cb, 0, sizeof(m_out_ndac_cb));
		memset(&m_out_ifc_cb, 0, sizeof(m_out_ifc_cb));
		memset(&m_out_srq_cb, 0, sizeof(m_out_srq_cb));
		memset(&m_out_atn_cb, 0, sizeof(m_out_atn_cb));
		memset(&m_out_ren_cb, 0, sizeof(m_out_ren_cb));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

inline void ieee488_device::set_signal(device_t *device, int signal, int state)
{
	bool changed = false;

	if (device == this)
	{
		if (m_line[signal] != state)
		{
			if (LOG) logerror("%s IEEE488: '%s' %s %u\n", machine().describe_context(), tag(), SIGNAL_NAME[signal], state);
			m_line[signal] = state;
			changed = true;
		}
	}
	else
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!strcmp(entry->m_device->tag(), device->tag()))
			{
				if (entry->m_line[signal] != state)
				{
					if (LOG) logerror("%s IEEE488: '%s' %s %u\n", machine().describe_context(), device->tag(), SIGNAL_NAME[signal], state);
					entry->m_line[signal] = state;
					changed = true;
				}
			}

			entry = entry->next();
		}
	}

	if (changed)
	{
		switch (signal)
		{
		case EOI:	m_out_eoi_func(state);	break;
		case DAV:	m_out_dav_func(state);	break;
		case NRFD:	m_out_nrfd_func(state);	break;
		case NDAC:	m_out_ndac_func(state);	break;
		case IFC:	m_out_ifc_func(state);	break;
		case SRQ:	m_out_srq_func(state);	break;
		case ATN:	m_out_atn_func(state);	break;
		case REN:	m_out_ren_func(state);	break;
		}

		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			switch (signal)
			{
			case EOI:
				entry->m_interface->ieee488_eoi(state);
				break;

			case DAV:
				entry->m_interface->ieee488_dav(state);
				break;

			case NRFD:
				entry->m_interface->ieee488_nrfd(state);
				break;

			case NDAC:
				entry->m_interface->ieee488_ndac(state);
				break;

			case IFC:
				entry->m_interface->ieee488_ifc(state);
				break;

			case SRQ:
				entry->m_interface->ieee488_srq(state);
				break;

			case ATN:
				entry->m_interface->ieee488_atn(state);
				break;

			case REN:
				entry->m_interface->ieee488_ren(state);
				break;
			}

			entry = entry->next();
		}

		if (LOG) logerror("IEEE488: EOI %u DAV %u NRFD %u NDAC %u IFC %u SRQ %u ATN %u REN %u DIO %02x\n",
			get_signal(EOI), get_signal(DAV), get_signal(NRFD), get_signal(NDAC),
			get_signal(IFC), get_signal(SRQ), get_signal(ATN), get_signal(REN), get_data());
	}
}


//-------------------------------------------------
//  get_signal -
//-------------------------------------------------

inline int ieee488_device::get_signal(int signal)
{
	int state = m_line[signal];

	if (state)
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!entry->m_line[signal])
			{
				state = 0;
				break;
			}

			entry = entry->next();
		}
	}

	return state;
}


//-------------------------------------------------
//  set_data -
//-------------------------------------------------

inline void ieee488_device::set_data(device_t *device, UINT8 data)
{
	if (device == this)
	{
		if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), tag(), data);

		m_dio = data;
	}
	else
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!strcmp(entry->m_device->tag(), device->tag()))
			{
				if (entry->m_dio != data)
				{
					if (LOG) logerror("%s IEEE488: '%s' DIO %02x\n", machine().describe_context(), device->tag(), data);
					entry->m_dio = data;
				}
			}

			entry = entry->next();
		}
	}
}


//-------------------------------------------------
//  get_data -
//-------------------------------------------------

inline UINT8 ieee488_device::get_data()
{
	UINT8 data = m_dio;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		data &= entry->m_dio;

		entry = entry->next();
	}

	return data;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ieee488_device - constructor
//-------------------------------------------------

ieee488_device::ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, IEEE488, "IEEE488 bus", tag, owner, clock),
	  m_dio(0xff)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ieee488_device::device_start()
{
	// resolve callbacks
    m_out_eoi_func.resolve(m_out_eoi_cb, *this);
    m_out_dav_func.resolve(m_out_dav_cb, *this);
    m_out_nrfd_func.resolve(m_out_nrfd_cb, *this);
    m_out_ndac_func.resolve(m_out_ndac_cb, *this);
    m_out_ifc_func.resolve(m_out_ifc_cb, *this);
    m_out_srq_func.resolve(m_out_srq_cb, *this);
    m_out_atn_func.resolve(m_out_atn_cb, *this);
    m_out_ren_func.resolve(m_out_ren_cb, *this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ieee488_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void ieee488_device::add_device(device_t *target, int address)
{
	daisy_entry *entry = auto_alloc(machine(), daisy_entry(target));

	entry->m_interface->m_bus = this;
	entry->m_interface->m_address = address;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

ieee488_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(NULL),
	  m_device(device),
	  m_interface(NULL),
	  m_dio(0xff)
{
	for (int i = 0; i < SIGNAL_COUNT; i++)
	{
		m_line[i] = 1;
	}

	device->interface(m_interface);
}


//-------------------------------------------------
//  dio_r -
//-------------------------------------------------

UINT8 ieee488_device::dio_r()
{
	return get_data();
}


//-------------------------------------------------
//  dio_r -
//-------------------------------------------------

READ8_MEMBER( ieee488_device::dio_r )
{
	return get_data();
}


//-------------------------------------------------
//  eoi_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::eoi_r )
{
	return get_signal(EOI);
}


//-------------------------------------------------
//  dav_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::dav_r )
{
	return get_signal(DAV);
}


//-------------------------------------------------
//  nrfd_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::nrfd_r )
{
	return get_signal(NRFD);
}


//-------------------------------------------------
//  ndac_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::ndac_r )
{
	return get_signal(NDAC);
}


//-------------------------------------------------
//  ifc_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::ifc_r )
{
	return get_signal(IFC);
}


//-------------------------------------------------
//  srq_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::srq_r )
{
	return get_signal(SRQ);
}


//-------------------------------------------------
//  atn_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::atn_r )
{
	return get_signal(ATN);
}


//-------------------------------------------------
//  ren_r -
//-------------------------------------------------

READ_LINE_MEMBER( ieee488_device::ren_r )
{
	return get_signal(REN);
}


//-------------------------------------------------
//  dio_w -
//-------------------------------------------------

void ieee488_device::dio_w(UINT8 data)
{
	return set_data(this, data);
}


//-------------------------------------------------
//  dio_w -
//-------------------------------------------------

WRITE8_MEMBER( ieee488_device::dio_w )
{
	return set_data(this, data);
}


//-------------------------------------------------
//  eoi_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::eoi_w )
{
	set_signal(this, EOI, state);
}


//-------------------------------------------------
//  dav_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::dav_w )
{
	set_signal(this, DAV, state);
}


//-------------------------------------------------
//  nrfd_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::nrfd_w )
{
	set_signal(this, NRFD, state);
}


//-------------------------------------------------
//  ndac_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::ndac_w )
{
	set_signal(this, NDAC, state);
}


//-------------------------------------------------
//  ifc_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::ifc_w )
{
	set_signal(this, IFC, state);
}


//-------------------------------------------------
//  srq_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::srq_w )
{
	set_signal(this, SRQ, state);
}


//-------------------------------------------------
//  atn_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::atn_w )
{
	set_signal(this, ATN, state);
}


//-------------------------------------------------
//  ren_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( ieee488_device::ren_w )
{
	set_signal(this, REN, state);
}


//-------------------------------------------------
//  dio_w -
//-------------------------------------------------

void ieee488_device::dio_w(device_t *device, UINT8 data)
{
	return set_data(device, data);
}


//-------------------------------------------------
//  eoi_w -
//-------------------------------------------------

void ieee488_device::eoi_w(device_t *device, int state)
{
	set_signal(device, EOI, state);
}


//-------------------------------------------------
//  dav_w -
//-------------------------------------------------

void ieee488_device::dav_w(device_t *device, int state)
{
	set_signal(device, DAV, state);
}


//-------------------------------------------------
//  nrfd_w -
//-------------------------------------------------

void ieee488_device::nrfd_w(device_t *device, int state)
{
	set_signal(device, NRFD, state);
}


//-------------------------------------------------
//  ndac_w -
//-------------------------------------------------

void ieee488_device::ndac_w(device_t *device, int state)
{
	set_signal(device, NDAC, state);
}


//-------------------------------------------------
//  ifc_w -
//-------------------------------------------------

void ieee488_device::ifc_w(device_t *device, int state)
{
	set_signal(device, IFC, state);
}


//-------------------------------------------------
//  srq_w -
//-------------------------------------------------

void ieee488_device::srq_w(device_t *device, int state)
{
	set_signal(device, SRQ, state);
}


//-------------------------------------------------
//  atn_w -
//-------------------------------------------------

void ieee488_device::atn_w(device_t *device, int state)
{
	set_signal(device, ATN, state);
}


//-------------------------------------------------
//  ren_w -
//-------------------------------------------------

void ieee488_device::ren_w(device_t *device, int state)
{
	set_signal(device, REN, state);
}
