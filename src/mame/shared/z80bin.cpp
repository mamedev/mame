// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "z80bin.h"

#include "imagedev/snapquik.h"

/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

std::error_condition z80bin_load_file(snapshot_image_device &image, address_space &space, uint16_t &exec_addr, uint16_t &start_addr, uint16_t &end_addr)
{
	uint16_t args[3]{};
	uint16_t i = 0U, j = 0U, size = 0U;
	uint8_t data = 0U;
	char pgmname[256]{};

	image.fseek(7, SEEK_SET);

	char ch = '\0';
	uint32_t bytes = 0;
	while ((bytes = image.fread(&ch, 1)) != 0 && ch != 0x1A)
	{
		if (ch != '\0')
		{
			if (i >= (std::size(pgmname) - 1))
			{
				osd_printf_error("File name too long\n");
				image.message(" File name too long");
				return image_error::INVALIDIMAGE;
			}

			pgmname[i] = ch;    /* build program name */
			i++;
		}
	}

	if (bytes == 0)
	{
		osd_printf_error("%s: Unexpected EOF while getting file name\n", image.basename());
		image.message(" Unexpected EOF while getting file name");
		return image_error::UNSPECIFIED;
	}

	pgmname[i] = '\0';  /* terminate string with a null */

	if (image.fread(args, sizeof(args)) != sizeof(args))
	{
		osd_printf_error("%s: Unexpected EOF while getting file size\n", image.basename());
		image.message(" Unexpected EOF while getting file size");
		return image_error::UNSPECIFIED;
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
			osd_printf_error("%s: Unexpected EOF while writing byte to %04X\n", image.basename(), j);
			image.message("%s: Unexpected EOF while writing byte to %04X", pgmname, (unsigned) j);
			return image_error::UNSPECIFIED;
		}
		space.write_byte(j, data);
	}

	return std::error_condition();
}
