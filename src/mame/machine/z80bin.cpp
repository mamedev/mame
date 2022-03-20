// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "machine/z80bin.h"

/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

image_init_result z80bin_load_file(device_image_interface &image, address_space &space, uint16_t &exec_addr, uint16_t &start_addr, uint16_t &end_addr)
{
	int ch = 0;
	uint16_t args[3]{};
	uint16_t i = 0U, j = 0U, size = 0U;
	uint8_t data = 0U;
	char pgmname[256]{};
	char message[512]{};

	image.fseek(7, SEEK_SET);

	while((ch = image.fgetc()) != 0x1A)
	{
		if (ch == EOF)
		{
			image.seterror(image_error::INVALIDIMAGE, "Unexpected EOF while getting file name");
			image.message(" Unexpected EOF while getting file name");
			return image_init_result::FAIL;
		}

		if (ch != '\0')
		{
			if (i >= (std::size(pgmname) - 1))
			{
				image.seterror(image_error::INVALIDIMAGE, "File name too long");
				image.message(" File name too long");
				return image_init_result::FAIL;
			}

			pgmname[i] = ch;    /* build program name */
			i++;
		}
	}

	pgmname[i] = '\0';  /* terminate string with a null */

	if (image.fread(args, sizeof(args)) != sizeof(args))
	{
		image.seterror(image_error::INVALIDIMAGE, "Unexpected EOF while getting file size");
		image.message(" Unexpected EOF while getting file size");
		return image_init_result::FAIL;
	}

	exec_addr = little_endianize_int16(args[0]);
	start_addr = little_endianize_int16(args[1]);
	end_addr = little_endianize_int16(args[2]);

	size = (end_addr - start_addr + 1) & 0xffff;

	/* display a message about the loaded quickload */
	image.message(" %s\nsize=%04X : start=%04X : end=%04X : exec=%04X",pgmname,size,start_addr,end_addr,exec_addr);

	for (i = 0; i < size; i++)
	{
		j = (start_addr + i) & 0xffff;
		if (image.fread(&data, 1) != 1)
		{
			snprintf(message, std::size(message), "%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			image.seterror(image_error::INVALIDIMAGE, message);
			image.message("%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			return image_init_result::FAIL;
		}
		space.write_byte(j, data);
	}

	return image_init_result::PASS;
}
