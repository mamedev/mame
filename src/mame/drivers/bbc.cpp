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

    GNB14 - Model B with Disc, Econet & Speech (German export)
    UNB09 - Model B with Disc, Econet & Speech (US export)
            Model B with Disc (Norway dealer import)

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

#include "emu.h"
#include "includes/bbc.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/acorn_dsk.h"
#include "formats/fsd_dsk.h"
#include "formats/pc_dsk.h"
#include "imagedev/cassette.h"
#include "formats/uef_cas.h"
#include "formats/csw_cas.h"

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
|   &2000 RAM       |1  |   |   |
|                   |   |   |   |
+   &3000           |   |   +   +
|2                  |   |   |   |
+   &4000           +   |1  |   |
|                   |   |   |   |
|   &5000           |   |   |   |
|         LYNNE     |   |   |2  |2
|   &6000           |   |   |   |
|                   |   |   |   |
|   &7000           |   |   |   |
|                   |   |   |   |
+   &8000           +   +   +   +
|         ANDY      |   |   |   |
+   &9000           |   |   |   +
|                   |   |   |   |
|   &A000           |   |   |   |
|                   |   |   |   |
|   &B000           |   |   +   |
|                   |   |   |   |
+   &C000           +   +   +   +
|                   |   |   |   |
|   &D000 HAZEL     |   |   |   |
|                   |   |   |   |
+   &E000           |   |   |   +
|                   |   |   |   |
|   &F000           |   |   |   |
|                   |   |   |   |
+   &FC00 FRED      +   +   +   +
+   &FD00 JIM       +   +   +   +
+   &FE00 SHEILA    +   +   +   +
+   &FF00           +   +   +   +
|                   |   |   |   |
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

void bbc_state::bbca_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(bbc_state::bbc_ram_r), FUNC(bbc_state::bbc_ram_w));                                   //    0000-7fff                 Regular RAM
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbc_paged_r), FUNC(bbc_state::bbc_paged_w));                               //    8000-bfff                 Paged ROM/RAM
	map(0xc000, 0xffff).rom().region("mos", 0);                                                                       //    c000-fbff                 OS ROM
	map(0xfe00, 0xfeff).r(FUNC(bbc_state::bbc_fe_r));                                                                 //    fe00-feff                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));   //    fe00-fe07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe0f).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));                        //    fe08-fe0f  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).w(FUNC(bbc_state::serial_ula_w));                                                             //    fe10-fe17  Serial ULA     Serial system chip
	map(0xfe20, 0xfe2f).w(FUNC(bbc_state::video_ula_w));                                                              // W: fe20-fe2f  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).w(FUNC(bbc_state::bbc_romsel_w));                                                             // W: fe30-fe3f  74LS161        Paged ROM selector
	map(0xfe40, 0xfe5f).m(m_via6522_0, FUNC(via6522_device::map));                                                    //    fe40-fe5f  6522 VIA       SYSTEM VIA
}


void bbc_state::bbc_base(address_map &map)
{
	map.unmap_value_high();
	map(0xc000, 0xffff).rw(FUNC(bbc_state::bbc_mos_r), FUNC(bbc_state::bbc_mos_w));                                   //    c000-fbff                 OS ROM
	map(0xfc00, 0xfcff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));  //    fc00-fcff                 FRED Address Page
	map(0xfd00, 0xfdff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));    //    fd00-fdff                 JIM Address Page
	map(0xfe00, 0xfeff).r(FUNC(bbc_state::bbc_fe_r));                                                                 //    fe00-feff                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));   //    fe00-fe07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe0f).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));                        //    fe08-fe0f  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).w(FUNC(bbc_state::serial_ula_w));                                                             //    fe10-fe17  Serial ULA     Serial system chip
	map(0xfe18, 0xfe1f).portr("STATID");                                                                              //    fe18-fe1f  INTOFF/STATID  ECONET Interrupt Off / ID No.
	map(0xfe20, 0xfe2f).w(FUNC(bbc_state::video_ula_w));                                                              // W: fe20-fe2f  Video ULA      Video system chip
	map(0xfe40, 0xfe5f).m(m_via6522_0, FUNC(via6522_device::map));                                                    //    fe40-fe5f  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe7f).m(m_via6522_1, FUNC(via6522_device::map));                                                    //    fe60-fe7f  6522 VIA       USER VIA
	map(0xfea0, 0xfebf).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                            //    fea0-febf  68B54 ADLC     ECONET controller
	map(0xfec0, 0xfedf).rw(m_upd7002, FUNC(upd7002_device::read), FUNC(upd7002_device::write));                       //    fec0-fedf  uPD7002        Analogue to digital converter
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           //    fee0-feff  Tube ULA       Tube system interface
}


void bbc_state::bbcb_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x7fff).rw(FUNC(bbc_state::bbc_ram_r), FUNC(bbc_state::bbc_ram_w));                                   //    0000-7fff                 Regular RAM
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbc_paged_r), FUNC(bbc_state::bbc_paged_w));                               //    8000-bfff                 Paged ROM/RAM
	map(0xfe30, 0xfe3f).rw(FUNC(bbc_state::bbc_romsel_r), FUNC(bbc_state::bbc_romsel_w));                             // W: fe30-fe3f  84LS161        Paged ROM selector
	map(0xfe80, 0xfe9f).rw(m_fdc, FUNC(bbc_fdc_slot_device::read), FUNC(bbc_fdc_slot_device::write));                 //    fe80-fe9f  8271 FDC       Floppy disc controller
}


void bbcbp_state::bbcbp_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              //    0000-2fff                 Regular RAM
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              //    3000-7fff                 Video/Shadow RAM
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbcbp_paged_r), FUNC(bbc_state::bbcbp_paged_w));                           //    8000-bfff                 Paged ROM/RAM
	map(0xfe30, 0xfe3f).w(FUNC(bbc_state::bbcbp_romsel_w));                                                           // W: fe30-fe3f  84LS161        Paged ROM selector
	map(0xfe80, 0xfe83).w(FUNC(bbc_state::bbcbp_drive_control_w));                                                    //    fe80-fe83  1770 FDC       Drive control register
	map(0xfe84, 0xfe9f).rw(m_wd_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                          //    fe84-fe9f  1770 FDC       Floppy disc controller
}


void bbcbp_state::reutapm_mem(address_map &map)
{
	bbc_base(map);
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              //    0000-2fff                 Regular RAM
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              //    3000-7fff                 Video/Shadow RAM
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbcbp_paged_r), FUNC(bbc_state::bbcbp_paged_w));                           //    8000-bfff                 Paged ROM/RAM
	map(0xfe30, 0xfe3f).w(FUNC(bbc_state::bbcbp_romsel_w));                                                           // W: fe30-fe3f  84LS161        Paged ROM selector
}


void bbcbp_state::bbcbp_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbc_state::bbcbp_fetch_r));
}


void bbcm_state::bbcm_mem(address_map &map)
{
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              //    0000-2fff                 Regular RAM
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              //    3000-7fff                 Video/Shadow RAM LYNNE
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbcm_paged_r), FUNC(bbc_state::bbcm_paged_w));                             //    8000-8fff                 Paged ROM/RAM or 4K of RAM ANDY
	map(0xc000, 0xffff).rw(FUNC(bbc_state::bbc_mos_r), FUNC(bbc_state::bbc_mos_w));                                   //    c000-ffff                 OS ROM
	map(0xc000, 0xdfff).rw(FUNC(bbc_state::bbcm_hazel_r), FUNC(bbc_state::bbcm_hazel_w));                             //    c000-dfff                 OS ROM or 8K of RAM HAZEL
	map(0xfc00, 0xfeff).m(m_bankdev, FUNC(address_map_bank_device::amap8));                                           //    fc00-ffff                 OS ROM or hardware IO
}


void bbcm_state::bbcm_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbc_state::bbcm_fetch_r));
}


void bbcm_state::bbcm_bankdev(address_map &map)
{
	map.unmap_value_high();
	/* ACCCON TST bit - normal state */
	map(0x0000, 0x02ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
	map(0x0000, 0x00ff).mirror(0x400).rw(FUNC(bbc_state::bbc_fred_r), FUNC(bbc_state::bbc_fred_w));                                   //    fc00-fcff  Master         FRED Address Page
	map(0x0100, 0x01ff).mirror(0x400).rw(FUNC(bbc_state::bbc_jim_r), FUNC(bbc_state::bbc_jim_w));                                     //    fd00-fdff  Master         JIM Address Page
	map(0x0200, 0x02ff).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe00-feff                 SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));                  //    fe00-fe07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0208, 0x020f).mirror(0x400).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));                          //    fe08-fe0f  6850 ACIA      Serial controller
	map(0x0210, 0x0217).mirror(0x400).w(FUNC(bbc_state::serial_ula_w));                                                               //    fe10-fe17  Serial ULA     Serial system chip
	map(0x0218, 0x021f).mirror(0x400).rw(m_upd7002, FUNC(upd7002_device::read), FUNC(upd7002_device::write));                         //    fe18-fe1f  uPD7002        Analogue to digital converter
	map(0x0220, 0x0223).mirror(0x400).w(FUNC(bbc_state::video_ula_w));                                                                //    fe20-fe23  Video ULA      Video system chip
	map(0x0224, 0x0227).mirror(0x400).w(FUNC(bbc_state::bbcm_drive_control_w));                                                       //    fe24-fe27  FDC Latch      1770 Control latch
	map(0x0228, 0x022f).mirror(0x400).rw(m_wd_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                            //    fe28-fe2f  1770 FDC       Floppy disc controller
	map(0x0230, 0x0233).mirror(0x400).w(FUNC(bbc_state::bbcm_romsel_w));                                                              //    fe30-fe33  ROMSEL         ROM Select
	map(0x0234, 0x0237).mirror(0x400).rw(FUNC(bbc_state::bbcm_acccon_r), FUNC(bbc_state::bbcm_acccon_w));                             //    fe34-fe37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe38-fe3b  INTOFF
	map(0x023c, 0x023f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe3c-fe3f  INTON
	map(0x0240, 0x025f).mirror(0x400).m(m_via6522_0, FUNC(via6522_device::map));                                                      //    fe40-fe5f  6522 VIA       SYSTEM VIA
	map(0x0260, 0x027f).mirror(0x400).m(m_via6522_1, FUNC(via6522_device::map));                                                      //    fe60-fe7f  6522 VIA       USER VIA
	map(0x0280, 0x029f).mirror(0x400).rw(m_modem, FUNC(bbc_modem_slot_device::read), FUNC(bbc_modem_slot_device::write));             //    fe80-fe9f  Int. Modem     Int. Modem
	map(0x02a0, 0x02bf).mirror(0x400).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                              //    fea0-febf  68B54 ADLC     ECONET controller
	map(0x02e0, 0x02ff).mirror(0x400).rw(FUNC(bbc_state::bbcm_tube_r), FUNC(bbc_state::bbcm_tube_w));                                 //    fee0-feff  Tube ULA       Tube system interface
	/* ACCCON TST bit - hardware test */
	map(0x0400, 0x06ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
}


void bbcm_state::bbcmet_bankdev(address_map &map)
{
	map.unmap_value_high();
	/* ACCCON TST bit - normal state */
	map(0x0000, 0x02ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
	map(0x0000, 0x00ff).mirror(0x400).unmaprw();                                                                                      //    fc00-fcff                 FRED Address Page
	map(0x0100, 0x01ff).mirror(0x400).unmaprw();                                                                                      //    fd00-fdff                 JIM Address Page
	map(0x0200, 0x02ff).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe00-feff                 SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));                  //    fe00-fe07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0208, 0x020f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe08-fe0f  6850 ACIA      Serial controller
	map(0x0210, 0x0217).mirror(0x400).w(FUNC(bbc_state::serial_ula_w));                                                               //    fe10-fe17  Serial ULA     Serial system chip
	map(0x0218, 0x021f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe18-fe1f  uPD7002        Analogue to digital converter
	map(0x0220, 0x0223).mirror(0x400).w(FUNC(bbc_state::video_ula_w));                                                                //    fe20-fe23  Video ULA      Video system chip
	map(0x0224, 0x0227).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe24-fe27  FDC Latch      1770 Control latch
	map(0x0228, 0x022f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe28-fe2f  1770 FDC       Floppy disc controller
	map(0x0230, 0x0233).mirror(0x400).w(FUNC(bbc_state::bbcm_romsel_w));                                                              //    fe30-fe33  ROMSEL         ROM Select
	map(0x0234, 0x0237).mirror(0x400).rw(FUNC(bbc_state::bbcm_acccon_r), FUNC(bbc_state::bbcm_acccon_w));                             //    fe34-fe37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe38-fe3b  INTOFF
	map(0x023c, 0x023f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe3c-fe3f  INTON
	map(0x0240, 0x025f).mirror(0x400).m(m_via6522_0, FUNC(via6522_device::map));                                                      //    fe40-fe5f  6522 VIA       SYSTEM VIA
	map(0x0260, 0x027f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe60-fe7f  6522 VIA       USER VIA
	map(0x0280, 0x029f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe80-fe9f  Int. Modem     Int. Modem
	map(0x02a0, 0x02bf).mirror(0x400).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                              //    fea0-febf  68B54 ADLC     ECONET controller
	map(0x02e0, 0x02ff).mirror(0x400).rw(FUNC(bbc_state::bbcm_tube_r), FUNC(bbc_state::bbcm_tube_w));                                 //    fee0-feff  Tube ULA       Tube system interface
	/* ACCCON TST bit - hardware test */
	map(0x0400, 0x06ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
}


void bbcm_state::bbcmc_mem(address_map &map)
{
	map(0x0000, 0x2fff).bankrw("bank1");                                                                              //    0000-2fff                 Regular RAM
	map(0x3000, 0x7fff).bankrw("bank2");                                                                              //    3000-7fff                 Video/Shadow RAM LYNNE
	map(0x8000, 0xbfff).rw(FUNC(bbc_state::bbcmc_paged_r), FUNC(bbc_state::bbcmc_paged_w));                           //    8000-8fff                 Paged ROM/RAM or 4K of RAM ANDY
	map(0xc000, 0xffff).rw(FUNC(bbc_state::bbc_mos_r), FUNC(bbc_state::bbc_mos_w));                                   //    c000-ffff                 OS ROM
	map(0xc000, 0xdfff).rw(FUNC(bbc_state::bbcm_hazel_r), FUNC(bbc_state::bbcm_hazel_w));                             //    c000-dfff                 OS ROM or 8K of RAM HAZEL
	map(0xfc00, 0xfeff).m(m_bankdev, FUNC(address_map_bank_device::amap8));                                           //    fc00-ffff                 OS ROM or hardware IO
}


void bbcm_state::bbcmc_bankdev(address_map &map)
{
	map.unmap_value_high();
	/* ACCCON TST bit - normal state */
	map(0x0000, 0x02ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
	map(0x0000, 0x00ff).mirror(0x400).rw(m_exp, FUNC(bbc_exp_slot_device::fred_r), FUNC(bbc_exp_slot_device::fred_w));                //    fc00-fcff  Compact        FRED Address Page
	map(0x0100, 0x01ff).mirror(0x400).rw(m_exp, FUNC(bbc_exp_slot_device::jim_r), FUNC(bbc_exp_slot_device::jim_w));                  //    fd00-fdff  Compact        JIM Address Page
	map(0x0200, 0x02ff).mirror(0x400).rw(m_exp, FUNC(bbc_exp_slot_device::sheila_r), FUNC(bbc_exp_slot_device::sheila_w));            //    fd00-fdff  Compact        SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));                  //    fe00-fe07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x406).rw(m_hd6845, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0208, 0x020f).mirror(0x400).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));                          //    fe08-fe0f  6850 ACIA      Serial controller
	map(0x0210, 0x0217).mirror(0x400).w(FUNC(bbc_state::serial_ula_w));                                                               //    fe10-fe17  Serial ULA     Serial system chip
	map(0x0220, 0x0223).mirror(0x400).w(FUNC(bbc_state::video_ula_w));                                                                //    fe20-fe23  Video ULA      Video system chip
	map(0x0224, 0x0227).mirror(0x400).w(FUNC(bbc_state::bbcm_drive_control_w));                                                       //    fe24-fe27  FDC Latch      1772 Control latch
	map(0x0228, 0x022f).mirror(0x400).rw(m_wd_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write));                            //    fe28-fe2f  1772 FDC       Floppy disc controller
	map(0x0230, 0x0233).mirror(0x400).w(FUNC(bbc_state::bbcm_romsel_w));                                                              //    fe30-fe33  ROMSEL         ROM Select
	map(0x0234, 0x0237).mirror(0x400).rw(FUNC(bbc_state::bbcm_acccon_r), FUNC(bbc_state::bbcm_acccon_w));                             //    fe34-fe37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe38-fe3b  INTOFF
	map(0x023c, 0x023f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe3c-fe3f  INTON
	map(0x0240, 0x025f).mirror(0x400).m(m_via6522_0, FUNC(via6522_device::map));                                                      //    fe40-fe5f  6522 VIA       SYSTEM VIA
	map(0x0260, 0x027f).mirror(0x400).m(m_via6522_1, FUNC(via6522_device::map));                                                      //    fe60-fe7f  6522 VIA       USER VIA
	map(0x0280, 0x029f).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fe80-fe9f  Int. Modem     Int. Modem
	map(0x02a0, 0x02bf).mirror(0x400).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                              //    fea0-febf  68B54 ADLC     ECONET controller
	map(0x02e0, 0x02ff).mirror(0x400).r(FUNC(bbc_state::bbc_fe_r));                                                                   //    fee0-feff  Tube ULA       Tube system interface
	/* ACCCON TST bit - hardware test */
	map(0x0400, 0x06ff).rom().region("mos", 0x3c00);                                                                                  //    fc00-ffff                 OS ROM (continued)
}


void bbcm_state::autoc15_bankdev(address_map &map)
{
	bbcmc_bankdev(map);
	map(0x0200, 0x0200).mirror(0x406).rw(m_hd6845, FUNC(hd6345_device::status_r), FUNC(hd6345_device::address_w));                    //    fe00-fe07  6345 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x406).rw(m_hd6845, FUNC(hd6345_device::register_r), FUNC(hd6345_device::register_w));
}


INPUT_CHANGED_MEMBER(bbc_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (!newval)
	{
		if (m_via6522_1) m_via6522_1->reset();
		if (m_adlc) m_adlc->reset();
		if (m_rtc) m_rtc->reset();
		if (m_fdc) m_fdc->reset();
		if (m_wd_fdc) m_wd_fdc->reset();
		if (m_1mhzbus) m_1mhzbus->reset();
		if (m_tube) m_tube->reset();
		if (m_intube) m_intube->reset();
		if (m_extube) m_extube->reset();
		if (m_internal) m_internal->reset();
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
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $")                PORT_CODE(KEYCODE_4)            PORT_CHAR('4')      PORT_CHAR('$')
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
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '")                PORT_CODE(KEYCODE_7)            PORT_CHAR('7')      PORT_CHAR('\'')
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
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"_ £")              PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('_') PORT_CHAR(0xA3)
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

	/* Keyboard columns 13 -> 14 are reserved for Torch */
	PORT_START("COL13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	/* Keyboard column 15 not known to be used */
	PORT_START("COL15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START(bbc_keyboard_no)
	PORT_INCLUDE(bbc_keyboard)

	PORT_MODIFY("COL7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Å")                PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR(0xc5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ø")                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(0xd8)

	PORT_MODIFY("COL8")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *")                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Æ")                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(0xc6)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +")                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(';') PORT_CHAR('+')

	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@")                  PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('@')
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


static INPUT_PORTS_START(autoc15)
	PORT_MODIFY("COL0")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INFO")               PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT COL")           PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')

	PORT_MODIFY("COL1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SETUP")              PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_MODIFY("COL2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("XFER")               PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_MODIFY("COL3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MOVE")               PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_MODIFY("COL4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FIND")               PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEF")                PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_MODIFY("COL5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FORM")               PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_MODIFY("COL6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COUNT")              PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_MODIFY("COL7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CUE")                PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_MODIFY("COL10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PARA")               PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL SHOT")           PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS CHAR")           PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS COL")            PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL MARK")           PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEXT MATCH")         PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS/OVER")           PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_MODIFY("COL11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL CHAR")           PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL COL")            PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS SHOT")           PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GOTO")               PORT_CODE(KEYCODE_BS_PAD)       PORT_CHAR(UCHAR_MAMEKEY(BS_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INS MARK")           PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CMD")                PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MAR REL")            PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))

	PORT_MODIFY("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("END")                PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                 PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("APPLY")              PORT_CODE(KEYCODE_COMMA_PAD)    PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FONT")               PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE STYLE")         PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
INPUT_PORTS_END


static INPUT_PORTS_START(torchb_keyboard)
	PORT_INCLUDE(bbc_keyboard)

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


static INPUT_PORTS_START(torchi_keyboard)
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift")              PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")                PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Begin")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Word")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Window")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Para")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")          PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")                PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl")               PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("And")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Screen")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("File")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0")                 PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt")                PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insert")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Undo")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Underline")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Redo")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")                 PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Exact Space")

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !")                PORT_CODE(KEYCODE_1)            PORT_CHAR('1')      PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad -")           PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")                  PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")                  PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")                  PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")                 PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"")               PORT_CODE(KEYCODE_2)            PORT_CHAR('2')      PORT_CHAR('\"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 0")           PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")                  PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")                  PORT_CODE(KEYCODE_S)            PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")                  PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")                 PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"3 £")              PORT_CODE(KEYCODE_3)            PORT_CHAR('3')      PORT_CHAR(0xA3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad +")           PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")                  PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")                  PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")                  PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")                 PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $")                PORT_CODE(KEYCODE_4)            PORT_CHAR('4')      PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1")           PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")                  PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")                  PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")                  PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")                 PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %")                PORT_CODE(KEYCODE_5)            PORT_CHAR('5')      PORT_CHAR('%')//
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2")           PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")                  PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")                  PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")                  PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6")                 PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)            PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^")                PORT_CODE(KEYCODE_6)            PORT_CHAR('6')      PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3")           PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")                  PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")                  PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")                  PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7")                 PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)              PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &")                PORT_CODE(KEYCODE_7)            PORT_CHAR('7')      PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 4")           PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")                  PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")                  PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")                  PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8")                 PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))//PORT_NAME("COPY")               PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Move <-")

	PORT_START("COL10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *")                PORT_CODE(KEYCODE_8)            PORT_CHAR('8')      PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5")           PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <")                PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')      PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")                  PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")                  PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F9")                 PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Move Past")

	PORT_START("COL11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (")                PORT_CODE(KEYCODE_9)            PORT_CHAR('9')      PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 6")           PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT)   PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >")                PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')      PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")                  PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")                  PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F10")                PORT_CODE(KEYCODE_F11)          PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Move ->")

	PORT_START("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )")                PORT_CODE(KEYCODE_0)            PORT_CHAR('0')      PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 7")           PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?")                PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')      PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :")                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')      PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")                  PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")               PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del <-")             PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("COL13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- _")                PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 8")           PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"# £")              PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('#')      PORT_CHAR(0xA3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("' @")                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'')     PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {")                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')      PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)            PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Line")           PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("COL14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= +")                PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=')      PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 9")           PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |")               PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('\\')     PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }")                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')      PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)           PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del ->")

	/* Keyboard column 15 not known to be used */
	PORT_START("COL15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("BRK")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK")              PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, trigger_reset, 0)
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
	PORT_DIPSETTING(   0x37,  "55" )    PORT_DIPSETTING(   0x38,  "56" )    PORT_DIPSETTING(   0x39,  "57" )    PORT_DIPSETTING(   0x3a,  "58" )    PORT_DIPSETTING(   0x3b,  "59" )
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
	PORT_DIPSETTING(   0x37,  "55" )    PORT_DIPSETTING(   0x38,  "56" )    PORT_DIPSETTING(   0x39,  "57" )    PORT_DIPSETTING(   0x3a,  "58" )    PORT_DIPSETTING(   0x3b,  "59" )
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


INPUT_CHANGED_MEMBER(bbc_state::reset_palette)
{
	m_vnula.disable = !BIT(m_bbcconfig.read_safe(0), 3);
	if (m_vnula.disable)
		update_palette((monitor_type)(m_bbcconfig.read_safe(0) & 0x03));
	else
		update_palette(monitor_type::COLOUR);
}


static INPUT_PORTS_START(bbc_config)
	PORT_START("BBCCONFIG")
	PORT_CONFNAME( 0x03, 0x00, "Monitor") PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, reset_palette, 0) PORT_CONDITION("BBCCONFIG", 0x08, EQUALS, 0x00)
	PORT_CONFSETTING(    0x00, "Colour")
	PORT_CONFSETTING(    0x01, "B&W")
	PORT_CONFSETTING(    0x02, "Green")
	PORT_CONFSETTING(    0x03, "Amber")
	PORT_CONFNAME( 0x04, 0x00, "Econet")
	PORT_CONFSETTING(    0x00, DEF_STR( No ))
	PORT_CONFSETTING(    0x04, DEF_STR( Yes ))
	PORT_CONFNAME( 0x08, 0x00, "VideoNuLA") PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_state, reset_palette, 0)
	PORT_CONFSETTING(    0x00, DEF_STR( No ))
	PORT_CONFSETTING(    0x08, DEF_STR( Yes ))
INPUT_PORTS_END


static INPUT_PORTS_START(bbca)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcb)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcb_links)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcb_no)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard_no)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcb_links)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcbp)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(bbc_dipswitch)
	PORT_INCLUDE(bbcbp_links)
INPUT_PORTS_END

static INPUT_PORTS_START(torchb)
	PORT_INCLUDE(torchb_keyboard)
	PORT_INCLUDE(bbcb_links)
INPUT_PORTS_END

static INPUT_PORTS_START(torchi)
	PORT_INCLUDE(torchi_keyboard)
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

static INPUT_PORTS_START(autoc)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_keyboard)
	PORT_INCLUDE(autoc15)
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


void bbc_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();

	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_ACORN_DOS_FORMAT);
	fr.add(FLOPPY_OPUS_DDOS_FORMAT);
	fr.add(FLOPPY_OPUS_DDCPM_FORMAT);
	fr.add(FLOPPY_FSD_FORMAT);
}

static void bbc_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525qd",   FLOPPY_525_QD);
	device.option_add("35dd",    FLOPPY_35_DD);
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
	update_nmi();
}


/***************************************************************************

    BBC Micro

****************************************************************************/


void bbc_state::bbca(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_state::bbca_mem);
	m_maincpu->set_periodic_int(FUNC(bbc_state::bbcb_keyscan), attotime::from_hz(1000)); /* scan keyboard */
	config.set_maximum_quantum(attotime::from_hz(60));

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	/* addressable latch */
	LS259(config, m_latch);
	m_latch->q_out_cb<0>().set(FUNC(bbc_state::snd_enable_w));
	m_latch->q_out_cb<3>().set(FUNC(bbc_state::kbd_enable_w));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	/* internal ram */
	RAM(config, m_ram).set_default_size("16K").set_extra_options("32K").set_default_value(0xff);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("hd6845", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);
	m_trom->set_screen_size(40, 25, 40);

	/* crtc */
	HD6845S(config, m_hd6845, 16_MHz_XTAL / 8);
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(12);
	//m_hd6845->set_begin_update_callback(FUNC(bbc_state::crtc_begin_update));
	m_hd6845->set_update_row_callback(FUNC(bbc_state::crtc_update_row));
	//m_hd6845->set_reconfigure_callback(FUNC(bbc_state::crtc_reconfig));
	m_hd6845->out_de_callback().set(FUNC(bbc_state::bbc_de_changed));
	m_hd6845->out_hsync_callback().set(FUNC(bbc_state::bbc_hsync_changed));
	m_hd6845->out_vsync_callback().set(FUNC(bbc_state::bbc_vsync_changed));

	config.set_default_layout(layout_bbc);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* cassette relay */
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(bbc_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(bbc_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED);
	m_cassette->set_interface("bbc_cass");

	/* acia */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set(FUNC(bbc_state::write_txd));
	m_acia->rts_handler().set(FUNC(bbc_state::write_rts));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(bbc_state::write_rxd));
	m_rs232->dcd_handler().set(FUNC(bbc_state::write_dcd));
	m_rs232->cts_handler().set(FUNC(bbc_state::write_cts));

	CLOCK(config, m_acia_clock, 16_MHz_XTAL / 13);
	m_acia_clock->signal_handler().set(FUNC(bbc_state::write_acia_clock));

	/* system via */
	MOS6522(config, m_via6522_0, 16_MHz_XTAL / 16);
	m_via6522_0->readpa_handler().set(FUNC(bbc_state::via_system_porta_r));
	m_via6522_0->readpb_handler().set(FUNC(bbc_state::via_system_portb_r));
	m_via6522_0->writepa_handler().set(FUNC(bbc_state::via_system_porta_w));
	m_via6522_0->writepb_handler().set(FUNC(bbc_state::via_system_portb_w));
	m_via6522_0->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	/* eprom sockets */
	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, nullptr); /* ic101 */
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, nullptr); /* ic100 */
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr); /* ic88 */
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, nullptr); /* ic52 */

	/* software lists */
	SOFTWARE_LIST(config, "cass_ls").set_original("bbc_cass").set_filter("A");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("B");
}


void bbc_state::bbcb(machine_config &config)
{
	bbca(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_state::bbcb_mem);

	/* addressable latch */
	m_latch->q_out_cb<1>().set(FUNC(bbc_state::speech_rsq_w));
	m_latch->q_out_cb<2>().set(FUNC(bbc_state::speech_wsq_w));

	/* internal ram */
	m_ram->set_default_size("32K");

	/* speech hardware */
	SPEECHROM(config, "vsm", 0);
	TMS5220(config, m_tms, 640000);
	m_tms->set_speechrom_tag("vsm");
	m_tms->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* user via */
	MOS6522(config, m_via6522_1, 16_MHz_XTAL / 16);
	m_via6522_1->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via6522_1->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via6522_1->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via6522_1->writepb_handler().append(m_internal, FUNC(bbc_internal_slot_device::latch_fe60_w));
	m_via6522_1->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_via6522_1->cb1_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb1));
	m_via6522_1->cb2_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb2));
	m_via6522_1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	/* adc */
	UPD7002(config, m_upd7002, 16_MHz_XTAL / 16);
	m_upd7002->set_get_analogue_callback(FUNC(bbc_state::get_analogue_input));
	m_upd7002->set_eoc_callback(FUNC(bbc_state::upd7002_eoc));

	/* printer */
	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via6522_1, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	/* fdc */
	BBC_FDC_SLOT(config, m_fdc, 16_MHz_XTAL / 2, bbc_fdc_devices, "acorn8271");
	m_fdc->intrq_wr_callback().set(FUNC(bbc_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbc_state::fdc_drq_w));

	/* econet */
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("econet", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbc_state::adlc_irq_w));

	econet_device &econet(ECONET(config, "econet", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", "econet", econet_devices).set_slot(254);

	/* analogue port */
	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set(FUNC(bbc_state::lpstb_w));

	/* 1mhz bus port */
	BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	/* tube port */
	BBC_TUBE_SLOT(config, m_tube, bbc_tube_devices, nullptr);
	m_tube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_via6522_1, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_via6522_1, FUNC(via6522_device::write_cb2));

	/* internal expansion boards */
	BBC_INTERNAL_SLOT(config, m_internal, 16_MHz_XTAL, bbcb_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
	m_internal->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	m_irqs->output_handler().append(m_internal, FUNC(bbc_internal_slot_device::irq6502_w));

	/* software lists */
	subdevice<software_list_device>("cass_ls")->set_filter("A,B");
	SOFTWARE_LIST(config, "flop_ls_b").set_original("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_original("bbcb_flop_orig");
	SOFTWARE_LIST(config, "hdd_ls").set_original("bbc_hdd").set_filter("B");
}


void bbc_state::bbcb_de(machine_config &config)
{
	bbcb(config);

	/* fdc */
	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);
}


void bbc_state::bbcb_us(machine_config &config)
{
	bbcb(config);

	/* video hardware */
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 262, 0, 200);

	/* fdc */
	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_b_us").set_original("bbcb_flop_us");
}


/***************************************************************************

    Cisco Systems

****************************************************************************/


void bbc_state::sist1(machine_config &config)
{
	bbcb(config);

	m_1mhzbus->set_default_option("cisco");
	m_1mhzbus->set_fixed(true);
}


/***************************************************************************

    Torch Computers

****************************************************************************/


void torch_state::torchf(machine_config &config)
{
	bbcb(config);

	/* fdc */
	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);

	/* Torch Z80 Communicator co-processor */
	m_tube->set_default_option("zep100");
	m_tube->set_fixed(true);
}


void torch_state::torchh(machine_config &config)
{
	torchf(config);

	/* fdc */
	//m_fdc->subdevice<floppy_connector>("acorn8271:i8271:1")->set_default_option(nullptr);

	/* 10MB or 21MB HDD */
	m_1mhzbus->set_default_option("torchhd");
	m_1mhzbus->set_fixed(true);
}


void torch_state::torch301(machine_config &config)
{
	torchf(config);

	/* fdc */
	//m_fdc->subdevice<floppy_connector>("acorn8271:i8271:1")->set_default_option(nullptr);

	/* Torch Z80 Communicator co-processor */
	m_tube->set_default_option("zep100");
	m_tube->set_fixed(true);
	m_tube->set_insert_rom(false);

	/* 20MB HDD */
	m_1mhzbus->set_default_option("torchhd");
	m_1mhzbus->set_fixed(true);
}


void torch_state::torch725(machine_config &config)
{
	torchf(config);

	/* fdc */
	//m_fdc->subdevice<floppy_connector>("acorn8271:i8271:1")->set_default_option(nullptr);

	/* Torch 68000 Atlas co-processor */
	//m_tube->set_default_option("atlas");
	m_tube->set_fixed(true);
	m_tube->set_insert_rom(false);

	/* 20MB HDD */
	m_1mhzbus->set_default_option("torchhd");
	m_1mhzbus->set_fixed(true);
}


/***************************************************************************

    BBC Model B+

****************************************************************************/


void bbcbp_state::bbcbp(machine_config &config)
{
	bbcb(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcbp_state::bbcbp_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcbp_state::bbcbp_fetch);

	/* internal ram */
	m_ram->set_default_size("64K");

	/* fdc */
	WD1770(config, m_wd_fdc, 16_MHz_XTAL / 2);
	m_wd_fdc->set_force_ready(true);
	m_wd_fdc->intrq_wr_callback().set(FUNC(bbc_state::fdc_intrq_w));
	m_wd_fdc->drq_wr_callback().set(FUNC(bbc_state::fdc_drq_w));
	config.device_remove("fdc");

	FLOPPY_CONNECTOR(config, "wd_fdc:0", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wd_fdc:1", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);

	/* remove sockets not present in B+ */
	config.device_remove("romslot0");
	config.device_remove("romslot1");
	config.device_remove("romslot2");
	config.device_remove("romslot3");

	/* eprom sockets */
	BBC_ROMSLOT32(config, m_rom[0x02], bbc_rom_devices, nullptr); /* ic35 32K socket */
	BBC_ROMSLOT32(config, m_rom[0x04], bbc_rom_devices, nullptr); /* ic44 32K socket */
	BBC_ROMSLOT32(config, m_rom[0x06], bbc_rom_devices, nullptr); /* ic57 32K socket */
	BBC_ROMSLOT32(config, m_rom[0x08], bbc_rom_devices, nullptr); /* ic62 32K socket */
	BBC_ROMSLOT32(config, m_rom[0x0a], bbc_rom_devices, nullptr); /* ic68 32K socket */

	/* internal expansion boards */
	BBC_INTERNAL_SLOT(config.replace(), m_internal, 16_MHz_XTAL, bbcbp_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
	m_internal->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	/* software lists */
	subdevice<software_list_device>("rom_ls")->set_filter("B+");
}


void bbcbp_state::bbcbp128(machine_config &config)
{
	bbcbp(config);

	/* internal ram */
	m_ram->set_default_size("128K");

	/* sideways RAM banks */
	BBC_ROMSLOT32(config, m_rom[0x00], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT32(config, m_rom[0x0c], bbc_rom_devices, "ram").set_fixed_ram(true);
}


void bbcbp_state::cfa3000bp(machine_config &config)
{
	bbcbp(config);

	/* fdc */
	m_wd_fdc->subdevice<floppy_connector>("0")->set_default_option(nullptr);
	m_wd_fdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	/* keyboard */
	m_userport->set_default_option("cfa3000kbd");
	m_userport->set_fixed(true);

	/* option board */
	m_1mhzbus->set_default_option("cfa3000opt");
	m_1mhzbus->set_fixed(true);

	/* analogue dials/sensors */
	m_analog->set_default_option("cfa3000a");
	m_analog->set_fixed(true);

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


/***************************************************************************

    Acorn Business Computers

****************************************************************************/


void bbcbp_state::abc110(machine_config &config)
{
	bbcbp(config);
	/* fdc */
	m_wd_fdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	/* Acorn Z80 co-processor */
	m_tube->set_default_option("z80w");
	m_tube->set_fixed(true);

	/* Acorn Winchester Disc 10MB */
	m_1mhzbus->set_default_option("awhd");
	m_1mhzbus->set_fixed(true);
}


void bbcbp_state::acw443(machine_config &config)
{
	bbcbp(config);
	/* fdc */
	m_wd_fdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	/* 32016 co-processor */
	m_tube->set_default_option("32016l");
	m_tube->set_fixed(true);

	/* Acorn Winchester Disc 20MB */
	m_1mhzbus->set_default_option("awhd");
	m_1mhzbus->set_fixed(true);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_32016").set_original("bbc_flop_32016");
}


void bbcbp_state::abc310(machine_config &config)
{
	bbcbp(config);
	/* fdc */
	m_wd_fdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	/* Acorn 80286 co-processor */
	m_tube->set_default_option("80286");
	m_tube->set_fixed(true);

	/* Acorn Winchester Disc 10MB */
	m_1mhzbus->set_default_option("awhd");
	m_1mhzbus->set_fixed(true);

	/* Acorn Mouse */
	m_userport->set_default_option("m512mouse");
}


/***************************************************************************

    Reuters APM Board (Application Processor Module)

****************************************************************************/


void bbcbp_state::reutapm(machine_config &config)
{
	bbcbp(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcbp_state::reutapm_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcbp_state::bbcbp_fetch);

	/* addressable latch */
	m_latch->q_out_cb<1>().set_nop();
	m_latch->q_out_cb<2>().set_nop();

	/* sound hardware */
	config.device_remove("mono");
	config.device_remove("sn76489");
	config.device_remove("samples");
	config.device_remove("vsm");
	config.device_remove("tms5220");

	/* cassette */
	config.device_remove("cassette");

	/* fdc */
	config.device_remove("wd_fdc");

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");

	/* expansion ports */
	config.device_remove("analogue");
}


/***************************************************************************

    Econet X25 Gateway

****************************************************************************/


void bbcbp_state::econx25(machine_config &config)
{
	bbcbp(config);
	/* sound hardware */
	config.device_remove("vsm");
	config.device_remove("tms5220");

	/* addressable latch */
	m_latch->q_out_cb<1>().set_nop();
	m_latch->q_out_cb<2>().set_nop();

	/* fdc */
	//config.device_remove("wd_fdc")

	/* Add Econet X25 Gateway co-processor */
	m_tube->set_default_option("x25");
	m_tube->set_fixed(true);

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


/***************************************************************************

    BBC Master Series

****************************************************************************/


void bbcm_state::bbcm(machine_config &config)
{
	/* basic machine hardware */
	M65SC02(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcm_state::bbcm_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcm_state::bbcm_fetch);
	m_maincpu->set_periodic_int(FUNC(bbc_state::bbcb_keyscan), attotime::from_hz(1000)); /* scan keyboard */
	config.set_maximum_quantum(attotime::from_hz(60));

	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&bbcm_state::bbcm_bankdev).set_options(ENDIANNESS_LITTLE, 8, 16, 0x0400);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	/* addressable latch */
	LS259(config, m_latch);
	m_latch->q_out_cb<0>().set(FUNC(bbc_state::snd_enable_w));
	m_latch->q_out_cb<3>().set(FUNC(bbc_state::kbd_enable_w));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	/* internal ram */
	RAM(config, m_ram).set_default_size("128K").set_default_value(0xff);

	config.set_default_layout(layout_bbcm);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("hd6845", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);
	m_trom->set_screen_size(40, 25, 40);

	/* crtc */
	HD6845S(config, m_hd6845, 16_MHz_XTAL / 8);
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(12);
	//m_hd6845->set_begin_update_callback(FUNC(bbc_state::crtc_begin_update));
	m_hd6845->set_update_row_callback(FUNC(bbc_state::crtc_update_row));
	//m_hd6845->set_reconfigure_callback(FUNC(bbc_state::crtc_reconfig));
	m_hd6845->out_de_callback().set(FUNC(bbc_state::bbc_de_changed));
	m_hd6845->out_hsync_callback().set(FUNC(bbc_state::bbc_hsync_changed));
	m_hd6845->out_vsync_callback().set(FUNC(bbc_state::bbc_vsync_changed));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* cassette relay */
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(bbc_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* rtc and cmos */
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_irqs, FUNC(input_merger_device::in_w<7>));

	/* printer */
	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via6522_1, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(bbc_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED);
	m_cassette->set_interface("bbc_cass");

	/* acia */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set(FUNC(bbc_state::write_txd));
	m_acia->rts_handler().set(FUNC(bbc_state::write_rts));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(bbc_state::write_rxd));
	m_rs232->dcd_handler().set(FUNC(bbc_state::write_dcd));
	m_rs232->cts_handler().set(FUNC(bbc_state::write_cts));

	CLOCK(config, m_acia_clock, 16_MHz_XTAL / 13);
	m_acia_clock->signal_handler().set(FUNC(bbc_state::write_acia_clock));

	/* adc */
	UPD7002(config, m_upd7002, 16_MHz_XTAL / 16);
	m_upd7002->set_get_analogue_callback(FUNC(bbc_state::get_analogue_input));
	m_upd7002->set_eoc_callback(FUNC(bbc_state::upd7002_eoc));

	/* system via */
	MOS6522(config, m_via6522_0, 16_MHz_XTAL / 16);
	m_via6522_0->readpa_handler().set(FUNC(bbc_state::via_system_porta_r));
	m_via6522_0->readpb_handler().set(FUNC(bbc_state::via_system_portb_r));
	m_via6522_0->writepa_handler().set(FUNC(bbc_state::via_system_porta_w));
	m_via6522_0->writepb_handler().set(FUNC(bbc_state::via_system_portb_w));
	m_via6522_0->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	/* user via */
	MOS6522(config, m_via6522_1, 16_MHz_XTAL / 16);
	m_via6522_1->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via6522_1->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via6522_1->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via6522_1->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_via6522_1->cb1_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb1));
	m_via6522_1->cb2_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb2));
	m_via6522_1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	/* fdc */
	WD1770(config, m_wd_fdc, 16_MHz_XTAL / 2);
	m_wd_fdc->set_force_ready(true);
	m_wd_fdc->intrq_wr_callback().set(FUNC(bbc_state::fdc_intrq_w));
	m_wd_fdc->drq_wr_callback().set(FUNC(bbc_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, "wd_fdc:0", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wd_fdc:1", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);

	/* econet */
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("econet", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbc_state::adlc_irq_w));

	econet_device &econet(ECONET(config, "econet", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", "econet", econet_devices).set_slot(254);

	/* analogue port */
	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);

	/* 1mhz bus port */
	BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbcm_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	/* tube ports */
	BBC_TUBE_SLOT(config, m_intube, bbc_intube_devices, nullptr);
	m_intube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));
	BBC_TUBE_SLOT(config, m_extube, bbc_extube_devices, nullptr);
	m_extube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_via6522_1, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_via6522_1, FUNC(via6522_device::write_cb2));

	/* cartridge slots */
	BBCM_CARTSLOT(config, m_cart[0], 16_MHz_XTAL, bbcm_cart, nullptr);
	m_cart[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<6>));
	m_cart[0]->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));
	BBCM_CARTSLOT(config, m_cart[1], 16_MHz_XTAL, bbcm_cart, nullptr);
	m_cart[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<7>));
	m_cart[1]->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	/* eprom sockets */
	BBC_ROMSLOT16(config, m_rom[0x08], bbc_rom_devices, nullptr); /* ic27 */
	BBC_ROMSLOT32(config, m_rom[0x04], bbc_rom_devices, "ram").set_fixed_ram(true); /* ic41 32K socket */
	BBC_ROMSLOT32(config, m_rom[0x06], bbc_rom_devices, "ram").set_fixed_ram(true); /* ic37 32K socket */

	/* internal expansion boards */
	BBC_INTERNAL_SLOT(config, m_internal, 16_MHz_XTAL, bbcm_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<8>));
	m_internal->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	/* internal modem port */
	BBC_MODEM_SLOT(config, m_modem, 16_MHz_XTAL / 16, bbcm_modem_devices, nullptr);
	m_modem->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<9>));

	/* software lists */
	SOFTWARE_LIST(config, "cass_ls").set_original("bbc_cass").set_filter("A,B,M");
	SOFTWARE_LIST(config, "flop_ls_m").set_original("bbcm_flop");
	SOFTWARE_LIST(config, "cart_ls_m").set_original("bbcm_cart");
	SOFTWARE_LIST(config, "flop_ls_b").set_compatible("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_compatible("bbcb_flop_orig");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("M");
	SOFTWARE_LIST(config, "hdd_ls").set_original("bbc_hdd").set_filter("M");
}


void bbcm_state::bbcmt(machine_config &config)
{
	bbcm(config);
	/* Acorn 65C102 co-processor */
	m_intube->set_default_option("65c102");
	m_intube->set_fixed(true);
}


void bbcm_state::bbcmaiv(machine_config &config)
{
	bbcm(config);
	/* Acorn 65C102 co-processor */
	m_intube->set_default_option("65c102");
	m_intube->set_fixed(true);

	/* Philips VP415 Laserdisc player */
	m_modem->set_default_option("scsiaiv");
	m_modem->set_fixed(true);

	/* Acorn Tracker Ball */
	m_userport->set_default_option("tracker");
}


void bbcm_state::bbcmet(machine_config &config)
{
	bbcm(config);

	m_bankdev->set_map(&bbcm_state::bbcmet_bankdev).set_options(ENDIANNESS_LITTLE, 8, 16, 0x0400);

	/* printer */
	config.device_remove("printer");
	config.device_remove("cent_data_out");

	/* cassette */
	config.device_remove("cassette");

	/* eprom sockets */
	config.device_remove("romslot4");
	config.device_remove("romslot6");

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");

	/* acia */
	config.device_remove("acia6850");
	config.device_remove("rs423");
	config.device_remove("acia_clock");

	/* devices */
	config.device_remove("upd7002");
	config.device_remove("via6522_1");

	/* fdc */
	config.device_remove("wd_fdc");

	/* expansion ports (not fitted) */
	config.device_remove("analogue");
	config.device_remove("extube");
	config.device_remove("1mhzbus");
	config.device_remove("userport");
	config.device_remove("modem");
}


void bbcm_state::bbcm512(machine_config &config)
{
	bbcm(config);
	/* Acorn Intel 80186 co-processor */
	m_intube->set_default_option("80186");
	m_intube->set_fixed(true);

	/* Acorn Mouse */
	m_userport->set_default_option("m512mouse");
}


void bbcm_state::bbcmarm(machine_config &config)
{
	bbcm(config);
	/* Acorn ARM co-processor */
	m_extube->set_default_option("arm");
	m_extube->set_fixed(true);

	/* Acorn Winchester Disc */
	m_1mhzbus->set_default_option("awhd");
}


void bbcm_state::daisy(machine_config &config)
{
	bbcm(config);
	/* Acorn 65C102 co-processor */
	m_intube->set_default_option("65c102");
	m_intube->set_fixed(true);

	/* lk18 and lk19 are set to enable rom, disabling ram */
	m_rom[0x04]->set_default_option(nullptr);
	m_rom[0x04]->set_fixed_ram(false);
	m_rom[0x06]->set_default_option(nullptr);
	m_rom[0x06]->set_fixed_ram(false);
}


void bbcm_state::discmon(machine_config &config)
{
	bbcm(config);
	/* Add coin slot */

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


void bbcm_state::discmate(machine_config &config)
{
	bbcm(config);
	/* Add Sony CDK-3000PII Auto Disc Loader */

	/* Add interface boards connected to cassette and RS423 */

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


void bbcm_state::mpc_prisma_default(device_t* device)
{
	device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus")->set_default_option("awhd");
	device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus")->set_fixed(true);
}


void bbcm_state::mpc800(machine_config& config)
{
	bbcm(config);
	/* Acorn 65C102 co-processor */
	m_intube->set_default_option("65c102");
	m_intube->set_fixed(true);

	/* Prisma 2 */
	//m_1mhzbus->set_default_option("prisma2");
	//m_1mhzbus->set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma2", mpc_prisma_default);

	/* Mouse (AMX compatible) */
	m_userport->set_default_option("amxmouse");
	m_userport->set_fixed(true);
}


void bbcm_state::mpc900(machine_config& config)
{
	mpc800(config);
	/* Prisma 3 */
	//m_1mhzbus->set_default_option("prisma3");
	//m_1mhzbus->set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma3", mpc_prisma_default);
}


void bbcm_state::mpc900gx(machine_config& config)
{
	mpc800(config);
	/* Prisma 3+ */
	//m_1mhzbus->set_default_option("prisma3p");
	//m_1mhzbus->set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma3p", mpc_prisma_default);
}


void bbcm_state::cfa3000(machine_config &config)
{
	bbcm(config);

	/* fdc */
	m_wd_fdc->subdevice<floppy_connector>("0")->set_default_option(nullptr);
	m_wd_fdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	/* lk18 and lk19 are set to enable rom, disabling ram */
	m_rom[0x04]->set_default_option(nullptr);
	m_rom[0x04]->set_fixed_ram(false);
	m_rom[0x06]->set_default_option(nullptr);
	m_rom[0x06]->set_fixed_ram(false);

	/* keyboard */
	m_userport->set_default_option("cfa3000kbd");
	m_userport->set_fixed(true);

	/* option board */
	m_1mhzbus->set_default_option("cfa3000opt");
	m_1mhzbus->set_fixed(true);

	/* analogue dials/sensors */
	m_analog->set_default_option("cfa3000a");
	m_analog->set_fixed(true);

	/* software lists */
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


/***************************************************************************

    BBC Master Compact

****************************************************************************/


void bbcm_state::bbcmc(machine_config &config)
{
	bbcm(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcm_state::bbcmc_mem);
	m_bankdev->set_map(&bbcm_state::bbcmc_bankdev).set_options(ENDIANNESS_LITTLE, 8, 16, 0x0400);

	/* cassette */
	config.device_remove("cassette");

	/* fdc */
	WD1772(config.replace(), m_wd_fdc, 16_MHz_XTAL / 2);
	m_wd_fdc->intrq_wr_callback().set(FUNC(bbc_state::fdc_intrq_w));
	m_wd_fdc->drq_wr_callback().set(FUNC(bbc_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, "wd_fdc:0", bbc_floppies, "35dd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wd_fdc:1", bbc_floppies, "35dd", bbc_state::floppy_formats).enable_sound(true);

	I2C_PCD8572(config, "i2cmem", 0);
	config.device_remove("rtc");

	/* user via */
	m_via6522_1->readpb_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::pb_r)).mask(0x1f);
	m_via6522_1->readpb_handler().append(m_exp, FUNC(bbc_exp_slot_device::pb_r)).mask(0xe0);
	m_via6522_1->writepb_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::pb_w)).mask(0x1f);
	m_via6522_1->writepb_handler().append(m_exp, FUNC(bbc_exp_slot_device::pb_w)).mask(0xe0);
	m_via6522_1->cb1_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::write_cb1));
	m_via6522_1->cb2_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::write_cb2));

	/* cartridge sockets */
	config.device_remove("cartslot1");
	config.device_remove("cartslot2");

	/* eprom sockets */
	config.device_remove("romslot8");
	BBC_ROMSLOT16(config, m_rom[0x03], bbc_rom_devices, nullptr); /* ic17 */
	BBC_ROMSLOT16(config, m_rom[0x02], bbc_rom_devices, nullptr); /* ic23 */
	BBC_ROMSLOT32(config, m_rom[0x00], bbc_rom_devices, nullptr); /* ic38 */
	BBC_ROMSLOT16(config, m_rom[0x08], bbc_rom_devices, nullptr); /* ic29 */

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_mc").set_original("bbcmc_flop");
	SOFTWARE_LIST(config, "flop_ls_pro128s").set_compatible("pro128s_flop");
	subdevice<software_list_device>("flop_ls_m")->set_compatible("bbcm_flop");
	config.device_remove("cass_ls");
	config.device_remove("cart_ls_m");

	/* expansion ports */
	BBC_EXP_SLOT(config, m_exp, 16_MHz_XTAL / 2, bbc_exp_devices, nullptr);
	m_exp->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_exp->nmi_handler().set(FUNC(bbc_state::bus_nmi_w));

	BBC_JOYPORT_SLOT(config, m_joyport, bbc_joyport_devices, "joystick");
	m_joyport->cb1_handler().set(m_via6522_1, FUNC(via6522_device::write_cb1));
	m_joyport->cb2_handler().set(m_via6522_1, FUNC(via6522_device::write_cb2));

	config.device_remove("1mhzbus");
	config.device_remove("analogue");
	config.device_remove("intube");
	config.device_remove("extube");
	config.device_remove("userport");
	config.device_remove("internal");
	config.device_remove("modem");
}


void bbcm_state::pro128s(machine_config &config)
{
	bbcmc(config);
	/* software lists */
	subdevice<software_list_device>("flop_ls_pro128s")->set_original("pro128s_flop");
	subdevice<software_list_device>("flop_ls_mc")->set_compatible("bbcmc_flop");
}


void bbcm_state::autoc15(machine_config &config)
{
	bbcmc(config);

	m_bankdev->set_map(&bbcm_state::autoc15_bankdev).set_options(ENDIANNESS_LITTLE, 8, 16, 0x0400);

	/* crtc - replaces HD6845 to support smooth scrolling */
	HD6345(config.replace(), m_hd6845, 16_MHz_XTAL / 8);
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(12);
	//m_hd6845->set_begin_update_callback(FUNC(bbc_state::crtc_begin_update));
	m_hd6845->set_update_row_callback(FUNC(bbc_state::crtc_update_row));
	//m_hd6845->set_reconfigure_callback(FUNC(bbc_state::crtc_reconfig));
	m_hd6845->out_de_callback().set(FUNC(bbc_state::bbc_de_changed));
	m_hd6845->out_hsync_callback().set(FUNC(bbc_state::bbc_hsync_changed));
	m_hd6845->out_vsync_callback().set(FUNC(bbc_state::bbc_vsync_changed));

	/* Autocue RAM disc */
	m_exp->set_default_option("autocue");
	m_exp->set_fixed(true);
}



ROM_START(bbca)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  SPARE SOCKET */
	/* rom page 1 04000 IC88  SPARE SOCKET */
	/* rom page 2 08000 IC100 SPARE SOCKET */
	/* rom page 3 0c000 IC101 BASIC */
	ROM_DEFAULT_BIOS("os12")
	ROM_SYSTEM_BIOS( 0, "os12", "OS 1.20" )
	ROMX_LOAD("os12.rom",   0x40000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(0))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "os10", "OS 1.00" )
	ROMX_LOAD("os10.rom",   0x40000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(1))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "os09", "OS 0.92")
	ROMX_LOAD("os092.rom",  0x40000, 0x4000, CRC(59ef7eb8) SHA1(dca33995c0d008a527efe923d03333394b01022c), ROM_BIOS(2))
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "os01", "OS 0.10" )
	ROMX_LOAD("os01.rom",   0x40000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(3))
	/* OS0.1 does not support rom paging, load BASIC into all pages */
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(3))
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcb)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  DFS */
	/* rom page 1 04000 IC88  SPARE SOCKET */
	/* rom page 2 08000 IC100 SPARE SOCKET */
	/* rom page 3 0c000 IC101 BASIC */
	ROM_DEFAULT_BIOS("os12")
	ROM_SYSTEM_BIOS( 0, "os12", "OS 1.20" )
	ROMX_LOAD("os12.rom",   0x40000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(0))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "os10", "OS 1.00" )
	ROMX_LOAD("os10.rom",   0x40000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(1))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "os09", "OS 0.92")
	ROMX_LOAD("os092.rom",  0x40000, 0x4000, CRC(59ef7eb8) SHA1(dca33995c0d008a527efe923d03333394b01022c), ROM_BIOS(2))
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "os01", "OS 0.10" )
	ROMX_LOAD("os01.rom",   0x40000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(3))
	/* OS0.1 does not support rom paging, load BASIC into all pages */
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(3))
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )

	/* appropriate DFS will be inserted by fdc device */
	//ROM_LOAD("dnfs120-201666.rom", 0x00000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(bbcb_de)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC72 DFS */
	/* rom page 1 04000 IC73 SPARE SOCKET */
	/* rom page 2 08000 IC74 SPARE SOCKET */
	/* rom page 3 0c000 IC75 BASIC */
	ROM_LOAD("basic2.rom", 0xc000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dfs10.rom",  0x0000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os_de.rom", 0x0000, 0x4000, CRC(b7262caf) SHA1(aadf90338ee9d1c85dfa73beba50e930c2a38f10))

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(bbcb_us)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC72 VIEW2.1 */
	/* rom page 1 04000 IC73 US DNFS */
	/* rom page 2 08000 IC74 US BASIC */
	/* rom page 3 0c000 IC75 SPARE SOCKET */
	ROM_LOAD("usbasic3.rom", 0x8000, 0x4000, CRC(161b9539) SHA1(b39014610a968789afd7695aa04d1277d874405c))
	ROM_LOAD("viewa210.rom", 0x0000, 0x4000, CRC(4345359f) SHA1(88c93df1854f5fbe6cd6e5f0e29a8bf4ea3b5614))
	ROM_LOAD("usdnfs10.rom", 0x4000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("usmos10.rom", 0x0000, 0x4000, CRC(c8e946a9) SHA1(83d91d089dca092d2c8b7c3650ff8143c9069b89))

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(bbcb_no)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC72 DFS */
	/* rom page 1 04000 IC73 VIEW2.1 */
	/* rom page 2 08000 IC74 SPARE SOCKET */
	/* rom page 3 0c000 IC75 BASIC */
	ROM_LOAD("dfs0.9h.rom",  0x0000, 0x2000, CRC(af2fa873) SHA1(dbbec4d2540a854c120be3194c7566a2b79d153b))
	ROM_LOAD("viewa210.rom", 0x4000, 0x4000, CRC(4345359f) SHA1(88c93df1854f5fbe6cd6e5f0e29a8bf4ea3b5614))
	ROM_LOAD("basic2.rom",   0xc000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("nos12.rom", 0x0000, 0x4000, CRC(49859294) SHA1(2b6aecd33a43f296c20832524e47cc7e3a9c3b17))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(sist1)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  DFS */
	/* rom page 1 04000 IC88  PROGS */
	/* rom page 2 08000 IC100 BASIC */
	/* rom page 3 0c000 IC101 STARTUP */
	ROM_LOAD("dnfs120-201666.rom", 0x0000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("sist1_progs.bin",    0x4000, 0x4000, CRC(aea21243) SHA1(4398ba29c871fa397654aa182c63ccdcad597625))
	ROM_LOAD("basic2.rom",         0x8000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("sist1_startup.bin",  0xc000, 0x4000, CRC(9cd1602c) SHA1(5ea266f47ff83821ccdbec006b8506b2e892b115))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("cm62024.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(torchf)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  BASIC */
	/* rom page 1 04000 IC88  DNFS */
	/* rom page 2 08000 IC100 CPN (inserted by device) */
	/* rom page 3 0c000 IC101 SPARE SOCKET */
	ROM_LOAD("basic2.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dnfs120-201666.rom", 0x4000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(torchh)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  BASIC */
	/* rom page 1 04000 IC88  DNFS */
	/* rom page 2 08000 IC100 CPN (inserted by device) */
	/* rom page 3 0c000 IC101 SPARE SOCKET */
	ROM_LOAD("basic2.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dnfs120-201666.rom", 0x4000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))

	DISK_REGION("1mhzbus:torchhd:sasi:0:s1410:image")
	DISK_IMAGE("torch_utilities", 0, BAD_DUMP SHA1(33a5f169bd91b9c6049e8bd0b237429c091fddd0)) /* NEC D5126 contains Standard and Hard Disc Utilities, not known what was factory installed */
ROM_END


ROM_START(torch301)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0 00000 IC52  BASIC */
	/* rom page 1 04000 IC88  ECO 3.35K */
	/* rom page 2 08000 IC100 SPARE SOCKET */
	/* rom page 3 0c000 IC101 MCP 1.01 */
	ROM_LOAD("basic2.rom",      0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("eco_3.35k.rom",   0x4000, 0x4000, CRC(3ca2faea) SHA1(7462ced7b83d74b822815bc00ed40a89f84e0276))
	ROM_LOAD("mcp_1.01_ci.rom", 0xc000, 0x4000, CRC(436e7fe9) SHA1(be10872aeb88714bd56462a2e86929953dee1c01))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(torch725)
	ROM_REGION(0x40000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	/* rom page 0  00000 IC52  BASIC */
	/* rom page 1  04000 IC88  ECO 3.35K */
	/* rom page 2  08000 IC100 Unix Host */
	/* rom page 3  0c000 IC101 MCP 1.22 */
	ROM_LOAD("basic2.rom",      0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("eco_3.35k.rom",   0x4000, 0x4000, CRC(3ca2faea) SHA1(7462ced7b83d74b822815bc00ed40a89f84e0276))
	ROM_LOAD("unix_1.00.rom",   0x8000, 0x4000, CRC(90f85ce9) SHA1(e37d043c8df30c49ba8717e1aa0b92105cb0c937))
	ROM_LOAD("mcp_1.22_ci.rom", 0xc000, 0x4000, CRC(764f4948) SHA1(409762deafb76b1f86be39bfbf2f812d5de3ff92))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phrom_us.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(bbcbp)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_LOAD("bpos2.ic71", 0x3c000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_bbcbp128 rom_bbcbp


ROM_START(abc110)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos200")
	ROM_SYSTEM_BIOS( 0, "mos200", "MOS2.00" )
	ROMX_LOAD("mos200.rom",   0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(0))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "mos123stor", "MOS1.23 + ViewStore" )
	ROMX_LOAD("mos123stor.rom", 0x3c000, 0x8000, CRC(4e84f452) SHA1(145ee54f04b3eb4d0e5afaabe21915be48db3c54), ROM_BIOS(1)) // rom page 15 3C000 ViewStore
	ROM_SYSTEM_BIOS( 2, "mos123", "MOS1.23" )
	ROMX_LOAD("mos123.rom",   0x40000, 0x4000, CRC(90d31d08) SHA1(42a01892cf8bd2ada4db1c8b36aff80c85eb5dcb), ROM_BIOS(2))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "mos120", "MOS1.20" )
	ROMX_LOAD("mos120.rom",   0x40000, 0x4000, CRC(0a1e83a0) SHA1(21dc3a94eef7c003b194686730fb461779f44925), ROM_BIOS(3))
	ROMX_LOAD("basic200.rom", 0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(3))
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_abc310 rom_abc110


ROM_START(acw443)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos210")
	ROM_SYSTEM_BIOS( 0, "mos210", "MOS2.10" )
	ROMX_LOAD("acwmos210.rom", 0x40000, 0x4000, CRC(168d6753) SHA1(dcd01d8f5f6e0cd92ae626ca52a3db71abf5d282), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "mos200", "MOS2.00" )
	ROMX_LOAD("mos200.rom", 0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(1))
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(reutapm)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("mos_r030.rom", 0x0000, 0x4000, CRC(8b652337) SHA1(6a5c7ace255c8ac96c983d5ba67084fbd71ff61e))
ROM_END


ROM_START(econx25)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
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
	ROM_LOAD("0246,215_02_x25tsi_v0.51.rom", 0x0c000, 0x4000, CRC(71dd84e4) SHA1(bbfa892fdcc6f753dda5134ecb97cc7c42b959c2))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcm)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "mos350", "Enhanced MOS 3.50" )
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "mos329", "FinMOS 3.29")
	ROMX_LOAD("mos329.ic24", 0x20000, 0x20000, CRC(8dd7338b) SHA1(4604203c70c04a9fd003103deec438fc5bd44839), ROM_BIOS(2))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(1))
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(2))
ROM_END


#define rom_bbcmt rom_bbcm
#define rom_bbcm512 rom_bbcm


ROM_START(bbcmaiv)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320aiv.cmos", 0x0e, 0x32, BAD_DUMP CRC(b9ae42a1) SHA1(abf3e94b013f24027ca36c96720963c3411e93f8), ROM_BIOS(0))
ROM_END


ROM_START(bbcmet)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos400")
	ROM_SYSTEM_BIOS( 0, "mos400", "Econet MOS 4.00" )
	ROMX_LOAD("mos400.ic24", 0x30000, 0x10000, CRC(81729034) SHA1(d4bc2c7f5e66b5298786138f395908e70c772971), ROM_BIOS(0))
	ROM_COPY("swr", 0x34000, 0x24000, 0xc000) /* Mirror */
	ROM_COPY("swr", 0x30000, 0x40000, 0x4000) /* Move loaded roms into place */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos400.cmos", 0x0e, 0x32, BAD_DUMP CRC(fff41cc5) SHA1(3607568758f90b3bd6c7dc9533e2aa24f9806ff3), ROM_BIOS(0))
ROM_END


ROM_START(bbcmarm)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS( 0, "mos320", "Original MOS 3.20" )
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) /* Move loaded roms into place */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320arm.cmos", 0x00, 0x40, CRC(56117257) SHA1(ed98563bef18f9d2a0b2d941cd20823d760fb127), ROM_BIOS(0))
ROM_END


ROM_START(bbcmc)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos510")
	ROM_SYSTEM_BIOS( 0, "mos510", "Enhanced MOS 5.10" )
	ROMX_LOAD("mos510.ic49", 0x30000, 0x10000, CRC(9a2a6086) SHA1(094ab37b0b6437c4f1653eaa0602ef102737adb6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "mos500", "Original MOS 5.00" )
	ROMX_LOAD("mos500.ic49", 0x30000, 0x10000, CRC(f6170023) SHA1(140d002d2d9cd34b47197a2ba823505af2a84633), ROM_BIOS(1))
	ROM_COPY("swr", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   IC38 or EXTERNAL */
	/* 04000 rom 1   IC38 or EXTERNAL */
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
	/* 34000 rom 13  IC49 ADFS */
	/* 38000 rom 14  IC49 BASIC */
	/* 3c000 rom 15  IC49 Utils */

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcmc_ar)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos511i")
	ROM_SYSTEM_BIOS( 0, "mos511i", "International MOS 5.11i" )
	ROMX_LOAD("mos511.ic49", 0x30000, 0x10000, CRC(8708803c) SHA1(d2170c8b9b536f3ad84a4a603a7fe712500cc751), ROM_BIOS(0))
	ROM_COPY("swr", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   IC38 or EXTERNAL */
	/* 04000 rom 1   IC38 or EXTERNAL */
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
	/* 34000 rom 13  IC49 ADFS */
	/* 38000 rom 14  IC49 BASIC */
	/* 3c000 rom 15  IC49 Utils */
	ROM_LOAD("international16.rom", 0x8000 , 0x4000, CRC(0ef527b1) SHA1(dc5149ccf588cd591a6ad47727474ef3313272ce) )
	ROM_LOAD("arabian-c22.rom"    , 0x20000, 0x4000, CRC(4f3aadff) SHA1(2bbf61ba68264ce5845aab9c54e750b0efe219c8) )

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(pro128s)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos510o")
	ROM_SYSTEM_BIOS(0, "mos510o", "Olivetti MOS 5.10")
	ROMX_LOAD("mos510o.ic49", 0x30000, 0x10000, CRC(c16858d3) SHA1(ad231ed21a55e493b553703285530d1cacd3de7a), ROM_BIOS(0)) // System ROM 0258,211-01
	ROM_COPY("swr", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)
	/* 00000 rom 0   IC38 or EXTERNAL */
	/* 04000 rom 1   IC38 or EXTERNAL */
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
	/* 34000 rom 13  IC49 ADFS */
	/* 38000 rom 14  IC49 BASIC */
	/* 3c000 rom 15  IC49 Utils */

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(autoc15)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos510i")
	ROM_SYSTEM_BIOS(0, "mos510i", "Swedish MOS 5.10i")
	ROMX_LOAD("swedish_mega_29-1.ic49", 0x20000, 0x20000, CRC(67512992) SHA1(5d04b6e53a3a75af22ab10c652cceb9a63b23a6d), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   IC38 SBII */
	/* 04000 rom 1   IC38 SBII */
	/* 08000 rom 2   IC23 APROMPT */
	/* 0c000 rom 3   IC17 Swedish 16K ISO */
	/* 10000 rom 4   SWRAM */
	/* 14000 rom 5   SWRAM */
	/* 18000 rom 6   SWRAM */
	/* 1c000 rom 7   SWRAM */
	/* 20000 rom 8   IC29 MODROM 0.47 */
	/* 24000 rom 9   IC49 ADFS */
	/* 28000 rom 10  IC49 BASIC */
	/* 2c000 rom 11  IC49 Autocue Giant */
	/* 30000 rom 12  IC49 Autocue Large */
	/* 34000 rom 13  IC49 Autocue Medium */
	/* 38000 rom 14  IC49 Autocue Small */
	/* 3c000 rom 15  IC49 Utils */
	ROM_LOAD("sbii-25-1-88.ic38",    0x00000, 0x8000, CRC(36af3215) SHA1(16d39f15b10b4e23e76bad23a53b4111ce877bc1))
	ROM_LOAD("aprompt.ic23",         0x08000, 0x4000, NO_DUMP)
	ROM_LOAD("swedish_16k_iso.ic17", 0x0c000, 0x4000, CRC(bd7716c0) SHA1(8a70f941f4de64d87e956e2086eb50287b8205b9))
	ROM_LOAD("modrom0_47.ic29",      0x20000, 0x4000, CRC(0d7874cb) SHA1(3f467f0b1618fb6546a2b94ca22b9f58d58bbdce))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)
ROM_END


ROM_START(daisy)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SIMDIST */
	/* 14000 rom 5   IC41 DHRFDSY */
	/* 18000 rom 6   IC37 IRFDSY */
	/* 1c000 rom 7   IC37 DAISY */
	/* 20000 rom 8   IC27 HiBASIC3 */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("simdist_2_22-2-89_dhrfdsy.rom", 0x10000, 0x8000, CRC(8a9d9c4a) SHA1(278c5c63e06359601cdf972f55e98f5a7442a713))
	ROM_LOAD("daisy_irfdsy_vr4_5-1-89.rom", 0x18000, 0x8000, CRC(9662d779) SHA1(0841c1fd34c152f3f02ace8633f1fa3f5139069e))
	ROM_LOAD("hibas03_a063.rom", 0x20000, 0x4000, CRC(6ea7affc) SHA1(99234b55fde57680e4217b72ef4ccb8fc56edeff))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(discmon)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(discmate)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(mpc800)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 800 Manager */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("mpc800manager-2.40.rom", 0x20000, 0x4000, CRC(d5a27b00) SHA1(533e846f47803d61508fe270fd7021c010a21a84))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(mpc900)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 900 Manager */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("mpc900_manager-1.20.rom", 0x20000, 0x4000, BAD_DUMP CRC(3470af89) SHA1(5d54ace2fbfdb9a7ec88aeaebcfe978688ef1893))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(mpc900gx)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_DEFAULT_BIOS("mos320")
	ROM_SYSTEM_BIOS(0, "mos320", "Original MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))
	ROM_COPY("swr", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)
	/* 00000 rom 0   SK3 Rear Cartridge bottom 16K */
	/* 04000 rom 1   SK3 Rear Cartridge top 16K */
	/* 08000 rom 2   SK4 Front Cartridge bottom 16K */
	/* 0c000 rom 3   SK4 Front Cartridge top 16K */
	/* 10000 rom 4   IC41 SWRAM or bottom 16K */
	/* 14000 rom 5   IC41 SWRAM or top 16K */
	/* 18000 rom 6   IC37 SWRAM or bottom 16K */
	/* 1c000 rom 7   IC37 SWRAM or top 16K */
	/* 20000 rom 8   IC27 900GX Manager */
	/* 24000 rom 9   IC24 DFS + SRAM */
	/* 28000 rom 10  IC24 Viewsheet */
	/* 2c000 rom 11  IC24 Edit */
	/* 30000 rom 12  IC24 BASIC */
	/* 34000 rom 13  IC24 ADFS */
	/* 38000 rom 14  IC24 View + MOS code */
	/* 3c000 rom 15  IC24 Terminal + Tube host + CFS */
	ROM_LOAD("mpc900gx_manager-1.20.rom", 0x20000, 0x4000, CRC(3470af89) SHA1(5d54ace2fbfdb9a7ec88aeaebcfe978688ef1893))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
ROM_END


ROM_START(cfa3000bp)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_LOAD("bpos2.ic71", 0x3c000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
	/* rom page 0  00000 SWRAM (B+ 128K only) */
	/* rom page 1  04000 SWRAM (B+ 128K only) */
	/* rom page 2  08000 IC35 32K IN PAGE 3 */
	/* rom page 3  0c000 IC35 SPARE SOCKET */
	/* rom page 4  10000 IC44 32K IN PAGE 5 */
	/* rom page 5  14000 IC44 SPARE SOCKET */
	/* rom page 6  18000 IC57 32K IN PAGE 7 */
	/* rom page 7  1c000 IC57 SPARE SOCKET */
	/* rom page 8  20000 IC62 32K IN PAGE 9 */
	/* rom page 9  24000 IC62 SPARE SOCKET */
	/* rom page 10 28000 IC68 32K IN PAGE 11 */
	/* rom page 11 2c000 IC68 SPARE SOCKET */
	/* rom page 12 30000 SWRAM (B+ 128K only) */
	/* rom page 13 34000 SWRAM (B+ 128K only) */
	/* rom page 14 38000 IC71 32K IN PAGE 15 */
	/* rom page 15 3C000 IC71 BASIC */
	ROM_SYSTEM_BIOS(0, "4", "Issue 4")
	ROMX_LOAD("cfa3000_3.rom", 0x14000, 0x4000, CRC(4f246cd5) SHA1(6ba9625248c585deed5c651a889eecc86384a60d), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_4.rom", 0x1c000, 0x4000, CRC(ca0e30fd) SHA1(abddc7ba6d16855ebda2ef55fe8662bc545ae755), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_s.rom", 0x24000, 0x4000, CRC(71fd4c8a) SHA1(5bad70ee55403bc0191f6b189c9b6e5effdbca4c), ROM_BIOS(0))

	/* link S13 set for BASIC to take low priority ROM numbers 0/1 */
	ROM_COPY("swr", 0x3c000, 0x4000, 0x4000)
	ROM_FILL(0x3c000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x4000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(cfa3000)
	ROM_REGION(0x44000, "swr", ROMREGION_ERASEFF) /* Sideways ROMs */
	ROM_SYSTEM_BIOS(0, "103", "Issue 10.3")
	ROMX_LOAD("cfa3000_3m4_iss10.3.ic41",           0x10000, 0x08000, CRC(ecb385ab) SHA1(eafa9b34cb1cf63790f74332bb7d85ee356b6973), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_sm_iss10.3.ic37",            0x18000, 0x08000, CRC(c07aee5f) SHA1(1994e3755dc15d1ea7e105bc19cd57893b719779), ROM_BIOS(0))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss10.3.ic24", 0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "102", " Issue 10.2")
	ROMX_LOAD("cfa3000_3m4_iss10.2.ic41",           0x10000, 0x08000, CRC(ecb385ab) SHA1(eafa9b34cb1cf63790f74332bb7d85ee356b6973), ROM_BIOS(1))
	ROMX_LOAD("cfa3000_sm_iss10.2.ic37",            0x18000, 0x08000, CRC(e733d5b3) SHA1(07e89943c6ac0953b75686ee06e947f33119dbed), ROM_BIOS(1))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss10.2.ic24", 0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "9", " Issue 9")
	ROMX_LOAD("cfa3000_3m4_iss9.ic41",              0x10000, 0x08000, CRC(a4bd5d53) SHA1(90747ff7bd81ac1e124bae964c206d8df163e1d6), ROM_BIOS(2))
	ROMX_LOAD("cfa3000_sm_iss9.ic37",               0x18000, 0x08000, CRC(559d1fae) SHA1(271e1ab9b53e82028e92e7cdb8c517df06e76477), ROM_BIOS(2))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss9.ic24",    0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "7", "Issue 7")
	ROMX_LOAD("cfa3000_3m4_iss7.ic41",              0x10000, 0x08000, CRC(a0b32288) SHA1(83b047e9eb35f0644bd8f0acb1a56e1428bacc0b), ROM_BIOS(3))
	ROMX_LOAD("cfa3000_sm_iss7.ic37",               0x18000, 0x08000, CRC(3cd42bbd) SHA1(17f6c66039d20a364cc9e1377c7ced14d5302603), ROM_BIOS(3))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss7.ic24",    0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(3))
	ROM_COPY("swr", 0x20000, 0x30000, 0x10000) /* Mirror MOS */
	ROM_COPY("swr", 0x30000, 0x40000, 0x04000) /* Move loaded roms into place */
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

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("swr", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0) /* mc146818 */
	/* Factory defaulted CMOS RAM, sets default language ROM, etc. */
	ROM_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac))
ROM_END


#define rom_ltmpbp rom_bbcbp
#define rom_ltmpm rom_bbcm


/*    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT   CLASS         INIT       COMPANY                        FULLNAME                              FLAGS */

/* Acorn Computers */
COMP( 1981, bbcb,       0,      bbca,  bbcb,       bbcb,   bbc_state,    init_bbc,  "Acorn Computers",             "BBC Micro Model B",                  MACHINE_IMPERFECT_GRAPHICS )
COMP( 1981, bbca,       bbcb,   0,     bbca,       bbca,   bbc_state,    init_bbc,  "Acorn Computers",             "BBC Micro Model A",                  MACHINE_IMPERFECT_GRAPHICS )
COMP( 1982, bbcb_de,    bbcb,   0,     bbcb_de,    bbcb,   bbc_state,    init_bbc,  "Acorn Computers",             "BBC Micro Model B (German)",         MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, bbcb_us,    bbcb,   0,     bbcb_us,    bbcb,   bbc_state,    init_bbc,  "Acorn Computers",             "BBC Micro Model B (US)",             MACHINE_IMPERFECT_GRAPHICS )
COMP( 1984, bbcb_no,    bbcb,   0,     bbcb_de,    bbcb_no, bbc_state,   init_bbc,  "Acorn Computers",             "BBC Micro Model B (Norway)",         MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, bbcbp,      0,      bbcb,  bbcbp,      bbcbp,  bbcbp_state,  init_bbc,  "Acorn Computers",             "BBC Micro Model B+ 64K",             MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, bbcbp128,   bbcbp,  0,     bbcbp128,   bbcbp,  bbcbp_state,  init_bbc,  "Acorn Computers",             "BBC Micro Model B+ 128K",            MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, abc110,     bbcbp,  0,     abc110,     abc,    bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 110",                            MACHINE_NOT_WORKING )
COMP( 1985, acw443,     bbcbp,  0,     acw443,     abc,    bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 210/Cambridge Workstation",      MACHINE_NOT_WORKING )
COMP( 1985, abc310,     bbcbp,  0,     abc310,     abc,    bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 310",                            MACHINE_NOT_WORKING )
COMP( 1985, reutapm,    bbcbp,  0,     reutapm,    bbcb,   bbcbp_state,  init_bbc,  "Acorn Computers",             "Reuters APM",                        MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 1986, econx25,    bbcbp,  0,     econx25,    bbcbp,  bbcbp_state,  init_bbc,  "Acorn Computers",             "Econet X25 Gateway",                 MACHINE_NOT_WORKING )
COMP( 1986, bbcm,       0,      bbcb,  bbcm,       bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master 128",                     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmt,      bbcm,   0,     bbcmt,      bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master Turbo",                   MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmaiv,    bbcm,   0,     bbcmaiv,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master AIV",                     MACHINE_NOT_WORKING )
COMP( 1986, bbcmet,     bbcm,   0,     bbcmet,     bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master ET",                      MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcm512,    bbcm,   0,     bbcm512,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master 512",                     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmarm,    bbcm,   0,     bbcmarm,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master (ARM Evaluation)",        MACHINE_NOT_WORKING )
COMP( 1986, bbcmc,      0,      bbcm,  bbcmc,      bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master Compact",                 MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmc_ar,   bbcmc,  0,     bbcmc,      bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master Compact (Arabic)",        MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, pro128s,    bbcmc,  0,     pro128s,    bbcm,   bbcm_state,   init_bbc,  "Olivetti",                    "Prodest PC 128S",                    MACHINE_IMPERFECT_GRAPHICS )

/* Torch Computers */
COMP( 1982, torchf,     bbcb,   0,     torchf,     torchb, torch_state,  init_bbc,  "Torch Computers",             "Torch CF240",                        MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, torchh,     bbcb,   0,     torchh,     torchb, torch_state,  init_bbc,  "Torch Computers",             "Torch CH240",                        MACHINE_IMPERFECT_GRAPHICS )
COMP( 1984, torch301,   bbcb,   0,     torch301,   torchi, torch_state,  init_bbc,  "Torch Computers",             "Torch Model 301",                    MACHINE_NOT_WORKING )
COMP( 1984, torch725,   bbcb,   0,     torch725,   torchi, torch_state,  init_bbc,  "Torch Computers",             "Torch Model 725",                    MACHINE_NOT_WORKING )

/* TV Production */
COMP( 1988, autoc15,    bbcmc,  0,     autoc15,    autoc,  bbcm_state,   init_bbc,  "Autocue Ltd.",                "Autocue 1500 Teleprompter",          MACHINE_NOT_WORKING )
COMP( 1987, mpc800,     bbcm,   0,     mpc800,     bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 800 Series",             MACHINE_NOT_WORKING )
COMP( 1988, mpc900,     bbcm,   0,     mpc900,     bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 900 Series",             MACHINE_NOT_WORKING )
COMP( 1990, mpc900gx,   bbcm,   0,     mpc900gx,   bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 900GX Series",           MACHINE_NOT_WORKING )

/* Jukeboxes */
COMP( 1988, discmon,    bbcm,   0,     discmon,    bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmonitor A-01",           MACHINE_NOT_WORKING )
COMP( 1988, discmate,   bbcm,   0,     discmate,   bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmate A-02",              MACHINE_NOT_WORKING )
//COMP( 1988, discmast,   bbcm,   0,     discmast,   bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmaster A-03",            MACHINE_NOT_WORKING )

/* Industrial */
COMP( 198?, sist1,      bbcb,   0,     sist1,      bbcb,   bbc_state,    init_bbc,  "Cisco Systems",               "Cisco SIST1 Terminal",               MACHINE_NOT_WORKING )
COMP( 1985, ltmpbp,     bbcbp,  0,     bbcbp,      ltmpbp, bbcbp_state,  init_ltmp, "Lawrie T&M Ltd.",             "LTM Portable (B+)",                  MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, ltmpm,      bbcm,   0,     bbcm,       ltmpm,  bbcm_state,   init_ltmp, "Lawrie T&M Ltd.",             "LTM Portable (Master)",              MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, daisy,      bbcm,   0,     daisy,      bbcm,   bbcm_state,   init_bbc,  "Comus Instruments Ltd.",      "Comus Daisy",                        MACHINE_NOT_WORKING )
COMP( 198?, cfa3000bp,  bbcbp,  0,     cfa3000bp,  bbcbp,  bbcbp_state,  init_cfa,  "Tinsley Medical Instruments", "Henson CFA 3000 (B+)",               MACHINE_NOT_WORKING )
COMP( 1989, cfa3000,    bbcm,   0,     cfa3000,    bbcm,   bbcm_state,   init_cfa,  "Tinsley Medical Instruments", "Henson CFA 3000 (Master)",           MACHINE_NOT_WORKING )
