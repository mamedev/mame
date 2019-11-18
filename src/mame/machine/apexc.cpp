// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Robbbert
/*
    machine/apexc.c : APEXC machine

    By Raphael Nabet

    see cpu/apexc.c for background and tech info
*/

#include "emu.h"
#include "machine/apexc.h"

DEFINE_DEVICE_TYPE(APEXC_CYLINDER, apexc_cylinder_image_device, "apexc_cylinder_image", "APEXC Cylinder")
DEFINE_DEVICE_TYPE(APEXC_TAPE_PUNCHER, apexc_tape_puncher_image_device, "apexc_tape_puncher_image", "APEXC Tape Puncher")
DEFINE_DEVICE_TYPE(APEXC_TAPE_READER, apexc_tape_reader_image_device, "apexc_tape_reader_image", "APEXC Tape Reader")

apexc_cylinder_image_device::apexc_cylinder_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APEXC_CYLINDER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

/*
    Open cylinder image and read RAM
*/
image_init_result apexc_cylinder_image_device::call_load()
{
	/* load RAM contents */
	m_writable = !is_readonly();

	fread( machine().root_device().memshare("maincpu")->ptr(), 0x1000);
#ifdef LSB_FIRST
	{   /* fix endianness */
		uint32_t *RAM = (uint32_t *)(machine().root_device().memshare("maincpu")->ptr());

		for (int i=0; i < 0x0400; i++)
			RAM[i] = big_endianize_int32(RAM[i]);
	}
#endif

	return image_init_result::PASS;
}

/*
    Save RAM to cylinder image and close it
*/
void apexc_cylinder_image_device::call_unload()
{
	if (m_writable)
	{   /* save RAM contents */
		/* rewind file */
		fseek(0, SEEK_SET);
#ifdef LSB_FIRST
		{   /* fix endianness */
			uint32_t *RAM = (uint32_t *)(machine().root_device().memshare("maincpu")->ptr());

			for (int i = 0; i < /*0x2000*/0x0400; i++)
				RAM[i] = big_endianize_int32(RAM[i]);
		}
#endif
		/* write */
		fwrite(machine().root_device().memshare("maincpu")->ptr(), /*0x8000*/0x1000);
	}
}


apexc_tape_puncher_image_device::apexc_tape_puncher_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APEXC_TAPE_PUNCHER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

/*
    Punch a tape character
*/

WRITE8_MEMBER(apexc_tape_puncher_image_device::write)
{
	if (exists())
	{
		const uint8_t data5 = data & 0x1f;
		fwrite(&data5, 1);
	}
}

apexc_tape_reader_image_device::apexc_tape_reader_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, APEXC_TAPE_READER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

/*
    Read a tape image
*/

READ8_MEMBER(apexc_tape_reader_image_device::read)
{
	uint8_t reply;
	if (exists() && (fread(&reply, 1) == 1))
		return reply & 0x1f;
	else
		return 0;   /* unit not ready - I don't know what we should do */
}

