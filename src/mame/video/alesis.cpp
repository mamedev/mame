// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis SR-16 LCD emulation

****************************************************************************/

#include "emu.h"
#include "includes/alesis.h"


void alesis_state::update_lcd_symbols(bitmap_ind16 &bitmap, uint8_t pos, uint8_t y, uint8_t x, int state)
{
	if (pos == 6)
	{
		switch(x)
		{
			case 0:
				switch(y)
				{
					case 0: m_a_next = state;  break;
					case 1: m_b_next = state;  break;
					case 2: m_fill_next = state; break;
					case 3: m_user_next = state; break;
					case 4: m_play = state;    break;
					case 5: m_record = state;  break;
					case 6: m_compose = state; break;
					case 7: m_perform = state; break;
				}
				break;
			case 1:
				if (y == 0)
				{
					m_song = state;
				}
				else if (y < 8)
				{
					if (state)
						m_lcd_digits[0] |= (1<<(y-1));
					else
						m_lcd_digits[0] &= ~(1<<(y-1));

					m_digit[0] = bitswap<8>(m_lcd_digits[0],7,3,1,4,6,5,2,0);
				}
				break;
			case 2:
				if (y == 0)
				{
					output().set_value("pattern", state);
				}
				else if (y < 8)
				{
					if (state)
						m_lcd_digits[1] |= (1<<(y-1));
					else
						m_lcd_digits[1] &= ~(1<<(y-1));

					m_digit[1] = bitswap<8>(m_lcd_digits[1],7,3,1,4,6,5,2,0);
				}
				break;
			case 3:
				switch(y)
				{
					case 0: m_b = state;       break;
					case 1: m_a = state;       break;
					case 2: m_fill = state;    break;
					case 3: m_user = state;    break;
					case 4: m_edited = state;  break;
					case 5: m_set = state;     break;
					case 6: m_drum = state;    break;
					case 7: m_press_play = state; break;
				}
				break;
			case 4:
				if (y == 7)
				{
					m_metronome = state;
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[4] |= (1<<y);
					else
						m_lcd_digits[4] &= ~(1<<y);

					m_digit[4] = bitswap<8>(m_lcd_digits[4],7,3,5,2,0,1,4,6);
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
					m_tempo = state;
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[3] |= (1<<y);
					else
						m_lcd_digits[3] &= ~(1<<y);

					m_digit[3] = bitswap<8>(m_lcd_digits[3],7,3,5,2,0,1,4,6);
				}
				break;
			case 1:
				if (y == 7)
				{
					m_page = state;
				}
				else if (y < 7)
				{
					if (state)
						m_lcd_digits[2] |= (1<<y);
					else
						m_lcd_digits[2] &= ~(1<<y);

					m_digit[2] = bitswap<8>(m_lcd_digits[2],7,3,5,2,0,1,4,6);
				}
				break;
			case 2:
				switch(y)
				{
					case 0: m_step_edit = state;   break;
					case 1: m_swing_off = state;   break;
					case 2: m_swing_62 = state;    break;
					case 3: m_click_l1 = state;    break;
					case 4: m_click_note = state;  break;
					case 5: m_click_l2 = state;    break;
					case 6: m_click_3 = state;     break;
					case 7: m_backup = state;      break;
				}
				break;
			case 3:
				switch(y)
				{
					case 0: m_drum_set = state;    break;
					case 1: m_swing = state;       break;
					case 2: m_swing_58 = state;    break;
					case 3: m_click_off = state;   break;
					case 4: m_click = state;       break;
					case 5: m_quantize_off = state; break;
					case 6: m_quantize_3 = state;  break;
					case 7: m_midi_setup = state;  break;
				}
				break;
			case 4:
				switch(y)
				{
					case 0: m_record_setup = state; break;
					case 1: m_quantize = state;    break;
					case 2: m_swing_54 = state;    break;
					case 3: m_quantize_l1 = state; break;
					case 4: m_quantize_l2 = state; break;
					case 5: m_quantize_l3 = state; break;
					case 6: m_quantize_note = state; break;
					case 7: m_setup = state;       break;
				}
				break;
		}
	}
}
