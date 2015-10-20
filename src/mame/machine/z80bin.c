// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "machine/z80bin.h"

/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

int z80bin_load_file(device_image_interface *image, address_space &space, const char *file_type, UINT16 *exec_addr, UINT16 *start_addr, UINT16 *end_addr )
{
	int ch;
	UINT16 args[3];
	UINT16 i=0, j, size;
	UINT8 data;
	char pgmname[256];
	char message[256];

	image->fseek(7, SEEK_SET);

	while((ch = image->fgetc()) != 0x1A)
	{
		if (ch == EOF)
		{
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file name");
			image->message(" Unexpected EOF while getting file name");
			return IMAGE_INIT_FAIL;
		}

		if (ch != '\0')
		{
			if (i >= (ARRAY_LENGTH(pgmname) - 1))
			{
				image->seterror(IMAGE_ERROR_INVALIDIMAGE, "File name too long");
				image->message(" File name too long");
				return IMAGE_INIT_FAIL;
			}

			pgmname[i] = ch;    /* build program name */
			i++;
		}
	}

	pgmname[i] = '\0';  /* terminate string with a null */

	if (image->fread(args, sizeof(args)) != sizeof(args))
	{
		image->seterror(IMAGE_ERROR_INVALIDIMAGE, "Unexpected EOF while getting file size");
		image->message(" Unexpected EOF while getting file size");
		return IMAGE_INIT_FAIL;
	}

	exec_addr[0] = LITTLE_ENDIANIZE_INT16(args[0]);
	start_addr[0] = LITTLE_ENDIANIZE_INT16(args[1]);
	end_addr[0] = LITTLE_ENDIANIZE_INT16(args[2]);

	size = (end_addr[0] - start_addr[0] + 1) & 0xffff;

	/* display a message about the loaded quickload */
	image->message(" %s\nsize=%04X : start=%04X : end=%04X : exec=%04X",pgmname,size,start_addr[0],end_addr[0],exec_addr[0]);

	for (i = 0; i < size; i++)
	{
		j = (start_addr[0] + i) & 0xffff;
		if (image->fread(&data, 1) != 1)
		{
			snprintf(message, ARRAY_LENGTH(message), "%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			image->seterror(IMAGE_ERROR_INVALIDIMAGE, message);
			image->message("%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			return IMAGE_INIT_FAIL;
		}
		space.write_byte(j, data);
	}

	return IMAGE_INIT_PASS;
}
