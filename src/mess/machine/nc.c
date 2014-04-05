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
void nc_state::nc_card_save(device_image_interface &image)
{
	/* if there is no data to write, quit */
	if (!m_card_ram || !m_card_size)
		return;

	logerror("attempting card save\n");

	/* write data */
	image.fwrite(m_card_ram, m_card_size);

	logerror("write succeeded!\r\n");
}

/* this mask will prevent overwrites from end of data */
int nc_state::nc_card_calculate_mask(int size)
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
int nc_state::nc_card_load(device_image_interface &image, unsigned char **ptr)
{
	int datasize;
	unsigned char *data;

	/* get file size */
	datasize = image.length();

	if (datasize!=0)
	{
		/* alloc memory for this data */
		data = global_alloc_array(unsigned char, datasize);

		if (data!=NULL)
		{
			m_card_size = datasize;

			/* read whole file */
			image.fread(data, datasize);

			*ptr = data;

			logerror("File loaded!\r\n");

			m_membank_card_ram_mask = nc_card_calculate_mask(datasize);

			logerror("Mask: %02x\n",m_membank_card_ram_mask);

			/* ok! */
			return 1;
		}
	}

	return 0;
}


DRIVER_INIT_MEMBER( nc_state, nc )
{
	/* card not present */
	nc_set_card_present_state(0);
	/* card ram NULL */
	m_card_ram = NULL;
	m_card_size = 0;
}


/* load pcmcia card */
DEVICE_IMAGE_LOAD_MEMBER( nc_state, nc_pcmcia_card )
{
	/* filename specified */

	/* attempt to load file */
	if (nc_card_load(image, &m_card_ram))
	{
		if (m_card_ram!=NULL)
		{
			/* card present! */
			if (m_membank_card_ram_mask!=0)
			{
				nc_set_card_present_state(1);
			}
			return IMAGE_INIT_PASS;
		}
	}

	/* nc100 can run without a image */
	return IMAGE_INIT_FAIL;
}


DEVICE_IMAGE_UNLOAD_MEMBER( nc_state, nc_pcmcia_card )
{
	/* save card data if there is any */
	nc_card_save(image);

	/* free ram allocated to card */
	if (m_card_ram!=NULL)
	{
		global_free_array(m_card_ram);
		m_card_ram = NULL;
	}
	m_card_size = 0;

	/* set card not present state */
	nc_set_card_present_state(0);
}
