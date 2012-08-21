/*
 * msx.c: MSX emulation
 *
 * Copyright (C) 2004 Sean Young
 *
 * Todo:
 *
 * - fix mouse support
 * - cassette support doesn't work
 * - Ensure changing cartridge after boot works
 * - wd2793, nms8255
 */

#include "includes/msx.h"

#define VERBOSE 0

static void msx_memory_init (running_machine &machine);
static void msx_memory_set_carts (running_machine &machine);


static int msx_probe_type (UINT8* pmem, int size)
{
	int kon4, kon5, asc8, asc16, i;

	if (size <= 0x10000) return 0;

	if ( (pmem[0x10] == 'Y') && (pmem[0x11] == 'Z') && (size > 0x18000) )
		return 6;

	kon4 = kon5 = asc8 = asc16 = 0;

	for (i=0;i<size-3;i++)
	{
		if (pmem[i] == 0x32 && pmem[i+1] == 0)
		{
			switch (pmem[i+2])
			{
			case 0x60:
			case 0x70:
				asc16++;
				asc8++;
				break;
			case 0x68:
			case 0x78:
				asc8++;
				asc16--;
			}

			switch (pmem[i+2])
			{
			case 0x60:
			case 0x80:
			case 0xa0:
				kon4++;
				break;
			case 0x50:
			case 0x70:
			case 0x90:
			case 0xb0:
				kon5++;
			}
		}
	}

	if (MAX (kon4, kon5) > MAX (asc8, asc16) )
		return (kon5 > kon4) ? 2 : 3;
	else
		return (asc8 > asc16) ? 4 : 5;
}

DEVICE_IMAGE_LOAD (msx_cart)
{
	msx_state *state = image.device().machine().driver_data<msx_state>();
	int size;
	int size_aligned;
	UINT8 *mem;
	int type = -1;
	const char *extra = NULL;
	char *sramfile;
	slot_state *st;
	int id = -1;

	if (strcmp(image.device().tag(),":cart1")==0)
		id = 0;

	if (strcmp(image.device().tag(),":cart2")==0)
		id = 1;

	if( id == -1 )
	{
		//logerror ("error: invalid cart tag '%s'\n", image->tag);
		return IMAGE_INIT_FAIL;
	}

	if ( image.software_entry() != NULL )
	{
		/* Load software from software list */
		/* TODO: Add proper SRAM (size) handling */

		const char *mapper = software_part_get_feature( (software_part*)image.part_entry(), "mapper" );
		const char *sram = software_part_get_feature( (software_part*)image.part_entry(), "sram" );

		if ( mapper != NULL )
		{
			/* TODO: Split out the SRAM recognition code and 8KB/16KB bank configuration */
			static const struct { const char *mapper_name; bool sram_present; int mapper_type; } mapper_types[] =
			{
				{ "M60002-0125SP",		false,		SLOT_ASCII8 },
				{ "M60002-0125SP",		true,		SLOT_ASCII8_SRAM },
				{ "LZ93A13",			false,		SLOT_ASCII8 },
				{ "LZ93A13",			true,		SLOT_ASCII8_SRAM },
				{ "LZ93A13-16",			false,		SLOT_ASCII16 },
				{ "LZ93A13-16",			true,		SLOT_ASCII16_SRAM },
				{ "M60002-0125SP-16",	false,		SLOT_ASCII16 },
				{ "M60002-0125SP-16",	true,		SLOT_ASCII16_SRAM },
				{ "IREM TAM-S1",		false,		SLOT_RTYPE },
				{ "MR6401",				false,		SLOT_ASCII16 },
				{ "NEOS MR6401",		false,		SLOT_ASCII8 },
				{ "BS6202",				false,		SLOT_ASCII8 },
				{ "BS6101",				false,		SLOT_ASCII8 },
				{ "BS6101-16",			false,		SLOT_ASCII16 },
			};

			for ( int i = 0; i < ARRAY_LENGTH(mapper_types) && type < 0; i++ )
			{
				if ( !mame_stricmp( mapper, mapper_types[i].mapper_name ) )
				{
					if ( sram == NULL && ! mapper_types[i].sram_present )
						type = mapper_types[i].mapper_type;


					if ( sram != NULL && mapper_types[i].sram_present )
						type = mapper_types[i].mapper_type;
				}
			}

			if ( -1 == type )
				logerror( "Mapper '%s' not recognized!\n", mapper );
		}

		UINT8 *rom_region = image.get_software_region( "rom" );
		size = size_aligned = image.get_software_region_length( "rom" );

		mem = auto_alloc_array( image.device().machine(), UINT8, size_aligned );
		memcpy( mem, rom_region, size_aligned );
	}
	else
	{
		/* Old style image loading */

		size = image.length ();
		if (size < 0x2000)
		{
			logerror ("cart #%d: error: file is smaller than 2kb, too small to be true!\n", id);
			return IMAGE_INIT_FAIL;
		}

		/* allocate memory and load */
		size_aligned = 0x2000;
		while (size_aligned < size)
			size_aligned *= 2;

		mem = auto_alloc_array(image.device().machine(),UINT8,size_aligned);
		if (!mem)
		{
			logerror ("cart #%d: error: failed to allocate memory for cartridge\n", id);
			return IMAGE_INIT_FAIL;
		}

		if (size < size_aligned)
			memset (mem, 0xff, size_aligned);

		if (image.fread(mem, size) != size)
		{
			logerror ("cart #%d: %s: can't read full %d bytes\n", id, image.filename (), size);
			return IMAGE_INIT_FAIL;
		}

		/* see if msx.crc will tell us more */
		extra = hashfile_extrainfo(image);

		if (!extra)
		{
			logerror("cart #%d: warning: no information in crc file\n", id);
			type = -1;
		}
		else
		if ((1 != sscanf(extra, "%d", &type) ) || type < 0 || type > SLOT_LAST_CARTRIDGE_TYPE)
		{
			logerror("cart #%d: warning: information in crc file not valid\n", id);
			type = -1;
		}
		else
			logerror ("cart #%d: info: cart extra info: '%s' = %s\n", id, extra, msx_slot_list[type].name);

		/* if not, attempt autodetection */
		if (type < 0)
		{
			type = msx_probe_type (mem, size);

			if (mem[0] != 'A' || mem[1] != 'B')
				logerror("cart #%d: %s: May not be a valid ROM file\n", id, image.filename ());

			logerror("cart #%d: Probed cartridge mapper %d/%s\n", id, type, msx_slot_list[type].name);
		}
	}

	/* mapper type 0 always needs 64kB */
	if (!type && size_aligned != 0x10000)
	{
		UINT8 *old_mem = mem;
		int old_size_aligned = size_aligned;

		size_aligned = 0x10000;
		mem = auto_alloc_array(image.device().machine(),UINT8, 0x10000);
		if (!mem)
		{
			auto_free(image.device().machine(),old_mem);
			logerror ("cart #%d: error: cannot allocate memory\n", id);
			return IMAGE_INIT_FAIL;
		}

		if (size < 0x10000)
			memset (mem + size, 0xff, 0x10000 - size);

		if (size > 0x10000)
		{
			logerror ("cart #%d: warning: rom truncated to 64kb due to mapperless type (possibly detected)\n", id);

			size = 0x10000;
		}

		/* Copy old contents to newly claimed memory */
		memcpy(mem,old_mem,old_size_aligned);
		auto_free(image.device().machine(),old_mem);
	}

	/* mapper type 0 (ROM) might need moving around a bit */
	if (!type)
	{
		int i, page = 1;

		/* find the correct page */
		if (mem[0] == 'A' && mem[1] == 'B')
		{
			for (i=2; i<=8; i += 2)
			{
				if (mem[i] || mem[i+1])
				{
					page = mem[i+1] / 0x40;
					break;
				}
			}
		}

		if (size <= 0x4000)
		{
			if (page == 1 || page == 2)
			{
				/* copy to the respective page */
				memcpy (mem + (page * 0x4000), mem, 0x4000);
				memset (mem, 0xff, 0x4000);
			}
			else
			{
				/* memory is repeated 4 times */
				page = -1;
				memcpy (mem + 0x4000, mem, 0x4000);
				memcpy (mem + 0x8000, mem, 0x4000);
				memcpy (mem + 0xc000, mem, 0x4000);
			}
		}
		else /*if (size <= 0xc000) */
		{
			if (page)
			{
				/* shift up 16kB; custom memcpy so overlapping memory isn't corrupted. ROM starts in page 1 (0x4000) */
				UINT8 *m;

				page = 1;
				i = 0xc000; m = mem + 0xffff;
				while (i--)
				{
					*m = *(m - 0x4000);
					m--;
				}
				memset (mem, 0xff, 0x4000);
			}
		}

		if (page)
			logerror ("cart #%d: info: rom in page %d\n", id, page);
		else
			logerror ("cart #%d: info: rom duplicted in all pages\n", id);
	}

	/* kludge */
	if (type == 0)
		type = SLOT_ROM;

	/* allocate and set slot_state for this cartridge */
	st = auto_alloc(image.device().machine(),slot_state);
	if (!st)
	{
		logerror ("cart #%d: error: cannot allocate memory for cartridge state\n", id);
		return IMAGE_INIT_FAIL;
	}
	memset (st, 0, sizeof (slot_state));

	st->m_type = type;
	sramfile = auto_alloc_array(image.device().machine(), char, strlen (image.filename () + 1));

	if (sramfile)
	{
		char *ext;

		strcpy (sramfile, image.basename ());
		ext = strrchr (sramfile, '.');
		if (ext)
			*ext = 0;

		st->m_sramfile = sramfile;
	}

	if (msx_slot_list[type].init (image.device().machine(), st, 0, mem, size_aligned))
		return IMAGE_INIT_FAIL;

	if (msx_slot_list[type].loadsram)
		msx_slot_list[type].loadsram (image.device().machine(), st);

	state->m_cart_state[id] = st;
	msx_memory_set_carts (image.device().machine());

	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_UNLOAD (msx_cart)
{
	msx_state *state = image.device().machine().driver_data<msx_state>();
	int id = -1;

	if (strcmp(image.device().tag(),":cart1")==0)
		id = 0;

	if (strcmp(image.device().tag(),":cart2")==0)
		id = 1;

	if( id == -1 )
	{
		//logerror ("error: invalid cart tag '%s'\n", image->tag);
		return;
	}

	if (msx_slot_list[state->m_cart_state[id]->m_type].savesram)
		msx_slot_list[state->m_cart_state[id]->m_type].savesram (image.device().machine(), state->m_cart_state[id]);
}

void msx_vdp_interrupt(device_t *, v99x8_device &device, int i)
{
	cputag_set_input_line (device.machine(), "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

void msx_state::msx_ch_reset_core ()
{
	msx_memory_reset ();
	msx_memory_map_all ();
}

MACHINE_START( msx )
{
	MACHINE_START_CALL( msx2 );
}

MACHINE_START( msx2 )
{
	msx_state *state = machine.driver_data<msx_state>();
	state->m_port_c_old = 0xff;
	state->m_dsk_stat = 0x7f;
}

MACHINE_RESET( msx )
{
	msx_state *state = machine.driver_data<msx_state>();
	state->msx_ch_reset_core ();
}

MACHINE_RESET( msx2 )
{
	msx_state *state = machine.driver_data<msx_state>();
	state->msx_ch_reset_core ();
}

I8255_INTERFACE( msx_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(msx_state, msx_ppi_port_a_w),
	DEVCB_DRIVER_MEMBER(msx_state, msx_ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(msx_state, msx_ppi_port_c_w)
};


static const UINT8 cc_op[0x100] = {
 4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
 8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
 7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
 7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
 5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 0+0,10+1,17+1, 7+1,11+1,
 5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 0+0, 7+1,11+1,
 5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 0+0, 7+1,11+1,
 5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 0+0, 7+1,11+1
};

static const UINT8 cc_cb[0x100] = {
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,12+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,15+2, 8+2
};

static const UINT8 cc_ed[0x100] = {
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 9+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2,18+2,
12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,12+2,12+2,15+2,20+2, 8+2,14+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,16+2,16+2,16+2,16+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2,
 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2, 8+2
};

static const UINT8 cc_xy[0x100] = {
 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
 4+2,14+2,20+2,10+2, 9+2, 9+2,11+2, 4+2, 4+2,15+2,20+2,10+2, 9+2, 9+2,11+2, 4+2,
 4+2, 4+2, 4+2, 4+2,23+2,23+2,19+2, 4+2, 4+2,15+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 9+2, 9+2, 9+2, 9+2, 9+2, 9+2,19+2, 9+2, 9+2, 9+2, 9+2, 9+2, 9+2, 9+2,19+2, 9+2,
19+2,19+2,19+2,19+2,19+2,19+2, 4+2,19+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2, 4+2, 4+2, 4+2, 4+2, 9+2, 9+2,19+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 0+0, 4+2, 4+2, 4+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
 4+2,14+2, 4+2,23+2, 4+2,15+2, 4+2, 4+2, 4+2, 8+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,
 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2,10+2, 4+2, 4+2, 4+2, 4+2, 4+2, 4+2
};

static const UINT8 cc_xycb[0x100] = {
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,20+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,
23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2,23+2
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};


DRIVER_INIT_MEMBER(msx_state,msx)
{
	device_set_input_line_vector (machine().device("maincpu"), 0, 0xff);

	msx_memory_init (machine());

	z80_set_cycle_tables( machine().device("maincpu"), cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex );
}

TIMER_DEVICE_CALLBACK( msx2_interrupt )
{
	msx_state *state = timer.machine().driver_data<msx_state>();
	state->m_v9938->set_sprite_limit(timer.machine().root_device().ioport("DSW")->read() & 0x20);
	state->m_v9938->set_resolution(timer.machine().root_device().ioport("DSW")->read() & 0x03);
	state->m_v9938->interrupt();
}

INTERRUPT_GEN( msx_interrupt )
{
	msx_state *state = device->machine().driver_data<msx_state>();
	int i;

	for (i=0; i<2; i++)
	{
		state->m_mouse[i] = state->ioport(i ? "MOUSE1" : "MOUSE0")->read();
		state->m_mouse_stat[i] = -1;
	}
}

/*
** The I/O functions
*/


READ8_MEMBER(msx_state::msx_psg_port_a_r)
{
	UINT8 data;

	data = (m_cass->input() > 0.0038 ? 0x80 : 0);

	if ( (m_psg_b ^ ioport("DSW")->read() ) & 0x40)
	{
		/* game port 2 */
		UINT8 inp = ioport("JOY1")->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[1] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[1] >> (4*m_mouse_stat[1]) ) & 15;
		}
	}
	else
	{
		/* game port 1 */
		UINT8 inp = ioport("JOY0")->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[0] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[0] >> (4*m_mouse_stat[0]) ) & 15;
		}
	}

	return data;
}

READ8_MEMBER(msx_state::msx_psg_port_b_r)
{
	return m_psg_b;
}

WRITE8_MEMBER(msx_state::msx_psg_port_a_w)
{
}

WRITE8_MEMBER(msx_state::msx_psg_port_b_w)
{
	/* Arabic or kana mode led */
	if ( (data ^ m_psg_b) & 0x80)
		set_led_status (machine(), 2, !(data & 0x80) );

	if ( (m_psg_b ^ data) & 0x10)
	{
		if (++m_mouse_stat[0] > 3) m_mouse_stat[0] = -1;
	}
	if ( (m_psg_b ^ data) & 0x20)
	{
		if (++m_mouse_stat[1] > 3) m_mouse_stat[1] = -1;
	}

	m_psg_b = data;
}

WRITE8_DEVICE_HANDLER( msx_printer_strobe_w )
{
	device->machine().device<centronics_device>("centronics")->strobe_w(BIT(data, 1));
}

WRITE8_DEVICE_HANDLER( msx_printer_data_w )
{
	if (device->machine().root_device().ioport("DSW")->read() & 0x80)
		/* SIMPL emulation */
		device->machine().device<dac_device>("dac")->write_signed8(data);
	else
		device->machine().device<centronics_device>("centronics")->write(*device->machine().memory().first_space(), 0, data);
}

READ8_DEVICE_HANDLER( msx_printer_status_r )
{
	UINT8 result = 0xfd;

	if (device->machine().root_device().ioport("DSW")->read() & 0x80)
		return 0xff;

	result |= device->machine().device<centronics_device>("centronics")->busy_r() << 1;

	return result;
}

WRITE8_MEMBER( msx_state::msx_fmpac_w )
{
	if (m_opll_active)
	{
		if (offset == 1)
			ym2413_w (m_ym, 1, data);
		else
			ym2413_w (m_ym, 0, data);
	}
}

/*
** RTC functions
*/

WRITE8_MEMBER( msx_state::msx_rtc_latch_w )
{
	m_rtc_latch = data & 15;
}

WRITE8_MEMBER( msx_state::msx_rtc_reg_w )
{
	m_rtc->write(space, m_rtc_latch, data);
}

READ8_MEMBER( msx_state::msx_rtc_reg_r )
{
	return m_rtc->read(space, m_rtc_latch);
}

/*
From: erbo@xs4all.nl (erik de boer)

sony and philips have used (almost) the same design
and this is the memory layout
but it is not a msx standard !

WD1793 or wd2793 registers

address

7FF8H read  status register
      write command register
7FF9H  r/w  track register (r/o on NMS 8245 and Sony)
7FFAH  r/w  sector register (r/o on NMS 8245 and Sony)
7FFBH  r/w  data register


hardware registers

address

7FFCH r/w  bit 0 side select
7FFDH r/w  b7>M-on , b6>in-use , b1>ds1 , b0>ds0  (all neg. logic)
7FFEH         not used
7FFFH read b7>drq , b6>intrq

set on 7FFDH bit 2 always to 0 (some use it as disk change reset)

*/

WRITE_LINE_MEMBER( msx_state::msx_wd179x_intrq_w )
{
	if (state)
		m_dsk_stat &= ~0x40;
	else
		m_dsk_stat |= 0x40;
}

WRITE_LINE_MEMBER( msx_state::msx_wd179x_drq_w )
{
	if (state)
		m_dsk_stat &= ~0x80;
	else
		m_dsk_stat |= 0x80;
}

const wd17xx_interface msx_wd17xx_interface =
{
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_LINE_MEMBER(msx_state, msx_wd179x_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(msx_state, msx_wd179x_drq_w),
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

LEGACY_FLOPPY_OPTIONS_START(msx)
	LEGACY_FLOPPY_OPTION(msx, "dsk", "MSX SS", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(msx, "dsk", "MSX DS", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

/*
** The PPI functions
*/

WRITE8_MEMBER( msx_state::msx_ppi_port_a_w )
{
	m_primary_slot = data;

	if (VERBOSE)
		logerror ("write to primary slot select: %02x\n", m_primary_slot);
	msx_memory_map_all ();
}

WRITE8_MEMBER( msx_state::msx_ppi_port_c_w )
{
	keylatch = data & 0x0f;

	/* caps lock */
	if ( BIT(m_port_c_old ^ data, 6) )
		set_led_status (machine(), 1, !BIT(data, 6) );

	/* key click */
	if ( BIT(m_port_c_old ^ data, 7) )
		m_dac->write_signed8(BIT(data, 7) ? 0x7f : 0);

	/* cassette motor on/off */
	if ( BIT(m_port_c_old ^ data, 4) )
		m_cass->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* cassette signal write */
	if ( BIT(m_port_c_old ^ data, 5) )
		m_cass->output(BIT(data, 5) ? -1.0 : 1.0);

	m_port_c_old = data;
}

READ8_MEMBER( msx_state::msx_ppi_port_b_r )
{
	UINT8 result = 0xff;
	int row, data;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	row = keylatch;
	if (row <= 10)
	{
		data = ioport(keynames[row / 2])->read();

		if (BIT(row, 0))
			data >>= 8;
		result = data & 0xff;
	}
	return result;
}

/************************************************************************
 *
 * New memory emulation !!
 *
 ***********************************************************************/

static void msx_memory_init (running_machine &machine)
{
	msx_state *state = machine.driver_data<msx_state>();
	int	prim, sec, page, extent, option;
	int size = 0;
	const msx_slot_layout *layout= (msx_slot_layout*)NULL;
	const msx_slot *slot;
	const msx_driver_struct *driver;
	slot_state *st;
	UINT8 *mem = NULL;

	state->m_empty = auto_alloc_array(machine, UINT8, 0x4000);
	memset (state->m_empty, 0xff, 0x4000);

	for (prim=0; prim<4; prim++) {
		for (sec=0; sec<4; sec++) {
			for (page=0; page<4; page++) {
				state->m_all_state[prim][sec][page]= (slot_state*)NULL;
			}
		}
	}

	for (driver = msx_driver_list; driver->name[0]; driver++) {
		if (!strcmp (driver->name, machine.system().name)) {
			layout = driver->layout;
		}
	}

	if (!layout) {
		logerror ("msx_memory_init: error: missing layout definition in "
				  "msx_driver_list\n");
		return;
	}

	state->layout = layout;

	for (; layout->entry != MSX_LAYOUT_LAST; layout++) {

		switch (layout->entry) {
		case MSX_LAYOUT_SLOT_ENTRY:
			prim = layout->slot_primary;
			sec = layout->slot_secondary;
			page = layout->slot_page;
			extent = layout->page_extent;

			if (layout->slot_secondary) {
				state->m_slot_expanded[layout->slot_primary]= TRUE;
			}

			slot = &msx_slot_list[layout->type];
			if (slot->slot_type != layout->type) {
				logerror ("internal error: msx_slot_list[%d].type != %d\n",
							slot->slot_type, slot->slot_type);
			}

			size = layout->size;
			option = layout->option;

			if (VERBOSE)
			{
				logerror ("slot %d/%d/%d-%d: type %s, size 0x%x\n",
					prim, sec, page, page + extent - 1, slot->name, size);
			}

			st = (slot_state*)NULL;
			if (layout->type == SLOT_CARTRIDGE1) {
				st = state->m_cart_state[0];
				if (!st) {
					slot = &msx_slot_list[SLOT_SOUNDCARTRIDGE];
					size = 0x20000;
				}
			}
			if (layout->type == SLOT_CARTRIDGE2) {
				st = state->m_cart_state[1];
				if (!st) {
					/* Check whether the optional FM-PAC rom is present */
					option = 0x10000;
					size = 0x10000;
					mem = machine.root_device().memregion("maincpu")->base() + option;
					if (machine.root_device().memregion("maincpu")->bytes() >= size + option && mem[0] == 'A' && mem[1] == 'B') {
						slot = &msx_slot_list[SLOT_FMPAC];
					}
					else {
						slot = &msx_slot_list[SLOT_EMPTY];
					}
				}
			}

			if (!st) {
				switch (slot->mem_type) {

				case MSX_MEM_HANDLER:
				case MSX_MEM_ROM:
					mem = machine.root_device().memregion("maincpu")->base() + option;
					break;
				case MSX_MEM_RAM:
					mem = NULL;
					break;
				}
				st = auto_alloc_clear (machine, slot_state);
				memset (st, 0, sizeof (slot_state));

				if (slot->init (machine, st, layout->slot_page, mem, size)) {
					continue;
				}
			}

			while (extent--) {
				if (page > 3) {
					logerror ("internal error: msx_slot_layout wrong, "
						 "page + extent > 3\n");
					break;
				}
				state->m_all_state[prim][sec][page] = st;
				page++;
			}
			break;
		case MSX_LAYOUT_KANJI_ENTRY:
			state->m_kanji_mem = machine.root_device().memregion("maincpu")->base() + layout->option;
			break;
		case MSX_LAYOUT_RAMIO_SET_BITS_ENTRY:
			state->m_ramio_set_bits = (UINT8)layout->option;
			break;
		}
	}
}

void msx_state::msx_memory_reset ()
{
	slot_state *st, *last_st = (slot_state*)NULL;
	int prim, sec, page;

	m_primary_slot = 0;

	for (prim=0; prim<4; prim++)
	{
		m_secondary_slot[prim] = 0;
		for (sec=0; sec<4; sec++)
		{
			for (page=0; page<4; page++)
			{
				st = m_all_state[prim][sec][page];
				if (st && st != last_st)
					msx_slot_list[st->m_type].reset (machine(), st);
				last_st = st;
			}
		}
	}
}

static void msx_memory_set_carts (running_machine &machine)
{
	msx_state *state = machine.driver_data<msx_state>();
	const msx_slot_layout *layout;
	int page;

	if (!state->layout)
		return;

	for (layout = state->layout; layout->entry != MSX_LAYOUT_LAST; layout++)
	{
		if (layout->entry == MSX_LAYOUT_SLOT_ENTRY)
		{
			switch (layout->type)
			{
			case SLOT_CARTRIDGE1:
				for (page=0; page<4; page++)
					state->m_all_state[layout->slot_primary][layout->slot_secondary][page]
						= state->m_cart_state[0];
				break;
			case SLOT_CARTRIDGE2:
				for (page=0; page<4; page++)
					state->m_all_state[layout->slot_primary][layout->slot_secondary][page]
						= state->m_cart_state[1];
				break;
			}
		}
	}
}

void msx_state::msx_memory_map_page (UINT8 page)
{
	int slot_primary;
	int slot_secondary;
	slot_state *st;
	const msx_slot *slot;

	slot_primary = (m_primary_slot >> (page * 2)) & 3;
	slot_secondary = (m_secondary_slot[slot_primary] >> (page * 2)) & 3;

	st = m_all_state[slot_primary][slot_secondary][page];
	slot = st ? &msx_slot_list[st->m_type] : &msx_slot_list[SLOT_EMPTY];
	m_state[page] = st;
	m_slot[page] = slot;

	if (VERBOSE)
		logerror ("mapping %s in %d/%d/%d\n", slot->name, slot_primary, slot_secondary, page);

	slot->map (machine(), st, page);
}

void msx_state::msx_memory_map_all ()
{
	for (UINT8 i=0; i<4; i++)
		msx_memory_map_page (i);
}

WRITE8_MEMBER( msx_state::msx_page0_w )
{
	if ( offset == 0 )
	{
		m_superloadrunner_bank = data;
		if (m_slot[2]->slot_type == SLOT_SUPERLOADRUNNER)
			m_slot[2]->map (machine(), m_state[2], 2);
	}

	switch (m_slot[0]->mem_type)
	{
		case MSX_MEM_RAM:
			m_ram_pages[0][offset] = data;
			break;
		case MSX_MEM_HANDLER:
			m_slot[0]->write (machine(), m_state[0], offset, data);
	}
}

WRITE8_MEMBER( msx_state::msx_page0_1_w )
{
	msx_page0_w( space, 0x2000 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page1_w )
{
	switch (m_slot[1]->mem_type)
	{
		case MSX_MEM_RAM:
			m_ram_pages[1][offset] = data;
			break;
		case MSX_MEM_HANDLER:
			m_slot[1]->write (machine(), m_state[1], 0x4000 + offset, data);
	}
}

WRITE8_MEMBER( msx_state::msx_page1_1_w )
{
	msx_page1_w( space, 0x2000 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page1_2_w )
{
	msx_page1_w( space, 0x3ff8 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page2_w )
{
	switch (m_slot[2]->mem_type)
	{
		case MSX_MEM_RAM:
			m_ram_pages[2][offset] = data;
			break;
		case MSX_MEM_HANDLER:
			m_slot[2]->write (machine(), m_state[2], 0x8000 + offset, data);
	}
}

WRITE8_MEMBER( msx_state::msx_page2_1_w )
{
	msx_page2_w( space, 0x1800 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page2_2_w )
{
	msx_page2_w( space, 0x2000 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page2_3_w )
{
	msx_page2_w( space, 0x3800 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_page3_w )
{
	switch (m_slot[3]->mem_type)
	{
		case MSX_MEM_RAM:
			m_ram_pages[3][offset] = data;
			break;
		case MSX_MEM_HANDLER:
			m_slot[3]->write (machine(), m_state[3], 0xc000 + offset, data);
	}
}

WRITE8_MEMBER( msx_state::msx_page3_1_w )
{
	msx_page3_w( space, 0x2000 + offset, data );
}

WRITE8_MEMBER( msx_state::msx_sec_slot_w )
{
	int slot = m_primary_slot >> 6;
	if (m_slot_expanded[slot])
	{
		if (VERBOSE)
			logerror ("write to secondary slot %d select: %02x\n", slot, data);

		m_secondary_slot[slot] = data;
		msx_memory_map_all ();
	}
	else
		msx_page3_w(space, 0x3fff, data);
}

READ8_MEMBER( msx_state::msx_sec_slot_r )
{
	UINT8 result;
	int slot = m_primary_slot >> 6;

	if (m_slot_expanded[slot])
		result = ~m_secondary_slot[slot];
	else
		result = m_top_page[0x1fff];

	return result;
}

WRITE8_MEMBER( msx_state::msx_ram_mapper_w )
{
	m_ram_mapper[offset] = data;
	if (m_slot[offset]->slot_type == SLOT_RAM_MM)
		m_slot[offset]->map (machine(), m_state[offset], offset);
}

READ8_MEMBER( msx_state::msx_ram_mapper_r )
{
	return m_ram_mapper[offset] | m_ramio_set_bits;
}

WRITE8_MEMBER( msx_state::msx_90in1_w )
{
	m_korean90in1_bank = data;
	if (m_slot[1]->slot_type == SLOT_KOREAN_90IN1)
		m_slot[1]->map (machine(), m_state[1], 1);

	if (m_slot[2]->slot_type == SLOT_KOREAN_90IN1)
		m_slot[2]->map (machine(), m_state[2], 2);
}

READ8_MEMBER( msx_state::msx_kanji_r )
{
	UINT8 result = 0xff;

	if (offset && m_kanji_mem)
	{
		int latch = m_kanji_latch;
		result = m_kanji_mem[latch++];

		m_kanji_latch &= ~0x1f;
		m_kanji_latch |= latch & 0x1f;
	}
	return result;
}

WRITE8_MEMBER( msx_state::msx_kanji_w )
{
	if (offset)
		m_kanji_latch = (m_kanji_latch & 0x007E0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}
