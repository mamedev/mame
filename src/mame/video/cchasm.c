/***************************************************************************

    Cinematronics Cosmic Chasm hardware

***************************************************************************/

#include "emu.h"
#include "video/vector.h"
#include "includes/cchasm.h"

#define HALT   0
#define JUMP   1
#define COLOR  2
#define SCALEY 3
#define POSY   4
#define SCALEX 5
#define POSX   6
#define LENGTH 7



TIMER_CALLBACK_MEMBER(cchasm_state::cchasm_refresh_end)
{
	machine().device("maincpu")->execute().set_input_line(2, ASSERT_LINE);
}

static void cchasm_refresh (running_machine &machine)
{
	cchasm_state *state = machine.driver_data<cchasm_state>();

	int pc = 0;
	int done = 0;
	int opcode, data;
	int currentx = 0, currenty = 0;
	int scalex = 0, scaley = 0;
	int color = 0;
	int total_length = 1;   /* length of all lines drawn in a frame */
	int move = 0;

	vector_clear_list();

	while (!done)
	{
		data = state->m_ram[pc];
		opcode = data >> 12;
		data &= 0xfff;
		if ((opcode > COLOR) && (data & 0x800))
			data |= 0xfffff000;

		pc++;

		switch (opcode)
		{
		case HALT:
			done=1;
			break;
		case JUMP:
			pc = data - 0xb00;
			logerror("JUMP to %x\n", data);
			break;
		case COLOR:
			color = VECTOR_COLOR444(data ^ 0xfff);
			break;
		case SCALEY:
			scaley = data << 5;
			break;
		case POSY:
			move = 1;
			currenty = state->m_ycenter + (data << 16);
			break;
		case SCALEX:
			scalex = data << 5;
			break;
		case POSX:
			move = 1;
			currentx = state->m_xcenter - (data << 16);
			break;
		case LENGTH:
			if (move)
			{
				vector_add_point (machine, currentx, currenty, 0, 0);
				move = 0;
			}

			currentx -= data * scalex;
			currenty += data * scaley;

			total_length += abs(data);

			if (color)
				vector_add_point (machine, currentx, currenty, color, 0xff);
			else
				move = 1;
			break;
		default:
			logerror("Unknown refresh proc opcode %x with data %x at pc = %x\n", opcode, data, pc-2);
			done = 1;
			break;
		}
	}
	/* Refresh processor runs with 6 MHz */
	machine.scheduler().timer_set (attotime::from_hz(6000000) * total_length, timer_expired_delegate(FUNC(cchasm_state::cchasm_refresh_end),state));
}


WRITE16_MEMBER(cchasm_state::cchasm_refresh_control_w)
{
	if (ACCESSING_BITS_8_15)
	{
		switch (data >> 8)
		{
		case 0x37:
			cchasm_refresh(machine());
			break;
		case 0xf7:
			machine().device("maincpu")->execute().set_input_line(2, CLEAR_LINE);
			break;
		}
	}
}

void cchasm_state::video_start()
{
	const rectangle &visarea = machine().primary_screen->visible_area();

	m_xcenter=visarea.xcenter() << 16;
	m_ycenter=visarea.ycenter() << 16;

	VIDEO_START_CALL_LEGACY(vector);
}
