// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.cpp

    CoCo/Dragon cartridge slot - typically used for "Program Paks"
    (which are simple ROMs) but in practice is the main extensibility
    mechanism for CoCo hardware

    CoCo and Dragon pinout listing
       ---  -------            ---  -------
         1  -12V                21  A2
         2  +12V                22  A3
         3  HALT                23  A4
         4  NMI                 24  A5
         5  RESET               25  A6
         6  EIN                 26  A7
         7  QIN                 27  A8
         8  CART                28  A9
         9  +5V                 29  A10
        10  D0                  30  A11
        11  D1                  31  A12
        12  D2                  32  CTS
        13  D3                  33  GND
        14  D4                  34  GND
        15  D5                  35  SND
        16  D6                  36  SCS
        17  D7                  37  A13
        18  R/!W                38  A14
        19  A0                  39  A15
        20  A1                  40  SLENB

    Notes:
        CTS - ROM read $C000-$FEFF ($FDFF on CoCo 3)
        SCS - Spare Chip Select:  IO space between $FF40-5F

*********************************************************************/

#include "emu.h"
#include "cococart.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_LINE                0


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	TIMER_CART,
	TIMER_NMI,
	TIMER_HALT
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCOCART_SLOT, cococart_slot_device, "cococart_slot", "CoCo Cartridge Slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cococart_slot_device - constructor
//-------------------------------------------------
cococart_slot_device::cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COCOCART_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_cart_callback(*this),
	m_nmi_callback(*this),
	m_halt_callback(*this), m_cart(nullptr)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cococart_slot_device::device_start()
{
	for(int i=0; i<TIMER_POOL; i++ )
	{
		m_cart_line.timer[i]    = timer_alloc(TIMER_CART);
		m_nmi_line.timer[i]     = timer_alloc(TIMER_NMI);
		m_halt_line.timer[i]    = timer_alloc(TIMER_HALT);
	}

	m_cart_line.timer_index     = 0;
	m_cart_line.delay           = 0;
	m_cart_line.value           = line_value::CLEAR;
	m_cart_line.line            = 0;
	m_cart_line.q_count         = 0;
	m_cart_callback.resolve();
	m_cart_line.callback = &m_cart_callback;

	m_nmi_line.timer_index      = 0;
	m_nmi_line.delay            = 0;
	m_nmi_line.value            = line_value::CLEAR;
	m_nmi_line.line             = 0;
	m_nmi_line.q_count          = 0;
	m_nmi_callback.resolve();
	m_nmi_line.callback = &m_nmi_callback;

	m_halt_line.timer_index     = 0;
	m_halt_line.delay           = 0;
	m_halt_line.value           = line_value::CLEAR;
	m_halt_line.line            = 0;
	m_halt_line.q_count         = 0;
	m_halt_callback.resolve();
	m_halt_line.callback = &m_halt_callback;

	m_cart = dynamic_cast<device_cococart_interface *>(get_card_device());
}



//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void cococart_slot_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_CART:
			set_line("CART", m_cart_line, (line_value) param);
			break;

		case TIMER_NMI:
			set_line("NMI", m_nmi_line, (line_value) param);
			break;

		case TIMER_HALT:
			set_line("HALT", m_halt_line, (line_value) param);
			break;
	}
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(cococart_slot_device::cts_read)
{
	uint8_t result = 0x00;
	if (m_cart)
		result = m_cart->cts_read(space, offset);
	return result;
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

WRITE8_MEMBER(cococart_slot_device::cts_write)
{
	if (m_cart)
		m_cart->cts_write(space, offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

READ8_MEMBER(cococart_slot_device::scs_read)
{
	uint8_t result = 0x00;
	if (m_cart)
		result = m_cart->scs_read(space, offset);
	return result;
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(cococart_slot_device::scs_write)
{
	if (m_cart)
		m_cart->scs_write(space, offset, data);
}



//-------------------------------------------------
//  line_value_string
//-------------------------------------------------

const char *cococart_slot_device::line_value_string(line_value value)
{
	const char *s;
	switch(value)
	{
		case line_value::CLEAR:
			s = "CLEAR";
			break;
		case line_value::ASSERT:
			s = "ASSERT";
			break;
		case line_value::Q:
			s = "Q";
			break;
		default:
			throw false && "Invalid value";
	}
	return s;
}


//-------------------------------------------------
//  set_line
//-------------------------------------------------

void cococart_slot_device::set_line(const char *line_name, coco_cartridge_line &line, cococart_slot_device::line_value value)
{
	if ((line.value != value) || (value == line_value::Q))
	{
		line.value = value;

		if (LOG_LINE)
			logerror("[%s]: set_line(): %s <= %s\n", machine().describe_context(), line_name, line_value_string(value));

		// engage in a bit of gymnastics for this odious 'Q' value
		switch(line.value)
		{
			case line_value::CLEAR:
				line.line = 0x00;
				line.q_count = 0;
				break;

			case line_value::ASSERT:
				line.line = 0x01;
				line.q_count = 0;
				break;

			case line_value::Q:
				line.line = line.line ? 0x00 : 0x01;
				if (line.q_count++ < 4)
					set_line_timer(line, value);
				break;
		}

		/* invoke the callback, if present */
		if (!(*line.callback).isnull())
			(*line.callback)(line.line);
	}
}


//-------------------------------------------------
//  set_line_timer
//-------------------------------------------------

void cococart_slot_device::set_line_timer(coco_cartridge_line &line, cococart_slot_device::line_value value)
{
	// calculate delay; delay dependant on cycles per second
	attotime delay = (line.delay != 0)
		? clocks_to_attotime(line.delay)
		: attotime::zero;

	line.timer[line.timer_index]->adjust(delay, (int) value);
	line.timer_index = (line.timer_index + 1) % TIMER_POOL;
}


//-------------------------------------------------
//  twiddle_line_if_q
//-------------------------------------------------

void cococart_slot_device::twiddle_line_if_q(coco_cartridge_line &line)
{
	if (line.value == line_value::Q)
	{
		line.q_count = 0;
		set_line_timer(line, line_value::Q);
	}
}


//-------------------------------------------------
//  twiddle_q_lines - hack to support twiddling the
//  Q line
//-------------------------------------------------

void cococart_slot_device::twiddle_q_lines()
{
	twiddle_line_if_q(m_cart_line);
	twiddle_line_if_q(m_nmi_line);
	twiddle_line_if_q(m_halt_line);
}


//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void cococart_slot_device::set_line_value(cococart_slot_device::line which, cococart_slot_device::line_value value)
{
	switch (which)
	{
	case cococart_slot_device::line::CART:
		set_line_timer(m_cart_line, value);
		break;

	case cococart_slot_device::line::NMI:
		set_line_timer(m_nmi_line, value);
		break;

	case cococart_slot_device::line::HALT:
		set_line_timer(m_halt_line, value);
		break;

	case cococart_slot_device::line::SOUND_ENABLE:
		if (m_cart)
			m_cart->set_sound_enable(value != cococart_slot_device::line_value::CLEAR);
		break;
	}
}


//-------------------------------------------------
//  set_line_delay
//-------------------------------------------------

void cococart_slot_device::set_line_delay(cococart_slot_device::line which, int cycles)
{
	switch (which)
	{
	case cococart_slot_device::line::CART:
		m_cart_line.delay = cycles;
		break;

	case cococart_slot_device::line::NMI:
		m_nmi_line.delay = cycles;
		break;

	case cococart_slot_device::line::HALT:
		m_halt_line.delay = cycles;
		break;

	default:
		throw false;
	}
}


//-------------------------------------------------
//  get_line_value
//-------------------------------------------------

cococart_slot_device::line_value cococart_slot_device::get_line_value(cococart_slot_device::line which) const
{
	line_value result;
	switch (which)
	{
	case cococart_slot_device::line::CART:
		result = m_cart_line.value;
		break;

	case cococart_slot_device::line::NMI:
		result = m_nmi_line.value;
		break;

	case cococart_slot_device::line::HALT:
		result = m_halt_line.value;
		break;

	default:
		result = line_value::CLEAR;
		break;
	}
	return result;
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

uint8_t* cococart_slot_device::get_cart_base()
{
	if (m_cart != nullptr)
		return m_cart->get_cart_base();
	return nullptr;
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

uint32_t cococart_slot_device::get_cart_size()
{
	if (m_cart != nullptr)
		return m_cart->get_cart_size();

	return 0x8000;
}

//-------------------------------------------------
//  set_cart_base_update
//-------------------------------------------------

void cococart_slot_device::set_cart_base_update(cococart_base_update_delegate update)
{
	if (m_cart)
		m_cart->set_cart_base_update(update);
}


//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result cococart_slot_device::call_load()
{
	if (m_cart)
	{
		memory_region *cart_mem = m_cart->get_cart_memregion();
		uint8_t *base = cart_mem->base();
		offs_t read_length, cart_length = cart_mem->bytes();

		if (!loaded_through_softlist())
		{
			read_length = fread(base, cart_length);
		}
		else
		{
			read_length = get_software_region_length("rom");
			memcpy(base, get_software_region("rom"), read_length);
		}

		while (read_length < cart_length)
		{
			offs_t len = std::min(read_length, m_cart->get_cart_size() - read_length);
			memcpy(base + read_length, base, len);
			read_length += len;
		}
	}
	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

std::string cococart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("pak");
}



//**************************************************************************
//  DEVICE COCO CART INTERFACE - Implemented by devices that plug into
//  CoCo cartridge slots
//**************************************************************************

template class device_finder<device_cococart_interface, false>;
template class device_finder<device_cococart_interface, true>;

//-------------------------------------------------
//  device_cococart_interface - constructor
//-------------------------------------------------

device_cococart_interface::device_cococart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_owning_slot(nullptr)
	, m_host(nullptr)
{
}


//-------------------------------------------------
//  ~device_cococart_interface - destructor
//-------------------------------------------------

device_cococart_interface::~device_cococart_interface()
{
}


//-------------------------------------------------
//  interface_config_complete
//-------------------------------------------------

void device_cococart_interface::interface_config_complete()
{
	m_owning_slot = dynamic_cast<cococart_slot_device *>(device().owner());
	m_host = m_owning_slot
		? dynamic_cast<device_cococart_host_interface *>(m_owning_slot->owner())
		: nullptr;
}


//-------------------------------------------------
//  interface_pre_start
//-------------------------------------------------

void device_cococart_interface::interface_pre_start()
{
	if (!m_owning_slot)
		throw emu_fatalerror("Expected device().owner() to be of type cococart_slot_device");
	if (!m_host)
		throw emu_fatalerror("Expected m_owning_slot->owner() to be of type device_cococart_host_interface");
}


//-------------------------------------------------
//  cts_read - Signifies a read where the CTS pin
//  on the cartridge slot was asserted ($C000-FFEF)
//-------------------------------------------------

READ8_MEMBER(device_cococart_interface::cts_read)
{
	memory_region *cart_mem = get_cart_memregion();
	offs_t cart_length = cart_mem->bytes();

	if (cart_mem)
		return cart_mem->base()[offset & (cart_length - 1)];
	else
		return 0x00;
}


//-------------------------------------------------
//  cts_write - Signifies a write where the CTS pin
//  on the cartridge slot was asserted ($C000-FFEF)
//-------------------------------------------------

WRITE8_MEMBER(device_cococart_interface::cts_write)
{
}


//-------------------------------------------------
//  scs_read - Signifies a read where the SCS pin
//  on the cartridge slot was asserted ($FF40-5F)
//-------------------------------------------------

READ8_MEMBER(device_cococart_interface::scs_read)
{
	return 0x00;
}


//-------------------------------------------------
//  scs_write - Signifies a write where the SCS pin
//  on the cartridge slot was asserted ($FF40-5F)
//-------------------------------------------------

WRITE8_MEMBER(device_cococart_interface::scs_write)
{
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void device_cococart_interface::set_sound_enable(bool sound_enable)
{
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

uint8_t* device_cococart_interface::get_cart_base()
{
	return nullptr;
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

uint32_t device_cococart_interface::get_cart_size()
{
	return 0x8000;
}


//-------------------------------------------------
//  set_cart_base_update
//-------------------------------------------------

void device_cococart_interface::set_cart_base_update(cococart_base_update_delegate update)
{
	m_update = update;
}


//-------------------------------------------------
//  cart_base_changed
//-------------------------------------------------

void device_cococart_interface::cart_base_changed(void)
{
	if (!m_update.isnull())
		m_update(get_cart_base());
	else
	{
		// propagate up to host interface
		device_cococart_interface *host = dynamic_cast<device_cococart_interface*>(m_host);
		host->cart_base_changed();
	}
}

/*-------------------------------------------------
    get_cart_memregion
-------------------------------------------------*/

memory_region* device_cococart_interface::get_cart_memregion()
{
	return 0;
}

//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &device_cococart_interface::cartridge_space()
{
	return host().cartridge_space();
}


//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void device_cococart_interface::set_line_value(cococart_slot_device::line line, cococart_slot_device::line_value value)
{
	owning_slot().set_line_value(line, value);
}
