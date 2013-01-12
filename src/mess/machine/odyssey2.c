/***************************************************************************

  odyssey2.c

  Machine file to handle emulation of the Odyssey 2.

***************************************************************************/

#include "emu.h"
#include "includes/odyssey2.h"
#include "imagedev/cartslot.h"
#include "sound/sp0256.h"



void odyssey2_state::switch_banks()
{
	switch ( m_cart_size )
	{
		case 12288:
			/* 12KB cart support (for instance, KTAA as released) */
			membank( "bank1" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0xC00 );
			membank( "bank2" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0xC00 + 0x800 );
			break;

		case 16384:
			/* 16KB cart support (for instance, full sized version KTAA) */
			membank( "bank1" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0x1000 + 0x400 );
			membank( "bank2" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0x1000 + 0xC00 );
			break;

		default:
			membank("bank1")->set_base(memregion("user1")->base() + (m_p1 & 0x03) * 0x800);
			membank("bank2")->set_base(memregion("user1")->base() + (m_p1 & 0x03) * 0x800 );
			break;
	}
}


WRITE_LINE_MEMBER(odyssey2_state::the_voice_lrq_callback)
{
	m_the_voice_lrq_state = state;
}


READ8_MEMBER(odyssey2_state::t0_read)
{
	return ( m_the_voice_lrq_state == ASSERT_LINE ) ? 0 : 1;
}


DRIVER_INIT_MEMBER(odyssey2_state,odyssey2)
{
	int i;
	int size = 0;
	UINT8 *gfx = memregion("gfx1")->base();
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine().device("cart"));

	m_ram        = auto_alloc_array(machine(), UINT8, 256);

	for (i = 0; i < 256; i++)
	{
		gfx[i] = i;     /* TODO: Why i and not 0? */
		m_ram[i] = 0;
	}

	if (image->exists())
	{
		if (image->software_entry() == NULL)
		{
			size = image->length();
		}
		else
		{
			size = image->get_software_region_length("rom");
		}
	}
	m_cart_size = size;
}


void odyssey2_state::machine_start()
{
	save_item(NAME(m_ef934x_ram_a));
	save_item(NAME(m_ef934x_ram_b));
	save_item(NAME(m_ef9340.X));
	save_item(NAME(m_ef9340.Y));
	save_item(NAME(m_ef9340.Y0));
	save_item(NAME(m_ef9340.R));
	save_item(NAME(m_ef9340.M));
	save_item(NAME(m_ef9341.TA));
	save_item(NAME(m_ef9341.TB));
	save_item(NAME(m_ef9341.busy));
	save_item(NAME(m_ef934x_ext_char_ram));
}


void odyssey2_state::machine_reset()
{
	m_lum = 0;

	/* jump to "last" bank, will work for all sizes due to being mirrored */
	m_p1 = 0xFF;
	m_p2 = 0xFF;
	switch_banks();

	for ( int i = 0; i < 8; i++ )
	{
		m_g7400_ic674_decode[i] = 0;
		m_g7400_ic678_decode[i] = 0;
	}
}

/****** External RAM ******************************/

READ8_MEMBER(odyssey2_state::io_read)
{
	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
	{
		return m_i8244->read(space, offset); /* seems to have higher priority than ram??? */
	}
	if (!(m_p1 & P1_EXT_RAM_ENABLE))
	{
		return m_ram[offset];
	}

	return 0;
}


WRITE8_MEMBER(odyssey2_state::io_write)
{
	if ((m_p1 & (P1_EXT_RAM_ENABLE | P1_VDC_COPY_MODE_ENABLE)) == 0x00)
	{
		m_ram[offset] = data;
		if ( offset & 0x80 )
		{
			if ( data & 0x20 )
			{
				logerror("voice write %02X, data = %02X (p1 = %02X)\n", offset, data, m_p1 );
				sp0256_ALD_w( machine().device("sp0256_speech"), space, 0, offset & 0x7F );
			}
			else
			{
				/* TODO: Reset sp0256 in this case */
			}
		}
	}
	else if (!(m_p1 & P1_VDC_ENABLE))
	{
		m_i8244->write(space, offset, data);
	}
}


READ8_MEMBER(odyssey2_state::g7400_io_read)
{
	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
	{
		return m_i8244->read(space, offset); /* seems to have higher priority than ram??? */
	}
	else if (!(m_p1 & P1_EXT_RAM_ENABLE))
	{
		return m_ram[offset];
	}
	else
	{
		return ef9341_r( offset & 0x02, offset & 0x01 );
	}

	return 0;
}


WRITE8_MEMBER(odyssey2_state::g7400_io_write)
{
	if ((m_p1 & (P1_EXT_RAM_ENABLE | P1_VDC_COPY_MODE_ENABLE)) == 0x00)
	{
		m_ram[offset] = data;
	}
	else if (!(m_p1 & P1_VDC_ENABLE))
	{
		m_i8244->write(space, offset, data);
	}
	else
	{
		ef9341_w( offset & 0x02, offset & 0x01, data );
	}
}

