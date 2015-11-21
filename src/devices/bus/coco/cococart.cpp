// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.c

    CoCo/Dragon cartridge management

*********************************************************************/

#include "emu.h"
#include "cococart.h"
#include "emuopts.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_LINE                0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCOCART_SLOT = &device_creator<cococart_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cococart_slot_device - constructor
//-------------------------------------------------
cococart_slot_device::cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, COCOCART_SLOT, "CoCo Cartridge Slot", tag, owner, clock, "cococart_slot", __FILE__),
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
	m_cart_line.value           = COCOCART_LINE_VALUE_CLEAR;
	m_cart_line.line            = 0;
	m_cart_line.q_count         = 0;
	m_cart_callback.resolve();
	m_cart_line.callback = &m_cart_callback;

	m_nmi_line.timer_index      = 0;
	/* 12 allowed one more instruction to finished after the line is pulled */
	m_nmi_line.delay            = 12;
	m_nmi_line.value            = COCOCART_LINE_VALUE_CLEAR;
	m_nmi_line.line             = 0;
	m_nmi_line.q_count          = 0;
	m_nmi_callback.resolve();
	m_nmi_line.callback = &m_nmi_callback;

	m_halt_line.timer_index     = 0;
	/* 6 allowed one more instruction to finished after the line is pulled */
	m_halt_line.delay           = 6;
	m_halt_line.value           = COCOCART_LINE_VALUE_CLEAR;
	m_halt_line.line            = 0;
	m_halt_line.q_count         = 0;
	m_halt_callback.resolve();
	m_halt_line.callback = &m_halt_callback;

	m_cart = dynamic_cast<device_cococart_interface *>(get_card_device());
}



//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cococart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void cococart_slot_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_CART:
			set_line("CART", m_cart_line, (cococart_line_value) param);
			break;

		case TIMER_NMI:
			set_line("NMI", m_nmi_line, (cococart_line_value) param);
			break;

		case TIMER_HALT:
			set_line("HALT", m_halt_line, (cococart_line_value) param);
			break;
	}
}



//-------------------------------------------------
//  coco_cartridge_r
//-------------------------------------------------

READ8_MEMBER(cococart_slot_device::read)
{
	UINT8 result = 0x00;
	if (m_cart)
		result = m_cart->read(space, offset);
	return result;
}


//-------------------------------------------------
//  coco_cartridge_w
//-------------------------------------------------

WRITE8_MEMBER(cococart_slot_device::write)
{
	if (m_cart)
		m_cart->write(space, offset, data);
}



//-------------------------------------------------
//  line_value_string
//-------------------------------------------------

static const char *line_value_string(cococart_line_value value)
{
	const char *s = NULL;
	switch(value)
	{
		case COCOCART_LINE_VALUE_CLEAR:
			s = "CLEAR";
			break;
		case COCOCART_LINE_VALUE_ASSERT:
			s = "ASSERT";
			break;
		case COCOCART_LINE_VALUE_Q:
			s = "Q";
			break;
		default:
			fatalerror("Invalid value\n");
	}
	return s;
}



//-------------------------------------------------
//  set_line
//-------------------------------------------------

void cococart_slot_device::set_line(const char *line_name, coco_cartridge_line &line, cococart_line_value value)
{
	if ((line.value != value) || (value == COCOCART_LINE_VALUE_Q))
	{
		line.value = value;

		if (LOG_LINE)
			logerror("[%s]: set_line(): %s <= %s\n", machine().describe_context(), line_name, line_value_string(value));
		/* engage in a bit of gymnastics for this odious 'Q' value */
		switch(line.value)
		{
			case COCOCART_LINE_VALUE_CLEAR:
				line.line = 0x00;
				line.q_count = 0;
				break;

			case COCOCART_LINE_VALUE_ASSERT:
				line.line = 0x01;
				line.q_count = 0;
				break;

			case COCOCART_LINE_VALUE_Q:
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
//  set_line_timer()
//-------------------------------------------------

void cococart_slot_device::set_line_timer(coco_cartridge_line &line, cococart_line_value value)
{
	/* calculate delay; delay dependant on cycles per second */
	attotime delay = (line.delay != 0)
		? machine().firstcpu->cycles_to_attotime(line.delay)
		: attotime::zero;

	line.timer[line.timer_index]->adjust(delay, (int) value);
	line.timer_index = (line.timer_index + 1) % TIMER_POOL;
}



//-------------------------------------------------
//  twiddle_line_if_q
//-------------------------------------------------

void cococart_slot_device::twiddle_line_if_q(coco_cartridge_line &line)
{
	if (line.value == COCOCART_LINE_VALUE_Q)
	{
		line.q_count = 0;
		set_line_timer(line, COCOCART_LINE_VALUE_Q);
	}
}



//-------------------------------------------------
//  coco_cartridge_twiddle_q_lines - hack to
//  support twiddling the Q line
//-------------------------------------------------

void cococart_slot_device::twiddle_q_lines()
{
	twiddle_line_if_q(m_cart_line);
	twiddle_line_if_q(m_nmi_line);
	twiddle_line_if_q(m_halt_line);
}


//-------------------------------------------------
//  coco_cartridge_set_line
//-------------------------------------------------

void cococart_slot_device::cart_set_line(cococart_line line, cococart_line_value value)
{
	switch (line)
	{
		case COCOCART_LINE_CART:
			set_line_timer(m_cart_line, value);
			break;

		case COCOCART_LINE_NMI:
			set_line_timer(m_nmi_line, value);
			break;

		case COCOCART_LINE_HALT:
			set_line_timer(m_halt_line, value);
			break;

		case COCOCART_LINE_SOUND_ENABLE:
			// do nothing for now
			break;
	}
}



//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

UINT8* cococart_slot_device::get_cart_base()
{
	if (m_cart != NULL)
		return m_cart->get_cart_base();
	return NULL;
}



//-------------------------------------------------
//  set_cart_base_update
//-------------------------------------------------

void cococart_slot_device::set_cart_base_update(cococart_base_update_delegate update)
{
	if (m_cart != NULL)
		m_cart->set_cart_base_update(update);
}



//-------------------------------------------------
//  call_load
//-------------------------------------------------

bool cococart_slot_device::call_load()
{
	if (m_cart)
	{
		offs_t read_length = 0;
		if (software_entry() == NULL)
		{
			read_length = fread(m_cart->get_cart_base(), 0x8000);
		}
		else
		{
			read_length = get_software_region_length("rom");
			memcpy(m_cart->get_cart_base(), get_software_region("rom"), read_length);
		}
		while(read_length < 0x8000)
		{
			offs_t len = MIN(read_length, 0x8000 - read_length);
			memcpy(m_cart->get_cart_base() + read_length, m_cart->get_cart_base(), len);
			read_length += len;
		}
	}
	return IMAGE_INIT_PASS;
}



//-------------------------------------------------
//  call_softlist_load
//-------------------------------------------------

bool cococart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}



//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

void cococart_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "pak");
}




//**************************************************************************
//  DEVICE COCO CART  INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cococart_interface - constructor
//-------------------------------------------------

device_cococart_interface::device_cococart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}



//-------------------------------------------------
//  ~device_cococart_interface - destructor
//-------------------------------------------------

device_cococart_interface::~device_cococart_interface()
{
}



//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER(device_cococart_interface::read)
{
	return 0x00;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(device_cococart_interface::write)
{
}



//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

UINT8* device_cococart_interface::get_cart_base()
{
	return NULL;
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
}
