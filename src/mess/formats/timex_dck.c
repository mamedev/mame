/***************************************************************************

  mess/formats/timex_dsk.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  Changes:

  KT 31/1/00 - Added support for .Z80. At the moment only 48k files are supported!
  DJR 8/2/00 - Added checks to avoid trying to load 128K .Z80 files into 48K machine!
  DJR 20/2/00 - Added support for .TAP files.
  -----------------27/02/00 10:54-------------------
  KT 27/2/00 - Added my changes for the WAV support
  --------------------------------------------------
  DJR 14/3/00 - Fixed +3 tape loading and added option to 'rewind' tapes when end reached.
  DJR 21/4/00 - Added support for 128K .SNA and .Z80 files.
  DJR 21/4/00 - Ensure 48K Basic ROM is used when running 48K snapshots on 128K machine.
  DJR 03/5/00 - Fixed bug of not decoding last byte of .Z80 blocks.
  DJR 08/5/00 - Fixed TS2068 .TAP loading.
  DJR 19/5/00 - .TAP files are now classified as cassette files.
  DJR 02/6/00 - Added support for .SCR files (screendumps).

***************************************************************************/

#include "emu.h"
#include "formats/timex_dck.h"
#include "sound/ay8910.h"


static timex_cart_t timex_cart;


DEVICE_IMAGE_LOAD( timex_cart )
{
	int file_size;
	UINT8 * file_data;

	int chunks_in_file = 0;

	int i;

	logerror ("Trying to load cart\n");

	file_size = image.length();

	if (file_size < 0x09)
	{
		logerror ("Bad file size\n");
		return IMAGE_INIT_FAIL;
	}

	file_data = (UINT8 *)malloc(file_size);
	if (file_data == NULL)
	{
		logerror ("Memory allocating error\n");
		return IMAGE_INIT_FAIL;
	}

	image.fread(file_data, file_size);

	for (i=0; i<8; i++)
		if(file_data[i+1]&0x02) chunks_in_file++;

	if (chunks_in_file*0x2000+0x09 != file_size)
	{
		free (file_data);
		logerror ("File corrupted\n");
		return IMAGE_INIT_FAIL;
	}

	switch (file_data[0x00])
	{
		case 0x00:  logerror ("DOCK cart\n");
				timex_cart.type = TIMEX_CART_DOCK;
				timex_cart.data = (UINT8*) malloc (0x10000);
				if (!timex_cart.data)
				{
					free (file_data);
					logerror ("Memory allocate error\n");
					return IMAGE_INIT_FAIL;
				}
				chunks_in_file = 0;
				for (i=0; i<8; i++)
				{
					timex_cart.chunks = timex_cart.chunks | ((file_data[i+1]&0x01)<<i);
					if (file_data[i+1]&0x02)
					{
						memcpy (timex_cart.data+i*0x2000, file_data+0x09+chunks_in_file*0x2000, 0x2000);
						chunks_in_file++;
					}
					else
					{
						if (file_data[i+1]&0x01)
							memset (timex_cart.data+i*0x2000, 0x00, 0x2000);
						else
							memset (timex_cart.data+i*0x2000, 0xff, 0x2000);
					}
				}
				free (file_data);
				break;

		default:    logerror ("Cart type not supported\n");
				free (file_data);
				timex_cart.type = TIMEX_CART_NONE;
				return IMAGE_INIT_FAIL;
	}

	logerror ("Cart loaded\n");
	logerror ("Chunks %02x\n", timex_cart.chunks);
	return IMAGE_INIT_PASS;
}

DEVICE_IMAGE_UNLOAD( timex_cart )
{
	if (timex_cart.data)
	{
		free (timex_cart.data);
		timex_cart.data = NULL;
	}
	timex_cart.type = TIMEX_CART_NONE;
	timex_cart.chunks = 0x00;
}

const timex_cart_t *timex_cart_data(void)
{
	return &timex_cart;
}
