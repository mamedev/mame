/***************************************************************************

  macrtc.c - the real-time clock & NVRAM chip used in Macs and the Apple IIgs.

  TODO: convert to a device, share between Mac and IIgs

***************************************************************************/

#include "emu.h"
#include "includes/mac.h"

#ifdef MAME_DEBUG
#define LOG_RTC			0
#else
#define LOG_RTC			0
#endif

enum
{
	RTC_STATE_NORMAL,
	RTC_STATE_WRITE,
	RTC_STATE_XPCOMMAND,
	RTC_STATE_XPWRITE
};

/* init the rtc core */
void mac_state::rtc_init()
{
	m_rtc_rTCClk = 0;

	m_rtc_write_protect = 0;
	m_rtc_rTCEnb = 0;
	rtc_write_rTCEnb(1);
	m_rtc_state = RTC_STATE_NORMAL;
}

/* write the rTCEnb state */
void mac_state::rtc_write_rTCEnb(int val)
{
	if (val && (! m_rtc_rTCEnb))
	{
		/* rTCEnb goes high (inactive) */
		m_rtc_rTCEnb = 1;
		/* abort current transmission */
		m_rtc_data_byte = m_rtc_bit_count = m_rtc_data_dir = m_rtc_data_out = 0;
		m_rtc_state = RTC_STATE_NORMAL;
	}
	else if ((! val) && m_rtc_rTCEnb)
	{
		/* rTCEnb goes low (active) */
		m_rtc_rTCEnb = 0;
		/* abort current transmission */
		m_rtc_data_byte = m_rtc_bit_count = m_rtc_data_dir = m_rtc_data_out = 0;
		m_rtc_state = RTC_STATE_NORMAL;
	}
}

/* shift data (called on rTCClk high-to-low transition (?)) */
void mac_state::rtc_shift_data(int data)
{
	if (m_rtc_rTCEnb)
		/* if enable line inactive (high), do nothing */
		return;

	if (m_rtc_data_dir)
	{	/* RTC -> VIA transmission */
		m_rtc_data_out = (m_rtc_data_byte >> --m_rtc_bit_count) & 0x01;
		if (LOG_RTC)
			logerror("RTC shifted new data %d\n", m_rtc_data_out);
	}
	else
	{	/* VIA -> RTC transmission */
		m_rtc_data_byte = (m_rtc_data_byte << 1) | (data ? 1 : 0);

		if (++m_rtc_bit_count == 8)
		{	/* if one byte received, send to command interpreter */
			rtc_execute_cmd(m_rtc_data_byte);
		}
	}
}

/* called every second, to increment the Clock count */
void mac_state::rtc_incticks()
{
	if (LOG_RTC)
		logerror("rtc_incticks called\n");

	if (++m_rtc_seconds[0] == 0)
		if (++m_rtc_seconds[1] == 0)
			if (++m_rtc_seconds[2] == 0)
				++m_rtc_seconds[3];

	/*if (++m_rtc_seconds[4] == 0)
        if (++m_rtc_seconds[5] == 0)
            if (++m_rtc_seconds[6] == 0)
                ++m_rtc_seconds[7];*/
}

/* Executes a command.
Called when the first byte after "enable" is received, and when the data byte after a write command
is received. */
void mac_state::rtc_execute_cmd(int data)
{
	int i;

	if (LOG_RTC)
		printf("rtc_execute_cmd: data=%x, state=%x\n", data, m_rtc_state);

	if (m_rtc_state == RTC_STATE_XPCOMMAND)
	{
		m_rtc_xpaddr = ((m_rtc_cmd & 7)<<5) | ((data&0x7c)>>2);
		if ((m_rtc_cmd & 0x80) != 0)
		{
			// read command
			if (LOG_RTC)
				printf("RTC: Reading extended address %x = %x\n", m_rtc_xpaddr, m_rtc_ram[m_rtc_xpaddr]);

			m_rtc_data_dir = 1;
			m_rtc_data_byte = m_rtc_ram[m_rtc_xpaddr];
			m_rtc_state = RTC_STATE_NORMAL;
		}
		else
		{
			// write command
			m_rtc_state = RTC_STATE_XPWRITE;
			m_rtc_data_byte = 0;
			m_rtc_bit_count = 0;
		}
	}
	else if (m_rtc_state == RTC_STATE_XPWRITE)
	{
		if (LOG_RTC)
			printf("RTC: writing %x to extended address %x\n", data, m_rtc_xpaddr);
		m_rtc_ram[m_rtc_xpaddr] = data;
		m_rtc_state = RTC_STATE_NORMAL;
	}
	else if (m_rtc_state == RTC_STATE_WRITE)
	{
		m_rtc_state = RTC_STATE_NORMAL;

		/* Writing an RTC register */
		i = (m_rtc_cmd >> 2) & 0x1f;
		if (m_rtc_write_protect && (i != 13))
			/* write-protection : only write-protect can be written again */
			return;
		switch(i)
		{
		case 0: case 1: case 2: case 3:	/* seconds register */
		case 4: case 5: case 6: case 7:	/* ??? (not described in IM III) */
			/* after various tries, I assumed m_rtc_seconds[4+i] is mapped to m_rtc_seconds[i] */
			if (LOG_RTC)
				logerror("RTC clock write, address = %X, data = %X\n", i, (int) m_rtc_data_byte);
			m_rtc_seconds[i & 3] = m_rtc_data_byte;
			break;

		case 8: case 9: case 10: case 11:	/* RAM address $10-$13 */
			if (LOG_RTC)
				printf("RTC RAM write, address = %X, data = %X\n", (i & 3) + 0x10, (int) m_rtc_data_byte);
			m_rtc_ram[(i & 3) + 0x10] = m_rtc_data_byte;
			break;

		case 12:
			/* Test register - do nothing */
			if (LOG_RTC)
				logerror("RTC write to test register, data = %X\n", (int) m_rtc_data_byte);
			break;

		case 13:
			/* Write-protect register  */
			if (LOG_RTC)
				printf("RTC write to write-protect register, data = %X\n", (int) m_rtc_data_byte&0x80);
			m_rtc_write_protect = (m_rtc_data_byte & 0x80) ? TRUE : FALSE;
			break;

		case 16: case 17: case 18: case 19:	/* RAM address $00-$0f */
		case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27:
		case 28: case 29: case 30: case 31:
			if (LOG_RTC)
				printf("RTC RAM write, address = %X, data = %X\n", i & 15, (int) m_rtc_data_byte);
			m_rtc_ram[i & 15] = m_rtc_data_byte;
			break;

		default:
			printf("Unknown RTC write command : %X, data = %d\n", (int) m_rtc_cmd, (int) m_rtc_data_byte);
			break;
		}
	}
	else
	{
		// always save this byte to m_rtc_cmd
		m_rtc_cmd = m_rtc_data_byte;

		if ((m_rtc_cmd & 0x78) == 0x38)	// extended command
		{
			m_rtc_state = RTC_STATE_XPCOMMAND;
			m_rtc_data_byte = 0;
			m_rtc_bit_count = 0;
		}
		else
		{
			if (m_rtc_cmd & 0x80)
			{
				m_rtc_state = RTC_STATE_NORMAL;

				/* Reading an RTC register */
				m_rtc_data_dir = 1;
				i = (m_rtc_cmd >> 2) & 0x1f;
				switch(i)
				{
					case 0: case 1: case 2: case 3:
					case 4: case 5: case 6: case 7:
						m_rtc_data_byte = m_rtc_seconds[i & 3];
						if (LOG_RTC)
							printf("RTC clock read, address = %X -> data = %X\n", i, m_rtc_data_byte);
						break;

					case 8: case 9: case 10: case 11:
						if (LOG_RTC)
							printf("RTC RAM read, address = %X data = %x\n", (i & 3) + 0x10, m_rtc_ram[(i & 3) + 0x10]);
						m_rtc_data_byte = m_rtc_ram[(i & 3) + 0x10];
						break;

					case 16: case 17: case 18: case 19:
					case 20: case 21: case 22: case 23:
					case 24: case 25: case 26: case 27:
					case 28: case 29: case 30: case 31:
						if (LOG_RTC)
							printf("RTC RAM read, address = %X data = %x\n", i & 15, m_rtc_ram[i & 15]);
						m_rtc_data_byte = m_rtc_ram[i & 15];
						break;

					default:
						if (LOG_RTC)
							logerror("Unknown RTC read command : %X\n", (int) m_rtc_cmd);
						m_rtc_data_byte = 0;
						break;
				}
			}
			else
			{
				/* Writing an RTC register */
				/* wait for extra data byte */
				if (LOG_RTC)
					logerror("RTC write, waiting for data byte : %X\n", (int) m_rtc_cmd);
				m_rtc_state = RTC_STATE_WRITE;
				m_rtc_data_byte = 0;
				m_rtc_bit_count = 0;
			}
		}
	}
}

/* should save PRAM to file */
/* TODO : save write_protect flag, save time difference with host clock */
NVRAM_HANDLER( mac )
{
	mac_state *mac = machine.driver_data<mac_state>();

	if (read_or_write)
	{
		if (LOG_RTC)
			logerror("Writing PRAM to file\n");
		file->write(mac->m_rtc_ram, sizeof(mac->m_rtc_ram));
	}
	else
	{
		if (file)
		{
			if (LOG_RTC)
				logerror("Reading PRAM from file\n");
			file->read(mac->m_rtc_ram, sizeof(mac->m_rtc_ram));
		}
		else
		{
			UINT8 *pram = &mac->m_rtc_ram[0];

			if (LOG_RTC)
				logerror("trashing PRAM\n");

			memset(mac->m_rtc_ram, 0, sizeof(mac->m_rtc_ram));

			// some Mac ROMs are buggy in the presence of
			// no NVRAM, so let's initialize it right
			pram[0] = 0xa8;	// valid
			pram[4] = 0xcc;
			pram[5] = 0x0a;
			pram[6] = 0xcc;
			pram[7] = 0x0a;
			pram[0xc] = 0x42;	// XPRAM valid for Plus/SE
			pram[0xd] = 0x75;
			pram[0xe] = 0x67;
			pram[0xf] = 0x73;
			pram[0x10] = 0x18;
			pram[0x11] = 0x88;
			pram[0x12] = 0x01;
			pram[0x13] = 0x4c;

			if (mac->m_model >= MODEL_MAC_II)
			{
				pram[0xc] = 0x4e;	// XPRAM valid is different for these
				pram[0xd] = 0x75;
				pram[0xe] = 0x4d;
				pram[0xf] = 0x63;
				pram[0x77] = 0x01;
				pram[0x78] = 0xff;
				pram[0x79] = 0xff;
				pram[0x7a] = 0xff;
				pram[0x7b] = 0xdf;

				if (mac->m_model == MODEL_MAC_IICI)
				{
					pram[0] = 0xa8;
					pram[1] = 0x80;
					pram[2] = 0x4f;
					pram[3] = 0x48;
					pram[4] = 0xcc;
					pram[5] = 0x0a;
					pram[6] = 0xcc;
					pram[7] = 0x0a;
					pram[0x10] = 0x18;
					pram[0x11] = 0x88;
					pram[0x12] = 0x03;
					pram[0x13] = 0xec;
					pram[0x57] = 0x1f;
					pram[0x58] = 0x83;
					pram[0x59] = 0x86;
					pram[0x81] = 0x86;
					pram[0x8a] = 0x05;
					pram[0xb9] = 0xb0;
					pram[0xf1] = 0x01;
					pram[0xf3] = 0x02;
					pram[0xf9] = 0x01;
					pram[0xfb] = 0x8e;
				}
			}
		}

		{
			/* Now we copy the host clock into the Mac clock */
			/* Cool, isn't it ? :-) */
			system_time systime;
			struct tm mac_reference;
			UINT32 seconds;

			machine.base_datetime(systime);

			/* The count starts on 1st January 1904 */
			mac_reference.tm_sec = 0;
			mac_reference.tm_min = 0;
			mac_reference.tm_hour = 0;
			mac_reference.tm_mday = 1;
			mac_reference.tm_mon = 0;
			mac_reference.tm_year = 4;
			mac_reference.tm_isdst = 0;

			seconds = difftime((time_t) systime.time, mktime(& mac_reference));

			if (LOG_RTC)
				logerror("second count 0x%lX\n", (unsigned long) seconds);

			mac->m_rtc_seconds[0] = seconds & 0xff;
			mac->m_rtc_seconds[1] = (seconds >> 8) & 0xff;
			mac->m_rtc_seconds[2] = (seconds >> 16) & 0xff;
			mac->m_rtc_seconds[3] = (seconds >> 24) & 0xff;
		}
	}
}



