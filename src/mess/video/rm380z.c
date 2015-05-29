// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z video code

*/


#include "includes/rm380z.h"


void rm380z_state::put_point(int charnum,int x,int y,int col)
{
	int mx=3;
	if (y==6) mx=4;

	for (unsigned int r=y;r<(y+mx);r++)
	{
		for (unsigned int c=x;c<(x+3);c++)
		{
			m_graphic_chars[charnum][c+(r*(RM380Z_CHDIMX+1))]=col;
		}
	}
}

void rm380z_state::init_graphic_chars()
{
	for (int c=0;c<0x3f;c++)
	{
		if (c&0x01) put_point(c,0,0,1);
		else                put_point(c,0,0,0);

		if (c&0x02) put_point(c,3,0,1);
		else                put_point(c,3,0,0);

		if (c&0x04) put_point(c,0,3,1);
		else                put_point(c,0,3,0);

		if (c&0x08) put_point(c,3,3,1);
		else                put_point(c,3,3,0);

		if (c&0x10) put_point(c,0,6,1);
		else                put_point(c,0,6,0);

		if (c&0x20) put_point(c,3,6,1);
		else                put_point(c,3,6,0);
	}
}

void rm380z_state::config_videomode()
{
	if (m_port0&0x20)
	{
		// 80 cols
		m_videomode=RM380Z_VIDEOMODE_80COL;
	}
	else
	{
		// 40 cols
		m_videomode=RM380Z_VIDEOMODE_40COL;
	}

	if (m_old_videomode!=m_videomode)
	{
		m_old_videomode=m_videomode;
	}
}

// char attribute bits in COS 4.0

// 0=alternate charset
// 1=underline
// 2=dim
// 3=reverse


void rm380z_state::decode_videoram_char(int pos,UINT8& chr,UINT8& attrib)
{
	UINT8 ch1=m_vramchars[pos];
	UINT8 ch2=m_vramattribs[pos];

	// "special" (unknown) cases first
	if ((ch1==0x80)&&(ch2==0x04))
	{
		// blank out
		chr=0x20;
		attrib=0;
		return;
	}
	else if ((ch1==0)&&(ch2==8))
	{
		// cursor
		chr=0x20;
		attrib=8;
		return;
	}
	else if ((ch1==0)&&(ch2==0))
	{
		// delete char (?)
		chr=0x20;
		attrib=0;
		return;
	}
	else if ((ch1==4)&&(ch2==4))
	{
		// reversed cursor?
		chr=0x20;
		attrib=0;
		return;
	}
	else if ((ch1==4)&&(ch2==8))
	{
		// normal cursor
		chr=0x20;
		attrib=8;
		return;
	}
	else
	{
		chr=ch1;
		attrib=ch2;

		//printf("unhandled character combination [%x][%x]\n",ch1,ch2);
	}
}

void rm380z_state::scroll_videoram()
{
	int lineWidth=0x80;
	if (m_videomode==RM380Z_VIDEOMODE_40COL)
	{
		lineWidth=0x40;
	}

	// scroll up one row of videoram

	for (int row=1;row<RM380Z_SCREENROWS;row++)
	{
		for (int c=0;c<lineWidth;c++)
		{
			int sourceaddr=(row*lineWidth)+c;
			int destaddr=((row-1)*lineWidth)+c;

			m_vram[destaddr]=m_vram[sourceaddr];
			m_vramchars[destaddr]=m_vramchars[sourceaddr];
			m_vramattribs[destaddr]=m_vramattribs[sourceaddr];
		}
	}

	// the last line is filled with spaces

	for (int c=0;c<lineWidth;c++)
	{
		m_vram[((RM380Z_SCREENROWS-1)*lineWidth)+c]=0x20;
		m_vramchars[((RM380Z_SCREENROWS-1)*lineWidth)+c]=0x20;
		m_vramattribs[((RM380Z_SCREENROWS-1)*lineWidth)+c]=0x00;
	}
}

void rm380z_state::check_scroll_register()
{
	UINT8 r[3];

	r[0]=m_old_old_fbfd;
	r[1]=m_old_fbfd;
	r[2]=m_fbfd;

	if ( ((r[1]&0x20)==0) && ((r[2]&0x20)==0) )
	{
		// it's a scroll command

		if (r[2]>r[1])
		{
			scroll_videoram();
		}
		else if ((r[2]==0x00)&&(r[1]==0x17))
		{
			// wrap-scroll
			scroll_videoram();
		}

	}
}

// after ctrl-L (clear screen?): routine at EBBD is executed
// EB30??? next line?
// memory at FF02 seems to hold the line counter (same as FBFD)
//
// basics:
// 20e2: prints "Ready:"
// 0195: prints "\n"

WRITE8_MEMBER( rm380z_state::videoram_write )
{
	//printf("vramw [%2.2x][%2.2x] port0 [%2.2x] fbfd [%2.2x] fbfe [%2.2x] PC [%4.4x]\n",offset,data,m_port0,m_fbfd,m_fbfe,m_maincpu->safe_pc());

	int lineWidth=0x80;
	if (m_videomode==RM380Z_VIDEOMODE_40COL)
	{
		lineWidth=0x40;
	}

	int rowadder=(m_fbfe&0x0f)*2;
	if (m_videomode==RM380Z_VIDEOMODE_40COL) rowadder=0; // FBFE register is not used in VDU-40

	int lineAdder=rowadder*lineWidth;
	int realA=(offset+lineAdder);

	// we suppose videoram is being written as character/attribute couple
	// fbfc 6th bit set=attribute, unset=char

	if (!(m_port0&0x40))
	{
		m_vramchars[realA%RM380Z_SCREENSIZE]=data;
	}
	else
	{
		m_vramattribs[realA%RM380Z_SCREENSIZE]=data;
	}

	//

	m_mainVideoram[offset]=data;
}

READ8_MEMBER( rm380z_state::videoram_read )
{
	return m_mainVideoram[offset];
}

void rm380z_state::putChar(int charnum,int attribs,int x,int y,bitmap_ind16 &bitmap,unsigned char* chsb,int vmode)
{
	//bool attrDim=false;
	bool attrRev=false;
	bool attrUnder=false;

	if (attribs&0x02) attrUnder=true;
	//if (attribs&0x04) attrDim=true;
	if (attribs&0x08) attrRev=true;

	if ((charnum>0)&&(charnum<=0x7f))
	{
		// normal chars (base set)

		if (vmode==RM380Z_VIDEOMODE_80COL)
		{
			int basex=RM380Z_CHDIMX*(charnum/RM380Z_NCY);
			int basey=RM380Z_CHDIMY*(charnum%RM380Z_NCY);

			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<RM380Z_CHDIMX;c++)
				{
					UINT8 chval=(chsb[((basey+r)*(RM380Z_CHDIMX*RM380Z_NCX))+(basex+c)])==0xff?0:1;

					if (attrRev)
					{
						if (chval==0) chval=1;
						else chval=0;
					}

					if (attrUnder)
					{
						if (r==(RM380Z_CHDIMY-1))
						{
							if (attrRev) chval=0;
							else chval=1;
						}
					}

					UINT16 *dest=&bitmap.pix16((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+c);
					*dest=chval;
				}
			}

			// last pixel of underline
			if (attrUnder&&(!attrRev))
			{
				UINT16 *dest=&bitmap.pix16((y*(RM380Z_CHDIMY+1))+(RM380Z_CHDIMY-1),(x*(RM380Z_CHDIMX+1))+RM380Z_CHDIMX);
				*dest=attrRev?0:1;
			}

			// if reversed, print another column of pixels on the right
			if (attrRev)
			{
				for (int r=0;r<RM380Z_CHDIMY;r++)
				{
					UINT16 *dest=&bitmap.pix16((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+RM380Z_CHDIMX);
					*dest=1;
				}
			}
		}
		else if (vmode==RM380Z_VIDEOMODE_40COL)
		{
			int basex=RM380Z_CHDIMX*(charnum/RM380Z_NCY);
			int basey=RM380Z_CHDIMY*(charnum%RM380Z_NCY);

			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<(RM380Z_CHDIMX*2);c+=2)
				{
					UINT8 chval=(chsb[((basey+r)*(RM380Z_CHDIMX*RM380Z_NCX))+(basex+(c/2))])==0xff?0:1;

					if (attrRev)
					{
						if (chval==0) chval=1;
						else chval=0;
					}

					if (attrUnder)
					{
						if (r==(RM380Z_CHDIMY-1))
						{
							if (attrRev) chval=0;
							else chval=1;
						}
					}

					UINT16 *dest=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c);
					UINT16 *dest2=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c+1);
					*dest=chval;
					*dest2=chval;
				}
			}

			// last 2 pixels of underline
			if (attrUnder)
			{
				UINT16 *dest=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+RM380Z_CHDIMY-1 , ((x*(RM380Z_CHDIMX+1))*2)+(RM380Z_CHDIMX*2));
				UINT16 *dest2=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+RM380Z_CHDIMY-1 , ((x*(RM380Z_CHDIMX+1))*2)+(RM380Z_CHDIMX*2)+1);
				*dest=attrRev?0:1;
				*dest2=attrRev?0:1;
			}

			// if reversed, print another 2 columns of pixels on the right
			if (attrRev)
			{
				for (int r=0;r<RM380Z_CHDIMY;r++)
				{
					UINT16 *dest=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+((RM380Z_CHDIMX)*2));
					UINT16 *dest2=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+((RM380Z_CHDIMX)*2)+1);
					*dest=1;
					*dest2=1;
				}
			}
		}
	}
	else
	{
		// graphic chars: 0x80-0xbf is "dimmed", 0xc0-0xff is full bright
		if (vmode==RM380Z_VIDEOMODE_80COL)
		{
			for (int r=0;r<(RM380Z_CHDIMY+1);r++)
			{
				for (int c=0;c<RM380Z_CHDIMX;c++)
				{
					UINT16 *dest=&bitmap.pix16((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+c);
					*dest=m_graphic_chars[charnum&0x3f][c+(r*(RM380Z_CHDIMX+1))];
				}
			}
		}
		else
		{
			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<(RM380Z_CHDIMX*2);c+=2)
				{
					UINT16 *dest=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c);
					UINT16 *dest2=&bitmap.pix16( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c+1);
					*dest=m_graphic_chars[charnum&0x3f][(c/2)+(r*(RM380Z_CHDIMX+1))];
					*dest2=m_graphic_chars[charnum&0x3f][(c/2)+(r*(RM380Z_CHDIMX+1))];
				}
			}
		}
	}
}

void rm380z_state::update_screen(bitmap_ind16 &bitmap)
{
	unsigned char* pChar=memregion("chargen")->base();

	int lineWidth=0x80;
	int ncols=80;

	if (m_videomode==RM380Z_VIDEOMODE_40COL)
	{
		lineWidth=0x40;
		ncols=40;
	}

	// blank screen
	bitmap.fill(0);

	for (int row=0;row<RM380Z_SCREENROWS;row++)
	{
		for (int col=0;col<ncols;col++)
		{
			UINT8 curch,attribs;
			decode_videoram_char((row*lineWidth)+col,curch,attribs);
			putChar(curch,attribs,col,row,bitmap,pChar,m_videomode);
			//putChar(0x44,0x00,10,10,bitmap,pChar,m_videomode);
		}
	}
}
