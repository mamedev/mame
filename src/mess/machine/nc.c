/***************************************************************************

  nc.c

***************************************************************************/

#include "emu.h"
#include "includes/nc.h"
#include "machine/i8251.h"

/*************************************************************************************************/
/* PCMCIA Ram Card management */

/* the data is stored as a simple memory dump, there is no header or other information */

/* stores size of actual file on filesystem */

/* save card data back */
static void nc_card_save(device_image_interface &image)
{
	nc_state *state = image.device().machine().driver_data<nc_state>();
	/* if there is no data to write, quit */
	if (!state->m_card_ram || !state->m_card_size)
		return;

	logerror("attempting card save\n");

	/* write data */
	image.fwrite(state->m_card_ram, state->m_card_size);

	logerror("write succeeded!\r\n");
}

/* this mask will prevent overwrites from end of data */
static int nc_card_calculate_mask(int size)
{
	int i;

	/* memory block is visible as 16k blocks */
	/* mask can only operate on power of two sizes */
	/* memory cards normally in power of two sizes */
	/* maximum of 64 16k blocks can be accessed using memory paging of nc computer */
	/* max card size is therefore 1mb */
	for (i=14; i<20; i++)
	{
		if (size<(1<<i))
			return 0x03f>>(19-i);
	}

	return 0x03f;
}


/* load card image */
static int nc_card_load(device_image_interface &image, unsigned char **ptr)
{
	nc_state *state = image.device().machine().driver_data<nc_state>();
	int datasize;
	unsigned char *data;

	/* get file size */
	datasize = image.length();

	if (datasize!=0)
	{
		/* malloc memory for this data */
		data = (unsigned char *)malloc(datasize);

		if (data!=NULL)
		{
			state->m_card_size = datasize;

			/* read whole file */
			image.fread(data, datasize);

			*ptr = data;

			logerror("File loaded!\r\n");

			state->m_membank_card_ram_mask = nc_card_calculate_mask(datasize);

			logerror("Mask: %02x\n",state->m_membank_card_ram_mask);

			/* ok! */
			return 1;
		}
	}

	return 0;
}

DEVICE_START( nc_pcmcia_card )
{
	nc_state *state = device->machine().driver_data<nc_state>();
	/* card not present */
	nc_set_card_present_state(device->machine(), 0);
	/* card ram NULL */
	state->m_card_ram = NULL;
	state->m_card_size = 0;
}

/* load pcmcia card */
DEVICE_IMAGE_LOAD( nc_pcmcia_card )
{
	nc_state *state = image.device().machine().driver_data<nc_state>();
	/* filename specified */

	/* attempt to load file */
	if (nc_card_load(image, &state->m_card_ram))
	{
		if (state->m_card_ram!=NULL)
		{
			/* card present! */
			if (state->m_membank_card_ram_mask!=0)
			{
				nc_set_card_present_state(image.device().machine(), 1);
			}
			return IMAGE_INIT_PASS;
		}
	}

	/* nc100 can run without a image */
	return IMAGE_INIT_FAIL;
}

DEVICE_IMAGE_UNLOAD( nc_pcmcia_card )
{
	nc_state *state = image.device().machine().driver_data<nc_state>();
	/* save card data if there is any */
	nc_card_save(image);

	/* free ram allocated to card */
	if (state->m_card_ram!=NULL)
	{
		free(state->m_card_ram);
		state->m_card_ram = NULL;
	}
	state->m_card_size = 0;

	/* set card not present state */
	nc_set_card_present_state(image.device().machine(), 0);
}


