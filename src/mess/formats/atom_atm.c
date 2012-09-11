/*********************************************************************

    formats/atom_atm.c

    Quickload code for Acorn Atom atm files

*********************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "formats/atom_atm.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    image_fread_memory - read image to memory
-------------------------------------------------*/

static void image_fread_memory(device_image_interface &image, UINT16 addr, UINT32 count)
{
	void *ptr = image.device().machine().firstcpu->memory().space(AS_PROGRAM)->get_write_ptr(addr);

	image.fread( ptr, count);
}

/*-------------------------------------------------
    QUICKLOAD_LOAD( atom_atm )
-------------------------------------------------*/

QUICKLOAD_LOAD( atom_atm )
{
	/*

        The format for the .ATM files is as follows:

        Offset Size     Description
        ------ -------- -----------------------------------------------------------
        0000h  16 BYTEs ATOM filename (if less than 16 BYTEs, rest is 00h bytes)
        0010h  WORD     Start address for load
        0012h  WORD     Execution address
        0014h  WORD     Size of data in BYTEs
        0016h  Size     Data

    */

	UINT8 header[0x16] = { 0 };

	image.fread(header, 0x16);

	UINT16 start_address = pick_integer_le(header, 0x10, 2);
	UINT16 run_address = pick_integer_le(header, 0x12, 2);
	UINT16 size = pick_integer_le(header, 0x14, 2);

	if (LOG)
	{
		header[16] = 0;
		logerror("ATM filename: %s\n", header);
		logerror("ATM start address: %04x\n", start_address);
		logerror("ATM run address: %04x\n", run_address);
		logerror("ATM size: %04x\n", size);
	}

	image_fread_memory(image, start_address, size);

	image.device().machine().firstcpu->set_pc(run_address);

	return IMAGE_INIT_PASS;
}
