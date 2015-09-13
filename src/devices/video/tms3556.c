// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    tms3556 emulation

    TODO:
    * implement remaining flags in control registers
    * test the whole thing
    * find the bloody tms3556 manual.  I mean the register and VRAM interfaces
      are mostly guesswork full of hacks, and I'd like to compare it with
      documentation.

    Raphael Nabet, 2004
*/

#include "emu.h"
#include "tms3556.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type TMS3556 = &device_creator<tms3556_device>;


// default address map
static ADDRESS_MAP_START( tms3556, AS_0, 8, tms3556_device )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *tms3556_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 tms3556_device::readbyte(offs_t address)
{
	return space().read_byte(address&0xFFFF);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void tms3556_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address&0xFFFF, data);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms3556_device - constructor
//-------------------------------------------------

tms3556_device::tms3556_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS3556, "Texas Instruments VDP TMS3556", tag, owner, clock, "tms3556", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 17, 0, NULL, *ADDRESS_MAP_NAME(tms3556)),
		m_reg_access_phase(0),
		m_row_col_written(0),
		m_bamp_written(0),
		m_colrow(0),
		m_vdp_acmpxy_mode(dma_write),
		m_vdp_acmpxy(0),
		m_vdp_acmp(0),
		m_init_read(0),
		m_scanline(0),
		m_blink(0),
		m_blink_count(0),
		m_bg_color(0)
{
	for (int i = 0; i < 8; i++)
	{
		m_control_regs[i] = 0;
		m_address_regs[i] = 0xFFFF;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms3556_device::device_start()
{
	// register for state saving
	save_item(NAME(m_control_regs));
	save_item(NAME(m_address_regs));
	save_item(NAME(m_reg_access_phase));
	save_item(NAME(m_row_col_written));
	save_item(NAME(m_bamp_written));
	save_item(NAME(m_colrow));
//    save_item(NAME(m_vdp_acmpxy_mode)); // FIXME : mame cannot save enum
	save_item(NAME(m_vdp_acmpxy));
	save_item(NAME(m_vdp_acmp));
	save_item(NAME(m_scanline));
	save_item(NAME(m_blink));
	save_item(NAME(m_blink_count));
	save_item(NAME(m_bg_color));
	save_item(NAME(m_name_offset));
	save_item(NAME(m_cg_flag));
	save_item(NAME(m_char_line_counter));
	save_item(NAME(m_dbl_h_phase));

	machine().first_screen()->register_screen_bitmap(m_bitmap);
}


/*static const char *const tms3556_mode_names[] = { "DISPLAY OFF", "TEXT", "GRAPHIC", "MIXED" };*/


UINT32 tms3556_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


//-------------------------------------------------
//  vram_r - VRAM read
//-------------------------------------------------

READ8_MEMBER( tms3556_device::vram_r )
{
	UINT8 ret;
	if (m_bamp_written) {
		m_bamp_written=false;
		m_vdp_acmpxy_mode=dma_write;
		if (m_init_read)
			m_vdp_acmp=VDP_BAMP;
		else
			m_vdp_acmp=(VDP_BAMP-1)&0xFFFF;
	}

	if (m_row_col_written) {
		m_row_col_written=0;
		m_vdp_acmpxy_mode=dma_read;
		if (m_init_read)
			m_vdp_acmpxy=m_colrow;
		else
			m_vdp_acmpxy=(m_colrow-1)&0xFFFF;
	}

	m_init_read=false;
	if (m_vdp_acmpxy_mode==dma_read) {
		ret=readbyte(m_vdp_acmpxy);
		m_vdp_acmpxy++;
		if (m_vdp_acmpxy==VDP_BAMTF) m_vdp_acmpxy=VDP_BAMP;
	} else {
		ret=readbyte(m_vdp_acmp);
		m_vdp_acmp++;
		if (m_vdp_acmp==VDP_BAMTF) m_vdp_acmp=VDP_BAMP;
	}
	return ret;
}

//-------------------------------------------------
//  vram_w - VRAM write
//-------------------------------------------------

WRITE8_MEMBER( tms3556_device::vram_w )
{
	if (m_bamp_written) {
		m_bamp_written=false;
		m_vdp_acmpxy_mode=dma_read;
		m_vdp_acmp=VDP_BAMP;
	}

	if (m_row_col_written) {
		m_row_col_written=0;
		m_vdp_acmpxy_mode=dma_write;
		m_vdp_acmpxy=m_colrow;
	}

	if (m_vdp_acmpxy_mode==dma_write) {
		writebyte(m_vdp_acmpxy,data);
		m_vdp_acmpxy++;
		if (m_vdp_acmpxy==VDP_BAMTF) m_vdp_acmpxy=VDP_BAMP;
	} else {
		writebyte(m_vdp_acmp,data);
		m_vdp_acmp++;
		if (m_vdp_acmp==VDP_BAMTF) m_vdp_acmp=VDP_BAMP;
	}

}


//-------------------------------------------------
//  reg_r - read from register port
//-------------------------------------------------

READ8_MEMBER( tms3556_device::reg_r )
{
	if (LOG) logerror("TMS3556 Reg Read: %06x\n", offset);

	int reply = 0; // FIXME : will send internal status (VBL, HBL...)
	m_reg_access_phase=0;
	return reply;
}

//-------------------------------------------------
//  reg_w - write to register port
//-------------------------------------------------

WRITE8_MEMBER( tms3556_device::reg_w )
{
	static int reg2=0; // FIXME : this static makes that only one TMS3556 will be present in one system...
	static int reg=0;

	if (LOG) logerror("TMS3556 Reg Write: %06x = %02x\n", offset, data);

	switch (m_reg_access_phase) {
	case 0:
		reg=data&0x0F;
		reg2=(data&0xF0)>>4;
		if (reg!=0)
			m_reg_access_phase=1;
		return;

	case 1:
		if (reg<8) {
					m_control_regs[reg]=data;
					// leve un flag si le dernier registre ecrit est row ou col
					if ((reg==2) || (reg==1)) {
						m_colrow=(m_control_regs[2]<<8)|m_control_regs[1];
						m_row_col_written=true;
						}

					if (reg2==0) {
						m_reg_access_phase=0;
						return;
					}
					else {
						m_reg_access_phase=1;
						reg=reg2;
						reg2=0;
						return;
					}
		} else {
				m_address_regs[reg-8]=(m_control_regs[2]<<8)|m_control_regs[1];
				// cas speciaux de decalage pour les generateurs
				if ((reg>=0xB) && (reg<=0xE)) {
					m_address_regs[reg-8]+=2;
					m_address_regs[reg-8]&=0xFFFF;
				} else {
					m_address_regs[reg-8]+=1;
					m_address_regs[reg-8]&=0xFFFF;
				}
				if (reg==9) {
					m_row_col_written=false;
					m_bamp_written=true;
					m_reg_access_phase=0;
					return;
				} else {
					m_row_col_written=0;
					m_bamp_written=false;
					m_reg_access_phase=2;//???
					return;
				}
				logerror("VDP16[%d] = x%x",reg,m_address_regs[reg-8]);
				if (reg2==0) {
					m_reg_access_phase=0;
					return;
				}
				else {
					m_reg_access_phase=1;
					reg=reg2;
					reg2=0;
					return;
				}
		}
		case 2:
			m_reg_access_phase=0;
			return;
	}
}

//--------------------------------------------------------------------------
//  initptr_r - set VDP in read mode (not exacly on the VDP but on the TAL)
//--------------------------------------------------------------------------

READ8_MEMBER( tms3556_device::initptr_r )
{
	m_init_read=true;
	return 0xff;
}


//-------------------------------------------------
//  redraw code
//-------------------------------------------------


//-------------------------------------------------
//  draw_line_empty - draw an empty line (used for
//  top and bottom borders, and screen off mode)
//-------------------------------------------------

void tms3556_device::draw_line_empty(UINT16 *ln)
{
	int i;

	for (i = 0; i < TMS3556_TOTAL_WIDTH; i++)
#if TMS3556_DOUBLE_WIDTH
		*ln++ = m_bg_color;
#endif
		*ln++ = m_bg_color;
}


//-------------------------------------------------
//  draw_line_text_common - draw a line of text
//  (called by draw_line_text and draw_line_mixed)
//-------------------------------------------------

void tms3556_device::draw_line_text_common(UINT16 *ln)
{
	int pattern, x, xx, i, name_offset;
	UINT16 fg, bg;
	offs_t nametbl_base;
	offs_t patterntbl_base[4];
	int name_hi, name_lo;
	int pattern_ix;
	int alphanumeric_mode, dbl_w, dbl_h, dbl_w_phase = 0;

	nametbl_base = m_address_regs[2];
	for (i = 0; i < 4; i++)
		patterntbl_base[i] = m_address_regs[i + 3];

	for (xx = 0; xx < TMS3556_LEFT_BORDER; xx++)
#if TMS3556_DOUBLE_WIDTH
		*ln++ = m_bg_color;
#endif
		*ln++ = m_bg_color;

	name_offset = m_name_offset;

	for (x = 0; x < 40; x++)
	{
		name_hi = readbyte(nametbl_base + name_offset);
		name_lo = readbyte(nametbl_base + name_offset + 1);
		pattern_ix = ((name_hi >> 2) & 2) | ((name_hi >> 4) & 1);
		alphanumeric_mode = (pattern_ix < 2) || ((pattern_ix == 3) && !(m_control_regs[7] & 0x08));
		fg = (name_hi >> 5) & 0x7;
		if (alphanumeric_mode)
		{
			if (name_hi & 4)
			{   /* inverted color */
				bg = fg;
				fg = m_bg_color;
			}
			else
				bg = m_bg_color;
			dbl_w = name_hi & 0x2;
			dbl_h = name_hi & 0x1;
		}
		else
		{
			bg = name_hi & 0x7;
			dbl_w = 0;
			dbl_h = 0;
		}
		if ((name_lo & 0x80) && m_blink)
			fg = bg;    /* blink off time */
		if (! dbl_h)
		{   /* single height */
			pattern = readbyte(patterntbl_base[pattern_ix] + (name_lo & 0x7f) + 128 * m_char_line_counter);
			if (m_char_line_counter == 0)
				m_dbl_h_phase[x] = 0;
		}
		else
		{   /* double height */
			if (! m_dbl_h_phase[x])
				/* first phase: pattern from upper half */
				pattern = readbyte(patterntbl_base[pattern_ix] + (name_lo & 0x7f) + 128 * (5 + (m_char_line_counter >> 1)));
			else
				/* second phase: pattern from lower half */
				pattern = readbyte(patterntbl_base[pattern_ix] + (name_lo & 0x7f) + 128 * (m_char_line_counter >> 1));
			if (m_char_line_counter == 0)
				m_dbl_h_phase[x] = !m_dbl_h_phase[x];
		}
		if (!dbl_w)
		{   /* single width */
			for (xx = 0; xx < 8; xx++)
			{
				UINT16 color = (pattern & 0x80) ? fg : bg;
#if TMS3556_DOUBLE_WIDTH
				*ln++ = color;
#endif
				*ln++ = color;
				pattern <<= 1;
			}
			dbl_w_phase = 0;
		}
		else
		{   /* double width */
			if (dbl_w_phase)
				/* second phase: display right half */
				pattern <<= 4;
			for (xx = 0; xx < 4; xx++)
			{
				UINT16 color = (pattern & 0x80) ? fg : bg;
#if TMS3556_DOUBLE_WIDTH
				*ln++ = color; *ln++ = color;
#endif
				*ln++ = color; *ln++ = color;
				pattern <<= 1;
			}
			dbl_w_phase = !dbl_w_phase;
		}
		name_offset += 2;
	}

	for (xx = 0; xx < TMS3556_RIGHT_BORDER; xx++)
#if TMS3556_DOUBLE_WIDTH
		*ln++ = m_bg_color;
#endif
		*ln++ = m_bg_color;

	if (m_char_line_counter == 0)
		m_name_offset = name_offset;
}


//-------------------------------------------------
//  draw_line_bitmap_common - draw a line of bitmap
//  (called by draw_line_bitmap and draw_line_mixed)
//-------------------------------------------------

void tms3556_device::draw_line_bitmap_common(UINT16 *ln)
{
	int x, xx;
	offs_t nametbl_base;
	int name_b, name_g, name_r;

	nametbl_base = m_address_regs[2];

	for (xx = 0; xx < TMS3556_LEFT_BORDER; xx++)
#if TMS3556_DOUBLE_WIDTH
		*ln++ = m_bg_color;
#endif
		*ln++ = m_bg_color;

	for (x = 0; x < 40; x++)
	{
		name_b = readbyte(nametbl_base + m_name_offset);
		name_g = readbyte(nametbl_base + m_name_offset + 1);
		name_r = readbyte(nametbl_base + m_name_offset + 2);
		for (xx = 0; xx < 8; xx++)
		{
			UINT16 color = ((name_b >> 5) & 0x4) | ((name_g >> 6) & 0x2) | ((name_r >> 7) & 0x1);
#if TMS3556_DOUBLE_WIDTH
			*ln++ = color;
#endif
			*ln++ = color;
			name_b <<= 1;
			name_g <<= 1;
			name_r <<= 1;
		}
		m_name_offset += 3;
	}

	for (xx = 0; xx < TMS3556_RIGHT_BORDER; xx++)
#if TMS3556_DOUBLE_WIDTH
		*ln++ = m_bg_color;
#endif
		*ln++ = m_bg_color;
}


//-------------------------------------------------
//  draw_line_text - draw a line in text mode
//-------------------------------------------------

void tms3556_device::draw_line_text(UINT16 *ln)
{
	if (m_char_line_counter == 0)
		m_char_line_counter = 10;
	m_char_line_counter--;
	draw_line_text_common(ln);
}


//-------------------------------------------------
//  draw_line_bitmap - draw a line in bitmap mode
//-------------------------------------------------

void tms3556_device::draw_line_bitmap(UINT16 *ln)
{
	draw_line_bitmap_common(ln);
	m_bg_color = (readbyte(m_address_regs[2] + m_name_offset) >> 5) & 0x7;
	m_name_offset += 2;
}


//-------------------------------------------------
//  draw_line_mixed - draw a line in mixed mode
//-------------------------------------------------

void tms3556_device::draw_line_mixed(UINT16 *ln)
{
	if (m_cg_flag)
	{   /* bitmap line */
		draw_line_bitmap_common(ln);
		m_bg_color = (readbyte(m_address_regs[2] + m_name_offset) >> 5) & 0x7;
		m_cg_flag = (readbyte(m_address_regs[2] + m_name_offset) >> 4) & 0x1;
		m_name_offset += 2;
	}
	else
	{   /* text line */
		if (m_char_line_counter == 0)
			m_char_line_counter = 10;
		m_char_line_counter--;
		draw_line_text_common(ln);
		if (m_char_line_counter == 0)
		{
			m_bg_color = (readbyte(m_address_regs[2] + m_name_offset) >> 5) & 0x7;
			m_cg_flag = (readbyte(m_address_regs[2] + m_name_offset) >> 4) & 0x1;
			m_name_offset += 2;
		}
	}
}


//-------------------------------------------------
//  draw_line - draw a line. If non-interlaced mode,
//  duplicate the line.
//-------------------------------------------------

void tms3556_device::draw_line(bitmap_ind16 &bmp, int line)
{
	int double_lines = 0;
	UINT16 *ln, *ln2 = NULL;

//  if (m_control_regs[4] & 0x??)
//  {   // interlaced mode
//      ln = &bmp->pix16(line, m_field);
//  }
//  else
	{   /* non-interlaced mode */
		ln = &bmp.pix16(line);
		ln2 = &bmp.pix16(line, 1);
		double_lines = 1;
	}

	if ((line < TMS3556_TOP_BORDER) || (line >= (TMS3556_TOP_BORDER + 250)))
	{
		/* draw top and bottom borders */
		draw_line_empty(ln);
		m_cg_flag=0; // FIXME : forme text mode for 1st line in mixed
	}
	else
	{
		/* draw useful area */
		switch (m_control_regs[6] >> 6)
		{
		case TMS3556_MODE_OFF:
			draw_line_empty(ln);
			break;
		case TMS3556_MODE_TEXT:
			draw_line_text(ln);
			break;
		case TMS3556_MODE_BITMAP:
			draw_line_bitmap(ln);
			break;
		case TMS3556_MODE_MIXED:
			draw_line_mixed(ln);
			break;
		}
	}

	if (double_lines)
	{
		// TODO: this overlaps in exeltel - use memmove for now
		//memcpy(ln2, ln, TMS3556_TOTAL_WIDTH * (TMS3556_DOUBLE_WIDTH ? 2 : 1));
		memmove(ln2, ln, TMS3556_TOTAL_WIDTH * (TMS3556_DOUBLE_WIDTH ? 2 : 1));
	}
}


//-------------------------------------------------
//  interrupt_start_vblank - Do vblank-time tasks
//-------------------------------------------------

void tms3556_device::interrupt_start_vblank(void)
{
	/* at every frame, vdp switches fields */
	//m_field = !m_field;

	/* color blinking */
	if (m_blink_count)
		m_blink_count--;
	if (!m_blink_count)
	{
		m_blink = !m_blink;
		m_blink_count = 60; /*no idea what the real value is*/
	}
	/* reset background color */
	m_bg_color = (m_control_regs[7] >> 5) & 0x7;
	/* reset name offset */
	m_name_offset = 0;
	/* reset character line counter */
	m_char_line_counter = 0;
	/* reset c/g flag */
	m_cg_flag = 0;
	/* reset double height phase flags */
	memset(m_dbl_h_phase, 0, sizeof(m_dbl_h_phase));
}


//-------------------------------------------------
//  interrupt - scanline handler
//-------------------------------------------------

void tms3556_device::interrupt(running_machine &machine)
{
	/* check for start of vblank */
	if (m_scanline == 310)  /*no idea what the real value is*/
		interrupt_start_vblank();

	/* render the current line */
	if ((m_scanline >= 0) && (m_scanline < TMS3556_TOTAL_HEIGHT))
	{
		//if (!video_skip_this_frame())
			draw_line(m_bitmap, m_scanline);
	}

	if (++m_scanline == 313)
		m_scanline = 0;
}
