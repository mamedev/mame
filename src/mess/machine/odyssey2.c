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
		return video_read(space, offset); /* seems to have higher priority than ram??? */
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
		video_write(space, offset, data);
	}
}


READ8_MEMBER(odyssey2_state::g7400_io_read)
{
	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
	{
		return video_read(space, offset); /* seems to have higher priority than ram??? */
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
		video_write(space, offset, data);
	}
	else
	{
		ef9341_w( offset & 0x02, offset & 0x01, data );
	}
}

/***** 8048 Ports ************************/

READ8_MEMBER(odyssey2_state::p1_read)
{
	UINT8 data = m_p1;

	logerror("%.9f p1 read %.2x\n", machine().time().as_double(), data);
	return data;
}


WRITE8_MEMBER(odyssey2_state::p1_write)
{
	m_p1 = data;

	switch_banks();

	lum_write(space, 0, m_p1 >> 7);

	logerror("%.6f p1 written %.2x\n", machine().time().as_double(), data);
}


READ8_MEMBER(odyssey2_state::p2_read)
{
	UINT8 h = 0xFF;
	int i, j;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	if (!(m_p1 & P1_KEYBOARD_SCAN_ENABLE))
	{
		if ((m_p2 & P2_KEYBOARD_SELECT_MASK) <= 5)  /* read keyboard */
		{
			h &= ioport(keynames[m_p2 & P2_KEYBOARD_SELECT_MASK])->read();
		}

		for (i= 0x80, j = 0; i > 0; i >>= 1, j++)
		{
			if (!(h & i))
			{
				m_p2 &= ~0x10;                   /* set key was pressed indicator */
				m_p2 = (m_p2 & ~0xE0) | (j << 5);  /* column that was pressed */

				break;
			}
		}

		if (h == 0xFF)  /* active low inputs, so no keypresses */
		{
			m_p2 = m_p2 | 0xF0;
		}
    }
    else
	{
		m_p2 = m_p2 | 0xF0;
	}

	logerror("%.6f p2 read %.2x\n", machine().time().as_double(), m_p2);
	return m_p2;
}


WRITE8_MEMBER(odyssey2_state::p2_write)
{
	m_p2 = data;

	if ( m_i8243 )
	{
		m_i8243->i8243_p2_w( space, 0, m_p2 & 0x0f );
	}

	logerror("%.6f p2 written %.2x\n", machine().time().as_double(), data);
}


READ8_MEMBER(odyssey2_state::bus_read)
{
	UINT8 data = 0xff;

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 1)
	{
		data &= ioport("JOY0")->read();       /* read joystick 1 */
	}

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 0)
	{
		data &= ioport("JOY1")->read();       /* read joystick 2 */
	}

	logerror("%.6f bus read %.2x\n", machine().time().as_double(), data);
	return data;
}


WRITE8_MEMBER(odyssey2_state::bus_write)
{
	logerror("%.6f bus written %.2x\n", machine().time().as_double(), data);
}

