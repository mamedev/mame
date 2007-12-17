/**********************************************************************

    Rockwell 10937/10957 interface and emulation by J.Wallace
    OKI MSC1937 is a clone of this chip

**********************************************************************/

#include "driver.h"
#include "roc10937.h"

static struct
{
	UINT8	type,				// type of alpha display

			reversed,			// Allows for the data being written from right to left, not left to right.

			changed,			// flag <>0, if contents are changed
			window_start,		// display window start pos 0-15
			window_end,			// display window end   pos 0-15
			window_size;		// window  size

	INT8	pcursor_pos,		// previous cursor pos
			cursor_pos;			// current cursor pos

	UINT16  brightness;			// display brightness level 0-31 (31=MAX)

	UINT8	string[18];			// text buffer
	UINT32  segments[16],		// segments
			outputs[16];		// standardised outputs

	UINT8	count,				// bit counter
			data;				// receive register

} roc10937[MAX_ROCK_ALPHAS];

//
// Rockwell 10937 charset to ASCII conversion table
//
static const char roc10937ASCII[]=
//0123456789ABCDEF0123456789ABC DEF01 23456789ABCDEF0123456789ABCDEF
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+;-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+;-./0123456789&%<=>?"\
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ ?\"#$%%'()*+;-./0123456789&%<=>?";

/*
   Rockwell 10937 16 segment charset lookup table
     0     1
    ---- ----
   |\   |   /|
 7 | \F |8 /9| 2
   |  \ | /  |
    -E-- --A-
   |  / | \  |
 6 | /D |C \B| 3
   |/   |   \|
    ---- ----  .11
      5    4   ,10

In 14 segment mode, 0 represents the whole top line,
and 5 the bottom line, allowing both modes to share
a charset.

Note that, although we call this a 16 segment display,
we actually have 18 segments, including the semicolon portions.
This means our segment maths needs to be more than 16-bit to work!
*/

static UINT32 roc10937charset[]=
{           // 11 10 FEDC BA98 7654 3210
	0x0507F, //  0  0 0101 0000 0111 1111 @.
	0x044CF, //  0  0 0100 0100 1100 1111 A.
	0x0153F, //  0  0 0001 0101 0011 1111 B.
	0x000F3, //  0  0 0000 0000 1111 0011 C.
	0x0113F, //  0  0 0001 0001 0011 1111 D.
	0x040F3, //  0  0 0100 0000 1111 0011 E.
	0x040C3, //  0  0 0100 0000 1100 0011 F.
	0x004FB, //  0  0 0000 0100 1111 1011 G.
	0x044CC, //  0  0 0100 0100 1100 1100 H.
	0x01133, //  0  0 0001 0001 0011 0011 I.
	0x0007C, //  0  0 0000 0000 0111 1100 J.
	0x04AC0, //  0  0 0100 1010 1100 0000 K.
	0x000F0, //  0  0 0000 0000 1111 0000 L.
	0x082CC, //  0  0 1000 0010 1100 1100 M.
	0x088CC, //  0  0 1000 1000 1100 1100 N.
	0x000FF, //  0  0 0000 0000 1111 1111 O.
	0x044C7, //  0  0 0100 0100 1100 0111 P.
	0x008FF, //  0  0 0000 1000 1111 1111 Q.
	0x04CC7, //  0  0 0100 1100 1100 0111 R.
	0x044BB, //  0  0 0100 0100 1011 1011 S.
	0x01103, //  0  0 0001 0001 0000 0011 T.
	0x000FC, //  0  0 0000 0000 1111 1100 U.
	0x022C0, //  0  0 0010 0010 1100 0000 V.
	0x028CC, //  0  0 0010 1000 1100 1100 W.
	0x0AA00, //  0  0 1010 1010 0000 0000 X.
	0x09200, //  0  0 1001 0010 0000 0000 Y.
	0x02233, //  0  0 0010 0010 0011 0011 Z.
	0x000E1, //  0  0 0000 0000 1110 0001 [.
	0x08800, //  0  0 1000 1000 0000 0000 \.
	0x0001E, //  0  0 0000 0000 0001 1110 ].
	0x02800, //  0  0 0010 1000 0000 0000 ^.
	0x00030, //  0  0 0000 0000 0011 0000 _.
	0x00000, //  0  0 0000 0000 0000 0000 dummy.
	0x08121, //  0  0 1000 0001 0010 0001 Unknown symbol.
	0x00180, //  0  0 0000 0001 1000 0000 ".
	0x0553C, //  0  0 0101 0101 0011 1100 #.
	0x055BB, //  0  0 0101 0101 1011 1011 $.
	0x07799, //  0  0 0111 0111 1001 1001 %.
	0x0C979, //  0  0 1100 1001 0111 1001 &.
	0x00200, //  0  0 0000 0010 0000 0000 '.
	0x00A00, //  0  0 0000 1010 0000 0000 (.
	0x0A050, //  0  0 1010 0000 0000 0000 ).
	0x0FF00, //  0  0 1111 1111 0000 0000 *.
	0x05500, //  0  0 0101 0101 0000 0000 +.
	0x30000, //  1  1 0000 0000 0000 0000 ;.
	0x04400, //  0  0 0100 0100 0000 0000 --.
	0x20000, //  1  0 0000 0000 0000 0000 . .
	0x02200, //  0  0 0010 0010 0000 0000 /.
	0x022FF, //  0  0 0010 0010 1111 1111 0.
	0x01100, //  0  0 0001 0001 0000 0000 1.
	0x04477, //  0  0 0100 0100 0111 0111 2.
	0x0443F, //  0  0 0100 0100 0011 1111 3.
	0x0448C, //  0  0 0100 0100 1000 1100 4.
	0x044BB, //  0  0 0100 0100 1011 1011 5.
	0x044FB, //  0  0 0100 0100 1111 1011 6.
	0x0000E, //  0  0 0000 0000 0000 1110 7.
	0x044FF, //  0  0 0100 0100 1111 1111 8.
	0x044BF, //  0  0 0100 0100 1011 1111 9.
	0x00021, //  0  0 0000 0000 0010 0001 -
	         //                           -.
	0x02001, //  0  0 0010 0000 0000 0001 -
			 //                           /.
	0x02430, //  0  0 0010 0100 0011 0000 <.
	0x04430, //  0  0 0100 0100 0011 0000 =.
	0x08830, //  0  0 1000 1000 0011 0000 >.
	0x01407, //  0  0 0001 0100 0000 0111 ?.
};

static const int roc10937poslut[]=
{
	1,//0
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	0//15
};

///////////////////////////////////////////////////////////////////////////

void ROC10937_init(int id, int type,int reversed)
{
	assert_always((id >= 0) && (id < MAX_ROCK_ALPHAS), "roc10937_init called on an invalid display ID!");

	memset( &roc10937[id], 0, sizeof(roc10937[0]));

	roc10937[id].type = type;
	roc10937[id].reversed = reversed;
	ROC10937_reset(id);
}

///////////////////////////////////////////////////////////////////////////

void ROC10937_reset(int id)
{
	roc10937[id].window_end  = 15;
	roc10937[id].window_size = (roc10937[id].window_end - roc10937[id].window_start)+1;
	memset(roc10937[id].string, ' ', 16);

	roc10937[id].brightness = 31;
	roc10937[id].count      = 0;

	roc10937[id].changed |= 1;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *ROC10937_get_segments(int id)
{
	return roc10937[id].segments;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *ROC10937_get_outputs(int id)
{
	return roc10937[id].outputs;
}

///////////////////////////////////////////////////////////////////////////

UINT32 *ROC10937_set_outputs(int id)
{
	int cursor,val;
	for (cursor = 0; cursor < 16; cursor++)
	{
		if (!roc10937[id].reversed)//Output to the screen is naturally backwards, so we need to invert it
		{
			val = 15-cursor;
		}
		else
		{
			//If the controller is reversed, things look normal.
			val = cursor;
		}

		if ( ROC10937_get_segments(id)[val] & 0x0001 )	roc10937[id].outputs[cursor] |=  0x0001;
		else                        					roc10937[id].outputs[cursor] &= ~0x0001;
		if ( ROC10937_get_segments(id)[val] & 0x0002 )	roc10937[id].outputs[cursor] |=  0x0002;
		else                        					roc10937[id].outputs[cursor] &= ~0x0002;
		if ( ROC10937_get_segments(id)[val] & 0x0004 )	roc10937[id].outputs[cursor] |=  0x0004;
		else  	    	                  				roc10937[id].outputs[cursor] &= ~0x0004;
		if ( ROC10937_get_segments(id)[val] & 0x0008 )	roc10937[id].outputs[cursor] |=  0x0008;
		else    	                    				roc10937[id].outputs[cursor] &= ~0x0008;
		if ( ROC10937_get_segments(id)[val] & 0x0010 )	roc10937[id].outputs[cursor] |=  0x0010;
		else            	            				roc10937[id].outputs[cursor] &= ~0x0010;
		if ( ROC10937_get_segments(id)[val] & 0x0020 )	roc10937[id].outputs[cursor] |=  0x0020;
		else                	        				roc10937[id].outputs[cursor] &= ~0x0020;
		if ( ROC10937_get_segments(id)[val] & 0x0040 )	roc10937[id].outputs[cursor] |=  0x0040;
		else                    	    				roc10937[id].outputs[cursor] &= ~0x0040;
		if ( ROC10937_get_segments(id)[val] & 0x0080 )	roc10937[id].outputs[cursor] |=  0x0080;
		else                        					roc10937[id].outputs[cursor] &= ~0x0080;
		if ( ROC10937_get_segments(id)[val] & 0x4000 )	roc10937[id].outputs[cursor] |=  0x0100;
		else                        					roc10937[id].outputs[cursor] &= ~0x0100;
		if ( ROC10937_get_segments(id)[val] & 0x0400 )	roc10937[id].outputs[cursor] |=  0x0200;
		else                        					roc10937[id].outputs[cursor] &= ~0x0200;
		if ( ROC10937_get_segments(id)[val] & 0x0100 )	roc10937[id].outputs[cursor] |=  0x0400;
		else                        					roc10937[id].outputs[cursor] &= ~0x0400;
		if ( ROC10937_get_segments(id)[val] & 0x1000 )	roc10937[id].outputs[cursor] |=  0x0800;
		else                        					roc10937[id].outputs[cursor] &= ~0x0800;
		if ( ROC10937_get_segments(id)[val] & 0x2000 )	roc10937[id].outputs[cursor] |=  0x1000;
		else                  		      				roc10937[id].outputs[cursor] &= ~0x1000;
		if ( ROC10937_get_segments(id)[val] & 0x8000 )	roc10937[id].outputs[cursor] |=  0x2000;
		else                        					roc10937[id].outputs[cursor] &= ~0x2000;
		if ( ROC10937_get_segments(id)[val] & 0x0200 )	roc10937[id].outputs[cursor] |=  0x4000;
		else                        					roc10937[id].outputs[cursor] &= ~0x4000;
		if ( ROC10937_get_segments(id)[val] & 0x0800 )	roc10937[id].outputs[cursor] |=  0x8000;
		else                        					roc10937[id].outputs[cursor] &= ~0x8000;

		if ( ROC10937_get_segments(id)[val] & 0x10000 )	roc10937[id].outputs[cursor] |=  0x10000;
		else                        					roc10937[id].outputs[cursor] &= ~0x10000;
		if ( ROC10937_get_segments(id)[val] & 0x20000 )	roc10937[id].outputs[cursor] |=  0x20000;
		else                        					roc10937[id].outputs[cursor] &= ~0x20000;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////

char  *ROC10937_get_string( int id)
{
	return (char *)roc10937[id].string;
}

///////////////////////////////////////////////////////////////////////////

void ROC10937_shift_data(int id, int data)
{
	roc10937[id].data <<= 1;

	if ( !data ) roc10937[id].data |= 1;

	if ( ++roc10937[id].count >= 8 )
	{
		if ( ROC10937_newdata(id, roc10937[id].data) )
		{
			roc10937[id].changed |= 1;
		}

	roc10937[id].count = 0;
	roc10937[id].data  = 0;
  }
}

///////////////////////////////////////////////////////////////////////////

int ROC10937_newdata(int id, int data)
{
	int change = 0;

	if ( data & 0x80 )
	{ // Control data received
		if ( (data & 0xF0) == 0xA0 ) // 1010 xxxx
		{ // 1010 xxxx Buffer Pointer control
			roc10937[id].cursor_pos = roc10937poslut[data & 0x0F];
		}
		else if ( (data & 0xF0) == 0xC0 ) // 1100 xxxx
		{ // 1100 xxxx Set number of digits
			data &= 0x07;

			if ( data == 0 ) roc10937[id].window_size = 16;
			else             roc10937[id].window_size = data+8;
			roc10937[id].window_start = 0;
			roc10937[id].window_end   = roc10937[id].window_size-1;
		}
		else if ( (data & 0xE0) == 0xE0 ) // 111x xxxx
		{ // 111x xxxx Set duty cycle ( brightness )
			roc10937[id].brightness = (data & 0xF)*2;
			change = 1;
		}
		else if ( (data & 0xE0) == 0x80 ) // 100x ---
		{ // 100x xxxx Test mode
		}
	}
	else
	{ // Display data
		data &= 0x3F;
		change = 1;

		switch ( data )
		{
			case 0x2C: // ;
			roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<17);//.
			roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<18);//,
			break;
			case 0x2E: //
			roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<17);//.
			break;
			case 0x6C: // ;
			roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<17);//.
			if ( roc10937[id].type == ROCKWELL10937)
			{
				roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<18);//,
			}
			break;
			case 0x6E: //
			if ( roc10937[id].type == ROCKWELL10937)
			{
				roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<17);//.
			}
			else
			{
				roc10937[id].segments[roc10937[id].pcursor_pos] |= (1<<18);//,
			}
			break;
			default :
			roc10937[id].pcursor_pos = roc10937[id].cursor_pos;
			roc10937[id].string[ roc10937[id].cursor_pos ] = roc10937ASCII[data];
			roc10937[id].segments[roc10937[id].cursor_pos] = roc10937charset[data & 0x3F];

			roc10937[id].cursor_pos++;
			if (  roc10937[id].cursor_pos > roc10937[id].window_end )
			{
					roc10937[id].cursor_pos = 0;
			}
		break;
		}

	}
	return change;
}

///////////////////////////////////////////////////////////////////////////

static void ROC10937_plot(int id, int power)
{
	int cursor;
	ROC10937_set_outputs(id);

	for (cursor = 0; cursor < 16; cursor++)
	{
		output_set_indexed_value("vfd", (id*16)+cursor, power?ROC10937_get_outputs(id)[cursor]:0x00000);

		if (ROC10937_get_outputs(id)[cursor] & 0x40000)
		{
			//activate flashing (unimplemented, just toggle on and off)
		}
		else
		{
			//deactivate flashing (unimplemented)
		}
	}
}
static void ROC10937_draw(int id, int segs)
{
	int cycle;
	for (cycle = 0; cycle < 32; cycle++)
	{
		if ((roc10937[id].brightness < cycle)||(roc10937[id].brightness == cycle))
		{
			ROC10937_plot(id,1);
		}
		else
		{
			ROC10937_plot(id,0);
		}
	}
}

//Helper functions
void ROC10937_draw_16seg(int id)
{
	ROC10937_draw(id,16);
}
void ROC10937_draw_14seg(int id)
{
	ROC10937_draw(id,14);
}
