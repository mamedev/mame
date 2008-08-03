/*
ADP (Merkur?) games from '90 running on similar hardware.
(68k + HD63484 + YM2149)

Skeleton driver by TS -  analog at op.pl

TODO:
(almost everything)
 - add emulation of HD63484 (like shanghai.c but 4bpp mode and much more commands)
 - add sound and i/o
 - protection in Fashion Gambler (NVRam based?)

Supported games :
- Quick Jack      ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1993")
- Skat TV           ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")
- Skat TV v. TS3  ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1995")
- Fashion Gambler ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1997")
- Backgammon        ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")



Skat TV (Version TS3)
Three board stack.

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |KM681000ALP7|     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |KM681000ALP7|     |_________| |
 |     |||   |____________|  |____________| |||             |
 |     |||   ___________                    |||  M62X42B    |
 | X   |||   |         |                    |||             |
 |     |||   |68EC000 8|  74HC32   74HC245  |||  MAX691CPE  |
 |     |||   |         |  74AC138  74HC573  |||    74HC32   |
 |           |         |                                    |
 | 74HC573   |_________|  74HC08   74HC10  74HC32  74HC21   |
 |__________________________________________________________|

Parts:

 68EC000FN8         - Motorola 68k CPU
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                    - 8MHz xtal
 3V Bat             - Lithium 3V power module

Video Board:
------------
 ____________________________________________________________
 |           ______________  ______________                 |
 |           | t2 i       |  |KM681000ALP7|     74HC573     |
 |           |____________|  |____________|                *|
 |                                              74HC573    *|
 |           ______________  ______________                *|
 |           | t2 ii      |  |KM681000ALP7|               P3|
 |       ||| |____________|  |____________|   |||          *|
 |       ||| ___________                      |||          *|
 |       ||| |         |                      |||          *|
 |       ||| | HD63484 |  74HC04   74HC00     |||         P6|
 |       ||| |         |  74HC74   74HC08     |||  74HC245  |
 |           |         |                                    |
 | 74HC573   |_________|  74HC166  74HC166 74HC166 74HC166  |
 |__________________________________________________________|

Parts:

 HD63484CP8         - Advanced CRT Controller
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM

Connectors:

 Two connectors to link with CPU Board
 Two connectors to link with Sound and I/O Board
 P3  - Monitor
 P6  - Lightpen

Sound  and I/O board:
---------------------
 _________________________________________________________________________________
 |                        TS271CN    74HC02                        ****  ****    |
 |*                      ________________                          P1    P2     *|
 |*         74HC574      | YM2149F      |                                       *|
 |*                  ||| |______________|   74HC393  74HC4015 |||               *|
 |P3        74HC245  |||                                      |||              P6|
 |*                  ||| ________________          X          ||| TL7705ACP     *|
 |*                  ||| |SCN68681C1N40 |                     |||               *|
 |*                  ||| |______________|   74HC32   74AC138  |||               *|
 |P7                 |||                                      |||              P8|
 |*                        TC428CPA                                             *|
 |*                                                                             *|
 |*    P11  P12    P13    P14       P15   P16   P17      P18   P19   P20  P21   *|
 |P9   **** *****  *****  ****  OO  ****  ****  *******  ****  ****  ***  *** P10|
 |_______________________________________________________________________________|

Parts:

 YM2149F         - Yamaha PSG
 SCN68681C1N40   - Dual Asynchronous Receiver/transmitter (DUART);
 TS271CN         - Programmable Low Power CMOS Single Op-amp
 TL7705ACP       - Supply Voltage Supervisor
 TC428CPA        - Dual CMOS High-speed Driver
 OO              - LEDs (red)
 X               - 3.6864MHz xtal

Connectors:

 Two connectors to link with Video Board
 P1  - Tueroeffn
 P2  - PSG In/Out
 P3  - Lautsprecher
 P6  - Service - Tast.
 P7  - Maschine (barely readable)
 P8  - Muenzeinheit
 P9  - Atzepter
 P10 - Reset Fadenfoul
 P11 - Netzteil
 P12 - Serienplan
 P13 - Serienplan 2
 P14 - Muenzeinheit 2
 P15 - I2C Bus
 P16 - Kodierg.
 P17 - TTL Ein-Aueg.
 P18 - Out
 P19 - In
 P20 - Serielle-S.
 P21 - Tuerschalter

There's also (external) JAMMA adapter - 4th board filled with resistors and diodes.

*/

#include "driver.h"
#include "sound/ay8910.h"

#define FIFO_LENGTH 50
#define HD63484_RAM_SIZE 0x200000
static int fifo_counter;
static UINT16 fifo[FIFO_LENGTH];
static UINT16 readfifo;
static UINT8 *HD63484_ram;
static UINT16 HD63484_reg[256/2];
static int org,rwp;
static UINT16 cl0,cl1,ccmp;
static INT16 cpx,cpy;


static const int instruction_length[64] =
{
	 0, 3, 2, 1,	/* 0x */
	 0, 0,-1, 2,	/* 1x */
	 0, 3, 3, 3,	/* 2x */
	 0, 0, 0, 0,	/* 3x */
	 0, 1, 2, 2,	/* 4x */
	 0, 0, 4, 4,	/* 5x */
	 5, 5, 5, 5,	/* 6x */
	 5, 5, 5, 5,	/* 7x */
	 3, 3, 3, 3, 	/* 8x */
	 3, 3,-2,-2,	/* 9x */
	-2,-2, 2, 4,	/* Ax */
	 5, 5, 7, 7,	/* Bx */
	 3, 3, 1, 1,	/* Cx */
	 2, 2, 2, 2,	/* Dx */
	 5, 5, 5, 5,	/* Ex */
	 5, 5, 5, 5 	/* Fx */
};

static const char *const instruction_name[64] =
{
	"undef","ORG  ","WPR  ","RPR  ",	/* 0x */
	"undef","undef","WPTN ","RPTN ",	/* 1x */
	"undef","DRD  ","DWT  ","DMOD ",	/* 2x */
	"undef","undef","undef","undef",	/* 3x */
	"undef","RD   ","WT   ","MOD  ",	/* 4x */
	"undef","undef","CLR  ","SCLR ",	/* 5x */
	"CPY  ","CPY  ","CPY  ","CPY  ",	/* 6x */
	"SCPY ","SCPY ","SCPY ","SCPY ",	/* 7x */
	"AMOVE","RMOVE","ALINE","RLINE", 	/* 8x */
	"ARCT ","RRCT ","APLL ","RPLL ",	/* 9x */
	"APLG ","RPLG ","CRCL ","ELPS ",	/* Ax */
	"AARC ","RARC ","AEARC","REARC",	/* Bx */
	"AFRCT","RFRCT","PAINT","DOT  ",	/* Cx */
	"PTN  ","PTN  ","PTN  ","PTN  ",	/* Dx */
	"AGCPY","AGCPY","AGCPY","AGCPY",	/* Ex */
	"RGCPY","RGCPY","RGCPY","RGCPY" 	/* Fx */
};

static void HD63484_start(void)
{
	fifo_counter = 0;
	HD63484_ram = auto_malloc(HD63484_RAM_SIZE);
	memset(HD63484_ram,0,HD63484_RAM_SIZE);
}

static void doclr(int opcode,UINT16 fill,int *dst,INT16 _ax,INT16 _ay)
{
	INT16 ax,ay;

	ax = _ax;
	ay = _ay;

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0003)
			{
				case 0:
					HD63484_ram[*dst]  = fill; break;
				case 1:
					HD63484_ram[*dst] |= fill; break;
				case 2:
					HD63484_ram[*dst] &= fill; break;
				case 3:
					HD63484_ram[*dst] ^= fill; break;
			}
			if (ax == 0) break;
			else if (ax > 0)
			{
				*dst = (*dst + 1) & (HD63484_RAM_SIZE-1);
				ax--;
			}
			else
			{
				*dst = (*dst - 1) & (HD63484_RAM_SIZE-1);
				ax++;
			}
		}

		ax = _ax;
		if (_ay < 0)
		{
			*dst = (*dst + 384 - ax) & (HD63484_RAM_SIZE-1);
			if (ay == 0) break;
			ay++;
		}
		else
		{
			*dst = (*dst - 384 - ax) & (HD63484_RAM_SIZE-1);
			if (ay == 0) break;
			ay--;
		}
	}
}

static void docpy(int opcode,int src,int *dst,INT16 _ax,INT16 _ay)
{
	int dstep1,dstep2;
	int ax = _ax;
	int ay = _ay;

	switch (opcode & 0x0700)
	{
		default:
		case 0x0000: dstep1 =  1; dstep2 = -384 - ax * dstep1; break;
		case 0x0100: dstep1 =  1; dstep2 =  384 - ax * dstep1; break;
		case 0x0200: dstep1 = -1; dstep2 = -384 + ax * dstep1; break;
		case 0x0300: dstep1 = -1; dstep2 =  384 + ax * dstep1; break;
		case 0x0400: dstep1 = -384; dstep2 =  1 - ay * dstep1; break;
		case 0x0500: dstep1 =  384; dstep2 =  1 - ay * dstep1; break;
		case 0x0600: dstep1 = -384; dstep2 = -1 + ay * dstep1; break;
		case 0x0700: dstep1 =  384; dstep2 = -1 + ay * dstep1; break; // used by kothello
	}

	for (;;)
	{
		for (;;)
		{
			switch (opcode & 0x0007)
			{
				case 0:
					HD63484_ram[*dst]  = HD63484_ram[src]; break;
				case 1:
					HD63484_ram[*dst] |= HD63484_ram[src]; break;
				case 2:
					HD63484_ram[*dst] &= HD63484_ram[src]; break;
				case 3:
					HD63484_ram[*dst] ^= HD63484_ram[src]; break;
				case 4:
					if (HD63484_ram[*dst] == (ccmp & 0xff))
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 5:
					if (HD63484_ram[*dst] != (ccmp & 0xff))
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 6:
					if (HD63484_ram[*dst] < HD63484_ram[src])
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
				case 7:
					if (HD63484_ram[*dst] > HD63484_ram[src])
						HD63484_ram[*dst] = HD63484_ram[src];
					break;
			}

			if (opcode & 0x0800)
			{
				if (ay == 0) break;
				if (_ay > 0)
				{
					src = (src - 384) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ay--;
				}
				else
				{
					src = (src + 384) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ay++;
				}
			}
			else
			{
				if (ax == 0) break;
				else if (ax > 0)
				{
					src = (src + 1) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ax--;
				}
				else
				{
					src = (src - 1) & (HD63484_RAM_SIZE-1);
					*dst = (*dst + dstep1) & (HD63484_RAM_SIZE-1);
					ax++;
				}
			}
		}

		if (opcode & 0x0800)
		{
			ay = _ay;
			if (_ax < 0)
			{
				src = (src - 1 + ay * 384) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ax == 0) break;
				ax++;
			}
			else
			{
				src = (src + 1 - ay * 384) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ax == 0) break;
				ax--;
			}
		}
		else
		{
			ax = _ax;
			if (_ay < 0)
			{
				src = (src + 384 - ax) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ay == 0) break;
				ay++;
			}
			else
			{
				src = (src - 384 - ax) & (HD63484_RAM_SIZE-1);
				*dst = (*dst + dstep2) & (HD63484_RAM_SIZE-1);
				if (ay == 0) break;
				ay--;
			}
		}
	}
}



#define PLOT(addr,OPM)								\
switch (OPM)										\
{													\
	case 0:											\
		HD63484_ram[addr]  = cl0; break;			\
	case 1:											\
		HD63484_ram[addr] |= cl0; break;			\
	case 2:											\
		HD63484_ram[addr] &= cl0; break;			\
	case 3:											\
		HD63484_ram[addr] ^= cl0; break;			\
	case 4:											\
		if (HD63484_ram[addr] == (ccmp & 0xff))		\
			HD63484_ram[addr] = cl0;				\
		break;										\
	case 5:											\
		if (HD63484_ram[addr] != (ccmp & 0xff))		\
			HD63484_ram[addr] = cl0;				\
		break;										\
	case 6:											\
		if (HD63484_ram[addr] < (cl0 & 0xff))		\
			HD63484_ram[addr] = cl0;				\
		break;										\
	case 7:											\
		if (HD63484_ram[addr] > (cl0 & 0xff))		\
			HD63484_ram[addr] = cl0;				\
		break;										\
}													\



static void HD63484_command_w(UINT16 cmd)
{
	int len;

	fifo[fifo_counter++] = cmd;

	len = instruction_length[fifo[0]>>10];
	if (len == -1)
	{
		if (fifo_counter < 2) return;
		else len = fifo[1]+2;
	}
	else if (len == -2)
	{
		if (fifo_counter < 2) return;
		else len = 2*fifo[1]+2;
	}

	if (fifo_counter >= len)
	{
		int i;

		logerror("PC %05x: HD63484 command %s (%04x) ",activecpu_get_pc(),instruction_name[fifo[0]>>10],fifo[0]);
		for (i = 1;i < fifo_counter;i++)
			logerror("%04x ",fifo[i]);
		logerror("\n");

		if (fifo[0] == 0x0400) { /* ORG */
			org = ((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4);
		}
		else if ((fifo[0] & 0xffe0) == 0x0800)	/* WPR */
		{
			if (fifo[0] == 0x0800)
				cl0 = fifo[1];
			else if (fifo[0] == 0x0801)
				cl1 = fifo[1];
			else if (fifo[0] == 0x0802)
				ccmp = fifo[1];
			else if (fifo[0] == 0x080c)
				rwp = (rwp & 0x00fff) | ((fifo[1] & 0x00ff) << 12);
			else if (fifo[0] == 0x080d)
				rwp = (rwp & 0xff000) | ((fifo[1] & 0xfff0) >> 4);
			else
logerror("unsupported register\n");
		}
		else if ((fifo[0] & 0xfff0) == 0x1800)	/* WPTN */
		{
			/* pattern RAM not supported */
		}
		else if (fifo[0] == 0x4400)	/* RD */
		{
			readfifo = HD63484_ram[2*rwp] | (HD63484_ram[2*rwp+1] << 8);
			rwp = (rwp + 1) & (HD63484_RAM_SIZE/2-1);
		}
		else if (fifo[0] == 0x4800)	/* WT */
		{
			HD63484_ram[2*rwp]   = fifo[1] & 0x00ff ;
			HD63484_ram[2*rwp+1] = (fifo[1] & 0xff00) >> 8;
			rwp = (rwp + 1) & (HD63484_RAM_SIZE/2-1);
		}
		else if (fifo[0] == 0x5800)	/* CLR */
		{
			int ax = 2*fifo[2];

			rwp *= 2;
			if (fifo[2] & 0x8000) { rwp += 1; ax -= 1; } else { ax += 1; }

			doclr(fifo[0],fifo[1],&rwp,ax,fifo[3]);

			if (fifo[2] & 0x8000) rwp -= 1;
			rwp /= 2;

			/*
			{
				int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
				if (fifo2<0) fifo2 *= -1;
				if (fifo3<0) fifo3 *= -1;
				rwp += ((fifo2+1)*(fifo3+1));
			}
			*/
		}
		else if ((fifo[0] & 0xfffc) == 0x5c00)	/* SCLR */
		{
			int ax = 2*fifo[2];

			rwp *= 2;
			if (fifo[2] & 0x8000) { rwp += 1; ax -= 1; } else { ax += 1; }

			doclr(fifo[0],fifo[1],&rwp,ax,fifo[3]);

			if (fifo[2] & 0x8000) rwp -= 1;
			rwp /= 2;

			/*
			{
				int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
				if (fifo2<0) fifo2 *= -1;
				if (fifo3<0) fifo3 *= -1;
				rwp += ((fifo2+1)*(fifo3+1));
			}
			*/
		}
		else if ((fifo[0] & 0xf0ff) == 0x6000)	/* CPY */
		{
			int src,ax;

			ax = 2*fifo[3];
			src = (((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4))*2;
			rwp *= 2;
			if (fifo[3] & 0x8000) { rwp += 1; src += 1; ax -= 1; } else { ax += 1; }

			docpy(fifo[0],src,&rwp,ax,fifo[4]);

			if (fifo[3] & 0x8000) rwp -= 1;
			rwp /= 2;

			/*
			{
				int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
				if (fifo2<0) fifo2 *= -1;
				if (fifo3<0) fifo3 *= -1;
				rwp += ((fifo2+1)*(fifo3+1));
			}
			*/
		}
		else if ((fifo[0] & 0xf0fc) == 0x7000)	/* SCPY */
		{
			int src,ax;

			ax = 2*fifo[3];
			src = (((fifo[1] & 0x00ff) << 12) | ((fifo[2] & 0xfff0) >> 4))*2;
			rwp *= 2;
			if (fifo[3] & 0x8000) { rwp += 1; src += 1; ax -= 1; } else { ax += 1; }

			docpy(fifo[0],src,&rwp,ax,fifo[4]);

			if (fifo[3] & 0x8000) rwp -= 1;
			rwp /= 2;

			/*
			{
				int fifo2 = (int)fifo[2],fifo3 = (int)fifo[3];
				if (fifo2<0) fifo2 *= -1;
				if (fifo3<0) fifo3 *= -1;
				rwp += ((fifo2+1)*(fifo3+1));
			}
			*/
		}
		else if (fifo[0] == 0x8000)	/* AMOVE */
		{
			cpx = fifo[1];
			cpy = fifo[2];
		}
//      else if ((fifo[0] & 0xff00) == 0x8800)  /* ALINE */
		else if ((fifo[0] & 0xfff8) == 0x8800)	/* ALINE */
		{
			INT16 ex,ey,sx,sy;
			INT16 ax,ay;
			int dst;

			sx = cpx;
			sy = cpy;
			ex = fifo[1];
			ey = fifo[2];

			ax = ex - sx;
			ay = ey - sy;

			if (abs(ax) >= abs(ay))
			{
				while (ax)
				{
					dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);
					PLOT(dst,fifo[0] & 0x0007)

					if (ax > 0)
					{
						cpx++;
						ax--;
					}
					else
					{
						cpx--;
						ax++;
					}
					cpy = sy + ay * (cpx - sx) / (ex - sx);
				}
			}
			else
			{
				while (ay)
				{
					dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);
					PLOT(dst,fifo[0] & 0x0007)

					if (ay > 0)
					{
						cpy++;
						ay--;
					}
					else
					{
						cpy--;
						ay++;
					}
					cpx = sx + ax * (cpy - sy) / (ey - sy);
				}
			}
		}
//      else if ((fifo[0] & 0xff00) == 0x9000)  /* ARCT */
		else if ((fifo[0] & 0xfff8) == 0x9000)	/* ARCT */
		{
			INT16 pcx,pcy;
			INT16 ax,ay;
			int dst;

			pcx = fifo[1];
			pcy = fifo[2];
			dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);

			ax = pcx - cpx;
			for (;;)
			{
				PLOT(dst,fifo[0] & 0x0007)

				if (ax == 0) break;
				else if (ax > 0)
				{
					dst = (dst + 1) & (HD63484_RAM_SIZE-1);
					ax--;
				}
				else
				{
					dst = (dst - 1) & (HD63484_RAM_SIZE-1);
					ax++;
				}
			}

			ay = pcy - cpy;
			for (;;)
			{
				PLOT(dst,fifo[0] & 0x0007)

				if (ay == 0) break;
				else if (ay > 0)
				{
					dst = (dst - 384) & (HD63484_RAM_SIZE-1);
					ay--;
				}
				else
				{
					dst = (dst + 384) & (HD63484_RAM_SIZE-1);
					ay++;
				}
			}

			ax = cpx - pcx;
			for (;;)
			{
				PLOT(dst,fifo[0] & 0x0007)

				if (ax == 0) break;
				else if (ax > 0)
				{
					dst = (dst + 1) & (HD63484_RAM_SIZE-1);
					ax--;
				}
				else
				{
					dst = (dst - 1) & (HD63484_RAM_SIZE-1);
					ax++;
				}
			}

			ay = cpy - pcy;
			for (;;)
			{
				PLOT(dst,fifo[0] & 0x0007)

				if (ay == 0) break;
				else if (ay > 0)
				{
					dst = (dst - 384) & (HD63484_RAM_SIZE-1);
					ay--;
				}
				else
				{
					dst = (dst + 384) & (HD63484_RAM_SIZE-1);
					ay++;
				}
			}
		}
//      else if ((fifo[0] & 0xff00) == 0xc000)  /* AFRCT */
		else if ((fifo[0] & 0xfff8) == 0xc000)	/* AFRCT */
		{
			INT16 pcx,pcy;
			INT16 ax,ay;
			int dst;

			pcx = fifo[1];
			pcy = fifo[2];
			ax = pcx - cpx;
			ay = pcy - cpy;
			dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);

			for (;;)
			{
				for (;;)
				{
					PLOT(dst,fifo[0] & 0x0007)

					if (ax == 0) break;
					else if (ax > 0)
					{
						dst = (dst + 1) & (HD63484_RAM_SIZE-1);
						ax--;
					}
					else
					{
						dst = (dst - 1) & (HD63484_RAM_SIZE-1);
						ax++;
					}
				}

				ax = pcx - cpx;
				if (pcy < cpy)
				{
					dst = (dst + 384 - ax) & (HD63484_RAM_SIZE-1);
					if (ay == 0) break;
					ay++;
				}
				else
				{
					dst = (dst - 384 - ax) & (HD63484_RAM_SIZE-1);
					if (ay == 0) break;
					ay--;
				}
			}
		}
//      else if ((fifo[0] & 0xff00) == 0xcc00)  /* DOT */
		else if ((fifo[0] & 0xfff8) == 0xcc00)	/* DOT */
		{
			int dst;

			dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);

			PLOT(dst,fifo[0] & 0x0007)
		}
//      else if ((fifo[0] & 0xf000) == 0xe000)  /* AGCPY */
		else if ((fifo[0] & 0xf0f8) == 0xe000)	/* AGCPY */
		{
			INT16 pcx,pcy;
			int src,dst;

			pcx = fifo[1];
			pcy = fifo[2];

			src = (2*org + pcx - pcy * 384) & (HD63484_RAM_SIZE-1);
			dst = (2*org + cpx - cpy * 384) & (HD63484_RAM_SIZE-1);

			docpy(fifo[0],src,&dst,fifo[3],fifo[4]);

			cpx = (dst - 2*org) % 384;
			cpy = (dst - 2*org) / 384;
		}
		else
{
logerror("unsupported command\n");
popmessage("unsupported command %s (%04x)",instruction_name[fifo[0]>>10],fifo[0]);
}

		fifo_counter = 0;
	}
}

static int regno;

static READ16_HANDLER( HD63484_status_r )
{
	if (activecpu_get_pc() != 0xfced6 && activecpu_get_pc() != 0xfe1d6) logerror("%05x: HD63484 status read\n",activecpu_get_pc());
	return 0xff22|4;	/* write FIFO ready + command end    + read FIFO ready */
}

static WRITE16_HANDLER( HD63484_address_w )
{
	/* only low 8 bits are used */
	if (ACCESSING_BITS_0_7)
		regno = data;
}

static WRITE16_HANDLER( HD63484_data_w )
{
	COMBINE_DATA(&HD63484_reg[regno/2]);
	if (regno & 0x80) regno += 2;	/* autoincrement */
logerror("PC %05x: HD63484 register %02x write %04x\n",activecpu_get_pc(),regno,HD63484_reg[regno/2]);
	if (regno == 0)	/* FIFO */
		HD63484_command_w(HD63484_reg[0]);
}

static READ16_HANDLER( HD63484_data_r )
{
	int res;

	if (regno == 0x80)
		res = video_screen_get_vpos(machine->primary_screen);
	else if (regno == 0)
	{
logerror("%05x: HD63484 read FIFO\n",activecpu_get_pc());
		res = readfifo;
	}
	else
	{
logerror("%05x: HD63484 read register %02x\n",activecpu_get_pc(),regno);
		res = 0;
	}

	return res;
}

static PALETTE_INIT( adp )
{
	int i;


	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (i >> 2) & 0x01;
		bit1 = (i >> 3) & 0x01;
		bit2 = (i >> 4) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (i >> 5) & 0x01;
		bit1 = (i >> 6) & 0x01;
		bit2 = (i >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (i >> 0) & 0x01;
		bit2 = (i >> 1) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static VIDEO_START(adp)
{
	HD63484_start();
}

static VIDEO_UPDATE(adp)
{
	int x,y,b;


	b = 2 * (((HD63484_reg[0xcc/2] & 0x000f) << 16) + HD63484_reg[0xce/2]);
	for (y = 0;y < 280;y++)
	{
		for (x = 0 ; x<384 ; x++)
		{
			b &= (HD63484_RAM_SIZE-1);
			*BITMAP_ADDR16(bitmap, y, x) = HD63484_ram[b];
			b++;
		}
	}

	if ((HD63484_reg[0x06/2] & 0x0300) == 0x0300)
	{
		int sy = (HD63484_reg[0x94/2] & 0x0fff) - (HD63484_reg[0x88/2] >> 8);
		int h = HD63484_reg[0x96/2] & 0x0fff;
		int sx = ((HD63484_reg[0x92/2] >> 8) - (HD63484_reg[0x84/2] >> 8)) * 4;
		int w = (HD63484_reg[0x92/2] & 0xff) * 4;
		if (sx < 0) sx = 0;	/* not sure about this (shangha2 title screen) */

		b = 2 * (((HD63484_reg[0xdc/2] & 0x000f) << 16) + HD63484_reg[0xde/2]);

		for (y = sy ; y<sy+h && y<280 ; y++)
		{
			for (x = 0 ; x < 384 ; x++)
			{
				b &= (HD63484_RAM_SIZE-1);
				if (x <= w && x + sx >= 0 && x+sx < 384)
					*BITMAP_ADDR16(bitmap, y, x+sx) = HD63484_ram[b];

				b++;
			}
		}
	}

	return 0;
}

static ADDRESS_MAP_START( skattv_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800180, 0x80019f) AM_NOP // sound?
	AM_RANGE(0x8000a0, 0x8000a1) AM_READWRITE(HD63484_status_r, HD63484_address_w) // bad
	AM_RANGE(0x8000a2, 0x8000a3) AM_READWRITE(HD63484_data_r, HD63484_data_w) // bad
	AM_RANGE(0x800084, 0xffbfff) AM_RAM // used?
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quickjac_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800080, 0x800081) AM_READWRITE(HD63484_status_r, HD63484_address_w) // bad
	AM_RANGE(0x800082, 0x800083) AM_READWRITE(HD63484_data_r, HD63484_data_w) // bad
	AM_RANGE(0x800084, 0xffbfff) AM_RAM // used?
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( backgamn_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x600000, 0x60001f) AM_NOP // sound?
	AM_RANGE(0x503ffa, 0x503ffb) AM_READWRITE(HD63484_status_r, HD63484_address_w) // bad
	AM_RANGE(0x503ffc, 0x503ffd) AM_READWRITE(HD63484_data_r, HD63484_data_w) // bad
	AM_RANGE(0x800084, 0xffbfff) AM_RAM // used?
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( adp )

INPUT_PORTS_END

static MACHINE_DRIVER_START( quickjac )
	MDRV_CPU_ADD("main", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(quickjac_mem, 0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(adp)
	MDRV_VIDEO_START(adp)
	MDRV_VIDEO_UPDATE(adp)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 3686400/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( skattv )
	MDRV_CPU_ADD("main", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(skattv_mem, 0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(adp)
	MDRV_VIDEO_START(adp)
	MDRV_VIDEO_UPDATE(adp)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 3686400/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( backgamn )
	MDRV_CPU_ADD("main", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(backgamn_mem, 0)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(adp)
	MDRV_VIDEO_START(adp)
	MDRV_VIDEO_UPDATE(adp)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, 3686400/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

ROM_START( quickjac )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "quick_jack_index_a.1.u2.bin", 0x00000, 0x10000, CRC(c2fba6fe) SHA1(f79e5913f9ded1e370cc54dd55860263b9c51d61) )
	ROM_LOAD16_BYTE( "quick_jack_index_a.2.u6.bin", 0x00001, 0x10000, CRC(210cb89b) SHA1(8eac60d40b60e845f9c02fee6c447f125ba5d1ab) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.1.u2.bin", 0x00000, 0x20000, CRC(73c27fc6) SHA1(12429bc0009b7754e08d2b6a5e1cd8251ab66e2d) )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.2.u6.bin", 0x00001, 0x20000, CRC(61d55be2) SHA1(bc17dc91fd1ef0f862eb0d7dbbbfa354a8403eb8) )
ROM_END

ROM_START( skattv )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "f2_i.bin", 0x00000, 0x20000, CRC(3cb8b431) SHA1(e7930876b6cd4cba837c3da05d6948ef9167daea) )
	ROM_LOAD16_BYTE( "f2_ii.bin", 0x00001, 0x20000, CRC(0db1d2d5) SHA1(a29b0299352e0b2b713caf02aa7978f2a4b34e37) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "f1_i.bin", 0x00000, 0x20000, CRC(4869a889) SHA1(ad9f3fcdfd3630f9ad5b93a9d2738de9fc3514d3) )
	ROM_LOAD16_BYTE( "f1_ii.bin", 0x00001, 0x20000, CRC(17681537) SHA1(133685854b2080aaa3d0cced0287bc454d1f3bfc) )
ROM_END

ROM_START( skattva )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.1.u2.bin", 0x00000, 0x20000, CRC(68f82fe8) SHA1(d5f9cb600531cdd748616d8c042b6a151ebe205a) )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.2.u6.bin", 0x00001, 0x20000, CRC(4f927832) SHA1(bbe013005fd00dd42d12939eab5c80ec44a54b71) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.1.u2.bin", 0x00000, 0x20000, CRC(de6f275b) SHA1(0c396fa4d1975c8ccc4967d330b368c0697d2124) )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.2.u5.bin", 0x00001, 0x20000, CRC(af3e60f9) SHA1(c88976ea42cf29a092fdee18377b32ffe91e9f33) )
ROM_END

ROM_START( backgamn )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "b_f2_i.bin", 0x00000, 0x10000, CRC(9e42937c) SHA1(85d462a560b85b03ee9d341e18815b7c396118ac) )
	ROM_LOAD16_BYTE( "b_f2_ii.bin", 0x00001, 0x10000, CRC(8e0ee50c) SHA1(2a05c337db1131b873646aa4109593636ebaa356) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b_f1_i.bin", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "b_f1_ii.bin", 0x00001, 0x20000, NO_DUMP )
ROM_END

ROM_START( fashiong )
	ROM_REGION( 0x100000, "main", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_i.bin", 0x00000, 0x80000, CRC(827a164d) SHA1(dc16380226cabdefbfd893cb50cbfca9e134be40) )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_ii.bin", 0x00001, 0x80000, CRC(5a2466d1) SHA1(c113a2295beed2011c70887a1f2fcdec00b055cb) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_i.bin", 0x00000, 0x80000, CRC(d1ee9133) SHA1(e5fdfa303a3317f8f5fbdc03438ee97415afff4b) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_ii.bin", 0x00001, 0x80000, CRC(07b1e722) SHA1(594cbe9edfea6b04a4e49d1c1594f1c3afeadef5) )

	ROM_REGION( 0x4000, "user1", 0 )
	//nvram - 16 bit
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

GAME( 1990, backgamn,        0, backgamn,    adp,    0, ROT0,  "ADP", "Backgammon", GAME_NOT_WORKING )
GAME( 1993, quickjac,        0, quickjac,    adp,    0, ROT0,  "ADP", "Quick Jack", GAME_NOT_WORKING )
GAME( 1994, skattv,          0, skattv,      adp,    0, ROT0,  "ADP", "Skat TV", GAME_NOT_WORKING )
GAME( 1995, skattva,    skattv, skattv,      adp,    0, ROT0,  "ADP", "Skat TV (version TS3)", GAME_NOT_WORKING )
GAME( 1997, fashiong,        0, skattv,      adp,    0, ROT0,  "ADP", "Fashion Gambler", GAME_NOT_WORKING )

