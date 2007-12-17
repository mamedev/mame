/**********************************************************************

    Bellfruit BD1 VFD module interface and emulation by J.Wallace

**********************************************************************/

#include "driver.h"
#include "bfm_bd1.h"

static struct
{
	UINT8	type,				// type of alpha display

			changed,			// flag <>0, if contents are changed
			window_start,		// display window start pos 0-15
			window_end,			// display window end   pos 0-15
			window_size,		// window  size
			pad;				// unused align byte

	INT8	pcursor_pos,		// previous cursor pos
			cursor_pos;			// current cursor pos

	UINT16  user_def,			// user defined character state
			user_data;			// user defined character data (16 bit)

	UINT8	scroll_active,		// flag <>0, scrolling active
			display_mode,		// display scroll   mode, 0/1/2/3
			display_blanking,	// display blanking mode, 0/1/2/3
			flash_rate,			// flash rate 0-F
			flash_control;		// flash control 0/1/2/3

	UINT8	string[18];			// text buffer
	UINT32  segments[16],		// segments
			outputs[16];		// standardised outputs

	UINT8	count,				// bit counter
			data;				// receive register

} bd1[MAX_BD1];


// local prototypes ///////////////////////////////////////////////////////

static void ScrollLeft( int id);
static int  BD1_setdata(int id, int segdata, int data);

// local vars /////////////////////////////////////////////////////////////

//
// Bellfruit BD1 charset to ASCII conversion table
//
static const char BD1ASCII[] =
//0123456789ABCDEF0123456789ABC DEF01 23456789ABCDEF0123456789ABCDEF
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+.-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+.-./0123456789&%<=>?";

/*
   BD1 14 segment charset lookup table
        2
    ---------
   |\   |3  /|
 0 | \6 |  /7| 1
   |  \ | /  |
    -F-- --E-
   |  / | \  |
 D | /4 |A \B| 5
   |/   |   \|
    ---------  C
        9

  */

static UINT16 BD1charset[]=
{           // FEDC BA98 7654 3210
	0xA626, // 1010 0110 0010 0110 @.
	0xE027, // 1110 0000 0010 0111 A.
	0x462E, // 0100 0110 0010 1110 B.
	0x2205, // 0010 0010 0000 0101 C.
	0x062E, // 0000 0110 0010 1110 D.
	0xA205, // 1010 0010 0000 0101 E.
	0xA005, // 1010 0000 0000 0101 F.
	0x6225, // 0110 0010 0010 0101 G.
	0xE023, // 1110 0000 0010 0011 H.
	0x060C, // 0000 0110 0000 1100 I.
	0x2222, // 0010 0010 0010 0010 J.
	0xA881, // 1010 1000 1000 0001 K.
	0x2201, // 0010 0010 0000 0001 L.
	0x20E3, // 0010 0000 1110 0011 M.
	0x2863, // 0010 1000 0110 0011 N.
	0x2227, // 0010 0010 0010 0111 O.
	0xE007, // 1110 0000 0000 0111 P.
	0x2A27, // 0010 1010 0010 0111 Q.
	0xE807, // 1110 1000 0000 0111 R.
	0xC225, // 1100 0010 0010 0101 S.
	0x040C, // 0000 0100 0000 1100 T.
	0x2223, // 0010 0010 0010 0011 U.
	0x2091, // 0010 0000 1001 0001 V.
	0x2833, // 0010 1000 0011 0011 W.
	0x08D0, // 0000 1000 1101 0000 X.
	0x04C0, // 0000 0100 1100 0000 Y.
	0x0294, // 0000 0010 1001 0100 Z.
	0x2205, // 0010 0010 0000 0101 [.
	0x0840, // 0000 1000 0100 0000 \.
	0x0226, // 0000 0010 0010 0110 ].
	0x0810, // 0000 1000 0001 0000 ^.
	0x0200, // 0000 0010 0000 0000 _
	0x0000, // 0000 0000 0000 0000
	0xC290, // 1100 0010 1001 0000 POUND.
	0x0009, // 0000 0000 0000 1001 ".
	0xC62A, // 1100 0110 0010 1010 #.
	0xC62D, // 1100 0110 0010 1101 $.
	0x0100, // 0000 0000 0000 0000 flash ?
	0x0000, // 0000 0000 0000 0000 not defined
	0x0040, // 0000 0000 1000 0000 '.
	0x0880, // 0000 1000 1000 0000 (.
	0x0050, // 0000 0000 0101 0000 ).
	0xCCD8, // 1100 1100 1101 1000 *.
	0xC408, // 1100 0100 0000 1000 +.
	0x1000, // 0001 0000 0000 0000 .
	0xC000, // 1100 0000 0000 0000 -.
	0x1000, // 0001 0000 0000 0000 ..
	0x0090, // 0000 0000 1001 0000 /.
	0x22B7, // 0010 0010 1011 0111 0.
	0x0408, // 0000 0100 0000 1000 1.
	0xE206, // 1110 0010 0000 0110 2.
	0x4226, // 0100 0010 0010 0110 3.
	0xC023, // 1100 0000 0010 0011 4.
	0xC225, // 1100 0010 0010 0101 5.
	0xE225, // 1110 0010 0010 0101 6.
	0x0026, // 0000 0000 0010 0110 7.
	0xE227, // 1110 0010 0010 0111 8.
	0xC227, // 1100 0010 0010 0111 9.
	0xFFFF, // 0000 0000 0000 0000 user defined, can be replaced by main program
	0x0000, // 0000 0000 0000 0000 dummy
	0x0290, // 0000 0010 1001 0000 <.
	0xC200, // 1100 0010 0000 0000 =.
	0x0A40, // 0000 1010 0100 0000 >.
	0x4406, // 0100 0100 0000 0110 ?
};

///////////////////////////////////////////////////////////////////////////

void BFM_BD1_init(int id)
{
	assert_always((id >= 0) && (id < MAX_BD1), "BFM_BD1_init called on an invalid display ID!");

	memset( &bd1[id], 0, sizeof(bd1[0]));

	BFM_BD1_reset(id);
}

///////////////////////////////////////////////////////////////////////////

void BFM_BD1_reset(int id)
{
	bd1[id].window_end  = 15;
	bd1[id].window_size = (bd1[id].window_end - bd1[id].window_start)+1;
	memset(bd1[id].string, ' ', 16);

	bd1[id].count      = 0;

	bd1[id].changed |= 1;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *BFM_BD1_get_segments(int id)
{
	return bd1[id].segments;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *BFM_BD1_get_outputs(int id)
{
	return bd1[id].outputs;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *BFM_BD1_set_outputs(int id)
{
	int cursor;
	for (cursor = 0; cursor < 16; cursor++)
	{
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0004 )	bd1[id].outputs[cursor] |=  0x0001;
		else    	                    					bd1[id].outputs[cursor] &= ~0x0001;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0002 )	bd1[id].outputs[cursor] |=  0x0002;
		else        	                					bd1[id].outputs[cursor] &= ~0x0002;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0020 )	bd1[id].outputs[cursor] |=  0x0004;
		else            	            					bd1[id].outputs[cursor] &= ~0x0004;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0200 )	bd1[id].outputs[cursor] |=  0x0008;
		else                	        					bd1[id].outputs[cursor] &= ~0x0008;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x2000 )	bd1[id].outputs[cursor] |=  0x0010;
		else                    	    					bd1[id].outputs[cursor] &= ~0x0010;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0001 )	bd1[id].outputs[cursor] |=  0x0020;
		else                        						bd1[id].outputs[cursor] &= ~0x0020;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x8000 )	bd1[id].outputs[cursor] |=  0x0040;
		else                        						bd1[id].outputs[cursor] &= ~0x0040;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x4000 )	bd1[id].outputs[cursor] |=  0x0080;
		else                		        				bd1[id].outputs[cursor] &= ~0x0080;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0008 )	bd1[id].outputs[cursor] |=  0x0100;
		else        		                				bd1[id].outputs[cursor] &= ~0x0100;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0400 )	bd1[id].outputs[cursor] |=  0x0200;
		else                        						bd1[id].outputs[cursor] &= ~0x0200;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0010 )	bd1[id].outputs[cursor] |=  0x0400;
		else                        						bd1[id].outputs[cursor] &= ~0x0400;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0040 )	bd1[id].outputs[cursor] |=  0x0800;
		else                        						bd1[id].outputs[cursor] &= ~0x0800;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0080 )	bd1[id].outputs[cursor] |=  0x1000;
		else                        						bd1[id].outputs[cursor] &= ~0x1000;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x0800 )	bd1[id].outputs[cursor] |=  0x2000;
		else                        						bd1[id].outputs[cursor] &= ~0x2000;
		if ( BFM_BD1_get_segments(id)[cursor] & 0x1000 )	bd1[id].outputs[cursor] |=  0x4000;
		else                        						bd1[id].outputs[cursor] &= ~0x4000;
		//Flashing ? Set an unused pin as a control
		if ( BFM_BD1_get_segments(id)[cursor] & 0x100 )	bd1[id].outputs[cursor] |=  0x40000;
		else                        				bd1[id].outputs[cursor] &= ~0x40000;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////

char  *BFM_BD1_get_string( int id)
{
	return (char *)bd1[id].string;
}

///////////////////////////////////////////////////////////////////////////

void BFM_BD1_shift_data(int id, int data)
{
	bd1[id].data <<= 1;

	if ( !data ) bd1[id].data |= 1;

	if ( ++bd1[id].count >= 8 )
	{
		if ( BFM_BD1_newdata(id, bd1[id].data) )
		{
			bd1[id].changed |= 1;
		}
	//logerror("vfd %3d -> %02X \"%s\"\n", id, bd1[id].data, bd1[id].string);

	bd1[id].count = 0;
	bd1[id].data  = 0;
  }
}

///////////////////////////////////////////////////////////////////////////

int BFM_BD1_newdata(int id, int data)
{
	int change = 0;
	int cursor;

	if ( bd1[id].user_def )
	{
		bd1[id].user_def--;

		bd1[id].user_data <<= 8;
		bd1[id].user_data |= data;

		if ( bd1[id].user_def )
		{
			return 0;
		}

	data = '@';
	change = BD1_setdata(id, bd1[id].user_def, data);
	}
	else
	{
	}

	switch ( data & 0xF0 )
	{
		case 0x80:	// 0x80 - 0x8F Set display blanking

		bd1[id].display_blanking = data & 0x0F;
		change = 1;
		break;

		case 0x90:	// 0x90 - 0x9F Set cursor pos

		bd1[id].cursor_pos = data & 0x0F;

		bd1[id].scroll_active = 0;
		if ( bd1[id].display_mode == 2 )
		{
			if ( bd1[id].cursor_pos >= bd1[id].window_end) bd1[id].scroll_active = 1;
		}
		break;

		case 0xA0:	// 0xA0 - 0xAF Set display mode

		bd1[id].display_mode = data &0x03;
		break;

		case 0xB0:	// 0xB0 - 0xBF Clear display area

		switch ( data & 0x03 )
		{
			case 0x00:	// clr nothing
			break;

			case 0x01:	// clr inside window

			if ( bd1[id].window_size > 0 )
			{
				memset( bd1[id].string+bd1[id].window_start, ' ',bd1[id].window_size );
			}
			break;

			case 0x02:	// clr outside window

			if ( bd1[id].window_size > 0 )
			{
				if ( bd1[id].window_start > 0 )
				{
					memset( bd1[id].string, ' ', bd1[id].window_start);
					for (cursor = 0; cursor < bd1[id].window_start; cursor++)
					{
						bd1[id].segments[cursor] = 0x0000;
					}
				}

				if (bd1[id].window_end < 15 )
				{
					memset( bd1[id].string+bd1[id].window_end, ' ', 15-bd1[id].window_end);
					for (cursor = bd1[id].window_end; cursor < 15-bd1[id].window_end; cursor++)
					{
						bd1[id].segments[cursor] = 0x0000;
					}

				}
			}
			case 0x03:	// clr entire display

			memset(bd1[id].string, ' ' , 16);
			for (cursor = 0; cursor < 16; cursor++)
			{
				bd1[id].segments[cursor] = 0x0000;
			}
			break;
		}
		change = 1;
		break;

		case 0xC0:	// 0xC0 - 0xCF Set flash rate

		bd1[id].flash_rate = data & 0x0F;
		break;

		case 0xD0:	// 0xD0 - 0xDF Set Flash control

		bd1[id].flash_control = data & 0x03;
		break;

		case 0xE0:	// 0xE0 - 0xEF Set window start pos

		bd1[id].window_start = data &0x0F;
		bd1[id].window_size  = (bd1[id].window_end - bd1[id].window_start)+1;
		break;

		case 0xF0:	// 0xF0 - 0xFF Set window end pos

		bd1[id].window_end   = data &0x0F;
		bd1[id].window_size  = (bd1[id].window_end - bd1[id].window_start)+1;

		bd1[id].scroll_active = 0;
		if ( bd1[id].display_mode == 2 )
		{
			if ( bd1[id].cursor_pos >= bd1[id].window_end)
			{
				bd1[id].scroll_active = 1;
				bd1[id].cursor_pos    = bd1[id].window_end;
			}
		}
		break;

		default:	// normal character

		change = BD1_setdata(id, BD1charset[data & 0x3F], data);
		break;
	}
	return change;
}

///////////////////////////////////////////////////////////////////////////

static void ScrollLeft(int id)
{
	int i = bd1[id].window_start;

	while ( i < bd1[id].window_end )
	{
		bd1[id].string[ i ] = bd1[id].string[ i+1 ];
		bd1[id].segments[i] = bd1[id].segments[i+1];
		i++;
	}
}

///////////////////////////////////////////////////////////////////////////

static int BD1_setdata(int id, int segdata, int data)
{
	int change = 0, move = 0;

	switch ( data )
	{
		case 0x25:	// flash
		move++;
		break;

		case 0x26:  // undefined
		break;

		case 0x2C:  // semicolon
		case 0x2E:  // decimal point

		bd1[id].segments[bd1[id].pcursor_pos] |= (1<<12);
		change++;
		break;

		case 0x3B:	// dummy char
		move++;
		break;

		case 0x3A:
		bd1[id].user_def = 2;
		break;

		default:
		move   = 1;
		change = 1;
	}

	if ( move )
	{
		int mode = bd1[id].display_mode;

		bd1[id].pcursor_pos = bd1[id].cursor_pos;

		if ( bd1[id].window_size <= 0 || (bd1[id].window_size > 16))
			{ // no window selected default to rotate mode
	  			if ( mode == 2 )      mode = 0;
				else if ( mode == 3 ) mode = 1;
				//mode &= -2;
	}

	switch ( mode )
	{
		case 0: // rotate left

		bd1[id].cursor_pos &= 0x0F;

		if ( change )
		{
			bd1[id].string[bd1[id].cursor_pos]   = BD1ASCII[data];
			bd1[id].segments[bd1[id].cursor_pos] = segdata;
		}
		bd1[id].cursor_pos++;
		if ( bd1[id].cursor_pos >= 16 ) bd1[id].cursor_pos = 0;
		break;


		case 1: // Rotate right

		bd1[id].cursor_pos &= 0x0F;

		if ( change )
		{
			bd1[id].string[bd1[id].cursor_pos]   = BD1ASCII[data];
			bd1[id].segments[bd1[id].cursor_pos] = segdata;
		}
		bd1[id].cursor_pos--;
		if ( bd1[id].cursor_pos < 0  ) bd1[id].cursor_pos = 15;
		break;

		case 2: // Scroll left

		if ( bd1[id].cursor_pos < bd1[id].window_end )
		{
			bd1[id].scroll_active = 0;
			if ( change )
			{
				bd1[id].string[bd1[id].cursor_pos]   = BD1ASCII[data];
				bd1[id].segments[bd1[id].cursor_pos] = segdata;
			}
			if ( move ) bd1[id].cursor_pos++;
		}
		else
		{
			if ( move )
			{
				if   ( bd1[id].scroll_active ) ScrollLeft(id);
				else                            bd1[id].scroll_active = 1;
			}

			if ( change )
			{
				bd1[id].string[bd1[id].window_end]   = BD1ASCII[data];
				bd1[id].segments[bd1[id].cursor_pos] = segdata;
		  	}
			else
			{
				bd1[id].string[bd1[id].window_end]   = ' ';
				bd1[id].segments[bd1[id].cursor_pos] = 0;
			}
		}
		break;

		case 3: // Scroll right

		if ( bd1[id].cursor_pos > bd1[id].window_start )
			{
				if ( change )
				{
					bd1[id].string[bd1[id].cursor_pos]   = BD1ASCII[data];
					bd1[id].segments[bd1[id].cursor_pos] = segdata;
				}
				bd1[id].cursor_pos--;
			}
			else
			{
				int i = bd1[id].window_end;

				while ( i > bd1[id].window_start )
				{
					bd1[id].string[ i ] = bd1[id].string[ i-1 ];
					bd1[id].segments[i] = bd1[id].segments[i-1];
					i--;
				}

				if ( change )
				{
					bd1[id].string[bd1[id].window_start]   = BD1ASCII[data];
					bd1[id].segments[bd1[id].window_start] = segdata;
				}
			}
			break;
		}
	}
	return change;
}

void BFM_BD1_draw(int id)
{
	int cursor;
	BFM_BD1_set_outputs(id);

	for (cursor = 0; cursor < 16; cursor++)
	{
		output_set_indexed_value("vfd", (id*16)+cursor, BFM_BD1_get_outputs(id)[cursor]);

		if (BFM_BD1_get_outputs(id)[cursor] & 0x40000)
		{
			//activate flashing (unimplemented, just toggle on and off)
		}
		else
		{
			//deactivate flashing (unimplemented)
		}
	}
}
