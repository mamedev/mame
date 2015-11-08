// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


    Atari 7800 cart emulation
    (through slot devices)

    Emulation of the cartslot for Atari 7800

    Quoting "ATARI 7800 BANKSWITCHING GUIDE" (by Eckhard Stolberg):
    7800 games can use the memory from $0400 to $047f, from $0500
    to $17ff and from $2800 to $ffff, but only the High-Score cart
    uses anything below $4000. It has 4KB of ROM at $3000-$3fff
    and 2KB of battery-backed RAM at $1000-$17ff.

    Accordingly, we use the following handlers:
    - read_04xx/write_04xx for accesses in the $0400 to $047f range
    - read_10xx/write_10xx for accesses in the $1000 to $17ff range
    - read_30xx/write_30xx for accesses in the $3000 to $3fff range
    - read_40xx/write_40xx for accesses in the $4000 to $ffff range
    even if not all carts use all of them (in particular no cart type
    seems to use access to the ranges $0500 to $0fff and $2800 to $2fff)


 ***********************************************************************************************************/


#include "emu.h"
#include "a78_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A78_CART_SLOT = &device_creator<a78_cart_slot_device>;


//-------------------------------------------------
//  device_vcs_cart_interface - constructor
//-------------------------------------------------

device_a78_cart_interface::device_a78_cart_interface (const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_rom_size(0),
		m_base_rom(0x8000),
		m_bank_mask(0)
{
}


//-------------------------------------------------
//  ~device_a78_cart_interface  - destructor
//-------------------------------------------------

device_a78_cart_interface::~device_a78_cart_interface ()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_a78_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(A78SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;

		// setup other helpers
		if ((size / 0x4000) & 1) // compensate for SuperGame carts with 9 x 16K banks (to my knowledge no other cart has m_bank_mask != power of 2)
			m_bank_mask = (size / 0x4000) - 2;
		else
			m_bank_mask = (size / 0x4000) - 1;

		// the rom is mapped to the top of the memory area
		// so we store the starting point of data to simplify
		// the access handling
		m_base_rom = 0x10000 - size;
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a78_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
	device().save_item(NAME(m_ram));
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_a78_cart_interface::nvram_alloc(UINT32 size)
{
	m_nvram.resize(size);
	device().save_item(NAME(m_nvram));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a78_cart_slot_device - constructor
//-------------------------------------------------
a78_cart_slot_device::a78_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, A78_CART_SLOT, "Atari 7800 Cartridge Slot", tag, owner, clock, "a78_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  a78_cart_slot_device - destructor
//-------------------------------------------------

a78_cart_slot_device::~a78_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a78_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_a78_cart_interface  *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void a78_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

int a78_cart_slot_device::validate_header(int head, bool log)
{
	switch (head & 0x3d)
	{
		case 0x05:
			if (log)
			{
				osd_printf_info("POKEY + RAM at $4000 (Header 0x05)\n");
				osd_printf_info("Disabling POKEY\n");
			}
			head &= ~0x01;
			break;
		case 0x09:
			if (log)
			{
				osd_printf_info("POKEY + Bank 0 of 144K ROM  at $4000 (Header 0x09)\n");
				osd_printf_info("Disabling POKEY\n");
			}
			head &= ~0x01;
			break;
		case 0x11:
			if (log)
			{
				osd_printf_info("POKEY + Bank 6 ROM at $4000 (Header 0x11)\n");
				osd_printf_info("Disabling POKEY\n");
			}
			head &= ~0x01;
			break;
		case 0x21:
			if (log)
			{
				osd_printf_info("POKEY + banked RAM at $4000 (Header 0x21)\n");
				osd_printf_info("Disabling POKEY\n");
			}
			head &= ~0x01;
			break;
		case 0x0c:
			if (log)
			{
				osd_printf_info("RAM + Bank 0 of 144K ROM at $4000 (Header 0x0c)\n");
				osd_printf_info("Disabling RAM\n");
			}
			head &= ~0x04;
			break;
		case 0x14:
			if (log)
			{
				osd_printf_info("RAM + Bank 6 ROM at $4000 (Header 0x14)\n");
				osd_printf_info("Disabling RAM\n");
			}
			head &= ~0x04;
			break;
		case 0x24:
			if (log)
			{
				osd_printf_info("RAM + Banked RAM at $4000 (Header 0x24)\n");
				osd_printf_info("Disabling RAM\n");
			}
			head &= ~0x04;
			break;
		case 0x18:
			if (log)
			{
				osd_printf_info("Bank 0 of 144K ROM + Bank 6 ROM at $4000 (Header 0x18)\n");
				osd_printf_info("Disabling Bank 0 ROM\n");
			}
			head &= ~0x08;
			break;
		case 0x28:
			if (log)
			{
				osd_printf_info("Bank 0 of 144K ROM + Banked RAM at $4000 (Header 0x28)\n");
				osd_printf_info("Disabling Bank 0 ROM\n");
			}
			head &= ~0x08;
			break;
		case 0x30:
			if (log)
			{
				osd_printf_info("Bank 6 ROM + banked RAM at $4000 (Header 0x30)\n");
				osd_printf_info("Disabling Bank 6 ROM\n");
			}
			head &= ~0x10;
			break;
	}

	if ((head & 0x3c) && !(head & 0x02))
	{
		if (log)
		{
			osd_printf_info("SuperCart bankswitch detected at $4000, with no SuperCart bit (Header 0x%X)\n", head);
			osd_printf_info("Enablig SuperCart bankswitch\n");
		}
		head |= 0x02;
	}

	if ((head & 0xff00) == 0x100 && (head & 0xff))
	{
		if (log)
		{
			osd_printf_info("Bankswitch detected for an Activision cart (Header 0x%X)\n", head);
			osd_printf_info("Disabling bankswitch\n");
		}
		head &= 0xff00;
	}

	if ((head & 0xff00) == 0x200 && (head & 0xff))
	{
		if (log)
		{
			osd_printf_info("Bankswitch detected for an Absolute cart (Header 0x%X)\n", head);
			osd_printf_info("Disabling bankswitch\n");
		}
		head &= 0xff00;
	}

	if ((head & 0xff00) > 0x300)
	{
		if (log)
		{
			osd_printf_info("Unsupported mapper, please contact MESSdevs (Header 0x%X)\n", head);
			osd_printf_info("Disabling special bits\n");
		}
		head &= 0x00ff;
	}

	return head;
}


//-------------------------------------------------
//  A78 PCBs
//-------------------------------------------------

struct a78_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

#define A78_POKEY0450 0x20

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const a78_slot slot_list[] =
{
	{ A78_TYPE0,      "a78_rom" },
	{ A78_TYPE1,      "a78_pokey" },
	{ A78_TYPE2,      "a78_sg" },
	{ A78_TYPE3,      "a78_sg_pokey" },
	{ A78_TYPE6,      "a78_sg_ram" },
	{ A78_TYPEA,      "a78_sg9" },
	{ A78_ABSOLUTE,   "a78_abs" },
	{ A78_ACTIVISION, "a78_act" },
	{ A78_HSC,        "a78_hsc" },
	{ A78_XB_BOARD,   "a78_xboard" },
	{ A78_XM_BOARD,   "a78_xm" },
	{ A78_MEGACART,   "a78_megacart" },
	{ A78_VERSABOARD, "a78_versa" },
	{ A78_TYPE0_POK450, "a78_p450_t0" },
	{ A78_TYPE1_POK450, "a78_p450_t1" },
	{ A78_TYPE6_POK450, "a78_p450_t6" },
	{ A78_TYPEA_POK450, "a78_p450_ta" },
	{ A78_VERSA_POK450, "a78_p450_vb" }
};

static int a78_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *a78_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "a78_rom";
}

bool a78_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len;

		if (software_entry() != NULL)
		{
			const char *pcb_name;
			bool has_ram = get_software_region("ram") ? TRUE : FALSE;
			bool has_nvram = get_software_region("nvram") ? TRUE : FALSE;
			len = get_software_region_length("rom");

			m_cart->rom_alloc(len, tag());
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), len);

			if ((pcb_name = get_feature("slot")) != NULL)
				m_type = a78_get_pcb_id(pcb_name);
			else
				m_type = A78_TYPE0;

			if (has_ram)
				m_cart->ram_alloc(get_software_region_length("ram"));
			if (has_nvram)
			{
				m_cart->nvram_alloc(get_software_region_length("nvram"));
				battery_load(m_cart->get_nvram_base(), get_software_region_length("nvram"), 0xff);
			}
		}
		else
		{
			// Load and check the header
			int mapper;
			char head[128];
			fread(head, 128);

			if (verify_header((char *)head) == IMAGE_VERIFY_FAIL)
				return IMAGE_INIT_FAIL;

			len = (head[49] << 24) | (head[50] << 16) | (head[51] << 8) | head[52];
			if (len + 128 > length())
			{
				logerror("Invalid length in the header. The game might be corrupted.\n");
				len = length() - 128;
			}

			// let's try to auto-fix some common errors in the header
			mapper = validate_header((head[53] << 8) | head[54], TRUE);

			switch (mapper & 0x2e)
			{
				case 0x0000:
					m_type = BIT(mapper, 0) ? A78_TYPE1 : A78_TYPE0;
					break;
				case 0x0002:
					m_type = BIT(mapper, 0) ? A78_TYPE3 : A78_TYPE2;
					break;
				case 0x0006:
					m_type = A78_TYPE6;
					break;
				case 0x000a:
					m_type = A78_TYPEA;
					break;
				case 0x0022:
				case 0x0026:
					if (len > 0x40000)
						m_type = A78_MEGACART;
					else
						m_type = A78_VERSABOARD;
					break;
			}

			// check if cart has a POKEY at $0450 (typically a VersaBoard variant)!
			if (mapper & 0x40)
			{
				if (m_type != A78_TYPE2)
				{
					m_type &= ~0x02;
					m_type += A78_POKEY0450;
				}
			}

			// check special bits, which override the previous
			if ((mapper & 0xff00) == 0x0100)
				m_type = A78_ACTIVISION;
			else if ((mapper & 0xff00) == 0x0200)
				m_type = A78_ABSOLUTE;

			logerror("Cart type: 0x%x\n", m_type);

			if (head[58] == 1)
			{
				osd_printf_info("This cart support external NVRAM savings, using HSC.\n");
				osd_printf_info("Run it with the High Score Cart mounted to exploit this feature.\n");
			}
			else if (head[58] == 2)
			{
				osd_printf_info("This cart support external NVRAM savings, using SaveKey.\n");
				osd_printf_info("This is not supported in MESS currently.\n");
			}

			if (head[63])
			{
				osd_printf_info("This cart requires XBoarD / XM expansion\n");
				osd_printf_info("Run it through the expansion to exploit this feature.\n");
			}

			internal_header_logging((UINT8 *)head, length());

			m_cart->rom_alloc(len, tag());
			fread(m_cart->get_rom_base(), len);

			if (m_type == A78_TYPE6)
				m_cart->ram_alloc(0x4000);
			if (m_type == A78_MEGACART || (m_type >= A78_VERSABOARD && m_type <= A78_VERSA_POK450))
				m_cart->ram_alloc(0x8000);
			if (m_type == A78_XB_BOARD || m_type == A78_XM_BOARD)
				m_cart->ram_alloc(0x20000);
			if (m_type == A78_HSC || m_type == A78_XM_BOARD)
			{
				m_cart->nvram_alloc(0x800);
				battery_load(m_cart->get_nvram_base(), 0x800, 0xff);
			}
		}

		//printf("Type: %s\n", a78_get_slot(m_type));
	}
	return IMAGE_INIT_PASS;
}


void a78_partialhash(hash_collection &dest, const unsigned char *data,
						unsigned long length, const char *functions)
{
	if (length <= 128)
		return;
	dest.compute(&data[128], length - 128, functions);
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void a78_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), 0x800);
}



/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool a78_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}

/*-------------------------------------------------
 verify_header - check the image (from fullpath)
 has an admissible header
 -------------------------------------------------*/

int a78_cart_slot_device::verify_header(char *header)
{
	const char *magic = "ATARI7800";

	if (strncmp(magic, header + 1, 9))
	{
		logerror("Not a valid A7800 image\n");
		return IMAGE_VERIFY_FAIL;
	}

	logerror("returning ID_OK\n");
	return IMAGE_VERIFY_PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void a78_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "a78_rom";
		dynamic_buffer head(128);
		int type = A78_TYPE0, mapper;

		// Load and check the header
		core_fread(m_file, &head[0], 128);

		// let's try to auto-fix some common errors in the header
		mapper = validate_header((head[53] << 8) | head[54], FALSE);

		switch (mapper & 0x2e)
		{
			case 0x0000:
				type = BIT(mapper, 0) ? A78_TYPE1 : A78_TYPE0;
				break;
			case 0x0002:
				type = BIT(mapper, 0) ? A78_TYPE3 : A78_TYPE2;
				break;
			case 0x0006:
				type = A78_TYPE6;
				break;
			case 0x000a:
				type = A78_TYPEA;
				break;
			case 0x0022:
			case 0x0026:
				if (core_fsize(m_file) > 0x40000)
					type = A78_MEGACART;
				else
					type = A78_VERSABOARD;
				break;
		}

		// check if cart has a POKEY at $0450 (typically a VersaBoard variant)!
		if (mapper & 0x40)
		{
			if (type != A78_TYPE2)
			{
				type &= ~0x02;
				type += A78_POKEY0450;
			}
		}

		// check special bits, which override the previous
		if ((mapper & 0xff00) == 0x0100)
			type = A78_ACTIVISION;
		else if ((mapper & 0xff00) == 0x0200)
			type = A78_ABSOLUTE;

		logerror("Cart type: %x\n", type);
		slot_string = a78_get_slot(type);

		clear();

		result.assign(slot_string);
	}
	else
		software_get_default_slot(result, "a78_rom");
}


/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(a78_cart_slot_device::read_04xx)
{
	if (m_cart)
		return m_cart->read_04xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_10xx)
{
	if (m_cart)
		return m_cart->read_10xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_30xx)
{
	if (m_cart)
		return m_cart->read_30xx(space, offset, mem_mask);
	else
		return 0xff;
}

READ8_MEMBER(a78_cart_slot_device::read_40xx)
{
	if (m_cart)
		return m_cart->read_40xx(space, offset, mem_mask);
	else
		return 0xff;
}


/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(a78_cart_slot_device::write_04xx)
{
	if (m_cart)
		m_cart->write_04xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_10xx)
{
	if (m_cart)
		m_cart->write_10xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_30xx)
{
	if (m_cart)
		m_cart->write_30xx(space, offset, data, mem_mask);
}

WRITE8_MEMBER(a78_cart_slot_device::write_40xx)
{
	if (m_cart)
		m_cart->write_40xx(space, offset, data, mem_mask);
}


/*-------------------------------------------------
 A78 header logging

 A78 HEADER FORMAT

 Bytes  | Content           | Length
 ========================================
 0      | Header version    |  1 byte
 -------|-------------------|------------
 1..16  | "ATARI7800     "  | 16 bytes
 -------|-------------------|------------
 17..48 | Cart title        | 32 bytes
 -------|-------------------|------------
 49..52 | Data length       |  4 bytes
 -------|-------------------|------------
 53..54 | Cart type [*]     |  2 bytes
 -------|-------------------|------------
 55     | Controller 1 type |  1 byte
        |                   |
        | 0 = None          |
        | 1 = Joystick      |
        | 2 = Light Gun     |
 -------|-------------------|------------
 56     | Controller 2 type |  1 byte
        |                   |
        | As above          |
 -------|-------------------|------------
 57     | TV System         |  1 byte
        |                   |
        | 0 = NTSC/1 = PAL  |
 -------|-------------------|------------
 58     | Save data         |  1 byte
        |                   |  (only v2)
        | 0 = None / Unk    |
        | 1 = High Score    |
        | 2 = Savekey       |
 -------|-------------------|-----------
 63     | Expansion module  |  1 byte
        |                   |
        | 0 = No expansion  |
        |     module        |
        | 1 = Expansion     |
        |     required      |
 -------|-------------------|-----------


 [*] Cart type:

 bit 0-7 - Hardware "flags"
 bit 0 [0x01] - POKEY at $4000
 bit 1 [0x02] - SuperCart bank switched
 bit 2 [0x04] - SuperCart RAM at $4000
 bit 3 [0x08] - bank 0 of 144K ROM at $4000
 bit 4 [0x10] - bank 6 at $4000
 bit 5 [0x20] - banked RAM at $4000

 bit 8-15 - Special values
 0 = Normal cart
 1 = Absolute (F18 Hornet)
 2 = Activision (Double Dragon & Rampage)
 3 = POKEY at $0450

 -------------------------------------------------*/

void a78_cart_slot_device::internal_header_logging(UINT8 *header, UINT32 len)
{
	char head_title[35];
	UINT32 head_length = (header[49] << 24) | (header[50] << 16) | (header[51] << 8) | header[52];
	UINT16 head_mapper = (header[53] << 8) | header[54];
	UINT8 head_ctrl1 = header[55];
	UINT8 head_ctrl2 = header[56];
	UINT8 head_ispal = header[57];
	std::string cart_mapper, ctrl1, ctrl2;
	memcpy(head_title, header + 0x11, 0x20);

	switch (head_mapper)
	{
		case 0x0000:
			cart_mapper.assign("No Bankswitch");
			break;
		case 0x0001:
			cart_mapper.assign("No Bankswitch + POKEY");
			break;
		case 0x0002:
			cart_mapper.assign("SuperCart Bankswitch");
			break;
		case 0x0003:
			cart_mapper.assign("SuperCart Bankswitch + POKEY");
			break;
		case 0x0006:
			cart_mapper.assign("SuperCart Bankswitch + RAM");
			break;
		case 0x000a:
			cart_mapper.assign("SuperCart 9Banks");
			break;
		case 0x000b:
			cart_mapper.assign("SuperCart XM Compatible");
			break;
		case 0x0020:
			cart_mapper.assign("SuperCart Bankswitch + 32K RAM");
			break;
		case 0x0100:
			cart_mapper.assign("Activision Bankswitch");
			break;
		case 0x0200:
			cart_mapper.assign("Absolute Bankswitch");
			break;
		default:
			cart_mapper.assign("Unknown mapper");
			break;
	}

	switch (head_ctrl1)
	{
		case 0x00:
			ctrl1.assign("None");
			break;
		case 0x01:
			ctrl1.assign("Joystick");
			break;
		case 0x02:
			ctrl1.assign("Light Gun");
			break;
		default:
			ctrl1.assign("Unknown controller");
			break;
	}

	switch (head_ctrl2)
	{
		case 0x00:
			ctrl2.assign("None");
			break;
		case 0x01:
			ctrl2.assign("Joystick");
			break;
		case 0x02:
			ctrl2.assign("Light Gun");
			break;
		default:
			ctrl2.assign("Unknown controller");
			break;
	}

	logerror( "ROM DETAILS\n" );
	logerror( "===========\n\n" );
	logerror( "\tTotal length (with header):  0x%x (%dK + 128b header)\n\n", len, len/0x400);
	logerror( "HEADER DETAILS\n" );
	logerror( "==============\n\n" );
	logerror( "\tTitle:           %.32s\n", head_title);
	logerror( "\tLength:          0x%X [real 0x%X]\n", head_length, len);
	logerror( "\tMapper:          %s [0x%X]\n", cart_mapper.c_str(), head_mapper);
	logerror( "\t\tPOKEY:           %s\n", BIT(head_mapper, 0) ? "Yes" : "No");
	logerror( "\t\tSC Bankswitch:   %s\n", BIT(head_mapper, 1) ? "Yes" : "No");
	logerror( "\t\tRAM at $4000:    %s\n", BIT(head_mapper, 2) ? "Yes" : "No");
	logerror( "\t\tbank0 at $4000:  %s\n", BIT(head_mapper, 3) ? "Yes" : "No");
	logerror( "\t\tbank6 at $4000:  %s\n", BIT(head_mapper, 4) ? "Yes" : "No");
	logerror( "\t\tbanked RAM:      %s\n", BIT(head_mapper, 5) ? "Yes" : "No");
	logerror( "\t\tSpecial:         %s ", (head_mapper & 0xff00) ? "Yes" : "No");
	if (head_mapper & 0xff00)
	{
		logerror( "[%s]\n", (head_mapper & 0xff00) == 0x100 ? "Absolute" :
								(head_mapper & 0xff00) == 0x200 ? "Activision" : "Unknown" );
	}
	else
		logerror( "\n");
	logerror( "\tController 1:    0x%.2X [%s]\n", head_ctrl1, ctrl1.c_str());
	logerror( "\tController 2:    0x%.2X [%s]\n", head_ctrl2, ctrl2.c_str());
	logerror( "\tVideo:           %s\n", (head_ispal) ? "PAL" : "NTSC");
}
