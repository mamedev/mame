// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek

#include "emu.h"
#include "pcecommn.h"
#include "cpu/h6280/h6280.h"

#define TG_16_JOY_SIG       0x00
#define PCE_JOY_SIG         0x40
#define NO_CD_SIG           0x80
#define CD_SIG              0x00
/* these might be used to indicate something, but they always seem to return 1 */
#define CONST_SIG           0x30

/* joystick related data*/
#define JOY_CLOCK   0x01
#define JOY_RESET   0x02


/* todo: how many input ports does the PCE have? */
void pce_common_state::pce_joystick_w(uint8_t data)
{
	/* bump counter on a low-to-high transition of bit 1 */
	if((!m_joystick_data_select) && (data & JOY_CLOCK))
	{
		m_joystick_port_select = (m_joystick_port_select + 1) & 0x07;
	}

	/* do we want buttons or direction? */
	m_joystick_data_select = data & JOY_CLOCK;

	/* clear counter if bit 2 is set */
	if(data & JOY_RESET)
	{
		m_joystick_port_select = 0;
	}
}

uint8_t pce_common_state::joy_read()
{
	return ioport("JOY")->read();
}

uint8_t pce_common_state::pce_joystick_r()
{
	uint8_t ret;
	int data = joy_read();
	if (m_joystick_data_select) data >>= 4;
	ret = (data & 0x0F) | m_io_port_options;
#ifdef UNIFIED_PCE
	ret &= ~0x40;
#endif
	return (ret);
}

void pce_common_state::init_pce_common()
{
	m_io_port_options = PCE_JOY_SIG | CONST_SIG;
}

uint32_t pce_common_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_huc6260->video_update( bitmap, cliprect );
	return 0;
}
