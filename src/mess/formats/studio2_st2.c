/*********************************************************************

    formats/studio2_st2.c

    Cartridge code for RCA Studio II st2 files

*********************************************************************/

#include "emu.h"
#include "includes/studio2.h"
#include "formats/studio2_st2.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

#define ST2_BLOCK_SIZE 256

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct st2_header
{
	UINT8 header[4];			/* "RCA2" in ASCII code */
	UINT8 blocks;				/* Total number of 256 byte blocks in file (including this one) */
	UINT8 format;				/* Format Code (this is format number 1) */
	UINT8 video;				/* If non-zero uses a special video driver, and programs cannot assume that it uses the standard Studio 2 one (top of screen at $0900+RB.0). A value of '1' here indicates the RAM is used normally, but scrolling is not (e.g. the top of the page is always at $900) */
	UINT8 reserved0;
	UINT8 author[2];			/* 2 byte ASCII code indicating the identity of the program coder */
	UINT8 dumper[2];			/* 2 byte ASCII code indicating the identity of the ROM Source */
	UINT8 reserved1[4];
	UINT8 catalogue[10];		/* RCA Catalogue Code as ASCIIZ string. If a homebrew ROM, may contain any identifying code you wish */
	UINT8 reserved2[6];
	UINT8 title[32];			/* Cartridge Program Title as ASCIIZ string */
	UINT8 page[64];				/* Contain the page addresses for each 256 byte block. The first byte at 64, contains the target address of the data at offset 256, the second byte contains the target address of the data at offset 512, and so on. Unused block bytes should be filled with $00 (an invalid page address). So, if byte 64 contains $1C, the ROM is paged into memory from $1C00-$1CFF */
	UINT8 reserved3[128];
};

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( st2_cartslot_load )
-------------------------------------------------*/

DEVICE_IMAGE_LOAD( st2_cartslot_load )
{
	st2_header header;

	/* check file size */
	int filesize = image.length();

	if (filesize <= ST2_BLOCK_SIZE) {
		logerror("Error loading cartridge: Invalid ROM file: %s.\n", image.filename());
		return IMAGE_INIT_FAIL;
	}

	/* read ST2 header */
	if (image.fread( &header, ST2_BLOCK_SIZE) != ST2_BLOCK_SIZE) {
		logerror("Error loading cartridge: Unable to read header from file: %s.\n", image.filename());
		return IMAGE_INIT_FAIL;
	}

	if (LOG) logerror("ST2 Catalogue: %s\n", header.catalogue);
	if (LOG) logerror("ST2 Title: %s\n", header.title);

	/* read ST2 cartridge into memory */
	for (int block = 0; block < (header.blocks - 1); block++)
	{
		UINT16 offset = header.page[block] << 8;
		UINT8 *ptr = ((UINT8 *) image.device().machine().root_device().memregion(CDP1802_TAG)->base()) + offset;

		if (LOG) logerror("ST2 Reading block %u to %04x\n", block, offset);

		if (image.fread( ptr, ST2_BLOCK_SIZE) != ST2_BLOCK_SIZE) {
			logerror("Error loading cartridge: Unable to read contents from file: %s.\n", image.filename());
			return IMAGE_INIT_FAIL;
		}
	}

	return IMAGE_INIT_PASS;
}
