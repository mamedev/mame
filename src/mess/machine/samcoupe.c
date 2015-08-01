// license:GPL-2.0+
// copyright-holders:Lee Hammerton, Dirk Best
/***************************************************************************

    Miles Gordon Technology SAM Coupe

***************************************************************************/

#include "emu.h"
#include "includes/samcoupe.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LMPR_RAM0    0x20   /* If bit set ram is paged into bank 0, else its rom0 */
#define LMPR_ROM1    0x40   /* If bit set rom1 is paged into bank 3, else its ram */
#define HMPR_MCNTRL  0x80   /* If set external RAM is enabled */


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

READ8_MEMBER(samcoupe_state::sam_bank1_r)
{
	if (sam_bank_read_ptr[0])
		return sam_bank_read_ptr[0][offset];

	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::sam_bank1_w)
{
	if (sam_bank_write_ptr[0])
		sam_bank_write_ptr[0][offset] = data;
}


READ8_MEMBER(samcoupe_state::sam_bank2_r)
{
	if (sam_bank_read_ptr[1])
		return sam_bank_read_ptr[1][offset];

	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::sam_bank2_w)
{
	if (sam_bank_write_ptr[1])
		sam_bank_write_ptr[1][offset] = data;
}


READ8_MEMBER(samcoupe_state::sam_bank3_r)
{
	if (sam_bank_read_ptr[2])
		return sam_bank_read_ptr[2][offset];

	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::sam_bank3_w)
{
	if (sam_bank_write_ptr[2])
		sam_bank_write_ptr[2][offset] = data;
}


READ8_MEMBER(samcoupe_state::sam_bank4_r)
{
	if (sam_bank_read_ptr[3])
		return sam_bank_read_ptr[3][offset];

	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::sam_bank4_w)
{
	if (sam_bank_write_ptr[3])
		sam_bank_write_ptr[3][offset] = data;
}

void samcoupe_state::samcoupe_update_bank(address_space &space, int bank_num, UINT8 *memory, int is_readonly)
{
	sam_bank_read_ptr[bank_num-1] = memory;
	if (!is_readonly)
		sam_bank_write_ptr[bank_num-1] = memory;
	else
		sam_bank_write_ptr[bank_num-1] = NULL;

	// installing banks on the fly is too slow (20% speed in Manic Miner gameplay vs 300% speed)
#if 0
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	if (memory)
	{
		membank(bank)->set_base(memory);
		space.install_read_bank (((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF, bank);
		if (is_readonly) {
			space.unmap_write(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF);
		} else {
			space.install_write_bank(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF, bank);
		}
	} else {
		space.nop_readwrite(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF);
	}
#endif
}


void samcoupe_state::samcoupe_install_ext_mem(address_space &space)
{
	UINT8 *mem;

	/* bank 3 */
	if (m_lext >> 6 < m_ram->size() >> 20)
		mem = &m_ram->pointer()[(m_ram->size() & 0xfffff) + (m_lext >> 6) * 0x100000 + (m_lext & 0x3f) * 0x4000];
	else
		mem = NULL;

	samcoupe_update_bank(space, 3, mem, FALSE);

	/* bank 4 */
	if (m_hext >> 6 < m_ram->size() >> 20)
		mem = &m_ram->pointer()[(m_ram->size() & 0xfffff) + (m_hext >> 6) * 0x100000 + (m_hext & 0x3f) * 0x4000];
	else
		mem = NULL;

	samcoupe_update_bank(space, 4, mem, FALSE);
}


void samcoupe_state::samcoupe_update_memory(address_space &space)
{
	const int PAGE_MASK = ((m_ram->size() & 0xfffff) / 0x4000) - 1;
	UINT8 *rom = m_region_maincpu->base();
	UINT8 *memory;
	int is_readonly;

	/* BANK1 */
	if (m_lmpr & LMPR_RAM0)   /* Is ram paged in at bank 1 */
	{
		if ((m_lmpr & 0x1F) <= PAGE_MASK)
			memory = &m_ram->pointer()[(m_lmpr & PAGE_MASK) * 0x4000];
		else
			memory = NULL;  /* Attempt to page in non existant ram region */
		is_readonly = FALSE;
	}
	else
	{
		memory = rom;   /* Rom0 paged in */
		is_readonly = TRUE;
	}
	samcoupe_update_bank(space, 1, memory, is_readonly);


	/* BANK2 */
	if (((m_lmpr + 1) & 0x1f) <= PAGE_MASK)
		memory = &m_ram->pointer()[((m_lmpr + 1) & PAGE_MASK) * 0x4000];
	else
		memory = NULL;  /* Attempt to page in non existant ram region */
	samcoupe_update_bank(space, 2, memory, FALSE);

	/* only update bank 3 and 4 when external memory is not enabled */
	if (m_hmpr & HMPR_MCNTRL)
	{
		samcoupe_install_ext_mem(space);
	}
	else
	{
		/* BANK3 */
		if ((m_hmpr & 0x1F) <= PAGE_MASK )
			memory = &m_ram->pointer()[(m_hmpr & PAGE_MASK)*0x4000];
		else
			memory = NULL;  /* Attempt to page in non existant ram region */
		samcoupe_update_bank(space, 3, memory, FALSE);


		/* BANK4 */
		if (m_lmpr & LMPR_ROM1)  /* Is Rom1 paged in at bank 4 */
		{
			memory = rom + 0x4000;
			is_readonly = TRUE;
		}
		else
		{
			if (((m_hmpr + 1) & 0x1f) <= PAGE_MASK)
				memory = &m_ram->pointer()[((m_hmpr + 1) & PAGE_MASK) * 0x4000];
			else
				memory = NULL;  /* Attempt to page in non existant ram region */
			is_readonly = FALSE;
		}
		samcoupe_update_bank(space, 4, memory, FALSE);
	}

	/* video memory location */
	if (m_vmpr & 0x40)   /* if bit set in 2 bank screen mode */
		m_videoram = &m_ram->pointer()[((m_vmpr & 0x1e) & PAGE_MASK) * 0x4000];
	else
		m_videoram = &m_ram->pointer()[((m_vmpr & 0x1f) & PAGE_MASK) * 0x4000];
}


WRITE8_MEMBER(samcoupe_state::samcoupe_ext_mem_w)
{
	address_space &space_program = m_maincpu->space(AS_PROGRAM);

	if (offset & 1)
		m_hext = data;
	else
		m_lext = data;

	/* external RAM enabled? */
	if (m_hmpr & HMPR_MCNTRL)
	{
		samcoupe_install_ext_mem(space_program);
	}
}


/***************************************************************************
    REAL TIME CLOCK
***************************************************************************/

READ8_MEMBER(samcoupe_state::samcoupe_rtc_r)
{
	address_space &spaceio = m_maincpu->space(AS_IO);
	return m_rtc->read(spaceio, offset >> 12);
}


WRITE8_MEMBER(samcoupe_state::samcoupe_rtc_w)
{
	address_space &spaceio = m_maincpu->space(AS_IO);
	m_rtc->write(spaceio, offset >> 12, data);
}


/***************************************************************************
    MOUSE
***************************************************************************/

TIMER_CALLBACK_MEMBER(samcoupe_state::samcoupe_mouse_reset)
{
	m_mouse_index = 0;
}

UINT8 samcoupe_state::samcoupe_mouse_r()
{
	UINT8 result;

	/* on a read, reset the timer */
	m_mouse_reset->adjust(attotime::from_usec(50));

	/* update when we are about to read the first real values */
	if (m_mouse_index == 2)
	{
		/* update values */
		int mouse_x = m_io_mouse_x->read();
		int mouse_y = m_io_mouse_y->read();

		int mouse_dx = m_mouse_x - mouse_x;
		int mouse_dy = m_mouse_y - mouse_y;

		m_mouse_x = mouse_x;
		m_mouse_y = mouse_y;

		/* button state */
		m_mouse_data[2] = m_mouse_buttons->read();

		/* y-axis */
		m_mouse_data[3] = (mouse_dy & 0xf00) >> 8;
		m_mouse_data[4] = (mouse_dy & 0x0f0) >> 4;
		m_mouse_data[5] = (mouse_dy & 0x00f) >> 0;

		/* x-axis */
		m_mouse_data[6] = (mouse_dx & 0xf00) >> 8;
		m_mouse_data[7] = (mouse_dx & 0x0f0) >> 4;
		m_mouse_data[8] = (mouse_dx & 0x00f) >> 0;
	}

	/* get current value */
	result = m_mouse_data[m_mouse_index++];

	/* reset if we are at the end */
	if (m_mouse_index == sizeof(m_mouse_data))
		m_mouse_index = 1;

	return result;
}

void samcoupe_state::machine_start()
{
	m_mouse_reset = timer_alloc(TIMER_MOUSE_RESET);

	/* schedule our video updates */
	m_video_update_timer = timer_alloc(TIMER_VIDEO_UPDATE);
	m_video_update_timer->adjust(machine().first_screen()->time_until_pos(0, 0));
}

/***************************************************************************
    RESET
***************************************************************************/

void samcoupe_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	address_space &spaceio = m_maincpu->space(AS_IO);

	/* initialize state */
	m_lmpr = 0x0f;      /* ROM0 paged in, ROM1 paged out RAM Banks */
	m_hmpr = 0x01;
	m_vmpr = 0x81;
	m_line_int = 0xff;  /* line interrupts disabled */
	m_status = 0x1f;    /* no interrupts active */

	/* initialize mouse */
	m_mouse_index = 0;
	m_mouse_data[0] = 0xff;
	m_mouse_data[1] = 0xff;

	if (m_config->read() & 0x01)
	{
		/* install RTC */
		spaceio.install_readwrite_handler(0xef, 0xef, 0xffff, 0xff00, read8_delegate(FUNC(samcoupe_state::samcoupe_rtc_r),this), write8_delegate(FUNC(samcoupe_state::samcoupe_rtc_w),this));
	}
	else
	{
		/* no RTC support */
		spaceio.unmap_readwrite(0xef, 0xef, 0xffff, 0xff00);
	}

	/* initialize memory */
	samcoupe_update_memory(space);
}
