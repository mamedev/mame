// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************
    BBC Model A,B

    ANA01 - Model A
    ANA02 - Model A with Econet interface

    ANB01 - Model B
    ANB02 - Model B with Econet interface
    ANB03 - Model B with Disc interface
    ANB04 - Model B with Disc and Econet interfaces

    GNB14 - Model B with Disc, Econet & Speech (German model)
    UNB09 - Model B with Disc, Econet & Speech (US model)

    BBC Model B+

    ANB51 - BBC Model B+ 64K
    ANB52 - BBC Model B+ 64K with Econet
    ANB53 - BBC Model B+ 64K with Disc interface
    ANB54 - BBC Model B+ 64K with Disc and Econet interfaces
    ANB55 - BBC Model B+ 128K with Disc interface

    BBC Master

    AMB15 - Master 128
    ADB12 - Master Econet Terminal
    AVC12 - Master AIV
    ARM1  - ARM Evaluation System
    ADB20 - Master Compact


    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@romvault.com
    Nigel Barnes
    ngbarnes@hotmail.com

******************************************************************************/

/* Core includes */
#include "emu.h"

/* Components */
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/6522via.h"
#include "machine/mc146818.h"       /* RTC & CMOS RAM */
#include "bus/centronics/ctronics.h"
#include "bus/econet/econet.h"
#include "sound/tms5220.h"          /* Speech */
#include "video/saa5050.h"          /* Teletext */
#include "bbc.lh"

/* Devices */
#include "imagedev/flopdrv.h"
#include "formats/bbc_dsk.h"
#include "formats/basicdsk.h"
#include "imagedev/cassette.h"
#include "formats/uef_cas.h"
#include "formats/csw_cas.h"
#include "includes/bbc.h"


/******************************************************************************
A  = BBC Model A
B  = BBC Model B
B+ = BBC Model B+
M  = BBC Master

                    A   B   B+  M
+   &0000           +   +   +   +
|                   |   |   |   |
|   &1000           |   |   |   |
|1                  |   |   |1  |1
|   &2000           |1  |   |   |
|                   |   |   |   |
+   &3000           |   |   +   +
|2                  |   |   |   |
+   &4000           +   |1  |   |
|                   |   |   |   |
|   &5000           |   |   |   |
|                   |   |   |2  |2
|3  &6000           |3  |   |   |
|                   |   |   |   |
|   &7000           |   |   |   |
|                   |   |   |   |
+   &8000           +   +   +   +
|4                  |   |   |   |4
+   &9000           |   |   |   +
|                   |   |   |4  |
|5  &A000           |4  |4  |   |5
|                   |   |   |   |
+   &B000           |   |   +   |
|6                  |   |   |6  |
+   &C000           +   +   +   +
|                   |   |   |   |7
|7  &D000           |   |   |   |
|                   |   |   |   |
+   &E000           |7  |7  |7  +
|                   |   |   |   |
|8  &F000           |   |   |   |8
|                   |   |   |   |
+   &FC00 FRED      +   +   +   +
    &FD00 JIM       +   +   +   +
    &FE00 SHEILA    +   +   +   +
+   &FF00           +   +   +   +
|9                  |9  |9  |9  |9
+   &FFFF           +   +   +   +



&00-&07 6845 CRTC       Video controller                8  ( 2 bytes x  4 ) 1MHz
&08-&0F 6850 ACIA       Serial controller               8  ( 2 bytes x  4 ) 1MHz
&10-&17 Serial ULA      Serial system chip              8  ( 1 byte  x  8 ) 1MHz
&18-&1f INTOFF/STATID   ECONET Interrupt Off / ID No.   8  ( 1 byte  x  8 ) 1MHz
write:
&20-&2F Video ULA       Video system chip               16 ( 2 bytes x  8 ) 2MHz
&30-&3F 74LS161         Paged ROM selector              16 ( 1 byte  x 16 ) 2MHz
read:
&20-&2F INTON           ECONET Interrupt On             16 ( 1 bytes x 16 ) 2MHz
&30-&3F Not Connected   Not Connected                                       2MHz

&40-&5F 6522 VIA        SYSTEM VIA                      32 (16 bytes x  2 ) 1MHz
&60-&7F 6522 VIA        USER VIA                        32 (16 bytes x  2 ) 1MHz
&80-&9F 8271 FDC        FDC Floppy disc controller      32 ( 8 bytes x  4 ) 2MHz
&A0-&BF 68B54 ADLC      ECONET controller               32 ( 4 bytes x  8 ) 2MHz
&C0-&DF uPD7002         Analogue to digital converter   32 ( 4 bytes x  8 ) 1MHz
&E0-&FF Tube ULA        Tube system interface           32 (32 bytes x  1 ) 2MHz
******************************************************************************/

READ8_MEMBER(bbc_state::bbc_fe_r)
{
	return 0xfe;
}

static ADDRESS_MAP_START( bbca_mem, AS_PROGRAM, 8, bbc_state )
	ADDRESS_MAP_UNMAP_HIGH                                                                      /*  Hardware marked with a # is not present in a Model A        */

	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE(bbc_memorya1_w)                     /*    0000-3fff                 Regular Ram                     */
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank3") AM_WRITE(bbc_memoryb3_w)                     /*    4000-7fff                 Repeat of the Regular Ram       */
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank4")                                              /*    8000-bfff                 Paged ROM                       */
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank7")                                              /*    c000-fbff                 OS ROM                          */
	AM_RANGE(0xfc00, 0xfdff) AM_NOP                                                             /*    fc00-fdff                 FRED & JIM Pages                */
																								/*    fe00-feff                 Shiela Address Page             */
	AM_RANGE(0xfe00, 0xfe07) AM_READWRITE(bbc_6845_r, bbc_6845_w)                               /*    fe00-fe07  6845 CRTC      Video controller                */
	AM_RANGE(0xfe08, 0xfe08) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, status_r, control_w)
	AM_RANGE(0xfe09, 0xfe09) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xfe10, 0xfe17) AM_READWRITE(bbc_fe_r, bbc_SerialULA_w)                            /*    fe10-fe17  Serial ULA     Serial system chip              */
	AM_RANGE(0xfe18, 0xfe1f) AM_NOP                                                             /*    fe18-fe1f  INTOFF/STATID  # ECONET Interrupt Off / ID No. */
	AM_RANGE(0xfe20, 0xfe2f) AM_WRITE(bbc_videoULA_w)                                           /* R: fe20-fe2f  INTON          # ECONET Interrupt On           */
																								/* W: fe20-fe2f  Video ULA      Video system chip               */
	AM_RANGE(0xfe30, 0xfe3f) AM_READWRITE(bbc_fe_r, bbc_page_selecta_w)                         /* R: fe30-fe3f  NC             Not Connected                   */
																								/* W: fe30-fe3f  84LS161        Paged ROM selector              */
	AM_RANGE(0xfe40, 0xfe5f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)          /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	AM_RANGE(0xfe60, 0xfe7f) AM_NOP                                                             /*    fe60-fe7f  6522 VIA       # USER VIA                      */
	AM_RANGE(0xfe80, 0xfe9f) AM_NOP                                                             /*    fe80-fe9f  8271/1770 FDC  # Floppy disc controller        */
	AM_RANGE(0xfea0, 0xfebf) AM_READ(bbc_fe_r)                                                  /*    fea0-febf  68B54 ADLC     # ECONET controller             */
	AM_RANGE(0xfec0, 0xfedf) AM_NOP                                                             /*    fec0-fedf  uPD7002        # Analogue to digital converter */
	AM_RANGE(0xfee0, 0xfeff) AM_READ(bbc_fe_r)                                                  /*    fee0-feff  Tube ULA       # Tube system interface         */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("os", 0x3f00)                                 /*    ff00-ffff                 OS Rom (continued)              */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbcb_mem, AS_PROGRAM, 8, bbc_state )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE(bbc_memorya1_w)                     /*    0000-3fff                 Regular Ram                     */
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank3") AM_WRITE(bbc_memoryb3_w)                     /*    4000-7fff                 Repeat of the Regular Ram       */
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank4") AM_WRITE(bbc_memoryb4_w)                     /*    8000-bfff                 Paged ROM                       */
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank7")                                              /*    c000-fbff                 OS ROM                          */
	AM_RANGE(0xfc00, 0xfdff) AM_READWRITE(bbc_opus_read, bbc_opus_write)                        /*    fc00-fdff                 OPUS Disc Controller            */
																								/*    fe00-feff                 Shiela Address Page             */
	AM_RANGE(0xfe00, 0xfe07) AM_READWRITE(bbc_6845_r, bbc_6845_w)                               /*    fe00-fe07  6845 CRTC      Video controller                */
	AM_RANGE(0xfe08, 0xfe08) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, status_r, control_w)
	AM_RANGE(0xfe09, 0xfe09) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xfe10, 0xfe17) AM_READWRITE(bbc_fe_r, bbc_SerialULA_w)                            /*    fe10-fe17  Serial ULA     Serial system chip              */
	AM_RANGE(0xfe18, 0xfe1f) AM_NOP                                                             /*    fe18-fe1f  INTOFF/STATID  ECONET Interrupt Off / ID No.   */
	AM_RANGE(0xfe20, 0xfe2f) AM_WRITE(bbc_videoULA_w)                                           /* R: fe20-fe2f  INTON          ECONET Interrupt On             */
																								/* W: fe20-fe2f  Video ULA      Video system chip               */
	AM_RANGE(0xfe30, 0xfe3f) AM_READWRITE(bbc_fe_r, bbc_page_selectb_w)                         /* R: fe30-fe3f  NC             Not Connected                   */
																								/* W: fe30-fe3f  84LS161        Paged ROM selector              */
	AM_RANGE(0xfe40, 0xfe5f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)          /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	AM_RANGE(0xfe60, 0xfe7f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)          /*    fe60-fe7f  6522 VIA       USER VIA                        */
	AM_RANGE(0xfe80, 0xfe9f) AM_READWRITE(bbc_disc_r, bbc_disc_w)                               /*    fe80-fe9f  8271 FDC       Floppy disc controller          */
	AM_RANGE(0xfea0, 0xfebf) AM_READ(bbc_fe_r)                                                  /*    fea0-febf  68B54 ADLC     ECONET controller               */
	AM_RANGE(0xfec0, 0xfedf) AM_DEVREADWRITE("upd7002", upd7002_device, read, write)            /*    fec0-fedf  uPD7002        Analogue to digital converter   */
	AM_RANGE(0xfee0, 0xfeff) AM_READ(bbc_fe_r)                                                  /*    fee0-feff  Tube ULA       Tube system interface           */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("os", 0x3f00)                                 /*    ff00-ffff                 OS Rom (continued)              */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbcbp_mem, AS_PROGRAM, 8, bbc_state )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x2fff) AM_READ_BANK("bank1") AM_WRITE(bbc_memorybp1_w)                    /*    0000-2fff                 Regular Ram                     */
	AM_RANGE(0x3000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE(bbc_memorybp2_w)                    /*    3000-7fff                 Video/Shadow Ram                */
	AM_RANGE(0x8000, 0xafff) AM_READ_BANK("bank4") AM_WRITE(bbc_memorybp4_w)                    /*    8000-afff                 Paged ROM or 12K of SWRAM       */
	AM_RANGE(0xb000, 0xbfff) AM_READ_BANK("bank6")                                              /*    b000-bfff                 Rest of paged ROM area          */
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank7")                                              /*    c000-fbff                 OS ROM                          */
	AM_RANGE(0xfc00, 0xfdff) AM_NOP                                                             /*    fc00-fdff                 FRED & JIM Pages                */
																								/*    fe00-feff                 Shiela Address Page             */
	AM_RANGE(0xfe00, 0xfe07) AM_READWRITE(bbc_6845_r, bbc_6845_w)                               /*    fe00-fe07  6845 CRTC      Video controller                */
	AM_RANGE(0xfe08, 0xfe08) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, status_r, control_w)
	AM_RANGE(0xfe09, 0xfe09) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xfe10, 0xfe17) AM_READWRITE(bbc_fe_r, bbc_SerialULA_w)                            /*    fe10-fe17  Serial ULA     Serial system chip              */
	AM_RANGE(0xfe18, 0xfe1f) AM_NOP                                                             /*    fe18-fe1f  INTOFF/STATID  ECONET Interrupt Off / ID No.   */
	AM_RANGE(0xfe20, 0xfe2f) AM_WRITE(bbc_videoULA_w)                                           /* R: fe20-fe2f  INTON          ECONET Interrupt On             */
																								/* W: fe20-fe2f  Video ULA      Video system chip               */
	AM_RANGE(0xfe30, 0xfe3f) AM_READWRITE(bbc_fe_r, bbc_page_selectbp_w)                        /* R: fe30-fe3f  NC             Not Connected                   */
																								/* W: fe30-fe3f  84LS161        Paged ROM selector              */
	AM_RANGE(0xfe40, 0xfe5f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)          /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	AM_RANGE(0xfe60, 0xfe7f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)          /*    fe60-fe7f  6522 VIA       USER VIA                        */
	AM_RANGE(0xfe80, 0xfe9f) AM_READWRITE(bbc_wd1770_read, bbc_wd1770_write)                    /*    fe80-fe9f  1770 FDC       Floppy disc controller          */
	AM_RANGE(0xfea0, 0xfebf) AM_READ(bbc_fe_r)                                                  /*    fea0-febf  68B54 ADLC     ECONET controller               */
	AM_RANGE(0xfec0, 0xfedf) AM_DEVREADWRITE("upd7002", upd7002_device, read, write)            /*    fec0-fedf  uPD7002        Analogue to digital converter   */
	AM_RANGE(0xfee0, 0xfeff) AM_READ(bbc_fe_r)                                                  /*    fee0-feff  Tube ULA       Tube system interface           */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("os", 0x3f00)                                 /*    ff00-ffff                 OS Rom (continued)              */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbcbp128_mem, AS_PROGRAM, 8, bbc_state )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x2fff) AM_READ_BANK("bank1") AM_WRITE(bbc_memorybp1_w)                    /*    0000-2fff                 Regular Ram                     */
	AM_RANGE(0x3000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE(bbc_memorybp2_w)                    /*    3000-7fff                 Video/Shadow Ram                */
	AM_RANGE(0x8000, 0xafff) AM_READ_BANK("bank4") AM_WRITE(bbc_memorybp4_128_w)                /*    8000-afff                 Paged ROM or 12K of SWRAM       */
	AM_RANGE(0xb000, 0xbfff) AM_READ_BANK("bank6") AM_WRITE(bbc_memorybp6_128_w)                /*    b000-bfff                 Rest of paged ROM area          */
	AM_RANGE(0xc000, 0xfbff) AM_READ_BANK("bank7")                                              /*    c000-fbff                 OS ROM                          */
	AM_RANGE(0xfc00, 0xfdff) AM_NOP                                                             /*    fc00-fdff                 FRED & JIM Pages                */
																								/*    fe00-feff                 Shiela Address Page             */
	AM_RANGE(0xfe00, 0xfe07) AM_READWRITE(bbc_6845_r, bbc_6845_w)                               /*    fe00-fe07  6845 CRTC      Video controller                */
	AM_RANGE(0xfe08, 0xfe08) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, status_r, control_w)
	AM_RANGE(0xfe09, 0xfe09) AM_MIRROR(0x06) AM_DEVREADWRITE("acia6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xfe10, 0xfe17) AM_READWRITE(bbc_fe_r, bbc_SerialULA_w)                            /*    fe10-fe17  Serial ULA     Serial system chip              */
	AM_RANGE(0xfe10, 0xfe17) AM_NOP                                                             /*    fe10-fe17  Serial ULA     Serial system chip              */
	AM_RANGE(0xfe18, 0xfe1f) AM_NOP                                                             /*    fe18-fe1f  INTOFF/STATID  ECONET Interrupt Off / ID No.   */
	AM_RANGE(0xfe20, 0xfe2f) AM_WRITE(bbc_videoULA_w)                                           /* R: fe20-fe2f  INTON          ECONET Interrupt On             */
																								/* W: fe20-fe2f  Video ULA      Video system chip               */
	AM_RANGE(0xfe30, 0xfe3f) AM_READWRITE(bbc_fe_r, bbc_page_selectbp_w)                        /* R: fe30-fe3f  NC             Not Connected                   */
																								/* W: fe30-fe3f  84LS161        Paged ROM selector              */
	AM_RANGE(0xfe40, 0xfe5f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)          /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	AM_RANGE(0xfe60, 0xfe7f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)          /*    fe60-fe7f  6522 VIA       USER VIA                        */
	AM_RANGE(0xfe80, 0xfe9f) AM_READWRITE(bbc_wd1770_read, bbc_wd1770_write)                    /*    fe80-fe9f  1770 FDC       Floppy disc controller          */
	AM_RANGE(0xfea0, 0xfebf) AM_READ(bbc_fe_r)                                                  /*    fea0-febf  68B54 ADLC     ECONET controller               */
	AM_RANGE(0xfec0, 0xfedf) AM_DEVREADWRITE("upd7002", upd7002_device, read, write)            /*    fec0-fedf  uPD7002        Analogue to digital converter   */
	AM_RANGE(0xfee0, 0xfeff) AM_READ(bbc_fe_r)                                                  /*    fee0-feff  Tube ULA       Tube system interface           */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("os", 0x3f00)                                 /*    ff00-ffff                 OS Rom (continued)              */
ADDRESS_MAP_END


/******************************************************************************
&FC00-&FCFF FRED
&FD00-&FDFF JIM
&FE00-&FEFF SHEILA      Read                    Write
&00-&07 6845 CRTC       Video controller        Video Controller         8 ( 2 bytes x  4 )
&08-&0F 6850 ACIA       Serial controller       Serial Controller        8 ( 2 bytes x  4 )
&10-&17 Serial ULA      -                       Serial system chip       8 ( 1 byte  x  8 )
&18-&1F uPD7002         A to D converter        A to D converter         8 ( 4 bytes x  2 )

&20-&23 Video ULA       -                       Video system chip        4 ( 2 bytes x  2 )
&24-&27 FDC Latch       1770 Control latch      1770 Control latch       4 ( 1 byte  x  4 )
&28-&2F 1770 registers  1770 Disc Controller    1170 Disc Controller     8 ( 4 bytes x  2 )
&30-&33 ROMSEL          -                       ROM Select               4 ( 1 byte  x  4 )
&34-&37 ACCCON          ACCCON select reg.      ACCCON select reg        4 ( 1 byte  x  4 )
&38-&3F NC              -                       -
&40-&5F 6522 VIA        SYSTEM VIA              SYSTEM VIA              32 (16 bytes x  2 ) 1MHz
&60-&7F 6522 VIA        USER VIA                USER VIA                32 (16 bytes x  2 ) 1MHz
&80-&9F Int. Modem      Int. Modem              Int Modem
&A0-&BF 68B54 ADLC      ECONET controller       ECONET controller       32 ( 4 bytes x  8 ) 2MHz
&C0-&DF NC              -                       -
&E0-&FF Tube ULA        Tube system interface   Tube system interface   32 (32 bytes x  1 ) 2MHz
******************************************************************************/


static ADDRESS_MAP_START(bbcm_mem, AS_PROGRAM, 8, bbc_state )
	AM_RANGE(0x0000, 0x2fff) AM_READ_BANK("bank1") AM_WRITE(bbc_memorybm1_w)                    /*    0000-2fff                 Regular Ram                     */
	AM_RANGE(0x3000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE(bbc_memorybm2_w)                    /*    3000-7fff                 Video/Shadow Ram                */
	AM_RANGE(0x8000, 0x8fff) AM_READ_BANK("bank4") AM_WRITE(bbc_memorybm4_w)                    /*    8000-8fff                 Paged ROM/RAM or 4K of RAM ANDY */
	AM_RANGE(0x9000, 0xbfff) AM_READ_BANK("bank5") AM_WRITE(bbc_memorybm5_w)                    /*    9000-bfff                 Rest of paged ROM/RAM area      */
	AM_RANGE(0xc000, 0xdfff) AM_READ_BANK("bank7") AM_WRITE(bbc_memorybm7_w)                    /*    c000-dfff                 OS ROM or 8K of RAM       HAZEL */
	AM_RANGE(0xe000, 0xfbff) AM_ROM AM_REGION("os", 0x2000)                                 /*    e000-fbff                 OS ROM                          */
	AM_RANGE(0xfc00, 0xfeff) AM_READ_BANK("bank8") AM_WRITE(bbcm_w)                             /*    this is now processed directly because it can be ROM or hardware */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("os", 0x3f00)                                 /*    ff00-ffff                 OS ROM (continued)              */
ADDRESS_MAP_END


static const rgb_t bbc_palette[8]=
{
	rgb_t(0x0ff,0x0ff,0x0ff),
	rgb_t(0x000,0x0ff,0x0ff),
	rgb_t(0x0ff,0x000,0x0ff),
	rgb_t(0x000,0x000,0x0ff),
	rgb_t(0x0ff,0x0ff,0x000),
	rgb_t(0x000,0x0ff,0x000),
	rgb_t(0x0ff,0x000,0x000),
	rgb_t(0x000,0x000,0x000)
};

PALETTE_INIT_MEMBER(bbc_state, bbc)
{
	palette.set_pen_colors(0, bbc_palette, ARRAY_LENGTH(bbc_palette));
}


INPUT_CHANGED_MEMBER(bbc_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}


/*  Port                                        Key description                 Emulated key                    Natural key         Shift 1         Shift 2 (Ctrl) */

static INPUT_PORTS_START(bbc_keyboard)
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")                  PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0")                 PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !")                PORT_CODE(KEYCODE_1)            PORT_CHAR('1')      PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPSLOCK")           PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFTLOCK")          PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB")                PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESCAPE")             PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")               PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #")                PORT_CODE(KEYCODE_3)            PORT_CHAR('3')      PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")                  PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"")               PORT_CODE(KEYCODE_2)            PORT_CHAR('2')      PORT_CHAR('\"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")                  PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")                  PORT_CODE(KEYCODE_S)            PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")                  PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")                 PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")                  PORT_CODE(KEYCODE_4)            PORT_CHAR('4')      PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")                  PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")                  PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")                  PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")                  PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")                 PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %")                PORT_CODE(KEYCODE_5)            PORT_CHAR('5')      PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")                  PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")                  PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")                  PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")                  PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")                  PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")                 PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")                 PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 \\")               PORT_CODE(KEYCODE_7)            PORT_CHAR('7')      PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &")                PORT_CODE(KEYCODE_6)            PORT_CHAR('6')      PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")                  PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")                  PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")                  PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")                 PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (")                PORT_CODE(KEYCODE_8)            PORT_CHAR('8')      PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")                  PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")                  PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")                  PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")                  PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")                  PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6")                 PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7")                 PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )")                PORT_CODE(KEYCODE_9)            PORT_CHAR('9')      PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")                  PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")                  PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")                  PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <")                PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')      PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8")                 PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =")                PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')      PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")                  PORT_CODE(KEYCODE_0)            PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")                  PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@")                  PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +")                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')      PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >")                PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')      PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F9")                 PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~")                PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_ \xC2\xA3")         PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('_') PORT_CHAR('\xA3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {")                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *")                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }")                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?")                PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |")               PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT")               PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN")               PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP")                 PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE")             PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")               PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT")              PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("BRK")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK")              PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, trigger_reset, 0)

	/* Keyboard columns 10 -> 12 are reserved for BBC Master */
	PORT_START("COL10")
	PORT_START("COL11")
	PORT_START("COL12")

	PORT_START("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_keypad)
	PORT_MODIFY("COL10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 6")           PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 8")           PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad +")           PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad /")           PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad #")           PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 0")           PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 4")           PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_MODIFY("COL11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 7")           PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 9")           PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad -")           PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad DELETE")      PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad *")           PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1")           PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5")           PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	PORT_MODIFY("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad RETURN")      PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad .")           PORT_CODE(KEYCODE_STOP)         PORT_CHAR(UCHAR_MAMEKEY(STOP))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,")           PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(UCHAR_MAMEKEY(COMMA))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3")           PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2")           PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_dipswitch)
	PORT_MODIFY("COL2")
	PORT_DIPNAME(0x01, 0x01, "DIP 8 (Default File System)")
	PORT_DIPSETTING(   0x00, "NFS" )
	PORT_DIPSETTING(   0x01, "DFS" )

	PORT_MODIFY("COL3")
	PORT_DIPNAME(0x01, 0x01, "DIP 7 (Not Used)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL4")
	PORT_DIPNAME(0x01, 0x01, "DIP 6 (Disc Timings)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL5")
	PORT_DIPNAME(0x01, 0x01, "DIP 5 (Disc Timings)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL6")
	PORT_DIPNAME(0x01, 0x01, "DIP 4 (Boot)")
	PORT_DIPSETTING(   0x00, "SHIFT" )
	PORT_DIPSETTING(   0x01, "SHIFT-BREAK" )

	PORT_MODIFY("COL7")
	PORT_DIPNAME(0x01, 0x01, "DIP 3 (Screen Mode)")
	PORT_DIPSETTING(   0x00, "+0" )
	PORT_DIPSETTING(   0x01, "+4" )

	PORT_MODIFY("COL8")
	PORT_DIPNAME(0x01, 0x01, "DIP 2 (Screen Mode)")
	PORT_DIPSETTING(   0x00, "+0" )
	PORT_DIPSETTING(   0x01, "+2" )

	PORT_MODIFY("COL9")
	PORT_DIPNAME(0x01, 0x01, "DIP 1 (Screen Mode)")
	PORT_DIPSETTING(   0x00, "+0" )
	PORT_DIPSETTING(   0x01, "+1" )
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_joy)
	PORT_START("JOY0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x0,0xff ) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x0,0xff ) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x0,0xff ) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x0,0xff ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_config)
	PORT_START("BBCCONFIG")

//  PORT_CONFNAME( 0x01, 0x00, "Speech Upgrade" )
//  PORT_CONFSETTING(    0x00, DEF_STR( On ) )
//  PORT_CONFSETTING(    0x01, DEF_STR( Off ) )

	PORT_CONFNAME( 0x07, 0x00, "DFS Select" )
	PORT_CONFSETTING(    0x00, "Acorn DFS 0.90 (read only)"  )
	PORT_CONFSETTING(    0x01, "Acorn DNFS 1.20 (read only)" )
	PORT_CONFSETTING(    0x02, "Watford DFS 1.44 (read only)" )
	PORT_CONFSETTING(    0x03, "Acorn DFS E00 (hack / read only)" )
	PORT_CONFSETTING(    0x04, "Acorn DDFS" )
	PORT_CONFSETTING(    0x05, "Watford DDFS (not working)" )
	PORT_CONFSETTING(    0x06, "Opus Challenger 512K (RAM drive only)" )
	PORT_CONFSETTING(    0x07, DEF_STR( None ) )

	PORT_CONFNAME( 0x18, 0x00, "Sideways RAM Type" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x08, "Solidisk 128K (fe62)" )
	PORT_CONFSETTING(    0x10, "Acorn 64K (fe30)" )
	PORT_CONFSETTING(    0x18, "Acorn 128K (fe30)" )
//  PORT_CONFSETTING(    0x20, "ATPL Sidewise 16K" )
INPUT_PORTS_END

static INPUT_PORTS_START(bbca)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcb)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbc_joy)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcm)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_keypad)
	PORT_INCLUDE(bbc_joy)
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(bbc_state::bbcb_vsync)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	via_0->write_ca1(1);
	via_0->write_ca1(0);
}


//static const struct TMS5220interface tms5220_interface =
//{
//  680000L,
//  50,
//  bbc_TMSint
//};



WRITE_LINE_MEMBER(bbc_state::bbcb_acia6850_irq_w)
{
	m_acia_irq = state;

	check_interrupts();
}

static LEGACY_FLOPPY_OPTIONS_START(bbc)
	LEGACY_FLOPPY_OPTION( ssd80, "bbc,img,ssd", "BBC SSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( dsd80, "dsd", "BBC DSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface bbc_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(bbc),
	"floppy_5_25"
};

FLOPPY_FORMATS_MEMBER( bbc_state::floppy_formats )
	FLOPPY_BBC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( bbc_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
	SLOT_INTERFACE("sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("ssdd", FLOPPY_525_SSDD)
	SLOT_INTERFACE("dd",   FLOPPY_525_DD)
	SLOT_INTERFACE("ssqd", FLOPPY_525_SSQD)
	SLOT_INTERFACE("qd",   FLOPPY_525_QD)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER(bbc_state::econet_clk_w)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
}

// 4 x EPROM sockets (16K) in BBC-A, these should grow to 16 for BBC-B and later...
static MACHINE_CONFIG_FRAGMENT( bbc_eprom_sockets )
	MCFG_GENERIC_SOCKET_ADD("exp_rom1", generic_linear_slot, "bbc_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(bbc_state, exp1_load)

	MCFG_GENERIC_SOCKET_ADD("exp_rom2", generic_linear_slot, "bbc_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(bbc_state, exp2_load)

	MCFG_GENERIC_SOCKET_ADD("exp_rom3", generic_linear_slot, "bbc_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(bbc_state, exp3_load)

	MCFG_GENERIC_SOCKET_ADD("exp_rom4", generic_linear_slot, "bbc_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(bbc_state, exp4_load)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( bbca, bbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 2000000)         /* 2.00 MHz */
	MCFG_CPU_PROGRAM_MAP(bbca_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bbc_state, bbcb_vsync)      /* screen refresh interrupts */
	MCFG_CPU_PERIODIC_INT_DRIVER(bbc_state, bbcb_keyscan, 1000)        /* scan keyboard */
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbca)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbca)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(128))
	MCFG_SCREEN_UPDATE_DEVICE("mc6845", mc6845_device, screen_update)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(bbc_state,bbc)

	MCFG_DEVICE_ADD("saa5050", SAA5050, XTAL_12MHz/2)
	MCFG_SAA5050_SCREEN_SIZE(40, 24, 40)

	/* crtc */
	MCFG_MC6845_ADD("mc6845", MC6845, "screen", 2000000)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(bbc_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(bbc_state, bbc_vsync))

	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbca)

	MCFG_DEFAULT_LAYOUT(layout_bbc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489", SN76489, 4000000) /* 4 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(bbc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("bbc_cass")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_ls_a", "bbca_cass")

	/* acia */
	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(bbc_state, bbc_txd_w))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(bbc_state, bbc_rts_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_acia6850_irq_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(WRITELINE(bbc_state, write_rxd_serial))
	MCFG_RS232_DCD_HANDLER(WRITELINE(bbc_state, write_dcd_serial))
	MCFG_RS232_CTS_HANDLER(WRITELINE(bbc_state, write_cts_serial))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, XTAL_16MHz / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bbc_state, write_acia_clock))

	/* system via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPA_HANDLER(READ8(bbc_state, bbcb_via_system_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(bbc_state, bbcb_via_system_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(bbc_state, bbcb_via_system_write_porta))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bbc_state, bbcb_via_system_write_portb))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_via_system_irq_w))

	/* EPROM sockets */
	MCFG_FRAGMENT_ADD(bbc_eprom_sockets)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcb, bbca )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(bbcb_mem)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcb)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcb)
	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbcb)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* speech hardware */
//  MCFG_SOUND_ADD("tms5220", TMS5220, 640000)
//  MCFG_TMS52XX_SPEECHROM("vsm")

	/* user via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 1000000)
	MCFG_VIA6522_READPB_HANDLER(READ8(bbc_state, bbcb_via_user_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bbc_state, bbcb_via_user_write_portb))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_via_user_irq_w))

	/* adc */
	MCFG_DEVICE_ADD("upd7002", UPD7002, 0)
	MCFG_UPD7002_GET_ANALOGUE_CB(bbc_state, BBC_get_analogue_input)
	MCFG_UPD7002_EOC_CB(bbc_state, BBC_uPD7002_EOC)

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* fdc */
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(bbc_state, bbc_i8271_interrupt))
	MCFG_I8271_FLOPPIES(FLOPPY_0, FLOPPY_1)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(bbc_floppy_interface)

	MCFG_WD1770_ADD("wd177x", XTAL_16MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd177x:0", bbc_floppies, "qd", bbc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd177x:1", bbc_floppies, "qd", bbc_state::floppy_formats)

	/* software lists */
	MCFG_DEVICE_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_ADD("cass_ls_b", "bbcb_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_a", "bbca_cass")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcb_us, bbca )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(bbcb_mem)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcb )
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcb )
	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbcb )

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* speech hardware */
//  MCFG_SOUND_ADD("tms5220", TMS5220, 640000)
//  MCFG_TMS52XX_SPEECHROM("vsm")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_REFRESH_RATE(60)

	/* system via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 1000000)
	MCFG_VIA6522_READPB_HANDLER(READ8(bbc_state, bbcb_via_user_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bbc_state, bbcb_via_user_write_portb))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_via_user_irq_w))

	/* adc */
	MCFG_DEVICE_ADD("upd7002", UPD7002, 0)
	MCFG_UPD7002_GET_ANALOGUE_CB(bbc_state, BBC_get_analogue_input)
	MCFG_UPD7002_EOC_CB(bbc_state, BBC_uPD7002_EOC)

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* fdc */
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(bbc_state, bbc_i8271_interrupt))
	MCFG_I8271_FLOPPIES(FLOPPY_0, FLOPPY_1)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(bbc_floppy_interface)

	MCFG_WD1770_ADD("wd177x", XTAL_16MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd177x:0", bbc_floppies, "qd", bbc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd177x:1", bbc_floppies, "qd", bbc_state::floppy_formats)

	/* software lists */
	MCFG_DEVICE_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_ADD("cass_ls_b", "bbcb_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_a", "bbca_cass")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcbp, bbcb )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )  /* M6512 */
	MCFG_CPU_PROGRAM_MAP(bbcbp_mem)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcbp)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcbp)
	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbcbp)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* fdc */
	MCFG_DEVICE_REMOVE("i8271")
	MCFG_DEVICE_REMOVE(FLOPPY_0)
	MCFG_DEVICE_REMOVE(FLOPPY_1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcbp128, bbcbp )
	/* basic machine hardware */
	MCFG_CPU_MODIFY( "maincpu" )  /* M6512 */
	MCFG_CPU_PROGRAM_MAP(bbcbp128_mem)
	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcbp)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcbp)
	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbcbp)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END


/* BBC Master Series */

static MACHINE_CONFIG_START( bbcm, bbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, 2000000)        /* 2.00 MHz */
	MCFG_CPU_PROGRAM_MAP(bbcm_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bbc_state, bbcb_vsync)      /* screen refresh interrupts */
	MCFG_CPU_PERIODIC_INT_DRIVER(bbc_state, bbcb_keyscan, 1000)        /* scan keyboard */
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcm)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcm)

	MCFG_DEFAULT_LAYOUT(layout_bbc)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(128))
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DEVICE("mc6845", mc6845_device, screen_update)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(bbc_state,bbc)

	MCFG_DEVICE_ADD("saa5050", SAA5050, XTAL_12MHz/2)
	MCFG_SAA5050_SCREEN_SIZE(40, 24, 40)

	/* crtc */
	MCFG_MC6845_ADD("mc6845", MC6845, "screen", 2000000)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(bbc_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(bbc_state, bbc_vsync))

	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbcm)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489", SN76489, 4000000) /* 4 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* rtc and cmos */
	MCFG_MC146818_ADD( "rtc", XTAL_32_768kHz )

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(bbc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY)
	MCFG_CASSETTE_INTERFACE("bbc_cass")

	// 2 x EPROM sockets (32K) in BBC-Master
	MCFG_GENERIC_SOCKET_ADD("exp_rom1", generic_plain_slot, "bbcm_cart")
	MCFG_GENERIC_LOAD(bbc_state, bbcm_exp1_load)

	MCFG_GENERIC_SOCKET_ADD("exp_rom2", generic_plain_slot, "bbcm_cart")
	MCFG_GENERIC_LOAD(bbc_state, bbcm_exp2_load)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_ls_m", "bbcm_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_a", "bbca_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_b", "bbcb_cass")
	MCFG_SOFTWARE_LIST_ADD("cart_ls_m", "bbcm_cart")

	/* acia */
	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(bbc_state, bbc_txd_w))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(bbc_state, bbc_rts_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_acia6850_irq_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(WRITELINE(bbc_state, write_rxd_serial))
	MCFG_RS232_DCD_HANDLER(WRITELINE(bbc_state, write_dcd_serial))
	MCFG_RS232_CTS_HANDLER(WRITELINE(bbc_state, write_cts_serial))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, XTAL_16MHz / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bbc_state, write_acia_clock))

	/* adc */
	MCFG_DEVICE_ADD("upd7002", UPD7002, 0)
	MCFG_UPD7002_GET_ANALOGUE_CB(bbc_state, BBC_get_analogue_input)
	MCFG_UPD7002_EOC_CB(bbc_state, BBC_uPD7002_EOC)

	/* system via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPA_HANDLER(READ8(bbc_state, bbcb_via_system_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(bbc_state, bbcb_via_system_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(bbc_state, bbcb_via_system_write_porta))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bbc_state, bbcb_via_system_write_portb))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_via_system_irq_w))

	/* user via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 1000000)
	MCFG_VIA6522_READPB_HANDLER(READ8(bbc_state, bbcb_via_user_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bbc_state, bbcb_via_user_write_portb))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(bbc_state, bbcb_via_user_irq_w))

	/* fdc */
	MCFG_WD1770_ADD("wd177x", XTAL_16MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd177x:0", bbc_floppies, "qd", bbc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd177x:1", bbc_floppies, "qd", bbc_state::floppy_formats)

	/* econet */
	MCFG_DEVICE_ADD("mc6854", MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(DEVWRITELINE(ECONET_TAG, econet_device, data_w))
	MCFG_ECONET_ADD()
	MCFG_ECONET_CLK_CALLBACK(WRITELINE(bbc_state, econet_clk_w))
	MCFG_ECONET_DATA_CALLBACK(DEVWRITELINE("mc6854", mc6854_device, set_rx))
	MCFG_ECONET_SLOT_ADD("econet254", 254, econet_devices, NULL)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcmt, bbcm )

	/* Add 65C102 co-processor */

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcmaiv, bbcm )

	/* Add 65C102 co-processor */

	/* Add Philips VP415 Laserdisc player */

	/* Add Acorn Tracker Ball */

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcmet, bbcm )

	/* Remove all devices not present in this model */

	/* sound hardware */
//  MCFG_DEVICE_REMOVE("mono")
//  MCFG_DEVICE_REMOVE("sn76489")

	/* printer */
//  MCFG_DEVICE_REMOVE("centronics")

	/* cassette */
	MCFG_DEVICE_REMOVE("cassette")

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")

	/* acia */
//  MCFG_DEVICE_REMOVE("acia6850")
	MCFG_DEVICE_REMOVE(RS232_TAG)

	/* devices */
//  MCFG_DEVICE_REMOVE("upd7002")
//  MCFG_DEVICE_REMOVE("via6522_1")

	/* fdc */
//  MCFG_DEVICE_REMOVE("wd177x")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcm512, bbcm )

	/* Add Intel 80186 co-processor */

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcmarm, bbcm )

	/* Add ARM co-processor */

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bbcmc, bbcm )

//  MCFG_DEVICE_REMOVE("rtc")

	/* fdc */
	MCFG_DEVICE_REMOVE("wd177x")

//  MCFG_WD1772_ADD("wd177x", XTAL_16MHz / 2)
	MCFG_WD1770_ADD("wd177x", XTAL_16MHz / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_state, bbc_wd177x_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd177x:0", bbc_floppies, "qd", bbc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd177x:1", bbc_floppies, "qd", bbc_state::floppy_formats)

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cart_ls_m")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_mc", "bbcmc_flop")
MACHINE_CONFIG_END


/* the BBC came with 4 rom sockets on the motherboard as shown in the model A driver */
/* you could get a number of rom upgrade boards that took this up to 16 roms as in the */
/* model B driver */

ROM_START(bbca)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x14000,"option",0) /* ROM */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000 */
	/* rom page 3  0c000 BASIC */
	ROM_DEFAULT_BIOS("os12b2")
	ROM_SYSTEM_BIOS( 0, "os12b2", "OS 1.20 / BASIC2" )
	ROMX_LOAD("os12.rom",   0x10000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(1)) /* os */
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1)) /* rom page 3  0c000 */
	ROM_SYSTEM_BIOS( 1, "os12b1", "OS 1.20 / BASIC1" )
	ROMX_LOAD("os12.rom",   0x10000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(2)) /* os */
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2)) /* rom page 3  0c000 */
	ROM_SYSTEM_BIOS( 2, "os10b2", "OS 1.00 / BASIC2" )
	ROMX_LOAD("os10.rom",   0x10000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(3)) /* os */
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(3)) /* rom page 3  0c000 */
	ROM_SYSTEM_BIOS( 3, "os10b1", "OS 1.00 / BASIC1" )
	ROMX_LOAD("os10.rom",   0x10000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(4)) /* os */
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(4)) /* rom page 3  0c000 */
	ROM_SYSTEM_BIOS( 4, "os01b2", "OS 0.10 / BASIC2" )
	ROMX_LOAD("os01.rom",   0x10000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(5)) /* os */
	ROMX_LOAD("basic2.rom", 0x00000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(5)) /* rom page 0  00000 */
	ROMX_LOAD("basic2.rom", 0x04000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(5)) /* rom page 1  04000 */
	ROMX_LOAD("basic2.rom", 0x08000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(5)) /* rom page 2  08000 */
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(5)) /* rom page 3  0c000 */
	ROM_SYSTEM_BIOS( 5, "os01b1", "OS 0.10 / BASIC1" )
	ROMX_LOAD("os01.rom",   0x10000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(6)) /* os */
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(6)) /* rom page 0  00000 */
	ROMX_LOAD("basic1.rom", 0x04000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(6)) /* rom page 1  04000 */
	ROMX_LOAD("basic1.rom", 0x08000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(6)) /* rom page 2  08000 */
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(6)) /* rom page 3  0c000 */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x10000, 0, 0x4000)
ROM_END



/*  0000- 7fff  ram */
/*  8000- bfff  this area is mapped over with one of the roms from "option" region 0x00000-0x40000 */
/*  c000- ffff  OS rom and memory mapped hardware at fc00-feff, from "option" region 0x40000-0x44000 */


ROM_START(bbcb)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x44000,"option",0) /* ROM */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000 */
	/* rom page 3  0c000 */
	/* rom page 4  10000 */
	/* rom page 5  14000 */
	/* rom page 6  18000 */
	/* rom page 7  1c000 */
	/* rom page 8  20000 */
	/* rom page 9  24000 */
	/* rom page 10 28000 */
	/* rom page 11 2c000 */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3c000 BASIC */
	ROM_DEFAULT_BIOS("os12b2")
	ROM_SYSTEM_BIOS( 0, "os12b2", "OS 1.20 / BASIC2" )
	ROMX_LOAD("os12.rom",   0x40000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(1)) /* os */
	ROMX_LOAD("basic2.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1)) /* rom page 15 3c000 */
	ROM_SYSTEM_BIOS( 1, "os12b1", "OS 1.20 / BASIC1" )
	ROMX_LOAD("os12.rom",   0x40000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(2)) /* os */
	ROMX_LOAD("basic1.rom", 0x3c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2)) /* rom page 15 3c000 */
	ROM_SYSTEM_BIOS( 2, "os10b2", "OS 1.00 / BASIC2" )
	ROMX_LOAD("os10.rom",   0x40000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(3)) /* os */
	ROMX_LOAD("basic2.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(3)) /* rom page 15 3c000 */
	ROM_SYSTEM_BIOS( 3, "os10b1", "OS 1.00 / BASIC1" )
	ROMX_LOAD("os10.rom",   0x40000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(4)) /* os */
	ROMX_LOAD("basic1.rom", 0x3c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(4)) /* rom page 15 3c000 */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x20000,"dfs",0) /* DFS ROMS */
	ROM_LOAD("dfs09.rom",    0x00000, 0x2000, CRC(3ce609cf) SHA1(5cc0f14b8f46855c70eaa653cca4ad079b458732))
	ROM_RELOAD(              0x02000, 0x2000                )
	ROM_LOAD("dnfs.rom",     0x04000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("dfs144.rom",   0x08000, 0x4000, CRC(9fb8d13f) SHA1(387d2468c6e1360f5b531784ce95d5f71a50c2b5))
	ROM_LOAD("zdfs-0.90.rom",0x0C000, 0x2000, CRC(ea579d4d) SHA1(59ad2a8994f4bddad6687891f1a2bc29f2fd32b8))
	ROM_LOAD("ddfs223.rom",  0x10000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))
	ROM_LOAD("ddfs-1.53.rom",0x14000, 0x4000, CRC(e1be4ee4) SHA1(6719dc958f2631e6dc8f045429797b289bfe649a))
	ROM_LOAD("ch103.rom",    0x18000, 0x4000, CRC(98367cf4) SHA1(eca3631aa420691f96b72bfdf2e9c2b613e1bf33))
	/*NONE*/
	ROM_REGION(0x80000, "disks", ROMREGION_ERASEFF) /* Opus Ram Disc Space */

	//ROM_REGION(0x2000, "torch", 0)
	//ROM_LOAD("torchz80_094.bin", 0x0000, 0x2000, CRC(49989bd4) SHA1(62b57c858a3baa4ff943c31f77d331c414772a61))
	//ROM_LOAD("torchz80_102.bin", 0x0000, 0x2000, CRC(2eb40a21) SHA1(e6ee738e5f2f8556002b79d18caa8ef21f14e08d))

	//ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	//ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END

ROM_START(bbcb_de)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x44000,"option",0) /* ROM */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000 */
	/* rom page 3  0c000 */
	/* rom page 4  10000 */
	/* rom page 5  14000 */
	/* rom page 6  18000 */
	/* rom page 7  1c000 */
	/* rom page 8  20000 */
	/* rom page 9  24000 */
	/* rom page 10 28000 */
	/* rom page 11 2c000 */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3c000 BASIC */
	ROM_DEFAULT_BIOS("os12")
	ROM_SYSTEM_BIOS( 0, "os12", "OS 1.20 / BASIC2" )
	ROMX_LOAD("os_de.rom",   0x40000, 0x4000, CRC(b7262caf) SHA1(aadf90338ee9d1c85dfa73beba50e930c2a38f10), ROM_BIOS(1))
	ROMX_LOAD("basic2.rom",  0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1)) /* rom page 15 3c000 */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x20000,"dfs",0) /* DFS ROMS */
	ROM_LOAD("dfs09.rom",    0x00000, 0x2000, CRC(3ce609cf) SHA1(5cc0f14b8f46855c70eaa653cca4ad079b458732))
	ROM_RELOAD(              0x02000, 0x2000                )

	ROM_LOAD("dnfs.rom",     0x04000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("dfs144.rom",   0x08000, 0x4000, CRC(9fb8d13f) SHA1(387d2468c6e1360f5b531784ce95d5f71a50c2b5))
	ROM_LOAD("zdfs-0.90.rom",0x0C000, 0x2000, CRC(ea579d4d) SHA1(59ad2a8994f4bddad6687891f1a2bc29f2fd32b8))
	ROM_LOAD("ddfs223.rom",  0x10000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))
	ROM_LOAD("ddfs-1.53.rom",0x14000, 0x4000, CRC(e1be4ee4) SHA1(6719dc958f2631e6dc8f045429797b289bfe649a))
	ROM_LOAD("ch103.rom",    0x18000, 0x4000, CRC(98367cf4) SHA1(eca3631aa420691f96b72bfdf2e9c2b613e1bf33))
	/*NONE*/
	ROM_REGION(0x80000, "disks", ROMREGION_ERASEFF) /* Opus Ram Disc Space */
ROM_END

ROM_START(bbcb_us)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x44000,"option",0) /* ROM */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000 */
	/* rom page 3  0c000 */
	/* rom page 4  10000 */
	/* rom page 5  14000 */
	/* rom page 6  18000 */
	/* rom page 7  1c000 */
	/* rom page 8  20000 */
	/* rom page 9  24000 */
	/* rom page 10 28000 */
	/* rom page 11 2c000 */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3c000 BASIC */
	ROM_DEFAULT_BIOS("os10b3")
	ROM_SYSTEM_BIOS( 0, "os10b3", "OS A1.0 / BASIC3" )
	ROMX_LOAD("os10_us.rom", 0x40000, 0x4000, CRC(c8e946a9) SHA1(83d91d089dca092d2c8b7c3650ff8143c9069b89), ROM_BIOS(1))
	ROMX_LOAD("basic3.rom",  0x3c000, 0x4000, CRC(161b9539) SHA1(b39014610a968789afd7695aa04d1277d874405c), ROM_BIOS(1)) /* rom page 15 3c000 */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x20000,"dfs",0) /* DFS ROMS */
	ROM_LOAD("dfs09.rom",    0x00000, 0x2000, CRC(3ce609cf) SHA1(5cc0f14b8f46855c70eaa653cca4ad079b458732))
	ROM_RELOAD(              0x02000, 0x2000                )

	ROM_LOAD("dnfs.rom",     0x04000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("dfs144.rom",   0x08000, 0x4000, CRC(9fb8d13f) SHA1(387d2468c6e1360f5b531784ce95d5f71a50c2b5))
	ROM_LOAD("zdfs-0.90.rom",0x0C000, 0x2000, CRC(ea579d4d) SHA1(59ad2a8994f4bddad6687891f1a2bc29f2fd32b8))
	ROM_LOAD("ddfs223.rom",  0x10000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))
	ROM_LOAD("ddfs-1.53.rom",0x14000, 0x4000, CRC(e1be4ee4) SHA1(6719dc958f2631e6dc8f045429797b289bfe649a))
	ROM_LOAD("ch103.rom",    0x18000, 0x4000, CRC(98367cf4) SHA1(eca3631aa420691f96b72bfdf2e9c2b613e1bf33))
	/*NONE*/
	ROM_REGION(0x80000, "disks", ROMREGION_ERASEFF) /* Opus Ram Disc Space */

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END

ROM_START(bbcbp)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("os20")
	ROM_SYSTEM_BIOS( 0, "os20", "OS 2.00" )
	ROMX_LOAD("bpos2.ic71", 0x3c000, 0x4000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2), ROM_BIOS(1)) /* rom page 15 3C000 BASIC */
	ROM_CONTINUE(           0x40000, 0x4000)  /* OS */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000  32K IN PAGE 3 */
	/* rom page 3  0c000  SPARE SOCKET */
	/* rom page 4  10000  32K IN PAGE 5 */
	/* rom page 5  14000  SPARE SOCKET */
	/* rom page 6  18000  32K IN PAGE 7 */
	/* rom page 7  1c000  DDFS */
	/* rom page 8  20000  32K IN PAGE 9 */
	/* rom page 9  24000  SPARE SOCKET */
	/* rom page 10 28000  32K IN PAGE 11 */
	/* rom page 11 2c000  SPARE SOCKET */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000  32K IN PAGE 15 */
	/* rom page 15 3C000  BASIC */
	/* ddfs 2.23 this is acorns 1770 disc controller Double density disc filing system */
	ROM_LOAD("ddfs223.rom", 0x1c000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcbp128)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("os20")
	ROM_SYSTEM_BIOS( 0, "os20", "OS 2.00" )
	ROMX_LOAD("bpos2.ic71", 0x3c000, 0x4000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2), ROM_BIOS(1)) /* rom page 15 3C000 BASIC */
	ROM_CONTINUE(           0x40000, 0x4000)  /* OS */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000  32K IN PAGE 3 */
	/* rom page 3  0c000  SPARE SOCKET */
	/* rom page 4  10000  32K IN PAGE 5 */
	/* rom page 5  14000  SPARE SOCKET */
	/* rom page 6  18000  32K IN PAGE 7 */
	/* rom page 7  1c000  DDFS */
	/* rom page 8  20000  32K IN PAGE 9 */
	/* rom page 9  24000  SPARE SOCKET */
	/* rom page 10 28000  32K IN PAGE 11 */
	/* rom page 11 2c000  SPARE SOCKET */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000  32K IN PAGE 15 */
	/* rom page 15 3C000  BASIC */
	/* ddfs 2.23 this is acorns 1770 disc controller Double density disc filing system */
	ROM_LOAD("ddfs223.rom", 0x1c000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcm)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos350")
	ROM_SYSTEM_BIOS( 0, "mos350", "Enhanced MOS 3.50" )
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0cfad2ce) SHA1(0275719aa7746dd3b627f95ccc4362b564063a5e), ROM_BIOS(2))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   SPARE SOCKET */
	/* 24000 rom 9   DFS + SRAM */
	/* 28000 rom 10  Viewsheet */
	/* 2c000 rom 11  Edit */
	/* 30000 rom 12  BASIC */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  View + MOS code */
	/* 3c000 rom 15  Terminal + Tube host + CFS */
//  ROM_LOAD("anfs424.rom", 0x20000, 0x4000, CRC(1b9f75fd) SHA1(875f71edd48f87c3a55371409d0cc2015d8b5853) ) // TODO where to load this?

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(1))
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(2))
ROM_END


ROM_START(bbcmt)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos350")
	ROM_SYSTEM_BIOS( 0, "mos350", "Enhanced MOS 3.50" )
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0cfad2ce) SHA1(0275719aa7746dd3b627f95ccc4362b564063a5e), ROM_BIOS(2))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   SPARE SOCKET */
	/* 24000 rom 9   DFS + SRAM */
	/* 28000 rom 10  Viewsheet */
	/* 2c000 rom 11  Edit */
	/* 30000 rom 12  BASIC */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  View + MOS code */
	/* 3c000 rom 15  Terminal + Tube host + CFS */
//  ROM_LOAD("anfs424.ic27", 0x20000, 0x4000, CRC(1b9f75fd) SHA1(875f71edd48f87c3a55371409d0cc2015d8b5853) ) // TODO where to load this?

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(1))
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(2))
ROM_END


ROM_START(bbcmaiv)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0cfad2ce) SHA1(0275719aa7746dd3b627f95ccc4362b564063a5e), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   VFS */
	/* 24000 rom 9   DFS + SRAM */
	/* 28000 rom 10  Viewsheet */
	/* 2c000 rom 11  Edit */
	/* 30000 rom 12  BASIC */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  View + MOS code */
	/* 3c000 rom 15  Terminal + Tube host + CFS */
	ROM_LOAD("vfs170.rom", 0x20000, 0x4000, CRC(b124a0bb) SHA1(ba31c757815cf470402d7829a70a0e1d3fb1355b) )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320aiv.cmos", 0x0E, 0x32, BAD_DUMP CRC(b9ae42a1) SHA1(abf3e94b013f24027ca36c96720963c3411e93f8), ROM_BIOS(1))
ROM_END


ROM_START(bbcmet)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos400")
	ROM_SYSTEM_BIOS( 0, "mos400", "Econet MOS 4.00" )
	ROMX_LOAD("mos400.ic24", 0x20000, 0x10000, BAD_DUMP CRC(81729034) SHA1(d4bc2c7f5e66b5298786138f395908e70c772971), ROM_BIOS(1)) /* Merged individual ROM bank dumps */
	ROM_COPY("option", 0x24000, 0x34000, 0xC000) /* Mirror */
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   NO SOCKET */
	/* 24000 rom 9   BASIC */
	/* 28000 rom 10  ANFS */
	/* 2c000 rom 11  MOS code */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  BASIC */
	/* 38000 rom 14  ANFS */
	/* 3c000 rom 15  MOS code */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos400.cmos", 0x0E, 0x32, BAD_DUMP CRC(fff41cc5) SHA1(3607568758f90b3bd6c7dc9533e2aa24f9806ff3), ROM_BIOS(1))
ROM_END


ROM_START(bbcm512)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos350")
	ROM_SYSTEM_BIOS( 0, "mos350", "Enhanced MOS 3.50" )
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0cfad2ce) SHA1(0275719aa7746dd3b627f95ccc4362b564063a5e), ROM_BIOS(2))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   SPARE SOCKET */
	/* 24000 rom 9   DFS + SRAM */
	/* 28000 rom 10  Viewsheet */
	/* 2c000 rom 11  Edit */
	/* 30000 rom 12  BASIC */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  View + MOS code */
	/* 3c000 rom 15  Terminal + Tube host + CFS */
//  ROM_LOAD("anfs424.ic27", 0x20000, 0x4000, CRC(1b9f75fd) SHA1(875f71edd48f87c3a55371409d0cc2015d8b5853) )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(1))
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(2))
ROM_END


ROM_START(bbcmarm)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0cfad2ce) SHA1(0275719aa7746dd3b627f95ccc4362b564063a5e), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xFFFF)
	/* 00000 rom 0   Rear Cartridge bottom 16K */
	/* 04000 rom 1   Rear Cartridge top 16K */
	/* 08000 rom 2   Front Cartridge bottom 16K */
	/* 0c000 rom 3   Front Cartridge top 16K */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   SPARE SOCKET */
	/* 24000 rom 9   DFS + SRAM */
	/* 28000 rom 10  Viewsheet */
	/* 2c000 rom 11  Edit */
	/* 30000 rom 12  BASIC */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  View + MOS code */
	/* 3c000 rom 15  Terminal + Tube host + CFS */
//  ROM_LOAD("anfs424.ic27", 0x20000, 0x4000, CRC(1b9f75fd) SHA1(875f71edd48f87c3a55371409d0cc2015d8b5853) )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320arm.cmos", 0x00, 0x40, CRC(56117257) SHA1(ed98563bef18f9d2a0b2d941cd20823d760fb127), ROM_BIOS(1))
ROM_END


ROM_START(bbcmc)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos510")
	ROM_SYSTEM_BIOS( 0, "mos510", "Enhanced MOS 5.10" )
	ROMX_LOAD("mos510.ic49", 0x30000, 0x10000, BAD_DUMP CRC(9a2a6086) SHA1(094ab37b0b6437c4f1653eaa0602ef102737adb6), ROM_BIOS(1)) /* Merged individual ROM bank dumps */
	ROM_SYSTEM_BIOS( 1, "mos500", "Original MOS 5.00" )
	ROMX_LOAD("mos500.ic49", 0x30000, 0x10000, BAD_DUMP CRC(f6170023) SHA1(140d002d2d9cd34b47197a2ba823505af2a84633), ROM_BIOS(2)) /* Merged individual ROM bank dumps */
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xFFFF)
	/* 00000 rom 0   EXTERNAL */
	/* 04000 rom 1   EXTERNAL */
	/* 08000 rom 2   SPARE SOCKET */
	/* 0c000 rom 3   SPARE SOCKET */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   SPARE SOCKET */
	/* 24000 rom 9   UNUSED */
	/* 28000 rom 10  UNUSED */
	/* 2c000 rom 11  UNUSED */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  BASIC */
	/* 3c000 rom 15  Utils */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

//  ROM_REGION(0x80,"mc146818",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
//  ROM_LOAD("mos500.cmos", 0x00, 0x80, CRC(d8458039) SHA1(72c056d493e74ceca41f48936012b012b496a226))
ROM_END


ROM_START(bbcmc_ar)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos511i")
	ROM_SYSTEM_BIOS( 0, "mos511i", "International MOS 5.11" )
	ROMX_LOAD("mos511.ic49", 0x30000, 0x10000, BAD_DUMP CRC(8708803c) SHA1(d2170c8b9b536f3ad84a4a603a7fe712500cc751), ROM_BIOS(1)) /* Merged individual ROM bank dumps */
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xFFFF)
	/* 00000 rom 0   EXTERNAL */
	/* 04000 rom 1   EXTERNAL */
	/* 08000 rom 2   International */
	/* 0c000 rom 3   SPARE SOCKET */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   Arabian */
	/* 24000 rom 9   UNUSED */
	/* 28000 rom 10  UNUSED */
	/* 2c000 rom 11  UNUSED */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  ADFS */
	/* 38000 rom 14  BASIC */
	/* 3c000 rom 15  Utils */
	ROM_LOAD("international16.rom", 0x8000 , 0x4000, CRC(0ef527b1) SHA1(dc5149ccf588cd591a6ad47727474ef3313272ce) )
	ROM_LOAD("arabian-c22.rom"    , 0x20000, 0x4000, CRC(4f3aadff) SHA1(2bbf61ba68264ce5845aab9c54e750b0efe219c8) )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

//  ROM_REGION(0x80,"mc146818",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
//  ROM_LOAD("mos500.cmos", 0x00, 0x80, CRC(d8458039) SHA1(72c056d493e74ceca41f48936012b012b496a226))
ROM_END


/*     YEAR  NAME      PARENT    COMPAT MACHINE   INPUT CLASS        INIT     COMPANY  FULLNAME */
COMP ( 1981, bbcb,     0,        bbca,  bbcb,     bbcb, bbc_state,   bbc,     "Acorn", "BBC Micro Model B", 0)
COMP ( 1981, bbca,     bbcb,     0,     bbca,     bbca, bbc_state,   bbc,     "Acorn", "BBC Micro Model A", 0)
COMP ( 1981, bbcb_us,  bbcb,     0,     bbcb_us,  bbcb, bbc_state,   bbc,     "Acorn", "Acorn Proton (US)", 0)
COMP ( 1981, bbcb_de,  bbcb,     0,     bbcb,     bbcb, bbc_state,   bbc,     "Acorn", "BBC Micro Model B (German)", 0)
COMP ( 1985, bbcbp,    0,        bbcb,  bbcbp,    bbcb, bbc_state,   bbc,     "Acorn", "BBC Micro Model B+ 64K", 0)
COMP ( 1985, bbcbp128, bbcbp,    0,     bbcbp128, bbcb, bbc_state,   bbc,     "Acorn", "BBC Micro Model B+ 128K", 0)
COMP ( 1986, bbcm,     0,        bbcb,  bbcm,     bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master 128", 0)
COMP ( 1986, bbcmt,    bbcm,     0,     bbcmt,    bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master Turbo", MACHINE_NOT_WORKING)
COMP ( 1986, bbcmaiv,  bbcm,     0,     bbcmaiv,  bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master AIV", MACHINE_NOT_WORKING)
COMP ( 1986, bbcmet,   bbcm,     0,     bbcmet,   bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master ET", 0)
COMP ( 1986, bbcm512,  bbcm,     0,     bbcm512,  bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master 512", MACHINE_NOT_WORKING)
COMP ( 1986, bbcmarm,  bbcm,     0,     bbcmarm,  bbcm, bbc_state,   bbcm,    "Acorn", "ARM Evaluation System", MACHINE_NOT_WORKING)
COMP ( 1986, bbcmc,    0,        bbcm,  bbcmc,    bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master Compact", 0)
COMP ( 1986, bbcmc_ar, bbcmc,    0,     bbcmc,    bbcm, bbc_state,   bbcm,    "Acorn", "BBC Master Compact (Arabic)", 0)
