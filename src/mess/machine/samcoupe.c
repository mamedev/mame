/***************************************************************************

    SAM Coupe Driver - Written By Lee Hammerton, Dirk Best

***************************************************************************/

#include "emu.h"
#include "includes/samcoupe.h"
#include "machine/msm6242.h"
#include "machine/ram.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LMPR_RAM0    0x20	/* If bit set ram is paged into bank 0, else its rom0 */
#define LMPR_ROM1    0x40	/* If bit set rom1 is paged into bank 3, else its ram */
#define HMPR_MCNTRL  0x80	/* If set external RAM is enabled */


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

static void samcoupe_update_bank(address_space &space, int bank_num, UINT8 *memory, int is_readonly)
{
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	samcoupe_state *state = space.machine().driver_data<samcoupe_state>();
	if (memory)
	{
		state->membank(bank)->set_base(memory);
		space.install_read_bank (((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF, bank);
		if (is_readonly) {
			space.unmap_write(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF);
		} else {
			space.install_write_bank(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF, bank);
		}
	} else {
		space.nop_readwrite(((bank_num-1) * 0x4000), ((bank_num-1) * 0x4000) + 0x3FFF);
	}
}


static void samcoupe_install_ext_mem(address_space &space)
{
	samcoupe_state *state = space.machine().driver_data<samcoupe_state>();
	UINT8 *mem;

	/* bank 3 */
	if (state->m_lext >> 6 < space.machine().device<ram_device>(RAM_TAG)->size() >> 20)
		mem = &space.machine().device<ram_device>(RAM_TAG)->pointer()[(space.machine().device<ram_device>(RAM_TAG)->size() & 0xfffff) + (state->m_lext >> 6) * 0x100000 + (state->m_lext & 0x3f) * 0x4000];
	else
		mem = NULL;

	samcoupe_update_bank(space, 3, mem, FALSE);

	/* bank 4 */
	if (state->m_hext >> 6 < space.machine().device<ram_device>(RAM_TAG)->size() >> 20)
		mem = &space.machine().device<ram_device>(RAM_TAG)->pointer()[(space.machine().device<ram_device>(RAM_TAG)->size() & 0xfffff) + (state->m_hext >> 6) * 0x100000 + (state->m_hext & 0x3f) * 0x4000];
	else
		mem = NULL;

	samcoupe_update_bank(space, 4, mem, FALSE);
}


void samcoupe_update_memory(address_space &space)
{
	samcoupe_state *state = space.machine().driver_data<samcoupe_state>();
	const int PAGE_MASK = ((space.machine().device<ram_device>(RAM_TAG)->size() & 0xfffff) / 0x4000) - 1;
	UINT8 *rom = state->memregion("maincpu")->base();
	UINT8 *memory;
	int is_readonly;

	/* BANK1 */
    if (state->m_lmpr & LMPR_RAM0)   /* Is ram paged in at bank 1 */
	{
		if ((state->m_lmpr & 0x1F) <= PAGE_MASK)
			memory = &space.machine().device<ram_device>(RAM_TAG)->pointer()[(state->m_lmpr & PAGE_MASK) * 0x4000];
		else
			memory = NULL;	/* Attempt to page in non existant ram region */
		is_readonly = FALSE;
	}
	else
	{
		memory = rom;	/* Rom0 paged in */
		is_readonly = TRUE;
	}
	samcoupe_update_bank(space, 1, memory, is_readonly);


	/* BANK2 */
	if (((state->m_lmpr + 1) & 0x1f) <= PAGE_MASK)
		memory = &space.machine().device<ram_device>(RAM_TAG)->pointer()[((state->m_lmpr + 1) & PAGE_MASK) * 0x4000];
	else
		memory = NULL;	/* Attempt to page in non existant ram region */
	samcoupe_update_bank(space, 2, memory, FALSE);

	/* only update bank 3 and 4 when external memory is not enabled */
	if (state->m_hmpr & HMPR_MCNTRL)
	{
		samcoupe_install_ext_mem(space);
	}
	else
	{
		/* BANK3 */
		if ((state->m_hmpr & 0x1F) <= PAGE_MASK )
			memory = &space.machine().device<ram_device>(RAM_TAG)->pointer()[(state->m_hmpr & PAGE_MASK)*0x4000];
		else
			memory = NULL;	/* Attempt to page in non existant ram region */
		samcoupe_update_bank(space, 3, memory, FALSE);


		/* BANK4 */
		if (state->m_lmpr & LMPR_ROM1)	/* Is Rom1 paged in at bank 4 */
		{
			memory = rom + 0x4000;
			is_readonly = TRUE;
		}
		else
		{
			if (((state->m_hmpr + 1) & 0x1f) <= PAGE_MASK)
				memory = &space.machine().device<ram_device>(RAM_TAG)->pointer()[((state->m_hmpr + 1) & PAGE_MASK) * 0x4000];
			else
				memory = NULL;	/* Attempt to page in non existant ram region */
			is_readonly = FALSE;
		}
		samcoupe_update_bank(space, 4, memory, FALSE);
	}

	/* video memory location */
	if (state->m_vmpr & 0x40)	/* if bit set in 2 bank screen mode */
		state->m_videoram = &space.machine().device<ram_device>(RAM_TAG)->pointer()[((state->m_vmpr & 0x1e) & PAGE_MASK) * 0x4000];
	else
		state->m_videoram = &space.machine().device<ram_device>(RAM_TAG)->pointer()[((state->m_vmpr & 0x1f) & PAGE_MASK) * 0x4000];
}


WRITE8_MEMBER(samcoupe_state::samcoupe_ext_mem_w)
{
	address_space &space_program = machine().device("maincpu")->memory().space(AS_PROGRAM);

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

static READ8_DEVICE_HANDLER( samcoupe_rtc_r )
{
	address_space &spaceio = device->machine().device("maincpu")->memory().space(AS_IO);
	msm6242_device *rtc = dynamic_cast<msm6242_device*>(device);
	return rtc->read(spaceio,offset >> 12);
}


static WRITE8_DEVICE_HANDLER( samcoupe_rtc_w )
{
	address_space &spaceio = device->machine().device("maincpu")->memory().space(AS_IO);
	msm6242_device *rtc = dynamic_cast<msm6242_device*>(device);
	rtc->write(spaceio,offset >> 12, data);
}


/***************************************************************************
    MOUSE
***************************************************************************/

static TIMER_CALLBACK( samcoupe_mouse_reset )
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	state->m_mouse_index = 0;
}

UINT8 samcoupe_mouse_r(running_machine &machine)
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	UINT8 result;

	/* on a read, reset the timer */
	state->m_mouse_reset->adjust(attotime::from_usec(50));

	/* update when we are about to read the first real values */
	if (state->m_mouse_index == 2)
	{
		/* update values */
		int mouse_x = machine.root_device().ioport("mouse_x")->read();
		int mouse_y = machine.root_device().ioport("mouse_y")->read();

		int mouse_dx = state->m_mouse_x - mouse_x;
		int mouse_dy = state->m_mouse_y - mouse_y;

		state->m_mouse_x = mouse_x;
		state->m_mouse_y = mouse_y;

		/* button state */
		state->m_mouse_data[2] = machine.root_device().ioport("mouse_buttons")->read();

		/* y-axis */
		state->m_mouse_data[3] = (mouse_dy & 0xf00) >> 8;
		state->m_mouse_data[4] = (mouse_dy & 0x0f0) >> 4;
		state->m_mouse_data[5] = (mouse_dy & 0x00f) >> 0;

		/* x-axis */
		state->m_mouse_data[6] = (mouse_dx & 0xf00) >> 8;
		state->m_mouse_data[7] = (mouse_dx & 0x0f0) >> 4;
		state->m_mouse_data[8] = (mouse_dx & 0x00f) >> 0;
	}

	/* get current value */
	result = state->m_mouse_data[state->m_mouse_index++];

	/* reset if we are at the end */
	if (state->m_mouse_index == sizeof(state->m_mouse_data))
		state->m_mouse_index = 1;

	return result;
}

void samcoupe_state::machine_start()
{
	m_mouse_reset = machine().scheduler().timer_alloc(FUNC(samcoupe_mouse_reset));

	/* schedule our video updates */
	m_video_update_timer = machine().scheduler().timer_alloc(FUNC(sam_video_update_callback));
	m_video_update_timer->adjust(machine().primary_screen->time_until_pos(0, 0));
}

/***************************************************************************
    RESET
***************************************************************************/

void samcoupe_state::machine_reset()
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	address_space &spaceio = machine().device("maincpu")->memory().space(AS_IO);

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

	if (machine().root_device().ioport("config")->read() & 0x01)
	{
		/* install RTC */
		device_t *rtc = machine().device("sambus_clock");
		spaceio.install_legacy_readwrite_handler(*rtc, 0xef, 0xef, 0xffff, 0xff00, FUNC(samcoupe_rtc_r), FUNC(samcoupe_rtc_w));
	}
	else
	{
		/* no RTC support */
		spaceio.unmap_readwrite(0xef, 0xef, 0xffff, 0xff00);
	}

	/* initialize memory */
	samcoupe_update_memory(space);
}
