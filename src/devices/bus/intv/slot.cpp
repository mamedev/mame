// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Mattel Intellivision cart emulation
    (through slot devices)


    This is a strange beast, because INTV carts had potentially access to
    a *LOT* of memory ranges!
    Quoting Joe Zbiciak's documentation for his emu (jzIntv):


      The Intellivision leaves many addresses available to cartridges.  However,
      several address ranges come with caveats, such as interactions with other
      devices in the system, or incompatibilities with various peripherals.

      Below is a summary.

      ADDRESSES      NOTES
      -------------- --------------------------------------------------------------
      $0400 - $04FF  RAM/ROM ok on all but Intellivision 2.
      $0500 - $06FF  RAM/ROM ok.
      $0700 - $0CFF  RAM/ROM ok if no Intellivoice.
      $0D00 - $0FFF  RAM/ROM ok.
      $2000 - $2FFF  RAM/ROM ok if no ECS.
      $4000 - $47FF  RAM/ROM ok if no ECS.
      $4800          ROM ok.  RAM ok only if boot ROM at $7000.
      $4801 - $4FFF  RAM/ROM ok.
      $5000 - $5014  ROM ok.  RAM ok only if boot ROM at $7000 or $4800.
      $5015 - $6FFF  RAM/ROM ok.
      $7000          ROM ok if no ECS.  RAM at $7000 confuses EXEC boot sequence.
      $7001 - $77FF  RAM/ROM ok if no ECS.
      $7800 - $7FFF  ROM ok if no ECS.  Do not map RAM here due to GRAM alias.
      $8000 - $8FFF  RAM/ROM ok.  Avoid STIC alias at $8000 - $803F.
      $9000 - $B7FF  RAM/ROM ok.
      $B800 - $BFFF  ROM ok.  Do not map RAM here due to GRAM alias.
      $C000 - $CFFF  RAM/ROM ok.  Avoid STIC alias at $C000 - $C03F.
      $D000 - $DFFF  RAM/ROM ok.
      $E000 - $EFFF  RAM/ROM ok if no ECS.
      $F000 - $F7FF  RAM/ROM ok.
      $F800 - $FFFF  ROM ok.  Do not map RAM here due to GRAM alias.


    We handle this, by always creating a 0x10000 wide ROM region to load the
    cart image and exposing the following (long list of) read handlers:
      read_rom04
      read_rom20
      read_rom40
      read_rom48
      read_rom50
      read_rom60
      read_rom70
      read_rom80
      read_rom90
      read_roma0
      read_romb0
      read_romc0
      read_romd0
      read_rome0
      read_romf0
    Each pcb types will then use the correct ones for its wiring setup.

    The BIN+CFG format introduced by INTVPC emulator includes metadata about where to
    load ROM into memory in the CFG file, but we don't support it (because we don't parse
    the CFG at all) and we rely instead on the intv.hsi metadata for fullpath loading of
    these.
    Alternatively, we support the .ROM format used by jzIntv.


 TODO:
    - Convert also the keyboard component to be a passthru slot device
    - Merge some of the ROM accessor above, once it is clear which ones can be merged

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"
#include "hashfile.h"

#define INTELLIVOICE_MASK   0x02
#define ECS_MASK            0x01

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type INTV_CART_SLOT = &device_creator<intv_cart_slot_device>;

//**************************************************************************
//    Intellivision Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_intv_cart_interface - constructor
//-------------------------------------------------

device_intv_cart_interface::device_intv_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_intv_cart_interface - destructor
//-------------------------------------------------

device_intv_cart_interface::~device_intv_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_intv_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(INTVSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		memset(m_rom, 0xff, size);
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_intv_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  intv_cart_slot_device - constructor
//-------------------------------------------------
intv_cart_slot_device::intv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, INTV_CART_SLOT, "Intellivision Cartridge Slot", tag, owner, clock, "intv_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(INTV_STD), m_cart(nullptr)
{
}


//-------------------------------------------------
//  intv_cart_slot_device - destructor
//-------------------------------------------------

intv_cart_slot_device::~intv_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intv_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_intv_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void intv_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  INTV PCB
//-------------------------------------------------

struct intv_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const intv_slot slot_list[] =
{
	{ INTV_STD,     "intv_rom" },
	{ INTV_RAM,     "intv_ram" },
	{ INTV_GFACT,   "intv_gfact" },
	{ INTV_WSMLB,   "intv_wsmlb" },
	{ INTV_VOICE,   "intv_voice" },
	{ INTV_ECS,     "intv_ecs" },
	{ INTV_KEYCOMP, "intv_keycomp" }
};

static int intv_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

#if 1
static const char *intv_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "intv_rom";
}
#endif

/*-------------------------------------------------
 call load
 -------------------------------------------------*/

int intv_cart_slot_device::load_fullpath()
{
	UINT8 temp;
	UINT8 num_segments;
	UINT8 start_seg;
	UINT8 end_seg;

	UINT32 current_address;
	UINT32 end_address;

	UINT8 high_byte;
	UINT8 low_byte;

	UINT8 *ROM;
	const char *file_type = filetype();

	/* if it is in .rom format, we enter here */
	if (!core_stricmp (file_type, "rom"))
	{
		// header
		fread(&temp, 1);
		if (temp != 0xa8)
			return IMAGE_INIT_FAIL;

		fread(&num_segments, 1);

		fread(&temp, 1);
		if (temp != (num_segments ^ 0xff))
			return IMAGE_INIT_FAIL;

		m_cart->rom_alloc(0x20000, tag());
		ROM = (UINT8 *)m_cart->get_rom_base();

		for (int i = 0; i < num_segments; i++)
		{
			fread(&start_seg, 1);
			current_address = start_seg * 0x100;

			fread(&end_seg, 1);
			end_address = end_seg * 0x100 + 0xff;

			while (current_address <= end_address)
			{
				fread(&low_byte, 1);
				ROM[(current_address << 1) + 1] = low_byte;
				fread(&high_byte, 1);
				ROM[current_address << 1] = high_byte;
				current_address++;
			}

			// Here we should calculate and compare the CRC16...
			fread(&temp, 1);
			fread(&temp, 1);
		}

		// Access tables and fine address restriction tables are not supported ATM
		for (int i = 0; i < (16 + 32 + 2); i++)
		{
			fread(&temp, 1);
		}
		return IMAGE_INIT_PASS;
	}
	/* otherwise, we load it as a .bin file, using extrainfo from intv.hsi in place of .cfg */
	else
	{
		// This code is a blatant hack, due to impossibility to load a separate .cfg file in MESS.
		// It shall be eventually replaced by the .xml loading

		// extrainfo format
		// 1. mapper number (to deal with bankswitch). no bankswitch is mapper 0 (most games).
		// 2.->5. current images have at most 4 chunks of data. we store here block size and location to load
		//  (value & 0xf0) >> 4 is the location / 0x1000
		//  (value & 0x0f) is the size / 0x800
		// 6. some images have a ram chunk. as above we store location and size in 8 bits
		// 7. extra = 1 ECS, 2 Intellivoice
		int start, size;
		int mapper, rom[5], ram, extra;
		std::string extrainfo;

		m_cart->rom_alloc(0x20000, tag());
		ROM = (UINT8 *)m_cart->get_rom_base();

		if (!hashfile_extrainfo(*this, extrainfo))
		{
			// If no extrainfo, we assume a single 0x2000 chunk at 0x5000
			for (int i = 0; i < 0x2000; i++ )
			{
				fread(&low_byte, 1);
				ROM[((0x5000 + i) << 1) + 1] = low_byte;
				fread(&high_byte, 1);
				ROM[(0x5000 + i) << 1] = high_byte;
			}
		}
		else
		{
			sscanf(extrainfo.c_str() ,"%d %d %d %d %d %d %d", &mapper, &rom[0], &rom[1], &rom[2],
					&rom[3], &ram, &extra);
			//printf("extrainfo: %d %d %d %d %d %d %d \n", mapper, rom[0], rom[1], rom[2], rom[3], ram, extra);

			if (mapper)
				logerror("Bankswitch not yet implemented!\n");

			if (ram)
			{
				start = ((ram & 0xf0) >> 4) * 0x1000;
				size = (ram & 0x0f) * 0x800;

				if (start == 0xd000 && size == 0x800)
				{
					m_type = INTV_RAM;
					m_cart->ram_alloc(0x800);
				}
				else if (start == 0x8800 && size == 0x800)
				{
					m_type = INTV_GFACT;
					m_cart->ram_alloc(0x800);
				}
				else
					printf("Unrecognized RAM setup [Start 0x%X - End 0x%X]. Please contact MESSdevs.\n", start, start + size);
			}
			if (extra & INTELLIVOICE_MASK)
			{
					printf("WARNING: This game requires emulation of the IntelliVoice module.\n");
			}

			if (extra & ECS_MASK)
			{
				printf("WARNING: This game requires emulation of the ECS module.\n");
			}

			for (int j = 0; j < 4; j++)
			{
				start = ((rom[j] & 0xf0) >> 4) * 0x1000;
				size = (rom[j] & 0x0f) * 0x800;

				// some cart has to be loaded to 0x4800, but none of the available ones goes to 0x4000.
				// Hence, we use 0x04 << 4 in extrainfo (to reduce the stored values) and fix the value here.
				if (start == 0x4000) start += 0x800;

//              logerror("step %d: %d %d \n", j, start / 0x1000, size / 0x1000);

				for (int i = 0; i < size; i++)
				{
					fread(&low_byte, 1);
					ROM[((start + i) << 1) + 1] = low_byte;
					fread(&high_byte, 1);
					ROM[(start + i) << 1] = high_byte;
				}
			}
		}

		return IMAGE_INIT_PASS;
	}
}

bool intv_cart_slot_device::call_load()
{
	if (m_cart)
	{
		if (software_entry() == NULL)
			return load_fullpath();
		else
		{
			UINT16 offset[] = { 0x400, 0x2000, 0x4000, 0x4800, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0xf000};
			const char* region_name[] = {"0400", "2000", "4000", "4800", "5000", "6000", "7000", "8000", "9000", "A000", "B000", "C000", "D000", "E000", "F000"};
			const char *pcb_name = get_feature("slot");
			bool extra_bank = false;

			if (pcb_name)
				m_type = intv_get_pcb_id(pcb_name);

			// these two carts have paged roms, which does not work well with our 0x10000 rom region
			// so if we are loading one of these, we allocate additional 0x2000 bytes for the paged bank
			if (m_type == INTV_WSMLB)
				extra_bank = true;

			UINT32 size = 0;
			UINT16 address = 0;
			UINT8 *ROM, *region;

			m_cart->rom_alloc(extra_bank ? 0x22000 : 0x20000, tag());
			ROM = m_cart->get_rom_base();

			for (int i = 0; i < 15; i++)
			{
				address = offset[i];
				size = get_software_region_length(region_name[i]);
				if (size)
				{
					region = get_software_region(region_name[i]);

					for (int j = 0; j < size / 2; j++)
					{
						ROM[((address + j) << 1) + 1] = region[2 * j];
						ROM[(address + j) << 1] = region[2 * j + 1];
					}
				}
			}

			if (m_type == INTV_RAM || m_type == INTV_GFACT || m_type == INTV_ECS)
				m_cart->ram_alloc(get_software_region_length("ram"));

			//printf("Type: %s\n", intv_get_slot(m_type));
			return IMAGE_INIT_PASS;
		}
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool intv_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void intv_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "intv_rom";
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		int type = INTV_STD;

		core_fread(m_file, &rom[0], len);

		if (rom[0] == 0xa8 && (rom[1] == (rom[2] ^ 0xff)))
		{
			// it's .ROM file, so that we don't have currently any way to distinguish RAM-equipped carts
		}
		else
		{
			// assume it's .BIN and try to use .hsi file to determine type (just RAM)
			int start;
			int mapper, rom[5], ram, extra;
			std::string extrainfo;

			if (hashfile_extrainfo(*this, extrainfo))
			{
				sscanf(extrainfo.c_str() ,"%d %d %d %d %d %d %d", &mapper, &rom[0], &rom[1], &rom[2],
						&rom[3], &ram, &extra);

				if (ram)
				{
					start = ((ram & 0xf0) >> 4) * 0x1000;
					if (start == 0xd000)
						type = INTV_RAM;
					if (start == 0x8800)
						type = INTV_GFACT;
				}
			}

		}

		slot_string = intv_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return;
	}
	software_get_default_slot(result, "intv_rom");
}

/*-------------------------------------------------
 read_ay
 -------------------------------------------------*/

READ16_MEMBER(intv_cart_slot_device::read_ay)
{
	if (m_cart)
		return m_cart->read_ay(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_ay
 -------------------------------------------------*/

WRITE16_MEMBER(intv_cart_slot_device::write_ay)
{
	if (m_cart)
		m_cart->write_ay(space, offset, data, mem_mask);
}

/*-------------------------------------------------
 read_speech
 -------------------------------------------------*/

READ16_MEMBER(intv_cart_slot_device::read_speech)
{
	if (m_cart)
		return m_cart->read_speech(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_speech
 -------------------------------------------------*/

WRITE16_MEMBER(intv_cart_slot_device::write_speech)
{
	if (m_cart)
		m_cart->write_speech(space, offset, data, mem_mask);
}



#include "bus/intv/rom.h"
#include "bus/intv/ecs.h"
//#include "bus/intv/keycomp.h"
#include "bus/intv/voice.h"

SLOT_INTERFACE_START(intv_cart)
	SLOT_INTERFACE_INTERNAL("intv_rom",     INTV_ROM_STD)
	SLOT_INTERFACE_INTERNAL("intv_ram",     INTV_ROM_RAM)
	SLOT_INTERFACE_INTERNAL("intv_gfact",   INTV_ROM_GFACT)
	SLOT_INTERFACE_INTERNAL("intv_wsmlb",   INTV_ROM_WSMLB)
	SLOT_INTERFACE_INTERNAL("intv_voice",   INTV_ROM_VOICE)
	SLOT_INTERFACE_INTERNAL("intv_ecs",     INTV_ROM_ECS)
//  SLOT_INTERFACE_INTERNAL("intv_keycomp", INTV_ROM_KEYCOMP)
SLOT_INTERFACE_END
