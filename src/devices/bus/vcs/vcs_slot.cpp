// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Cowering
/***********************************************************************************************************


    Atari VCS 2600 cart emulation
    (through slot devices)

    Emulation of the cartslot for Atari 2600
    Several banking schemes have been used for larger roms,
    and some carts contained RAM (so called "Special Chip")

    Mapper identification routines based on Cowering's code.

 ***********************************************************************************************************/


#include "emu.h"
#include "vcs_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VCS_CART_SLOT = &device_creator<vcs_cart_slot_device>;


//-------------------------------------------------
//  device_vcs_cart_interface - constructor
//-------------------------------------------------

device_vcs_cart_interface::device_vcs_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_vcs_cart_interface - destructor
//-------------------------------------------------

device_vcs_cart_interface::~device_vcs_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_vcs_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(A26SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_vcs_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
	device().save_item(NAME(m_ram));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_cart_slot_device - constructor
//-------------------------------------------------
vcs_cart_slot_device::vcs_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, VCS_CART_SLOT, "Atari VCS 2600 Cartridge Slot", tag, owner, clock, "vcs_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr), m_type(0)
{
}


//-------------------------------------------------
//  vcs_cart_slot_device - destructor
//-------------------------------------------------

vcs_cart_slot_device::~vcs_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_vcs_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vcs_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

//-------------------------------------------------
//  VCS PCBs
//-------------------------------------------------

struct vcs_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const vcs_slot slot_list[] =
{
	{ A26_2K, "a26_2k" },
	{ A26_4K, "a26_4k" },
	{ A26_F4, "a26_f4" },
	{ A26_F6, "a26_f6" },
	{ A26_F8, "a26_f8" },
	{ A26_F8SW, "a26_f8sw" },
	{ A26_FA, "a26_fa" },
	{ A26_FE, "a26_fe" },
	{ A26_E0, "a26_e0" },
	{ A26_E7, "a26_e7" },
	{ A26_3E, "a26_3e" },
	{ A26_3F, "a26_3f" },
	{ A26_UA, "a26_ua" },
	{ A26_CV, "a26_cv" },
	{ A26_DC, "a26_dc" },
	{ A26_FV, "a26_fv" },
	{ A26_JVP, "a26_jvp" },
	{ A26_CM, "a26_cm" },
	{ A26_SS, "a26_ss" },
	{ A26_DPC, "a26_dpc" },
	{ A26_4IN1, "a26_4in1" },
	{ A26_8IN1, "a26_8in1" },
	{ A26_32IN1, "a26_32in1" },
	{ A26_X07, "a26_x07" },
	{ A26_HARMONY, "a26_harmony" },
};

static int vcs_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *vcs_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "a26_4k";
}

bool vcs_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM;
		UINT32 len;

		if (software_entry() != nullptr)
			len = get_software_region_length("rom");
		else
			len = length();

		//printf("Size: 0x%X\n", len);

		// check that filesize is among the supported ones
		switch (len)
		{
			case 0x00800:
			case 0x01000:
			case 0x02000:
			case 0x028ff:
			case 0x02900:
			case 0x03000:
			case 0x04000:
			case 0x08000:
			case 0x10000:
			case 0x80000:
				break;

			default:
				seterror(IMAGE_ERROR_UNSUPPORTED, "Invalid rom file size" );
				return IMAGE_INIT_FAIL;
		}

		m_cart->rom_alloc(len, tag());
		ROM = m_cart->get_rom_base();

		if (software_entry() != nullptr)
		{
			const char *pcb_name;
			bool has_ram = get_software_region("ram") ? TRUE : FALSE;
			memcpy(ROM, get_software_region("rom"), len);

			if ((pcb_name = get_feature("slot")) != nullptr)
				m_type = vcs_get_pcb_id(pcb_name);
			else
			{
				// identify type based on size
				switch (len)
				{
					case 0x800:
						m_type = A26_2K;
						break;
					case 0x1000:
						m_type = A26_4K;
						break;
					case 0x2000:
						m_type = A26_F8;
						break;
					case 0x28ff:
					case 0x2900:
						m_type = A26_DPC;
						break;
					case 0x3000:
						m_type = A26_FA;
						break;
					case 0x4000:
						m_type = A26_F6;
						break;
					case 0x8000:
						m_type = A26_F4;
						break;
					case 0x10000:
						m_type = A26_32IN1;
						break;
					case 0x80000:
						m_type = A26_3F;
						break;
					default:
						m_type = A26_4K;
						printf("Unrecognized cart type!\n");
						break;
				}
			}

			if (has_ram)
				m_cart->ram_alloc(get_software_region_length("ram"));
		}
		else
		{
			fread(ROM, len);
			m_type = identify_cart_type(ROM, len);

			// check for Special Chip (128bytes of RAM)
			if (len == 0x2000 || len == 0x4000 || len == 0x8000)
				if (detect_super_chip(ROM, len))
				{
					m_cart->ram_alloc(0x80);
					//printf("Super Chip detected!\n");
				}
			// Super chip games:
			// dig dig, crystal castles, millipede, stargate, defender ii, jr. Pac Man,
			// desert falcon, dark chambers, super football, sprintmaster, fatal run,
			// off the wall, shooting arcade, secret quest, radar lock, save mary, klax

			// add CBS RAM+ (256bytes of RAM)
			if (m_type == A26_FA)
				m_cart->ram_alloc(0x100);
			// add M Network RAM
			else if (m_type == A26_E7)
				m_cart->ram_alloc(0x800);
			// add Commavid RAM
			else if (m_type == A26_CV)
				m_cart->ram_alloc(0x400);
			// add Starpath Superchager RAM
			else if (m_type == A26_SS)
				m_cart->ram_alloc(0x1800);
			// add Boulder Dash RAM
			else if (m_type == A26_3E)
				m_cart->ram_alloc(0x8000);
		}

		//printf("Type: %s\n", vcs_get_slot(m_type));

		// pass a pointer to the now allocated ROM for the DPC chip
		if (m_type == A26_DPC)
			m_cart->setup_addon_ptr((UINT8 *)m_cart->get_rom_base() + 0x2000);

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void vcs_cart_slot_device::call_unload()
{
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool vcs_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}


/*-------------------------------------------------
  detection helper routines
 -------------------------------------------------*/

int vcs_cart_slot_device::detect_modeDC(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	// signature is also in 'video reflex'.. maybe figure out that controller port someday...
	static const unsigned char signature[3] = { 0x8d, 0xf0, 0xff };

	if (len == 0x10000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeF6(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signature[3] = { 0x8d, 0xf6, 0xff };

	if (len == 0x4000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_snowhite(UINT8 *cart, UINT32 len)
{
	static const unsigned char snowwhite[] = { 0x10, 0xd0, 0xff, 0xff }; // Snow White Proto

	if (len == 0x2000 && !memcmp(&cart[0x1ffc], snowwhite, sizeof(snowwhite)))
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_mode3E(UINT8 *cart, UINT32 len)
{
	// this one is a little hacky... looks for STY $3e, which is unique to
	// 'not boulderdash', but is the only example I have (cow)
	// Would have used STA $3e, but 'Alien' and 'Star Raiders' do that for unknown reasons
	int numfound = 0;
	static const unsigned char signature[3] = { 0x84, 0x3e, 0x9d };

	if (len == 0x0800 || len == 0x1000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeSS(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signature[5] = { 0xbd, 0xe5, 0xff, 0x95, 0x81 };

	if (len == 0x0800 || len == 0x1000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeFE(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][5] =  {
									{ 0x20, 0x00, 0xd0, 0xc6, 0xc5 },
									{ 0x20, 0xc3, 0xf8, 0xa5, 0x82 },
									{ 0xd0, 0xfb, 0x20, 0x73, 0xfe },
									{ 0x20, 0x00, 0xf0, 0x84, 0xd6 }
	};

	if (len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeE0(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][3] =  {
									{ 0x8d, 0xe0, 0x1f },
									{ 0x8d, 0xe0, 0x5f },
									{ 0x8d, 0xe9, 0xff },
									{ 0xad, 0xe9, 0xff },
									{ 0xad, 0xed, 0xff },
									{ 0xad, 0xf3, 0xbf }
	};

	if (len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeCV(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][3] = {
									{ 0x9d, 0xff, 0xf3 },
									{ 0x99, 0x00, 0xf4 }
	};

	if (len == 0x0800 || len == 0x1000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeFV(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][3] = { { 0x2c, 0xd0, 0xff } };

	if (len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeJVP(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][4] = {
									{ 0x2c, 0xc0, 0xef, 0x60 },
									{ 0x8d, 0xa0, 0x0f, 0xf0 }
	};

	if (len == 0x4000 || len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeE7(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signatures[][3] = {
									{ 0xad, 0xe5, 0xff },
									{ 0x8d, 0xe7, 0xff }
	};

	if (len == 0x2000 || len == 0x4000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (int j = 0; j < (sizeof signatures/sizeof signatures[0]) && !numfound; j++)
			{
				if (!memcmp(&cart[i], &signatures[j], sizeof signatures[0]))
				{
					numfound = 1;
				}
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_modeUA(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signature[3] = { 0x8d, 0x40, 0x02 };

	if (len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound = 1;
			}
		}
	}
	if (numfound)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_8K_mode3F(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signature1[4] = { 0xa9, 0x01, 0x85, 0x3f };
	static const unsigned char signature2[4] = { 0xa9, 0x02, 0x85, 0x3f };
	// have to look for two signatures because 'not boulderdash' gives false positive otherwise

	if (len == 0x2000)
	{
		for (int i = 0; i < len - sizeof signature1; i++)
		{
			if (!memcmp(&cart[i], signature1, sizeof signature1))
			{
				numfound |= 0x01;
			}
			if (!memcmp(&cart[i], signature2, sizeof signature2))
			{
				numfound |= 0x02;
			}
		}
	}
	if (numfound == 0x03)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_32K_mode3F(UINT8 *cart, UINT32 len)
{
	int numfound = 0;
	static const unsigned char signature[4] = { 0xa9, 0x0e, 0x85, 0x3f };

	if (len >= 0x8000)
	{
		for (int i = 0; i < len - sizeof signature; i++)
		{
			if (!memcmp(&cart[i], signature, sizeof signature))
			{
				numfound++;
			}
		}
	}
	if (numfound > 1)
		return 1;
	return 0;
}

int vcs_cart_slot_device::detect_super_chip(UINT8 *cart, UINT32 len)
{
	static const unsigned char signatures[][5] = {
									{ 0xa2, 0x7f, 0x9d, 0x00, 0xf0 }, // dig dug
									{ 0xae, 0xf6, 0xff, 0x4c, 0x00 } // off the wall
	};

	if (len == 0x4000)
	{
		for (int i = 0; i < len - sizeof signatures[0]; i++)
		{
			for (auto & signature : signatures)
			{
				if (!memcmp(&cart[i], &signature, sizeof signatures[0]))
				{
					return 1;
				}
			}
		}
	}
	for (int i = 0x1000; i < len; i += 0x1000)
	{
		if (memcmp(cart, cart + i, 0x100))
		{
			return 0;
		}
	}
	/* Check the reset vector does not point into the super chip RAM area */
	if ((((cart[0x0ffd] << 8) | cart[0x0ffc]) & 0x0fff) < 0x0100)
	{
		return 0;
	}
	return 1;
}


/*-------------------------------------------------
 identify_cart_type - code to detect cart type from
 fullpath
 -------------------------------------------------*/

// 4in1 & 8in1 are not currently detected from fullpath...
int vcs_cart_slot_device::identify_cart_type(UINT8 *ROM, UINT32 len)
{
	int type = 0xff;

	// auto-detect bank mode
	if (detect_modeDC(ROM, len))
		type = A26_DC;
	else if (detect_mode3E(ROM, len))
		type = A26_3E;
	else if (detect_modeFE(ROM, len))
		type = A26_FE;
	else if (detect_modeSS(ROM, len))
		type = A26_SS;
	else if (detect_modeE0(ROM, len))
		type = A26_E0;
	else if (detect_modeCV(ROM, len))
		type = A26_CV;
	else if (detect_modeFV(ROM, len))
		type = A26_FV;
	else if (detect_modeJVP(ROM, len))
		type = A26_JVP;
	else if (detect_modeUA(ROM, len))
		type = A26_UA;
	else if (detect_8K_mode3F(ROM, len))
		type = A26_3F;
	else if (detect_32K_mode3F(ROM, len))
		type = A26_3F;
	else if (detect_modeE7(ROM, len))
		type = A26_E7;
	else if (detect_snowhite(ROM, len))
		type = A26_F8SW;

	// otherwise, choose based on size
	if (type == 0xff)
	{
		switch (len)
		{
			case 0x800:
				type = A26_2K;
				break;
			case 0x1000:
				type = A26_4K;
				break;
			case 0x2000:
				type = A26_F8;
				break;
			case 0x28ff:
			case 0x2900:
				type = A26_DPC;
				break;
			case 0x3000:
				type = A26_FA;
				break;
			case 0x4000:
				type = A26_F6;
				break;
			case 0x8000:
				type = A26_F4;
				break;
			case 0x10000:
				type = A26_32IN1;
				break;
			case 0x80000:
				type = A26_3F;
				break;
			default:
				type = A26_4K;
				printf("Unrecognized cart type!\n");
				break;
		}
	}

	return type;
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void vcs_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		int type;

		core_fread(m_file, &rom[0], len);

		type = identify_cart_type(&rom[0], len);
		slot_string = vcs_get_slot(type);

		clear();

		result.assign(slot_string);
	}
	else
		software_get_default_slot(result, "a26_4k");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(vcs_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(vcs_cart_slot_device::read_bank)
{
	if (m_cart)
		return m_cart->read_bank(space, offset, mem_mask);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(vcs_cart_slot_device::write_bank)
{
	if (m_cart)
		m_cart->write_bank(space, offset, data, mem_mask);
}

WRITE8_MEMBER(vcs_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data, mem_mask);
}


/*-------------------------------------------------
 direct update
 -------------------------------------------------*/

DIRECT_UPDATE_MEMBER(vcs_cart_slot_device::cart_opbase)
{
	if (m_cart)
		return m_cart->cart_opbase(direct, address);
	else
		return address;
}
