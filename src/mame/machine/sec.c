// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Serial Electronic Counter (SEC) */

#include "emu.h"
#include "sec.h"



void SEC::reset(void)
{
	m_clk = 1;
	m_curbyte = m_clks = n_reqpos = m_rxclk = m_rxlen = m_rxpos = 0;
}

void SEC::write_cs_line(UINT8 bit)
{
	if ( bit )
	{
		if ( !enabled )
		{
			enabled = true;
			m_rxdat = 0;
		}
	}
	else
	{
		if ( enabled )
		{
			m_rxdat = 1;
		}

		enabled = false;
		reset();
	}
}

void SEC::write_data_line(UINT8 bit)
{
	m_data = bit ? 1 : 0;
}

UINT8 SEC::read_data_line(void)
{
	return m_rxdat;
}

void SEC::write_clock_line(UINT8 bit)
{
	bit = bit ? 1 : 0;

	if ( (m_clk ^ bit) & 1 )
	{
		if ( !bit )
		{
			m_curbyte = ( m_curbyte << 1 ) | m_data;
			if ( m_rxclk == 8 )
			{
				m_rxclk = 0;
				m_rxpos++;
				m_rxlen--;
			}

			if ( m_rxlen )
				m_rxdat = (m_reply[m_rxpos] & 0x80) >> 7;
			else
				m_rxdat = enabled ? 0 : 1;
		}
		else
		{
			m_clks++;
			if ( m_rxlen )
			{
				m_reply[m_rxpos] <<= 1;
				m_rxclk++;
			}

			if ( m_clks == 8 )
			{
				m_clks = 0;
				if ( !m_rxlen )
				{
					m_request[n_reqpos++] = m_curbyte;
					if ( chars_left )
					{
						chars_left--;
						if ( !chars_left )
						{
							n_reqpos--;
							Do_Command();
							n_reqpos = 0;
						}
					}
					if ( n_reqpos == 3 )
						chars_left = m_curbyte + 1;
				}
			}
		}
		m_clk = bit;
	}
}



void SEC::Do_Command(void)
{
	m_last = m_request[1];
	switch ( m_request[0] )
	{
		case SEC_REQUEST_VERSION:    Cmd_Get_Ver(); break;
		case SEC_REQUEST_STATUS:     Cmd_Get_Sta(); break;
		case SEC_REQUEST_MARKET:     Cmd_Get_Mrk(); break;
		case SEC_REQEUST_LAST_ERROR: Cmd_Get_Err(); break;
		case SEC_REQUEST_FINGERPRNT: Cmd_Get_Fpr(); break;
		case SEC_REQUEST_LAST_CMD:   Cmd_Get_Lst(); break;
		case SEC_REQUEST_COUNT_VAL:  Cmd_Get_Cnt(); break;
		case SEC_SET_NUM_COUNTERS:   Cmd_Set_Ncn(); break;
		case SEC_SET_MARKET:         Cmd_Set_Mrk(); break;
		case SEC_SET_COUNTER_TXT:    Cmd_Set_Txt(); break;
		case SEC_COUNT_INC_SMALL:    Cmd_Inc_Sml(); break;
		case SEC_COUNT_INC_MED:      Cmd_Inc_Med(); break;
		case SEC_COUNT_INC_LARGE:    Cmd_Inc_Lrg(); break;

		/* acknowledge these without doing anything */
		case SEC_SHOW_TEXT:          Cmd_NOP();     break;
		case SEC_SHOW_COUNTER_VAL:   Cmd_NOP();     break;
		case SEC_SHOW_COUNTER_TXT:   Cmd_NOP();     break;
		case SEC_SHOW_BITPATTERN:    Cmd_NOP();     break;
		case SEC_COUNT_CYCLE_DISP:   Cmd_NOP();     break;
		case SEC_STOP_CYCLE:         Cmd_NOP();     break;
		case SEC_SELF_TEST:          Cmd_NOP();     break;
	}
}

UINT8 SEC::CalcByteSum(int length)
{
	UINT8 csum = 0;
	for ( int i = 0; i < 3 + length; i++ )
	{
		csum += m_reply[i];
	}
	return csum;
}

void SEC::Cmd_Get_Err(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = 0; // Last Error

	m_reply[4] = CalcByteSum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void SEC::Cmd_Get_Fpr(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	// fingerprint
	m_reply[3] = 0x11;
	m_reply[4] = 0x01;
	m_reply[5] = 0x00;
	m_reply[6] = 0x00;

	m_reply[7] = CalcByteSum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void SEC::Cmd_Get_Lst(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	// Last Command (is this really 4 bytes?)
	m_reply[3] = m_last;
	m_reply[4] = 0x00;
	m_reply[5] = 0x00;
	m_reply[6] = 0x00;

	m_reply[7] = CalcByteSum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void SEC::Cmd_Get_Ver(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 3;

	// version
	m_reply[3] = '0';
	m_reply[4] = '2';
	m_reply[5] = 'E';

	m_reply[6] = CalcByteSum(3);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 7;
}

void SEC::Cmd_Get_Cnt(void)
{
	char temp[10];

	sprintf(temp,"%07d0", m_counters[m_request[3]]);

	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 4;

	m_reply[3] = ((temp[0] - 0x30) << 4) + (temp[1] - 0x30);
	m_reply[4] = ((temp[2] - 0x30) << 4) + (temp[3] - 0x30);
	m_reply[5] = ((temp[4] - 0x30) << 4) + (temp[5] - 0x30);
	m_reply[6] = ((temp[6] - 0x30) << 4) + (temp[7] - 0x30);

	m_reply[7] = CalcByteSum(4);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 8;
}

void SEC::Cmd_Get_Sta(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = 0x20; // Status

	m_reply[4] = CalcByteSum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void SEC::Cmd_Get_Mrk(void)
{
	m_reply[0] = SEC_DAT;
	m_reply[1] = m_last;
	m_reply[2] = 1;

	m_reply[3] = m_market;

	m_reply[4] = CalcByteSum(1);

	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 5;
}

void SEC::Cmd_Set_Ncn(void)
{
	m_nocnt = m_request[3];

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void SEC::Cmd_Set_Mrk(void)
{
	m_market = m_request[3];

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void SEC::Cmd_Set_Txt(void)
{
	char valbuf[8];
	UINT8 meter = m_request[3];
	memcpy( valbuf, &m_request[4], 7);
	valbuf[7] = 0;
	strcpy(m_strings[meter], valbuf);

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}


void SEC::Cmd_Inc_Sml(void)
{
	UINT8 meter = m_request[3];
	UINT8 value = m_request[4] & 0xf;
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void SEC::Cmd_Inc_Med(void)
{
	UINT8 meter = m_request[3];
	UINT8 value = m_request[4];
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void SEC::Cmd_Inc_Lrg(void)
{
	UINT8 meter = m_request[3];
	UINT8 value = m_request[4] + 256 * m_request[5];
	m_counters[meter] += value;

	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}

void SEC::Cmd_NOP(void)
{
	m_reply[0] = SEC_ACK;
	m_reply[1] = m_last;
	m_reply[2] = 0;
	m_reply[3] = CalcByteSum(0);
	m_rxpos = 0;
	m_rxclk = 0;
	m_rxlen = 4;
}
