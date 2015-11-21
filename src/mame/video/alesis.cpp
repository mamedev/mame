// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis SR-16 LCD emulation

****************************************************************************/

#include "emu.h"
#include "includes/alesis.h"


void alesis_state::update_lcd_symbols(bitmap_ind16 &bitmap, UINT8 pos, UINT8 y, UINT8 x, int state)
{
	if (pos == 6)
	{
		switch(x)
		{
			case 0:
				switch(y)
				{
					case 0: output_set_value("a_next", state);  break;
					case 1: output_set_value("b_next", state);  break;
					case 2: output_set_value("fill_next", state); break;
					case 3: output_set_value("user_next", state); break;
					case 4: output_set_value("play", state);    break;
					case 5: output_set_value("record", state);  break;
					case 6: output_set_value("compose", state); break;
					case 7: output_set_value("perform", state); break;
				}
				break;
			case 1:
				if (y == 0)
				{
					output_set_value("song", state);
				}
				else if (y < 8)
				{
					if (state)
						m_lcd_digits[0] |= (1<<(y-1));
					else
						m_lcd_digits[0] &= ~(1<<(y-1));

					output_set_value("digit0", BITSWAP8(m_lcd_digits[0],7,3,1,4,6,5,2,0));
				}
				break;
			case 2:
				if (y == 0)
				{
					output_set_value("pattern", state);
				}
				else if (y < 8)
				{
					if (state)
						m_lcd_digits[1] |= (1<<(y-1));
					else
						m_lcd_digits[1] &= ~(1<<(y-1));

					output_set_value("digit1", BITSWAP8(m_lcd_digits[1],7,3,1,4,6,5,2,0));
				}
				break;
			case 3:
				switch(y)
				{
					case 0: output_set_value("b", state);       break;
					case 1: output_set_value("a", state);       break;
					case 2: output_set_value("fill", state);    break;
					case 3: output_set_value("user", state);    break;
					case 4: output_set_value("edited", state);  break;
					case 5: output_set_value("set", state);     break;
					case 6: output_set_value("drum", state);    break;
					case 7: output_set_value("press_play", state); break;
				}
				break;
			case 4:
				if (y == 7)
				{
					output_set_value("metronome", state);
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[4] |= (1<<y);
					else
						m_lcd_digits[4] &= ~(1<<y);

					output_set_value("digit4", BITSWAP8(m_lcd_digits[4],7,3,5,2,0,1,4,6));
				}
				break;
		}
	}
	else
	{
		switch(x)
		{
			case 0:
				if (y == 7)
				{
					output_set_value("tempo", state);
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[3] |= (1<<y);
					else
						m_lcd_digits[3] &= ~(1<<y);

					output_set_value("digit3", BITSWAP8(m_lcd_digits[3],7,3,5,2,0,1,4,6));
				}
				break;
			case 1:
				if (y == 7)
				{
					output_set_value("page", state);
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[2] |= (1<<y);
					else
						m_lcd_digits[2] &= ~(1<<y);

					output_set_value("digit2", BITSWAP8(m_lcd_digits[2],7,3,5,2,0,1,4,6));
				}
				break;
			case 2:
				switch(y)
				{
					case 0: output_set_value("step_edit", state);   break;
					case 1: output_set_value("swing_off", state);   break;
					case 2: output_set_value("swing_62", state);    break;
					case 3: output_set_value("click_l1", state);    break;
					case 4: output_set_value("click_note", state);  break;
					case 5: output_set_value("click_l2", state);    break;
					case 6: output_set_value("click_3", state);     break;
					case 7: output_set_value("backup", state);      break;
				}
				break;
			case 3:
				switch(y)
				{
					case 0: output_set_value("drum_set", state);    break;
					case 1: output_set_value("swing", state);       break;
					case 2: output_set_value("swing_58", state);    break;
					case 3: output_set_value("click_off", state);   break;
					case 4: output_set_value("click", state);       break;
					case 5: output_set_value("quantize_off", state); break;
					case 6: output_set_value("quantize_3", state);  break;
					case 7: output_set_value("midi_setup", state);  break;
				}
				break;
			case 4:
				switch(y)
				{
					case 0: output_set_value("record_setup", state); break;
					case 1: output_set_value("quantize", state);    break;
					case 2: output_set_value("swing_54", state);    break;
					case 3: output_set_value("quantize_l1", state); break;
					case 4: output_set_value("quantize_l2", state); break;
					case 5: output_set_value("quantize_l3", state); break;
					case 6: output_set_value("quantize_note", state); break;
					case 7: output_set_value("setup", state);       break;
				}
				break;
		}
	}
}
