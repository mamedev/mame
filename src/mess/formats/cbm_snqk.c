/***********************************************

 CBM Quickloads

 ***********************************************/

#include "emu.h"
#include "cbm_snqk.h"

/* prg file format
 * sfx file format
 * sda file format
 * 0 lsb 16bit address
 * 2 chip data */

/* p00 file format (p00 .. p63, s00 .. s63, ..)
 * 0x0000 C64File
 * 0x0007 0
 * 0x0008 Name in commodore encoding?
 * 0x0018 0 0
 * 0x001a lsb 16bit address
 * 0x001c data */


static int general_cbm_loadsnap( device_image_interface &image, const char *file_type, int snapshot_size,
								offs_t offset, void (*cbm_sethiaddress)(running_machine &machine, UINT16 hiaddress) )
{
	char buffer[7];
	UINT8 *data = NULL;
	UINT32 bytesread;
	UINT16 address = 0;
	int i;
	address_space *space = image.device().machine().firstcpu->memory().space(AS_PROGRAM);

	if (!file_type)
		goto error;

	if (!mame_stricmp(file_type, "prg"))
	{
		/* prg files */
	}
	else if (!mame_stricmp(file_type, "p00"))
	{
		/* p00 files */
		if (image.fread( buffer, sizeof(buffer)) != sizeof(buffer))
			goto error;
		if (memcmp(buffer, "C64File", sizeof(buffer)))
			goto error;
		image.fseek(26, SEEK_SET);
		snapshot_size -= 26;
	}
	else if (!mame_stricmp(file_type, "t64"))
	{
		/* t64 files - for GB64 Single T64s loading to x0801 - header is always the same size */
		if (image.fread( buffer, sizeof(buffer)) != sizeof(buffer))
			goto error;
		if (memcmp(buffer, "C64 tape image file", sizeof(buffer)))
			goto error;
		image.fseek(94, SEEK_SET);
		snapshot_size -= 94;
	}
	else
	{
		goto error;
	}

	image.fread( &address, 2);
	address = LITTLE_ENDIANIZE_INT16(address);
	if (!mame_stricmp(file_type, "t64"))
		address = 2049;
	snapshot_size -= 2;

	data = (UINT8*)malloc(snapshot_size);
	if (!data)
		goto error;

	bytesread = image.fread( data, snapshot_size);
	if (bytesread != snapshot_size)
		goto error;

	for (i = 0; i < snapshot_size; i++)
		space->write_byte(address + i + offset, data[i]);

	cbm_sethiaddress(image.device().machine(), address + snapshot_size);
	free(data);
	return IMAGE_INIT_PASS;

error:
	if (data)
		free(data);
	return IMAGE_INIT_FAIL;
}

static void cbm_quick_sethiaddress( running_machine &machine, UINT16 hiaddress )
{
	address_space *space = machine.firstcpu->memory().space(AS_PROGRAM);

	space->write_byte(0x31, hiaddress & 0xff);
	space->write_byte(0x2f, hiaddress & 0xff);
	space->write_byte(0x2d, hiaddress & 0xff);
	space->write_byte(0x32, hiaddress >> 8);
	space->write_byte(0x30, hiaddress >> 8);
	space->write_byte(0x2e, hiaddress >> 8);
}

QUICKLOAD_LOAD( cbm_c16 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_quick_sethiaddress);
}

QUICKLOAD_LOAD( cbm_c64 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_quick_sethiaddress);
}

QUICKLOAD_LOAD( cbm_vc20 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_quick_sethiaddress);
}

static void cbm_pet_quick_sethiaddress( running_machine &machine, UINT16 hiaddress )
{
	address_space *space = machine.firstcpu->memory().space(AS_PROGRAM);

	space->write_byte(0x2e, hiaddress & 0xff);
	space->write_byte(0x2c, hiaddress & 0xff);
	space->write_byte(0x2a, hiaddress & 0xff);
	space->write_byte(0x2f, hiaddress >> 8);
	space->write_byte(0x2d, hiaddress >> 8);
	space->write_byte(0x2b, hiaddress >> 8);
}

QUICKLOAD_LOAD( cbm_pet )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_pet_quick_sethiaddress);
}

static void cbm_pet1_quick_sethiaddress(running_machine &machine, UINT16 hiaddress)
{
	address_space *space = machine.firstcpu->memory().space(AS_PROGRAM);

	space->write_byte(0x80, hiaddress & 0xff);
	space->write_byte(0x7e, hiaddress & 0xff);
	space->write_byte(0x7c, hiaddress & 0xff);
	space->write_byte(0x81, hiaddress >> 8);
	space->write_byte(0x7f, hiaddress >> 8);
	space->write_byte(0x7d, hiaddress >> 8);
}

QUICKLOAD_LOAD( cbm_pet1 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_pet1_quick_sethiaddress);
}

static void cbmb_quick_sethiaddress(running_machine &machine, UINT16 hiaddress)
{
	address_space *space = machine.firstcpu->memory().space(AS_PROGRAM);

	space->write_byte(0xf0046, hiaddress & 0xff);
	space->write_byte(0xf0047, hiaddress >> 8);
}

QUICKLOAD_LOAD( cbmb )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0x10000, cbmb_quick_sethiaddress);
}

QUICKLOAD_LOAD( p500 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbmb_quick_sethiaddress);
}

static void cbm_c65_quick_sethiaddress( running_machine &machine, UINT16 hiaddress )
{
	address_space *space = machine.firstcpu->memory().space(AS_PROGRAM);

	space->write_byte(0x82, hiaddress & 0xff);
	space->write_byte(0x83, hiaddress >> 8);
}

QUICKLOAD_LOAD( cbm_c65 )
{
	return general_cbm_loadsnap(image, file_type, quickload_size, 0, cbm_c65_quick_sethiaddress);
}
