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

    Econet

    AEH25 - Econet X25 Gateway

    BBC Master

    AMB15 - Master 128
    ADB12 - Master Econet Terminal
    AVC12 - Master AIV
    ARM1  - ARM Evaluation System
    ADB20 - Master Compact

    Acorn Business Computer

    ABC110        - 64K,   10MB HDD, Z80, CP/M 2.2
    ABC210/ACW443 - 4096K, 20MB HDD, 32016, PanOS
    ABC310        - 1024K, 10MB HDD, 80286, DOS 3.1/GEM

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/bbc.h"

/* Components */
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/6522via.h"
#include "bus/centronics/ctronics.h"
#include "bus/econet/econet.h"
#include "sound/tms5220.h"          /* Speech */
#include "video/saa5050.h"          /* Teletext */

/* Devices */
#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"
#include "formats/pc_dsk.h"
#include "imagedev/cassette.h"
#include "formats/uef_cas.h"
#include "formats/csw_cas.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "bbc.lh"
#include "bbcm.lh"

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

void bbc_state::bbca_mem(address_map &map)
{
	map.unmap_value_high();                                                                                           /*  Hardware marked with a # is not present in a Model A        */
	map(0x0000, 0x3fff).bankrw("bank1");                                                                              /*    0000-3fff                 Regular RAM                     */
	map(0x4000, 0x7fff).bankrw("bank3");                                                                              /*    4000-7fff                 Regular RAM mirrored            */
	map(0x8000, 0xbfff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memoryb4_w));                                      /*    8000-bfff                 Paged ROM                       */
	map(0xc000, 0xfbff).bankr("bank7");                                                                               /*    c000-fbff                 OS ROM                          */
	map(0xfc00, 0xfdff).noprw();                                                                                      /*    fc00-fdff                 FRED & JIM Pages                */
																																																										/*    fe00-feff                 SHEILA Address Page             */
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_hd6845, FUNC(hd6845_device::status_r), FUNC(hd6845_device::address_w));     /*    fe00-fe07  6845 CRTC      Video controller                */
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_hd6845, FUNC(hd6845_device::register_r), FUNC(hd6845_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));           /*    fe08-fe0f  6850 ACIA      Serial controller               */
	map(0xfe10, 0xfe17).rw(this, FUNC(bbc_state::bbc_fe_r), FUNC(bbc_state::bbc_SerialULA_w));                        /*    fe10-fe17  Serial ULA     Serial system chip              */
	map(0xfe18, 0xfe1f).noprw();                                                                                      /*    fe18-fe1f  INTOFF/STATID  # ECONET Interrupt Off / ID No. */
	map(0xfe20, 0xfe2f).rw(this, FUNC(bbc_state::bbc_fe_r), FUNC(bbc_state::bbc_videoULA_w));                         /* R: fe20-fe2f  INTON          # ECONET Interrupt On           */
																																																										/* W: fe20-fe2f  Video ULA      Video system chip               */
	map(0xfe30, 0xfe3f).rw(this, FUNC(bbc_state::bbc_fe_r), FUNC(bbc_state::page_selecta_w));                         /* W: fe30-fe3f  74LS161        Paged ROM selector              */
	map(0xfe40, 0xfe5f).rw(m_via6522_0, FUNC(via6522_device::read), FUNC(via6522_device::write));                     /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	map(0xfe60, 0xfe7f).noprw();                                                                                      /*    fe60-fe7f  6522 VIA       # USER VIA                      */
	map(0xfe80, 0xfe9f).noprw();                                                                                      /*    fe80-fe9f  8271/1770 FDC  # Floppy disc controller        */
	map(0xfea0, 0xfebf).r(this, FUNC(bbc_state::bbc_fe_r));                                                           /*    fea0-febf  68B54 ADLC     # ECONET controller             */
	map(0xfec0, 0xfedf).noprw();                                                                                      /*    fec0-fedf  uPD7002        # Analogue to digital converter */
	map(0xfee0, 0xfeff).r(this, FUNC(bbc_state::bbc_fe_r));                                                           /*    fee0-feff  Tube ULA       # Tube system interface         */
	map(0xff00, 0xffff).rom().region("os", 0x3f00);                                                                   /*    ff00-ffff                 OS Rom (continued)              */
}


void bbc_state::bbc_base(address_map &map)
{
	map.unmap_value_high();
	map(0xc000, 0xfbff).bankr("bank7");                                                                               /*    c000-fbff                 OS ROM                          */
	map(0xfc00, 0xfdff).noprw();                                                                                      /*    fc00-fdff                 FRED & JIM Pages                */
	map(0xfe00, 0xfeff).r(this, FUNC(bbc_state::bbc_fe_r));                                                           /*    fe00-feff                 SHEILA Address Page             */
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_hd6845, FUNC(hd6845_device::status_r), FUNC(hd6845_device::address_w));     /*    fe00-fe07  6845 CRTC      Video controller                */
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_hd6845, FUNC(hd6845_device::register_r), FUNC(hd6845_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));           /*    fe08-fe0f  6850 ACIA      Serial controller               */
	map(0xfe10, 0xfe17).rw(this, FUNC(bbc_state::bbc_fe_r), FUNC(bbc_state::bbc_SerialULA_w));                        /*    fe10-fe17  Serial ULA     Serial system chip              */
	map(0xfe18, 0xfe1f).portr("STATID");                                                                              /*    fe18-fe1f  INTOFF/STATID  ECONET Interrupt Off / ID No.   */
	map(0xfe20, 0xfe2f).rw(this, FUNC(bbc_state::bbc_fe_r), FUNC(bbc_state::bbc_videoULA_w));                         /* R: fe20-fe2f  INTON          ECONET Interrupt On             */
																																																										/* W: fe20-fe2f  Video ULA      Video system chip               */
	map(0xfe40, 0xfe5f).rw(m_via6522_0, FUNC(via6522_device::read), FUNC(via6522_device::write));                     /*    fe40-fe5f  6522 VIA       SYSTEM VIA                      */
	map(0xfe60, 0xfe7f).rw(m_via6522_1, FUNC(via6522_device::read), FUNC(via6522_device::write));                     /*    fe60-fe7f  6522 VIA       USER VIA                        */
																																																										/*    fe80-fe9f  FDC            Floppy disc controller          */
	map(0xfea0, 0xfebf).r(this, FUNC(bbc_state::bbc_fe_r));                                                           /*    fea0-febf  68B54 ADLC     ECONET controller               */
	map(0xfec0, 0xfedf).rw(m_upd7002, FUNC(upd7002_device::read), FUNC(upd7002_device::write));                       /*    fec0-fedf  uPD7002        Analogue to digital converter   */
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           /*    fee0-feff  Tube ULA       Tube system interface           */
	map(0xff00, 0xffff).rom().region("os", 0x3f00);                                                                   /*    ff00-ffff                 OS ROM (continued)              */
}


void bbc_state::bbcb_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x3fff).bankrw("bank1");                                                                              /*    0000-3fff                 Regular RAM                     */
	map(0x4000, 0x7fff).bankrw("bank3");                                                                              /*    4000-7fff                 Regular RAM                     */
	map(0x8000, 0xbfff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memoryb4_w));                                      /*    8000-bfff                 Paged ROM/RAM                   */
	map(0xfe30, 0xfe3f).w(this, FUNC(bbc_state::page_selectb_w));                                                     /* W: fe30-fe3f  84LS161        Paged ROM selector              */
	map(0xfe80, 0xfe83).m(m_i8271, FUNC(i8271_device::map));                                                          /*    fe80-fe83  8271 FDC       Floppy disc controller          */
	map(0xfe84, 0xfe9f).rw(m_i8271, FUNC(i8271_device::data_r), FUNC(i8271_device::data_w));                          /*    fe84-fe9f  8271 FDC       Floppy disc controller          */
}


void bbc_state::bbcb_nofdc_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x3fff).bankrw("bank1");                                                                              /*    0000-3fff                 Regular RAM                     */
	map(0x4000, 0x7fff).bankrw("bank3");                                                                              /*    4000-7fff                 Regular RAM                     */
	map(0x8000, 0xbfff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memoryb4_w));                                      /*    8000-bfff                 Paged ROM/RAM                   */
	map(0xfe30, 0xfe3f).w(this, FUNC(bbc_state::page_selectb_w));                                                     /* W: fe30-fe3f  84LS161        Paged ROM selector              */
}


void bbc_state::bbcbp_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              /*    0000-2fff                 Regular RAM                     */
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              /*    3000-7fff                 Video/Shadow RAM                */
	map(0x8000, 0xafff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memorybp4_w));                                     /*    8000-afff                 Paged ROM or 12K of SWRAM       */
	map(0xb000, 0xbfff).bankr("bank6").w(this, FUNC(bbc_state::bbc_memorybp6_w));                                     /*    b000-bfff                 Rest of paged ROM area          */
	map(0xfe30, 0xfe3f).w(this, FUNC(bbc_state::page_selectbp_w));                                                    /* W: fe30-fe3f  84LS161        Paged ROM selector              */
	map(0xfe80, 0xfe83).w(this, FUNC(bbc_state::bbc_wd1770_status_w));                                                /*    fe80-fe83  1770 FDC       Drive control register          */
	map(0xfe84, 0xfe9f).rw(m_wd1770, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                          /*    fe84-fe9f  1770 FDC       Floppy disc controller          */
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           /*    fee0-feff  Tube ULA       Tube system interface           */
}


void bbc_state::bbcbp128_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              /*    0000-2fff                 Regular RAM                     */
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              /*    3000-7fff                 Video/Shadow RAM                */
	map(0x8000, 0xafff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memorybp4_w));                                     /*    8000-afff                 Paged ROM or 12K of SWRAM       */
	map(0xb000, 0xbfff).bankr("bank6").w(this, FUNC(bbc_state::bbc_memorybp6_w));                                     /*    b000-bfff                 Rest of paged ROM area          */
	map(0xfe30, 0xfe3f).w(this, FUNC(bbc_state::page_selectbp128_w));                                                 /* W: fe30-fe3f  84LS161        Paged ROM selector              */
	map(0xfe80, 0xfe83).w(this, FUNC(bbc_state::bbc_wd1770_status_w));                                                /*    fe80-fe83  1770 FDC       Drive control register          */
	map(0xfe84, 0xfe9f).rw(m_wd1770, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                          /*    fe84-fe9f  1770 FDC       Floppy disc controller          */
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           /*    fee0-feff  Tube ULA       Tube system interface           */
}


void bbc_state::reutapm_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              /*    0000-2fff                 Regular RAM                     */
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              /*    3000-7fff                 Video/Shadow RAM                */
	map(0x8000, 0xafff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memorybp4_w));                                     /*    8000-afff                 Paged ROM or 12K of SWRAM       */
	map(0xb000, 0xbfff).bankr("bank6").w(this, FUNC(bbc_state::bbc_memorybp6_w));                                     /*    b000-bfff                 Rest of paged ROM area          */
	map(0xfe30, 0xfe3f).w(this, FUNC(bbc_state::page_selectbp_w));                                                    /* W: fe30-fe3f  84LS161        Paged ROM selector              */
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           /*    fee0-feff  Tube ULA       Tube system interface           */
}


void bbc_state::bbcbp_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(this, FUNC(bbc_state::bbcbp_fetch_r));
}


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
&28-&2F 1770 registers  1770 Disc Controller    1770 Disc Controller     8 ( 4 bytes x  2 )
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


void bbc_state::bbcm_mem(address_map &map)
{
	map(0x0000, 0x2fff).bankrw("bank1");                                           /*    0000-2fff                 Regular RAM                     */
	map(0x3000, 0x7fff).bankrw("bank2");                                           /*    3000-7fff                 Video/Shadow RAM                */
	map(0x8000, 0x8fff).bankr("bank4").w(this, FUNC(bbc_state::bbc_memorybm4_w));  /*    8000-8fff                 Paged ROM/RAM or 4K of RAM ANDY */
	map(0x9000, 0xbfff).bankr("bank5").w(this, FUNC(bbc_state::bbc_memorybm5_w));  /*    9000-bfff                 Rest of paged ROM/RAM area      */
	map(0xc000, 0xdfff).bankr("bank7").w(this, FUNC(bbc_state::bbc_memorybm7_w));  /*    c000-dfff                 OS ROM or 8K of RAM       HAZEL */
	map(0xe000, 0xffff).rom().region("os", 0x2000);                                /*    e000-ffff                 OS ROM                          */
	map(0xfc00, 0xfeff).bankr("bank8").w(this, FUNC(bbc_state::bbcm_w));           /*    processed directly because it can be ROM or hardware      */
}

void bbc_state::bbcm_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(this, FUNC(bbc_state::bbcm_fetch_r));
}


INPUT_CHANGED_MEMBER(bbc_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval)
	{
		if (m_via6522_1) m_via6522_1->reset();
		if (m_adlc) m_adlc->reset();
		if (m_rtc) m_rtc->reset();
		if (m_fdc) m_fdc->reset();
		if (m_i8271) m_i8271->reset();
		if (m_1mhzbus) m_1mhzbus->reset();
		if (m_tube) m_tube->reset();
		if (m_intube) m_intube->reset();
		if (m_extube) m_extube->reset();
	}
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
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_ \xC2\xA3")         PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('_') PORT_CHAR(0xA3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {")                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *")                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }")                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?")                PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |")               PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)            PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)            PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)              PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE")             PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")               PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)           PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("BRK")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK")              PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, trigger_reset, 0)

	/* Keyboard columns 10 -> 12 are reserved for BBC Master */
	PORT_START("COL10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
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
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad DELETE")      PORT_CODE(KEYCODE_BS_PAD)       PORT_CHAR(UCHAR_MAMEKEY(BS_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad *")           PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1")           PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5")           PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	PORT_MODIFY("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad RETURN")      PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad .")           PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,")           PORT_CODE(KEYCODE_COMMA_PAD)    PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3")           PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2")           PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
INPUT_PORTS_END


static INPUT_PORTS_START(torch_keyboard)
	PORT_MODIFY("COL9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MOVE LEFT")          PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOWERCASE")          PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UPPERCASE")          PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE THIS")        PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MOVE PAST")          PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MOVE RIGHT")         PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

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
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad DELETE")      PORT_CODE(KEYCODE_BS_PAD)       PORT_CHAR(UCHAR_MAMEKEY(BS_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad *")           PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1")           PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5")           PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	PORT_MODIFY("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad RETURN")      PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad .")           PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,")           PORT_CODE(KEYCODE_COMMA_PAD)    PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3")           PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2")           PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_dipswitch)
	PORT_MODIFY("COL2")
	PORT_DIPNAME(0x01, 0x01, "Default File System") PORT_DIPLOCATION("KBD:1")
	PORT_DIPSETTING(   0x00, "NFS" )
	PORT_DIPSETTING(   0x01, "DFS" )

	PORT_MODIFY("COL3")
	PORT_DIPNAME(0x01, 0x01, "Not Used") PORT_DIPLOCATION("KBD:2")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL4")
	PORT_DIPNAME(0x01, 0x01, "Disc Timings") PORT_DIPLOCATION("KBD:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL5")
	PORT_DIPNAME(0x01, 0x01, "Disc Timings") PORT_DIPLOCATION("KBD:4")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))

	PORT_MODIFY("COL6")
	PORT_DIPNAME(0x01, 0x01, "Boot") PORT_DIPLOCATION("KBD:5")
	PORT_DIPSETTING(   0x00, "BREAK" )
	PORT_DIPSETTING(   0x01, "SHIFT-BREAK" )

	PORT_MODIFY("COL7")
	PORT_DIPNAME(0x01, 0x01, "Screen Mode") PORT_DIPLOCATION("KBD:6")
	PORT_DIPSETTING(   0x00, "0" )
	PORT_DIPSETTING(   0x01, "4" )

	PORT_MODIFY("COL8")
	PORT_DIPNAME(0x01, 0x01, "Screen Mode") PORT_DIPLOCATION("KBD:7")
	PORT_DIPSETTING(   0x00, "0" )
	PORT_DIPSETTING(   0x01, "2" )

	PORT_MODIFY("COL9")
	PORT_DIPNAME(0x01, 0x01, "Screen Mode") PORT_DIPLOCATION("KBD:8")
	PORT_DIPSETTING(   0x00, "0" )
	PORT_DIPSETTING(   0x01, "1" )
INPUT_PORTS_END


static INPUT_PORTS_START(bbcb_links)
	PORT_START("STATID")
	PORT_DIPNAME(0xff, 0xfe, "Econet ID") PORT_DIPLOCATION("S11:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0x00,   "0" )    PORT_DIPSETTING(   0x01,   "1" )    PORT_DIPSETTING(   0x02,   "2" )    PORT_DIPSETTING(   0x03,   "3" )    PORT_DIPSETTING(   0x04,   "4" )
	PORT_DIPSETTING(   0x05,   "5" )    PORT_DIPSETTING(   0x06,   "6" )    PORT_DIPSETTING(   0x07,   "7" )    PORT_DIPSETTING(   0x08,   "8" )    PORT_DIPSETTING(   0x09,   "9" )
	PORT_DIPSETTING(   0x0a,  "10" )    PORT_DIPSETTING(   0x0b,  "11" )    PORT_DIPSETTING(   0x0c,  "12" )    PORT_DIPSETTING(   0x0d,  "13" )    PORT_DIPSETTING(   0x0e,  "14" )
	PORT_DIPSETTING(   0x0f,  "15" )    PORT_DIPSETTING(   0x10,  "16" )    PORT_DIPSETTING(   0x11,  "17" )    PORT_DIPSETTING(   0x12,  "18" )    PORT_DIPSETTING(   0x13,  "19" )
	PORT_DIPSETTING(   0x14,  "20" )    PORT_DIPSETTING(   0x15,  "21" )    PORT_DIPSETTING(   0x16,  "22" )    PORT_DIPSETTING(   0x17,  "23" )    PORT_DIPSETTING(   0x18,  "24" )
	PORT_DIPSETTING(   0x19,  "25" )    PORT_DIPSETTING(   0x1a,  "26" )    PORT_DIPSETTING(   0x1b,  "27" )    PORT_DIPSETTING(   0x1c,  "28" )    PORT_DIPSETTING(   0x1d,  "29" )
	PORT_DIPSETTING(   0x1e,  "30" )    PORT_DIPSETTING(   0x1f,  "31" )    PORT_DIPSETTING(   0x20,  "32" )    PORT_DIPSETTING(   0x21,  "33" )    PORT_DIPSETTING(   0x22,  "34" )
	PORT_DIPSETTING(   0x23,  "35" )    PORT_DIPSETTING(   0x24,  "36" )    PORT_DIPSETTING(   0x25,  "37" )    PORT_DIPSETTING(   0x26,  "38" )    PORT_DIPSETTING(   0x27,  "39" )
	PORT_DIPSETTING(   0x28,  "40" )    PORT_DIPSETTING(   0x29,  "41" )    PORT_DIPSETTING(   0x2a,  "42" )    PORT_DIPSETTING(   0x2b,  "43" )    PORT_DIPSETTING(   0x2c,  "44" )
	PORT_DIPSETTING(   0x2d,  "45" )    PORT_DIPSETTING(   0x2e,  "46" )    PORT_DIPSETTING(   0x2f,  "47" )    PORT_DIPSETTING(   0x30,  "48" )    PORT_DIPSETTING(   0x31,  "49" )
	PORT_DIPSETTING(   0x32,  "50" )    PORT_DIPSETTING(   0x33,  "51" )    PORT_DIPSETTING(   0x34,  "52" )    PORT_DIPSETTING(   0x35,  "53" )    PORT_DIPSETTING(   0x36,  "54" )
	PORT_DIPSETTING(   0x37,  "15" )    PORT_DIPSETTING(   0x38,  "56" )    PORT_DIPSETTING(   0x39,  "57" )    PORT_DIPSETTING(   0x3a,  "58" )    PORT_DIPSETTING(   0x3b,  "59" )
	PORT_DIPSETTING(   0x3c,  "60" )    PORT_DIPSETTING(   0x3d,  "61" )    PORT_DIPSETTING(   0x3e,  "62" )    PORT_DIPSETTING(   0x3f,  "63" )    PORT_DIPSETTING(   0x40,  "64" )
	PORT_DIPSETTING(   0x41,  "65" )    PORT_DIPSETTING(   0x42,  "66" )    PORT_DIPSETTING(   0x43,  "67" )    PORT_DIPSETTING(   0x44,  "68" )    PORT_DIPSETTING(   0x45,  "69" )
	PORT_DIPSETTING(   0x46,  "70" )    PORT_DIPSETTING(   0x47,  "71" )    PORT_DIPSETTING(   0x48,  "72" )    PORT_DIPSETTING(   0x49,  "73" )    PORT_DIPSETTING(   0x4a,  "74" )
	PORT_DIPSETTING(   0x4b,  "75" )    PORT_DIPSETTING(   0x4c,  "76" )    PORT_DIPSETTING(   0x4d,  "77" )    PORT_DIPSETTING(   0x4e,  "78" )    PORT_DIPSETTING(   0x4f,  "79" )
	PORT_DIPSETTING(   0x50,  "80" )    PORT_DIPSETTING(   0x51,  "81" )    PORT_DIPSETTING(   0x52,  "82" )    PORT_DIPSETTING(   0x53,  "83" )    PORT_DIPSETTING(   0x54,  "84" )
	PORT_DIPSETTING(   0x55,  "85" )    PORT_DIPSETTING(   0x56,  "86" )    PORT_DIPSETTING(   0x57,  "87" )    PORT_DIPSETTING(   0x58,  "88" )    PORT_DIPSETTING(   0x59,  "89" )
	PORT_DIPSETTING(   0x5a,  "90" )    PORT_DIPSETTING(   0x5b,  "91" )    PORT_DIPSETTING(   0x5c,  "92" )    PORT_DIPSETTING(   0x5d,  "93" )    PORT_DIPSETTING(   0x5e,  "94" )
	PORT_DIPSETTING(   0x5f,  "95" )    PORT_DIPSETTING(   0x60,  "96" )    PORT_DIPSETTING(   0x61,  "97" )    PORT_DIPSETTING(   0x62,  "98" )    PORT_DIPSETTING(   0x63,  "99" )
	PORT_DIPSETTING(   0x64, "100" )    PORT_DIPSETTING(   0x65, "101" )    PORT_DIPSETTING(   0x66, "102" )    PORT_DIPSETTING(   0x67, "103" )    PORT_DIPSETTING(   0x68, "104" )
	PORT_DIPSETTING(   0x69, "105" )    PORT_DIPSETTING(   0x6a, "106" )    PORT_DIPSETTING(   0x6b, "107" )    PORT_DIPSETTING(   0x6c, "108" )    PORT_DIPSETTING(   0x6d, "109" )
	PORT_DIPSETTING(   0x6e, "110" )    PORT_DIPSETTING(   0x6f, "111" )    PORT_DIPSETTING(   0x70, "112" )    PORT_DIPSETTING(   0x71, "113" )    PORT_DIPSETTING(   0x72, "114" )
	PORT_DIPSETTING(   0x73, "115" )    PORT_DIPSETTING(   0x74, "116" )    PORT_DIPSETTING(   0x75, "117" )    PORT_DIPSETTING(   0x76, "118" )    PORT_DIPSETTING(   0x77, "119" )
	PORT_DIPSETTING(   0x78, "120" )    PORT_DIPSETTING(   0x79, "121" )    PORT_DIPSETTING(   0x7a, "122" )    PORT_DIPSETTING(   0x7b, "123" )    PORT_DIPSETTING(   0x7c, "124" )
	PORT_DIPSETTING(   0x7d, "125" )    PORT_DIPSETTING(   0x7e, "126" )    PORT_DIPSETTING(   0x7f, "127" )    PORT_DIPSETTING(   0x80, "128" )    PORT_DIPSETTING(   0x81, "129" )
	PORT_DIPSETTING(   0x82, "130" )    PORT_DIPSETTING(   0x83, "131" )    PORT_DIPSETTING(   0x84, "132" )    PORT_DIPSETTING(   0x85, "133" )    PORT_DIPSETTING(   0x86, "134" )
	PORT_DIPSETTING(   0x87, "135" )    PORT_DIPSETTING(   0x88, "136" )    PORT_DIPSETTING(   0x89, "137" )    PORT_DIPSETTING(   0x8a, "138" )    PORT_DIPSETTING(   0x8b, "139" )
	PORT_DIPSETTING(   0x8c, "140" )    PORT_DIPSETTING(   0x8d, "141" )    PORT_DIPSETTING(   0x8e, "142" )    PORT_DIPSETTING(   0x8f, "143" )    PORT_DIPSETTING(   0x90, "144" )
	PORT_DIPSETTING(   0x91, "145" )    PORT_DIPSETTING(   0x92, "146" )    PORT_DIPSETTING(   0x93, "147" )    PORT_DIPSETTING(   0x94, "148" )    PORT_DIPSETTING(   0x95, "149" )
	PORT_DIPSETTING(   0x96, "150" )    PORT_DIPSETTING(   0x97, "151" )    PORT_DIPSETTING(   0x98, "152" )    PORT_DIPSETTING(   0x99, "153" )    PORT_DIPSETTING(   0x9a, "154" )
	PORT_DIPSETTING(   0x9b, "155" )    PORT_DIPSETTING(   0x9c, "156" )    PORT_DIPSETTING(   0x9d, "157" )    PORT_DIPSETTING(   0x9e, "158" )    PORT_DIPSETTING(   0x9f, "159" )
	PORT_DIPSETTING(   0xa0, "160" )    PORT_DIPSETTING(   0xa1, "161" )    PORT_DIPSETTING(   0xa2, "162" )    PORT_DIPSETTING(   0xa3, "163" )    PORT_DIPSETTING(   0xa4, "164" )
	PORT_DIPSETTING(   0xa5, "165" )    PORT_DIPSETTING(   0xa6, "166" )    PORT_DIPSETTING(   0xa7, "167" )    PORT_DIPSETTING(   0xa8, "168" )    PORT_DIPSETTING(   0xa9, "169" )
	PORT_DIPSETTING(   0xaa, "170" )    PORT_DIPSETTING(   0xab, "171" )    PORT_DIPSETTING(   0xac, "172" )    PORT_DIPSETTING(   0xad, "173" )    PORT_DIPSETTING(   0xae, "174" )
	PORT_DIPSETTING(   0xaf, "175" )    PORT_DIPSETTING(   0xb0, "176" )    PORT_DIPSETTING(   0xb1, "177" )    PORT_DIPSETTING(   0xb2, "178" )    PORT_DIPSETTING(   0xb3, "179" )
	PORT_DIPSETTING(   0xb4, "180" )    PORT_DIPSETTING(   0xb5, "181" )    PORT_DIPSETTING(   0xb6, "182" )    PORT_DIPSETTING(   0xb7, "183" )    PORT_DIPSETTING(   0xb8, "184" )
	PORT_DIPSETTING(   0xb9, "185" )    PORT_DIPSETTING(   0xba, "186" )    PORT_DIPSETTING(   0xbb, "187" )    PORT_DIPSETTING(   0xbc, "188" )    PORT_DIPSETTING(   0xbd, "189" )
	PORT_DIPSETTING(   0xbe, "190" )    PORT_DIPSETTING(   0xbf, "191" )    PORT_DIPSETTING(   0xc0, "192" )    PORT_DIPSETTING(   0xc1, "193" )    PORT_DIPSETTING(   0xc2, "194" )
	PORT_DIPSETTING(   0xc3, "195" )    PORT_DIPSETTING(   0xc4, "196" )    PORT_DIPSETTING(   0xc5, "197" )    PORT_DIPSETTING(   0xc6, "198" )    PORT_DIPSETTING(   0xc7, "199" )
	PORT_DIPSETTING(   0xc8, "200" )    PORT_DIPSETTING(   0xc9, "201" )    PORT_DIPSETTING(   0xca, "202" )    PORT_DIPSETTING(   0xcb, "203" )    PORT_DIPSETTING(   0xcc, "204" )
	PORT_DIPSETTING(   0xcd, "205" )    PORT_DIPSETTING(   0xce, "206" )    PORT_DIPSETTING(   0xcf, "207" )    PORT_DIPSETTING(   0xd0, "208" )    PORT_DIPSETTING(   0xd1, "209" )
	PORT_DIPSETTING(   0xd2, "210" )    PORT_DIPSETTING(   0xd3, "211" )    PORT_DIPSETTING(   0xd4, "212" )    PORT_DIPSETTING(   0xd5, "213" )    PORT_DIPSETTING(   0xd6, "214" )
	PORT_DIPSETTING(   0xd7, "215" )    PORT_DIPSETTING(   0xd8, "216" )    PORT_DIPSETTING(   0xd9, "217" )    PORT_DIPSETTING(   0xda, "218" )    PORT_DIPSETTING(   0xdb, "219" )
	PORT_DIPSETTING(   0xdc, "220" )    PORT_DIPSETTING(   0xdd, "221" )    PORT_DIPSETTING(   0xde, "222" )    PORT_DIPSETTING(   0xdf, "223" )    PORT_DIPSETTING(   0xe0, "224" )
	PORT_DIPSETTING(   0xe1, "225" )    PORT_DIPSETTING(   0xe2, "226" )    PORT_DIPSETTING(   0xe3, "227" )    PORT_DIPSETTING(   0xe4, "228" )    PORT_DIPSETTING(   0xe5, "229" )
	PORT_DIPSETTING(   0xe6, "230" )    PORT_DIPSETTING(   0xe7, "231" )    PORT_DIPSETTING(   0xe8, "232" )    PORT_DIPSETTING(   0xe9, "233" )    PORT_DIPSETTING(   0xea, "234" )
	PORT_DIPSETTING(   0xeb, "235" )    PORT_DIPSETTING(   0xec, "236" )    PORT_DIPSETTING(   0xed, "237" )    PORT_DIPSETTING(   0xee, "238" )    PORT_DIPSETTING(   0xef, "239" )
	PORT_DIPSETTING(   0xf0, "240" )    PORT_DIPSETTING(   0xf1, "241" )    PORT_DIPSETTING(   0xf2, "242" )    PORT_DIPSETTING(   0xf3, "243" )    PORT_DIPSETTING(   0xf4, "244" )
	PORT_DIPSETTING(   0xf5, "245" )    PORT_DIPSETTING(   0xf6, "246" )    PORT_DIPSETTING(   0xf7, "247" )    PORT_DIPSETTING(   0xf8, "248" )    PORT_DIPSETTING(   0xf9, "249" )
	PORT_DIPSETTING(   0xfa, "250" )    PORT_DIPSETTING(   0xfb, "251" )    PORT_DIPSETTING(   0xfc, "252" )    PORT_DIPSETTING(   0xfd, "253" )    PORT_DIPSETTING(   0xfe, "254" )
	PORT_DIPSETTING(   0xff, "255" )
INPUT_PORTS_END


static INPUT_PORTS_START(bbcbp_links)
	PORT_START("STATID")
	PORT_DIPNAME(0xff, 0xfe, "Econet ID") PORT_DIPLOCATION("S23:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0x00,   "0" )    PORT_DIPSETTING(   0x01,   "1" )    PORT_DIPSETTING(   0x02,   "2" )    PORT_DIPSETTING(   0x03,   "3" )    PORT_DIPSETTING(   0x04,   "4" )
	PORT_DIPSETTING(   0x05,   "5" )    PORT_DIPSETTING(   0x06,   "6" )    PORT_DIPSETTING(   0x07,   "7" )    PORT_DIPSETTING(   0x08,   "8" )    PORT_DIPSETTING(   0x09,   "9" )
	PORT_DIPSETTING(   0x0a,  "10" )    PORT_DIPSETTING(   0x0b,  "11" )    PORT_DIPSETTING(   0x0c,  "12" )    PORT_DIPSETTING(   0x0d,  "13" )    PORT_DIPSETTING(   0x0e,  "14" )
	PORT_DIPSETTING(   0x0f,  "15" )    PORT_DIPSETTING(   0x10,  "16" )    PORT_DIPSETTING(   0x11,  "17" )    PORT_DIPSETTING(   0x12,  "18" )    PORT_DIPSETTING(   0x13,  "19" )
	PORT_DIPSETTING(   0x14,  "20" )    PORT_DIPSETTING(   0x15,  "21" )    PORT_DIPSETTING(   0x16,  "22" )    PORT_DIPSETTING(   0x17,  "23" )    PORT_DIPSETTING(   0x18,  "24" )
	PORT_DIPSETTING(   0x19,  "25" )    PORT_DIPSETTING(   0x1a,  "26" )    PORT_DIPSETTING(   0x1b,  "27" )    PORT_DIPSETTING(   0x1c,  "28" )    PORT_DIPSETTING(   0x1d,  "29" )
	PORT_DIPSETTING(   0x1e,  "30" )    PORT_DIPSETTING(   0x1f,  "31" )    PORT_DIPSETTING(   0x20,  "32" )    PORT_DIPSETTING(   0x21,  "33" )    PORT_DIPSETTING(   0x22,  "34" )
	PORT_DIPSETTING(   0x23,  "35" )    PORT_DIPSETTING(   0x24,  "36" )    PORT_DIPSETTING(   0x25,  "37" )    PORT_DIPSETTING(   0x26,  "38" )    PORT_DIPSETTING(   0x27,  "39" )
	PORT_DIPSETTING(   0x28,  "40" )    PORT_DIPSETTING(   0x29,  "41" )    PORT_DIPSETTING(   0x2a,  "42" )    PORT_DIPSETTING(   0x2b,  "43" )    PORT_DIPSETTING(   0x2c,  "44" )
	PORT_DIPSETTING(   0x2d,  "45" )    PORT_DIPSETTING(   0x2e,  "46" )    PORT_DIPSETTING(   0x2f,  "47" )    PORT_DIPSETTING(   0x30,  "48" )    PORT_DIPSETTING(   0x31,  "49" )
	PORT_DIPSETTING(   0x32,  "50" )    PORT_DIPSETTING(   0x33,  "51" )    PORT_DIPSETTING(   0x34,  "52" )    PORT_DIPSETTING(   0x35,  "53" )    PORT_DIPSETTING(   0x36,  "54" )
	PORT_DIPSETTING(   0x37,  "15" )    PORT_DIPSETTING(   0x38,  "56" )    PORT_DIPSETTING(   0x39,  "57" )    PORT_DIPSETTING(   0x3a,  "58" )    PORT_DIPSETTING(   0x3b,  "59" )
	PORT_DIPSETTING(   0x3c,  "60" )    PORT_DIPSETTING(   0x3d,  "61" )    PORT_DIPSETTING(   0x3e,  "62" )    PORT_DIPSETTING(   0x3f,  "63" )    PORT_DIPSETTING(   0x40,  "64" )
	PORT_DIPSETTING(   0x41,  "65" )    PORT_DIPSETTING(   0x42,  "66" )    PORT_DIPSETTING(   0x43,  "67" )    PORT_DIPSETTING(   0x44,  "68" )    PORT_DIPSETTING(   0x45,  "69" )
	PORT_DIPSETTING(   0x46,  "70" )    PORT_DIPSETTING(   0x47,  "71" )    PORT_DIPSETTING(   0x48,  "72" )    PORT_DIPSETTING(   0x49,  "73" )    PORT_DIPSETTING(   0x4a,  "74" )
	PORT_DIPSETTING(   0x4b,  "75" )    PORT_DIPSETTING(   0x4c,  "76" )    PORT_DIPSETTING(   0x4d,  "77" )    PORT_DIPSETTING(   0x4e,  "78" )    PORT_DIPSETTING(   0x4f,  "79" )
	PORT_DIPSETTING(   0x50,  "80" )    PORT_DIPSETTING(   0x51,  "81" )    PORT_DIPSETTING(   0x52,  "82" )    PORT_DIPSETTING(   0x53,  "83" )    PORT_DIPSETTING(   0x54,  "84" )
	PORT_DIPSETTING(   0x55,  "85" )    PORT_DIPSETTING(   0x56,  "86" )    PORT_DIPSETTING(   0x57,  "87" )    PORT_DIPSETTING(   0x58,  "88" )    PORT_DIPSETTING(   0x59,  "89" )
	PORT_DIPSETTING(   0x5a,  "90" )    PORT_DIPSETTING(   0x5b,  "91" )    PORT_DIPSETTING(   0x5c,  "92" )    PORT_DIPSETTING(   0x5d,  "93" )    PORT_DIPSETTING(   0x5e,  "94" )
	PORT_DIPSETTING(   0x5f,  "95" )    PORT_DIPSETTING(   0x60,  "96" )    PORT_DIPSETTING(   0x61,  "97" )    PORT_DIPSETTING(   0x62,  "98" )    PORT_DIPSETTING(   0x63,  "99" )
	PORT_DIPSETTING(   0x64, "100" )    PORT_DIPSETTING(   0x65, "101" )    PORT_DIPSETTING(   0x66, "102" )    PORT_DIPSETTING(   0x67, "103" )    PORT_DIPSETTING(   0x68, "104" )
	PORT_DIPSETTING(   0x69, "105" )    PORT_DIPSETTING(   0x6a, "106" )    PORT_DIPSETTING(   0x6b, "107" )    PORT_DIPSETTING(   0x6c, "108" )    PORT_DIPSETTING(   0x6d, "109" )
	PORT_DIPSETTING(   0x6e, "110" )    PORT_DIPSETTING(   0x6f, "111" )    PORT_DIPSETTING(   0x70, "112" )    PORT_DIPSETTING(   0x71, "113" )    PORT_DIPSETTING(   0x72, "114" )
	PORT_DIPSETTING(   0x73, "115" )    PORT_DIPSETTING(   0x74, "116" )    PORT_DIPSETTING(   0x75, "117" )    PORT_DIPSETTING(   0x76, "118" )    PORT_DIPSETTING(   0x77, "119" )
	PORT_DIPSETTING(   0x78, "120" )    PORT_DIPSETTING(   0x79, "121" )    PORT_DIPSETTING(   0x7a, "122" )    PORT_DIPSETTING(   0x7b, "123" )    PORT_DIPSETTING(   0x7c, "124" )
	PORT_DIPSETTING(   0x7d, "125" )    PORT_DIPSETTING(   0x7e, "126" )    PORT_DIPSETTING(   0x7f, "127" )    PORT_DIPSETTING(   0x80, "128" )    PORT_DIPSETTING(   0x81, "129" )
	PORT_DIPSETTING(   0x82, "130" )    PORT_DIPSETTING(   0x83, "131" )    PORT_DIPSETTING(   0x84, "132" )    PORT_DIPSETTING(   0x85, "133" )    PORT_DIPSETTING(   0x86, "134" )
	PORT_DIPSETTING(   0x87, "135" )    PORT_DIPSETTING(   0x88, "136" )    PORT_DIPSETTING(   0x89, "137" )    PORT_DIPSETTING(   0x8a, "138" )    PORT_DIPSETTING(   0x8b, "139" )
	PORT_DIPSETTING(   0x8c, "140" )    PORT_DIPSETTING(   0x8d, "141" )    PORT_DIPSETTING(   0x8e, "142" )    PORT_DIPSETTING(   0x8f, "143" )    PORT_DIPSETTING(   0x90, "144" )
	PORT_DIPSETTING(   0x91, "145" )    PORT_DIPSETTING(   0x92, "146" )    PORT_DIPSETTING(   0x93, "147" )    PORT_DIPSETTING(   0x94, "148" )    PORT_DIPSETTING(   0x95, "149" )
	PORT_DIPSETTING(   0x96, "150" )    PORT_DIPSETTING(   0x97, "151" )    PORT_DIPSETTING(   0x98, "152" )    PORT_DIPSETTING(   0x99, "153" )    PORT_DIPSETTING(   0x9a, "154" )
	PORT_DIPSETTING(   0x9b, "155" )    PORT_DIPSETTING(   0x9c, "156" )    PORT_DIPSETTING(   0x9d, "157" )    PORT_DIPSETTING(   0x9e, "158" )    PORT_DIPSETTING(   0x9f, "159" )
	PORT_DIPSETTING(   0xa0, "160" )    PORT_DIPSETTING(   0xa1, "161" )    PORT_DIPSETTING(   0xa2, "162" )    PORT_DIPSETTING(   0xa3, "163" )    PORT_DIPSETTING(   0xa4, "164" )
	PORT_DIPSETTING(   0xa5, "165" )    PORT_DIPSETTING(   0xa6, "166" )    PORT_DIPSETTING(   0xa7, "167" )    PORT_DIPSETTING(   0xa8, "168" )    PORT_DIPSETTING(   0xa9, "169" )
	PORT_DIPSETTING(   0xaa, "170" )    PORT_DIPSETTING(   0xab, "171" )    PORT_DIPSETTING(   0xac, "172" )    PORT_DIPSETTING(   0xad, "173" )    PORT_DIPSETTING(   0xae, "174" )
	PORT_DIPSETTING(   0xaf, "175" )    PORT_DIPSETTING(   0xb0, "176" )    PORT_DIPSETTING(   0xb1, "177" )    PORT_DIPSETTING(   0xb2, "178" )    PORT_DIPSETTING(   0xb3, "179" )
	PORT_DIPSETTING(   0xb4, "180" )    PORT_DIPSETTING(   0xb5, "181" )    PORT_DIPSETTING(   0xb6, "182" )    PORT_DIPSETTING(   0xb7, "183" )    PORT_DIPSETTING(   0xb8, "184" )
	PORT_DIPSETTING(   0xb9, "185" )    PORT_DIPSETTING(   0xba, "186" )    PORT_DIPSETTING(   0xbb, "187" )    PORT_DIPSETTING(   0xbc, "188" )    PORT_DIPSETTING(   0xbd, "189" )
	PORT_DIPSETTING(   0xbe, "190" )    PORT_DIPSETTING(   0xbf, "191" )    PORT_DIPSETTING(   0xc0, "192" )    PORT_DIPSETTING(   0xc1, "193" )    PORT_DIPSETTING(   0xc2, "194" )
	PORT_DIPSETTING(   0xc3, "195" )    PORT_DIPSETTING(   0xc4, "196" )    PORT_DIPSETTING(   0xc5, "197" )    PORT_DIPSETTING(   0xc6, "198" )    PORT_DIPSETTING(   0xc7, "199" )
	PORT_DIPSETTING(   0xc8, "200" )    PORT_DIPSETTING(   0xc9, "201" )    PORT_DIPSETTING(   0xca, "202" )    PORT_DIPSETTING(   0xcb, "203" )    PORT_DIPSETTING(   0xcc, "204" )
	PORT_DIPSETTING(   0xcd, "205" )    PORT_DIPSETTING(   0xce, "206" )    PORT_DIPSETTING(   0xcf, "207" )    PORT_DIPSETTING(   0xd0, "208" )    PORT_DIPSETTING(   0xd1, "209" )
	PORT_DIPSETTING(   0xd2, "210" )    PORT_DIPSETTING(   0xd3, "211" )    PORT_DIPSETTING(   0xd4, "212" )    PORT_DIPSETTING(   0xd5, "213" )    PORT_DIPSETTING(   0xd6, "214" )
	PORT_DIPSETTING(   0xd7, "215" )    PORT_DIPSETTING(   0xd8, "216" )    PORT_DIPSETTING(   0xd9, "217" )    PORT_DIPSETTING(   0xda, "218" )    PORT_DIPSETTING(   0xdb, "219" )
	PORT_DIPSETTING(   0xdc, "220" )    PORT_DIPSETTING(   0xdd, "221" )    PORT_DIPSETTING(   0xde, "222" )    PORT_DIPSETTING(   0xdf, "223" )    PORT_DIPSETTING(   0xe0, "224" )
	PORT_DIPSETTING(   0xe1, "225" )    PORT_DIPSETTING(   0xe2, "226" )    PORT_DIPSETTING(   0xe3, "227" )    PORT_DIPSETTING(   0xe4, "228" )    PORT_DIPSETTING(   0xe5, "229" )
	PORT_DIPSETTING(   0xe6, "230" )    PORT_DIPSETTING(   0xe7, "231" )    PORT_DIPSETTING(   0xe8, "232" )    PORT_DIPSETTING(   0xe9, "233" )    PORT_DIPSETTING(   0xea, "234" )
	PORT_DIPSETTING(   0xeb, "235" )    PORT_DIPSETTING(   0xec, "236" )    PORT_DIPSETTING(   0xed, "237" )    PORT_DIPSETTING(   0xee, "238" )    PORT_DIPSETTING(   0xef, "239" )
	PORT_DIPSETTING(   0xf0, "240" )    PORT_DIPSETTING(   0xf1, "241" )    PORT_DIPSETTING(   0xf2, "242" )    PORT_DIPSETTING(   0xf3, "243" )    PORT_DIPSETTING(   0xf4, "244" )
	PORT_DIPSETTING(   0xf5, "245" )    PORT_DIPSETTING(   0xf6, "246" )    PORT_DIPSETTING(   0xf7, "247" )    PORT_DIPSETTING(   0xf8, "248" )    PORT_DIPSETTING(   0xf9, "249" )
	PORT_DIPSETTING(   0xfa, "250" )    PORT_DIPSETTING(   0xfb, "251" )    PORT_DIPSETTING(   0xfc, "252" )    PORT_DIPSETTING(   0xfd, "253" )    PORT_DIPSETTING(   0xfe, "254" )
	PORT_DIPSETTING(   0xff, "255" )
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(bbc_state::monitor_changed)
{
	m_monitortype = m_bbcconfig.read_safe(0) & 0x03;
}


static INPUT_PORTS_START(bbc_config)
	PORT_START("BBCCONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Monitor") PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, monitor_changed, 0)
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x01, "B&W")
	PORT_CONFSETTING(    0x02, "Green")
	PORT_CONFSETTING(    0x03, "Amber")
INPUT_PORTS_END


static INPUT_PORTS_START(bbcb_config)
	PORT_START("BBCCONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, monitor_changed, 0)
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x01, "B&W")
	PORT_CONFSETTING(    0x02, "Green")
	PORT_CONFSETTING(    0x03, "Amber")
	PORT_CONFNAME( 0x0c, 0x00, "Sideways RAM Board")
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	//PORT_CONFSETTING(    0x04, "Solidisk 128K (fe62)" )
	PORT_CONFSETTING(    0x08, "Acorn 64K (fe30)" )
	PORT_CONFSETTING(    0x0c, "Acorn 128K (fe30)" )
INPUT_PORTS_END

static INPUT_PORTS_START(bbca)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcb)
	PORT_INCLUDE(bbcb_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcb_links)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcbp)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcbp_links)
INPUT_PORTS_END

static INPUT_PORTS_START(torch)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(torch_keyboard)
	PORT_INCLUDE(bbcb_links)
INPUT_PORTS_END

static INPUT_PORTS_START(abc)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_keypad)
	PORT_INCLUDE(bbcbp_links)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcm)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_keypad)
INPUT_PORTS_END

static INPUT_PORTS_START(ltmpbp)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcbp_links)
INPUT_PORTS_END

static INPUT_PORTS_START(ltmpm)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_keypad)
INPUT_PORTS_END


FLOPPY_FORMATS_MEMBER( bbc_state::floppy_formats_bbc )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT,
	FLOPPY_ACORN_DOS_FORMAT,
	FLOPPY_OPUS_DDOS_FORMAT,
	FLOPPY_OPUS_DDCPM_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

static void bbc_floppies_35(device_slot_interface &device)
{
	device.option_add("35dd",   FLOPPY_35_DD);
}


static const char *const bbc_sample_names[] =
{
	"*bbc",
	"motoroff",
	"motoron",
	nullptr
};


WRITE_LINE_MEMBER(bbc_state::adlc_irq_w)
{
	m_adlc_irq = state;

	bbc_update_nmi();
}


WRITE_LINE_MEMBER(bbc_state::econet_clk_w)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
}

// 4 x EPROM sockets (16K) in BBC-A, these should grow to 16 for BBC-B and later...
MACHINE_CONFIG_START(bbc_state::bbc_eprom_sockets)
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


/***************************************************************************

    BBC Micro

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::bbca)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6502, 16_MHz_XTAL/8)         /* 2.00 MHz */
	MCFG_DEVICE_PROGRAM_MAP(bbca_mem)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(bbc_state, bbcb_keyscan, 1000)        /* scan keyboard */
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_INPUT_MERGER_ANY_HIGH("irqs")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", M6502_IRQ_LINE))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbca)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbca)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DEVICE("hd6845", hd6845_device, screen_update)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(bbc_state,bbc)

	MCFG_DEVICE_ADD("saa5050", SAA5050, 12_MHz_XTAL/2)
	MCFG_SAA5050_SCREEN_SIZE(40, 25, 40)

	/* crtc */
	MCFG_MC6845_ADD("hd6845", HD6845, "screen", 16_MHz_XTAL / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(12)
	MCFG_MC6845_UPDATE_ROW_CB(bbc_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(*this, bbc_state, bbc_de_changed))
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(*this, bbc_state, bbc_hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(*this, bbc_state, bbc_vsync_changed))

	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbc)

	MCFG_DEFAULT_LAYOUT(layout_bbc)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("sn76489", SN76489, 16_MHz_XTAL/4) /* 4 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cassette relay */
	MCFG_DEVICE_ADD("samples", SAMPLES)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(bbc_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(bbc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED)
	MCFG_CASSETTE_INTERFACE("bbc_cass")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_ls_a", "bbca_cass")

	/* acia */
	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(*this, bbc_state, bbc_txd_w))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(*this, bbc_state, bbc_rts_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD(RS232_TAG, RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(*this, bbc_state, write_rxd_serial))
	MCFG_RS232_DCD_HANDLER(WRITELINE(*this, bbc_state, write_dcd_serial))
	MCFG_RS232_CTS_HANDLER(WRITELINE(*this, bbc_state, write_cts_serial))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 16_MHz_XTAL / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, bbc_state, write_acia_clock))

	/* system via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 16_MHz_XTAL / 16)
	MCFG_VIA6522_READPA_HANDLER(READ8(*this, bbc_state, bbcb_via_system_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(*this, bbc_state, bbcb_via_system_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(*this, bbc_state, bbcb_via_system_write_porta))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(*this, bbc_state, bbcb_via_system_write_portb))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<1>))

	/* EPROM sockets */
	bbc_eprom_sockets(config);
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcb)
	bbca(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(bbcb_nofdc_mem)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcb)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcb)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* speech hardware */
	MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)
	MCFG_DEVICE_ADD("tms5220", TMS5220, 640000)
	MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* user via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 16_MHz_XTAL / 16)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_READPB_HANDLER(READ8("userport", bbc_userport_slot_device, pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8("userport", bbc_userport_slot_device, pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE("centronics", centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<2>))

	/* adc */
	MCFG_DEVICE_ADD("upd7002", UPD7002, 0)
	MCFG_UPD7002_GET_ANALOGUE_CB(bbc_state, BBC_get_analogue_input)
	MCFG_UPD7002_EOC_CB(bbc_state, BBC_uPD7002_EOC)

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* fdc */
	MCFG_BBC_FDC_SLOT_ADD("fdc", bbc_fdc_devices, "acorn8271", false)
	MCFG_BBC_FDC_SLOT_INTRQ_HANDLER(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_BBC_FDC_SLOT_DRQ_HANDLER(WRITELINE(*this, bbc_state, fdc_drq_w))

	/* econet */
	MCFG_DEVICE_ADD("mc6854", MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(WRITELINE(ECONET_TAG, econet_device, data_w))
	MCFG_MC6854_OUT_IRQ_CB(WRITELINE(*this, bbc_state, adlc_irq_w))
	MCFG_ECONET_ADD()
	MCFG_ECONET_CLK_CALLBACK(WRITELINE(*this, bbc_state, econet_clk_w))
	MCFG_ECONET_DATA_CALLBACK(WRITELINE("mc6854", mc6854_device, set_rx))
	MCFG_ECONET_SLOT_ADD("econet254", 254, econet_devices, nullptr)

	/* analogue port */
	MCFG_BBC_ANALOGUE_SLOT_ADD("analogue", bbc_analogue_devices, nullptr)

	/* 1mhz bus port */
	MCFG_BBC_1MHZBUS_SLOT_ADD("1mhzbus", bbc_1mhzbus_devices, nullptr)
	MCFG_BBC_1MHZBUS_SLOT_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<3>))
	MCFG_BBC_1MHZBUS_SLOT_NMI_HANDLER(WRITELINE(*this, bbc_state, bus_nmi_w))

	/* tube port */
	MCFG_BBC_TUBE_SLOT_ADD("tube", bbc_extube_devices, nullptr)
	MCFG_BBC_TUBE_SLOT_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<4>))

	/* user port */
	MCFG_BBC_USERPORT_SLOT_ADD("userport", bbc_userport_devices, nullptr)
	MCFG_BBC_USERPORT_CB1_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb1))
	MCFG_BBC_USERPORT_CB2_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb2))

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_ls_b",      "bbcb_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_b",      "bbcb_flop")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_b_orig", "bbcb_flop_orig")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcb_de)
	bbcb(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(bbcb_mem)

	/* fdc */
	MCFG_DEVICE_REMOVE("fdc")
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_I8271_HDL_CALLBACK(WRITELINE(*this, bbc_state, motor_w))
	MCFG_I8271_OPT_CALLBACK(WRITELINE(*this, bbc_state, side_w))
	MCFG_FLOPPY_DRIVE_ADD("i8271:0", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("i8271:1", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_b_de", "bbcb_cass_de")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcb_us)
	bbcb(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(bbcb_mem)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_REFRESH_RATE(60)

	/* fdc */
	MCFG_DEVICE_REMOVE("fdc")
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_I8271_HDL_CALLBACK(WRITELINE(*this, bbc_state, motor_w))
	MCFG_I8271_OPT_CALLBACK(WRITELINE(*this, bbc_state, side_w))

	MCFG_FLOPPY_DRIVE_ADD("i8271:0", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("i8271:1", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_b_us", "bbcb_flop_us")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcbp)
	bbcb(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")  /* M6512 */
	MCFG_DEVICE_PROGRAM_MAP(bbcbp_mem)
	MCFG_DEVICE_OPCODES_MAP(bbcbp_fetch)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcbp)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcbp)

	/* fdc */
	MCFG_DEVICE_REMOVE("fdc")
	MCFG_WD1770_ADD("wd1770", 16_MHz_XTAL / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcbp128)
	bbcbp(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")  /* M6512 */
	MCFG_DEVICE_PROGRAM_MAP(bbcbp128_mem)
	MCFG_DEVICE_OPCODES_MAP(bbcbp_fetch)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcbp)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcbp)
MACHINE_CONFIG_END


/***************************************************************************

    Torch Computers

****************************************************************************/


MACHINE_CONFIG_START(torch_state::torchf)
	bbcb(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(bbcb_mem)

	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, torch)

	/* fdc */
	MCFG_DEVICE_REMOVE("fdc")
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_I8271_HDL_CALLBACK(WRITELINE(*this, bbc_state, motor_w))
	MCFG_I8271_OPT_CALLBACK(WRITELINE(*this, bbc_state, side_w))

	MCFG_FLOPPY_DRIVE_ADD_FIXED("i8271:0", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD_FIXED("i8271:1", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* Add Torch Z80 Communicator co-processor */
	MCFG_DEVICE_MODIFY("tube")
	MCFG_SLOT_DEFAULT_OPTION("zep100")
	MCFG_SLOT_FIXED(true)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(torch_state::torchh10)
	torchf(config);
	/* fdc */
	MCFG_DEVICE_REMOVE("i8271:1")

	/* Add 10MB HDD */

MACHINE_CONFIG_END


MACHINE_CONFIG_START(torch_state::torchh21)
	torchf(config);
	/* fdc */
	MCFG_DEVICE_REMOVE("i8271:1")

	/* Add 21MB HDD */

MACHINE_CONFIG_END


/***************************************************************************

    Acorn Business Computers

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::abc110)
	bbcbp(config);
	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770:1")

	/* Acorn Z80 co-processor */
	MCFG_DEVICE_MODIFY("tube")
	MCFG_SLOT_DEFAULT_OPTION("z80")
	MCFG_SLOT_FIXED(true)

	/* Add ADAPTEC ACB-4000 Winchester Disc Controller */
	//MCFG_DEVICE_ADD(SCSIBUS_TAG, SCSI_PORT, 0)
	//MCFG_SCSI_DATA_INPUT_BUFFER("scsi_data_in")
	//MCFG_SCSI_MSG_HANDLER(WRITELINE("scsi_ctrl_in", input_buffer_device, write_bit0))
	//MCFG_SCSI_BSY_HANDLER(WRITELINE(*this, bbc_state, scsi_bsy_w))
	//MCFG_SCSI_REQ_HANDLER(WRITELINE(*this, bbc_state, scsi_req_w))
	//MCFG_SCSI_IO_HANDLER(WRITELINE("scsi_ctrl_in", input_buffer_device, write_bit6))
	//MCFG_SCSI_CD_HANDLER(WRITELINE("scsi_ctrl_in", input_buffer_device, write_bit7))
	//MCFG_SCSIDEV_ADD(SCSIBUS_TAG ":" SCSI_PORT_DEVICE1, "harddisk", ACB4070, SCSI_ID_0)

	//MCFG_SCSI_OUTPUT_LATCH_ADD("scsi_data_out", SCSIBUS_TAG)
	//MCFG_DEVICE_ADD("scsi_data_in", INPUT_BUFFER, 0)
	//MCFG_DEVICE_ADD("scsi_ctrl_in", INPUT_BUFFER, 0)
	/* Add 10MB ST-412 Winchester */

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::acw443)
	bbcbp(config);
	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770:1")

	/* Add 32016 co-processor */
	//MCFG_DEVICE_MODIFY("tube")
	//MCFG_SLOT_DEFAULT_OPTION("32016")
	//MCFG_SLOT_FIXED(true)

	/* Add ADAPTEC ACB-4000 Winchester Disc Controller */

	/* Add 10MB ST-412 Winchester ABC210 */

	/* Add 20MB ST-412 Winchester Cambridge */

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_32016", "bbc_flop_32016")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::abc310)
	bbcbp(config);
	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770:1")

	/* Acorn 80286 co-processor */
	MCFG_DEVICE_MODIFY("tube")
	MCFG_SLOT_DEFAULT_OPTION("80286")
	MCFG_SLOT_FIXED(true)

	/* Add ADAPTEC ACB-4000 Winchester Disc Controller */

	/* Add 10MB ST-412 Winchester */

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
MACHINE_CONFIG_END


/***************************************************************************

    Reuters APM Board (Application Processor Module)

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::reutapm)
	bbcbp(config);
	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")  /* M6512 */
	MCFG_DEVICE_PROGRAM_MAP(reutapm_mem)
	MCFG_DEVICE_OPCODES_MAP(bbcbp_fetch)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("mono")
	MCFG_DEVICE_REMOVE("sn76489")
	MCFG_DEVICE_REMOVE("samples")
	MCFG_DEVICE_REMOVE("vsm")
	MCFG_DEVICE_REMOVE("tms5220")

	/* cassette */
	MCFG_DEVICE_REMOVE( "cassette" )

	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770")

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")

	/* expansion ports */
	MCFG_DEVICE_REMOVE("analogue")
MACHINE_CONFIG_END


/***************************************************************************

    Econet X25 Gateway

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::econx25)
	bbcbp(config);
	/* sound hardware */
	MCFG_DEVICE_REMOVE("vsm")
	MCFG_DEVICE_REMOVE("tms5220")

	/* fdc */
	//MCFG_DEVICE_REMOVE("wd1770")

	/* Add Econet X25 Gateway co-processor */
	//MCFG_DEVICE_MODIFY("tube")
	//MCFG_DEVICE_SLOT_INTERFACE(bbc_x25tube_devices, "x25", true)

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


/***************************************************************************

    BBC Master Series

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::bbcm)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M65SC02, 16_MHz_XTAL/8)        /* 2.00 MHz */
	MCFG_DEVICE_PROGRAM_MAP(bbcm_mem)
	MCFG_DEVICE_OPCODES_MAP(bbcm_fetch)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(bbc_state, bbcb_keyscan, 1000)        /* scan keyboard */
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_INPUT_MERGER_ANY_HIGH("irqs")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", M6502_IRQ_LINE))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcm)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcm)

	MCFG_DEFAULT_LAYOUT(layout_bbcm)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DEVICE("hd6845", hd6845_device, screen_update)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(bbc_state, bbc)

	MCFG_DEVICE_ADD("saa5050", SAA5050, 12_MHz_XTAL / 2)
	MCFG_SAA5050_SCREEN_SIZE(40, 25, 40)

	/* crtc */
	MCFG_MC6845_ADD("hd6845", HD6845, "screen", 16_MHz_XTAL / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(12)
	MCFG_MC6845_UPDATE_ROW_CB(bbc_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(*this, bbc_state, bbc_de_changed))
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(*this, bbc_state, bbc_hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(*this, bbc_state, bbc_vsync_changed))

	MCFG_VIDEO_START_OVERRIDE(bbc_state, bbc)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("sn76489", SN76489, 16_MHz_XTAL/4) /* 4 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* cassette relay */
	MCFG_DEVICE_ADD("samples", SAMPLES)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(bbc_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* rtc and cmos */
	MCFG_MC146818_ADD( "rtc", 32.768_kHz_XTAL )

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(bbc_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED)
	MCFG_CASSETTE_INTERFACE("bbc_cass")

	// 2 x cartridge sockets in BBC-Master
	MCFG_GENERIC_CARTSLOT_ADD("exp_rom1", generic_plain_slot, "bbcm_cart")
	MCFG_GENERIC_LOAD(bbc_state, bbcm_exp1_load)

	MCFG_GENERIC_CARTSLOT_ADD("exp_rom2", generic_plain_slot, "bbcm_cart")
	MCFG_GENERIC_LOAD(bbc_state, bbcm_exp2_load)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cass_ls_m", "bbcm_cass")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_m", "bbcm_flop")
	MCFG_SOFTWARE_LIST_ADD("cart_ls_m", "bbcm_cart")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_a",      "bbca_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("cass_ls_b",      "bbcb_cass")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop_ls_b",      "bbcb_flop")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop_ls_b_orig", "bbcb_flop_orig")

	/* acia */
	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(*this, bbc_state, bbc_txd_w))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE(*this, bbc_state, bbc_rts_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<0>))

	MCFG_DEVICE_ADD(RS232_TAG, RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(*this, bbc_state, write_rxd_serial))
	MCFG_RS232_DCD_HANDLER(WRITELINE(*this, bbc_state, write_dcd_serial))
	MCFG_RS232_CTS_HANDLER(WRITELINE(*this, bbc_state, write_cts_serial))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 16_MHz_XTAL / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, bbc_state, write_acia_clock))

	/* adc */
	MCFG_DEVICE_ADD("upd7002", UPD7002, 0)
	MCFG_UPD7002_GET_ANALOGUE_CB(bbc_state, BBC_get_analogue_input)
	MCFG_UPD7002_EOC_CB(bbc_state, BBC_uPD7002_EOC)

	/* system via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 16_MHz_XTAL / 16)
	MCFG_VIA6522_READPA_HANDLER(READ8(*this, bbc_state, bbcb_via_system_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(*this, bbc_state, bbcb_via_system_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(*this, bbc_state, bbcb_via_system_write_porta))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(*this, bbc_state, bbcb_via_system_write_portb))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<1>))

	/* user via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 16_MHz_XTAL / 16)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_READPB_HANDLER(READ8("userport", bbc_userport_slot_device, pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8("userport", bbc_userport_slot_device, pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE("centronics", centronics_device, write_strobe))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<2>))

	/* fdc */
	MCFG_WD1770_ADD("wd1770", 16_MHz_XTAL / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", bbc_floppies_525, "525qd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* econet */
	MCFG_DEVICE_ADD("mc6854", MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(WRITELINE(ECONET_TAG, econet_device, data_w))
	MCFG_MC6854_OUT_IRQ_CB(WRITELINE(*this, bbc_state, adlc_irq_w))
	MCFG_ECONET_ADD()
	MCFG_ECONET_CLK_CALLBACK(WRITELINE(*this, bbc_state, econet_clk_w))
	MCFG_ECONET_DATA_CALLBACK(WRITELINE("mc6854", mc6854_device, set_rx))
	MCFG_ECONET_SLOT_ADD("econet254", 254, econet_devices, nullptr)

	/* analogue port */
	MCFG_BBC_ANALOGUE_SLOT_ADD("analogue", bbc_analogue_devices, nullptr)

	/* 1mhz bus port */
	MCFG_BBC_1MHZBUS_SLOT_ADD("1mhzbus", bbc_1mhzbus_devices, nullptr)
	MCFG_BBC_1MHZBUS_SLOT_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<3>))
	MCFG_BBC_1MHZBUS_SLOT_NMI_HANDLER(WRITELINE(*this, bbc_state, bus_nmi_w))

	/* tube ports */
	MCFG_BBC_TUBE_SLOT_ADD("intube", bbc_intube_devices, nullptr)
	MCFG_BBC_TUBE_SLOT_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<4>))
	MCFG_BBC_TUBE_SLOT_ADD("extube", bbc_extube_devices, nullptr)
	MCFG_BBC_TUBE_SLOT_IRQ_HANDLER(WRITELINE("irqs", input_merger_device, in_w<5>))

	/* user port */
	MCFG_BBC_USERPORT_SLOT_ADD("userport", bbc_userport_devices, nullptr)
	MCFG_BBC_USERPORT_CB1_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb1))
	MCFG_BBC_USERPORT_CB2_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb2))
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcmt)
	bbcm(config);
	/* Acorn 65C102 co-processor */
	MCFG_DEVICE_MODIFY("intube")
	MCFG_SLOT_DEFAULT_OPTION("65c102")
	MCFG_SLOT_FIXED(true)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcmaiv)
	bbcm(config);
	/* Acorn 65C102 co-processor */
	MCFG_DEVICE_MODIFY("intube")
	MCFG_SLOT_DEFAULT_OPTION("65c102")
	MCFG_SLOT_FIXED(true)

	/* Add Philips VP415 Laserdisc player */

	/* Acorn Tracker Ball */
	MCFG_DEVICE_MODIFY("userport")
	MCFG_SLOT_DEFAULT_OPTION("tracker")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcmet)
	bbcm(config);
	/* printer */
	MCFG_DEVICE_REMOVE("centronics")
	MCFG_DEVICE_REMOVE("cent_data_out")

	/* cassette */
	MCFG_DEVICE_REMOVE("cassette")

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")

	/* acia */
	MCFG_DEVICE_REMOVE("acia6850")
	MCFG_DEVICE_REMOVE(RS232_TAG)
	MCFG_DEVICE_REMOVE("acia_clock")

	/* devices */
	MCFG_DEVICE_REMOVE("upd7002")
	MCFG_DEVICE_REMOVE("via6522_1")

	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770")

	/* expansion ports */
	MCFG_DEVICE_REMOVE("analogue")
	MCFG_DEVICE_REMOVE("extube")
	MCFG_DEVICE_REMOVE("1mhzbus")
	MCFG_DEVICE_REMOVE("userport")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcm512)
	bbcm(config);
	/* Acorn Intel 80186 co-processor */
	MCFG_DEVICE_MODIFY("intube")
	MCFG_SLOT_DEFAULT_OPTION("80186")
	MCFG_SLOT_FIXED(true)

	/* Acorn Mouse */
	MCFG_DEVICE_MODIFY("userport")
	MCFG_SLOT_DEFAULT_OPTION("m512mouse")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::bbcmarm)
	bbcm(config);
	/* Acorn ARM co-processor */
	MCFG_DEVICE_MODIFY("extube")
	MCFG_SLOT_DEFAULT_OPTION("arm")
	MCFG_SLOT_FIXED(true)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::discmon)
	bbcm(config);
	/* Add coin slot */

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::discmate)
	bbcm(config);
	/* Add Sony CDK-3000PII Auto Disc Loader */

	/* Add interface boards connected to cassette and RS423 */

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::cfa3000)
	bbcm(config);
	MCFG_MACHINE_START_OVERRIDE(bbc_state, cfa3000)

	/* fdc */
	MCFG_DEVICE_MODIFY("wd1770:0")
	MCFG_DEVICE_SLOT_INTERFACE(bbc_floppies_525, nullptr, false)
	MCFG_DEVICE_MODIFY("wd1770:1")
	MCFG_DEVICE_SLOT_INTERFACE(bbc_floppies_525, nullptr, false)

	/* keyboard */
	MCFG_DEVICE_MODIFY("userport")
	MCFG_SLOT_DEFAULT_OPTION("cfa3000kbd")

	/* option board */
	MCFG_DEVICE_MODIFY("1mhzbus")
	MCFG_SLOT_DEFAULT_OPTION("cfa3000opt")

	/* analogue dials/sensors */
	MCFG_DEVICE_MODIFY("analogue")
	MCFG_SLOT_DEFAULT_OPTION("cfa3000a")

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
MACHINE_CONFIG_END


/***************************************************************************

    BBC Master Compact

****************************************************************************/


MACHINE_CONFIG_START(bbc_state::bbcmc)
	bbcm(config);
	/* cassette */
	MCFG_DEVICE_REMOVE("cassette")

	/* fdc */
	MCFG_DEVICE_REMOVE("wd1770")

	MCFG_WD1772_ADD("wd1772", 16_MHz_XTAL / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD_FIXED("wd1772:0", bbc_floppies_35, "35dd", bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:1", bbc_floppies_35, nullptr, bbc_state::floppy_formats_bbc)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* eeprom pcd8572 */
	//MCFG_DEVICE_REMOVE("rtc")

	MCFG_MACHINE_START_OVERRIDE(bbc_state, bbcmc)
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, bbcmc)

	/* user via */
	MCFG_DEVICE_MODIFY("via6522_1")
	// TODO: Joyport connected to PB0-PB4 only. PB5-PB7 are expansion port.
	MCFG_VIA6522_READPB_HANDLER(READ8("joyport", bbc_joyport_slot_device, pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8("joyport", bbc_joyport_slot_device, pb_w))

	/* cartridge sockets */
	MCFG_DEVICE_REMOVE("exp_rom1")
	MCFG_DEVICE_REMOVE("exp_rom2")

	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_a")
	MCFG_SOFTWARE_LIST_REMOVE("cass_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_m")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_b_orig")
	MCFG_SOFTWARE_LIST_REMOVE("cart_ls_m")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_mc", "bbcmc_flop")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop_ls_pro128s", "pro128s_flop")

	/* expansion ports */
	MCFG_BBC_JOYPORT_ADD("joyport", bbc_joyport_devices, "joystick")
	MCFG_BBC_JOYPORT_CB1_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb1))
	MCFG_BBC_JOYPORT_CB2_HANDLER(WRITELINE("via6522_1", via6522_device, write_cb2))

	MCFG_DEVICE_REMOVE("analogue")
	MCFG_DEVICE_REMOVE("intube")
	MCFG_DEVICE_REMOVE("extube")
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::pro128s)
	bbcmc(config);
	/* software lists */
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_mc")
	MCFG_SOFTWARE_LIST_REMOVE("flop_ls_pro128s")
	MCFG_SOFTWARE_LIST_ADD("flop_ls_pro128s", "pro128s_flop")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("flop_ls_mc", "bbcmc_flop")
MACHINE_CONFIG_END


/***************************************************************************

    LTM Portables

****************************************************************************/

/* Both LTM machines used a 9" Hantarex MT3000 green monitor */

MACHINE_CONFIG_START(bbc_state::ltmpbp)
	bbcbp(config);
	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, ltmpbp)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(bbc_state::ltmpm)
	bbcm(config);
	/* basic machine hardware */
	MCFG_MACHINE_RESET_OVERRIDE(bbc_state, ltmpm)
MACHINE_CONFIG_END


/* the BBC came with 4 rom sockets on the motherboard as shown in the model A driver */
/* you could get a number of rom upgrade boards that took this up to 16 roms as in the */
/* model B driver */

ROM_START(bbca)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x14000,"option",0) /* ROM */
	/* rom page 12 00000 IC52  SPARE SOCKET */
	/* rom page 13 04000 IC88  SPARE SOCKET */
	/* rom page 14 08000 IC100 SPARE SOCKET */
	/* rom page 15 0c000 IC101 BASIC */
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
	ROM_SYSTEM_BIOS( 4, "os01b1", "OS 0.10 / BASIC1" )
	ROMX_LOAD("os01.rom",   0x10000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(5)) /* os */
	/* OS0.1 does not support rom paging, load BASIC into all pages */
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(5)) /* rom page 0  00000 */
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )

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
	/* rom page 12 30000 IC52  DFS */
	/* rom page 13 34000 IC88  SPARE SOCKET */
	/* rom page 14 38000 IC100 SPARE SOCKET */
	/* rom page 15 3c000 IC101 BASIC */
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
	ROM_SYSTEM_BIOS( 4, "os01b1", "OS 0.10 / BASIC1" )
	ROMX_LOAD("os01.rom",   0x40000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(5)) /* os */
	/* OS0.1 does not support rom paging, load BASIC into all pages */
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(5)) /* rom page 0 00000 */
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )

	ROM_LOAD("dnfs120-201666.rom", 0x30000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
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
	/* rom page 12 30000 IC72 DFS */
	/* rom page 13 34000 IC73 SPARE SOCKET */
	/* rom page 14 38000 IC74 SPARE SOCKET */
	/* rom page 15 3c000 IC75 BASIC */
	ROM_DEFAULT_BIOS("os12")
	ROM_SYSTEM_BIOS( 0, "os12", "OS 1.20 / BASIC2" )
	ROMX_LOAD("os_de.rom",   0x40000, 0x4000, CRC(b7262caf) SHA1(aadf90338ee9d1c85dfa73beba50e930c2a38f10), ROM_BIOS(1))
	ROMX_LOAD("basic2.rom",  0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1)) /* rom page 15 3c000 */

	ROM_LOAD("dfs10.rom",    0x30000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
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
	/* rom page 12 30000 IC72 VIEW2.1 */
	/* rom page 13 34000 IC73 US DNFS */
	/* rom page 14 38000 IC74 US BASIC */
	/* rom page 15 3c000 IC75 SPARE SOCKET */
	ROM_DEFAULT_BIOS("os10b3")
	ROM_SYSTEM_BIOS( 0, "os10b3", "OS A1.0 / BASIC3" )
	ROMX_LOAD("usmos10.rom",  0x40000, 0x4000, CRC(c8e946a9) SHA1(83d91d089dca092d2c8b7c3650ff8143c9069b89), ROM_BIOS(1))
	ROMX_LOAD("usbasic3.rom", 0x38000, 0x4000, CRC(161b9539) SHA1(b39014610a968789afd7695aa04d1277d874405c), ROM_BIOS(1)) /* rom page 15 3c000 */

	ROM_LOAD("viewa210.rom", 0x30000, 0x4000, CRC(4345359f) SHA1(88c93df1854f5fbe6cd6e5f0e29a8bf4ea3b5614))
	ROM_LOAD("usdnfs10.rom", 0x34000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(torchf)
	ROM_REGION(0x08000,"maincpu",ROMREGION_ERASEFF) /* RAM */

	ROM_REGION(0x44000,"option",0) /* ROM */
	/* rom page 12 30000 IC52  BASIC */
	/* rom page 13 34000 IC88  DNFS */
	/* rom page 14 38000 IC100 CPN */
	/* rom page 15 3c000 IC101 SPARE SOCKET */
	ROM_LOAD("os12.rom", 0x40000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d)) /* os */

	ROM_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dnfs120-201666.rom", 0x34000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_DEFAULT_BIOS("mcp120cbl")
	ROM_SYSTEM_BIOS( 0, "mcp120cbl", "MCP120CBL" )
	ROMX_LOAD("mcp120cbl.rom", 0x38000, 0x4000, CRC(851d0879) SHA1(2e54ef15692ba7dd9fcfd1ef0d660464a772b156), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mcp101ci", "MCP101CI" )
	ROMX_LOAD("mcp101ci.rom", 0x38000, 0x4000, NO_DUMP, ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "mcp041cbl", "MCP041CBL" )
	ROMX_LOAD("mcp041cbl.rom", 0x38000, 0x4000, CRC(b36f07f4) SHA1(bd53f09bf73357845a6f97df1ee9e5aea5cdca90), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "cpn071", "CPN71+" )
	ROMX_LOAD("cpn071.rom",    0x38000, 0x2000, CRC(fcb1bdc8) SHA1(756e22f6d76eb26206765f92c78c7152944102b6), ROM_BIOS(4))
	ROM_RELOAD(                0x3a000, 0x2000 )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


#define rom_torchh10 rom_torchf


#define rom_torchh21 rom_torchf


ROM_START(bbcbp)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("os20")
	ROM_SYSTEM_BIOS( 0, "os20", "OS 2.00" )
	ROMX_LOAD("bpos2.ic71", 0x3c000, 0x4000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2), ROM_BIOS(1)) /* rom page 15 3C000 BASIC */
	ROM_CONTINUE(           0x40000, 0x4000)  /* OS */
	/* rom page 0  00000 SWRAM (B+ 128K only) */
	/* rom page 1  04000 SWRAM (B+ 128K only) */
	/* rom page 2  08000 IC35 32K IN PAGE 3 */
	/* rom page 3  0c000 IC35 SPARE SOCKET */
	/* rom page 4  10000 IC44 32K IN PAGE 5 */
	/* rom page 5  14000 IC44 ADFS */
	/* rom page 6  18000 IC57 32K IN PAGE 7 */
	/* rom page 7  1c000 IC57 DDFS */
	/* rom page 8  20000 IC62 32K IN PAGE 9 */
	/* rom page 9  24000 IC62 SPARE SOCKET */
	/* rom page 10 28000 IC68 32K IN PAGE 11 */
	/* rom page 11 2c000 IC68 SPARE SOCKET */
	/* rom page 12 30000 SWRAM (B+ 128K only) */
	/* rom page 13 34000 SWRAM (B+ 128K only) */
	/* rom page 14 38000 IC71 32K IN PAGE 15 */
	/* rom page 15 3C000 IC71 BASIC */
	ROM_LOAD("adfs130.rom", 0x14000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8))
	ROM_LOAD("ddfs223.rom", 0x1c000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_bbcbp128 rom_bbcbp


ROM_START(abc110)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos200")
	ROM_SYSTEM_BIOS( 0, "mos200", "MOS2.00" )
	ROMX_LOAD("mos200.rom", 0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(1))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos123stor", "MOS1.23 + ViewStore" )
	ROMX_LOAD("mos123stor.rom", 0x3c000, 0x4000, CRC(4e84f452) SHA1(145ee54f04b3eb4d0e5afaabe21915be48db3c54), ROM_BIOS(2)) /* rom page 15 3C000 ViewStore */
	ROM_CONTINUE(               0x40000, 0x4000)  /* OS */
	ROM_SYSTEM_BIOS( 2, "mos123", "MOS1.23" )
	ROMX_LOAD("mos123.rom", 0x40000, 0x4000, CRC(90d31d08) SHA1(42a01892cf8bd2ada4db1c8b36aff80c85eb5dcb), ROM_BIOS(3))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "mos120", "MOS1.20" )
	ROMX_LOAD("mos120.rom", 0x40000, 0x4000, CRC(0a1e83a0) SHA1(21dc3a94eef7c003b194686730fb461779f44925), ROM_BIOS(4))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(4))
	/* rom page 0  00000 */
	/* rom page 1  04000 IC71 selectable with link S13 */
	/* rom page 2  08000 IC35 32K IN PAGE 3 */
	/* rom page 3  0c000 IC35 SPARE SOCKET */
	/* rom page 4  10000 IC44 32K IN PAGE 5 */
	/* rom page 5  14000 IC44 DDFS */
	/* rom page 6  18000 IC57 32K IN PAGE 7 */
	/* rom page 7  1c000 IC57 ADFS */
	/* rom page 8  20000 IC62 32K IN PAGE 9 */
	/* rom page 9  24000 IC62 SPARE SOCKET */
	/* rom page 10 28000 IC68 32K IN PAGE 11 */
	/* rom page 11 2c000 IC68 SPARE SOCKET */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3C000 IC71 BASIC */
	//ROM_LOAD("ddfs223.rom", 0x14000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))
	ROM_LOAD("acwddfs225.rom", 0x14000, 0x4000, CRC(7d0f9016) SHA1(bdfe44c79e18142d747436627e71a362a04cf746))
	ROM_LOAD("adfs130.rom", 0x1c000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_abc310 rom_abc110


ROM_START(acw443)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos210")
	ROM_SYSTEM_BIOS( 0, "mos210", "MOS2.10" )
	ROMX_LOAD("acwmos210.rom", 0x40000, 0x4000, CRC(168d6753) SHA1(dcd01d8f5f6e0cd92ae626ca52a3db71abf5d282), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos200", "MOS2.00" )
	ROMX_LOAD("mos200.rom", 0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(2))
	/* rom page 0  00000 */
	/* rom page 1  04000  IC71 selectable with link S13 */
	/* rom page 2  08000  IC35 32K IN PAGE 3 */
	/* rom page 3  0c000  IC35 DNFS */
	/* rom page 4  10000  IC44 32K IN PAGE 5 */
	/* rom page 5  14000  IC44 ACW DFS */
	/* rom page 6  18000  IC57 32K IN PAGE 7 */
	/* rom page 7  1c000  IC57 TERMINAL */
	/* rom page 8  20000  IC62 32K IN PAGE 9 */
	/* rom page 9  24000  IC62 ADFS */
	/* rom page 10 28000  IC68 BASIC */
	/* rom page 11 2c000  IC68 Unused OS? */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3C000  IC71 selectable with link S13 */
	ROM_LOAD("dnfs120-201666.rom", 0x0c000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("acwddfs225.rom", 0x14000, 0x4000, CRC(7d0f9016) SHA1(bdfe44c79e18142d747436627e71a362a04cf746))
	ROM_LOAD("acwterminal.rom", 0x1c000, 0x4000, CRC(81afaeb9) SHA1(6618ed9158776b4b8aa030957bd19ba77e4a993c))
	ROM_LOAD("adfs130.rom", 0x24000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8))
	ROM_LOAD("basic200.rom", 0x28000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(reutapm)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	/* rom page 0  00000 */
	/* rom page 1  04000 */
	/* rom page 2  08000  32K IN PAGE 3 */
	/* rom page 3  0c000  SPARE SOCKET */
	/* rom page 4  10000  32K IN PAGE 5 */
	/* rom page 5  14000  SPARE SOCKET */
	/* rom page 6  18000  32K IN PAGE 7 */
	/* rom page 7  1c000  SPARE SOCKET */
	/* rom page 8  20000  32K IN PAGE 9 */
	/* rom page 9  24000  SPARE SOCKET */
	/* rom page 10 28000  32K IN PAGE 11 */
	/* rom page 11 2c000  SPARE SOCKET */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000  32K IN PAGE 15 */
	/* rom page 15 3C000  SPARE SOCKET */
	ROM_LOAD("reutera100.rom", 0x1c000, 0x4000, CRC(98ebabfb) SHA1(a7887e1e5c206203491e1e06682b9508b0fef49d))
	ROM_LOAD("reuterb.rom",    0x2c000, 0x4000, CRC(9e02f59b) SHA1(1e63aa3bf4b37bf9ba41e454f95db05c3d15bfbf))

	ROM_REGION(0x4000, "os", 0)
	ROM_LOAD("mos_r030.rom", 0x0000, 0x4000, CRC(8b652337) SHA1(6a5c7ace255c8ac96c983d5ba67084fbd71ff61e))
ROM_END


ROM_START(econx25)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000, "option", 0) /* ROM */
	ROM_LOAD("0246,201_01_x25os.rom", 0x40000, 0x4000, CRC(8b652337) SHA1(6a5c7ace255c8ac96c983d5ba67084fbd71ff61e))
	/* rom page 0  00000 */
	/* rom page 1  04000  IC71 selectable with link S13 */
	/* rom page 2  08000  IC35 32K IN PAGE 3 */
	/* rom page 3  0c000  IC35 SPARE SOCKET */
	/* rom page 4  10000  IC44 32K IN PAGE 5 */
	/* rom page 5  14000  IC44 SPARE SOCKET */
	/* rom page 6  18000  IC57 32K IN PAGE 7 */
	/* rom page 7  1c000  IC57 ANFS */
	/* rom page 8  20000  IC62 BASIC */
	/* rom page 9  24000  IC62 Unused OS */
	/* rom page 10 28000  IC68 32K IN PAGE 11 */
	/* rom page 11 2c000  IC68 SPARE SOCKET */
	/* rom page 12 30000 */
	/* rom page 13 34000 */
	/* rom page 14 38000 */
	/* rom page 15 3C000  IC71 selectable with link S13 */
	ROM_LOAD("2201,248_03_anfs.rom",  0x1c000, 0x4000, CRC(744a60a7) SHA1(c733b108d74cf3b1c5de395335236800a7c9c0d8))
	ROM_LOAD("0201,241_01_bpos2.rom", 0x20000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
	/* X25 TSI is in IC37 which is supposed to take a speech PHROM, so not sure where this is mapped */
	ROM_LOAD("0246,215_02_x25tsi_v0.51.rom", 0x30000, 0x4000, CRC(71dd84e4) SHA1(bbfa892fdcc6f753dda5134ecb97cc7c42b959c2))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcm)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mos350", "Enhanced MOS 3.50" )
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(2))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 ANFS */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	//ROM_LOAD("anfs425-2201351.rom", 0x20000, 0x4000, CRC(c2a6655e) SHA1(14f75d36ffe9af14aaac42df55b4fe3729ba75cf))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(1))
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(2))
ROM_END


#define rom_bbcmt rom_bbcm
#define rom_bbcm512 rom_bbcm


ROM_START(bbcmaiv)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 VFS */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
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
	ROMX_LOAD("mos400.ic24", 0x30000, 0x10000, CRC(81729034) SHA1(d4bc2c7f5e66b5298786138f395908e70c772971), ROM_BIOS(1))
	ROM_COPY("option", 0x34000, 0x24000, 0xC000) /* Mirror */
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   NO SOCKET */
	/* 24000 rom 9   IC24 BASIC */
	/* 28000 rom 10  IC24 ANFS */
	/* 2c000 rom 11  IC24 MOS code */
	/* 30000 rom 12  IC24 UNUSED */
	/* 34000 rom 13  IC24 BASIC */
	/* 38000 rom 14  IC24 ANFS */
	/* 3c000 rom 15  IC24 MOS code */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40,"rtc",0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos400.cmos", 0x0E, 0x32, BAD_DUMP CRC(fff41cc5) SHA1(3607568758f90b3bd6c7dc9533e2aa24f9806ff3), ROM_BIOS(1))
ROM_END


ROM_START(bbcmarm)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 ANFS */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	//ROM_LOAD("anfs425-2201351.rom", 0x20000, 0x4000, CRC(c2a6655e) SHA1(14f75d36ffe9af14aaac42df55b4fe3729ba75cf))

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
	ROMX_LOAD("mos500.ic49", 0x30000, 0x10000, CRC(f6170023) SHA1(140d002d2d9cd34b47197a2ba823505af2a84633), ROM_BIOS(2))
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   EXTERNAL */
	/* 04000 rom 1   EXTERNAL */
	/* 08000 rom 2   IC23 SPARE SOCKET */
	/* 0c000 rom 3   IC17 SPARE SOCKET */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   IC29 SPARE SOCKET */
	/* 24000 rom 9   UNUSED */
	/* 28000 rom 10  UNUSED */
	/* 2c000 rom 11  UNUSED */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  IC16 ADFS */
	/* 38000 rom 14  IC16 BASIC */
	/* 3c000 rom 15  IC16 Utils */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcmc_ar)
	ROM_REGION(0x10000,"maincpu",ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000,"option",0) /* ROM */
	ROM_DEFAULT_BIOS("mos511i")
	ROM_SYSTEM_BIOS( 0, "mos511i", "International MOS 5.11" )
	ROMX_LOAD("mos511.ic49", 0x30000, 0x10000, BAD_DUMP CRC(8708803c) SHA1(d2170c8b9b536f3ad84a4a603a7fe712500cc751), ROM_BIOS(1)) /* Merged individual ROM bank dumps */
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   EXTERNAL */
	/* 04000 rom 1   EXTERNAL */
	/* 08000 rom 2   IC23 International */
	/* 0c000 rom 3   IC17 SPARE SOCKET */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   IC29 Arabian */
	/* 24000 rom 9   UNUSED */
	/* 28000 rom 10  UNUSED */
	/* 2c000 rom 11  UNUSED */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  IC16 ADFS */
	/* 38000 rom 14  IC16 BASIC */
	/* 3c000 rom 15  IC16 Utils */
	ROM_LOAD("international16.rom", 0x8000 , 0x4000, CRC(0ef527b1) SHA1(dc5149ccf588cd591a6ad47727474ef3313272ce) )
	ROM_LOAD("arabian-c22.rom"    , 0x20000, 0x4000, CRC(4f3aadff) SHA1(2bbf61ba68264ce5845aab9c54e750b0efe219c8) )

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(pro128s)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000, "option", 0) /* ROM */
	ROM_DEFAULT_BIOS("mos510o")
	ROM_SYSTEM_BIOS(0, "mos510o", "Olivetti MOS 5.10")
	ROMX_LOAD("mos510o.ic49", 0x30000, 0x10000, CRC(c16858d3) SHA1(ad231ed21a55e493b553703285530d1cacd3de7a), ROM_BIOS(1))
	ROM_COPY("option", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   EXTERNAL */
	/* 04000 rom 1   EXTERNAL */
	/* 08000 rom 2   IC23 SPARE SOCKET */
	/* 0c000 rom 3   IC17 SPARE SOCKET */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   IC29 SPARE SOCKET */
	/* 24000 rom 9   UNUSED */
	/* 28000 rom 10  UNUSED */
	/* 2c000 rom 11  UNUSED */
	/* 30000 rom 12  UNUSED */
	/* 34000 rom 13  IC16 ADFS */
	/* 38000 rom 14  IC16 BASIC */
	/* 3c000 rom 15  IC16 Utils */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)
ROM_END


ROM_START(discmon)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000, "option", 0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 DiscMonitor */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("discmonitor406.rom", 0x20000, 0x4000, CRC(12e30e9b) SHA1(0e5356531978e08e75913e793cb0afc0e75e61ad))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(1))
ROM_END


ROM_START(discmate)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000, "option", 0) /* ROM */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(1))
	ROM_COPY("option", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 Discmaster */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("discmaster303.rom", 0x20000, 0x4000, CRC(73974057) SHA1(79f99eae62ab46818386ab8a67fe50319ae30226))

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(1))
ROM_END


ROM_START(cfa3000)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) /* ROM MEMORY */

	ROM_REGION(0x44000, "option", 0) /* ROM */
	ROM_LOAD("cfa3000_3_4_iss10.3.ic41",             0x10000, 0x08000, CRC(ecb385ab) SHA1(eafa9b34cb1cf63790f74332bb7d85ee356b6973))
	ROM_LOAD("cfa3000_sm_iss10.3.ic37",              0x18000, 0x08000, CRC(c07aee5f) SHA1(1994e3755dc15d1ea7e105bc19cd57893b719779))
	ROM_LOAD("acorn_mos,tinsley_64k,iss10.3.ic24", 0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54))
	ROM_COPY("option", 0x20000, 0x30000, 0x10000) /* Mirror MOS */
	ROM_COPY("option", 0x30000, 0x40000, 0x04000) /* Move loaded roms into place */
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 */
	/* 24000 rom 9   IC24 */
	/* 28000 rom 10  IC24 */
	/* 2c000 rom 11  IC24 */
	/* 30000 rom 12  IC24 */
	/* 34000 rom 13  IC24 DFS */
	/* 38000 rom 14  IC24 BASIC */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */

	ROM_REGION(0x4000, "os", 0)
	ROM_COPY("option", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROM_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac))
ROM_END


#define rom_ltmpbp rom_bbcbp
#define rom_ltmpm rom_bbcm


/*     YEAR  NAME      PARENT  COMPAT MACHINE   INPUT   CLASS        INIT      COMPANY            FULLNAME                              FLAGS */
COMP ( 1981, bbcb,     0,      bbca,  bbcb,     bbcb,   bbc_state,   init_bbc, "Acorn",           "BBC Micro Model B",                  MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1981, bbca,     bbcb,   0,     bbca,     bbca,   bbc_state,   init_bbc, "Acorn",           "BBC Micro Model A",                  MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1982, torchf,   bbcb,   0,     torchf,   torch,  torch_state, init_bbc, "Torch Computers", "Torch CF240",                        MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1982, torchh10, bbcb,   0,     torchh10, torch,  torch_state, init_bbc, "Torch Computers", "Torch CH240/10",                     MACHINE_NOT_WORKING)
COMP ( 1982, torchh21, bbcb,   0,     torchh21, torch,  torch_state, init_bbc, "Torch Computers", "Torch CH240/21",                     MACHINE_NOT_WORKING)
COMP ( 1982, bbcb_de,  bbcb,   0,     bbcb_de,  bbcb,   bbc_state,   init_bbc, "Acorn",           "BBC Micro Model B (German)",         MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1983, bbcb_us,  bbcb,   0,     bbcb_us,  bbcb,   bbc_state,   init_bbc, "Acorn",           "BBC Micro Model B (US)",             MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1985, bbcbp,    0,      bbcb,  bbcbp,    bbcbp,  bbc_state,   init_bbc, "Acorn",           "BBC Micro Model B+ 64K",             MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1985, bbcbp128, bbcbp,  0,     bbcbp128, bbcbp,  bbc_state,   init_bbc, "Acorn",           "BBC Micro Model B+ 128K",            MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1985, abc110,   bbcbp,  0,     abc110,   abc,    bbc_state,   init_bbc, "Acorn",           "ABC 110",                            MACHINE_NOT_WORKING)
COMP ( 1985, acw443,   bbcbp,  0,     acw443,   abc,    bbc_state,   init_bbc, "Acorn",           "ABC 210/Cambridge Workstation",      MACHINE_NOT_WORKING)
COMP ( 1985, abc310,   bbcbp,  0,     abc310,   abc,    bbc_state,   init_bbc, "Acorn",           "ABC 310",                            MACHINE_NOT_WORKING)
COMP ( 1985, ltmpbp,   bbcbp,  0,     ltmpbp,   ltmpbp, bbc_state,   init_bbc, "Lawrie T&M Ltd.", "LTM Portable (B+)",                  MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1985, reutapm,  bbcbp,  0,     reutapm,  bbcb,   bbc_state,   init_bbc, "Acorn",           "Reuters APM",                        MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
COMP ( 1986, econx25,  bbcbp,  0,     econx25,  bbcbp,  bbc_state,   init_bbc, "Acorn",           "Econet X25 Gateway",                 MACHINE_NOT_WORKING)
COMP ( 1986, bbcm,     0,      bbcb,  bbcm,     bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master 128",                     MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcmt,    bbcm,   0,     bbcmt,    bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master Turbo",                   MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcmaiv,  bbcm,   0,     bbcmaiv,  bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master AIV",                     MACHINE_NOT_WORKING)
COMP ( 1986, bbcmet,   bbcm,   0,     bbcmet,   bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master ET",                      MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcm512,  bbcm,   0,     bbcm512,  bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master 512",                     MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcmarm,  bbcm,   0,     bbcmarm,  bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master (ARM Evaluation)",        MACHINE_NOT_WORKING)
COMP ( 1986, ltmpm,    bbcm,   0,     ltmpm,    ltmpm,  bbc_state,   init_bbc, "Lawrie T&M Ltd.", "LTM Portable (Master)",              MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcmc,    0,      bbcm,  bbcmc,    bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master Compact",                 MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1986, bbcmc_ar, bbcmc,  0,     bbcmc,    bbcm,   bbc_state,   init_bbc, "Acorn",           "BBC Master Compact (Arabic)",        MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1987, pro128s,  bbcmc,  0,     pro128s,  bbcm,   bbc_state,   init_bbc, "Olivetti",        "Prodest PC 128S",                    MACHINE_IMPERFECT_GRAPHICS)
COMP ( 1988, discmon,  bbcm,   0,     discmon,  bbcm,   bbc_state,   init_bbc, "Arbiter Leisure", "Arbiter Discmonitor A-01",           MACHINE_NOT_WORKING)
COMP ( 1988, discmate, bbcm,   0,     discmate, bbcm,   bbc_state,   init_bbc, "Arbiter Leisure", "Arbiter Discmate A-02",              MACHINE_NOT_WORKING)
//COMP ( 1988, discmast, bbcm,   0,     discmast, bbcm,   bbc_state,   init_bbc, "Arbiter Leisure", "Arbiter Discmaster A-03",            MACHINE_NOT_WORKING)
COMP ( 1989, cfa3000,  bbcm,   0,     cfa3000,  bbcm,   bbc_state,   init_bbc, "Tinsley Medical Instruments",  "Henson CFA 3000",       MACHINE_NOT_WORKING)
