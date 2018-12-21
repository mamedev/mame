// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

static char digit[] = "0123456789abcdef";

static void dprintdec(int64_t val, bool zeropad, int size)
{
	if (val == 0 && !zeropad)
	{
		printf("0");
		return;
	}

	char dc[22];
	for (int rem = 0; rem < 22; rem++) dc[rem] = '0';

	int ptr = 0;
	int64_t v = val;
	char c = ' ';
	if (val < 0)
	{
		c = '-';
		val = -val;
	}

	int indx1 = 0;
	while (v > 10)
	{
		int64_t rem = v / 10;
		int indx = v - (rem * 10);
		dc[ptr] = digit[indx];
		v = rem;
		if (indx != 0)
			indx1 = ptr;
		ptr += 1;
	}

	dc[ptr] = digit[v];

	if (v != 0)
		indx1 = ptr; // the leading non zero digit.

	if (zeropad && size != 0)
		ptr = size;
	else
		ptr = indx1; // don't print leading 0s

	if (c == '-')
		printf("-");

	while (ptr >= 0)
	{
		printf("%c", dc[ptr]);
		ptr -= 1;
	}
}

static void dprintudec(uint64_t val, bool zeropad, int size)
{
	if (val == 0 && !zeropad)
	{
		printf("0");
		return;
	}

	char dc[22];
	for (int rem = 0; rem < 22; dc[rem++] = '0');

	int ptr = 0;
	uint64_t v = val;
	int indx1 = 0;
	while (v > 10)
	{
		uint64_t rem = v / 10;
		int indx = v - (rem * 10);
		dc[ptr] = digit[indx];
		v = rem;
		if (indx != 0)
			indx1 = ptr;
		ptr += 1;
	}

	dc[ptr] = digit[v];

	if (v != 0)
		indx1 = ptr; // the leading non zero digit.

	if (zeropad && size != 0)
		ptr = size;
	else
		ptr = indx1; // don't print leading 0s

	while (ptr >= 0)
	{
		printf("%c", dc[ptr]);
		ptr -= 1;
	}
}

static void dprinthex(uint64_t val, bool zeropad, int pos)
{
	if (val == 0 && !zeropad)
	{
		printf("0");
		return;
	}

	int pcount = pos;
	int indx = 0;

	char c;
	while (pcount >= 0)
	{
		if (pcount == 0)
			c = digit[val & 0xf];
		else
			c = digit[(val >> pcount) & 0xf];

		if ((c == 'X') && (c > '9'))
			c = c - 'a' + 'A';

		if (c != '0')
		{
			indx += 1;
			printf("%c", c);
		}
		else
		{
			if (zeropad || indx != 0)
				printf("%c", c);
		}
		pcount -= 4;
	}
}

void dprintoct(uint64_t val, bool zeropad, int pos)
{
	if (val == 0 && !zeropad)
	{
		printf("0");
		return;
	}

	int pcount = pos;
	int indx = 0;

	char c;
	while (pcount >= 0)
	{
		if (pcount == 0)
			c = digit[val & 0x7];
		else
			c = digit[(val >> pcount) & 0x7];

		if (c != '0')
		{
			indx += 1;
			printf("%c", c);
		}
		else
		{
			if (zeropad || indx != 0)
				printf("%c", c);
		}
		pcount -= 3;
	}
}

#define Bhex    4
#define Shex    12
#define Ihex    28
#define Lhex    60
#define Boct    3
#define Soct    15
#define Ioct    30
#define Loct    63

static uint64_t dprintf_get_arg64(uint8_t *buf, uint32_t &curr)
{
	curr = (curr + 3) & ~3;
	const uint64_t ret = ((uint64_t)buf[curr+0] << 56) | ((uint64_t)buf[curr+1] << 48) | ((uint64_t)buf[curr+2] << 40) | ((uint64_t)buf[curr+3] << 32) |
						 ((uint64_t)buf[curr+4] << 24) | ((uint64_t)buf[curr+5] << 16) | ((uint64_t)buf[curr+6] << 8)  |            buf[curr+7];
	curr += 8;
	return ret;
}

static uint32_t dprintf_get_arg32(uint8_t *buf, uint32_t &curr)
{
	curr = (curr + 3) & ~3;
	const uint32_t ret = ((uint32_t)buf[curr+0] << 24) | ((uint32_t)buf[curr+1] << 16) | ((uint32_t)buf[curr+2] << 8) | buf[curr+3];
	curr += 4;
	return ret;
}

static uint16_t dprintf_get_arg16(uint8_t *buf, uint32_t &curr)
{
	curr = (curr + 1) & ~1;
	const uint16_t ret = ((uint16_t)buf[curr+0] << 8) | buf[curr+1];
	curr += 2;
	return ret;
}

static uint8_t dprintf_get_arg8(uint8_t *buf, uint32_t &curr)
{
	const uint8_t ret = buf[curr++];
	return ret;
}

void mips3_device::do_o2_dprintf(uint32_t fmt_addr, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t stack)
{
	char buf[4096];
	uint8_t argbuf[4096];
	int idx = 0;
	uint8_t byte_val = 0;
	fmt_addr &= 0x1fffffff;
	argbuf[0] = (uint8_t)(a1 >> 24);
	argbuf[1] = (uint8_t)(a1 >> 16);
	argbuf[2] = (uint8_t)(a1 >>  8);
	argbuf[3] = (uint8_t)a1;
	argbuf[4] = (uint8_t)(a2 >> 24);
	argbuf[5] = (uint8_t)(a2 >> 16);
	argbuf[6] = (uint8_t)(a2 >>  8);
	argbuf[7] = (uint8_t)a2;
	argbuf[8] = (uint8_t)(a3 >> 24);
	argbuf[9] = (uint8_t)(a3 >> 16);
	argbuf[10] = (uint8_t)(a3 >>  8);
	argbuf[11] = (uint8_t)a3;
	stack &= 0x1fffffff;
	for (int i = 0; i < 4096-12; i++)
	{
		argbuf[i+12] = m_program->read_byte(i+stack);
	}
	uint32_t argcurr = 0;
	do
	{
		byte_val = m_program->read_byte(fmt_addr++);
		buf[idx++] = (char)byte_val;
	} while(byte_val != 0);

	char *p = buf;
	char errQ[3];
	int state = 0;
	int size = 0;
	int errP = 0;
	bool zeropad = false;
	while (*p)
	{
		switch (state)
		{
		case 0:
			if (*p != '%')
			{
				printf("%c", *p);
			}
			else
			{
				errQ[errP++] = '%';
				state = 1;
			}
			p++;
			break;
		case 1: // check for zero padding
			state = 2;
			if (*p == '0')
			{
				errQ[errP++] = '0';
				zeropad = true;
				p++;
			}
			else
			{
				zeropad = false;
			}
			break;
		case 2: // check the size of the object to be printed
			state = 3;
			switch (*p)
			{
			case 'l':
				errQ[errP++] = 'l';
				size = 64;
				p++;
				break;
			case 'h':
				errQ[errP++] = 'h';
				size = 16;
				p++;
				break;
			case 'b':
				errQ[errP++] = 'b';
				size = 8;
				p++;
				break;
			default:
				size = 32;
				break;
			}
			break;
		case 3: // do the print
			switch (*p)
			{
			case '%':
				printf("%c", '%');
				break;
			case 'd':
			case 'i':
				switch (size)
				{
				case 64:
					dprintdec((int64_t)dprintf_get_arg64(argbuf, argcurr), zeropad, 0);
					break;
				case 16: // short
					dprintdec((int64_t)dprintf_get_arg16(argbuf, argcurr), zeropad, 4);
					break;
				case 8:  // byte
					dprintdec((int64_t)dprintf_get_arg8(argbuf, argcurr), zeropad, 3);
					break;
				default: // int
					dprintdec((int64_t)dprintf_get_arg32(argbuf, argcurr), zeropad, 9);
					break;
				}
				break;
			case 'u':
				switch (size)
				{
				case 64:
					dprintudec((uint64_t)dprintf_get_arg64(argbuf, argcurr), zeropad, 0);
					break;
				case 16:
					dprintudec((uint64_t)dprintf_get_arg16(argbuf, argcurr), zeropad, 4);
					break;
				case 8:
					dprintudec((uint64_t)dprintf_get_arg8(argbuf, argcurr), zeropad, 3);
					break;
				default:
					dprintudec((uint64_t)dprintf_get_arg32(argbuf, argcurr), zeropad, 9);
					break;
				}
				break;
			case 'o':
				switch (size)
				{
				case 64:
					dprintoct((uint64_t)dprintf_get_arg64(argbuf, argcurr), zeropad, Loct);
					break;
				case 16: // short
					dprintoct((uint64_t)dprintf_get_arg16(argbuf, argcurr), zeropad, Soct);
					break;
				case 8:  // byte
					dprintoct((uint64_t)dprintf_get_arg8(argbuf, argcurr), zeropad, Boct);
					break;
				default: // int
					dprintoct((uint64_t)dprintf_get_arg32(argbuf, argcurr), zeropad, Ioct);
					break;
				}
				break;
			case 'x':
			case 'X':
				switch (size)
				{
				case 64:
					dprinthex((uint64_t)dprintf_get_arg64(argbuf, argcurr), zeropad, Lhex);
					break;
				case 16:
					dprinthex((uint64_t)dprintf_get_arg16(argbuf, argcurr), zeropad, Shex);
					break;
				case 8:
					dprinthex((uint64_t)dprintf_get_arg8(argbuf, argcurr), zeropad, Bhex);
					break;
				default:
					dprinthex((uint64_t)dprintf_get_arg32(argbuf, argcurr), zeropad, Ihex);
					break;
				}
				break;
			case 'c':
				printf("%c", (char)dprintf_get_arg8(argbuf, argcurr));
				break;
			case 's':
			{
				uint64_t str_addr = dprintf_get_arg64(argbuf, argcurr);
				uint8_t strbyte = 0;
				do
				{
					strbyte = m_program->read_byte(str_addr);
					str_addr++;
					printf("%c", (char)strbyte);
				} while(strbyte);
				break;
			}
			case '0': // error
			case 'l': // error
			case 'h': // error
			default:  // error
				for (int i = 0; i < 3; i++)
					printf("%c", (char)errQ[i]);
				printf("%c", *p++);
				break;
			}
			state = 0;          // reset the state machine
			size = 0;           // reset the size
			zeropad = false;    // reset the zero padding
			errP = 0;           // reset errQ
			p++;
		}
	}
}
