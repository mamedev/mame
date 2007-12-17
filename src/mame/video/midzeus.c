/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#include "driver.h"
#include "eminline.h"
#include "includes/midzeus.h"
#include "video/poly.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define WAVERAM_WIDTH	512
#define WAVERAM_HEIGHT	4096



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	const UINT32 *	palbase;
	const UINT32 *	texbase;
	UINT16			transcolor;
	UINT16			texwidth;
	UINT16			color;
	UINT32			alpha;
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT32 *zeusbase;

static poly_manager *poly;

static UINT32 zeuscmd_buffer[20];
static UINT8 zeuscmd_buffer_words;
static UINT8 zeuscmd_buffer_expected_words;
static INT16 zeus_matrix[3][3];
static INT32 zeus_point[3];
static UINT32 *zeus_palbase;
static int zeus_enable_logging;
static UINT32 zeus_blend;
static UINT32 zeus_texdata;
static UINT32 zeus_objdata;

static UINT32 *waveram;

static UINT8 single_step;
static poly_extra_data single_step_extra;
static UINT32 single_step_val1;
static UINT32 single_step_val2;
static INT32 single_step_dx, single_step_dy;
static UINT8 single_step_advance;



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void exit_handler(running_machine *machine);

static void zeus_write_16bit(offs_t offset, UINT16 data, int logit);
static void zeus_write_32bit(offs_t offset, UINT32 data, int logit);
static void zeus_process_write(offs_t offset);
static void zeuscmd_execute(int words);
static UINT8 zeuscmd_length(UINT32 word);



/*************************************
 *
 *  Inlines
 *
 *************************************/

INLINE UINT32 *waveram_ptr_from_addr(UINT32 addr)
{
	return &waveram[((addr & 0x1ff) | ((addr & 0xfff000) >> 3)) * 2];
}


INLINE UINT32 *waveram_pixel_ptr(int bank, int y, int x)
{
	return &waveram[bank * (WAVERAM_HEIGHT * WAVERAM_WIDTH) + y * WAVERAM_WIDTH + (x & ~1)];
}


INLINE void waveram_plot(int y, int x, UINT16 color)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT32 *ptr = waveram_pixel_ptr(1, y, x);
		if (x & 1)
			*ptr = (*ptr & 0x0000ffff) | (color << 16);
		else
			*ptr = (*ptr & 0xffff0000) | color;
	}
}


INLINE void waveram_plot_depth(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT32 *ptr = waveram_pixel_ptr(1, y, x);
		if (x & 1)
		{
			ptr[0] = (ptr[0] & 0x0000ffff) | (color << 16);
			ptr[1] = (ptr[1] & 0x0000ffff) | (depth << 16);
		}
		else
		{
			ptr[0] = (ptr[0] & 0xffff0000) | color;
			ptr[1] = (ptr[1] & 0xffff0000) | depth;
		}
	}
}


INLINE void waveram_plot_check_depth(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT32 *ptr = waveram_pixel_ptr(1, y, x);
		if (x & 1)
		{
			if (depth <= (ptr[1] >> 16))
			{
				ptr[0] = (ptr[0] & 0x0000ffff) | (color << 16);
				ptr[1] = (ptr[1] & 0x0000ffff) | (depth << 16);
			}
		}
		else
		{
			if (depth <= (ptr[1] & 0xffff))
			{
				ptr[0] = (ptr[0] & 0xffff0000) | color;
				ptr[1] = (ptr[1] & 0xffff0000) | depth;
			}
		}
	}
}


INLINE UINT32 *zeus_pixaddr(void)
{
	int bank = (zeusbase[0xb6] >> 31) & 1;
	int x = (zeusbase[0xb4] & 0xff) << 1;
	int y = ((zeusbase[0xb4] >> 15) & 0xffe) | ((zeusbase[0xb4] >> 8) & 0x01);
	return waveram_pixel_ptr(bank, y, x);
}


INLINE UINT8 get_texel_8bit(const UINT32 *base, int y, int x, int width)
{
	if (x >= width)
		return 0;
	base += (y / 2) * (width / 2) + (x / 4) * 2 + (y & 1);
	return *base >> (8 * (x & 3));
}


INLINE UINT8 get_texel_4bit(const UINT32 *base, int y, int x, int width)
{
	if (x >= width * 2)
		return 0;
	base += (y / 2) * (width / 2) + (x / 8) * 2 + (y & 1);
	return (*base >> (4 * (x & 7))) & 0x0f;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( midzeus )
{
	int i;

	/* allocate memory for "wave" RAM */
	waveram = auto_malloc(2 * WAVERAM_WIDTH * WAVERAM_HEIGHT * sizeof(*waveram));
	state_save_register_global_pointer(waveram, 2 * WAVERAM_WIDTH * WAVERAM_HEIGHT);

	/* initialize a 5-5-5 palette */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine, i, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));

	/* initialize polygon engine */
	poly = poly_alloc(1000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	/* we need to cleanup on exit */
	add_exit_callback(machine, exit_handler);
}


static void exit_handler(running_machine *machine)
{
	FILE *f = fopen("waveram.dmp", "w");
	int i;

	for (i = 0; i < 2 * WAVERAM_WIDTH * WAVERAM_HEIGHT; i += 2)
	{
		if (i % 8 == 0) fprintf(f, "%03X%03X: ", (i / 1024), (i / 2) & 0x1ff);
		fprintf(f, " %08X %08X ", waveram[i+0], waveram[i+1]);
		if (i % 8 == 6) fprintf(f, "\n");
	}
	fclose(f);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( midzeus )
{
	int x, y;

	/* normal update case */
	if (!input_code_pressed(KEYCODE_W))
	{
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT32 *src = waveram_pixel_ptr(1, y, 0);
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
			{
				UINT32 pixels = *src;
				dest[x+0] = pixels & 0x7fff;
				dest[x+1] = (pixels >> 16) & 0x7fff;
				src += 2;
			}
		}

		if (single_step)
		{
			int texwshift = ((single_step_val2 >> 6) + single_step_dx) & 7;
			const UINT32 *texbase = &waveram[(single_step_val1 << ((7 + single_step_dy) - texwshift)) & 0x3fffff];
			int texwidth = 512 >> texwshift;
			int texdepth = (single_step_val2 & 0x20) ? 8 : 4;
			int x, y;

			for (y = 0; y < 256; y++)
			{
				UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels + 400;
				for (x = 0; x < 256; x++)
				{
					if (x < texwidth)
						dest[x] = single_step_extra.palbase[(texdepth == 8) ?
									get_texel_8bit(texbase, y, x, texwidth) :
									get_texel_4bit(texbase, y, x, texwidth)];
					else
						dest[x] = 0;
				}
			}

			popmessage("val1=%08X  val2=%08X\nwidth=%d  depth=%d", single_step_val1, single_step_val2, texwidth, texdepth);

			if (input_code_pressed(KEYCODE_UP)) { single_step_dy--; while (input_code_pressed(KEYCODE_UP)) ; }
			if (input_code_pressed(KEYCODE_DOWN)) { single_step_dy++; while (input_code_pressed(KEYCODE_DOWN)) ; }
			if (input_code_pressed(KEYCODE_LEFT)) { single_step_dx--; while (input_code_pressed(KEYCODE_LEFT)) ; }
			if (input_code_pressed(KEYCODE_RIGHT)) { single_step_dx++; while (input_code_pressed(KEYCODE_RIGHT)) ; }
			if (input_code_pressed(KEYCODE_LSHIFT)) { single_step_advance = TRUE; while (input_code_pressed(KEYCODE_LSHIFT)) ; }
		}
	}

	/* waveram drawing case */
	else
	{
		static int yoffs = 0;
		static int width = 256;
		UINT32 *base;

		if (input_code_pressed(KEYCODE_DOWN)) yoffs += input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_UP)) yoffs -= input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_LEFT) && width > 4) { width >>= 1; while (input_code_pressed(KEYCODE_LEFT)) ; }
		if (input_code_pressed(KEYCODE_RIGHT) && width < 512) { width <<= 1; while (input_code_pressed(KEYCODE_RIGHT)) ; }

		if (yoffs < 0) yoffs = 0;
		base = waveram_ptr_from_addr(yoffs << 12);

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 tex = get_texel_8bit(base, y, x, width);
				dest[x] = (tex << 8) | tex;
			}
		}
		popmessage("offs = %06X", yoffs << 12);
	}

	return 0;
}






/*
    Control = $80F6 when doing initial screen clear.
       Writes 256 consecutive values to $B0, must mean autoincrement
       Uses latched value in $B2 (maybe also in $B0)

    Control = $82F0 when doing video RAM test write phase.
       Writes 512 values across, but updates address before each one.

    Control = $84F1 when doing video RAM test read phase.
       Reads 512 values across, but updates address before each one.

    Control = $80F2 when doing screen clear.
       Writes 256 consecutive values to $B0, must mean autoincrement.
       Uses latched value in $B2 (maybe also in $B0)

    Control = $80A0/$8050 when plotting pixels.

    Control = $42F0 when doing wave RAM test write phase.
       Writes 512 values across, but updates address before each one.

    Control = $44F1 when doing wave RAM test read phase.
       Reads 512 values across, but updates address before each one.
*/


/*

    Known commands:

        $00rrxxxx = another type of register set?
         yyyyyyyy

            $0000C040 = set palette to 'yyyyyyyy'

        $01rrxxxx = another type of register set?
         yyyyyyyy

            $01008000 = set object pointer (equivalent to word 2 of command $67)
            $0100C0B0 = set texture data (equivalent to word 1 of command $67)
            $0100C09C
            $0100C0A3
            $0100C0F0
            $0100C040 = set palette base

        $17rrxxxx = 16-bit write 'xxxx' to register 'rr'

            $1752xxxx = write 'xxxx' to something that reflects back through $B2
            $1770xxxx = write 'xxxx' to upper 16 bits of X coordinate
            $1772xxxx = write 'xxxx' to upper 16 bits of Y coordinate
            $1774xxxx = write 'xxxx' to upper 16 bits of Z coordinate

        $18rr0000 = 32-bit write 'xxxxxxxx' to register ('rr' & 0x7f)
         xxxxxxxx

        $1A000000 = pipeline synchronize (guess)

        $1C00cccc = load matrix (abc)(def)(ghi) and point (xyz)
         ffffiiii
         bbbbaaaa
         eeeedddd
         hhhhgggg
         xxxxxxxx
         yyyyyyyy
         zzzzzzzz

        $6700xxxx = render object described at 'bbbbbb' in waveram
         aabbbbbb
         ccddcccc
*/

/*

MK4:

Zeus cmd 18 : 18D60000 500A00FF
Zeus cmd 00 : 0005C0A5 21001125
Zeus cmd 25 : 25000000 007F0018 01000400 B0F60008
Zeus cmd 18 : 18D60000 500A00FF
Zeus cmd 18 : 18CE0000 00FFFFFF
Zeus cmd 1A : 1A000000
Zeus cmd 18 : 187E0000 0000FFFF
Zeus cmd 1A : 1A000000
Zeus cmd 18 : 18CE0000 00FFFFFF
Zeus cmd 1A : 1A000000
Zeus cmd 18 : 187E0000 FF980000
Zeus cmd 00 : 0080C0A5 3F00102F
Zeus cmd 17 : 17681370
Zeus cmd 17 : 176940C4
Zeus cmd 1C : ( 4000 0000 0000 ) ( 0000 4000 0000 ) ( 0000 0000 4000 )  -152.00   -50.00  1.25000
Zeus cmd 00 : 0000C040 0318E132 -> palette @ (18E,132)
Zeus cmd 67 : 67008000 0919F1C4 151 01FA4C -> render @ (19F,1C4)
Zeus cmd 1C : ( 4000 0000 0000 ) ( 0000 4000 0000 ) ( 0000 0000 4000 )  -135.00   -50.00  1.25000
Zeus cmd 67 : 67008000 0919F1C4 151 01FA4C
Zeus cmd 1C : ( 4000 0000 0000 ) ( 0000 4000 0000 ) ( 0000 0000 4000 )  -118.00   -50.00  1.25000
Zeus cmd 67 : 67008000 0919F1C4 151 01F988
Zeus cmd 1C : ( 4000 0000 0000 ) ( 0000 4000 0000 ) ( 0000 0000 4000 )  -101.00   -50.00  1.25000

19F1BA:  19DC0000 00000000  19840000 00F01605  25810000 21010000  FFD00000 00000000  FFD00038 000E0000  00080038 0E0E0000  00080000 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1C4:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD0FFFC 00000000  FFD0003C 00100000  0008003C 0E100000  0008FFFC 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1CE:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD0FFFC 00000000  FFD00028 000B0000  00080028 0E0B0000  0008FFFC 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1D8:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD00000 00000000  FFD00034 000D0000  00080034 0E0D0000  00080000 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1E2:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD00000 00000000  FFD00028 000A0000  000C0028 0F0A0000  000C0000 0F000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1EC:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD00000 00000000  FFD00014 00050000  00080014 0E050000  00080000 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
19F1F6:  19DC0000 00000000  19840000 00F01605  25810000 21010000  FFD0FFFC 00000000  FFD00034 000E0000  00080034 0E0E0000  0008FFFC 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0000:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFCC0000 00000000  FFCC0038 000E0000  000C0038 100E0000  000C0000 10000000  30000000 30000000  30000000 30000000  08000000 00000000
1A000A:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFCC0000 00000000  FFCC0034 000D0000  000C0034 100D0000  000C0000 10000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0014:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFCC0000 00000000  FFCC002C 000B0000  0008002C 0F0B0000  00080000 0F000000  30000000 30000000  30000000 30000000  08000000 00000000
1A001E:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFCC0000 00000000  FFCC002C 000B0000  000C002C 100B0000  000C0000 10000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0028:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD00000 00000000  FFD0002C 000B0000  000C002C 0F0B0000  000C0000 0F000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0032:  19DC0000 00000000  19840000 00F01605  25800000 21010000  FFD00000 00000000  FFD00018 00060000  00080018 0E060000  00080000 0E000000  30000000 30000000  30000000 30000000  08000000 00000000
1A003C:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF9CFF64 00000000  FF9C0084 00480000  005C0084 30480000  005CFF64 30000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0046:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF80FF48 00000000  FF80009C 00550000  0070009C 3C550000  0070FF48 3C000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0050:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF68FF30 00000000  FF6800B0 00600000  008800B0 48600000  0088FF30 48000000  30000000 30000000  30000000 30000000  08000000 00000000
1A005A:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF54FF20 00000000  FF5400C4 00690000  00A000C4 53690000  00A0FF20 53000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0064:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF44FF10 00000000  FF4400D4 00710000  00B000D4 5B710000  00B0FF10 5B000000  30000000 30000000  30000000 30000000  08000000 00000000
1A006E:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF3CFF08 00000000  FF3C00E4 00770000  00B400E4 5E770000  00B4FF08 5E000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0078:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF38FEFC 00000000  FF3800E4 007A0000  00C000E4 627A0000  00C0FEFC 62000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0082:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF28FEFC 00000000  FF2800EC 007C0000  00C000EC 667C0000  00C0FEFC 66000000  30000000 30000000  30000000 30000000  08000000 00000000
1A008C:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF60FF28 00000000  FF6000C0 00660000  009C00C0 4F660000  009CFF28 4F000000  30000000 30000000  30000000 30000000  08000000 00000000
1A0096:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF80FF48 00000000  FF80009C 00550000  0078009C 3E550000  0078FF48 3E000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00A0:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FF94FF5C 00000000  FF940088 004B0000  00640088 344B0000  0064FF5C 34000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00AA:  19DC0000 40B68800  19840000 00001604  25810000 21010000  FFA4FF6C 00000000  FFA4007C 00440000  0054007C 2C440000  0054FF6C 2C000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00B4:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FFACFF78 00000000  FFAC0070 003E0000  00480070 273E0000  0048FF78 27000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00BE:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FFB4FF7C 00000000  FFB4006C 003C0000  0044006C 243C0000  0044FF7C 24000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00C8:  19DC0000 00000000  19840000 00001605  25800000 21000000  FF42FF02 00000000  FF4200FE 007F0000  00BE00FE 5F7F0000  00BEFF02 5F000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00D2:  19DC0000 00000000  19840000 00001605  25800000 21010000  FFB0FF70 00000000  FFB0007C 00430000  0054007C 29430000  0054FF70 29000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00DC:  19DC0000 40B68800  19840000 00001604  25800000 21010000  FA10F810 00000000  FA1007F0 007F0000  05F007F0 5F7F0000  05F0F810 5F000000  30000000 30000000  30000000 30000000  08000000 00000000
1A00E6:  0C02C0F0 0F18E1BE
         19DC0000 4B23CB00  19840000 00001605  252978EF 200C0000  04480000 00000000  043D0000 0000001E  043DFFEB 00000015  043DFFEB 00000015  00040000 0A331400  0743178C 0743178C
1A00F0:                                        251178D7 200C0000  04480000 00000000  043DFFEB 00000015  043DFFE2 00000000  043DFFE2 00000000  00040000 0743178C  0003175D 0003175D
                                               25EF78D7 200C0000  04480000 00000000  043DFFE2 00000000  043DFFEB 0000FFEB  043DFFEB 0000FFEB  00040000 0003175D  38C3178C 38C3178C
                                               25D778EF 200C0000  04480000 00000000  043DFFEB 0000FFEB  043D0000 0000FFE2  043D0000 0000FFE2  00040000 38C3178C  35D31400 35D31400
                                               25D77811 200C0000  04480000 00000000  043D0000 0000FFE2  043D0015 0000FFEB  043D0015 0000FFEB  00040000 35D31400  38C31474 38C31474
1A010C:                                        25EF7829 200C0000  04480000 00000000  043D0015 0000FFEB  043D001E 00000000  043D001E 00000000  00040000 38C31474  000314A3 000314A3
                                               25117829 200C0000  04480000 00000000  043D001E 00000000  043D0015 00000015  043D0015 00000015  00040000 000314A3  07431474 07431474
                                               25297811 200C0000  04480000 00000000  043D0015 00000015  043D0000 0000001E  043D0000 0000001E  00040000 07431474  0A331400 0A331400
                                               0C018000 381A0122
                                               252186F3 200C0000  F64A0000 00000000  F652FFEB 00000015  F6520000 0000001E  F6520000 0000001E  000C0000 06ECD393  09ACD3FF 09ACD3FF
                                               250D86DF 200C0000  F64A0000 00000000  F652FFE2 00000000  F652FFEB 00000015  F652FFEB 00000015  000C0000 001CD366  06DCD392 06DCD392
1A0130:                                        25F386DF 200C0000  F64A0000 00000000  F652FFEB 0000FFEB  F652FFE2 00000000  F652FFE2 00000000  000C0000 393CD392  3FFCD366 3FFCD366
                                               25DF86F3 200C0000  F64A0000 00000000  F6520000 0000FFE2  F652FFEB 0000FFEB  F652FFEB 0000FFEB  000C0000 366CD3FF  392CD393 392CD393
                                               25DF860D 200C0000  F64A0000 00000000  F6520015 0000FFEB  F6520000 0000FFE2  F6520000 0000FFE2  000C0000 392CD06D  366CD001 366CD001
                                               25F38621 200C0000  F64A0000 00000000  F652001E 00000000  F6520015 0000FFEB  F6520015 0000FFEB  000C0000 3FFCD09A  393CD06E 393CD06E
1A014C:                                        250D8621 200C0000  F64A0000 00000000  F6520015 00000015  F652001E 00000000  F652001E 00000000  000C0000 06DCD06E  001CD09A 001CD09A
                                               2521860D 200C0000  F6520000 0000001E  F6520015 00000015  F64A0000 00000000  F64A0000 00000000  09ACD001 06ECD06D  000C0000 000C0000
                                               0C018000 381A015B
                                               25D53969 200C0000  0429001D 0000FFE3  04290029 00000000  043D001E 00000000  043D0015 0000FFEB  356110B3 3FA110F7  3EB2D8B3 3902D88D
                                               252B3969 200C0000  04290029 00000000  0429001D 0000001D  043D0015 00000015  043D001E 00000000  006110F7 0AA110B3  0702D88D 0152D8B3
                                               2569392B 200C0000  0429001D 0000001D  04290000 00000029  043D0000 0000001E  043D0015 00000015  0B3110AA 0F711006  0B32D815 08D2D870
1A0170:                                        256939D5 200C0000  04290000 00000029  0429FFE3 0000001D  043DFFEB 00000015  043D0000 0000001E  0F7113FA 0B311356  08D2DB90 0B32DBEB
                                               252B3997 200C0000  0429FFE3 0000001D  0429FFD7 00000000  043DFFE2 00000000  043DFFEB 00000015  0AA1134D 00611309  0152DB4D 0702DB73
                                               25D53997 200C0000  0429FFD7 00000000  0429FFE3 0000FFE3  043DFFEB 0000FFEB  043DFFE2 00000000  3FA11309 3561134D  3902DB73 3EB2DB4D
                                               259739D5 200C0000  0429FFE3 0000FFE3  04290000 0000FFD7  043D0000 0000FFE2  043DFFEB 0000FFEB  34D11356 309113FA  34D2DBEB 3732DB90
1A018C:                                        2597392B 200C0000  04290000 0000FFD7  0429001D 0000FFE3  043D0015 0000FFEB  043D0000 0000FFE2  30911006 34D110AA  3732D870 34D2D815
                                               0C018000 381A0194
1A0194:                                        252BC769 200C0000  F666001D 0000001D  F6660029 00000000  F652001E 00000000  F6520015 00000015  0AAEF0B3 006EF0F7  018D20B0 06CD208D
                                               25D5C769 200C0000  F6660029 00000000  F666001D 0000FFE3  F6520015 0000FFEB  F652001E 00000000  3FAEF0F7 356EF0B3  394D208D 3E8D20B0
                                               2597C72B 200C0000  F666001D 0000FFE3  F6660000 0000FFD7  F6520000 0000FFE2  F6520015 0000FFEB  34DEF0AA 309EF006  350D2018 373D206C
                                               2597C7D5 200C0000  F6660000 0000FFD7  F666FFE3 0000FFE3  F652FFEB 0000FFEB  F6520000 0000FFE2  309EF3FA 34DEF356  373D2394 350D23E8
1A01B0:                                        25D5C797 200C0000  F666FFE3 0000FFE3  F666FFD7 00000000  F652FFE2 00000000  F652FFEB 0000FFEB  356EF34D 3FAEF309  3E8D2350 394D2373
                                               252BC797 200C0000  F666FFD7 00000000  F666FFE3 0000001D  F652FFEB 00000015  F652FFE2 00000000  006EF309 0AAEF34D  06CD2373 018D2350
                                               2569C7D5 200C0000  F666FFE3 0000001D  F6660000 00000029  F6520000 0000001E  F652FFEB 00000015  0B3EF356 0F7EF3FA  0B0D23E8 08DD2394
                                               2569C72B 200C0000  F6660000 00000029  F666001D 0000001D  F6520015 00000015  F6520000 0000001E  0F7EF006 0B3EF0AA  08DD206C 0B0D2018
1A01CC:                                        0C018000 311A01CD
                                               25300076 200C0000  F6660029 00000000  F666001D 0000001D  0429001D 0000001D  04290029 00000000  00FF24FA 0A6F24BB  0A60DCBB 00F0DCFA
1A01D4:                                        25760030 200C0000  F666001D 0000001D  F6660000 00000029  04290000 00000029  0429001D 0000001D  0BBF24A6 0FAF240F  0FA0DC0F 0BB0DCA6
                                               257600D0 200C0000  F6660000 00000029  F666FFE3 0000001D  0429FFE3 0000001D  04290000 00000029  0FAF27F1 0BBF275A  0BB0DF5A 0FA0DFF1
                                               2530008A 200C0000  F666FFE3 0000001D  F666FFD7 00000000  0429FFD7 00000000  0429FFE3 0000001D  0A6F2745 00FF2706  00F0DF06 0A60DF45
                                               25D0008A 200C0000  F666FFD7 00000000  F666FFE3 0000FFE3  0429FFE3 0000FFE3  0429FFD7 00000000  3F1F2706 35AF2745  35A0DF45 3F10DF06
1A01F0:                                        258A00D0 200C0000  F666FFE3 0000FFE3  F6660000 0000FFD7  04290000 0000FFD7  0429FFE3 0000FFE3  345F275A 306F27F1  3060DFF1 3450DF5A
                                               258A0030 200C0000  F6660000 0000FFD7  F666001D 0000FFE3  0429001D 0000FFE3  04290000 0000FFD7  306F240F 345F24A6  3450DCA6 3060DC0F
                                               0C018000 071A1000  00000000 00000000
1A1000:                                        25D00076 200C0000  F666001D 0000FFE3  F6660029 00000000  04290029 00000000  0429001D 0000FFE3  35AF24BB 3F1F24FA  3F10DCFA 35A0DCBB
                                               08000000 00000000
*/

/*

Note this sequence:
Zeus cmd 17 : 1757500A
Zeus cmd 17 : 175600FF
Zeus cmd 17 : 176F0032
Zeus cmd 17 : 176E0031
Zeus cmd 17 : 176D003B
Zeus cmd 17 : 176C002F

Appears to do the same thing as:
Zeus cmd 18 : 18D60000 500A00FF
Zeus cmd 18 : 18EE0000 00320031
Zeus cmd 18 : 18EC0000 003B002F



Also this sequence:
Zeus cmd 17 : 176940C4
Zeus cmd 17 : 17681370

Seems to have the same purpose as this part of the command stream:
    19E80000 40C41370



Invasion startup:

00CC86:zeus16_w(81) = 0000 [00000000]
00CC86:zeus16_w(80) = 0000 [00000000]
00CC86:zeus16_w(4B) = 018F [018F0000]
00CC86:zeus16_w(4A) = 01FE [018F01FE]
00CC86:zeus16_w(49) = 018F [018F0000]
00CC86:zeus16_w(48) = 01FE [018F01FE]
00CC86:zeus16_w(CE) = 7620 [00007620]
00CC86:zeus16_w(CD) = 0000 [00000000]
00CC86:zeus16_w(CC) = 0000 [00000000]
00CC86:zeus16_w(CB) = 0116 [01160000]
00CC86:zeus16_w(CA) = 0107 [01160107]
00CC86:zeus16_w(C9) = 0103 [01030000]
00CC86:zeus16_w(C8) = 00FF [010300FF]
00CC86:zeus16_w(C7) = 0211 [02110000]
00CC86:zeus16_w(C6) = 007F [0211007F]
00CC86:zeus16_w(C5) = 000C [000C0000]
00CC86:zeus16_w(C4) = 004C [000C004C]
00CC86:zeus16_w(C3) = 001F [001F0000]
00CC86:zeus16_w(C2) = E591 [001FE591]
00CC86:zeus16_w(C1) = 801F [801F0000]
00CC86:zeus16_w(C0) = 2500 [801F2500]
00CC86:zeus16_w(86) = 8003 [00008003]
00CC86:zeus16_w(80) = 0C05 [00000C05]
00CC86:zeus16_w(8F) = 0206 [02060000]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0226 [02260000]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8B) = 0000 [00000000]
00CC86:zeus16_w(8A) = FAFA [0000FAFA]
00CC86:zeus16_w(89) = 0000 [00000000]
00CC86:zeus16_w(88) = FAFA [0000FAFA]
00CC86:zeus16_w(80) = 0001 [00000001]
00CC86:zeus16_w(8F) = 0207 [02070000]
00CC86:zeus16_w(8E) = BFFF [0207BFFF]
00CC86:zeus16_w(8F) = 0206 [0206BFFF]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0227 [02270000]
00CC86:zeus16_w(8C) = BFFF [0227BFFF]
00CC86:zeus16_w(8D) = 0226 [0226BFFF]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8F) = 0207 [02070000]
00CC86:zeus16_w(8E) = F020 [0207F020]
00CC86:zeus16_w(8F) = 0206 [0206F020]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0227 [02270000]
00CC86:zeus16_w(8C) = F020 [0227F020]
00CC86:zeus16_w(8D) = 0226 [0226F020]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8F) = 0207 [02070000]
00CC86:zeus16_w(8E) = DFFF [0207DFFF]
00CC86:zeus16_w(8F) = 0206 [0206DFFF]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0227 [02270000]
00CC86:zeus16_w(8C) = DFFF [0227DFFF]
00CC86:zeus16_w(8D) = 0226 [0226DFFF]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8F) = 0207 [02070000]
00CC86:zeus16_w(8E) = DFFF [0207DFFF]
00CC86:zeus16_w(8F) = 0206 [0206DFFF]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0227 [02270000]
00CC86:zeus16_w(8C) = DFFF [0227DFFF]
00CC86:zeus16_w(8D) = 0226 [0226DFFF]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8F) = 0206 [02060000]
00CC86:zeus16_w(8E) = 0000 [02060000]
00CC86:zeus16_w(8D) = 0226 [02260000]
00CC86:zeus16_w(8C) = 0000 [02260000]
00CC86:zeus16_w(8B) = 0000 [0000FAFA]
00CC86:zeus16_w(8A) = D254 [0000D254]
00CC86:zeus16_w(89) = 8FFF [8FFFFAFA]
00CC86:zeus16_w(88) = D794 [8FFFD794]
00CC86:zeus16_w(85) = 0000 [00000000]
00CC86:zeus16_w(84) = 0000 [00000000]
00CC86:zeus16_w(83) = 1F1F [1F1F0000]
00CC86:zeus16_w(82) = 2011 [1F1F2011]
00CC86:zeus16_w(69) = 0003 [00030000]
00CC86:zeus16_w(69) = 0000 [00000000]
00CC86:zeus16_w(68) = 0100 [00000100]
00CC92:zeus16_w(200) = 0100 [00000100]
00CC92:zeus16_w(201) = 0000 [00000100]
00CC92:zeus16_w(202) = 0000 [00000000]
00CC92:zeus16_w(203) = 0000 [00000000]
00CC92:zeus16_w(204) = 00D4 [000000D4]
00CC92:zeus16_w(205) = 0000 [000000D4]
00CC92:zeus16_w(206) = 0000 [00000000]
00CC92:zeus16_w(207) = A000 [A0000000]
00CC92:zeus16_w(208) = 0050 [00000050]
00CC92:zeus16_w(209) = 0000 [00000050]
00CC92:zeus16_w(20A) = 0000 [00000000]
00CC92:zeus16_w(20B) = A000 [A0000000]
00CC92:zeus16_w(20C) = 0703 [00000703]
00CC92:zeus16_w(20D) = 0000 [00000703]
00CC92:zeus16_w(20E) = 0000 [00000000]
00CC92:zeus16_w(20F) = 0000 [00000000]
00CC92:zeus16_w(210) = 0050 [00000050]
00CC92:zeus16_w(211) = 0000 [00000050]
00CC92:zeus16_w(212) = 0000 [00000000]
00CC92:zeus16_w(213) = A000 [A0000000]
00CC92:zeus16_w(214) = 005A [0000005A]
00CC92:zeus16_w(215) = 01C0 [01C0005A]
00CC92:zeus16_w(216) = 0000 [00000000]
00CC92:zeus16_w(217) = B000 [B0000000]
00CC92:zeus16_w(218) = 00D8 [000000D8]
00CC92:zeus16_w(219) = 0000 [000000D8]
00CC92:zeus16_w(21A) = 0000 [00000000]
00CC92:zeus16_w(21B) = 9000 [90000000]
00CC92:zeus16_w(21C) = 0607 [00000607]
00CC92:zeus16_w(21D) = 0000 [00000607]
00CC92:zeus16_w(21E) = 0000 [00000000]
00CC92:zeus16_w(21F) = 0000 [00000000]
00CC92:zeus16_w(220) = C000 [0000C000]
00CC92:zeus16_w(221) = 0008 [0008C000]
00CC92:zeus16_w(222) = 0000 [00000000]
00CC92:zeus16_w(223) = 003F [003F0000]
00CC92:zeus16_w(224) = 006A [0000006A]
00CC92:zeus16_w(225) = 01D0 [01D0006A]
00CC92:zeus16_w(226) = 0025 [00000025]
00CC92:zeus16_w(227) = B000 [B0000025]
00CC92:zeus16_w(228) = 0F00 [00000F00]
00CC92:zeus16_w(229) = 0000 [00000F00]
00CC92:zeus16_w(22A) = 0001 [00000001]
00CC92:zeus16_w(22B) = 0000 [00000001]
00CC92:zeus16_w(22C) = 0E08 [00000E08]
00CC92:zeus16_w(22D) = 0000 [00000E08]
00CC92:zeus16_w(22E) = 0000 [00000000]
00CC92:zeus16_w(22F) = 0000 [00000000]
00CC92:zeus16_w(230) = 00D4 [000000D4]
00CC92:zeus16_w(231) = 0000 [000000D4]
00CC92:zeus16_w(232) = 0000 [00000000]
00CC92:zeus16_w(233) = A000 [A0000000]
00CC92:zeus16_w(234) = 0050 [00000050]
00CC92:zeus16_w(235) = 0000 [00000050]
00CC92:zeus16_w(236) = 0000 [00000000]
00CC92:zeus16_w(237) = A000 [A0000000]
00CC92:zeus16_w(238) = 070E [0000070E]
00CC92:zeus16_w(239) = 0000 [0000070E]
00CC92:zeus16_w(23A) = 0000 [00000000]
00CC92:zeus16_w(23B) = 0000 [00000000]
00CC92:zeus16_w(23C) = 0050 [00000050]
00CC92:zeus16_w(23D) = 0000 [00000050]
00CC92:zeus16_w(23E) = 0000 [00000000]
00CC92:zeus16_w(23F) = A000 [A0000000]
00CC92:zeus16_w(240) = 005A [0000005A]
00CC92:zeus16_w(241) = 01C0 [01C0005A]
00CC92:zeus16_w(242) = 0000 [00000000]
00CC92:zeus16_w(243) = B000 [B0000000]
00CC92:zeus16_w(244) = 00D8 [000000D8]
00CC92:zeus16_w(245) = 0000 [000000D8]
00CC92:zeus16_w(246) = 0000 [00000000]
00CC92:zeus16_w(247) = 8CC0 [8CC00000]
00CC92:zeus16_w(248) = 0612 [00000612]
00CC92:zeus16_w(249) = 0000 [00000612]
00CC92:zeus16_w(24A) = 0000 [00000000]
00CC92:zeus16_w(24B) = 0000 [00000000]
00CC92:zeus16_w(24C) = D000 [0000D000]
00CC92:zeus16_w(24D) = 0008 [0008D000]
00CC92:zeus16_w(24E) = 8000 [00008000]
00CC92:zeus16_w(24F) = 003E [003E8000]
00CC92:zeus16_w(250) = 006A [0000006A]
00CC92:zeus16_w(251) = 01D0 [01D0006A]
00CC92:zeus16_w(252) = 0025 [00000025]
00CC92:zeus16_w(253) = B000 [B0000025]
00CC92:zeus16_w(254) = 0F00 [00000F00]
00CC92:zeus16_w(255) = 0000 [00000F00]
00CC92:zeus16_w(256) = 0001 [00000001]
00CC92:zeus16_w(257) = 0000 [00000001]
00CC92:zeus16_w(258) = 0E13 [00000E13]
00CC92:zeus16_w(259) = 0000 [00000E13]
00CC92:zeus16_w(25A) = 0000 [00000000]
00CC92:zeus16_w(25B) = 0000 [00000000]
00CC92:zeus16_w(25C) = 0F00 [00000F00]
00CC92:zeus16_w(25D) = 05C0 [05C00F00]
00CC92:zeus16_w(25E) = 0000 [00000000]
00CC92:zeus16_w(25F) = B000 [B0000000]
00CC92:zeus16_w(260) = 0F80 [00000F80]
00CC92:zeus16_w(261) = 0400 [04000F80]
00CC92:zeus16_w(262) = 0000 [00000000]
00CC92:zeus16_w(263) = 9000 [90000000]
00CC92:zeus16_w(264) = 0F80 [00000F80]
00CC92:zeus16_w(265) = 0400 [04000F80]
00CC92:zeus16_w(266) = 0000 [00000000]
00CC92:zeus16_w(267) = 8CC0 [8CC00000]
00CC92:zeus16_w(268) = 011A [0000011A]
00CC92:zeus16_w(269) = 0000 [0000011A]
00CC92:zeus16_w(26A) = 0000 [00000000]
00CC92:zeus16_w(26B) = 0000 [00000000]
00CC92:zeus16_w(26C) = 0F00 [00000F00]
00CC92:zeus16_w(26D) = 0000 [00000F00]
00CC92:zeus16_w(26E) = 0000 [00000000]
00CC92:zeus16_w(26F) = 0000 [00000000]
00CC92:zeus16_w(270) = A001 [0000A001]
00CC92:zeus16_w(271) = 0003 [0003A001]
00CC92:zeus16_w(272) = 0025 [00000025]
00CC92:zeus16_w(273) = 0000 [00000025]
00CC92:zeus16_w(274) = 0000 [00000000]
00CC92:zeus16_w(275) = 40F3 [40F30000]
00CC92:zeus16_w(276) = 0000 [00000000]
00CC92:zeus16_w(277) = 0000 [00000000]
00CC92:zeus16_w(278) = 4000 [00004000]
00CC92:zeus16_w(279) = 0803 [08034000]
00CC92:zeus16_w(27A) = 2000 [00002000]
00CC92:zeus16_w(27B) = 0013 [00132000]
00CC92:zeus16_w(27C) = 8000 [00008000]
00CC92:zeus16_w(27D) = 0003 [00038000]
00CC92:zeus16_w(27E) = 1000 [00001000]
00CC92:zeus16_w(27F) = 003E [003E1000]
00CC92:zeus16_w(280) = 20F0 [000020F0]
00CC92:zeus16_w(281) = 0001 [000120F0]
00CC92:zeus16_w(282) = 0000 [00000000]
00CC92:zeus16_w(283) = 93C0 [93C00000]
00CC92:zeus16_w(284) = 60F2 [000060F2]
00CC92:zeus16_w(285) = 0001 [000160F2]
00CC92:zeus16_w(286) = 0000 [00000000]
00CC92:zeus16_w(287) = 9380 [93800000]
00CC92:zeus16_w(288) = FFF4 [0000FFF4]
00CC92:zeus16_w(289) = 0001 [0001FFF4]
00CC92:zeus16_w(28A) = 0000 [00000000]
00CC92:zeus16_w(28B) = 90C0 [90C00000]
00CC92:zeus16_w(28C) = 0000 [00000000]
00CC92:zeus16_w(28D) = 0000 [00000000]
00CC92:zeus16_w(28E) = 0000 [00000000]
00CC92:zeus16_w(28F) = 0000 [00000000]
00CC92:zeus16_w(290) = 0000 [00000000]
00CC92:zeus16_w(291) = 0000 [00000000]
00CC92:zeus16_w(292) = 0000 [00000000]
00CC92:zeus16_w(293) = 0000 [00000000]
00CC92:zeus16_w(294) = 0000 [00000000]
00CC92:zeus16_w(295) = 0000 [00000000]
00CC92:zeus16_w(296) = 0000 [00000000]
00CC92:zeus16_w(297) = 0000 [00000000]
00CC92:zeus16_w(298) = 0000 [00000000]
00CC92:zeus16_w(299) = 0000 [00000000]
00CC92:zeus16_w(29A) = 0000 [00000000]
00CC92:zeus16_w(29B) = 0000 [00000000]
00CC92:zeus16_w(29C) = 0000 [00000000]
00CC92:zeus16_w(29D) = 0000 [00000000]
00CC92:zeus16_w(29E) = 0000 [00000000]
00CC92:zeus16_w(29F) = 0000 [00000000]
00CC92:zeus16_w(2A0) = 0000 [00000000]
00CC92:zeus16_w(2A1) = 0000 [00000000]
00CC92:zeus16_w(2A2) = 0000 [00000000]
00CC92:zeus16_w(2A3) = 0000 [00000000]
00CC92:zeus16_w(2A4) = 0000 [00000000]
00CC92:zeus16_w(2A5) = 0000 [00000000]
00CC92:zeus16_w(2A6) = 0000 [00000000]
00CC92:zeus16_w(2A7) = 0000 [00000000]
00CC92:zeus16_w(2A8) = 0000 [00000000]
00CC92:zeus16_w(2A9) = 0000 [00000000]
00CC92:zeus16_w(2AA) = 0000 [00000000]
00CC92:zeus16_w(2AB) = 0000 [00000000]
00CC92:zeus16_w(2AC) = 0000 [00000000]
00CC92:zeus16_w(2AD) = 0000 [00000000]
00CC92:zeus16_w(2AE) = 0000 [00000000]
00CC92:zeus16_w(2AF) = 0000 [00000000]
00CC92:zeus16_w(2B0) = 0000 [00000000]
00CC92:zeus16_w(2B1) = 0000 [00000000]
00CC92:zeus16_w(2B2) = 0000 [00000000]
00CC92:zeus16_w(2B3) = 0000 [00000000]
00CC92:zeus16_w(2B4) = 7F00 [00007F00]
00CC92:zeus16_w(2B5) = 0001 [00017F00]
00CC92:zeus16_w(2B6) = 0000 [00000000]
00CC92:zeus16_w(2B7) = 03C0 [03C00000]
00CC92:zeus16_w(2B8) = C000 [0000C000]
00CC92:zeus16_w(2B9) = 0003 [0003C000]
00CC92:zeus16_w(2BA) = 0000 [00000000]
00CC92:zeus16_w(2BB) = 0000 [00000000]
00CC92:zeus16_w(2BC) = EF00 [0000EF00]
00CC92:zeus16_w(2BD) = 0001 [0001EF00]
00CC92:zeus16_w(2BE) = 0000 [00000000]
00CC92:zeus16_w(2BF) = 03C0 [03C00000]
00CC92:zeus16_w(2C0) = 0007 [00000007]
00CC86:zeus16_w(68) = 0008 [00000008]
00CC86:zeus16_w(80) = 0C01 [00000C01]
00CC86:zeus16_w(80) = 0C1D [00000C1D]
00CC86:zeus16_w(69) = 0003 [00030008]
00CC86:zeus16_w(69) = 0000 [00000008]
00CC86:zeus16_w(80) = 0C01 [00000C01]
00CC86:zeus16_w(80) = FC1D [0000FC1D]
00CC86:zeus16_w(81) = 0022 [0022FC1D]
Zeus cmd 1A : 1A000000
00CFDC:zeus32_w(86) = 00008020
00CFE0:zeus32_w(8E) = 0207F440
00CFE1:zeus32_w(100) = 00000000
00CFE5:zeus32_w(86) = 00008000
00CFE7:zeus32_w(8E) = 02060000
00D0FC:zeus32_w(CE) = 00C97020
00D09A:zeus32_w(80) = 0022FC3F
00D104:zeus32_w(CE) = 00C87020
00D034:zeus32_w(84) = 00000100
00D0C9:zeus32_w(68) = 00000100
00D0CA:zeus32_w(00) = 0000C3F0
00D0CC:zeus32_w(5C) = 00000000
00D0CE:zeus32_w(06) = FFFF0030
00D0D0:zeus32_w(02) = 000C0000
00D0D2:zeus32_w(04) = 00000E01
00D0D4:zeus32_w(4C) = 00808080
00D0D5:zeus32_w(4E) = 00808080
00D0D6:zeus32_w(08) = 00000000
00D0D7:zeus32_w(0C) = 00FF018F
00D0DD:zeus32_w(0A) = 0000018F
00D0E3:zeus32_w(0E) = 00FF0000
00D0E5:zeus32_w(1E) = FFFFFFFF
00D0E6:zeus32_w(1C) = FFFFFFFF
00D0E7:zeus32_w(1A) = FFFFFFFF
00D0E8:zeus32_w(18) = FFFFFFFF
00D0EA:zeus32_w(46) = 00000000
00D0EB:zeus32_w(44) = 00000000
00D0EC:zeus32_w(42) = 00000000
00D0ED:zeus32_w(40) = 00000000
00D0EE:zeus32_w(26) = 00000000
00D0EF:zeus32_w(24) = 00000000
00D0F0:zeus32_w(22) = 00000000
00D0F1:zeus32_w(20) = 00000000
00D09A:zeus32_w(80) = 0022FCFF
00D0F6:zeus32_w(60) = 00000001
00D09A:zeus32_w(80) = 0022FC3F
00D01E:zeus32_w(76) = F8800000
00D020:zeus32_w(CC) = 00000000
00D034:zeus32_w(84) = 00000080
00D0FC:zeus32_w(CE) = 00C97020
00D09A:zeus32_w(80) = 0022FC3F
00D104:zeus32_w(CE) = 00C87020
Zeus cmd 1A : 1A000000
Zeus cmd 18 : 187E0000 0000FFFF
Zeus cmd 18 : 187C0000 FFFFFFE0
Zeus cmd 18 : 187A0000 00800000
Zeus cmd 18 : 18780000 000000C7
Zeus cmd 18 : 18760000 F8800000
Zeus cmd 17 : 176F0032
Zeus cmd 17 : 176E0031
Zeus cmd 17 : 176D003B
Zeus cmd 17 : 176C002B
Zeus cmd 18 : 184E0000 00808080
Zeus cmd 18 : 184C0000 00808080
Zeus cmd 18 : 184A0000 00FF018E
Zeus cmd 18 : 18480000 00FF018E
Zeus cmd 17 : 176940C4
Zeus cmd 17 : 17681370
Zeus cmd 17 : 1707FFFF
Zeus cmd 17 : 17060030
Zeus cmd 17 : 17010000
Zeus cmd 2D : 2D007FFF
Zeus cmd 2E : 2E000800 00000000
Zeus cmd 17 : 1757500A
Zeus cmd 17 : 175600FF
Zeus cmd 17 : 17524321
Zeus cmd 17 : 1752BCDE
Zeus cmd 01 : 0100C0B0 3F001059
00D09A:zeus32_w(80) = 0222FC3F
....
Zeus cmd 1A : 1A000000
Zeus cmd 1A : 1A000000
....
Zeus cmd 1A : 1A000000
Zeus cmd 1A : 1A000000
....
Zeus cmd 01 : 0100C0B0 3F001059
00D09A:zeus32_w(80) = 0222FC3F
....
Zeus cmd 1A : 1A000000
Zeus cmd 1A : 1A000000
....
00D09A:zeus32_w(80) = FB22FC3F
00D01E:zeus32_w(76) = F8800000
00D020:zeus32_w(CC) = 00800000
00D034:zeus32_w(84) = 00000000
00D0C9:zeus32_w(68) = 00000100
00D0CA:zeus32_w(00) = 00000000
00D0CC:zeus32_w(5C) = 00000000
00D0CE:zeus32_w(06) = FFFF0030
00D0D0:zeus32_w(02) = 000C0000
00D0D2:zeus32_w(04) = 00000E01
00D0D4:zeus32_w(4C) = 00808080
00D0D5:zeus32_w(4E) = 00808080
00D0D6:zeus32_w(08) = 00000000
00D0D7:zeus32_w(0C) = 00FF018F
00D0DD:zeus32_w(0A) = 0000018F
00D0E3:zeus32_w(0E) = 00FF0000
00D0E5:zeus32_w(1E) = FFFFFFFF
00D0E6:zeus32_w(1C) = FFFFFFFF
00D0E7:zeus32_w(1A) = FFFFFFFF
00D0E8:zeus32_w(18) = FFFFFFFF
00D0EA:zeus32_w(46) = 00000000
00D0EB:zeus32_w(44) = 00000000
00D0EC:zeus32_w(42) = 00000000
00D0ED:zeus32_w(40) = 00000000
00D0EE:zeus32_w(26) = 00000000
00D0EF:zeus32_w(24) = 00000000
00D0F0:zeus32_w(22) = 00000000
00D0F1:zeus32_w(20) = 00000000
00D09A:zeus32_w(80) = FB22FCFF
00D0F6:zeus32_w(60) = 00000001
00D09A:zeus32_w(80) = FB22FC3F
Zeus cmd 17 : 1757500A
Zeus cmd 17 : 175600FF
00D09A:zeus32_w(80) = 0222FC3F


*/
static void render_poly_4bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	const UINT32 *texbase = extra->texbase;
	const UINT32 *palbase = extra->palbase;
	UINT16 transcolor = extra->transcolor;
	int texwidth = extra->texwidth;
	int x;

if (extra->alpha < 0x80)
	return;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		float z = recip_approx(ooz);
		int u = (int)(uoz * z) & ((texwidth * 2) - 1);
		int v = (int)(voz * z) & 255;
		UINT8 texel = get_texel_4bit(texbase, v, u, texwidth);
		if (texel != transcolor)
		{
			UINT16 color = palbase[texel >> 1] >> (16 * (texel & 1));
			INT32 depth = z * 256.0f;
			if (depth > 0x7fff) depth = 0x7fff;
			if (depth >= 0)
				waveram_plot_check_depth(scanline, x, color, depth);
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}


static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	const UINT32 *texbase = extra->texbase;
	const UINT32 *palbase = extra->palbase;
	UINT16 transcolor = extra->transcolor;
	int texwidth = extra->texwidth;
	int x;

if (extra->alpha < 0x80)
	return;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		float z = recip_approx(ooz);
		int u = (int)(uoz * z) & (texwidth - 1);
		int v = (int)(voz * z) & 255;
		UINT8 texel = get_texel_8bit(texbase, v, u, texwidth);
		if (texel != transcolor)
		{
			UINT16 color = palbase[texel >> 1] >> (16 * (texel & 1));
			INT32 depth = z * 256.0f;
			if (depth > 0x7fff) depth = 0x7fff;
			if (depth >= 0)
				waveram_plot_check_depth(scanline, x, color, depth);
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}


static void zeus_draw_model(UINT32 addr_and_count, UINT32 texdata, int logcmd)
{
	static UINT32 *loglist[1000];
	static int loglist_last = 0;
	UINT32 *data = waveram_ptr_from_addr(addr_and_count);
	UINT32 val1, val2;
	int i, logit = FALSE;
	int texwshift;
	int testbit = 0;

	val1 = ((texdata >> 10) & 0x3f0000) | (texdata & 0xffff);
	val2 = (texdata >> 16) & 0x3ff;
	texwshift = (val2 >> 6) & 7;

	if (logcmd)
	{
		int texwshift = (val2 >> 6) & 7;
		logerror(" -- model @ %08X val2=%03X val1=%06X (w=%d bpp=%d adj=%03X%03X)\n",
			addr_and_count, val2, val1,
			512 >> texwshift,
			(val2 & 0x20) ? 8 : 4,
			((val1 << (7 - texwshift)) >> 10) & 0xfff,
			((val1 << (7 - texwshift)) >> 1) & 0x1ff
			);
	}

	if (logcmd)
	{
		for (i = 0; i < loglist_last; i++)
			if (loglist[i] == data)
				break;
		logit = (i == loglist_last);
		if (i == loglist_last)
			loglist[loglist_last++] = data;
	}

	if (input_code_pressed(KEYCODE_Z)) testbit = 1;
	if (input_code_pressed(KEYCODE_X)) testbit = 2;
	if (input_code_pressed(KEYCODE_C)) testbit = 3;
	if (input_code_pressed(KEYCODE_V)) testbit = 4;
	if (input_code_pressed(KEYCODE_B)) testbit = 5;
	if (input_code_pressed(KEYCODE_N)) testbit = 6;
	if (input_code_pressed(KEYCODE_M)) testbit = 7;

	if (testbit != 0)
		texwshift = testbit;

	while (1)
	{
		const UINT32 *data = waveram_ptr_from_addr(addr_and_count);
		int count = ((addr_and_count >> 24) + 1) * 2;
		UINT32 next_addr_and_count = 0;
		int datanum;
		int length;

		for (datanum = 0; datanum < count; datanum += length * 2)
		{
			int offs;

			length = ((data[datanum] >> 24) == 0x25 || (data[datanum] >> 24) == 0x30) ? 7 : 1;

			if (logit)
			{
				logerror("\t%08X", data[datanum]);
				for (offs = 1; offs < length * 2; offs++)
					logerror(" %08X", data[datanum + offs]);
				logerror("\n");
			}

			if ((data[datanum] >> 24) == 0x25 || (data[datanum] >> 24) == 0x30 /* invasn */)
			{
				poly_draw_scanline callback;
				poly_extra_data *extra;
				poly_vertex vert[4];
				int i;

				callback = (val2 & 0x20) ? render_poly_8bit : render_poly_4bit;

				for (i = 0; i < 4; i++)
				{
					UINT32 ixy = data[datanum + 2 + 2*i];
					UINT32 iuvz = data[datanum + 3 + 2*i];
					INT32 xo = (INT16)ixy;
					INT32 yo = (INT16)(ixy >> 16);
					INT32 zo = (INT16)iuvz;
					UINT8 u = iuvz >> 16;
					UINT8 v = iuvz >> 24;
					float z, ooz;

					vert[i].x = (xo * zeus_matrix[0][0] + yo * zeus_matrix[0][1] + zo * zeus_matrix[0][2] + zeus_point[0]) * (1.0f / 65536.0f);
					vert[i].y = (xo * zeus_matrix[1][0] + yo * zeus_matrix[1][1] + zo * zeus_matrix[1][2] + zeus_point[1]) * (1.0f / 65536.0f);
					z = (xo * zeus_matrix[2][0] + yo * zeus_matrix[2][1] + zo * zeus_matrix[2][2] + zeus_point[2]) * (1.0f / (512.0f * 65536.0f));

					ooz = 1.0f / z;
					vert[i].x *= ooz;
					vert[i].y *= ooz;
					vert[i].p[0] = ooz;
					vert[i].p[1] = u * ooz;
					vert[i].p[2] = v * ooz;

					vert[i].x += 200.0f;
					vert[i].y += 128.0f;
				}

				extra = poly_get_extra_data(poly);
				extra->transcolor = 0x100;
				extra->texbase = &waveram[(val1 << (7 - texwshift)) & 0x3fffff];
				extra->texwidth = 512 >> texwshift;
				extra->palbase = zeus_palbase;
				extra->alpha = zeus_blend;

				if (single_step)
				{
					single_step_extra = *extra;
					single_step_val1 = val1;
					single_step_val2 = val2;
					single_step_dx = single_step_dy = 0;
					single_step_advance = FALSE;
				}

				poly_render_quad(poly, NULL, &Machine->screen[0].visarea, callback, 3, &vert[0], &vert[1], &vert[2], &vert[3]);
				poly_wait(poly, "Normal");

				if (single_step)
					while (!single_step_advance) video_frame_update(TRUE);
			}
			else if (data[datanum] == 0x0c018000/* mk4 */ || data[datanum] == 0x0c008000/* invasn */)
				next_addr_and_count = data[datanum + 1];
		}
		if (datanum != count)
			logerror("Overran data\n");

		if (next_addr_and_count == 0)
			break;
		else
			addr_and_count = next_addr_and_count;
	}
}


READ32_HANDLER( zeus_r )
{
	int logit = (offset < 0xb0 || offset > 0xb7);
	UINT32 result = zeusbase[offset & ~1];

	switch (offset & ~1)
	{
		case 0x00:
			// crusnexo wants bit 0x20 to be non-zero
			result = 0x20;
			break;

		case 0x51:
			// crusnexo expects a reflection of the data at 0x08 here (b425)
			result = zeusbase[0x08];
			break;

		case 0xf2:
			// polled frequently during gametime; expected to change often
			// as it is polled multiple times and a consistent value is looked for
			result = video_screen_get_vpos(0);
			logit = 0;
			break;

		case 0xf4:
			result = 6;
			if (video_screen_get_vblank(0))
				result |= 0x800;
			logit = 0;
			break;

		case 0xf6:		// status -- they wait for this & 9 == 0
			if (zeusbase[0xb6] == 0x80040000)
				result = 1;
			else
				result = 0;
			logit = 0;
			break;
	}

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
	{
		if (offset & 1)
			result >>= 16;
		if (logit)
		{
			if (offset & 1)
				logerror("%06X:zeus32_r(%02X) = %08X -- unexpected in 32-bit mode\n", activecpu_get_pc(), offset, result);
			else if (offset != 0xe0)
				logerror("%06X:zeus32_r(%02X) = %08X\n", activecpu_get_pc(), offset, result);
			else
				logerror("%06X:zeus32_r(%02X) = %08X\n", activecpu_get_pc(), offset, result);
		}
	}

	/* 16-bit mode */
	else
	{
		if (offset & 1)
			result >>= 16;
		else
			result &= 0xffff;
		if (logit)
			logerror("%06X:zeus16_r(%02X) = %04X\n", activecpu_get_pc(), offset, result);
	}
	return result;
}


WRITE32_HANDLER( zeus_w )
{
	int logit = zeus_enable_logging || ((offset < 0xb0 || offset > 0xb7) && (offset < 0xe0 || offset > 0xe1));

	if (logit)
		logerror("%06X:zeus_w", activecpu_get_pc());

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
		zeus_write_32bit(offset, data, logit);

	/* 16-bit mode */
	else
		zeus_write_16bit(offset, data, logit);
}


READ32_HANDLER( zeus2_r )
{
	return zeus_r(offset * 2, 0);
}


WRITE32_HANDLER( zeus2_w )
{
	zeus_w(offset * 2, data, mem_mask);
}


static void zeus_write_16bit(offs_t offset, UINT16 data, int logit)
{
	/* write to high part on odd addresses */
	if (offset & 1)
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0x0000ffff) | (data << 16);

	/* write to low part on event addresses */
	else
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0xffff0000) | (data & 0xffff);

	/* log appropriately */
	if (logit)
		logerror("(%02X) = %04X [%08X]\n", offset, data & 0xffff, zeusbase[offset & ~1]);

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_process_write(offset);
}


static void zeus_write_32bit(offs_t offset, UINT32 data, int logit)
{
	/* always write to low word? */
	zeusbase[offset & ~1] = data;

	/* log appropriately */
	if (logit)
	{
		if (offset & 1)
			logerror("(%02X) = %08X -- unexpected in 32-bit mode\n", offset, data);
		else if (offset != 0xe0)
			logerror("(%02X) = %08X\n", offset, data);
		else
			logerror("(%02X) = %08X\n", offset, data);
	}

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_process_write(offset);
}


/*

    New analysis:

    Offset
    ------

    $06  W  Low 16 bits = texture mode

    $58  W  Usually written after $5A; after writing, waits for $F6 bit 4 to be cleared

    $5A  W  Usually written first, followed by $58

    $68  W  Written at startup:
            $00030000 = toggled on then off at startup

    $76  W

    $7C  W

    $80  W  Master control

            $40000000 = set along with $02000000 when enabling FIFO empty interrupt
            $02000000 = enable internal data FIFO empty interrupt
            $01000000 = cleared along with $02000000 when disabling FIFO empty interrupt
            $00020000 = 16-bit (0) or 32-bit (1) mode
            $00000080 = set just before writing direct command to $60
            $00000040 = set just before writing direct command to $60
            $00000008 = cleared just before writing to offsets $200-$3FF

    $82  W
    $84  W

            $00000080 = render page? (generally set when $CC is set to $000000 and clear when $CC is set to $800000)

    $86  W
    $88  W
    $8A  W
    $8C  W
    $8E  W

    $B0 R/W Data port for direct RAM access
    $B2 R/W Data port for direct RAM access

    $B4  W  Address for direct RAM access

            $07FF0100 = mask for Y coordinate
            $000000FF = mask for X coordinate

    $B6  W  Direct RAM access control

            $80000000 = enable writes to wave RAM 0
            $40000000 = enable writes to wave RAM 1
            $02000000 = perform access on write to B0(0) or B2(1) ?????
            $00800000 = enable writes from [$B2] to depth buffer
            $00400000 = enable writes from [$B0] to depth buffer
            $00200000 = enable writes from [$B2] to RAM buffer
            $00100000 = enable writes from [$B0] to RAM buffer
            $00020000 = auto increment the address
            $00010000 = perform a read when the address is written
            $00000001 = enable direct RAM access

            Examples:
                $80A00000 when writing a single pixel (address written each time)
                $80A20001 when writing the right pixel of a pair (autoinc assumed)
                $80F60001 when writing the middle pairs of pixels (autoinc assumed)
                $80520001 when writing the left pixel of a pair (autoinc assumed)
                $82F00001 when testing wave RAM page 1 (address written each pair)
                $42F00001 when testing wave RAM page 0 (address written each pair)
                $82F60001 when uploading to wave RAM page 1 (autoinc assumed)
                $42F60001 when uploading to wave RAM page 0 (autoinc assumed)

    $C0  W  Written at startup = $801F2500
    $C2  W  Written at startup = $0015E591
    $C4  W  Written at startup = $000C0042 (HSYNC start/HSYNC end?)
    $C6  W  Written at startup = $0211007F (HTOTAL/HBLANK end?)
    $C8  W  Written at startup = $010300FF (VSYNC start/VBLANK start?)
    $CA  W  Written at startup = $01160107 (VTOTAL/VSYNC end?)
    $CC  W  Written at startup = $00000000 (display start address? switches to $00800000)
    $CE  W  Written at startup = $00C87620

    $E0  W  Internal data FIFO

    $F2  R  Looped on until it returns a stable value; must change frequently; PRNG?

    $F4  R  Status register

            $00000800 = VBLANK? code waits for it to clear to 0 and then reset to 1
            $00000140 = tested together in tight loop until both are 0
            $00000008 = internal data FIFO full
            $00000004 = tested in tight loop along with $00000002 until at least one is set
            $00000002 = tested in tight loop after handling FIFO interrupt until set

    $F6  R  Status register 2

            $00000010 = tested in tight loop after writing 5A/59/58 until set to 0

   $200- W  Unknown, but written in a loop with master control bit $00000008 cleared
   $3FF     Possible fast buffer clear? One set of writes stores $7F7F to all entries.
            A second set writes $E00,$25,$E01,$25,...,$E7F,$25. A third set writes
            $52,$A0000000,...

Notes:

    In 16-bit mode, writes to $E0 are done via $E1 first then $E0

-------------------------------------------------------

Old info:

    offset B7 = control....

        8000 = page 1 enable
        4000 = page 0 enable

        0080 = 2nd pixel enable (maybe Z buff?)
        0040 = 1st pixel enable (maybe Z buff?)
        0020 = 2nd pixel enable
        0010 = 1st pixel enable


    writes 80a2 to write an odd pixel at startup
    writes 80f6 to write the middle pixels
    writes 8052 to write an odd pixel at the end

    writes 8050 before writing an odd pixel
    writes 80a0 before writing an even pixel

    writes 44f1 before reading from page 0 of wave ram
    writes 84f1 before reading from page 1 of wave ram

    writes 42f0 before writing to page 0 of wave ram
    writes 82f0 before writing to page 1 of wave ram

*/

static void zeus_process_write(offs_t offset)
{
	/* handle the writes; only trigger on low accesses */
	switch (offset)
	{
		case 0x60:
			/* invasn writes here to execute a command (?) */
			if (zeusbase[0x60] & 1)
			{
				if ((zeusbase[0x80] & 0xffffff) == 0x22FCFF)
				{
					int startx, stopx, starty, stopy;
					int x, y;
					// zeusbase[0x00] = color
					// zeusbase[0x02] = ??? = 0x000C0000
					// zeusbase[0x04] = ??? = 0x00000E01
					// zeusbase[0x06] = ??? = 0xFFFF0030
					// zeusbase[0x08] = vert[0] = (y0 << 16) | x0
					// zeusbase[0x0a] = vert[1] = (y1 << 16) | x1
					// zeusbase[0x0c] = vert[2] = (y2 << 16) | x2
					// zeusbase[0x0e] = vert[3] = (y3 << 16) | x3
					// zeusbase[0x18] = ??? = 0xFFFFFFFF
					// zeusbase[0x1a] = ??? = 0xFFFFFFFF
					// zeusbase[0x1c] = ??? = 0xFFFFFFFF
					// zeusbase[0x1e] = ??? = 0xFFFFFFFF
					// zeusbase[0x20] = ??? = 0x00000000
					// zeusbase[0x22] = ??? = 0x00000000
					// zeusbase[0x24] = ??? = 0x00000000
					// zeusbase[0x26] = ??? = 0x00000000
					// zeusbase[0x40] = ??? = 0x00000000
					// zeusbase[0x42] = ??? = 0x00000000
					// zeusbase[0x44] = ??? = 0x00000000
					// zeusbase[0x46] = ??? = 0x00000000
					// zeusbase[0x4c] = ??? = 0x00808080 (brightness?)
					// zeusbase[0x4e] = ??? = 0x00808080 (brightness?)
					startx = (zeusbase[0x08] & 0xffff);
					starty = zeusbase[0x08] >> 16;
					stopx = (zeusbase[0x0c] & 0xffff);
					stopy = zeusbase[0x0c] >> 16;
					for (y = starty; y <= stopy; y++)
						for (x = startx; x <= stopx; x++)
							waveram[(1 * WAVERAM_HEIGHT + (y % WAVERAM_HEIGHT)) * WAVERAM_WIDTH + x] = zeusbase[0x00];
	single_step = input_code_pressed(KEYCODE_S);
	if (single_step)
		popmessage("Single Step");
				}
				else
					logerror("Execute unknown command\n");
			}
			break;

		case 0xb0:
		case 0xb2:
			if ((zeusbase[0xb6] >> 16) != 0)
			{
				UINT32 *dest = zeus_pixaddr();
				if ((offset == 0xb0 && (zeusbase[0xb6] & 0x02000000) == 0) ||
					(offset == 0xb2 && (zeusbase[0xb6] & 0x02000000) != 0))
				{
					if (zeusbase[0xb6] & 0x00100000)
						dest[0] = (dest[0] & 0xffff0000) | (zeusbase[0xb0] & 0x0000ffff);
					if (zeusbase[0xb6] & 0x00200000)
						dest[0] = (dest[0] & 0x0000ffff) | (zeusbase[0xb0] & 0xffff0000);
					if (zeusbase[0xb6] & 0x00400000)
						dest[1] = (dest[1] & 0xffff0000) | (zeusbase[0xb2] & 0x0000ffff);
					if (zeusbase[0xb6] & 0x00800000)
						dest[1] = (dest[1] & 0x0000ffff) | (zeusbase[0xb2] & 0xffff0000);
					if (zeusbase[0xb6] & 0x00020000)
						zeusbase[0xb4]++;
				}
			}
			break;

		case 0xb4:
			if (zeusbase[0xb6] & 0x00010000)
			{
				UINT32 *src = zeus_pixaddr();
				zeusbase[0xb0] = src[0];
				zeusbase[0xb2] = src[1];
			}
			break;

		case 0x84:
			/* MK4: Written in tandem with 0xcc */
			/* MK4: Writes either 0x80 (and 0x000000 to 0xcc) or 0x00 (and 0x800000 to 0xcc) */
			break;

		case 0xcc:
			/* see 0x84 above */
			break;

		case 0x70:
			zeus_point[0] = zeusbase[0x70] << 16;
			break;

		case 0x72:
			zeus_point[1] = zeusbase[0x72] << 16;
			break;

		case 0x74:
			zeus_point[2] = zeusbase[0x74] << 16;
			break;

		case 0x4e:
			zeus_blend = zeusbase[0x4e];
			break;
//          if ((zeuscmd_buffer[0] >> 16) == 0x1770)
//              zeus_point[0] = zeuscmd_buffer[0] << 16;
//          if ((zeuscmd_buffer[0] >> 16) == 0x1772)
//              zeus_point[1] = zeuscmd_buffer[0] << 16;
//          if ((zeuscmd_buffer[0] >> 16) == 0x1774)
//              zeus_point[2] = zeuscmd_buffer[0] << 16;
//          if ((zeuscmd_buffer[0] >> 16) == 0x18ce)
//              zeus_blend = zeuscmd_buffer[1];

		case 0x52:
			zeusbase[0xb2] = zeusbase[0x52];
			break;
//          if ((zeuscmd_buffer[0] >> 16) == 0x1752)
//          {
//              zeusbase[0xb2] = zeuscmd_buffer[0];
//              zeus_enable_logging = 1;
//          }

		case 0xe0:
			zeuscmd_buffer[zeuscmd_buffer_words++] = zeusbase[0xe0];
			if (zeuscmd_buffer_words == 1)
				zeuscmd_buffer_expected_words = zeuscmd_length(zeusbase[0xe0]);
			if (zeuscmd_buffer_words == zeuscmd_buffer_expected_words)
			{
				zeuscmd_execute(zeuscmd_buffer_expected_words);
				zeuscmd_buffer_words = 0;
			}
			break;
	}
}


static void log_waveram(UINT32 length_and_base)
{
	static struct
	{
		UINT32 lab, checksum;
	} recent_entries[100];

	UINT32 numoctets = (length_and_base >> 24) + 1;
	const UINT32 *ptr = waveram_ptr_from_addr(length_and_base);
	UINT32 checksum = length_and_base;
	int foundit = FALSE;
	int i;

	for (i = 0; i < numoctets; i++)
		checksum += ptr[i * 2 + 0] + ptr[i * 2 + 1];

	for (i = 0; i < ARRAY_LENGTH(recent_entries); i++)
		if (recent_entries[i].lab == length_and_base && recent_entries[i].checksum == checksum)
		{
			foundit = TRUE;
			break;
		}

	if (i == ARRAY_LENGTH(recent_entries))
		i--;
	if (i != 0)
	{
		memmove(&recent_entries[1], &recent_entries[0], i * sizeof(recent_entries[0]));
		recent_entries[0].lab = length_and_base;
		recent_entries[0].checksum = checksum;
	}
	if (foundit)
		return;

	for (i = 0; i < numoctets; i++)
		logerror("\t%02X: %08X %08X\n", i, ptr[i * 2 + 0], ptr[i * 2 + 1]);
}


static void zeuscmd_execute(int words)
{
	static int colorcount;
	int logcmd = 1;//input_code_pressed(KEYCODE_L);
	int wordnum;

/* command observations:

    0x18800000 | (reg << 16), <data>:

        18F80000 00000C88 -- similar to write to $78 = 00000C78
        18FA0000 00808000 -- similar to write to $7A = 00800000
        18C80000 00FF018E -- similar to write to $48 = 00FF0190
        18CA0000 00FF18E0 -- similar to write to $4A = 00FF1900

        -- during setup/text screen
        187E0000 0000FFFF FF980000
        18C80000 01FF01FF 00FF018E
        18CA0000 01FF1FF0 00FF18E0
        18CC0000 00808080
        18CE0000 00000080 00FFFFFF
        18D60000 500A00FF
        18EC0000 003B002F
        18EE0000 00320031
        18F80000 00000C88
        18FA0000 00808000

        -- after text screen
        187E0000 0000FFFF FF980000
        18CE0000 0000007B(ramp down to 0) 00FFFFFF
        18D60000 500A00FF

    0x17:

        1707FFFF
        1752159D
        17680070 17681370
        176940C4
        1770xxxx - set x coordinate
        1772yyyy - set y coordinate
        1774zzzz - set z coordinate

    0x00:

        0004C09C 08001026
        0002C0E7 0800101C
        0003C0F0 00001025
        0080C0A5 3F00102F
        0005C0A5 21001125
        0000C040 0318E132 - set palette base

    command 25:

        25000000 007F0018 01000400 B0F60008 -- during diagnostics screen
        25000080 007F0018 01000400 B0F60008 -- alternate frames during text screen


    write to ($60) = 1 after setting up:


*/

	if (logcmd)
	{
		logerror("Zeus cmd %02X :", zeuscmd_buffer[0] >> 24);
		for (wordnum = 0; wordnum < words; wordnum++)
			logerror(" %08X", zeuscmd_buffer[wordnum]);
	}

	switch (zeuscmd_buffer[0] >> 24)
	{
		/* 0x00/0x01: set pointer */
		/* in model data, this is 0x0C */
		case 0x00:
		case 0x01:
			switch (zeuscmd_buffer[0] & 0xffffff)
			{
				case 0x008000:
				/* in model data, case 0x018000 is a goto */
					if (logcmd)
						logerror(" -- setptr(objdata)\n");
					zeus_objdata = zeuscmd_buffer[1];
					break;

				case 0x00c0b0:
					if (logcmd)
						logerror(" -- setptr(texdata)\n");
					zeus_texdata = zeuscmd_buffer[1];
					break;

				case 0x00c040:
					if (logcmd)
						logerror(" -- setptr(palbase)\n");
					zeus_palbase = waveram_ptr_from_addr(zeuscmd_buffer[1]);
					break;

				case 0x80c0a5:
					if (logcmd)
						logerror(" -- setptr(hack clear screen)\n");
					{
						/* not right -- just a hack */
						int x, y;
						for (y = 0; y <= 255; y++)
							for (x = 0; x <= 399; x++)
								waveram_plot_depth(y, x, 0, 0x7fff);
						colorcount = 0;
					}
					break;

				default:
					if (logcmd)
						logerror(" -- setptr(%06X)\n", zeuscmd_buffer[0] & 0xffffff);
					break;
			}

			if (logcmd)
				log_waveram(zeuscmd_buffer[1]);
			break;

		/* 0x13: render model based on previously set information */
		case 0x13:	/* invasn */
			zeus_draw_model(zeus_objdata, zeus_texdata, logcmd);
			break;

		/* 0x17: write 16-bit value to low registers */
		case 0x17:
			if (logcmd)
				logerror(" -- reg16");
			zeus_write_16bit((zeuscmd_buffer[0] >> 16) & 0x7f, zeuscmd_buffer[0], logcmd);
			break;

		/* 0x18: write 32-bit value to low registers */
		/* in model data, this is 0x19 */
		case 0x18:
			if (logcmd)
				logerror(" -- reg32");
			zeus_write_32bit((zeuscmd_buffer[0] >> 16) & 0x7f, zeuscmd_buffer[1], logcmd);
			break;

		/* 0x1A: sync pipeline(?) */
		case 0x1a:
			if (logcmd)
				logerror(" -- sync\n");
			break;

		/* 0x1C: write matrix and translation vector */
		case 0x1c:
			if (logcmd)
				logerror(" -- matrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X ) vector %8.2f %8.2f %8.5f\n",
					zeuscmd_buffer[2] & 0xffff,
					zeuscmd_buffer[2] >> 16,
					zeuscmd_buffer[0] & 0xffff,
					zeuscmd_buffer[3] & 0xffff,
					zeuscmd_buffer[3] >> 16,
					zeuscmd_buffer[1] >> 16,
					zeuscmd_buffer[4] & 0xffff,
					zeuscmd_buffer[4] >> 16,
					zeuscmd_buffer[1] & 0xffff,
					(float)(INT32)zeuscmd_buffer[5] * (1.0f / 65536.0f),
					(float)(INT32)zeuscmd_buffer[6] * (1.0f / 65536.0f),
					(float)(INT32)zeuscmd_buffer[7] * (1.0f / (65536.0f * 512.0f)));

			zeus_matrix[0][0] = zeuscmd_buffer[2];
			zeus_matrix[0][1] = zeuscmd_buffer[2] >> 16;
			zeus_matrix[0][2] = zeuscmd_buffer[0];
			zeus_matrix[1][0] = zeuscmd_buffer[3];
			zeus_matrix[1][1] = zeuscmd_buffer[3] >> 16;
			zeus_matrix[1][2] = zeuscmd_buffer[1] >> 16;
			zeus_matrix[2][0] = zeuscmd_buffer[4];
			zeus_matrix[2][1] = zeuscmd_buffer[4] >> 16;
			zeus_matrix[2][2] = zeuscmd_buffer[1];
			zeus_point[0] = zeuscmd_buffer[5];
			zeus_point[1] = zeuscmd_buffer[6];
			zeus_point[2] = zeuscmd_buffer[7];
			break;

		/* 0x23: some additional X,Y,Z coordinates */
		case 0x23:
			if (logcmd)
				logerror(" -- additional xyz = %d,%d,%d\n", (INT16)zeuscmd_buffer[0], (INT16)(zeuscmd_buffer[1] >> 16), (INT16)zeuscmd_buffer[1]);
			break;

		/* 0x25: display control? */
		/* 0x30: same for invasn */
		case 0x25:
		case 0x30:
			if (logcmd)
				logerror(" -- unknown control\n");
			break;

		/* 0x67: render model with inline texture info */
		case 0x67:
			zeus_draw_model(zeuscmd_buffer[1], zeuscmd_buffer[2], logcmd);
			break;

		default:
			if (logcmd)
				logerror("\n");
			break;
	}
}


static UINT8 zeuscmd_length(UINT32 word)
{
	switch (word >> 24)
	{
		case 0x00:
		case 0x01:
			return (word == 0) ? 1 : 2;

		case 0x13:
		case 0x17:
		case 0x1a:
		case 0x2d:		/* invasn */
		case 0x70:
			return 1;

		case 0x18:
		case 0x23:
		case 0x2e:		/* invasn */
			return 2;

		case 0x67:
			return 3;

		case 0x25:
		case 0x30:		/* invasn */
			return 4;	/* ??? */

		case 0x1c:
			if ((word & 0xffffff) == 0x7fff)	/* invasn */
				return 8+5;
			return 8;

		default:
			printf("Unknown command %02X\n", zeusbase[0xe0] >> 24);
			break;
	}
	return 1;
}


