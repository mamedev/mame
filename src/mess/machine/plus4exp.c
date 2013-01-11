/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/plus4exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PLUS4_EXPANSION_SLOT = &device_creator<plus4_expansion_slot_device>;



//**************************************************************************
//  DEVICE PLUS4_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_expansion_card_interface - constructor
//-------------------------------------------------

device_plus4_expansion_card_interface::device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_c1l(NULL),
		m_c1h(NULL),
		m_c2l(NULL),
		m_c2h(NULL),
		m_ram(NULL),
		m_nvram(NULL),
		m_nvram_size(0),
		m_c1l_mask(0),
		m_c1h_mask(0),
		m_c2l_mask(0),
		m_c2h_mask(0),
		m_ram_mask(0)
{
	m_slot = dynamic_cast<plus4_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_plus4_expansion_card_interface - destructor
//-------------------------------------------------

device_plus4_expansion_card_interface::~device_plus4_expansion_card_interface()
{
}


//-------------------------------------------------
//  plus4_c1l_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c1l_pointer(running_machine &machine, size_t size)
{
	if (m_c1l == NULL)
	{
		m_c1l = auto_alloc_array(machine, UINT8, size);

		m_c1l_mask = size - 1;
	}

	return m_c1l;
}


//-------------------------------------------------
//  plus4_c1h_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c1h_pointer(running_machine &machine, size_t size)
{
	if (m_c1h == NULL)
	{
		m_c1h = auto_alloc_array(machine, UINT8, size);

		m_c1h_mask = size - 1;
	}

	return m_c1h;
}


//-------------------------------------------------
//  plus4_c2l_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c2l_pointer(running_machine &machine, size_t size)
{
	if (m_c2l == NULL)
	{
		m_c2l = auto_alloc_array(machine, UINT8, size);

		m_c2l_mask = size - 1;
	}

	return m_c2l;
}


//-------------------------------------------------
//  plus4_c2h_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c2h_pointer(running_machine &machine, size_t size)
{
	if (m_c2h == NULL)
	{
		m_c2h = auto_alloc_array(machine, UINT8, size);

		m_c2h_mask = size - 1;
	}

	return m_c2h;
}


//-------------------------------------------------
//  plus4_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);

		m_ram_mask = size - 1;
	}

	return m_ram;
}


//-------------------------------------------------
//  plus4_ram_pointer - get NVRAM pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_nvram_pointer(running_machine &machine, size_t size)
{
	if (m_nvram == NULL)
	{
		m_nvram = auto_alloc_array(machine, UINT8, size);

		m_nvram_mask = size - 1;
		m_nvram_size = size;
	}

	return m_nvram;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_expansion_slot_device - constructor
//-------------------------------------------------

plus4_expansion_slot_device::plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PLUS4_EXPANSION_SLOT, "Expansion Port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  plus4_expansion_slot_device - destructor
//-------------------------------------------------

plus4_expansion_slot_device::~plus4_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void plus4_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const plus4_expansion_slot_interface *intf = reinterpret_cast<const plus4_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<plus4_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_dma_cd_cb, 0, sizeof(m_in_dma_cd_cb));
		memset(&m_out_dma_cd_cb, 0, sizeof(m_out_dma_cd_cb));
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_aec_cb, 0, sizeof(m_out_aec_cb));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_plus4_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_in_dma_cd_func.resolve(m_in_dma_cd_cb, *this);
	m_out_dma_cd_func.resolve(m_out_dma_cd_cb, *this);
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_aec_func.resolve(m_out_aec_cb, *this);

	// inherit bus clock
	if (clock() == 0)
	{
		plus4_expansion_slot_device *root = machine().device<plus4_expansion_slot_device>(PLUS4_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_expansion_slot_device::device_reset()
{
	breset_w(ASSERT_LINE);
	breset_w(CLEAR_LINE);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			// TODO
		}
		else
		{
			size = get_software_region_length("c1l");
			if (size) memcpy(m_cart->plus4_c1l_pointer(machine(), size), get_software_region("c1l"), size);

			size = get_software_region_length("c1h");
			if (size) memcpy(m_cart->plus4_c1h_pointer(machine(), size), get_software_region("c1h"), size);

			size = get_software_region_length("c2l");
			if (size) memcpy(m_cart->plus4_c2l_pointer(machine(), size), get_software_region("c2l"), size);

			size = get_software_region_length("c2h");
			if (size) memcpy(m_cart->plus4_c2h_pointer(machine(), size), get_software_region("c2h"), size);

			size = get_software_region_length("ram");
			if (size) memset(m_cart->plus4_ram_pointer(machine(), size), 0, size);

			size = get_software_region_length("nvram");
			if (size) memset(m_cart->plus4_nvram_pointer(machine(), size), 0, size);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * plus4_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 plus4_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_cart != NULL)
	{
		data = m_cart->plus4_cd_r(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void plus4_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_cart != NULL)
	{
		m_cart->plus4_cd_w(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}
}


//-------------------------------------------------
//  breset_w - buffered reset write
//-------------------------------------------------

WRITE_LINE_MEMBER( plus4_expansion_slot_device::breset_w )
{
	if (m_cart != NULL)
	{
		m_cart->plus4_breset_w(state);
	}
}


//-------------------------------------------------
//  dma_cd_r - DMA read
//-------------------------------------------------

UINT8 plus4_expansion_slot_device::dma_cd_r(offs_t offset)
{
	return m_in_dma_cd_func(offset);
}


//-------------------------------------------------
//  dma_cd_w - DMA write
//-------------------------------------------------

void plus4_expansion_slot_device::dma_cd_w(offs_t offset, UINT8 data)
{
	m_out_dma_cd_func(offset, data);
}


WRITE_LINE_MEMBER( plus4_expansion_slot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( plus4_expansion_slot_device::aec_w ) { m_out_aec_func(state); }


//-------------------------------------------------
//  phi2 - system clock frequency
//-------------------------------------------------

int plus4_expansion_slot_device::phi2()
{
	return clock();
}
