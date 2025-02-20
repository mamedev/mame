// license:GPL-2.0+
// copyright-holders:Daniel Coulom,Sandro Ronco
/*********************************************************************

    ef9345.cpp

    Thomson EF9345 video controller emulator code

    This code is based on Daniel Coulom's implementation in DCVG5k
    and DCAlice released by Daniel Coulom under GPL license

    TS9347 variant support added by Jean-François DEL NERO
*********************************************************************/

#include "emu.h"
#include "ef9345.h"

#include "screen.h"


#define MODE24x40   0
#define MODEVAR40   1
#define MODE8x80    2
#define MODE12x80   3
#define MODE16x40   4

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(EF9345, ef9345_device, "ef9345", "EF9345")
DEFINE_DEVICE_TYPE(TS9347, ts9347_device, "ts9347", "TS9347")

// default address map
void ef9345_device::ef9345(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0x3fff).ram();
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ef9345_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

// calculate the internal RAM offset
inline uint16_t ef9345_device::indexram(uint8_t r)
{
	uint8_t x = m_registers[r];
	uint8_t y = m_registers[r - 1];
	if (y < 8)
		y &= 1;
	return ((x&0x3f) | ((x & 0x40) << 6) | ((x & 0x80) << 4) | ((y & 0x1f) << 6) | ((y & 0x20) << 8));
}

// calculate the internal ROM offset
inline uint16_t ef9345_device::indexrom(uint8_t r)
{
	uint8_t x = m_registers[r];
	uint8_t y = m_registers[r - 1];
	if (y < 8)
		y &= 1;
	return((x&0x3f)|((x&0x40)<<6)|((x&0x80)<<4)|((y&0x1f)<<6));
}

// increment x
inline void ef9345_device::inc_x(uint8_t r)
{
	uint8_t i = (m_registers[r] & 0x3f) + 1;
	if (i > 39)
	{
		i -= 40;
		m_state |= 0x40;
	}
	m_registers[r] = (m_registers[r] & 0xc0) | i;
}

// increment y
inline void ef9345_device::inc_y(uint8_t r)
{
	uint8_t i = (m_registers[r] & 0x1f) + 1;
	if (i > 31)
		i -= 24;
	m_registers[r] = (m_registers[r] & 0xe0) | i;
}


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  ef9345_device - constructor
//-------------------------------------------------

ef9345_device::ef9345_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ef9345_device(mconfig, EF9345, tag, owner, clock, EF9345_MODE::TYPE_EF9345)
{
}

ef9345_device::ef9345_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, EF9345_MODE variant) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(ef9345_device::ef9345), this)),
	m_charset(*this, DEVICE_SELF),
	m_variant(variant),
	m_palette(*this, finder_base::DUMMY_TAG)
{
}

ts9347_device::ts9347_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ef9345_device(mconfig, TS9347, tag, owner, clock, EF9345_MODE::TYPE_TS9347)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ef9345_device::device_start()
{
	m_busy_timer = timer_alloc(FUNC(ef9345_device::clear_busy_flag), this);
	m_blink_timer = timer_alloc(FUNC(ef9345_device::blink_tick), this);

	m_videoram = &space(0);

	m_screen_out.allocate(496, screen().height());

	m_blink_timer->adjust(attotime::from_msec(500), 0, attotime::from_msec(500));

	init_accented_chars();

	save_item(NAME(m_border));
	save_item(NAME(m_registers));
	save_item(NAME(m_last_dial));
	save_item(NAME(m_ram_base));
	save_item(NAME(m_bf));
	save_item(NAME(m_char_mode));
	save_item(NAME(m_state));
	save_item(NAME(m_tgs));
	save_item(NAME(m_mat));
	save_item(NAME(m_pat));
	save_item(NAME(m_dor));
	save_item(NAME(m_ror));
	save_item(NAME(m_block));
	save_item(NAME(m_blink));
	save_item(NAME(m_latchc0));
	save_item(NAME(m_latchm));
	save_item(NAME(m_latchi));
	save_item(NAME(m_latchu));

	save_item(NAME(m_screen_out));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void ef9345_device::device_reset()
{
	m_tgs = m_mat = m_pat = m_dor = m_ror = 0;
	m_state = 0;
	m_bf = 0;
	m_block = 0;
	m_blink = 0;
	m_latchc0 = 0;
	m_latchm = 0;
	m_latchi = 0;
	m_latchu = 0;
	m_char_mode = MODE24x40;

	memset(m_last_dial, 0, sizeof(m_last_dial));
	memset(m_registers, 0, sizeof(m_registers));
	memset(m_border, 0, sizeof(m_border));
	memset(m_border, 0, sizeof(m_ram_base));

	m_screen_out.fill(0);

	set_video_mode();
}

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ef9345_device::clear_busy_flag)
{
	m_bf = 0;
}

TIMER_CALLBACK_MEMBER(ef9345_device::blink_tick)
{
	m_blink = !m_blink;
}


// set busy flag and timer to clear it
void ef9345_device::set_busy_flag(int period)
{
	m_bf = 1;
	m_busy_timer->adjust(attotime::from_nsec(period));
}

// draw a char in 40 char line mode
void ef9345_device::draw_char_40(uint8_t *c, uint16_t x, uint16_t y)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const int scan_xsize = std::min( screen().width() - (x * 8), 8);
	const int scan_ysize = std::min( screen().height() - (y * 10), 10);

	for(int i = 0; i < scan_ysize; i++)
		for(int j = 0; j < scan_xsize; j++)
			m_screen_out.pix(y * 10 + i, x * 8 + j) = palette[c[8 * i + j] & 0x07];
}

// draw a char in 80 char line mode
void ef9345_device::draw_char_80(uint8_t *c, uint16_t x, uint16_t y)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const int scan_xsize = std::min( screen().width() - (x * 6), 6);
	const int scan_ysize = std::min( screen().height() - (y * 10), 10);

	for(int i = 0; i < scan_ysize; i++)
		for(int j = 0; j < scan_xsize; j++)
			m_screen_out.pix(y * 10 + i, x * 6 + j) = palette[c[6 * i + j] & 0x07];
}


// set then ef9345 mode
void ef9345_device::set_video_mode(void)
{
	if (m_variant == EF9345_MODE::TYPE_TS9347)
	{
		// Only TGS 7 & 6 used for the char mode with the TS9347
		m_char_mode = ((m_tgs & 0xc0) >> 6);
	}
	else
	{
		// PAT 7, TGS 7 & 6
		m_char_mode = ((m_pat & 0x80) >> 5) | ((m_tgs & 0xc0) >> 6);
	}

	uint16_t new_width = (m_char_mode == MODE12x80 || m_char_mode == MODE8x80) ? 492 : 336;

	if (screen().width() != new_width)
	{
		rectangle visarea = screen().visible_area();
		visarea.max_x = new_width - 1;

		screen().configure(new_width, screen().height(), visarea, screen().frame_period().attoseconds());
	}

	//border color
	memset(m_border, m_mat & 0x07, sizeof(m_border));

	//set the base for the m_videoram charset
	m_ram_base[0] = ((m_dor & 0x07) << 11);
	m_ram_base[1] = m_ram_base[0];
	m_ram_base[2] = ((m_dor & 0x30) << 8);
	m_ram_base[3] = m_ram_base[2] + 0x0800;

	//address of the current memory block
	m_block = 0x0800 * ((((m_ror & 0xf0) >> 4) | ((m_ror & 0x40) >> 5) | ((m_ror & 0x20) >> 3)) & 0x0c);
}

// initialize the ef9345 accented chars
void ef9345_device::init_accented_chars(void)
{
	uint16_t i, j;
	for(j = 0; j < 0x10; j++)
		for(i = 0; i < 0x200; i++)
			m_acc_char[(j << 9) + i] = m_charset[0x0600 + i];

	for(j = 0; j < 0x200; j += 0x40)
		for(i = 0; i < 4; i++)
		{
			m_acc_char[0x0200 + j + i +  4] |= 0x1c; //tilde
			m_acc_char[0x0400 + j + i +  4] |= 0x10; //acute
			m_acc_char[0x0400 + j + i +  8] |= 0x08; //acute
			m_acc_char[0x0600 + j + i +  4] |= 0x04; //grave
			m_acc_char[0x0600 + j + i +  8] |= 0x08; //grave

			m_acc_char[0x0a00 + j + i +  4] |= 0x1c; //tilde
			m_acc_char[0x0c00 + j + i +  4] |= 0x10; //acute
			m_acc_char[0x0c00 + j + i +  8] |= 0x08; //acute
			m_acc_char[0x0e00 + j + i +  4] |= 0x04; //grave
			m_acc_char[0x0e00 + j + i +  8] |= 0x08; //grave

			m_acc_char[0x1200 + j + i +  4] |= 0x08; //point
			m_acc_char[0x1400 + j + i +  4] |= 0x14; //trema
			m_acc_char[0x1600 + j + i + 32] |= 0x08; //cedilla
			m_acc_char[0x1600 + j + i + 36] |= 0x04; //cedilla

			m_acc_char[0x1a00 + j + i +  4] |= 0x08; //point
			m_acc_char[0x1c00 + j + i +  4] |= 0x14; //trema
			m_acc_char[0x1e00 + j + i + 32] |= 0x08; //cedilla
			m_acc_char[0x1e00 + j + i + 36] |= 0x04; //cedilla
		}
}

// read a char in charset or in m_videoram
uint8_t ef9345_device::read_char(uint8_t index, uint16_t addr)
{
	if (index < 0x04)
		return m_charset[0x0800*index + addr];
	else if (index < 0x08)
		return m_acc_char[0x0800*(index&3) + addr];
	else if (index < 0x0c)
		return m_videoram->read_byte(m_ram_base[index-8] + addr);
	else
		return m_videoram->read_byte(addr);
}

// calculate the dial position of the char
uint8_t ef9345_device::get_dial(uint8_t x, uint8_t attrib)
{
	if (x > 0 && m_last_dial[x-1] == 1)         //top right
		m_last_dial[x] = 2;
	else if (x > 0 && m_last_dial[x-1] == 5)    //half right
		m_last_dial[x] = 10;
	else if (m_last_dial[x] == 1)               //bottom left
		m_last_dial[x] = 4;
	else if (m_last_dial[x] == 2)               //bottom right
		m_last_dial[x] = 8;
	else if (m_last_dial[x] == 3)               //lower half
		m_last_dial[x] = 12;
	else if (attrib == 1)                       //Left half
		m_last_dial[x] = 5;
	else if (attrib == 2)                       //half high
		m_last_dial[x] = 3;
	else if (attrib == 3)                       //top left
		m_last_dial[x] = 1;
	else                                        //none
		m_last_dial[x] = 0;

	return m_last_dial[x];
}

// zoom the char
void ef9345_device::zoom(uint8_t *pix, uint16_t n)
{
	uint8_t i, j;
	if ((n & 0x0a) == 0)
		for(i = 0; i < 80; i += 8) // 1, 4, 5
			for(j = 7; j > 0; j--)
				pix[i + j] = pix[i + j / 2];

	if ((n & 0x05) == 0)
		for(i = 0; i < 80; i += 8) // 2, 8, 10
			for(j =0 ; j < 7; j++)
				pix[i + j] = pix[i + 4 + j / 2];

	if ((n & 0x0c) == 0)
		for(i = 0; i < 8; i++) // 1, 2, 3
			for(j = 9; j > 0; j--)
				pix[i + 8 * j] = pix[i + 8 * (j / 2)];

	if ((n & 0x03) == 0)
		for(i = 0; i < 8; i++) // 4, 8, 12
			for(j = 0; j < 9; j++)
				pix[i + 8 * j] = pix[i + 40 + 8 * (j / 2)];
}


// calculate the address of the char x,y
uint16_t ef9345_device::indexblock(uint16_t x, uint16_t y)
{
	uint16_t i = x, j;

	if (m_variant == EF9345_MODE::TYPE_EF9345)
	{
		// On the EF9345 the service row is always displayed at the top, and
		// it can be fetched from either Y=0 or Y=1.
		j = (y == 0) ? ((m_tgs & 0x20) >> 5) : ((m_ror & 0x1f) + y - 1);
	}
	else
	{
		// On the TS9347 the service row is displayed either at the top or at
		// the bottom, and it is always fetched from Y=0.
		if (m_tgs & 1)
			j = (y == 24) ? 0 : ((m_ror & 0x1f) + y);
		else
			j = (y == 0) ? 0 : ((m_ror & 0x1f) + y - 1);
	}

	j = (j > 31) ? (j - 24) : j;

	//right side of a double width character
	if ((m_tgs & 0x80) == 0 && x > 0)
	{
		if (m_last_dial[x - 1] == 1) i--;
		if (m_last_dial[x - 1] == 4) i--;
		if (m_last_dial[x - 1] == 5) i--;
	}

	return 0x40 * j + i;
}

// draw bichrome character (40 columns)
void ef9345_device::bichrome40(uint8_t type, uint16_t address, uint8_t dial, uint16_t iblock, uint16_t x, uint16_t y, uint8_t c0, uint8_t c1, uint8_t insert, uint8_t flash, uint8_t conceal, uint8_t negative, uint8_t underline)
{
	uint16_t i;
	uint8_t pix[80];

	if (m_variant == EF9345_MODE::TYPE_TS9347)
	{
		c0 = 0;
	}

	if (flash && m_pat & 0x40 && m_blink)
		c1 = c0;                    //flash
	if (conceal && m_pat & 0x08)
		c1 = c0;                    //conceal
	if (negative)                   //negative
	{
		i = c1;
		c1 = c0;
		c0 = i;
	}

	if ((m_pat & 0x30) == 0x30)
		insert = 0;                 //active area mark
	if (insert == 0)
		c1 += 8;                    //foreground color
	if ((m_pat & 0x30) == 0x00)
		insert = 1;                 //insert mode
	if (insert == 0)
		c0 += 8;                    //background color

	//draw the cursor
	i = (m_registers[6] & 0x1f);
	if (i < 8)
		i &= 1;

	if (iblock == 0x40 * i + (m_registers[7] & 0x3f))   //cursor position
	{
		switch(m_mat & 0x70)
		{
		case 0x40:                  //00 = fixed complemented
			c0 = (23 - c0) & 15;
			c1 = (23 - c1) & 15;
			break;
		case 0x50:                  //01 = fixed underlined
			underline = 1;
			break;
		case 0x60:                  //10 = flash complemented
			if (m_blink)
			{
				c0 = (23 - c0) & 15;
				c1 = (23 - c1) & 15;
			}
			break;
		case 0x70:                  //11 = flash underlined
			if (m_blink)
				underline = 1;
			break;
		}
	}

	// generate the pixel table
	for(i = 0; i < 40; i+=4)
	{
		uint8_t ch = read_char(type, address + i);

		for (uint8_t b=0; b<8; b++)
			pix[i*2 + b] = (ch & (1<<b)) ? c1 : c0;
	}

	//draw the underline
	if (underline)
		memset(&pix[72], c1, 8);

	if (dial > 0)
		zoom(pix, dial);

	//doubles the height of the char
	if (m_mat & 0x80)
		zoom(pix, (y & 0x01) ? 0x0c : 0x03);

	draw_char_40(pix, x + 1 , y + 1);
}

// draw quadrichrome character (40 columns)
void ef9345_device::quadrichrome40(uint8_t c, uint8_t b, uint8_t a, uint16_t x, uint16_t y)
{
	//C0-6= character code
	//B0= insert             not yet implemented !!!
	//B1= low resolution
	//B2= subset index (low resolution only)
	//B3-5 = set number
	//A0-6 = 4 color palette

	uint8_t i, j, n, col[8], pix[80];
	uint8_t lowresolution = (b & 0x02) >> 1, ramx, ramy, ramblock;
	uint16_t ramindex;

	if (m_variant == EF9345_MODE::TYPE_TS9347)
	{
		// No quadrichrome support into the TS9347
		return;
	}

	//quadrichrome don't suppor double size
	m_last_dial[x] = 0;

	//initialize the color table
	for(j = 1, n = 0, i = 0; i < 8; i++)
	{
		col[i] = 7;

		if (a & j)
			col[n++] = i;

		j <<= 1;
	}

	//find block number in ram
	ramblock = 0;
	if (b & 0x20)   ramblock |= 4;      //B5
	if (b & 0x08)   ramblock |= 2;      //B3
	if (b & 0x10)   ramblock |= 1;      //B4

	//find character address in ram
	ramx = c & 0x03;
	ramy =(c & 0x7f) >> 2;
	ramindex = 0x0800 * ramblock + 0x40 * ramy + ramx;
	if (lowresolution) ramindex += 5 * (b & 0x04);

	//fill pixel table
	for(i = 0, j = 0; i < 10; i++)
	{
		uint8_t ch = read_char(0x0c, ramindex + 4 * (i >> lowresolution));
		pix[j] = pix[j + 1] = col[(ch & 0x03) >> 0]; j += 2;
		pix[j] = pix[j + 1] = col[(ch & 0x0c) >> 2]; j += 2;
		pix[j] = pix[j + 1] = col[(ch & 0x30) >> 4]; j += 2;
		pix[j] = pix[j + 1] = col[(ch & 0xc0) >> 6]; j += 2;
	}

	draw_char_40(pix, x + 1, y + 1);
}

// draw bichrome character (80 columns)
void ef9345_device::bichrome80(uint8_t c, uint8_t a, uint16_t x, uint16_t y, uint8_t cursor)
{
	uint8_t c0, c1, pix[60];
	uint16_t i, j, d;

	c1 = (a & 1) ? (m_dor >> 4) & 7 : m_dor & 7;    //foreground color = DOR
	c0 =  m_mat & 7;                                //background color = MAT

	switch(c & 0x80)
	{
	case 0: //alphanumeric G0 set
		//A0: D = color set
		//A1: U = underline
		//A2: F = flash
		//A3: N = negative
		//C0-6: character code

		if ((a & 4) && (m_pat & 0x40) && (m_blink))
			c1 = c0;    //flash
		if (a & 8)      //negative
		{
			i = c1;
			c1 = c0;
			c0 = i;
		}

		if ((cursor == 0x40) || ((cursor == 0x60) && m_blink))
		{
			i = c1;
			c1 = c0;
			c0 = i;
		}

		d = ((c & 0x7f) >> 2) * 0x40 + (c & 0x03);  //char position

		for(i=0, j=0; i < 10; i++)
		{
			uint8_t ch = read_char(0, d + 4 * i);
			for (uint8_t b=0; b<6; b++)
				pix[j++] = (ch & (1<<b)) ? c1 : c0;
		}

		//draw the underline
		if ((a & 2) || (cursor == 0x50) || ((cursor == 0x70) && m_blink))
			memset(&pix[54], c1, 6);

		break;
	default: //dedicated mosaic set
		//A0: D = color set
		//A1-3: 3 blocks de 6 pixels
		//C0-6: 7 blocks de 6 pixels
		pix[ 0] = (c & 0x01) ? c1 : c0;
		pix[ 3] = (c & 0x02) ? c1 : c0;
		pix[12] = (c & 0x04) ? c1 : c0;
		pix[15] = (c & 0x08) ? c1 : c0;
		pix[24] = (c & 0x10) ? c1 : c0;
		pix[27] = (c & 0x20) ? c1 : c0;
		pix[36] = (c & 0x40) ? c1 : c0;
		pix[39] = (a & 0x02) ? c1 : c0;
		pix[48] = (a & 0x04) ? c1 : c0;
		pix[51] = (a & 0x08) ? c1 : c0;

		for(i = 0; i < 60; i += 12)
		{
			pix[i + 6] = pix[i];
			pix[i + 9] = pix[i + 3];
		}

		for(i = 0; i < 60; i += 3)
			pix[i + 2] = pix[i + 1] = pix[i];

		break;
	}

	draw_char_80(pix, x, y);
}

// generate 16 bits 40 columns char
void ef9345_device::makechar_16x40(uint16_t x, uint16_t y)
{
	uint8_t a, b, c0, c1, i, f, m, n, u, type, dial;
	uint16_t address, iblock;

	iblock = (m_mat & 0x80 && y > 1) ? indexblock(x, y / 2) : indexblock(x, y);
	a = m_videoram->read_byte(m_block + iblock);
	b = m_videoram->read_byte(m_block + iblock + 0x0800);

	dial = get_dial(x, (a & 0x80) ? 0 : (((a & 0x20) >> 5) | ((a & 0x10) >> 3)));

	//type and address of the char
	type = ((b & 0x80) >> 4) | ((a & 0x80) >> 6);
	address = ((b & 0x7f) >> 2) * 0x40 + (b & 0x03);

	//reset attributes latch
	if (x == 0)
	{
		m_latchm = m_latchi = m_latchu = m_latchc0 = 0;
	}

	//delimiter
	if ((b & 0xe0) == 0x80)
	{
		type = 0;
		address = ((127) >> 2) * 0x40 + (127 & 0x03); // Force character 127 (negative space) of first type.

		m_latchm = b & 1;
		m_latchi = (b & 2) >> 1;
		m_latchu = (b & 4) >> 2;
	}

	if (a & 0x80)
	{
		m_latchc0 = (a & 0x70) >> 4;
	}

	//char attributes
	c0 = m_latchc0;                         //background
	c1 = a & 0x07;                          //foreground
	i = m_latchi;                           //insert mode
	f  = (a & 0x08) >> 3;                   //flash
	m = m_latchm;                           //conceal
	n  = (a & 0x80) ? 0: ((a & 0x40) >> 6); //negative
	u = m_latchu;                           //underline

	bichrome40(type, address, dial, iblock, x, y, c0, c1, i, f, m, n, u);
}

// generate 24 bits 40 columns char
void ef9345_device::makechar_24x40(uint16_t x, uint16_t y)
{
	uint8_t a, b, c, c0, c1, i, f, m, n, u, type, dial;
	uint16_t address, iblock;

	iblock = (m_mat & 0x80 && y > 1) ? indexblock(x, y / 2) : indexblock(x, y);
	c = m_videoram->read_byte(m_block + iblock);
	b = m_videoram->read_byte(m_block + iblock + 0x0800);
	a = m_videoram->read_byte(m_block + iblock + 0x1000);

	if ((b & 0xc0) == 0xc0)
	{
		quadrichrome40(c, b, a, x, y);
		return;
	}

	dial = get_dial(x, (b & 0x02) + ((b & 0x08) >> 3));

	//type and address of the char
	address = ((c & 0x7f) >> 2) * 0x40 + (c & 0x03);
	type = (b & 0xf0) >> 4;

	//char attributes
	c0 = a & 0x07;                  //background
	c1 = (a & 0x70) >> 4;           //foreground
	i = b & 0x01;                   //insert
	f = (a & 0x08) >> 3;            //flash
	m = (b & 0x04) >> 2;            //conceal
	n = ((a & 0x80) >> 7);          //negative
	u = (((b & 0x60) == 0) || ((b & 0xc0) == 0x40)) ? ((b & 0x10) >> 4) : 0; //underline

	bichrome40(type, address, dial, iblock, x, y, c0, c1, i, f, m, n, u);
}

// generate 12 bits 80 columns char
void ef9345_device::makechar_12x80(uint16_t x, uint16_t y)
{
	uint16_t iblock = indexblock(x, y);
	//draw the cursor
	uint8_t cursor = 0;
	uint8_t b = BIT(m_registers[7], 7);

	uint8_t i = (m_registers[6] & 0x1f);
	if (i < 8)
		i &= 1;

	if (iblock == 0x40 * i + (m_registers[7] & 0x3f))   //cursor position
		cursor = m_mat & 0x70;

	bichrome80(m_videoram->read_byte(m_block + iblock), (m_videoram->read_byte(m_block + iblock + 0x1000) >> 4) & 0x0f, 2 * x + 1, y + 1, b ? 0 : cursor);
	bichrome80(m_videoram->read_byte(m_block + iblock + 0x0800), m_videoram->read_byte(m_block + iblock + 0x1000) & 0x0f, 2 * x + 2, y + 1, b ? cursor : 0);
}

void ef9345_device::draw_border(uint16_t line)
{
	if (m_char_mode == MODE12x80 || m_char_mode == MODE8x80)
		for(int i = 0; i < 82; i++)
			draw_char_80(m_border, i, line);
	else
		for(int i = 0; i < 42; i++)
			draw_char_40(m_border, i, line);
}

void ef9345_device::makechar(uint16_t x, uint16_t y)
{
	switch (m_char_mode)
	{
		case MODE24x40:
			makechar_24x40(x, y);
			break;
		case MODEVAR40:
			if (m_variant == EF9345_MODE::TYPE_TS9347)
			{ // TS9347 char mode definition is different.
				makechar_16x40(x, y);
				break;
			}
			[[fallthrough]];
		case MODE8x80:
			logerror("Unemulated EF9345 mode: %02x\n", m_char_mode);
			break;
		case MODE12x80:
			makechar_12x80(x, y);
			break;
		case MODE16x40:
			if (m_variant == EF9345_MODE::TYPE_TS9347)
			{
				logerror("Unemulated EF9345 mode: %02x\n", m_char_mode);
			}
			else
			{
				makechar_16x40(x, y);
			}
			break;
		default:
			logerror("Unknown EF9345 mode: %02x\n", m_char_mode);
			break;
	}
}

// Execute EF9345 command
void ef9345_device::ef9345_exec(uint8_t cmd)
{
	m_state = 0;
	if ((m_registers[5] & 0x3f) == 39) m_state |= 0x10; //S4(LXa) set
	if ((m_registers[7] & 0x3f) == 39) m_state |= 0x20; //S5(LXm) set

	uint16_t a = indexram(7);

	switch(cmd)
	{
		case 0x00:  //KRF: R1,R2,R3->ram
		case 0x01:  //KRF: R1,R2,R3->ram + increment
			set_busy_flag(4000);
			m_videoram->write_byte(a, m_registers[1]);
			m_videoram->write_byte(a + 0x0800, m_registers[2]);
			m_videoram->write_byte(a + 0x1000, m_registers[3]);
			if (cmd&1) inc_x(7);
			break;
		case 0x02:  //KRG: R1,R2->ram
		case 0x03:  //KRG: R1,R2->ram + increment
			set_busy_flag(5500);
			m_videoram->write_byte(a, m_registers[1]);
			m_videoram->write_byte(a + 0x0800, m_registers[2]);
			if (cmd&1) inc_x(7);
			break;
		case 0x08:  //KRF: ram->R1,R2,R3
		case 0x09:  //KRF: ram->R1,R2,R3 + increment
			set_busy_flag(7500);
			m_registers[1] = m_videoram->read_byte(a);
			m_registers[2] = m_videoram->read_byte(a + 0x0800);
			m_registers[3] = m_videoram->read_byte(a + 0x1000);
			if (cmd&1) inc_x(7);
			break;
		case 0x0a:  //KRG: ram->R1,R2
		case 0x0b:  //KRG: ram->R1,R2 + increment
			set_busy_flag(7500);
			m_registers[1] = m_videoram->read_byte(a);
			m_registers[2] = m_videoram->read_byte(a + 0x0800);
			if (cmd&1) inc_x(7);
			break;
		case 0x30:  //OCT: R1->RAM, main pointer
		case 0x31:  //OCT: R1->RAM, main pointer + inc
			set_busy_flag(4000);
			m_videoram->write_byte(indexram(7), m_registers[1]);

			if (cmd&1)
			{
				inc_x(7);
				if ((m_registers[7] & 0x3f) == 0)
					inc_y(6);
			}
			break;
		case 0x34:  //OCT: R1->RAM, aux pointer
		case 0x35:  //OCT: R1->RAM, aux pointer + inc
			set_busy_flag(4000);
			m_videoram->write_byte(indexram(5), m_registers[1]);

			if (cmd&1)
				inc_x(5);
			break;
		case 0x38:  //OCT: RAM->R1, main pointer
		case 0x39:  //OCT: RAM->R1, main pointer + inc
			set_busy_flag(4500);
			m_registers[1] = m_videoram->read_byte(indexram(7));

			if (cmd&1)
			{
				inc_x(7);

				if ((m_registers[7] & 0x3f) == 0)
					inc_y(6);
			}
			break;
		case 0x3c:  //OCT: RAM->R1, aux pointer
		case 0x3d:  //OCT: RAM->R1, aux pointer + inc
			set_busy_flag(4500);
			m_registers[1] = m_videoram->read_byte(indexram(5));

			if (cmd&1)
				inc_x(5);
			break;
		case 0x50:  //KRL: 80 uint8_t - 12 bits write
		case 0x51:  //KRL: 80 uint8_t - 12 bits write + inc
			set_busy_flag(12500);
			m_videoram->write_byte(a, m_registers[1]);
			switch((a / 0x0800) & 1)
			{
				case 0:
				{
					uint8_t tmp_data = m_videoram->read_byte(a + 0x1000);
					m_videoram->write_byte(a + 0x1000, (tmp_data & 0x0f) | (m_registers[3] & 0xf0));
					break;
				}
				case 1:
				{
					uint8_t tmp_data = m_videoram->read_byte(a + 0x0800);
					m_videoram->write_byte(a + 0x0800, (tmp_data & 0xf0) | (m_registers[3] & 0x0f));
					break;
				}
			}
			if (cmd&1)
			{
				if ((m_registers[7] & 0x80) == 0x00) { m_registers[7] |= 0x80; return; }
				m_registers[7] &= ~0x80;
				inc_x(7);
			}
			break;
		case 0x58:  //KRL: 80 uint8_t - 12 bits read
		case 0x59:  //KRL: 80 uint8_t - 12 bits read + inc
			set_busy_flag(11500);
			m_registers[1] = m_videoram->read_byte(a);
			switch((a / 0x0800) & 1)
			{
				case 0:
					m_registers[3] = m_videoram->read_byte(a + 0x1000);
					break;
				case 1:
					m_registers[3] = m_videoram->read_byte(a + 0x0800);
					break;
			}
			if (cmd&1)
			{
				if ((m_registers[7] & 0x80) == 0x00)
				{
					m_registers[7] |= 0x80;
					break;
				}
				m_registers[7] &= 0x80;
				inc_x(7);
			}
			break;
		case 0x80:  //IND: R1->ROM (impossible ?)
			break;
		case 0x81:  //IND: R1->TGS
		case 0x82:  //IND: R1->MAT
		case 0x83:  //IND: R1->PAT
		case 0x84:  //IND: R1->DOR
		case 0x87:  //IND: R1->ROR
			set_busy_flag(2000);
			switch(cmd&7)
			{
				case 1:     m_tgs = m_registers[1]; break;
				case 2:     m_mat = m_registers[1]; break;
				case 3:     m_pat = m_registers[1]; break;
				case 4:     m_dor = m_registers[1]; break;
				case 7:     m_ror = m_registers[1]; break;
			}
			set_video_mode();
			m_state &= 0x8f;  //reset S4(LXa), S5(LXm), S6(Al)
			break;
		case 0x88:  //IND: ROM->R1
		case 0x89:  //IND: TGS->R1
		case 0x8a:  //IND: MAT->R1
		case 0x8b:  //IND: PAT->R1
		case 0x8c:  //IND: DOR->R1
		case 0x8f:  //IND: ROR->R1
			set_busy_flag(3500);
			switch(cmd&7)
			{
				case 0:     m_registers[1] = m_charset[indexrom(7) & 0x1fff]; break;
				case 1:     m_registers[1] = m_tgs; break;
				case 2:     m_registers[1] = m_mat; break;
				case 3:     m_registers[1] = m_pat; break;
				case 4:     m_registers[1] = m_dor; break;
				case 7:     m_registers[1] = m_ror; break;
			}
			m_state &= 0x8f;  //reset S4(LXa), S5(LXm), S6(Al)
			break;
		case 0x90:  //NOP: no operation
		case 0x91:  //NOP: no operation
		case 0x95:  //VRM: vertical sync mask reset
		case 0x99:  //VSM: vertical sync mask set
			break;
		case 0xb0:  //INY: increment Y
			set_busy_flag(2000);
			inc_y(6);
			m_state &= 0x8f;  //reset S4(LXa), S5(LXm), S6(Al)
			break;
		case 0xd5:  //MVB: move buffer MP->AP stop
		case 0xd6:  //MVB: move buffer MP->AP nostop
		case 0xd9:  //MVB: move buffer AP->MP stop
		case 0xda:  //MVB: move buffer AP->MP nostop
		case 0xe5:  //MVD: move double buffer MP->AP stop
		case 0xe6:  //MVD: move double buffer MP->AP nostop
		case 0xe9:  //MVD: move double buffer AP->MP stop
		case 0xea:  //MVD: move double buffer AP->MP nostop
		case 0xf5:  //MVT: move triple buffer MP->AP stop
		case 0xf6:  //MVT: move triple buffer MP->AP nostop
		case 0xf9:  //MVT: move triple buffer AP->MP stop
		case 0xfa:  //MVT: move triple buffer AP->MP nostop
		{
			uint16_t i, a1, a2;
			uint8_t n = (cmd>>4) - 0x0c;
			uint8_t r1 = (cmd&0x04) ? 7 : 5;
			uint8_t r2 = (cmd&0x04) ? 5 : 7;
			int busy = 2000;

			for(i = 0; i < 1280; i++)
			{
				a1 = indexram(r1); a2 = indexram(r2);
				m_videoram->write_byte(a2, m_videoram->read_byte(a1));

				if (n > 1) m_videoram->write_byte(a2 + 0x0800, m_videoram->read_byte(a1 + 0x0800));
				if (n > 2) m_videoram->write_byte(a2 + 0x1000, m_videoram->read_byte(a1 + 0x1000));

				inc_x(r1);
				inc_x(r2);
				if ((m_registers[5] & 0x3f) == 0 && (cmd&1))
					break;

				if ((m_registers[7] & 0x3f) == 0)
				{
					if (cmd&1)
						break;
					else
					inc_y(6);
				}

				busy += 4000 * n;
			}
			m_state &= 0x8f;  //reset S4(LXa), S5(LXm), S6(Al)
			set_busy_flag(busy);
		}
		break;
		case 0x05:  //CLF: Clear page 24 bits
		case 0x07:  //CLG: Clear page 16 bits
		case 0x40:  //KRC: R1 -> ram
		case 0x41:  //KRC: R1 -> ram + inc
		case 0x48:  //KRC: 80 characters - 8 bits
		case 0x49:  //KRC: 80 characters - 8 bits
		default:
			logerror("Unemulated EF9345 cmd: %02x\n", cmd);
	}
}


/**************************************************************
            EF9345 interface
**************************************************************/

uint32_t ef9345_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_screen_out, 0, 0, 0, 0, cliprect);
	return 0;
}

void ef9345_device::update_scanline(uint16_t scanline)
{
	uint16_t i;

	if (scanline == 250)
		m_state &= 0xfb;

	set_busy_flag(104000);

	if (m_char_mode == MODE12x80 || m_char_mode == MODE8x80)
	{
		draw_char_80(m_border, 0, (scanline / 10) + 1);
		draw_char_80(m_border, 81, (scanline / 10) + 1);
	}
	else
	{
		draw_char_40(m_border, 0, (scanline / 10) + 1);
		draw_char_40(m_border, 41, (scanline / 10) + 1);
	}

	if (scanline == 0)
	{
		m_state |= 0x04;
		draw_border(0);
		if (m_pat & 1)
			for(i = 0; i < 40; i++)
				makechar(i, (scanline / 10));
		else
			for(i = 0; i < 42; i++)
				draw_char_40(m_border, i, 1);
	}
	else if (scanline < 120)
	{
		if (m_pat & 2)
			for(i = 0; i < 40; i++)
				makechar(i, (scanline / 10));
		else
			draw_border(scanline / 10);
	}
	else if (scanline < 250)
	{
		if (m_variant == EF9345_MODE::TYPE_TS9347)
		{
			for(i = 0; i < 40; i++)
				makechar(i, (scanline / 10));
		}
		else
		{
			if (m_pat & 4) // Lower bulk enable
				for(i = 0; i < 40; i++)
					makechar(i, (scanline / 10));
			else
				draw_border(scanline / 10);

			if (scanline == 240)
				draw_border(26);
		}
	}
}

uint8_t ef9345_device::data_r(offs_t offset)
{
	if (offset & 7)
		return m_registers[offset & 7];

	if (m_bf)
		m_state |= 0x80;
	else
		m_state &= 0x7f;

	return m_state;
}

void ef9345_device::data_w(offs_t offset, uint8_t data)
{
	m_registers[offset & 7] = data;

	if (offset & 8)
		ef9345_exec(m_registers[0] & 0xff);
}
