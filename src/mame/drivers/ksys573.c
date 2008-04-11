/***************************************************************************

  Konami System 573
  ===========================================================
  Driver by R. Belmont & smf

  NOTE: The first time you run each game, it will go through a special initialization
  procedure.  This can be quite lengthy (in the case of Dark Horse Legend).  Let it
  complete all the way before exiting MAME and you will not have to do it again!

  NOTE 2: The first time you run Konami 80's Gallery, it will dump you on a clock
  setting screen.  Press DOWN to select "SAVE AND EXIT" then press player 1 START
  to continue.

  Note 3: If you are asked to insert a different cartridge then use the fake dip
  switch to change it.

  Note 4: Some games require you to press f2 to skip the rtc cleared note.

  TODO:
  * fix root counters in machine/psx.c so the hack here (actually MAME 0.89's machine/psx.c code)
    can be removed
  * integrate ATAPI code with Aaron's ATA/IDE code

  -----------------------------------------------------------------------------------------

  System 573 Hardware Overview
  Konami, 1998-2001

  This system uses Konami PSX-based hardware with an ATAPI CDROM drive.
  There is a slot for a security cart (cart is installed in CN14) and also a PCMCIA card slot,
  which is unused. The main board and CDROM drive are housed in a black metal box.
  The games can be swapped by exchanging the CDROM disc and the security cart, whereby the main-board
  FlashROMs are re-programmed after a small wait. On subsequent power-ups, there is a check to test if the
  contents of the FlashROMs matches the CDROM, then the game boots up immediately.

  The games that run on this system include...

  Game                                  Year    Hardware Code     CD Code
  --------------------------------------------------------------------------
  *Anime Champ                          2000
  Bass Angler                           1998    GE765 JA          765 JA A02
  *Bass Angler 2                        1999
  Dark Horse Legend                     1998    GX706 JA          706 JA A02
  *Dark Horse 2                         ?
  *Fighting Mania                       2000
  Fisherman's Bait                      1998    GE765 UA          765 UA B02
  Fisherman's Bait 2                    1998    GC865 UA          865 UA B02
  Fisherman's Bait Marlin Challenge     1999
  *Gun Mania                            2001
  *Gun Mania Zone Plus                  2001
  *Gachaga Champ                        1999
  *Hyper Bishi Bashi                    1999
  *Hyper Bishi Bashi Champ              2000
  Jikkyo Pawafuru Pro Yakyu             1998    GX802 JA          802 JA B02
  *Kick & Kick                          2001
  Konami 80's Arcade Gallery            1998    GC826 JA          826 JA A01
  Konami 80's AC Special                1998    GC826 UA          826 UA A01
  Salary Man Champ                      2001
  *Step Champ                           1999

  Note:
       Not all games listed above are confirmed to run on System 573.
       * - denotes not dumped yet. If you can help with the remaining undumped System 573 games,
       please contact http://www.mameworld.net/gurudumps/comments.html


  Main PCB Layout
  ---------------
                                                     External controls port
  GX700-PWB(A)B                                               ||
  (C)1997 KONAMI CO. LTD.                                     \/
  |-----------------------------------------------------==============-------|
  |   CN15            CNA                     CN10                           |
  |        CN16                                                              |
  |                                                 |------------------------|
  | PQ30RV21                                        |                        |
  |                         |-------|               |                        |
  |             KM416V256   |SONY   |               |     PCMCIA SLOT        |
  |                         |CXD2925|               |                        |
  |                         |-------|               |                        |
  |                                                 |                        |
  |                                                 |------------------------|
  | |-----|                                        CN21                      |
  | |32M  |  |---------|     |---------|                                     |
  | |-----|  |SONY     |     |SONY     |                                     |
  |          |CXD8561Q |     |CXD8530CQ|           29F016   29F016   |--|    |
  | |-----|  |         |     |         |                             |  |    |
  | |32M  |  |         |     |         |                             |  |    |
  | |-----|  |---------|     |---------|           29F016   29F016   |  |    |
  |      53.693175MHz    67.7376MHz                                  |  |    |
  |                                     |-----|                      |  |CN14|
  |      KM48V514      KM48V514         |9536 |    29F016   29F016   |  |    |
  |            KM48V514       KM48V514  |     |                      |  |    |
  |      KM48V514      KM48V514         |-----|                      |  |    |
  |            KM48V514      KM48V514              29F016   29F016   |--|    |
  | MC44200FT                          M48T58Y-70PC1                         |
  |                                                                      CN12|
  |                                    700A01.22                             |
  |                             14.7456MHz                                   |
  |                  |-------|                                               |
  |                  |KONAMI |    |----|                               LA4705|
  |   058232         |056879 |    |3644|                            SM5877   |
  |                  |       |    |----|         ADC0834                LM358|
  |                  |-------|            ADM485           CN4               |
  |                                                         CN3      CN17    |
  |                                TEST_SW  DIP4 USB   CN8     RCA-L/R   CN9 |
  |--|          JAMMA            |-------------------------------------------|
     |---------------------------|
  Notes:
  CNA       - 40-pin IDE cable connector
  CN3       - 10-pin connector labelled 'ANALOG', connected to a 9-pin DSUB connector mounted in the
              front face of the housing, labelled 'OPTION1'
  CN4       - 12-pin connector labelled 'EXT-OUT'
  CN5       - 10-pin connector labelled 'EXT-IN', connected to a 9-pin DSUB connector mounted in the
              front face of the housing, labelled 'OPTION2'
  CN8       - 15-pin DSUB plug labelled 'VGA-DSUB15' extending from the front face of the housing
              labelled 'RGB'. Use of this connector is optional because the video is output via the
              standard JAMMA connector
  CN9       - 4-pin connector for amplified stereo sound output to 2 speakers
  CN10      - Custom 80-pin connector (for mounting an additional plug-in board for extra controls,
              possibly with CN21 also)
  CN12      - 4-pin CD-DA input connector (for Red-Book audio from CDROM drive to main board)
  CN14      - 44-pin security cartridge connector. The cartridge only contains a small PCB labelled
              'GX700-PWB(D) (C)1997 KONAMI' and has locations for 2 ICs only
              IC1 - Small SOIC8 chip, identified as a XICOR X76F041 security supervisor containing 4X
              128 x8 secureFLASH arrays, stamped '0038323 E9750'
              IC2 - Solder pads for mounting of a PLCC68 or QFP68 packaged IC (not populated)
  CN15      - 4-pin CDROM power connector
  CN16      - 2-pin fan connector
  CN17      - 6-pin power connector, connected to an 8-pin power plug mounted in the front face
              of the housing. This can be left unused because the JAMMA connector supplies all power
              requirements to the PCB
  CN21      - Custom 30-pin connector (purpose unknown, but probably for mounting an additional
              plug-in board with CN10 also)
  TEST_SW   - Push-button test switch
  DIP4      - 4-position DIP switch
  USB       - USB connector extended from the front face of the housing labelled 'I/O'
  RCA-L/R   - RCA connectors for left/right audio output
  PQ30RV21  - Sharp PQ30RV21 low-power voltage regulator (5 Volt to 3 Volt)
  LA4705    - Sanyo LA4705 15W 2-channel power amplifier (SIP18)
  LM358     - National Semiconductor LM358 low power dual operational amplifier (SOIC8, @ 33C)
  CXD2925Q  - Sony CXD2925Q SPU (QFP100, @ 15Q)
  CXD8561Q  - Sony CXD8561Q GTE (QFP208, @ 10M)
  CXD8530CQ - Sony CXD8530CQ R3000-based CPU (QFP208, @ 17M)
  9536      - Xilinx XC9536 in-system-programmable CPLD (PLCC44, @ 22J)
  3644      - Hitachi H8/3644 HD6473644H microcontroller with 32k ROM & 1k RAM (QFP64, @ 18E,
              labelled '700 02 38920')
  056879    - Konami 056879 custom IC (QFP120, @ 13E)
  MC44200FT - Motorola MC44200FT Triple 8-bit Video DAC (QFP44)
  058232    - Konami 058232 custom ceramic IC (SIP14, @ 6C)
  SM5877    - Nippon Precision Circuits SM5877 2-channel D/A convertor (SSOP24, @32D)
  ADM485    - Analog Devices ADM485 low power EIA RS-485 transceiver (SOIC8, @ 20C)
  ADC0834   - National Semiconductor ADC0834 8-Bit Serial I/O A/D Converter with Multiplexer
              Option (SOIC14, @ 24D)
  M48T58Y-70- STMicroelectronics M48T58Y-70PC1 8k x8 Timekeeper RAM (DIP32, @ 22H)
              Note that this is not used for protection. If you put in a new blank Timekeeper RAM
              it will be programmed with some data on power-up. If you swap games, the Timekeeper
              is updated with the new game data
  29F016      Fujitsu 29F016A-90PFTN 2M x8 FlashROM (TSOP48, @ 27H/J/L/M & 31H/J/L/M)
              Also found Sharp LH28F016S (2M x8 TSOP40) in some units
  KM416V256 - Samsung Electronics KM416V256BT-7 256k x 16 DRAM (TSOP44/40, @ 11Q)
  KM48V514  - Samsung Electronics KM48V514BJ-6 512k x 8 EDO DRAM (SOJ28, @ 16G/H, 14G/H, 12G/H, 9G/H)
  32M       - NEC D481850GF-A12 128k x 32Bit x 2 Banks SGRAM (QFP100, @ 4P & 4L)

  Software  -
              - 700A01.22G 4M MaskROM (DIP32, @ 22G)
              - SONY ATAPI CDROM drive, with CDROM disc containing program + graphics + sound
                Some System 573 units contain a CR-583 drive dated October 1997, some contain a
                CR-587 drive dated March 1998. Note that the CR-587 will not read CDR discs ;-)


  Auxillary Controls PCB
  ----------------------

  GE765-PWB(B)A (C)1998 KONAMI CO. LTD.
  |-----------------------------|
  |          CN33     C2242     |
  |                   C2242     |
  |       NRPS11-G1A            |
  |                         CN35|
  |  D4701                      |
  |        74LS14     PC817     |-----------------|
  |                                               |
  |  PAL         PAL                              |
  | (E765B1)    (E765B2)         LCX245           |
  |                                               |
  |  74LS174     PAL                              |
  |             (E765B1)                          |
  |                                               |
  |              74LS174       CN31               |
  |-----------------------------------------------|
  Notes: (all IC's shown)
        This PCB is known to be used for the fishing reel controls on all of the fishing games (at least).

        CN31       - Connector joining this PCB to the MAIN PCB
        CN33       - Connector used to join the external controls connector mounted on the outside of the
                     metal case to this PCB.
        CN35       - Power connector
        NRPS11-G1A - Relay?
        D4701      - NEC uPD4701 Encoder (SOP24)
        C2242      - 2SC2242 Transistor
        PC817      - Sharp PC817 Photo-coupler IC (DIP4)
        PAL        - AMD PALCE16V8Q, stamped 'E765Bx' (DIP20)
*/

#include "driver.h"
#include "deprecat.h"
#include "cdrom.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"
#include "machine/intelfsh.h"
#include "machine/cr589.h"
#include "machine/timekpr.h"
#include "machine/adc083x.h"
#include "machine/ds2401.h"
#include "machine/upd4701.h"
#include "machine/x76f041.h"
#include "machine/x76f100.h"
#include "machine/zs01.h"
#include "sound/psx.h"
#include "sound/cdda.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(2,3) verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		if( cpu_getactivecpu() != -1 )
		{
			logerror( "%08x: %s", activecpu_get_pc(), buf );
		}
		else
		{
			logerror( "(timer) : %s", buf );
		}
	}
}

static INT32 flash_bank;
static int flash_chips;
static int onboard_flash_start;
static int pccard1_flash_start;
static int pccard2_flash_start;
static int pccard3_flash_start;
static int pccard4_flash_start;
static int security_cart_number = 0;

static int chiptype[ 2 ];
static int has_ds2401[ 2 ];

/* EEPROM handlers */

static void (*nvram_handler_security_cart_0)( running_machine *machine, mame_file *file, int read_or_write );
static void (*nvram_handler_security_cart_1)( running_machine *machine, mame_file *file, int read_or_write );

static NVRAM_HANDLER( konami573 )
{
	int i;

	NVRAM_HANDLER_CALL(timekeeper_0);

	if( nvram_handler_security_cart_0 != NULL )
	{
		NVRAM_HANDLER_CALL(security_cart_0);
	}

	if( nvram_handler_security_cart_1 != NULL )
	{
		NVRAM_HANDLER_CALL(security_cart_1);
	}

	for( i = 0; i < flash_chips; i++ )
	{
		nvram_handler_intelflash( machine, i, file, read_or_write );
	}
}

static WRITE32_HANDLER( mb89371_w )
{
	verboselog( 2, "mb89371_w %08x %08x %08x\n", offset, mem_mask, data );
}

static READ32_HANDLER( mb89371_r )
{
	UINT32 data = 0xffffffff;
	verboselog( 2, "mb89371_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

static READ32_HANDLER( jamma_r )
{
	UINT32 data = 0;

	switch (offset)
	{
	case 0:
		data = input_port_read_indexed(machine, 0);
		break;
	case 1:
	{
		data = input_port_read_indexed(machine, 1);
		data |= 0x000000c0;

		if( has_ds2401[ security_cart_number ] )
		{
			data |= ds2401_read( security_cart_number ) << 14;
		}

		data |= adc083x_do_read( 0 ) << 16;

		switch( chiptype[ security_cart_number ] )
		{
		case 1:
			data |= x76f041_sda_read( security_cart_number ) << 18;
			break;

		case 2:
			data |= x76f100_sda_read( security_cart_number ) << 18;
			break;

		case 3:
			data |= zs01_sda_read( security_cart_number ) << 18;
			break;
		}

		if( pccard1_flash_start < 0 )
		{
			data |= ( 1 << 26 );
		}
		if( pccard2_flash_start < 0 )
		{
			data |= ( 1 << 27 );
		}
		break;
	}
	case 2:
		data = input_port_read_indexed(machine, 2);
		break;
	case 3:
		data = input_port_read_indexed(machine, 3);
		break;
	}

	verboselog( 2, "jamma_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

static WRITE32_HANDLER( jamma_w )
{
	verboselog( 2, "jamma_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		adc083x_cs_write( 0, ( data >> 1 ) & 1 );
		adc083x_clk_write( 0, ( data >> 2 ) & 1 );
		adc083x_di_write( 0, ( data >> 0 ) & 1 );
		adc083x_se_write( 0, 0 );
		break;

	default:
		verboselog( 0, "jamma_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data );
		break;
	}
}

static UINT32 control;

static READ32_HANDLER( control_r )
{
	verboselog( 2, "control_r( %08x, %08x ) %08x\n", offset, mem_mask, control );

	return control;
}

static WRITE32_HANDLER( control_w )
{
//  int old_bank = flash_bank;
	COMBINE_DATA(&control);

	verboselog( 2, "control_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	flash_bank = -1;

	switch( chiptype[ security_cart_number ] )
	{
	case 3:
		zs01_sda_write( security_cart_number, !( ( control >> 6 ) & 1 ) ); /* 0x40 */
		break;
	}

	if( onboard_flash_start >= 0 && ( control & ~0x43 ) == 0x00 )
	{
		flash_bank = onboard_flash_start + ( ( control & 3 ) * 2 );
//      if( flash_bank != old_bank ) mame_printf_debug( "onboard %d\r", control & 3 );
	}
	else if( pccard1_flash_start >= 0 && ( control & ~0x47 ) == 0x10 )
	{
		flash_bank = pccard1_flash_start + ( ( control & 7 ) * 2 );
//      if( flash_bank != old_bank ) mame_printf_debug( "pccard1 %d\r", control & 7 );
	}
	else if( pccard2_flash_start >= 0 && ( control & ~0x47 ) == 0x20 )
	{
		flash_bank = pccard2_flash_start + ( ( control & 7 ) * 2 );
//      if( flash_bank != old_bank ) mame_printf_debug( "pccard2 %d\r", control & 7 );
	}
	else if( pccard3_flash_start >= 0 && ( control & ~0x47 ) == 0x20 )
	{
		flash_bank = pccard3_flash_start + ( ( control & 7 ) * 2 );
//      if( flash_bank != old_bank ) mame_printf_debug( "pccard3 %d\r", control & 7 );
	}
	else if( pccard4_flash_start >= 0 && ( control & ~0x47 ) == 0x28 )
	{
		flash_bank = pccard4_flash_start + ( ( control & 7 ) * 2 );
//      if( flash_bank != old_bank ) mame_printf_debug( "pccard4 %d\r", control & 7 );
	}
}

#define ATAPI_CYCLES_PER_SECTOR (5000)	// plenty of time to allow DMA setup etc.  BIOS requires this be at least 2000, individual games may vary.

#define ATAPI_STAT_BSY	   0x80
#define ATAPI_STAT_DRDY    0x40
#define ATAPI_STAT_DMARDDF 0x20
#define ATAPI_STAT_SERVDSC 0x10
#define ATAPI_STAT_DRQ     0x08
#define ATAPI_STAT_CORR    0x04
#define ATAPI_STAT_CHECK   0x01

#define ATAPI_INTREASON_COMMAND 0x01
#define ATAPI_INTREASON_IO      0x02
#define ATAPI_INTREASON_RELEASE 0x04

#define ATAPI_REG_DATA		0
#define ATAPI_REG_ERRFEAT	1
#define ATAPI_REG_INTREASON	2
#define ATAPI_REG_SAMTAG	3
#define ATAPI_REG_COUNTLOW	4
#define ATAPI_REG_COUNTHIGH	5
#define ATAPI_REG_DRIVESEL	6
#define ATAPI_REG_CMDSTATUS	7
#define ATAPI_REG_MAX 16

#define ATAPI_DATA_SIZE ( 64 * 1024 )

static UINT8 *atapi_regs;
static emu_timer *atapi_timer;
static SCSIInstance *inserted_cdrom;
static SCSIInstance *available_cdroms[ 2 ];
static UINT8 *atapi_data;
static int atapi_data_ptr, atapi_data_len, atapi_xferlen, atapi_xferbase, atapi_cdata_wait, atapi_xfermod;

#define MAX_TRANSFER_SIZE ( 63488 )

static TIMER_CALLBACK( atapi_xfer_end )
{
	int i, n_this;
	UINT8 sector_buffer[ 4096 ];

	timer_adjust_oneshot(atapi_timer, attotime_never, 0);

//  verboselog( 2, "atapi_xfer_end( %d ) atapi_xferlen = %d, atapi_xfermod=%d\n", x, atapi_xfermod, atapi_xferlen );

//  mame_printf_debug("ATAPI: xfer_end.  xferlen = %d, atapi_xfermod = %d\n", atapi_xferlen, atapi_xfermod);

	while (atapi_xferlen > 0 )
	{
		// get a sector from the SCSI device
		SCSIReadData( inserted_cdrom, sector_buffer, 2048 );

		atapi_xferlen -= 2048;

		i = 0;
		n_this = 2048 / 4;
		while( n_this > 0 )
		{
			g_p_n_psxram[ atapi_xferbase / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			atapi_xferbase += 4;
			i += 4;
			n_this--;
		}
	}

	if (atapi_xfermod > MAX_TRANSFER_SIZE)
	{
		atapi_xferlen = MAX_TRANSFER_SIZE;
		atapi_xfermod = atapi_xfermod - MAX_TRANSFER_SIZE;
	}
	else
	{
		atapi_xferlen = atapi_xfermod;
		atapi_xfermod = 0;
	}

	if (atapi_xferlen > 0)
	{
		//mame_printf_debug("ATAPI: starting next piece of multi-part transfer\n");
		atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
		atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

		timer_adjust_oneshot(atapi_timer, ATTOTIME_IN_CYCLES((ATAPI_CYCLES_PER_SECTOR * (atapi_xferlen/2048)), 0), 0);
	}
	else
	{
		//mame_printf_debug("ATAPI: Transfer completed, dropping DRQ\n");
		atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
		atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO | ATAPI_INTREASON_COMMAND;
	}

	psx_irq_set(0x400);

	verboselog( 2, "atapi_xfer_end: %d %d\n", atapi_xferlen, atapi_xfermod );
}

static READ32_HANDLER( atapi_r )
{
	int reg, data;

	if (mem_mask == 0xffff0000)	// word-wide command read
	{
//      mame_printf_debug("ATAPI: packet read = %04x\n", atapi_data[atapi_data_ptr]);

		// assert IRQ and drop DRQ
		if (atapi_data_ptr == 0 && atapi_data_len == 0)
		{
			// get the data from the device
			if( atapi_xferlen > 0 )
			{
				SCSIReadData( inserted_cdrom, atapi_data, atapi_xferlen );
				atapi_data_len = atapi_xferlen;
			}

			if (atapi_xfermod > MAX_TRANSFER_SIZE)
			{
				atapi_xferlen = MAX_TRANSFER_SIZE;
				atapi_xfermod = atapi_xfermod - MAX_TRANSFER_SIZE;
			}
			else
			{
				atapi_xferlen = atapi_xfermod;
				atapi_xfermod = 0;
			}

			verboselog( 2, "atapi_r: atapi_xferlen=%d\n", atapi_xferlen );
			if( atapi_xferlen != 0 )
			{
				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}
			else
			{
				//mame_printf_debug("ATAPI: dropping DRQ\n");
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
				atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
			}

			atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
			atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

			psx_irq_set(0x400);
		}

		if( atapi_data_ptr < atapi_data_len )
		{
			data = atapi_data[atapi_data_ptr++];
			data |= ( atapi_data[atapi_data_ptr++] << 8 );
			if( atapi_data_ptr >= atapi_data_len )
			{
//              verboselog( 2, "atapi_r: read all bytes\n" );
				atapi_data_ptr = 0;
				atapi_data_len = 0;

				if( atapi_xferlen == 0 )
				{
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
					psx_irq_set(0x400);
				}
			}
		}
		else
		{
			data = 0;
		}
	}
	else
	{
		int shift;
		reg = offset<<1;
		shift = 0;
		if (mem_mask == 0xff00ffff)
		{
			reg += 1;
			shift = 16;
		}

		data = atapi_regs[reg];

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( 1, "atapi_r: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( 1, "atapi_r: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( 1, "atapi_r: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( 1, "atapi_r: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( 1, "atapi_r: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( 1, "atapi_r: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( 1, "atapi_r: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( 1, "atapi_r: cmdstatus=%02x\n", data );
			break;
		}

//      mame_printf_debug("ATAPI: read reg %d = %x (PC=%x)\n", reg, data, activecpu_get_pc());

		data <<= shift;
	}

	verboselog( 2, "atapi_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( atapi_w )
{
	int reg;

	verboselog( 2, "atapi_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (mem_mask == 0xffff0000)	// word-wide command write
	{
		verboselog( 2, "atapi_w: data=%04x\n", data );

//      mame_printf_debug("ATAPI: packet write %04x\n", data);
		atapi_data[atapi_data_ptr++] = data & 0xff;
		atapi_data[atapi_data_ptr++] = data >> 8;

		if (atapi_cdata_wait)
		{
//          mame_printf_debug("ATAPI: waiting, ptr %d wait %d\n", atapi_data_ptr, atapi_cdata_wait);
			if (atapi_data_ptr == atapi_cdata_wait)
			{
				// send it to the device
				SCSIWriteData( inserted_cdrom, atapi_data, atapi_cdata_wait );

				// assert IRQ
				psx_irq_set(0x400);

				// not sure here, but clear DRQ at least?
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
			}
		}

		else if ( atapi_data_ptr == 12 )
		{
			int phase;

			verboselog( 2, "atapi_w: command %02x\n", atapi_data[0]&0xff );

			// reset data pointer for reading SCSI results
			atapi_data_ptr = 0;
			atapi_data_len = 0;

			// send it to the SCSI device
			SCSISetCommand( inserted_cdrom, atapi_data, 12 );
			SCSIExecCommand( inserted_cdrom, &atapi_xferlen );
			SCSIGetPhase( inserted_cdrom, &phase );

			if (atapi_xferlen != -1)
			{
//              mame_printf_debug("ATAPI: SCSI command %02x returned %d bytes from the device\n", atapi_data[0]&0xff, atapi_xferlen);

				// store the returned command length in the ATAPI regs, splitting into
				// multiple transfers if necessary
				atapi_xfermod = 0;
				if (atapi_xferlen > MAX_TRANSFER_SIZE)
				{
					atapi_xfermod = atapi_xferlen - MAX_TRANSFER_SIZE;
					atapi_xferlen = MAX_TRANSFER_SIZE;
				}

				atapi_regs[ATAPI_REG_COUNTLOW] = atapi_xferlen & 0xff;
				atapi_regs[ATAPI_REG_COUNTHIGH] = (atapi_xferlen>>8)&0xff;

				if (atapi_xferlen == 0)
				{
					// if no data to return, set the registers properly
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO|ATAPI_INTREASON_COMMAND;
				}
				else
				{
					// indicate data ready: set DRQ and DMA ready, and IO in INTREASON
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_SERVDSC;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
				}

				switch( phase )
				{
				case SCSI_PHASE_DATAOUT:
					atapi_cdata_wait = atapi_xferlen;
					break;
				}

				// perform special ATAPI processing of certain commands
				switch (atapi_data[0]&0xff)
				{
					case 0x00: // BUS RESET / TEST UNIT READY
					case 0xbb: // SET CDROM SPEED
						atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
						break;

					case 0x45: // PLAY
						atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_BSY;
						timer_adjust_oneshot( atapi_timer, ATTOTIME_IN_CYCLES( ATAPI_CYCLES_PER_SECTOR, 0 ), 0 );
						break;
				}

				// assert IRQ
				psx_irq_set(0x400);
			}
			else
			{
//              mame_printf_debug("ATAPI: SCSI device returned error!\n");

				atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ | ATAPI_STAT_CHECK;
				atapi_regs[ATAPI_REG_ERRFEAT] = 0x50;	// sense key = ILLEGAL REQUEST
				atapi_regs[ATAPI_REG_COUNTLOW] = 0;
				atapi_regs[ATAPI_REG_COUNTHIGH] = 0;
			}
		}
	}
	else
	{
		reg = offset<<1;
		if (mem_mask == 0xff00ffff)
		{
			reg += 1;
			data >>= 16;
		}

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( 1, "atapi_w: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( 1, "atapi_w: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( 1, "atapi_w: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( 1, "atapi_w: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( 1, "atapi_w: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( 1, "atapi_w: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( 1, "atapi_w: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( 1, "atapi_w: cmdstatus=%02x\n", data );
			break;
		}

		atapi_regs[reg] = data;
//      mame_printf_debug("ATAPI: reg %d = %x (offset %x mask %x PC=%x)\n", reg, data, offset, mem_mask, activecpu_get_pc());

		if (reg == ATAPI_REG_CMDSTATUS)
		{
//          mame_printf_debug("ATAPI command %x issued! (PC=%x)\n", data, activecpu_get_pc());

			switch (data)
			{
				case 0xa0:	// PACKET
		 			atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_COMMAND;

					atapi_data_ptr = 0;
					atapi_data_len = 0;

					/* we have no data */
					atapi_xferlen = 0;
					atapi_xfermod = 0;

					atapi_cdata_wait = 0;
					break;

				case 0xa1:	// IDENTIFY PACKET DEVICE
		 			atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;

					atapi_data_ptr = 0;
					atapi_data_len = 512;

					/* we have no data */
					atapi_xferlen = 0;
					atapi_xfermod = 0;

					memset( atapi_data, 0, atapi_data_len );

					atapi_data[ 0 ^ 1 ] = 0x85;	// ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command
					atapi_data[ 1 ^ 1 ] = 0x00;

					memset( &atapi_data[ 46 ], ' ', 8 );
					atapi_data[ 46 ^ 1 ] = '1';
					atapi_data[ 47 ^ 1 ] = '.';
					atapi_data[ 48 ^ 1 ] = '0';

					memset( &atapi_data[ 54 ], ' ', 40 );
					atapi_data[ 54 ^ 1 ] = 'M';
					atapi_data[ 55 ^ 1 ] = 'A';
					atapi_data[ 56 ^ 1 ] = 'T';
					atapi_data[ 57 ^ 1 ] = 'S';
					atapi_data[ 58 ^ 1 ] = 'H';
					atapi_data[ 59 ^ 1 ] = 'I';
					atapi_data[ 60 ^ 1 ] = 'T';
					atapi_data[ 61 ^ 1 ] = 'A';
					atapi_data[ 62 ^ 1 ] = ' ';
					atapi_data[ 63 ^ 1 ] = 'C';
					atapi_data[ 64 ^ 1 ] = 'R';
					atapi_data[ 65 ^ 1 ] = '-';
					atapi_data[ 66 ^ 1 ] = '5';
					atapi_data[ 67 ^ 1 ] = '8';
					atapi_data[ 68 ^ 1 ] = '9';
					atapi_data[ 69 ^ 1 ] = ' ';

					atapi_data[ 98 ^ 1 ] = 0x04; // IORDY may be disabled
					atapi_data[ 99 ^ 1 ] = 0x00;

					atapi_regs[ATAPI_REG_COUNTLOW] = 0;
					atapi_regs[ATAPI_REG_COUNTHIGH] = 2;

					psx_irq_set(0x400);
					break;

				case 0xef:	// SET FEATURES
		 			atapi_regs[ATAPI_REG_CMDSTATUS] = 0;

					atapi_data_ptr = 0;
					atapi_data_len = 0;

					psx_irq_set(0x400);
					break;

				default:
					mame_printf_debug("ATAPI: Unknown IDE command %x\n", data);
					break;
			}
		}
	 }
}

static void atapi_exit(running_machine* machine)
{
	int i;

	for( i = 0; i < 2; i++ )
	{
		if( get_disk_handle( i ) != NULL )
		{
			SCSIDeleteInstance( available_cdroms[ i ] );
		}
	}
}

static void atapi_init(running_machine *machine)
{
	int i;

	atapi_regs = auto_malloc( ATAPI_REG_MAX );
	memset(atapi_regs, 0, sizeof(atapi_regs));

	atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
	atapi_regs[ATAPI_REG_ERRFEAT] = 1;
	atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
	atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

	atapi_data_ptr = 0;
	atapi_data_len = 0;
	atapi_cdata_wait = 0;

	atapi_timer = timer_alloc( atapi_xfer_end , NULL);
	timer_adjust_oneshot(atapi_timer, attotime_never, 0);

	for( i = 0; i < 2; i++ )
	{
		if( get_disk_handle( i ) != NULL )
		{
			SCSIAllocInstance( &SCSIClassCr589, &available_cdroms[ i ], i );
		}
		else
		{
			available_cdroms[ i ] = NULL;
		}
	}
	add_exit_callback(machine, atapi_exit);

	atapi_data = auto_malloc( ATAPI_DATA_SIZE );

	state_save_register_global_pointer( atapi_regs, ATAPI_REG_MAX );
	state_save_register_global_pointer( atapi_data, ATAPI_DATA_SIZE / 2 );
	state_save_register_global( atapi_data_ptr );
	state_save_register_global( atapi_data_len );
	state_save_register_global( atapi_xferlen );
	state_save_register_global( atapi_xferbase );
	state_save_register_global( atapi_cdata_wait );
	state_save_register_global( atapi_xfermod );
}

static WRITE32_HANDLER( atapi_reset_w )
{
	verboselog( 2, "atapi_reset_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (data)
	{
		verboselog( 2, "atapi_reset_w: reset\n" );

//      mame_printf_debug("ATAPI reset\n");

		atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
		atapi_regs[ATAPI_REG_ERRFEAT] = 1;
		atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
		atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

		atapi_data_ptr = 0;
		atapi_data_len = 0;
		atapi_cdata_wait = 0;

		atapi_xferlen = 0;
		atapi_xfermod = 0;
	}
}

static void cdrom_dma_read( UINT32 n_address, INT32 n_size )
{
	verboselog( 2, "cdrom_dma_read( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA read: address %08x size %08x\n", n_address, n_size);
}

static void cdrom_dma_write( UINT32 n_address, INT32 n_size )
{
	verboselog( 2, "cdrom_dma_write( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA write: address %08x size %08x\n", n_address, n_size);

	atapi_xferbase = n_address;

	verboselog( 2, "atapi_xfer_end: %d %d\n", atapi_xferlen, atapi_xfermod );

	// set a transfer complete timer (Note: CYCLES_PER_SECTOR can't be lower than 2000 or the BIOS ends up "out of order")
	timer_adjust_oneshot(atapi_timer, ATTOTIME_IN_CYCLES((ATAPI_CYCLES_PER_SECTOR * (atapi_xferlen/2048)), 0), 0);
}

static UINT32 m_n_security_control;
static void (*security_bit7_write)( int data );
static void (*security_bit6_write)( int data );
static void (*security_bit5_write)( int data );

static WRITE32_HANDLER( security_w )
{
	COMBINE_DATA( &m_n_security_control );

	verboselog( 2, "security_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( ACCESSING_BITS_0_15 )
	{
		switch( chiptype[ security_cart_number ] )
		{
		case 1:
			x76f041_sda_write( security_cart_number, ( data >> 0 ) & 1 );
			x76f041_scl_write( security_cart_number, ( data >> 1 ) & 1 );
			x76f041_cs_write( security_cart_number, ( data >> 2 ) & 1 );
			x76f041_rst_write( security_cart_number, ( data >> 3 ) & 1 );
			break;

		case 2:
			x76f100_sda_write( security_cart_number, ( data >> 0 ) & 1 );
			x76f100_scl_write( security_cart_number, ( data >> 1 ) & 1 );
			x76f100_cs_write( security_cart_number, ( data >> 2 ) & 1 );
			x76f100_rst_write( security_cart_number, ( data >> 3 ) & 1 );
			break;

		case 3:
			zs01_scl_write( security_cart_number, ( data >> 1 ) & 1 );
			zs01_cs_write( security_cart_number, ( data >> 2 ) & 1 );
			zs01_rst_write( security_cart_number, ( data >> 3 ) & 1 );
			break;
		}

		if( has_ds2401[ security_cart_number ] )
		{
			ds2401_write( security_cart_number, !( ( data >> 4 ) & 1 ) );
		}

		if( security_bit5_write != NULL )
		{
			security_bit5_write( ( data >> 5 ) & 1 );
		}

		if( security_bit6_write != NULL )
		{
			security_bit6_write( ( data >> 6 ) & 1 );
		}

		if( security_bit7_write != NULL )
		{
			security_bit7_write( ( data >> 7 ) & 1 );
		}
	}
}

static READ32_HANDLER( security_r )
{
	UINT32 data = m_n_security_control;
	verboselog( 2, "security_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static READ32_HANDLER( flash_r )
{
	UINT32 data = 0;

	if( flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_r( %08x, %08x ) no bank selected %08x\n", activecpu_get_pc(), offset, mem_mask, control );
		data = 0xffffffff;
	}
	else
	{
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			data |= ( intelflash_read( flash_bank + 0, adr + 0 ) & 0xff ) << 0; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_8_15 )
		{
			data |= ( intelflash_read( flash_bank + 1, adr + 0 ) & 0xff ) << 8; // 27m/27l/27j/27h
		}
		if( ACCESSING_BITS_16_23 )
		{
			data |= ( intelflash_read( flash_bank + 0, adr + 1 ) & 0xff ) << 16; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_24_31 )
		{
			data |= ( intelflash_read( flash_bank + 1, adr + 1 ) & 0xff ) << 24; // 27m/27l/27j/27h
		}
	}

	verboselog( 2, "flash_r( %08x, %08x, %08x)\n", offset, mem_mask, data );

	return data;
}

static WRITE32_HANDLER( flash_w )
{
	verboselog( 2, "flash_w( %08x, %08x, %08x\n", offset, mem_mask, data );

	if( flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_w( %08x, %08x, %08x ) no bank selected %08x\n", activecpu_get_pc(), offset, mem_mask, data, control );
	}
	else
	{
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			intelflash_write( flash_bank + 0, adr + 0, ( data >> 0 ) & 0xff );
		}
		if( ACCESSING_BITS_8_15 )
		{
			intelflash_write( flash_bank + 1, adr + 0, ( data >> 8 ) & 0xff );
		}
		if( ACCESSING_BITS_16_23 )
		{
			intelflash_write( flash_bank + 0, adr + 1, ( data >> 16 ) & 0xff );
		}
		if( ACCESSING_BITS_24_31 )
		{
			intelflash_write( flash_bank + 1, adr + 1, ( data >> 24 ) & 0xff );
		}
	}
}

/* Root Counters */

static emu_timer *m_p_timer_root[ 3 ];
static UINT16 m_p_n_root_count[ 3 ];
static UINT16 m_p_n_root_mode[ 3 ];
static UINT16 m_p_n_root_target[ 3 ];
static UINT64 m_p_n_root_start[ 3 ];

#define RC_STOP ( 0x01 )
#define RC_RESET ( 0x04 ) /* guess */
#define RC_COUNTTARGET ( 0x08 )
#define RC_IRQTARGET ( 0x10 )
#define RC_IRQOVERFLOW ( 0x20 )
#define RC_REPEAT ( 0x40 )
#define RC_CLC ( 0x100 )
#define RC_DIV ( 0x200 )

static UINT64 psxcpu_gettotalcycles( void )
{
	/* TODO: should return the start of the current tick. */
	return cpunum_gettotalcycles(0) * 2;
}

static int root_divider( int n_counter )
{
	if( n_counter == 0 && ( m_p_n_root_mode[ n_counter ] & RC_CLC ) != 0 )
	{
		/* TODO: pixel clock, probably based on resolution */
		return 5;
	}
	else if( n_counter == 1 && ( m_p_n_root_mode[ n_counter ] & RC_CLC ) != 0 )
	{
		return 2150;
	}
	else if( n_counter == 2 && ( m_p_n_root_mode[ n_counter ] & RC_DIV ) != 0 )
	{
		return 8;
	}
	return 1;
}

static UINT16 root_current( int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_STOP ) != 0 )
	{
		return m_p_n_root_count[ n_counter ];
	}
	else
	{
		UINT64 n_current;
		n_current = psxcpu_gettotalcycles() - m_p_n_root_start[ n_counter ];
		n_current /= root_divider( n_counter );
		n_current += m_p_n_root_count[ n_counter ];
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			m_p_n_root_count[ n_counter ] = n_current;
			m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles();
		}
		return n_current;
	}
}

static int root_target( int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_COUNTTARGET ) != 0 ||
		( m_p_n_root_mode[ n_counter ] & RC_IRQTARGET ) != 0 )
	{
		return m_p_n_root_target[ n_counter ];
	}
	return 0x10000;
}

static void root_timer_adjust( int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_STOP ) != 0 )
	{
		timer_adjust_oneshot( m_p_timer_root[ n_counter ], attotime_never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( n_counter ) - root_current( n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( n_counter );

		timer_adjust_oneshot( m_p_timer_root[ n_counter ], attotime_mul(ATTOTIME_IN_HZ(33868800), n_duration), n_counter);
	}
}

static TIMER_CALLBACK( root_finished )
{
	int n_counter = param;

//  if( ( m_p_n_root_mode[ n_counter ] & RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as RC_COUNTTARGET & RC_IRQTARGET don't have to be the same. */
		m_p_n_root_count[ n_counter ] = 0;
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles();
	}
	if( ( m_p_n_root_mode[ n_counter ] & RC_REPEAT ) != 0 )
	{
		root_timer_adjust( n_counter );
	}
	if( ( m_p_n_root_mode[ n_counter ] & RC_IRQOVERFLOW ) != 0 ||
		( m_p_n_root_mode[ n_counter ] & RC_IRQTARGET ) != 0 )
	{
		psx_irq_set( 0x10 << n_counter );
	}
}

static WRITE32_HANDLER( k573_counter_w )
{
	int n_counter;

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		m_p_n_root_count[ n_counter ] = data;
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles();
		break;
	case 1:
		m_p_n_root_count[ n_counter ] = root_current( n_counter );
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles();
		m_p_n_root_mode[ n_counter ] = data;

		if( ( m_p_n_root_mode[ n_counter ] & RC_RESET ) != 0 )
		{
			/* todo: check this is correct */
			m_p_n_root_count[ n_counter ] = 0;
			m_p_n_root_mode[ n_counter ] &= ~( RC_STOP | RC_RESET );
		}
//      if( ( data & 0xfca6 ) != 0 ||
//          ( ( data & 0x0100 ) != 0 && n_counter != 0 && n_counter != 1 ) ||
//          ( ( data & 0x0200 ) != 0 && n_counter != 2 ) )
//      {
//          printf( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
//      }
		break;
	case 2:
		m_p_n_root_target[ n_counter ] = data;
		break;
	default:
		return;
	}

	root_timer_adjust( n_counter );
}

static READ32_HANDLER( k573_counter_r )
{
	int n_counter;
	UINT32 data;

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( n_counter );
		break;
	case 1:
		data = m_p_n_root_mode[ n_counter ];
		break;
	case 2:
		data = m_p_n_root_target[ n_counter ];
		break;
	default:
		return 0;
	}
	return data;
}

static ADDRESS_MAP_START( konami573_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE(1) AM_BASE(&g_p_n_psxram) AM_SIZE(&g_n_psxramsize) /* ram */
	AM_RANGE(0x1f000000, 0x1f3fffff) AM_READWRITE( flash_r, flash_w )
	AM_RANGE(0x1f400000, 0x1f40001f) AM_READWRITE( jamma_r, jamma_w )
	AM_RANGE(0x1f480000, 0x1f48000f) AM_READWRITE( atapi_r, atapi_w )	// IDE controller, used mostly in ATAPI mode (only 3 pure IDE commands seen so far)
	AM_RANGE(0x1f500000, 0x1f500003) AM_READWRITE( control_r, control_w )	// Konami can't make a game without a "control" register.
	AM_RANGE(0x1f560000, 0x1f560003) AM_WRITE( atapi_reset_w )
	AM_RANGE(0x1f5c0000, 0x1f5c0003) AM_WRITENOP 				// watchdog?
	AM_RANGE(0x1f620000, 0x1f623fff) AM_READWRITE( timekeeper_0_32le_lsb16_r, timekeeper_0_32le_lsb16_w )
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f6a0000, 0x1f6a0003) AM_READWRITE( security_r, security_w )
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_READ(psx_spu_delay_r)
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(k573_counter_r, k573_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_READWRITE(psx_spu_r, psx_spu_w)
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE(2) AM_REGION(REGION_USER1, 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE(1) /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE(2) /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE(1) /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE(2) /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END


static void flash_init( void )
{
	int i;
	int chip;
	int size;
	UINT8 *data;
	static const struct
	{
		int *start;
		int region;
		int chips;
		int type;
		int size;
	}
	flash_init[] =
	{
		{ &onboard_flash_start, REGION_USER3,  8, FLASH_FUJITSU_29F016A, 0x200000 },
		{ &pccard1_flash_start, REGION_USER4, 16, FLASH_FUJITSU_29F016A, 0x200000 },
		{ &pccard2_flash_start, REGION_USER5, 16, FLASH_FUJITSU_29F016A, 0x200000 },
		{ &pccard3_flash_start, REGION_USER6, 16, FLASH_FUJITSU_29F016A, 0x200000 },
		{ &pccard4_flash_start, REGION_USER7, 16, FLASH_FUJITSU_29F016A, 0x200000 },
		{ NULL, 0, 0, 0, 0 },
	};

	flash_chips = 0;

	i = 0;
	while( flash_init[ i ].start != NULL )
	{
		data = memory_region( flash_init[ i ].region );
		if( data != NULL )
		{
			size = 0;
			*( flash_init[ i ].start ) = flash_chips;
			for( chip = 0; chip < flash_init[ i ].chips; chip++ )
			{
				intelflash_init( flash_chips, flash_init[ i ].type, data + size );
				size += flash_init[ i ].size;
				flash_chips++;
			}
			if( size != memory_region_length( flash_init[ i ].region ) )
			{
				fatalerror( "flash_init %d incorrect region length\n", i );
			}
		}
		else
		{
			*( flash_init[ i ].start ) = -1;
		}
		i++;
	}

	state_save_register_global( flash_bank );
	state_save_register_global( control );
}

static double analogue_inputs_callback( int input )
{
	switch( input )
	{
	case ADC083X_CH0:
		return (double) ( input_port_read_safe(Machine,  "analog0", 0 ) * 5 ) / 255;
	case ADC083X_CH1:
		return (double) ( input_port_read_safe(Machine,  "analog1", 0 ) * 5 ) / 255;
	case ADC083X_CH2:
		return (double) ( input_port_read_safe(Machine,  "analog2", 0 ) * 5 ) / 255;
	case ADC083X_CH3:
		return (double) ( input_port_read_safe(Machine,  "analog3", 0 ) * 5 ) / 255;
	case ADC083X_AGND:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 0;
}

static void *atapi_get_device(void)
{
	void *ret;
	SCSIGetDevice( inserted_cdrom, &ret );
	return ret;
}

static void security_cart_init( int cart, int eeprom_region, int ds2401_region )
{
	UINT8 *eeprom_rom = memory_region( eeprom_region );
	int eeprom_length = memory_region_length( eeprom_region );
	UINT8 *ds2401_rom = memory_region( ds2401_region );

	if( eeprom_rom != NULL )
	{
		switch( eeprom_length )
		{
		case 0x224:
			x76f041_init( cart, eeprom_rom );
			chiptype[ cart ] = 1;

			switch( cart )
			{
			case 0:
				nvram_handler_security_cart_0 = nvram_handler_x76f041_0;
				break;
			case 1:
				nvram_handler_security_cart_1 = nvram_handler_x76f041_1;
				break;
			}

			break;

		case 0x84:
			x76f100_init( cart, eeprom_rom );
			chiptype[ cart ] = 2;

			switch( cart )
			{
			case 0:
				nvram_handler_security_cart_0 = nvram_handler_x76f100_0;
				break;
			case 1:
				nvram_handler_security_cart_1 = nvram_handler_x76f100_1;
				break;
			}

			break;

		case 0x1014:
			zs01_init( cart, eeprom_rom, NULL, NULL, ds2401_rom );
			chiptype[ cart ] = 3;

			switch( cart )
			{
			case 0:
				nvram_handler_security_cart_0 = nvram_handler_zs01_0;
				break;
			case 1:
				nvram_handler_security_cart_1 = nvram_handler_zs01_1;
				break;
			}

			break;

		default:
			fatalerror( "security_cart_init(%d) invalid eeprom size %d\n", cart, eeprom_length );
			break;
		}
	}
	else
	{
		chiptype[ cart ] = 0;
	}

	if( chiptype[ cart ] != 3 && ds2401_rom != NULL )
	{
		ds2401_init( cart, ds2401_rom );
		has_ds2401[ cart ] = 1;
	}
	else
	{
		has_ds2401[ cart ] = 0;
	}
}

static DRIVER_INIT( konami573 )
{
	int i;

	psx_driver_init(machine);
	atapi_init(machine);
	psx_dma_install_read_handler(5, cdrom_dma_read);
	psx_dma_install_write_handler(5, cdrom_dma_write);

	for (i = 0; i < 3; i++)
	{
		m_p_timer_root[i] = timer_alloc(root_finished, NULL);
	}

	timekeeper_init( 0, TIMEKEEPER_M48T58, memory_region( REGION_USER11 ) );

	state_save_register_global( m_n_security_control );

	security_cart_init( 0, REGION_USER2, REGION_USER9 );
	security_cart_init( 1, REGION_USER8, REGION_USER10 );

	state_save_register_item_array( "KSYS573", 0, m_p_n_root_count );
	state_save_register_item_array( "KSYS573", 0, m_p_n_root_mode );
	state_save_register_item_array( "KSYS573", 0, m_p_n_root_target );
	state_save_register_item_array( "KSYS573", 0, m_p_n_root_start );

	adc083x_init( 0, ADC0834, analogue_inputs_callback );
	flash_init();
}

static MACHINE_RESET( konami573 )
{
	psx_machine_init();

	if( chiptype[ 0 ] != 0 )
	{
		/* security cart */
		psx_sio_input( 1, PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );
	}

	flash_bank = -1;
}

static const struct PSXSPUinterface konami573_psxspu_interface =
{
	&g_p_n_psxram,
	psx_irq_set,
	psx_dma_install_read_handler,
	psx_dma_install_write_handler
};

static void update_mode( running_machine *machine )
{
	int cart = input_port_read(machine,  "CART" );
	int cd = input_port_read(machine,  "CD" );
	static SCSIInstance *new_cdrom;

	if( chiptype[ 1 ] != 0 )
	{
		security_cart_number = cart;
	}
	else
	{
		security_cart_number = 0;
	}

	if( available_cdroms[ 1 ] != NULL )
	{
		new_cdrom = available_cdroms[ cd ];
	}
	else
	{
		new_cdrom = available_cdroms[ 0 ];
	}

	if( inserted_cdrom != new_cdrom )
	{
		inserted_cdrom = new_cdrom;
		cdda_set_cdrom(0, atapi_get_device());
	}
}

static INTERRUPT_GEN( sys573_vblank )
{
	update_mode(machine);

	if( strcmp( machine->gamedrv->name, "ddr2ml" ) == 0 )
	{
		/* patch out security-plate error */

		/* 8001f850: jal $8003221c */
		if( g_p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c887 )
		{
			/* 8001f850: j $8001f888 */
			g_p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}

	psx_vblank(machine, cpunum);
}

/*
GE765-PWB(B)A

todo:
  find out what offset 4 is
  fix reel type detection
  find adc0834 SARS

*/

static READ32_HANDLER( ge765pwbba_r )
{
	UINT32 data = 0;

	switch( offset )
	{
	case 0x26:
		uPD4701_y_add( 0, input_port_read_safe(machine,  "uPD4701_y", 0 ) );
		uPD4701_switches_set( 0, input_port_read_safe(machine,  "uPD4701_switches", 0 ) );

		uPD4701_cs_w( 0, 0 );
		uPD4701_xy_w( 0, 1 );

		if( ACCESSING_BITS_0_7 )
		{
			uPD4701_ul_w( 0, 0 );
			data |= uPD4701_d_r( 0 ) << 0;
		}

		if( ACCESSING_BITS_16_23 )
		{
			uPD4701_ul_w( 0, 1 );
			data |= uPD4701_d_r( 0 ) << 16;
		}

		uPD4701_cs_w( 0, 1 );
		break;

	default:
		verboselog( 0, "ge765pwbba_r: unhandled offset %08x %08x\n", offset, mem_mask );
		break;
	}

	verboselog( 2, "ge765pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( ge765pwbba_w )
{
	switch( offset )
	{
	case 0x04:
		break;

	case 0x20:
		if( ACCESSING_BITS_0_7 )
		{
			output_set_value( "motor", data & 0xff );
		}
		break;

	case 0x22:
		if( ACCESSING_BITS_0_7 )
		{
			output_set_value( "brake", data & 0xff );
		}
		break;

	case 0x28:
		if( ACCESSING_BITS_0_7 )
		{
			uPD4701_resety_w( 0, 1 );
			uPD4701_resety_w( 0, 0 );
		}
		break;

	default:
		verboselog( 0, "ge765pwbba_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data );
		break;
	}

	verboselog( 2, "ge765pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
}

static DRIVER_INIT( ge765pwbba )
{
	DRIVER_INIT_CALL(konami573);

	uPD4701_init( 0 );

	memory_install_read32_handler ( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, ge765pwbba_r );
	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, ge765pwbba_w );
}

/*

GX700-PWB(F)

Analogue I/O board

*/

static UINT8 gx700pwbf_output_data[ 4 ];
static void (*gx700pwfbf_output_callback)( int offset, int data );

static READ32_HANDLER( gx700pwbf_io_r )
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0x20:
		/* result not used? */
		break;

	case 0x22:
		/* result not used? */
		break;

	case 0x24:
		/* result not used? */
		break;

	case 0x26:
		/* result not used? */
		break;

	default:
//      printf( "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}

	verboselog( 2, "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

static void gx700pwbf_output( int offset, UINT8 data )
{
	if( gx700pwfbf_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 7, 6, 1, 0, 5, 4, 3, 2 };
		for( i = 0; i < 8; i++ )
		{
			int oldbit = ( gx700pwbf_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				gx700pwfbf_output_callback( ( offset * 8 ) + i, newbit );
			}
		}
	}
	gx700pwbf_output_data[ offset ] = data;
}

static WRITE32_HANDLER( gx700pwbf_io_w )
{
	verboselog( 2, "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0x20:

		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( 0, data & 0xff );
		}
		break;

	case 0x22:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( 1, data & 0xff );
		}
		break;

	case 0x24:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( 2, data & 0xff );
		}
		break;

	case 0x26:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( 3, data & 0xff );
		}
		break;

	default:
//      printf( "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
		break;
	}
}

static void gx700pwfbf_init( void (*output_callback_func)( int offset, int data ) )
{
	memset( gx700pwbf_output_data, 0, sizeof( gx700pwbf_output_data ) );

	gx700pwfbf_output_callback = output_callback_func;

	memory_install_read32_handler ( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, gx700pwbf_io_r );
	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, gx700pwbf_io_w );

	state_save_register_global_array( gx700pwbf_output_data );
}

/*

GN845-PWB(B)

DDR Stage Multiplexor

*/

static UINT32 stage_mask = 0xffffffff;

#define DDR_STAGE_IDLE ( 0 )
#define DDR_STAGE_INIT ( 1 )

static struct
{
	int DO;
	int clk;
	int shift;
	int state;
	int bit;
} stage[ 2 ];

static const int mask[] =
{
	0, 6, 2, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 6
};

static void gn845pwbb_do_w( int offset, int data )
{
	stage[ offset ].DO = !data;
}

static void gn845pwbb_clk_w( int offset, int data )
{
	int clk = !data;

	if( clk != stage[ offset ].clk )
	{
		stage[ offset ].clk = clk;

		if( clk )
		{
			stage[ offset ].shift = ( stage[ offset ].shift >> 1 ) | ( stage[ offset ].DO << 12 );

			switch( stage[ offset ].state )
			{
			case DDR_STAGE_IDLE:
				if( stage[ offset ].shift == 0xc90 )
				{
					stage[ offset ].state = DDR_STAGE_INIT;
					stage[ offset ].bit = 0;
					stage_mask = 0xfffff9f9;
				}
				break;

			case DDR_STAGE_INIT:
				stage[ offset ].bit++;
				if( stage[ offset ].bit < 22 )
				{
					int a = ( ( ( ( ~0x06 ) | mask[ stage[ 0 ].bit ] ) & 0xff ) << 8 );
					int b = ( ( ( ( ~0x06 ) | mask[ stage[ 1 ].bit ] ) & 0xff ) << 0 );

					stage_mask = 0xffff0000 | a | b;
				}
				else
				{
					stage[ offset ].bit = 0;
					stage[ offset ].state = DDR_STAGE_IDLE;

					stage_mask = 0xffffffff;
				}
				break;
			}
		}
	}

	verboselog( 2, "stage: %dp data clk=%d state=%d d0=%d shift=%08x bit=%d stage_mask=%08x\n", offset + 1, clk, stage[ offset ].state, stage[ offset ].DO, stage[ offset ].shift, stage[ offset ].bit, stage_mask );
}

static CUSTOM_INPUT( gn845pwbb_read )
{
	return input_port_read(machine,  "STAGE" ) & stage_mask;
}

static void gn845pwbb_output_callback( int offset, int data )
{
	switch( offset )
	{
	case 0:
		output_set_value( "foot 1p up", !data );
		break;

	case 1:
		output_set_value( "foot 1p left", !data );
		break;

	case 2:
		output_set_value( "foot 1p right", !data );
		break;

	case 3:
		output_set_value( "foot 1p down", !data );
		break;

	case 4:
		gn845pwbb_do_w( 0, !data );
		break;

	case 7:
		gn845pwbb_clk_w( 0, !data );
		break;

	case 8:
		output_set_value( "foot 2p up", !data );
		break;

	case 9:
		output_set_value( "foot 2p left", !data );
		break;

	case 10:
		output_set_value( "foot 2p right", !data );
		break;

	case 11:
		output_set_value( "foot 2p down", !data );
		break;

	case 12:
		gn845pwbb_do_w( 1, !data );
		break;

	case 15:
		gn845pwbb_clk_w( 1, !data );
		break;

	case 17:
		output_set_led_value( 0, !data ); // start 1
		break;

	case 18:
		output_set_led_value( 1, !data ); // start 2
		break;

	case 20:
		output_set_value( "body right low", !data );
		break;

	case 21:
		output_set_value( "body left low", !data );
		break;

	case 22:
		output_set_value( "body left high", !data );
		break;

	case 23:
		output_set_value( "body right high", !data );
		break;

	case 28: // digital
	case 30: // analogue
		output_set_value( "speaker", !data );
		break;

	default:
//        printf( "%d=%d\n", offset, data );
		break;
	}
}

static DRIVER_INIT( ddr )
{
	DRIVER_INIT_CALL(konami573);

	gx700pwfbf_init( gn845pwbb_output_callback );

	state_save_register_global( stage_mask );
}

/*

Guitar Freaks

todo:
  find out what offset 4 is
  find out the pcb id

*/

static READ32_HANDLER( gtrfrks_io_r )
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0:
		break;

	default:
		verboselog( 0, "gtrfrks_io_r: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}

	verboselog( 2, "gtrfrks_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( gtrfrks_io_w )
{
	verboselog( 2, "gtrfrks_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_value( "spot left", !( ( data >> 7 ) & 1 ) );
		output_set_value( "spot right", !( ( data >> 6 ) & 1 ) );
		output_set_led_value( 0, !( ( data >> 5 ) & 1 ) ); // start left
		output_set_led_value( 1, !( ( data >> 4 ) & 1 ) ); // start right
		break;

	case 4:
		break;

	default:
		verboselog( 0, "gtrfrks_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

static DRIVER_INIT( gtrfrks )
{
	DRIVER_INIT_CALL(konami573);

	memory_install_read32_handler ( 0, ADDRESS_SPACE_PROGRAM, 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_r );
	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_w );
}

/* GX894 digital i/o */

static const UINT8 ds2401_xid[] =
{
	0x3d, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0x01
};

static UINT32 gx894_ram_write_offset;
static UINT32 gx894_ram_read_offset;
static UINT16 *gx894_ram;

static READ32_HANDLER( gx894pwbba_r )
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0x00:
		data |= 0x10000;
		break;
	case 0x20:
		if( ACCESSING_BITS_0_15 )
		{
			data |= 0x00001234;
		}
		break;
	case 0x2b:
		/* sound? */
		if( ACCESSING_BITS_0_15 )
		{
//          data |= 0x00001000; /* ? */
			data |= 0x00002000; /* ? */
		}
		if( ACCESSING_BITS_16_31 )
		{
//          data |= 0x10000000; /* rdy??? */
		}
		break;
	case 0x2d:
		if( ACCESSING_BITS_0_15 )
		{
			data |= gx894_ram[ gx894_ram_read_offset / 2 ];
//          printf( "reading %08x %04x\r", gx894_ram_read_offset, gx894_ram[ gx894_ram_read_offset / 2 ] );
			gx894_ram_read_offset += 2;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "read offset 2d msw32\n" );
		}
		break;
	case 0x30:
		/* mp3? */
		if( ACCESSING_BITS_0_15 )
		{
			/* unknown data word */
		}
		if( ACCESSING_BITS_16_31 )
		{
			/* 0x000-0x1ff */
			data |= 0x1ff0000;
		}
		break;
	case 0x31:
		/* mp3? */
		if( ACCESSING_BITS_0_15 )
		{
			/* unknown data word count */
			data |= 0x0000;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "read offset 31 msw32\n" );
		}
		break;
	case 0x32:
		if( ACCESSING_BITS_16_31 )
		{
			data |= 0 & 0xffff0000;
		}
		/* todo */
		break;
	case 0x33:
		if( ACCESSING_BITS_0_15 )
		{
			data |= 0 & 0x0000ffff;
		}
		/* todo */
		break;
	case 0x3b:
		if( ACCESSING_BITS_16_31 )
		{
			data |= ds2401_read( 2 ) << 28;
		}
		break;
	case 0x3d:
		if( ACCESSING_BITS_16_31 )
		{
			/* fails if !8000 */
			/* fails if  4000 */
			/* fails if !2000 */
			/* fails if !1000 */
			data |= ( 0x8000 | 0x2000 | 0x1000 ) << 16;
		}
		break;
	default:
//      printf( "read offset %08x\n", offset );
		break;
	}

	verboselog( 2, "gx894pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
//  printf( "%08x: gx894pwbba_r( %08x, %08x ) %08x\n", activecpu_get_pc(), offset, mem_mask, data );
	return data;
}

static char *binary( UINT32 data )
{
	static char s[ 33 ];
	int i;
	for( i = 0; i < 32; i++ )
	{
		s[ i ] = '0' + ( ( data >> ( 31 - i ) ) & 1 );
	}
	s[ i ] = 0;
	return s;
}

static UINT32 a,b,c,d;

static UINT16 gx894pwbba_output_data[ 8 ];
static void (*gx894pwbba_output_callback)( int offset, int data );

static void gx894pwbba_output( int offset, UINT8 data )
{
	if( gx894pwbba_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 0, 2, 3, 1 };
		for( i = 0; i < 4; i++ )
		{
			int oldbit = ( gx894pwbba_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				gx894pwbba_output_callback( ( offset * 4 ) + i, newbit );
			}
		}
	}
	gx894pwbba_output_data[ offset ] = data;
}

static WRITE32_HANDLER( gx894pwbba_w )
{
	UINT32 olda=a,oldb=b,oldc=c,oldd=d;

//  printf( "gx894pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( offset == 4 )
	{
		return;
	}

	verboselog( 2, "gx894pwbba_w( %08x, %08x, %08x) %s\n", offset, mem_mask, data, binary( data ) );

	switch( offset )
	{
	case 0x2b:
		/* sound? */
		break;
	case 0x2c:
		if( ACCESSING_BITS_0_15 )
		{
			gx894_ram_write_offset &= 0x0000ffff;
			gx894_ram_write_offset |= ( data & 0x0000ffff ) << 16;
		}
		if( ACCESSING_BITS_16_31 )
		{
			gx894_ram_write_offset &= 0xffff0000;
			gx894_ram_write_offset |= ( data & 0xffff0000 ) >> 16;
		}
		break;
	case 0x2d:
		if( ACCESSING_BITS_0_15 )
		{
			gx894_ram[ gx894_ram_write_offset / 2 ] = data & 0xffff;
//          printf( "writing %08x %04x\r", gx894_ram_write_offset, gx894_ram[ gx894_ram_write_offset / 2 ] );
			gx894_ram_write_offset += 2;
		}
		if( ACCESSING_BITS_16_31 )
		{
			gx894_ram_read_offset &= 0x0000ffff;
			gx894_ram_read_offset |= ( data & 0xffff0000 ) << 0;
		}
		break;
	case 0x2e:
		if( ACCESSING_BITS_0_15 )
		{
			gx894_ram_read_offset &= 0xffff0000;
			gx894_ram_read_offset |= ( data & 0x0000ffff ) >> 0;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "write offset 2e msw32\n" );
		}
		break;
	case 0x38:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( 0, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( 1, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &a );
		break;
	case 0x39:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( 7, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( 3, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &b );
		break;
	case 0x3b:
		if( ACCESSING_BITS_16_31 )
		{
			ds2401_write( 2, !( ( data >> 28 ) & 1 ) );
		}
		break;
	case 0x3e:
		if( ACCESSING_BITS_0_15 )
		{
			/* 12 */
			/* 13 */
			/* 14 */
			/* 15 */

			static int s = 0;
			static int b = 0;
			static int o = 0;

			s = ( s >> 1 ) | ( ( data & 0x8000 ) >> 8 );
			b++;
			if( b == 8 )
			{
//              printf( "%04x %02x\n", o, s );
				c = 0;
				b = 0;
				o++;
			}
		}

		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( 4, ( data >> 28 ) & 0xf );
		}
		COMBINE_DATA( &c );
		break;
	case 0x3f:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( 2, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( 5, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &d );
		break;
	default:
//      printf( "write offset %08x\n", offset );
		break;
	}
	if( a != olda || b != oldb || c != oldc || d != oldd )
	{
//      printf( "%08x %08x %08x %08x\n", a, b, c, d );
	}
}

static void gx894pwbba_init( void (*output_callback_func)( int offset, int data ) )
{
	int gx894_ram_size = 24 * 1024 * 1024;

	gx894pwbba_output_callback = output_callback_func;

	memory_install_read32_handler ( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, gx894pwbba_r );
	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f640000, 0x1f6400ff, 0, 0, gx894pwbba_w );

	gx894_ram_write_offset = 0;
	gx894_ram_read_offset = 0;
	gx894_ram = auto_malloc( gx894_ram_size );

	ds2401_init( 2, ds2401_xid ); /* todo: load this from roms */

	state_save_register_global_array( gx894pwbba_output_data );
	state_save_register_global_pointer( gx894_ram, gx894_ram_size / 4 );
}

/* ddr digital */

static DRIVER_INIT( ddrdigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( gn845pwbb_output_callback );
}

/* guitar freaks digital */

static DRIVER_INIT( gtrfrkdigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( NULL );

	memory_install_read32_handler ( 0, ADDRESS_SPACE_PROGRAM, 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_r );
	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_w );
}

/* ddr solo */

static void ddrsolo_output_callback( int offset, int data )
{
	switch( offset )
	{
	case 4:
	case 7:
	case 12:
	case 15:
		/* DDR stage i/o */
		break;

	case 8:
		output_set_value( "extra 4", !data );
		break;

	case 9:
		output_set_value( "extra 2", !data );
		break;

	case 10:
		output_set_value( "extra 1", !data );
		break;

	case 11:
		output_set_value( "extra 3", !data );
		break;

	case 16:
		output_set_value( "speaker", !data );
		break;

	case 20:
		output_set_led_value( 0, !data ); // start
		break;

	case 21:
		output_set_value( "body center", !data );
		break;

	case 22:
		output_set_value( "body right", !data );
		break;

	case 23:
		output_set_value( "body left", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

static DRIVER_INIT( ddrsolo )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( ddrsolo_output_callback );
}

/* drummania */

static void drmn_output_callback( int offset, int data )
{
	switch( offset )
	{
	case 0: // drmn2+
	case 16: // drmn
		output_set_value( "hi-hat", !data );
		break;

	case 1: // drmn2+
	case 17: // drmn
		output_set_value( "high tom", !data );
		break;

	case 2: // drmn2+
	case 18: // drmn
		output_set_value( "low tom", !data );
		break;

	case 3: // drmn2+
	case 19: // drmn
		output_set_value( "snare", !data );
		break;

	case 8: // drmn2+
	case 30: // drmn
		output_set_value( "spot left & right", !data );
		break;

	case 9: // drmn2+
	case 31: // drmn
		output_set_value( "neon top", data );
		break;

	case 11: // drmn2+
	case 27: // drmn
		output_set_value( "neon woofer", data );
		break;

	case 12: // drmn2+
	case 20: // drmn
		output_set_value( "cymbal", !data );
		break;

	case 13: // drmn2+
	case 21: // drmn
		output_set_led_value( 0, data ); // start
		break;

	case 14: // drmn2+
	case 22: // drmn
		output_set_value( "select button", data );
		break;

	case 23: // drmn
	case 26: // drmn
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

static DRIVER_INIT( drmn )
{
	DRIVER_INIT_CALL(konami573);

	gx700pwfbf_init( drmn_output_callback );
}

static DRIVER_INIT( drmndigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( drmn_output_callback );
}

/* dance maniax */

static void dmx_output_callback( int offset, int data )
{
	switch( offset )
	{
	case 0:
		output_set_value( "blue io 8", !data );
		break;

	case 1:
		output_set_value( "blue io 9", !data );
		break;

	case 2:
		output_set_value( "red io 9", !data );
		break;

	case 3:
		output_set_value( "red io 8", !data );
		break;

	case 4:
		output_set_value( "blue io 6", !data );
		break;

	case 5:
		output_set_value( "blue io 7", !data );
		break;

	case 6:
		output_set_value( "red io 7", !data );
		break;

	case 7:
		output_set_value( "red io 6", !data );
		break;

	case 8:
		output_set_value( "blue io 4", !data );
		break;

	case 9:
		output_set_value( "blue io 5", !data );
		break;

	case 10:
		output_set_value( "red io 5", !data );
		break;

	case 11:
		output_set_value( "red io 4", !data );
		break;

	case 12:
		output_set_value( "blue io 10", !data );
		break;

	case 13:
		output_set_value( "blue io 11", !data );
		break;

	case 14:
		output_set_value( "red io 11", !data );
		break;

	case 15:
		output_set_value( "red io 10", !data );
		break;

	case 16:
		output_set_value( "blue io 0", !data );
		break;

	case 17:
		output_set_value( "blue io 1", !data );
		break;

	case 18:
		output_set_value( "red io 1", !data );
		break;

	case 19:
		output_set_value( "red io 0", !data );
		break;

	case 20:
		output_set_value( "blue io 2", !data );
		break;

	case 21:
		output_set_value( "blue io 3", !data );
		break;

	case 22:
		output_set_value( "red io 3", !data );
		break;

	case 23:
		output_set_value( "red io 2", !data );
		break;

	case 28:
		output_set_value( "yellow spot light", !data );
		break;

	case 29:
		output_set_value( "blue spot light", !data );
		break;

	case 31:
		output_set_value( "pink spot light", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

static WRITE32_HANDLER( dmx_io_w )
{
	verboselog( 2, "dmx_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_value( "left 2p", !( ( data >> 0 ) & 1 ) );
		output_set_led_value( 1, !( ( data >> 1 ) & 1 ) ); // start 1p
		output_set_value( "right 2p", !( ( data >> 2 ) & 1 ) );

		output_set_value( "left 1p", !( ( data >> 3 ) & 1 ) );
		output_set_led_value( 0, !( ( data >> 4 ) & 1 ) ); // start 2p
		output_set_value( "right 1p", !( ( data >> 5 ) & 1 ) );
		break;

	default:
		verboselog( 0, "dmx_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

static DRIVER_INIT( dmx )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( dmx_output_callback );

	memory_install_write32_handler( 0, ADDRESS_SPACE_PROGRAM, 0x1f600000, 0x1f6000ff, 0, 0, dmx_io_w );
}

/* salary man champ */

static int salarymc_lamp_bits;
static int salarymc_lamp_shift;
static int salarymc_lamp_data;
static int salarymc_lamp_clk;

static void salarymc_lamp_data_write( int data )
{
	salarymc_lamp_data = data;
}

static void salarymc_lamp_rst_write( int data )
{
	if( data )
	{
		salarymc_lamp_bits = 0;
		salarymc_lamp_shift = 0;
	}
}

static void salarymc_lamp_clk_write( int data )
{
	if( salarymc_lamp_clk != data )
	{
		salarymc_lamp_clk = data;

		if( salarymc_lamp_clk )
		{
			salarymc_lamp_shift <<= 1;

			if( salarymc_lamp_data )
			{
				salarymc_lamp_shift |= 1;
			}

			salarymc_lamp_bits++;
			if( salarymc_lamp_bits == 16 )
			{
				if( ( salarymc_lamp_shift & ~0xe38 ) != 0 )
				{
					verboselog( 0, "unknown bits in salarymc_lamp_shift %08x\n", salarymc_lamp_shift & ~0xe38 );
				}

				output_set_value( "player 1 red", ( salarymc_lamp_shift >> 11 ) & 1 );
				output_set_value( "player 1 green", ( salarymc_lamp_shift >> 10 ) & 1 );
				output_set_value( "player 1 blue", ( salarymc_lamp_shift >> 9 ) & 1 );

				output_set_value( "player 2 red", ( salarymc_lamp_shift >> 5 ) & 1 );
				output_set_value( "player 2 green", ( salarymc_lamp_shift >> 4 ) & 1 );
				output_set_value( "player 2 blue", ( salarymc_lamp_shift >> 3 ) & 1 );

				salarymc_lamp_bits = 0;
				salarymc_lamp_shift = 0;
			}
		}
	}
}

static DRIVER_INIT( salarymc )
{
	DRIVER_INIT_CALL(konami573);

	security_bit7_write = salarymc_lamp_data_write;
	security_bit6_write = salarymc_lamp_rst_write;
	security_bit5_write = salarymc_lamp_clk_write;

	state_save_register_global( salarymc_lamp_bits );
	state_save_register_global( salarymc_lamp_shift );
	state_save_register_global( salarymc_lamp_data );
	state_save_register_global( salarymc_lamp_clk );
}

static MACHINE_DRIVER_START( konami573 )
	/* basic machine hardware */
	MDRV_CPU_ADD( PSXCPU, XTAL_67_7376MHz )
	MDRV_CPU_PROGRAM_MAP( konami573_map, 0 )
	MDRV_CPU_VBLANK_INT("main", sys573_vblank)

	MDRV_MACHINE_RESET( konami573 )
	MDRV_NVRAM_HANDLER( konami573 )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE( 60 )
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC( 0 ))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 1024, 1024 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )

	MDRV_PALETTE_LENGTH( 65536 )

	MDRV_PALETTE_INIT( psx )
	MDRV_VIDEO_START( psx_type2 )
	MDRV_VIDEO_UPDATE( psx )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD( PSXSPU, 0 )
	MDRV_SOUND_CONFIG( konami573_psxspu_interface )
	MDRV_SOUND_ROUTE( 0, "left", 1.0 )
	MDRV_SOUND_ROUTE( 1, "right", 1.0 )

	MDRV_SOUND_ADD( CDDA, 0 )
	MDRV_SOUND_ROUTE( 0, "left", 1.0 )
	MDRV_SOUND_ROUTE( 1, "right", 1.0 )
MACHINE_DRIVER_END

static INPUT_PORTS_START( konami573 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Unused 1" ) PORT_DIPLOCATION( "DIP SW:1" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Screen Flip" ) PORT_DIPLOCATION( "DIP SW:2" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, "V-Flip" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Unused 2") PORT_DIPLOCATION( "DIP SW:3" )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000000, "Start Up Device" ) PORT_DIPLOCATION( "DIP SW:4" )
	PORT_DIPSETTING(          0x00000008, "CD-ROM Drive" )
	PORT_DIPSETTING(          0x00000000, "Flash ROM" )
	PORT_BIT( 0x000000f0, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* 0xc0 */
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_SPECIAL )
//  PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x00001000, 0x00001000, "Network?" )
	PORT_CONFSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( On ) )
//  PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* adc0834 d0 */
//  PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* x76f041/zs01 sda */
    PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
//  PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* PCCARD 1 */
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* PCCARD 2 */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
//  PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* skip init? */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 ) /* skip init? */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* skip init? */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 ) /* skip init? */

	PORT_START_TAG("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
//  PORT_BIT( 0xf0fff0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("CART")
	PORT_CONFNAME( 1, 0, "Security Cart" )
	PORT_CONFSETTING( 0, "Install" )
	PORT_CONFSETTING( 1, "Game" )

	PORT_START_TAG("CD")
	PORT_CONFNAME( 1, 0, "CD" )
	PORT_CONFSETTING( 0, "1" )
	PORT_CONFSETTING( 1, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitbc )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN3")

	PORT_START_TAG( "uPD4701_y" )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_MINMAX( 0, 0xfff ) PORT_SENSITIVITY( 15 ) PORT_KEYDELTA( 8 ) PORT_RESET

	PORT_START_TAG( "uPD4701_switches" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitmc )
	PORT_INCLUDE( fbaitbc )

	PORT_START_TAG( "analog0" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE

	PORT_START_TAG( "analog1" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( ddr )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(gn845pwbb_read,0)

	PORT_START_TAG( "STAGE" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) /* serial? */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1)    /* serial? */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) /* serial? */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(2)    /* serial? */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ddrsolo )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Right 1" ) /* serial? */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up 1" ) /* serial? */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 1" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 2" ) /* P1 BUTTON 1 */ /* skip init? */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 2" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 2" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 1" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Right 1" ) /* P2 RIGHT */ /* serial? */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */ /* serial? */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME( "P1 Up 2" ) /* P2 BUTTON1 */ /* skip init? */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Right 2" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "P1 Up-Right 2" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "P1 Select L" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "P1 Select R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( gtrfrks )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* SERVICE1 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Effect 1")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Effect 2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Pick")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Wailing")
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Button G")
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button B")
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Effect 1")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Effect 2")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Pick")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Wailing")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Button G")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Button B")
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* P1 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( dmx )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D1 L" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D1 R" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Select L" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Select R" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor U L" ) /* P1 BUTTON1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor U R" ) /* P1 BUTTON2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D1 L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D1 R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2) PORT_NAME( "P2 Select L" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) PORT_NAME( "P2 Select R" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor U L" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor U R" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D0 L" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "D-Sensor D0 R" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D0 L" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME( "D-Sensor D0 R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( drmn )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* COIN2 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "High Tom" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Low Tom" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Hi-Hat" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Snare" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Cymbal" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Bass Drum" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "Select R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

#define SYS573_BIOS_A ROM_LOAD( "700a01.22g",   0x0000000, 0x080000, CRC(11812ef8) )

// BIOS
ROM_START( sys573 )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )
ROM_END

// Games
ROM_START( bassangl )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ee1b32a7) SHA1(c0f6b14b054f5a95ce474e794a3e0ca78faac681) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "765jaa02", 0, MD5(11693b1234458c238ed613ef37f71245) SHA1(d820f8166b7d5ffcf41e7a70c8c4c4d1c207c1bd) )
ROM_END

ROM_START( cr589fw )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "700b04", 0, MD5(4847e008189b7c700f2129ecb362b924) SHA1(13ac92eb242de48317924b9c725f9f693a263cf5) )
ROM_END

ROM_START( cr589fwa )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "700a04", 0, MD5(211850ed73d05ccbf5951f1fe19a6767) SHA1(bf7865629775a34a8f8b628053e97f25b51ade2e) )
ROM_END

ROM_START( darkhleg )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx706ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(72b42574) SHA1(79dc959f0ce95ccb9ac0dbf0a72aec973e91bc56) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "706jaa02", 0, MD5(4f096051df039b0d104d4c0fff5dadb8) SHA1(4c8d976096c2da6d01804a44957daf9b50103c90) )
ROM_END

ROM_START( ddrextrm )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc36ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c1601287) SHA1(929691a78f7bb6dd830f832f301116df0da1619b) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gcc36ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "c36jaa02", 0, MD5(83fa51031d826d603c0371b18180aeda) SHA1(a1591cb4f1da7e460de57afb17a85592719243e0) )
ROM_END

ROM_START( ddru )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(c9e7fced) SHA1(aac4dde100091bc64d397f53484a0ffbf68b8101) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845uaa02", 0, MD5(32d52ee2b37559d7413788c87085f37c) SHA1(e82610e1a34fba144499f9ee892ac882d1e96853) )
ROM_END

ROM_START( ddrj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(a16f42b8) SHA1(da4f1eb3eb2b28cb3a0bc74bb9b9945970f56ac2) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845jba02", 0, MD5(314f64301c3429312770ecdeb975d285) SHA1(8ebd9a68bbea3d9947a95d896347b0fea2145e4a) )
ROM_END

ROM_START( ddrja )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.27m",  0x200000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.31l",  0x400000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.27l",  0x600000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.31j",  0x800000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.27j",  0xa00000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.31h",  0xc00000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jaa.27h",  0xe00000, 0x200000, NO_DUMP )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845jaa02", 0, MD5(045237b7ba76f393f69dd95eae14b61a) SHA1(eb59d00300424be89817ce3f8d7e68b8cf0f7943) )
	DISK_IMAGE_READONLY( "845jaa01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddrjb )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.27m",  0x200000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.31l",  0x400000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.27l",  0x600000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.31j",  0x800000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.27j",  0xa00000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.31h",  0xc00000, 0x200000, NO_DUMP )
	ROM_LOAD( "gc845jab.27h",  0xe00000, 0x200000, NO_DUMP )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845jab02", 0, MD5(ebaaf265a1a7efae93cd745a67ea2cb2) SHA1(10ee3081065ebf5a814abbd30c8dee91b384a849) )
	DISK_IMAGE_READONLY( "845jab01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddra )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(327c4851) SHA1(f0939224af706fd103a67aae9c96518c1db90ac9) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845aaa02", 0, MD5(2ab58fc647d35673861788a78df2afba) SHA1(fe2d18cdab7a3088f7c876ce531d64a2f3ae9294) )
ROM_END

ROM_START( ddr2m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "895jaa02", 0, MD5(f83ea1459c51aba2e16830b775444db3) SHA1(f1d47440ec7ba902f0fc5cad241729613f24fce1) )
ROM_END

ROM_START( ddr2mc )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn896ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "896jaa01", 0, MD5(3cab3bc6d9459360da8f6784dd861067) SHA1(cb99e52eac5223509e914648d9b5dec59ed242f8) )
	DISK_IMAGE_READONLY( "895jaa02", 1, MD5(f83ea1459c51aba2e16830b775444db3) SHA1(f1d47440ec7ba902f0fc5cad241729613f24fce1) )
ROM_END

ROM_START( ddr2mc2 )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "984jaa01", 0, MD5(945de47f526007f7c607c398b9b6275a) SHA1(da257e5a553a75439970393bdafc581f6971f946) )
	DISK_IMAGE_READONLY( "895jaa02", 1, MD5(f83ea1459c51aba2e16830b775444db3) SHA1(f1d47440ec7ba902f0fc5cad241729613f24fce1) )
ROM_END

ROM_START( ddr2ml )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "885jaa02", 0, MD5(696e39fa7113f61181875bffca13a1b4) SHA1(ece1d34a3bdbe07b608429abe30802bc7327a94a) )
ROM_END

ROM_START( ddr3ma )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "887aaa02", 0, MD5(20a95a94413dfba836fd1b0d7923dbfc) SHA1(75500b4393519f1f9bce7c9bdfd45ef365a8672c) )
ROM_END

ROM_START( ddr3mj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(3a377cec) SHA1(5bf3107a89547bd7697d9f0ab8f67240e101a559) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(2f633432) SHA1(bce44f20a5a7318af6aea4fdfa8af64ddb76047c) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "887jaa02", 0, MD5(c2241d1277a98d6a8cafd3aed0c9b9da) SHA1(04f639c3e72aa6dd546ea5b5b84fb3fcb10acc46) )
ROM_END

ROM_START( ddr3mk )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "887kba02", 0, MD5(3ebd603c800158697c968caf187a7cc6) SHA1(8feb5dcada45e6f6aa0695439dc718fafb978b4d) )
ROM_END

ROM_START( ddr3mka )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "887kaa02", 0, MD5(9ecee52213411d4a518f4724c87ee9d3) SHA1(10af2af753c5a54d1d3ac40a9ccc3e0324183f4d) )
ROM_END

ROM_START( ddr3mp )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea22ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ef370ff7) SHA1(cb7a043f8bfa535e54ae9af728031d1018ed0734) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca22ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(6291defc) SHA1(bb9dad69896826aeb42dafa91cb99599467c31ff) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a22jaa02", 0, MD5(b386ecc1d54e2ac73c16057af9220aa6) SHA1(cc4fba48dfff96f0ac85a438dab95e00891aeac5) )
ROM_END

ROM_START( ddr4m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(7bd2a24f) SHA1(62c73a54c4ed7697cf81ddbf3d13d4b0ca827be5) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33aa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6feb2bd) SHA1(dfd5bd532338849289e2e4c155c0ca86e79b9ae5) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a33aaa02", 0, BAD_DUMP MD5(d843cba35726f3b0af357f712b8870a4) SHA1(40456e772c39c339828dd4726766ef2e0981d3a1) )
ROM_END

ROM_START( ddr4mj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e5230867) SHA1(44aea9ccc90d81e7f41e5e9a62b28fcbdd75363b) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "a33jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca33ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a33jaa02", 0, MD5(855456931374d1c99dfd44c52a0a3178) SHA1(ad7672bf30becc9030c4fa097cc60deecad6f36d) )
ROM_END

ROM_START( ddr4ms )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33ab.u1",   0x000000, 0x000224, BAD_DUMP CRC(32fb3d13) SHA1(3ca6c77438f96b13d2c05f13a10fcff89a1403a2) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ab.u1",   0x000000, 0x001014, BAD_DUMP CRC(312ac13f) SHA1(05d733edc03cfc5ea03db6c683f59ed6ff860b5a) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a33aba02", 0, MD5(d843cba35726f3b0af357f712b8870a4) SHA1(40456e772c39c339828dd4726766ef2e0981d3a1) )
ROM_END

ROM_START( ddr4msj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jba.u1",    0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(00e4b531) SHA1(f421fc33642c5a3cd89fb14dc8cd601bdddd1f55) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "a33jba.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca33jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a33jba02", 0, BAD_DUMP MD5(855456931374d1c99dfd44c52a0a3178) SHA1(ad7672bf30becc9030c4fa097cc60deecad6f36d) )
ROM_END

ROM_START( ddr4mp )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e9b6ce56) SHA1(f040fba2b2b446baa840026dcd10f9785f8cc0a3) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, REGION_USER11, 0 ) /* timekeeper */
	ROM_LOAD( "gca34ja.22h",  0x000000, 0x002000, CRC(80575c1f) SHA1(a0594ca0f75bc7d49b645e835e9fa48a73c3c9c7) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a34jaa02", 0, MD5(d8292d7f1e359d308b779e44fd0809ef) SHA1(23aadca5274bff5f130357701c1ab269943b387d) )
ROM_END

ROM_START( ddr4mps )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(0c717300) SHA1(00d21f39fe90494ffec2f8799767cc46a9cd2b00) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, REGION_USER11, 0 ) /* timekeeper */
	ROM_LOAD( "gca34jb.22h",  0x000000, 0x002000, CRC(bc6c8bd7) SHA1(10ceec5c7bc5ca9fca88f3c083a7d97012982079) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a34jba02", 0, BAD_DUMP MD5(d8292d7f1e359d308b779e44fd0809ef) SHA1(23aadca5274bff5f130357701c1ab269943b387d) )
ROM_END

ROM_START( ddr5m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gca27ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ec526036) SHA1(f47d94d19268fdfa3ae9d42db9f2e2f9be318f2b) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gca27ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a27jaa02", 0, MD5(502ecebf4d2e931f6b75b6a22b7d620c) SHA1(2edb3a0160c2783db6b0ddfce0f7c9ebb35b481f) )
ROM_END

ROM_START( ddrbocd )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "892jaa01", 0, MD5(b29b63bafdd35f38662ff8daf5fc59f7) SHA1(c929c488d206e055e756ca506c3b1ff430a46aaa) )
	DISK_IMAGE_READONLY( "895jaa02", 1, MD5(f83ea1459c51aba2e16830b775444db3) SHA1(f1d47440ec7ba902f0fc5cad241729613f24fce1) )
ROM_END

ROM_START( ddrs2k )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(36d18e2f) SHA1(e976047dfbee62de9ad9e5de8e7629a24c29d581) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(21073a3e) SHA1(afa12404ceb462b9016a41c40775da87aa09cfeb) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "905aaa02", 0, MD5(8b753bae1b4cbd5c8b641eb723b660a1) SHA1(cfd40cee9380588dc2ced107ace2a7486d91944d) )
ROM_END

ROM_START( ddrs2kj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(a077b0a1) SHA1(8f247b38c933a104a325ebf1f1691ef260480e1a) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(b7a104b0) SHA1(0f6901e41640f729f8a084a33148a9b900475594) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge905ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "905jaa02", 0, MD5(098496f8d9b5ac0357f093a62d6d59f0) SHA1(37f9aff936a51b3482ae4717227993106be8b476) )
ROM_END

ROM_START( ddrmax )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb19ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(2255626a) SHA1(cb70c4b551265ffc6cc41f7bd2678696e8067060) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gcb19ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "b19jaa02", 0, MD5(0f080892f7e5ad0d83e6db70e4cb80d7) SHA1(59361e85641f2deda88ccbc2cd1634523f1c1a3b) )
ROM_END

ROM_START( ddrmax2 )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb20ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(fb7e0f58) SHA1(e6da23257a2a2ba7c69e817a91a0a8864f009386) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gcb20ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "b20jaa02", 0, MD5(2d57e32a263b355391a94f45c145d0fe) SHA1(e43c34bd113ef20e579ce7c6248288a257a5ccde) )
ROM_END

ROM_START( ddrsbm )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq894ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(cc3a47de) SHA1(f6e2e101870370b1e295a4a9ed546aa2d8bc2010) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gq894ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "894jaa02", 0, MD5(05f3cd86796f41353528999ac3fbd26b) SHA1(5b65ac6bc2497b3ab99542f5acae3a64895f221d) )
ROM_END

ROM_START( ddrusa )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gka44ua.u1",   0x000000, 0x001014, BAD_DUMP CRC(2ef7c4f1) SHA1(9004d27179ece86883d01b3e6bbfeebc1b478d57) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gka44ua.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a44uaa02", 0, MD5(f9694956bab44593784fa728c6b53712) SHA1(e40b52ce3b1e90e3ec9190b83ba875c4ab1b0f2f) )
ROM_END

ROM_START( drmn )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(7dca0b3f) SHA1(db6d5c527e2a99133b516e01433024d3173848c6) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x0c00000, 0xff )
	ROM_LOAD( "gq881ja.31h",  0xc00000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_LOAD( "gq881ja.27h",  0xe00000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, REGION_USER11, 0 ) /* timekeeper */
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "881jad01", 0, MD5(8191ad7747f5ac87a40534f53a2a6cd4) SHA1(61bfd356d262f53f1ab55654aba2981fb8eb1420) ) // upgrade or bootleg?
	DISK_IMAGE_READONLY( "881jaa02", 1, NO_DUMP )
ROM_END

ROM_START( drmn2m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "912jab", 0, BAD_DUMP MD5(60dadc836f00f22d50b5b634250aa624) SHA1(5316b2e2e89af7bc038a3febc91525edac91fe7e) )
ROM_END

ROM_START( drmn2mpu )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "912jab",  0, BAD_DUMP MD5(60dadc836f00f22d50b5b634250aa624) SHA1(5316b2e2e89af7bc038a3febc91525edac91fe7e) )
	DISK_IMAGE_READONLY( "912za01", 1, BAD_DUMP MD5(1d3fa130a3a6c8433c276e609b373e4f) SHA1(121ca6df16084a01c88ef26167b13053c9adc7ce) )
ROM_END

ROM_START( drmn3m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(90e544fa) SHA1(1feb617c36bad41aa720a6e5d3ec9e5cb2030567) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(5af1b5da) SHA1(cf862ef9ab60e8da89af96266943137827e4a261) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "a23jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca23ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a23jaa02", 0, MD5(3b6477be8e66e447cc3f94047c28a2a2) SHA1(4c607eb2b212785520d1c6dc21d013fe6f489741) )
ROM_END

ROM_START( dmx )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "ge874ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c5536373) SHA1(1492221f7dd9485f7745ecb0a982a88c8e768e53) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "ge874ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "874jaa", 0, BAD_DUMP MD5(457f6e55aec68537ae47c6045de1ee26) SHA1(ff6332e032b1528691a87c5001bf808d6a8f5ef7) )
ROM_END

ROM_START( dmx2m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gca39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ecc75eb7) SHA1(af66ced28ba5e79ae32ae0ef12d2ebe4207f3822) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gca39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a39jaa02", 0, MD5(3106d45c868ad30e47f7873ea1dffc8a) SHA1(20df1b5636622d8c0e45623bd1af6bc1249fec65) )
ROM_END

ROM_START( dmx2majp )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gca38ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(99a746b8) SHA1(333236e59a707ecaf840a66f9b947ceade2cf2c9) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.31m",  0x000000, 0x200000, CRC(a0f54ab5) SHA1(a5ae67d7619393779c79a2e227cac0675eeef538) )
	ROM_LOAD( "gca38ja.27m",  0x200000, 0x200000, CRC(6c3934b8) SHA1(f0e4a692b6caaf60fefaec87fd23da577439f69d) )
	ROM_FILL( 0x400000, 0x0c00000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gca38ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a38jaa02", 0, MD5(3c4070547d17605db68a3db333865eb0) SHA1(e9fe714bb0a5354b199b1fb7b33f366db502c03a) )
ROM_END

ROM_START( dncfrks )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gk874ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(7a6f4672) SHA1(2e009e57760e92f48070a69cff5597c37a4783a2) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gk874ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "874kaa", 0, BAD_DUMP MD5(5e02c8dba12f949ce99834e597d8d08d) SHA1(659281e1b63c5ff0616b27d7f87a4e7f1c493372) )
ROM_END

ROM_START( dsem2 )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gkc23ea.u1",   0x000000, 0x001014, BAD_DUMP CRC(aec2421a) SHA1(5ea9e9ce6161ebc99a50db0b7304385511bd4553) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gkc23ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "c23eaa02", 0, MD5(e7275ba9f53334ac528afaa278df7153) SHA1(661f0b53ab64fc746b8e3b7ff3dd32d2bed852ac) )
ROM_END

ROM_START( dsfdct )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja_gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(08a60147) SHA1(0d39dca5e9e17fff0e64f296c8416e4ca23fdc1b) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910jc.u1",   0x000000, 0x000084, BAD_DUMP CRC(3c1ca973) SHA1(32211a72e3ac88b2723f82dac0b26f93031b3a9c) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja_gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gc910jc.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "910jca02", 0, MD5(c909982a234dbd59388892bd627a466e) SHA1(687ce1d480eb13e78289171b6c56fd5b1e7d5d9e) )
ROM_END

ROM_START( dsfdcta )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x0000084, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(59a23808) SHA1(fcff1c68ff6cfbd391ac997a40fb5253fc62de82) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER5, 0 ) /* PCCARD2 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gc910ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "910jaa02", 0, MD5(e5380280b5cbb10822391f74866f8bee) SHA1(3d17320f56be7edb1be6b1bd53269d452f71123d) )
ROM_END

ROM_START( dsftkd )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "884jaa02", 0, MD5(d73444ab74efb8587c2bf455e3ec0d13) SHA1(92522380b92333b10d401fda4f81592073f3e601) )
ROM_END

ROM_START( dstage )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(db643af7) SHA1(881221da640b883302e657b906ea0a4e74555679) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "845ea", 0, BAD_DUMP MD5(32d52ee2b37559d7413788c87085f37c) SHA1(e82610e1a34fba144499f9ee892ac882d1e96853) )
ROM_END

ROM_START( fbait2bc )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc865ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8f0b4b) SHA1(363b1ea1a520b239ba8bca867366bbe8a9977a43) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "865uab02", 0, MD5(5a253a58417539f9b0cb9726311f73d5) SHA1(86ccbaac30e9e2d7d0ad6ae65a4f53f606f50525) )
ROM_END

ROM_START( fbaitbc )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(588748c6) SHA1(ea1ead61e0dcb324ef7b6106cae00bcf6702d6c4) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "765uab02", 0, MD5(56d8a23bb592932631f8f81b9797fce6) SHA1(dda131b8655e3c4394e50749fe3a1e468f9df353) )
ROM_END

ROM_START( fbaitmc )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(753ad84e) SHA1(e024cefaaee7c9945ccc1f9a3d896b8560adce2e) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "889ea", 0, BAD_DUMP MD5(43eae52edd38019f0836897ea8def527) SHA1(2e6937c265c222ac2cea50fbf32201ade425ee30) )
ROM_END

ROM_START( fbaitmca )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9c22aae8) SHA1(c107b0bf7fa76708f2d4f6aaf2cf27b3858378a3) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "889aa", 0, BAD_DUMP MD5(43eae52edd38019f0836897ea8def527) SHA1(2e6937c265c222ac2cea50fbf32201ade425ee30) )
ROM_END

ROM_START( fbaitmcj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(6278603c) SHA1(d6b59e270cfe4016e12565aedec8a4f0702e1a6f) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "889ja", 0, BAD_DUMP MD5(43eae52edd38019f0836897ea8def527) SHA1(2e6937c265c222ac2cea50fbf32201ade425ee30) )
ROM_END

ROM_START( fbaitmcu )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(67b91e54) SHA1(4d94bfab08e2bf6e34ee606dd3c4e345d8e5d158) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "889ua", 0, BAD_DUMP MD5(43eae52edd38019f0836897ea8def527) SHA1(2e6937c265c222ac2cea50fbf32201ade425ee30) )
ROM_END

ROM_START( gtrfrks )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(06bd6c4f) SHA1(61930e467ad135e2f31393ff5af981ed52f3bef9) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "886ea", 0, BAD_DUMP MD5(b8b39a6e48867fdad640bd256273bdfc) SHA1(2277c6268b8327be8d7636d4812920e5d3b353cd) )
ROM_END

ROM_START( gtrfrksu )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(143eaa55) SHA1(51a4fa3693f1cb1646a8986003f9b6cc1ae8b630) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "886ua", 0, BAD_DUMP MD5(b8b39a6e48867fdad640bd256273bdfc) SHA1(2277c6268b8327be8d7636d4812920e5d3b353cd) )
ROM_END

ROM_START( gtrfrksj )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(11ffd43d) SHA1(27f4f4d782604379254fb98c3c57e547aa4b321f) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "886ja", 0, BAD_DUMP MD5(b8b39a6e48867fdad640bd256273bdfc) SHA1(2277c6268b8327be8d7636d4812920e5d3b353cd) )
ROM_END

ROM_START( gtrfrksa )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(efa51ee9) SHA1(3374d936de69c287e0161bc526546441c2943555) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "886aa", 0, BAD_DUMP MD5(b8b39a6e48867fdad640bd256273bdfc) SHA1(2277c6268b8327be8d7636d4812920e5d3b353cd) )
ROM_END

ROM_START( gtrfrk2m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(687868c4) SHA1(1230e74e4cf17953febe501df56d8bbec1de9356) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP MD5(dfe595184dd55046da6fe93bb14b83be) SHA1(c76d5c11422b2ef1750e92a1edac2812666aadae) )
ROM_END

ROM_START( gtrfrk3m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "949jac01", 0, MD5(0f78f8e06edd3b8fa0abed22155d06d9) SHA1(3e43a5018aa88ed78c9e2fb50f65489a6c7de093) )
	DISK_IMAGE_READONLY( "949jab02", 1, MD5(331a7516a33d9cf9f04b8c9aa5de5fc1) SHA1(9aae90c6b0f5c31f47a420f876c0dbc81d43b756) )
ROM_END

ROM_START( gtfrk3ma )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "949jab02", 0, MD5(331a7516a33d9cf9f04b8c9aa5de5fc1) SHA1(9aae90c6b0f5c31f47a420f876c0dbc81d43b756) )
ROM_END

ROM_START( gtfrk3mb )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(61f35ee1) SHA1(0a2b66742364d76ec18647b2761590bd49229625) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* game security cart id */
	ROM_LOAD( "ge949jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "949jaz02", 0, MD5(22f7e1e61ea5f627e28c32f2209b5138) SHA1(62bd62cafb4bd4a1e393071f5e55d5ab57e3a880) )
ROM_END

ROM_START( gtrfrk4m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "a24jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(29e326fe) SHA1(41a600105b08accc9d7ebd2b8ae08c0863758aa0) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gea24ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(d1fccf11) SHA1(6dcd79f3171d6e4bd7e1149901638f8ea58ff623) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "a24jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gea24ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a24jaa02", 0, MD5(1e113d8f5601b0b4f38852894717e486) SHA1(f0748312663a9683fd5a115c16f135cb58b993b1) )
ROM_END

ROM_START( gtrfrk5m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gea26jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(c2725fca) SHA1(b70a3266c61af5cbe0478a6f3dd850ebcab980dc) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.31m", 0x000000, 0x200000, CRC(1a25e660) SHA1(dbd8fad0bac307723c70d00763cadf4261a7ed73) )
	ROM_LOAD( "gea26jaa.27m", 0x200000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )
	ROM_FILL( 0x400000, 0x0c00000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gea26jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a26jaa02", 0, MD5(885b18ce273770330aefa0276911c046) SHA1(635aee062df45c83b080612d29101fe70c14979d) )
ROM_END

ROM_START( gtrfrk6m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gcb06ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(673c98ab) SHA1(b1d889bf4fc5e425056acb6b72b2c563966fb7d7) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gcb06ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "b06jaa02", 0, MD5(8191da2660bb645fcfee9fb60baef242) SHA1(e8be8bdc0cbfb95a0a56ab89f39de3089d31f305) )
ROM_END

ROM_START( gtrfrk7m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "gcb17jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(5a338c31) SHA1(0fd9ee306335858dd6bef680a62557a8bf055cc3) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.31m", 0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_LOAD( "gcb17jaa.27m", 0x200000, 0x200000, CRC(7e7da9a9) SHA1(1882418779a48b5aefd113895756116379a6a4f7) )
	ROM_FILL( 0x400000, 0x0c00000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gcb17jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "b17jaa02", 0, MD5(65b47fbf7d682e8dc8b2a3137aaab9b7) SHA1(bf8eb8f857c08595bb1c19f470ac689400ee0cab) )
ROM_END

ROM_START( gtfrk11m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(9bd81d0a) SHA1(c95f6d7317bf88177f7217de4ba4376485d5cdbf) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x2000000, REGION_USER4, 0 ) /* PCCARD1 */
	ROM_FILL( 0x0000000, 0x2000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "gcd39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "d39jaa02", 0, MD5(4730dd81132cdac6f0cb7cc4c9753329) SHA1(b9425ab6bc7305eac2fef9799b7d46b18462ea84) )
ROM_END

ROM_START( konam80a )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9b38b959) SHA1(6b4fca340a9b1c2ae21ad3903c1ac1e39ab08b1a) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "826aaa01", 0, BAD_DUMP MD5(456f683c5d47dd73cfb73ce80b8a7351) SHA1(452c94088ffefe42e61c978b48d425e7094a5af6) )
ROM_END

ROM_START( konam80j )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(e9e861e8) SHA1(45841db0b42d096213d9539a8d076d39391dca6d) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "826jaa01", 0, MD5(456f683c5d47dd73cfb73ce80b8a7351) SHA1(452c94088ffefe42e61c978b48d425e7094a5af6) )
ROM_END

ROM_START( konam80k )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ka.u1", 0x000000, 0x000224, BAD_DUMP CRC(d41f7e38) SHA1(73e2bb132e23be72e72ea5b0686ccad28e47574a) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "826kaa01", 0, BAD_DUMP MD5(456f683c5d47dd73cfb73ce80b8a7351) SHA1(452c94088ffefe42e61c978b48d425e7094a5af6) )
ROM_END

ROM_START( konam80s )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(6ce4c619) SHA1(d2be08c213c0a74e30b7ebdd93946374cc64457f) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "826eaa01", 0, BAD_DUMP MD5(456f683c5d47dd73cfb73ce80b8a7351) SHA1(452c94088ffefe42e61c978b48d425e7094a5af6) )
ROM_END

ROM_START( konam80u )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(0577379b) SHA1(3988a2a5ef1f1d5981c4767cbed05b73351be903) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "826uaa01", 0, MD5(456f683c5d47dd73cfb73ce80b8a7351) SHA1(452c94088ffefe42e61c978b48d425e7094a5af6) )
ROM_END

ROM_START( pbballex )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gx802ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8bdda3) SHA1(780034ab08871631ef0e3e9b779ca89e016c26a8) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "802jab02", 0, MD5(52bb53327ba48f87dcb030d5e50fe94f) SHA1(67ddce1ad7e436c18e08d5a8c77f3259dbf30572) )
ROM_END

ROM_START( pcnfrk3m )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, REGION_USER2, 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23kaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(d71c4b5c) SHA1(3911c5dd933c30e6e44c8cf417bb4c284ecb4b80) )

	ROM_REGION( 0x0001014, REGION_USER8, 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(f392349c) SHA1(e7eb7979db276de560d5820163a70d97e6c023d8) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* install security cart id */
	ROM_LOAD( "a23kaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, REGION_USER10, 0 ) /* game security cart id */
	ROM_LOAD( "gca23ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a23kaa02", 0, MD5(26a270851944d86d0b00db5a302de5ce) SHA1(5feaedf614c68932accc441f77c05c7bce67b2f3) )
ROM_END

ROM_START( salarymc )
	ROM_REGION32_LE( 0x080000, REGION_USER1, 0 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, REGION_USER2, 0 ) /* security cart eeprom */
	ROM_LOAD( "gca18jaa.u1",  0x000000, 0x000084, CRC(c9197f67) SHA1(8e95a89008f756a79295f2cb557c39efca1351e7) )

	ROM_REGION( 0x1000000, REGION_USER3, 0 ) /* onboard flash */
	ROM_FILL( 0x0000000, 0x1000000, 0xff )

	ROM_REGION( 0x000008, REGION_USER9, 0 ) /* security cart id */
	ROM_LOAD( "gca18jaa.u6",  0x000000, 0x000008, CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE_READONLY( "a18ja", 0, BAD_DUMP MD5(51d6cd5e34b6ee5601d8519a50be5cdc) SHA1(364a927a45ff83ee3cac66637359d6cfb44ea2fc) )
ROM_END

// System 573 BIOS (we're missing the later version that boots up with a pseudo-GUI)
GAME( 1998, sys573,   0,        konami573, konami573, konami573,  ROT0, "Konami", "System 573 BIOS", GAME_IS_BIOS_ROOT )

GAME( 1998, darkhleg, sys573,   konami573, konami573, konami573,  ROT0, "Konami", "Dark Horse Legend (GX706 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, fbaitbc,  sys573,   konami573, fbaitbc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - A Bass Challenge (GE765 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, bassangl, fbaitbc,  konami573, fbaitbc,   ge765pwbba, ROT0, "Konami", "Bass Angler (GE765 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, pbballex, sys573,   konami573, konami573, konami573,  ROT0, "Konami", "Powerful Pro Baseball EX (GX802 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80s, sys573,   konami573, konami573, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80u, konam80s, konami573, konami573, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80j, konam80s, konami573, konami573, konami573,  ROT90, "Konami", "Konami 80's Gallery (GC826 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80a, konam80s, konami573, konami573, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, konam80k, konam80s, konami573, konami573, konami573,  ROT90, "Konami", "Konami 80's AC Special (GC826 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dstage,   sys573,   konami573, ddr,       ddr,        ROT0, "Konami", "Dancing Stage (GN845 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddru,     dstage,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GN845 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, ddrj,     dstage,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution - Internet Ranking Ver (GC845 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, ddrja,    dstage,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1998, ddrjb,    dstage,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, ddra,     dstage,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution (GN845 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, fbait2bc, sys573,   konami573, fbaitbc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn,     sys573,   konami573, drmn,      drmn,       ROT0, "Konami", "DrumMania (GQ881 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, gtrfrks,  sys573,   konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. EAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksu, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. UAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksj, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksa, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. AAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmc,  sys573,   konami573, fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. EA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcu, fbaitmc,  konami573, fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. UA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcj, fbaitmc,  konami573, fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. JA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmca, fbaitmc,  konami573, fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. AA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2m,    sys573,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix (GN895 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddrbocd,  ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2ml,   ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc,   ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc2,  ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2m, sys573,   konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dsftkd,   sys573,   konami573, ddr,       ddr,        ROT0, "Konami", "Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fw,  sys573,   konami573, konami573, konami573,  ROT0, "Konami", "CD-ROM Drive Updater 2.0 (700B04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fwa, sys573,   konami573, konami573, konami573,  ROT0, "Konami", "CD-ROM Drive Updater (700A04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mk,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 2000, ddr3mka,  ddr3mk,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddr3ma,   ddr3mk,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.1 */
GAME( 1999, ddr3mj,   ddr3mk,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.0 */
GAME( 1999, ddrsbm,   sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, ddrs2k,   sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddrs2kj,  ddrs2k,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.2 */
GAME( 1999, dsfdct,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JCA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, dsfdcta,  dsfdct,   konami573, ddr,       ddr,        ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn2m,   sys573,   konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2mpu, drmn2m,   konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 2000, dncfrks,  sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Freaks (G*874 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmx,      dncfrks,  konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax (G*874 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, gtrfrk3m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3ma, gtrfrk3m, konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3mb, gtrfrk3m, konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, salarymc, sys573,   konami573, konami573, salarymc,   ROT0, "Konami", "Salary Man Champ (GCA18 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mp,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, pcnfrk3m, sys573,   konami573, drmn,      drmndigital,ROT0, "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, drmn3m,   pcnfrk3m, konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 3rd Mix (G*A23 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, gtrfrk4m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 4th Mix (G*A24 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4m,    sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mj,   ddr4m,    konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4ms,   sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. ABA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4msj,  ddr4ms,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddrusa,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution USA (G*A44 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mp,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, ddr4mps,  sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, dmx2m,    sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix (G*A39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk5m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 5th Mix (G*A26 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, ddr5m,    sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 5th Mix (G*A27 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, dmx2majp, sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk6m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 6th Mix (G*B06 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk7m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 7th Mix (G*B17 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, ddrmax,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "DDR Max - Dance Dance Revolution 6th Mix (G*B19 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, ddrmax2,  sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "DDR Max 2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, dsem2,    sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage Euro Mix 2 (G*C23 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, ddrextrm, sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution Extreme (G*C36 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, gtfrk11m, sys573,   konami573, gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 11th Mix (G*D39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
