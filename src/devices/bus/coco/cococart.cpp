// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    cococart.c

    CoCo/Dragon cartridge management

*********************************************************************/

#include "emu.h"
#include "cococart.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_LINE                0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCOCART_SLOT = device_creator<cococart_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cococart_slot_device - constructor
//-------------------------------------------------
cococart_slot_device::cococart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
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
	m_cart_line.value           = line_value::CLEAR;
	m_cart_line.line            = 0;
	m_cart_line.q_count         = 0;
	m_cart_callback.resolve();
	m_cart_line.callback = &m_cart_callback;

	m_nmi_line.timer_index      = 0;
	/* 12 allowed one more instruction to finished after the line is pulled */
	m_nmi_line.delay            = 12;
	m_nmi_line.value            = line_value::CLEAR;
	m_nmi_line.line             = 0;
	m_nmi_line.q_count          = 0;
	m_nmi_callback.resolve();
	m_nmi_line.callback = &m_nmi_callback;

	m_halt_line.timer_index     = 0;
	/* 6 allowed one more instruction to finished after the line is pulled */
	m_halt_line.delay           = 6;
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
//  coco_cartridge_r
//-------------------------------------------------

READ8_MEMBER(cococart_slot_device::read)
{
	uint8_t result = 0x00;
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
			fatalerror("Invalid value\n");
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
//  set_line_timer()
//-------------------------------------------------

void cococart_slot_device::set_line_timer(coco_cartridge_line &line, cococart_slot_device::line_value value)
{
	// calculate delay; delay dependant on cycles per second
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
	if (line.value == line_value::Q)
	{
		line.q_count = 0;
		set_line_timer(line, line_value::Q);
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

void cococart_slot_device::cart_set_line(cococart_slot_device::line which, cococart_slot_device::line_value value)
{
	switch (which)
	{
		case line::CART:
			set_line_timer(m_cart_line, value);
			break;

		case line::NMI:
			set_line_timer(m_nmi_line, value);
			break;

		case line::HALT:
			set_line_timer(m_halt_line, value);
			break;

		case line::SOUND_ENABLE:
			if (m_cart)
				m_cart->set_sound_enable(value != cococart_slot_device::line_value::CLEAR);
			break;
	}
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
//  set_cart_base_update
//-------------------------------------------------

void cococart_slot_device::set_cart_base_update(cococart_base_update_delegate update)
{
	if (m_cart != nullptr)
		m_cart->set_cart_base_update(update);
}



//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result cococart_slot_device::call_load()
{
	if (m_cart)
	{
		offs_t read_length;
		if (!loaded_through_softlist())
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
			offs_t len = std::min(read_length, 0x8000 - read_length);
			memcpy(m_cart->get_cart_base() + read_length, m_cart->get_cart_base(), len);
			read_length += len;
		}
	}
	return image_init_result::PASS;
}



//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

std::string cococart_slot_device::get_default_card_software()
{
	return software_get_default_slot("pak");
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
