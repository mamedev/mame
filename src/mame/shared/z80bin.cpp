// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "z80bin.h"

#include "imagedev/snapquik.h"


/*-------------------------------------------------
    z80bin_load_file - load a z80bin file into
    memory
-------------------------------------------------*/

std::pair<std::error_condition, std::string> z80bin_load_file(snapshot_image_device &image, address_space &space, uint16_t &exec_addr, uint16_t &start_addr, uint16_t &end_addr)
{
	uint16_t args[3]{};
	uint16_t i, size = 0U;
	uint8_t data = 0U;
	char pgmname[256]{};

	image.fseek(7, SEEK_SET);

	char ch = '\0';
	uint32_t bytes = 0;
	i = 0;
	while ((bytes = image.fread(&ch, 1)) != 0 && ch != 0x1A)
	{
		if (ch != '\0')
		{
			if (i >= (std::size(pgmname) - 1))
				return std::make_pair(image_error::INVALIDIMAGE, "Program file name too long");

			pgmname[i] = ch;    // build program name
			i++;
		}
	}

	if (bytes == 0)
		return std::make_pair(image_error::UNSPECIFIED, "Unexpected EOF while reading program file name");

	pgmname[i] = '\0';  // terminate string with a NUL

	if (image.fread(args, sizeof(args)) != sizeof(args))
		return std::make_pair(image_error::UNSPECIFIED, "Unexpected EOF while reading program file size");

	exec_addr = little_endianize_int16(args[0]);
	start_addr = little_endianize_int16(args[1]);
	end_addr = little_endianize_int16(args[2]);

	size = (end_addr - start_addr + 1) & 0xffff;

	// display a message about the loaded quickload
	image.message(" %s\nsize=%04X : start=%04X : end=%04X : exec=%04X",pgmname,size,start_addr,end_addr,exec_addr);

	for (i = 0; i < size; i++)
	{
		uint16_t const j = (start_addr + i) & 0xffff;
		if (image.fread(&data, 1) != 1)
		{
			return std::make_pair(
					image_error::UNSPECIFIED,
					util::string_format("Unexpected EOF while reading program byte %04X", j));
		}
		space.write_byte(j, data);
	}

	return std::make_pair(std::error_condition(), std::string());
}
