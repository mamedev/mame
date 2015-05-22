// license:GPL-2.0+
// copyright-holders:Wilbert Pol, Kevin Thacker
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


/* this is not a real register, it is used to record card status */
/* ==0, card not inserted, !=0 card is inserted */

// set pcmcia card present state
void nc_state::set_card_present_state(int state)
{
	m_card_status = state;
}


// this mask will prevent overwrites from end of data
int nc_state::card_calculate_mask(int size)
{
	/* memory block is visible as 16k blocks */
	/* mask can only operate on power of two sizes */
	/* memory cards normally in power of two sizes */
	/* maximum of 64 16k blocks can be accessed using memory paging of nc computer */
	/* max card size is therefore 1mb */
	for (int i = 14; i < 20; i++)
	{
		if (size < (1 << i))
			return 0x03f >> (19 - i);
	}

	return 0x03f;
}


// load pcmcia card data
DEVICE_IMAGE_LOAD_MEMBER( nc_state, nc_pcmcia_card )
{
	UINT32 size = m_card->common_get_size("rom");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");

	set_card_present_state(1);
	m_membank_card_ram_mask = card_calculate_mask(size);

	return IMAGE_INIT_PASS;
}


// save pcmcia card data back
DEVICE_IMAGE_UNLOAD_MEMBER( nc_state, nc_pcmcia_card )
{
	// if there is no data to write, quit
	if (!m_card_size)
		return;

	logerror("attempting card save\n");

	// write data
	image.fwrite(m_card_ram, m_card_size);

	logerror("write succeeded!\r\n");

	// set card not present state
	set_card_present_state(0);
	m_card_size = 0;
}

DRIVER_INIT_MEMBER( nc_state, nc )
{
	// set card not present state
	set_card_present_state(0);
	m_card_size = 0;
}
