/*********************************************************************

    formats/trs_cmd.c

    Quickload code for TRS-80 /CMD files

*********************************************************************/

#include "emu.h"
#include "formats/trs_cmd.h"
#include "cpu/z80/z80.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

#define CMD_TYPE_OBJECT_CODE							0x01
#define CMD_TYPE_TRANSFER_ADDRESS						0x02
#define CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER		0x04
#define CMD_TYPE_LOAD_MODULE_HEADER						0x05
#define CMD_TYPE_PARTITIONED_DATA_SET_HEADER			0x06
#define CMD_TYPE_PATCH_NAME_HEADER						0x07
#define CMD_TYPE_ISAM_DIRECTORY_ENTRY					0x08
#define CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY			0x0a
#define CMD_TYPE_PDS_DIRECTORY_ENTRY					0x0c
#define CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY				0x0e
#define CMD_TYPE_YANKED_LOAD_BLOCK						0x10
#define CMD_TYPE_COPYRIGHT_BLOCK						0x1f

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

QUICKLOAD_LOAD( trs80_cmd )
{
    address_space *program = image.device().machine().firstcpu->memory().space(AS_PROGRAM);

	UINT8 type, length;
	UINT8 data[0x100];
	UINT8 addr[2];
	void *ptr;

	while (!image.image_feof())
	{
		image.fread( &type, 1);
		image.fread( &length, 1);

		length -= 2;
		int block_length = length ? length : 256;

		switch (type)
		{
		case CMD_TYPE_OBJECT_CODE:
			{
			image.fread( &addr, 2);
			UINT16 address = (addr[1] << 8) | addr[0];
			if (LOG) logerror("/CMD object code block: address %04x length %u\n", address, block_length);
			ptr = program->get_write_ptr(address);
			image.fread( ptr, block_length);
			}
			break;

		case CMD_TYPE_TRANSFER_ADDRESS:
			{
			image.fread( &addr, 2);
			UINT16 address = (addr[1] << 8) | addr[0];
			if (LOG) logerror("/CMD transfer address %04x\n", address);
			cpu_set_reg(image.device().machine().firstcpu, Z80_PC, address);
			}
			break;

		case CMD_TYPE_LOAD_MODULE_HEADER:
			image.fread( &data, block_length);
			if (LOG) logerror("/CMD load module header '%s'\n", data);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK:
			image.fread( &data, block_length);
			if (LOG) logerror("/CMD copyright block '%s'\n", data);
			break;

		default:
			image.fread( &data, block_length);
			logerror("/CMD unsupported block type %u!\n", type);
		}
	}

	return IMAGE_INIT_PASS;
}
