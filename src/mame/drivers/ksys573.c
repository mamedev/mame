// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
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

  Note 3: Some games require an installation cassette when installing from CD.
  Go to the slot devices menu in the tab menu and change the cassette from "game"
  to "install" and select reset. After installing you need to change the cassette
  back to "game" and select reset.

  Note 4: Some games require you to press f2 to skip the rtc cleared note.

  -----------------------------------------------------------------------------------------

  System 573 Hardware Overview
  Konami, 1998-2001

  This system uses Konami PSX-based hardware with an ATAPI CDROM drive.
  Gun Mania (and probably Gun Mania Zone Plus) has no CDROM drive.
  There is a slot for a security cart (cart is installed in CN14) and also a PCMCIA card slot.
  The main board and CDROM drive are housed in a black metal box. Gun Mania doesn't have that box.
  The games can be swapped by exchanging the CDROM disc and the security cart, whereby the main-board
  FlashROMs are re-programmed after a small wait. On subsequent power-ups, there is a check to test if the
  contents of the FlashROMs matches the CDROM, then the game boots up immediately.

  PCMCIA card slot is used by Dance Dance Revolution (2ndMIX link ver. and later),
  GUITARFREAKS (2ndMIX link ver. and later), drummania (7thMIX and later), and Gun Mania.
  DDR and GF: 32M flash card is used to store edit data (players can edit data by PlayStation
  console, and send data to arcade machine via PS memory card).
  DM and GF: network PCB unit (for e-AMUSEMENT) is connected via PCMCIA slot.
  GM: unknown (program data is stored in flash card?)

  The games that run on this system include...

  Game                                         Year       Hardware Code     CD Code
  ---------------------------------------------------------------------------------
P *Anime Champ                                  2000.12
P Bass Angler                                   1998.03    GE765 JA          765 JA A02
P Bass Angler 2                                 1998.07    GC865 JA          865 JA A02
P *DAM-DDR Dance Dance Revolution for DAM       1999.11
P *DAM-DDR Dance Dance Revolution for DAM 2nd   2000.07
A Dance Dance Revolution                        1998.09    GC845 JA          845 JA(missing)/UA A01 / 845 JA A02
A Dance Dance Revolution Internet Ranking ver.  1998.11    GC845 JB          845 JB A01 / 845 JA/UA A02
A Dance Dance Revolution 2ndMIX                 1999.01    GC895 JA          895 JA A02
A Dance Dance Revolution 2ndMIX LINK version    1999.04    GE885 JA          885 JA A02
A DDR 2ndMIX with bmIIDX CLUB ver.              1999.05    GN896 JA          896 JA A01
A DDR 2ndMIX AND bmIIDX substream CLUB ver. 2   1999.05    GE984 JA          984 JA A01
D Dance Dance Revolution Solo BASS MIX          1999.08    GQ894 JA          894 JA A02
D Dance Dance Revolution 3rdMIX                 1999.10    GN887 AA/JA/KA    887 AA/JA/KA A02
D Dance Dance Revolution Solo 2000              1999.12    GC905 AA/JA       905 AA/JA A02
D Dance Dance Revolution 3rdMIX PLUS            2000.06    GCA22 JA          A22 JA A02
D Dance Dance Revolution 4thMIX                 2000.08    GCA33 JA          A33 JA A02
D Dance Dance Revolution 4thMIX PLUS            2000.12    GCA34 JA          A34 JA A02
? *Dance Dance Revolution Kids                  2000.12
D Dance Dance Revolution 5thMIX                 2001.03    GCA27 JA          A27 JA A02
D DDRMAX Dance Dance Revolution 6thMIX          2001.10    GCB19 JA          B19 JA A02
D DDRMAX2 Dance Dance Revolution 7thMIX         2002.03    GCB20 JA          B20 JA A02
D Dance Dance Revolution EXTREME                2002.12    GCC36 JA          C36 JA A02
D Dance Maniax                                  2000.06    GE874 JA          874 JA A(needs redump)
D Dance Maniax 2ndMIX                           2000.12    GCA39 JA          A39 JA A02
D *Dance Maniax 2ndMIX APPEND J PARADISE        2001.04
A Dancing Stage                                 1999.08    GN845 EA          845 EA(needs redump)
D Dancing Stage Euro Mix                        2000       GE936 EA          936 EA A(needs redump)
D Dancing Stage Euro Mix 2                      2000       G*C23 EA          C23 EA A02
D Dancing Stage featuring Disney's Rave         2000.11    GCA37 JA          A37 JA A02
D Dancing Stage featuring DREAMS COME TRUE      1999.12    GC910 JA          910 JA/JC A02
A Dancing Stage featuring TRUE KiSS DESTiNATiON 1999.07    G*884 JA          884 JA A02
P Dark Horse Legend                             1998.03    GX706 JA          706 JA A02
A drummania                                     1999.07    GQ881 JA          881 JA D01 / 881 JA A02(missing, audio CD)
D drummania 2ndMIX                              2000.03    GE912 JA          912 JA B02
D drummania 3rdMIX                              2000.09    GCA23 JA          A23 JA A02
D drummania 4thMIX                              2001.03    GEA25 JA          A25 JA A02
D drummania 5thMIX                              2001.09    GCB05 JA          B05 JA A02
D drummania 6thMIX                              2002.02    GCB16 JA          B16 JA A02
N drummania 7thMIX                              2002.08    GCC07 JA          C07 JA A02
N drummania 7thMIX power-up ver.                2002.08    GEC07 JB          C07 JC A02
N drummania 8thMIX                              2003.04    GCC38 JA          C38 JA A02
N drummania 9thMIX                              2003.10    GCD09 JA          D09 JA A02
N *drummania 10thMIX                            2004.04
? *Fighting Mania                               2000
P Fisherman's Bait                              1998.06    GE765 UA          765 UA B02
P Fisherman's Bait 2                            1998       GC865 UA          865 UA B02
P Fisherman's Bait Marlin Challenge             1999       GX889             889 AA/EA/JA/UA(needs redump)
P Gachagachamp                                  1999.01    GQ877 JA          GE877-JA(PCMCIA card)
A GUITARFREAKS                                  1999.02    GQ886 EA/JA/UA    886 ** C02
A GUITARFREAKS 2ndMIX                           1999.07    GQ883 JA          929 JB B02(needs redump)
A *GUITARFREAKS 2ndMIX Link ver.                1999.09
D GUITARFREAKS 3rdMIX                           2000.04    GE949 JA          949 JA C01 / 949 JA C02
D GUITARFREAKS 4thMIX                           2000.08    GEA24 JA          A24 JA A02
D GUITARFREAKS 5thMIX                           2001.03    GCA26 JA          A26 JA A02
D GUITARFREAKS 6thMIX                           2001.09    GCB06 JA          B06 JA A02
D GUITARFREAKS 7thMIX                           2002.02    GCB17 JA          B17 JA A02
N GUITARFREAKS 8thMIX                           2002.08    GCC08 JA          C08 JA A02
N GUITARFREAKS 8thMIX power-up ver.             2002.11    GEC08 JB          C08 JB A02
N GUITARFREAKS 9thMIX                           2003.04    GCC39 JA          C39 JA A02
N GUITARFREAKS 10thMIX                          2003.10    GCD10 JA          D10 JA A02
N *GUITARFREAKS 11thMIX                         2004.04
G *Gun Mania                                    2000.07    G?906 JA          (no CD)
? *Gun Mania Zone Plus                          2000.10
P Handle Champ                                  1997.12    GQ710 JA          (no CD)
P Hyper Bishi Bashi Champ                       1998.07    GC876 EA          (no CD)
P Hyper Bishi Bashi Champ - 2 Player            1999.08    GC908 JA          908    A02
P Jikkyou Powerful Pro Yakyuu EX                1998.04    GX802 JA          802 JA B02
P *Jikkyou Powerful Pro Yakyuu EX 98            1998.08
? *Kick & Kick                                  2001
P Konami 80's Arcade Gallery                    1998.11    GC826 JA          826 JA A01
P Konami 80's AC Special                        1998       GC826 UA          826 UA A01
D *Mambo a GoGo                                 2001.06
D Punchmania Hokuto no Ken                      2000.03                      918 JA B02
D Punchmania Hokuto no Ken 2                    2000.12                      A09 JA A02
P Salary Man Champ
P Salary Man Champ - 2 Player                   2001.02    GCA18 JA          A18 JA(needs redump)
P *Step Champ                                   1999.12

P: plain System573
A: uses ext. analog I/O board
D: uses ext. digital sound and I/O board
N: uses network PCB unit + ext. digital sound and I/O board
G: gun mania only, drives air soft gun (this game uses real BB bullet)

  Note:
       Not all games listed above are confirmed to run on System 573.
       * - denotes not dumped yet.

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
  |                              CN5                        CN3      CN17    |
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
  CXD8561Q  - Sony CXD8561Q GPU (QFP208, @ 10M) Also found CXD8561BQ in some units
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
  KM416V256 - Samsung Electronics KM416V256BT-7 256k x 16 DRAM (TSOP44/40, @ 11Q labelled 'SPUDR4M')
  KM48V514  - Samsung Electronics KM48V514BJ-6 512k x 8 EDO DRAM (SOJ28, @ 16G/H, 14G/H, 12G/H, 9G/H labelled 'HDR4M8SJ')
              Also found NEC 424805AL-A60 in some units
  32M       - NEC D481850GF-A12 128k x 32Bit x 2 Banks SGRAM (QFP100, @ 4P & 4L)
              Also found Samsung KM4132G271Q-12 in some units
  Software  -
              - 700A01.22G 4M MaskROM (DIP32, @ 22G). AMD 27C040 is also used
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


 GE877-PWB(C) (C)1998 KONAMI
  |----------------------|
|--       JAMMA OUT      --|
|                          |
|      CN6                 |
|     CN5  CN4  CN3  CN2   |
|                          |
|                          |
|         JAMMA IN         |
|--------------------------|
 Notes: This PCB is used for Gachagachamp. No ICs.

        CN5 - To control lever unit (1P). uses 9 pins out of 15 pins of B15P-SHF-1AA
        CN6 - To control lever unit (2P). uses 9 pins out of 14 pins of B14P-SHF-1AA
        (CN4, CN3, CN2 is printed pattern only, no actual connector)


  PCMCIA Flash Card
  -----------------

  Front

  |----PCMCIA CONNECTOR-----|
  |                         |
  | HT04A MB624018 MB624019 |
  | AT28C16                 |
  |                         |
  | 29F017A.1L   29F017A.1U |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.2L   29F017A.2U |
  | 90PFTN       90PFTR     |
  |                         |
  | 29F017A.3L   29F017A.3U |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.4L   29F017A.4U |
  | 90PFTN       90PFTR     |
  |                         |
  |------------------SWITCH-|

  Back

  |----PCMCIA CONNECTOR-----|
  |                         |
  |                         |
  |                         |
  |                         |
  | 29F017A.5U   29F017A.5L |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.6U   29F017A.6L |
  | 90PFTN       90PFTR     |
  |                         |
  | 29F017A.7U   29F017A.7L |
  | 90PFTR       90PFTN     |
  |                         |
  | 29F017A.8U   29F017A.8L |
  | 90PFTN       90PFTR     |
  |                         |
  |-SWITCH------------------|

  Texas Instruments HT04A
  Fujitsu MB624018 CMOS GATE ARRAY
  Fujitsu MB624019 CMOS GATE ARRAY
  Atmel AT28C16 16K (2K x 8) Parallel EEPROM
  Fujitsu 29F017A-90PFTR 16M (2M x 8) BIT Flash Memory Reverse Pinout (Gachaga Champ card used 29F017-12PFTR instead)
  Fujitsu 29F017A-90PFTN 16M (2M x 8) BIT Flash Memory Standard Pinout

  */

#include "cpu/psx/psx.h"
#include "machine/adc083x.h"
#include "machine/ataintf.h"
#include "machine/bankdev.h"
#include "machine/cr589.h"
#include "machine/ds2401.h"
#include "machine/linflash.h"
#include "machine/k573cass.h"
#include "machine/k573dio.h"
#include "machine/k573mcr.h"
#include "machine/k573msu.h"
#include "machine/k573npu.h"
#include "machine/mb89371.h"
#include "machine/timekpr.h"
#include "machine/upd4701.h"
#include "sound/spu.h"
#include "sound/cdda.h"
#include "video/psx.h"
#include "cdrom.h"

#define VERBOSE_LEVEL ( 0 )

#define ATAPI_CYCLES_PER_SECTOR ( 5000 )  // plenty of time to allow DMA setup etc.  BIOS requires this be at least 2000, individual games may vary.

class ksys573_state : public driver_device
{
public:
	ksys573_state( const machine_config &mconfig, device_type type, const char *tag ) :
		driver_device( mconfig, type, tag ),
		m_analog0(*this, "analog0" ),
		m_analog1(*this, "analog1" ),
		m_analog2(*this, "analog2" ),
		m_analog3(*this, "analog3" ),
		m_pads(*this, "PADS" ),
		m_psxirq(*this, ":maincpu:irq" ),
		m_ata(*this, "ata" ),
		m_h8_response(*this, "h8_response"),
		m_maincpu(*this, "maincpu" ),
		m_ram(*this, "maincpu:ram" ),
		m_flashbank(*this, "flashbank" ),
		m_out1(*this, "OUT1" ),
		m_out2(*this, "OUT2" ),
		m_cd(*this, "CD" ),
		m_upd4701(*this, "upd4701" ),
		m_upd4701_y(*this, "uPD4701_y" ),
		m_upd4701_switches(*this, "uPD4701_switches" ),
		m_stage(*this, "STAGE" ),
		m_gunx(*this, "GUNX" ),
		m_sensor(*this, "SENSOR" ),
		m_encoder(*this, "ENCODER" ),
		m_gunmania_id(*this, "gunmania_id" )
	{
	}

	DECLARE_CUSTOM_INPUT_MEMBER( gn845pwbb_read );
	DECLARE_CUSTOM_INPUT_MEMBER( gunmania_tank_shutter_sensor );
	DECLARE_CUSTOM_INPUT_MEMBER( gunmania_cable_holder_sensor );
	DECLARE_READ16_MEMBER( control_r );
	DECLARE_WRITE16_MEMBER( control_w );
	DECLARE_WRITE16_MEMBER( atapi_reset_w );
	DECLARE_WRITE16_MEMBER( security_w );
	DECLARE_READ16_MEMBER( security_r );
	DECLARE_READ16_MEMBER( ge765pwbba_r );
	DECLARE_WRITE16_MEMBER( ge765pwbba_w );
	DECLARE_READ16_MEMBER( gx700pwbf_io_r );
	DECLARE_WRITE16_MEMBER( gx700pwbf_io_w );
	DECLARE_WRITE16_MEMBER( gunmania_w );
	DECLARE_READ16_MEMBER( gunmania_r );
	DECLARE_DRIVER_INIT( salarymc );
	DECLARE_DRIVER_INIT( pnchmn );
	DECLARE_DRIVER_INIT( ddr );
	DECLARE_DRIVER_INIT( hyperbbc );
	DECLARE_DRIVER_INIT( drmn );
	DECLARE_MACHINE_RESET( konami573 );
	WRITE_LINE_MEMBER( h8_clk_w );
	DECLARE_READ_LINE_MEMBER( h8_d0_r );
	DECLARE_READ_LINE_MEMBER( h8_d1_r );
	DECLARE_READ_LINE_MEMBER( h8_d2_r );
	DECLARE_READ_LINE_MEMBER( h8_d3_r );
	DECLARE_WRITE_LINE_MEMBER( gtrfrks_lamps_b7 );
	DECLARE_WRITE_LINE_MEMBER( gtrfrks_lamps_b6 );
	DECLARE_WRITE_LINE_MEMBER( gtrfrks_lamps_b5 );
	DECLARE_WRITE_LINE_MEMBER( gtrfrks_lamps_b4 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b0 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b1 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b2 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b3 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b4 );
	DECLARE_WRITE_LINE_MEMBER( dmx_lamps_b5 );
	DECLARE_WRITE_LINE_MEMBER( mamboagg_lamps_b3 );
	DECLARE_WRITE_LINE_MEMBER( mamboagg_lamps_b4 );
	DECLARE_WRITE_LINE_MEMBER( mamboagg_lamps_b5 );
	DECLARE_WRITE_LINE_MEMBER( salarymc_lamp_rst );
	DECLARE_WRITE_LINE_MEMBER( salarymc_lamp_d );
	DECLARE_WRITE_LINE_MEMBER( salarymc_lamp_clk );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_red );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_green );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_blue );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_start );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_strobe1 );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_strobe2 );
	DECLARE_WRITE_LINE_MEMBER( hyperbbc_lamp_strobe3 );
	DECLARE_WRITE_LINE_MEMBER( ata_interrupt );
	TIMER_CALLBACK_MEMBER( atapi_xfer_end );
	DECLARE_WRITE8_MEMBER( ddr_output_callback );
	DECLARE_WRITE8_MEMBER( ddrsolo_output_callback );
	DECLARE_WRITE8_MEMBER( drmn_output_callback );
	DECLARE_WRITE8_MEMBER( dmx_output_callback );
	DECLARE_WRITE8_MEMBER( mamboagg_output_callback );
	DECLARE_WRITE8_MEMBER( punchmania_output_callback );
	ADC083X_INPUT_CB(analogue_inputs_callback);

	void cdrom_dma_read( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void cdrom_dma_write( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void sys573_vblank( screen_device &screen, bool vblank_state );
	double m_pad_position[ 6 ];
	required_ioport m_analog0;
	required_ioport m_analog1;
	required_ioport m_analog2;
	required_ioport m_analog3;
	optional_ioport m_pads;

protected:
	virtual void driver_start();

private:
	inline void ATTR_PRINTF( 3,4 ) verboselog( int n_level, const char *s_fmt, ... );
	void update_disc();
	void gx700pwbf_output( int offset, UINT8 data );
	void gx700pwfbf_init( void ( ksys573_state::*output_callback_func )( address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask ) );
	void gn845pwbb_do_w( int offset, int data );
	void gn845pwbb_clk_w( int offset, int data );

	required_device<psxirq_device> m_psxirq;

	required_device<ata_interface_device> m_ata;
	cdrom_file *m_available_cdroms[ 2 ];
	emu_timer *m_atapi_timer;
	int m_atapi_xferbase;
	int m_atapi_xfersize;

	UINT32 m_control;
	UINT16 m_n_security_control;

	required_region_ptr<UINT8> m_h8_response;
	int m_h8_index;
	int m_h8_clk;

	UINT8 m_gx700pwbf_output_data[ 4 ];
	void ( ksys573_state::*m_gx700pwfbf_output_callback )( address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask );

	UINT32 m_stage_mask;
	struct
	{
		int DO;
		int clk;
		int shift;
		int state;
		int bit;
	} m_stage_state[ 2 ];

	int m_salarymc_lamp_bits;
	int m_salarymc_lamp_shift;
	int m_salarymc_lamp_d;
	int m_salarymc_lamp_clk;

	int m_hyperbbc_lamp_red;
	int m_hyperbbc_lamp_green;
	int m_hyperbbc_lamp_blue;
	int m_hyperbbc_lamp_start;
	int m_hyperbbc_lamp_strobe1;
	int m_hyperbbc_lamp_strobe2;
	int m_hyperbbc_lamp_strobe3;

	UINT32 *m_p_n_psxram;

	int m_tank_shutter_position;
	int m_cable_holder_release;

	required_device<psxcpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_flashbank;
	required_ioport m_out1;
	required_ioport m_out2;
	required_ioport m_cd;
	optional_device<upd4701_device> m_upd4701;
	optional_ioport m_upd4701_y;
	optional_ioport m_upd4701_switches;
	optional_ioport m_stage;
	optional_ioport m_gunx;
	optional_ioport m_sensor;
	optional_ioport m_encoder;
	optional_device<ds2401_device> m_gunmania_id;
};

void ATTR_PRINTF( 3,4 )  ksys573_state::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine().describe_context(), buf );
	}
}

static ADDRESS_MAP_START( konami573_map, AS_PROGRAM, 32, ksys573_state )
	AM_RANGE( 0x1f000000, 0x1f3fffff ) AM_DEVICE16( "flashbank", address_map_bank_device, amap16, 0xffffffff )
	AM_RANGE( 0x1f400000, 0x1f400003 ) AM_READ_PORT( "IN0" ) AM_WRITE_PORT( "OUT0" )
	AM_RANGE( 0x1f400004, 0x1f400007 ) AM_READ_PORT( "IN1" )
	AM_RANGE( 0x1f400008, 0x1f40000b ) AM_READ_PORT( "IN2" )
	AM_RANGE( 0x1f40000c, 0x1f40000f ) AM_READ_PORT( "IN3" )
	AM_RANGE( 0x1f480000, 0x1f48000f ) AM_DEVREADWRITE16( "ata", ata_interface_device, read_cs0, write_cs0, 0xffffffff )
	AM_RANGE( 0x1f500000, 0x1f500003 ) AM_READWRITE16( control_r, control_w, 0x0000ffff )    // Konami can't make a game without a "control" register.
	AM_RANGE( 0x1f560000, 0x1f560003 ) AM_WRITE16( atapi_reset_w, 0x0000ffff )
	AM_RANGE( 0x1f5c0000, 0x1f5c0003 ) AM_WRITENOP                // watchdog?
	AM_RANGE( 0x1f600000, 0x1f600003 ) AM_WRITE_PORT( "LAMPS" )
	AM_RANGE( 0x1f620000, 0x1f623fff ) AM_DEVREADWRITE8( "m48t58", timekeeper_device, read, write, 0x00ff00ff )
	AM_RANGE( 0x1f680000, 0x1f68001f ) AM_DEVREADWRITE8( "mb89371", mb89371_device, read, write, 0x00ff00ff )
	AM_RANGE( 0x1f6a0000, 0x1f6a0003 ) AM_READWRITE16( security_r, security_w, 0x0000ffff )
ADDRESS_MAP_END

static ADDRESS_MAP_START( flashbank_map, AS_PROGRAM, 16, ksys573_state )
	AM_RANGE( 0x0000000, 0x03fffff ) AM_DEVREADWRITE8( "29f016a.31m", intelfsh8_device, read, write, 0x00ff )
	AM_RANGE( 0x0000000, 0x03fffff ) AM_DEVREADWRITE8( "29f016a.27m", intelfsh8_device, read, write, 0xff00 )
	AM_RANGE( 0x0400000, 0x07fffff ) AM_DEVREADWRITE8( "29f016a.31l", intelfsh8_device, read, write, 0x00ff )
	AM_RANGE( 0x0400000, 0x07fffff ) AM_DEVREADWRITE8( "29f016a.27l", intelfsh8_device, read, write, 0xff00 )
	AM_RANGE( 0x0800000, 0x0bfffff ) AM_DEVREADWRITE8( "29f016a.31j", intelfsh8_device, read, write, 0x00ff )
	AM_RANGE( 0x0800000, 0x0bfffff ) AM_DEVREADWRITE8( "29f016a.27j", intelfsh8_device, read, write, 0xff00 )
	AM_RANGE( 0x0c00000, 0x0ffffff ) AM_DEVREADWRITE8( "29f016a.31h", intelfsh8_device, read, write, 0x00ff )
	AM_RANGE( 0x0c00000, 0x0ffffff ) AM_DEVREADWRITE8( "29f016a.27h", intelfsh8_device, read, write, 0xff00 )
	AM_RANGE( 0x4000000, 0x7ffffff ) AM_DEVREADWRITE( "pccard1", pccard_slot_device, read_memory, write_memory )
	AM_RANGE( 0x8000000, 0xbffffff ) AM_DEVREADWRITE( "pccard2", pccard_slot_device, read_memory, write_memory )
ADDRESS_MAP_END

static ADDRESS_MAP_START( konami573d_map, AS_PROGRAM, 32, ksys573_state )
	AM_IMPORT_FROM( konami573_map )
	AM_RANGE( 0x1f640000, 0x1f6400ff ) AM_DEVICE16( "k573dio", k573dio_device, amap, 0xffffffff )
ADDRESS_MAP_END

static ADDRESS_MAP_START( konami573a_map, AS_PROGRAM, 32, ksys573_state )
	AM_IMPORT_FROM( konami573_map )
	AM_RANGE( 0x1f640000, 0x1f6400ff ) AM_READWRITE16( gx700pwbf_io_r, gx700pwbf_io_w, 0xffffffff )
ADDRESS_MAP_END

static ADDRESS_MAP_START( fbaitbc_map, AS_PROGRAM, 32, ksys573_state )
	AM_IMPORT_FROM( konami573_map )
	AM_RANGE( 0x1f640000, 0x1f6400ff ) AM_READWRITE16( ge765pwbba_r, ge765pwbba_w, 0xffffffff )
ADDRESS_MAP_END

static ADDRESS_MAP_START( gunmania_map, AS_PROGRAM, 32, ksys573_state )
	AM_IMPORT_FROM( konami573_map )
	AM_RANGE( 0x1f640000, 0x1f6400ff ) AM_READWRITE16( gunmania_r, gunmania_w, 0xffffffff )
ADDRESS_MAP_END

READ16_MEMBER( ksys573_state::control_r )
{
	verboselog( 2, "control_r( %08x, %08x ) %08x\n", offset, mem_mask, m_control );

	return m_control;
}

WRITE16_MEMBER( ksys573_state::control_w )
{
	COMBINE_DATA( &m_control );

	verboselog( 2, "control_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	m_out2->write( data, mem_mask );

	m_flashbank->set_bank( m_control & 0x3f );
}

TIMER_CALLBACK_MEMBER( ksys573_state::atapi_xfer_end )
{
	/// TODO: respect timing of data from ATAPI device.

	m_atapi_timer->adjust( attotime::never );

	address_space &space = m_maincpu->space( AS_PROGRAM );

	for( int i = 0; i < m_atapi_xfersize; i++ )
	{
		UINT32 d = m_ata->read_cs0( space, (UINT32) 0, (UINT32) 0xffff ) << 0;
		d |= m_ata->read_cs0( space, (UINT32) 0, (UINT32) 0xffff ) << 16;

		m_p_n_psxram[ m_atapi_xferbase / 4 ] = d;
		m_atapi_xferbase += 4;
	}

	/// HACK: konami80s only works if you dma more data than requested
	if( ( m_ata->read_cs1( space, (UINT32) 6, (UINT32) 0xffff ) & 8 ) != 0 )
	{
		m_atapi_timer->adjust( m_maincpu->cycles_to_attotime( ( ATAPI_CYCLES_PER_SECTOR * ( m_atapi_xfersize / 64 ) ) ) );
	}
}

WRITE_LINE_MEMBER( ksys573_state::ata_interrupt )
{
	m_psxirq->intin10( state );
}

WRITE16_MEMBER( ksys573_state::atapi_reset_w )
{
	if( !( data & 1 ) )
	{
		m_ata->reset();
	}
}

void ksys573_state::cdrom_dma_read( UINT32 *ram, UINT32 n_address, INT32 n_size )
{
	verboselog( 2, "cdrom_dma_read( %08x, %08x )\n", n_address, n_size );
//  osd_printf_debug( "DMA read: address %08x size %08x\n", n_address, n_size );
}

void ksys573_state::cdrom_dma_write( UINT32 *ram, UINT32 n_address, INT32 n_size )
{
	m_p_n_psxram = ram;

	verboselog( 2, "cdrom_dma_write( %08x, %08x )\n", n_address, n_size );
//  osd_printf_debug( "DMA write: address %08x size %08x\n", n_address, n_size );

	m_atapi_xferbase = n_address;
	m_atapi_xfersize = n_size;
	// set a transfer complete timer ( Note: CYCLES_PER_SECTOR can't be lower than 2000 or the BIOS ends up "out of order" )
	m_atapi_timer->adjust( m_maincpu->cycles_to_attotime( ( ATAPI_CYCLES_PER_SECTOR * ( n_size / 512 ) ) ) );
}

WRITE16_MEMBER( ksys573_state::security_w )
{
	COMBINE_DATA( &m_n_security_control );

	verboselog( 2, "security_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	m_out1->write( data, mem_mask );
}

READ16_MEMBER( ksys573_state::security_r )
{
	UINT16 data = m_n_security_control;
	verboselog( 2, "security_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

void ksys573_state::update_disc()
{
	int cd = m_cd->read();
	cdrom_file *new_cdrom;

	if( m_available_cdroms[ 1 ] != NULL )
	{
		new_cdrom = m_available_cdroms[ cd ];
	}
	else
	{
		new_cdrom = m_available_cdroms[ 0 ];
	}

	atapi_hle_device *image = machine().device<atapi_hle_device>( "ata:0:cr589" );
	if( image != NULL )
	{
		void *current_cdrom = NULL;
		image->GetDevice( &current_cdrom );

		if( current_cdrom != new_cdrom )
		{
			current_cdrom = new_cdrom;

			image->SetDevice( new_cdrom );
		}
	}
}

void ksys573_state::driver_start()
{
	m_atapi_timer = machine().scheduler().timer_alloc( timer_expired_delegate( FUNC( ksys573_state::atapi_xfer_end ),this ) );
	m_atapi_timer->adjust( attotime::never );

	m_available_cdroms[ 0 ] = cdrom_open( get_disk_handle( machine(), ":cdrom0" ) );
	m_available_cdroms[ 1 ] = cdrom_open( get_disk_handle( machine(), ":cdrom1" ) );

	m_n_security_control = 0;
	m_control = 0;

	save_item( NAME( m_n_security_control ) );
	save_item( NAME( m_control ) );
}

MACHINE_RESET_MEMBER( ksys573_state,konami573 )
{
	update_disc();

	m_h8_index = 0;
	m_h8_clk = 0;
}

void ksys573_state::sys573_vblank( screen_device &screen, bool vblank_state )
{
	update_disc();

	/// TODO: emulate the memory controller board
	if( strcmp( machine().system().name, "ddr2ml" ) == 0 )
	{
		/* patch out security-plate error */

		UINT32 *p_n_psxram = (UINT32 *) m_ram->pointer();

		/* install cd */

		/* 801e1540: jal $801e1f7c */
		if( p_n_psxram[ 0x1e1540 / 4 ] == 0x0c0787df )
		{
			/* 801e1540: j $801e1560 */
			p_n_psxram[ 0x1e1540 / 4 ] = 0x08078558;
		}

		/* flash */

		/* 8001f850: jal $80031fd8 */
		if( p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c7f6 )
		{
			/* 8001f850: j $8001f888 */
			p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}
	else if( strcmp( machine().system().name, "ddr2mla" ) == 0 )
	{
		/* patch out security-plate error */

		UINT32 *p_n_psxram = (UINT32 *) m_ram->pointer();

		/* 8001f850: jal $8003221c */
		if( p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c887 )
		{
			/* 8001f850: j $8001f888 */
			p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}
}

// H8 check at startup (JVS related)

WRITE_LINE_MEMBER( ksys573_state::h8_clk_w )
{
	if( m_h8_clk != state )
	{
		if( state )
		{
			if( m_h8_index < m_h8_response.length() - 1 )
			{
				m_h8_index++;
			}
		}

		m_h8_clk = state;
	}
}

READ_LINE_MEMBER( ksys573_state::h8_d0_r )
{
	return ( m_h8_response[ m_h8_index ] >> 0 ) & 1;
}

READ_LINE_MEMBER( ksys573_state::h8_d1_r )
{
	return ( m_h8_response[ m_h8_index ] >> 1 ) & 1;
}

READ_LINE_MEMBER( ksys573_state::h8_d2_r )
{
	return ( m_h8_response[ m_h8_index ] >> 2 ) & 1;
}

READ_LINE_MEMBER( ksys573_state::h8_d3_r )
{
	return ( m_h8_response[ m_h8_index ] >> 3 ) & 1;
}


/*
GE765-PWB(B)A

todo:
  find out what offset 4 is
  fix reel type detection
  find adc0834 SARS

*/

READ16_MEMBER( ksys573_state::ge765pwbba_r )
{
	UINT32 data = 0;

	switch( offset )
	{
	case 0x4c:
	case 0x4d:
		m_upd4701->y_add( m_upd4701_y->read() );
		m_upd4701->switches_set( m_upd4701_switches->read() );

		m_upd4701->cs_w( 0 );
		m_upd4701->xy_w( 1 );

		if( offset == 0x4c )
		{
			m_upd4701->ul_w( 0 );
		}
		else
		{
			m_upd4701->ul_w( 1 );
		}

		data = m_upd4701->d_r( space, 0, 0xffff );
		m_upd4701->cs_w( 1 );
		break;

	default:
		verboselog( 0, "ge765pwbba_r: unhandled offset %08x %08x\n", offset, mem_mask );
		break;
	}

	verboselog( 2, "ge765pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

WRITE16_MEMBER( ksys573_state::ge765pwbba_w )
{
	switch( offset )
	{
	case 0x08:
		break;

	case 0x40:
		output_set_value( "motor", data & 0xff );
		break;

	case 0x44:
		output_set_value( "brake", data & 0xff );
		break;

	case 0x50:
		m_upd4701->resety_w( 1 );
		m_upd4701->resety_w( 0 );
		break;

	default:
		verboselog( 0, "ge765pwbba_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data );
		break;
	}

	verboselog( 2, "ge765pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
}

/*

GX700-PWB(F)

Analogue I/O board

*/

READ16_MEMBER( ksys573_state::gx700pwbf_io_r )
{
	UINT32 data = 0;
	switch( offset )
	{
	case 0x40:
		/* result not used? */
		break;

	case 0x44:
		/* result not used? */
		break;

	case 0x48:
		/* result not used? */
		break;

	case 0x4c:
		/* result not used? */
		break;

	default:
//      printf( "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}

	verboselog( 2, "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

void ksys573_state::gx700pwbf_output( int offset, UINT8 data )
{
	if( m_gx700pwfbf_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 7, 6, 1, 0, 5, 4, 3, 2 };
		for( i = 0; i < 8; i++ )
		{
			int oldbit = ( m_gx700pwbf_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				( this->*m_gx700pwfbf_output_callback )( m_maincpu->space( AS_PROGRAM ), ( offset * 8 ) + i, newbit, 0xff );
			}
		}
	}
	m_gx700pwbf_output_data[ offset ] = data;
}

WRITE16_MEMBER( ksys573_state::gx700pwbf_io_w )
{
	verboselog( 2, "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0x40:
		gx700pwbf_output( 0, data & 0xff );
		break;

	case 0x44:
		gx700pwbf_output( 1, data & 0xff );
		break;

	case 0x48:
		gx700pwbf_output( 2, data & 0xff );
		break;

	case 0x4c:
		gx700pwbf_output( 3, data & 0xff );
		break;

	default:
//      printf( "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
		break;
	}
}

void ksys573_state::gx700pwfbf_init( void ( ksys573_state::*output_callback_func )( address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask ) )
{
	memset( m_gx700pwbf_output_data, 0, sizeof( m_gx700pwbf_output_data ) );

	m_gx700pwfbf_output_callback = output_callback_func;

	save_item( NAME( m_gx700pwbf_output_data ) );
}

/*

GN845-PWB( B )

DDR Stage Multiplexor

*/

#define DDR_STAGE_IDLE ( 0 )
#define DDR_STAGE_INIT ( 1 )

static const int mask[] =
{
	0, 6, 2, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 4,
	0, 4, 0, 6
};

void ksys573_state::gn845pwbb_do_w( int offset, int data )
{
	m_stage_state[ offset ].DO = !data;
}

void ksys573_state::gn845pwbb_clk_w( int offset, int data )
{
	int clk = !data;

	if( clk != m_stage_state[ offset ].clk )
	{
		m_stage_state[ offset ].clk = clk;

		if( clk )
		{
			m_stage_state[ offset ].shift = ( m_stage_state[ offset ].shift >> 1 ) | ( m_stage_state[ offset ].DO << 12 );

			switch( m_stage_state[ offset ].state )
			{
			case DDR_STAGE_IDLE:
				if( m_stage_state[ offset ].shift == 0xc90 )
				{
					m_stage_state[ offset ].state = DDR_STAGE_INIT;
					m_stage_state[ offset ].bit = 0;
					m_stage_mask = 0xfffff9f9;
				}
				break;

			case DDR_STAGE_INIT:
				m_stage_state[ offset ].bit++;
				if( m_stage_state[ offset ].bit < 22 )
				{
					int a = ( ( ( ( ~0x06 ) | mask[ m_stage_state[ 0 ].bit ] ) & 0xff ) << 8 );
					int b = ( ( ( ( ~0x06 ) | mask[ m_stage_state[ 1 ].bit ] ) & 0xff ) << 0 );

					m_stage_mask = 0xffff0000 | a | b;
				}
				else
				{
					m_stage_state[ offset ].bit = 0;
					m_stage_state[ offset ].state = DDR_STAGE_IDLE;

					m_stage_mask = 0xffffffff;
				}
				break;
			}
		}
	}

	verboselog( 2, "stage: %dp data clk=%d state=%d d0=%d shift=%08x bit=%d stage_mask=%08x\n", offset + 1, clk,
		m_stage_state[ offset ].state, m_stage_state[ offset ].DO, m_stage_state[ offset ].shift, m_stage_state[ offset ].bit, m_stage_mask );
}

CUSTOM_INPUT_MEMBER( ksys573_state::gn845pwbb_read )
{
	return m_stage->read() & m_stage_mask;
}

WRITE8_MEMBER( ksys573_state::ddr_output_callback )
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
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

DRIVER_INIT_MEMBER( ksys573_state, ddr )
{
	m_stage_mask = 0xffffffff;
	gx700pwfbf_init( &ksys573_state::ddr_output_callback );

	save_item( NAME( m_stage_mask ) );
}

/* Guitar freaks */

WRITE_LINE_MEMBER( ksys573_state::gtrfrks_lamps_b7 )
{
	output_set_value( "spot left", state );
}

WRITE_LINE_MEMBER( ksys573_state::gtrfrks_lamps_b6 )
{
	output_set_value( "spot right", state );
}

WRITE_LINE_MEMBER( ksys573_state::gtrfrks_lamps_b5 )
{
	output_set_led_value( 0, state ); // start left
}

WRITE_LINE_MEMBER( ksys573_state::gtrfrks_lamps_b4 )
{
	output_set_led_value( 1, state ); // start right
}

/* ddr solo */

WRITE8_MEMBER( ksys573_state::ddrsolo_output_callback )
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

/* drummania */

WRITE8_MEMBER( ksys573_state::drmn_output_callback )
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

DRIVER_INIT_MEMBER( ksys573_state,drmn )
{
	gx700pwfbf_init( &ksys573_state::drmn_output_callback );
}

/* dance maniax */

WRITE8_MEMBER( ksys573_state::dmx_output_callback )
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

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b0 )
{
	output_set_value( "left 2p", state );
}

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b1 )
{
	output_set_led_value( 1, state ); // start 1p
}

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b2 )
{
	output_set_value( "right 2p", state );
}

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b3 )
{
	output_set_value( "left 1p", state );
}

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b4 )
{
	output_set_led_value( 0, state ); // start 2p
}

WRITE_LINE_MEMBER( ksys573_state::dmx_lamps_b5 )
{
	output_set_value( "right 1p", state );
}

/* salary man champ */

WRITE_LINE_MEMBER( ksys573_state::salarymc_lamp_rst )
{
	if( state )
	{
		m_salarymc_lamp_bits = 0;
		m_salarymc_lamp_shift = 0;
	}
}

WRITE_LINE_MEMBER( ksys573_state::salarymc_lamp_d )
{
	m_salarymc_lamp_d = state;
}

WRITE_LINE_MEMBER( ksys573_state::salarymc_lamp_clk )
{
	if( state && !m_salarymc_lamp_clk )
	{
		m_salarymc_lamp_bits++;

		m_salarymc_lamp_shift <<= 1;
		m_salarymc_lamp_shift |= m_salarymc_lamp_d;

		if( m_salarymc_lamp_bits == 16 )
		{
			if( ( m_salarymc_lamp_shift & ~0xe38 ) != 0 )
			{
				verboselog( 0, "unknown bits in salarymc_lamp_shift %08x\n", m_salarymc_lamp_shift & ~0xe38 );
			}

			output_set_value( "player 1 red", ( m_salarymc_lamp_shift >> 11 ) & 1 );
			output_set_value( "player 1 green", ( m_salarymc_lamp_shift >> 10 ) & 1 );
			output_set_value( "player 1 blue", ( m_salarymc_lamp_shift >> 9 ) & 1 );

			output_set_value( "player 2 red", ( m_salarymc_lamp_shift >> 5 ) & 1 );
			output_set_value( "player 2 green", ( m_salarymc_lamp_shift >> 4 ) & 1 );
			output_set_value( "player 2 blue", ( m_salarymc_lamp_shift >> 3 ) & 1 );

			m_salarymc_lamp_bits = 0;
			m_salarymc_lamp_shift = 0;
		}
	}

	m_salarymc_lamp_clk = state;
}

static MACHINE_CONFIG_FRAGMENT( salarymc_cassette_install )
	MCFG_DEVICE_MODIFY( DEVICE_SELF )
	MCFG_KONAMI573_CASSETTE_Y_D5_HANDLER( DEVWRITELINE( ":", ksys573_state, salarymc_lamp_clk ) )
	MCFG_KONAMI573_CASSETTE_Y_D6_HANDLER( DEVWRITELINE( ":", ksys573_state, salarymc_lamp_rst ) )
	MCFG_KONAMI573_CASSETTE_Y_D7_HANDLER( DEVWRITELINE( ":", ksys573_state, salarymc_lamp_d ) )
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER( ksys573_state, salarymc )
{
	m_salarymc_lamp_bits = 0;
	m_salarymc_lamp_shift = 0;
	m_salarymc_lamp_d = 0;
	m_salarymc_lamp_clk = 0;

	save_item( NAME( m_salarymc_lamp_bits ) );
	save_item( NAME( m_salarymc_lamp_shift ) );
	save_item( NAME( m_salarymc_lamp_d ) );
	save_item( NAME( m_salarymc_lamp_clk ) );
}

/* Hyper Bishi Bashi Champ */

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_red )
{
	m_hyperbbc_lamp_red = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_green )
{
	m_hyperbbc_lamp_green = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_blue )
{
	m_hyperbbc_lamp_blue = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_start )
{
	m_hyperbbc_lamp_start = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_strobe1 )
{
	if( state && !m_hyperbbc_lamp_strobe1 )
	{
		output_set_value( "player 1 red", m_hyperbbc_lamp_red );
		output_set_value( "player 1 green", m_hyperbbc_lamp_green );
		output_set_value( "player 1 blue", m_hyperbbc_lamp_blue );
		output_set_value( "player 1 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe1 = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_strobe2 )
{
	if( state && !m_hyperbbc_lamp_strobe2 )
	{
		output_set_value( "player 2 red", m_hyperbbc_lamp_red );
		output_set_value( "player 2 green", m_hyperbbc_lamp_green );
		output_set_value( "player 2 blue", m_hyperbbc_lamp_blue );
		output_set_value( "player 2 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe2 = state;
}

WRITE_LINE_MEMBER( ksys573_state::hyperbbc_lamp_strobe3 )
{
	if( state && !m_hyperbbc_lamp_strobe3 )
	{
		output_set_value( "player 3 red", m_hyperbbc_lamp_red );
		output_set_value( "player 3 green", m_hyperbbc_lamp_green );
		output_set_value( "player 3 blue", m_hyperbbc_lamp_blue );
		output_set_value( "player 3 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe3 = state;
}

static MACHINE_CONFIG_FRAGMENT( hyperbbc_cassette_install )
	MCFG_DEVICE_MODIFY( DEVICE_SELF )
	MCFG_KONAMI573_CASSETTE_Y_D0_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_strobe3 ) ) // line shared with x76f100 sda
	MCFG_KONAMI573_CASSETTE_Y_D1_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_strobe2 ) ) // line shared with x76f100 scl
	MCFG_KONAMI573_CASSETTE_Y_D3_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_strobe1 ) ) // line shared with x76f100 rst
	MCFG_KONAMI573_CASSETTE_Y_D4_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_green ) )
	MCFG_KONAMI573_CASSETTE_Y_D5_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_blue ) )
	MCFG_KONAMI573_CASSETTE_Y_D6_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_red ) )
	MCFG_KONAMI573_CASSETTE_Y_D7_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_start ) )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( hypbbc2p_cassette_install )
	MCFG_DEVICE_MODIFY( DEVICE_SELF )
	MCFG_KONAMI573_CASSETTE_Y_D0_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_strobe2 ) ) // line shared with x76f100 sda
	MCFG_KONAMI573_CASSETTE_Y_D3_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_strobe1 ) ) // line shared with x76f100 rst
	MCFG_KONAMI573_CASSETTE_Y_D4_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_green ) )
	MCFG_KONAMI573_CASSETTE_Y_D5_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_blue ) )
	MCFG_KONAMI573_CASSETTE_Y_D6_HANDLER( DEVWRITELINE( ":", ksys573_state, hyperbbc_lamp_red ) )
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER( ksys573_state, hyperbbc )
{
	m_hyperbbc_lamp_red = 0;
	m_hyperbbc_lamp_green = 0;
	m_hyperbbc_lamp_blue = 0;
	m_hyperbbc_lamp_start = 0;
	m_hyperbbc_lamp_strobe1 = 0;
	m_hyperbbc_lamp_strobe2 = 0;
	m_hyperbbc_lamp_strobe3 = 0;

	save_item( NAME( m_hyperbbc_lamp_red ) );
	save_item( NAME( m_hyperbbc_lamp_green ) );
	save_item( NAME( m_hyperbbc_lamp_blue ) );
	save_item( NAME( m_hyperbbc_lamp_start ) );
	save_item( NAME( m_hyperbbc_lamp_strobe1 ) );
	save_item( NAME( m_hyperbbc_lamp_strobe2 ) );
	save_item( NAME( m_hyperbbc_lamp_strobe3 ) );
}

/* Mambo A Go Go */

WRITE8_MEMBER( ksys573_state::mamboagg_output_callback )
{
	switch( offset )
	{
	case 4:
		output_set_value( "fire lamp left", !data );
		break;
	case 5:
		output_set_value( "fire fan left", !data );
		break;
	case 6:
		output_set_value( "fire fan right", !data );
		break;
	case 7:
		output_set_value( "fire lamp right", !data );
		break;
	case 28:
		output_set_value( "conga left", !data );
		break;
	case 29:
		output_set_value( "conga right", !data );
		break;
	case 31:
		output_set_value( "conga centre", !data );
		break;
	}
}

WRITE_LINE_MEMBER( ksys573_state::mamboagg_lamps_b3 )
{
	output_set_led_value( 0, state ); // start 1p
}

WRITE_LINE_MEMBER( ksys573_state::mamboagg_lamps_b4 )
{
	output_set_value( "select right", state );
}

WRITE_LINE_MEMBER( ksys573_state::mamboagg_lamps_b5 )
{
	output_set_value( "select left", state );
}


/* punch mania */


ADC083X_INPUT_CB(konami573_cassette_xi_device::punchmania_inputs_callback)
{
	ksys573_state *state = machine().driver_data<ksys573_state>();
	double *pad_position = state->m_pad_position;
	int pads = state->m_pads->read();
	for( int i = 0; i < 6; i++ )
	{
		if( ( pads & ( 1 << i ) ) != 0 )
		{
			pad_position[ i ] = 5;
		}
	}

	switch( input )
	{
	case ADC083X_CH0:
		return pad_position[ 0 ]; /* Left Top */
	case ADC083X_CH1:
		return pad_position[ 1 ]; /* Left Middle */
	case ADC083X_CH2:
		return pad_position[ 2 ]; /* Left Bottom */
	case ADC083X_CH3:
		return pad_position[ 3 ]; /* Right Top */
	case ADC083X_CH4:
		return pad_position[ 4 ]; /* Right Middle */
	case ADC083X_CH5:
		return pad_position[ 5 ]; /* Right Bottom */
	case ADC083X_COM:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 5;
}


static MACHINE_CONFIG_FRAGMENT( punchmania_cassette_install )
	MCFG_DEVICE_MODIFY( "adc0838" )
	MCFG_ADC083X_INPUT_CB( konami573_cassette_xi_device, punchmania_inputs_callback )
MACHINE_CONFIG_END


int pad_light[ 6 ];

WRITE8_MEMBER( ksys573_state::punchmania_output_callback )
{
	double *pad_position = m_pad_position;
	char pad[ 7 ];

	switch( offset )
	{
	case 8:
		output_set_value( "select left right", !data );
		break;
	case 9:
		pad_light[ 2 ] = !data;
		output_set_value( "left bottom lamp", !data );
		break;
	case 10:
		pad_light[ 1 ] = !data;
		output_set_value( "left middle lamp", !data );
		break;
	case 11:
		output_set_value( "start lamp", !data );
		break;
	case 12:
		pad_light[ 0 ] = !data;
		output_set_value( "left top lamp", !data );
		break;
	case 13:
		pad_light[ 4 ] = !data;
		output_set_value( "right middle lamp", !data );
		break;
	case 14:
		pad_light[ 3 ] = !data;
		output_set_value( "right top lamp", !data );
		break;
	case 15:
		pad_light[ 5 ] = !data;
		output_set_value( "right bottom lamp", !data );
		break;
	case 16:
		if( data )
		{
			pad_position[ 0 ] = 0; // left top motor +
		}
		break;
	case 17:
		if( data )
		{
			pad_position[ 1 ] = 0; // left middle motor +
		}
		break;
	case 18:
		if( data )
		{
			pad_position[ 1 ] = 5; // left middle motor -
		}
		break;
	case 19:
		if( data )
		{
			pad_position[ 0 ] = 5; // left top motor -
		}
		break;
	case 20:
		if( data )
		{
			pad_position[ 2 ] = 0; // left bottom motor +
		}
		break;
	case 21:
		if( data )
		{
			pad_position[ 3 ] = 5; // right top motor -
		}
		break;
	case 22:
		if( data )
		{
			pad_position[ 3 ] = 0; // right top motor +
		}
		break;
	case 23:
		if( data )
		{
			pad_position[ 2 ] = 5; // left bottom motor -
		}
		break;
	case 26:
		if( data )
		{
			pad_position[ 5 ] = 0; // right bottom motor +
		}
		break;
	case 27:
		if( data )
		{
			pad_position[ 4 ] = 0; // right middle motor +
		}
		break;
	case 30:
		if( data )
		{
			pad_position[ 4 ] = 5; // right middle motor -
		}
		break;
	case 31:
		if( data )
		{
			pad_position[ 5 ] = 5; // right bottom motor -
		}
		break;
	}
	sprintf( pad, "%d%d%d%d%d%d",
		( int )pad_position[ 0 ], ( int )pad_position[ 1 ], ( int )pad_position[ 2 ],
		( int )pad_position[ 3 ], ( int )pad_position[ 4 ], ( int )pad_position[ 5 ] );

	if( pad_light[ 0 ] ) pad[ 0 ] = '*';
	if( pad_light[ 1 ] ) pad[ 1 ] = '*';
	if( pad_light[ 2 ] ) pad[ 2 ] = '*';
	if( pad_light[ 3 ] ) pad[ 3 ] = '*';
	if( pad_light[ 4 ] ) pad[ 4 ] = '*';
	if( pad_light[ 5 ] ) pad[ 5 ] = '*';

	popmessage( "%s", pad );
}

DRIVER_INIT_MEMBER( ksys573_state,pnchmn )
{
	gx700pwfbf_init( &ksys573_state::punchmania_output_callback );
}

/* GunMania */

WRITE16_MEMBER( ksys573_state::gunmania_w )
{
	char s[ 1024 ] = "";

	switch( offset )
	{
	case 0x4c:
		m_gunmania_id->write( ( data >> 5 ) & 1 );
		break;

	case 0x54:
		switch( data & 0xa0 )
		{
		case 0x20:
			strcat( s, "cable holder motor release " );

			m_cable_holder_release = 1;
			break;

		case 0x80:
			strcat( s, "cable holder motor catch " );

			m_cable_holder_release = 0;
			break;

		case 0xa0:
			strcat( s, "cable holder motor stop " );
			break;
		}

		switch( data & 0x50 )
		{
		case 0x10:
			strcat( s, "bullet supply motor rotate " );
			break;

		case 0x40:
			strcat( s, "bullet supply motor reverse " );
			break;

		case 0x50:
			strcat( s, "bullet shutter motor unknown " );
			break;
		}

		switch( data & 0x0a )
		{
		case 0x02:
			strcat( s, "tank shutter motor close " );

			if( m_tank_shutter_position > 0 )
			{
				m_tank_shutter_position--;
			}

			break;

		case 0x08:
			strcat( s, "tank shutter motor open " );

			if( m_tank_shutter_position < 100 )
			{
				m_tank_shutter_position++;
			}

			break;

		case 0x0a:
			strcat( s, "tank shutter motor unknown " );
			break;
		}

		if( ( data & ~0xfa ) != 0 )
		{
			char unknown[ 128 ];
			sprintf( unknown, "unknown bits %08x", data & ~0xfa );
			strcat( s, unknown );
		}

		if( s[ 0 ] != 0 )
		{
//          popmessage( "%s", s );
		}

		break;
	}

	verboselog( 2, "gunmania_w %08x %08x %08x\n", offset, mem_mask, data );
}

CUSTOM_INPUT_MEMBER( ksys573_state::gunmania_tank_shutter_sensor )
{
	if( m_tank_shutter_position == 0 )
	{
		return 1;
	}

	return 0;
}

CUSTOM_INPUT_MEMBER( ksys573_state::gunmania_cable_holder_sensor )
{
	return m_cable_holder_release;
}

READ16_MEMBER( ksys573_state::gunmania_r )
{
	UINT32 data = 0;

	switch( offset )
	{
	case 0x40:
		data = m_gunx->read();
		break;

	case 0x44:
		data = m_sensor->read();
		break;

	case 0x68:
		data = m_encoder->read();
		popmessage( "encoder %04x", data );
		break;
	}

	verboselog( 2, "gunmania_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

/* ADC0834 Interface */

ADC083X_INPUT_CB(ksys573_state::analogue_inputs_callback)
{
	switch( input )
	{
	case ADC083X_CH0:
		return (double)( 5 * m_analog0->read() ) / 255.0;
	case ADC083X_CH1:
		return (double)( 5 * m_analog1->read() ) / 255.0;
	case ADC083X_CH2:
		return (double)( 5 * m_analog2->read() ) / 255.0;
	case ADC083X_CH3:
		return (double)( 5 * m_analog3->read() ) / 255.0;
	case ADC083X_AGND:
		return 0;
	case ADC083X_VREF:
		return 5;
	}

	return 0;
}

static MACHINE_CONFIG_FRAGMENT( cr589_config )
	MCFG_DEVICE_MODIFY( "cdda" )
	MCFG_SOUND_ROUTE( 0, "^^^^lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "^^^^rspeaker", 1.0 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( konami573, ksys573_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konami573_map )

	MCFG_RAM_MODIFY( "maincpu:ram" )
	MCFG_RAM_DEFAULT_SIZE( "4M" )

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( ksys573_state::cdrom_dma_read ), (ksys573_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( ksys573_state::cdrom_dma_write ), (ksys573_state *) owner ) )

	MCFG_MACHINE_RESET_OVERRIDE( ksys573_state, konami573 )

	MCFG_DEVICE_ADD( "mb89371", MB89371, 0 )

	MCFG_DEVICE_ADD( "ata", ATA_INTERFACE, 0 )
	MCFG_ATA_INTERFACE_IRQ_HANDLER( WRITELINE( ksys573_state, ata_interrupt ) )

	MCFG_DEVICE_MODIFY( "ata:0" )
	MCFG_SLOT_OPTION_ADD( "cr589", CR589 )
	MCFG_SLOT_OPTION_MACHINE_CONFIG( "cr589", cr589_config )
	MCFG_SLOT_DEFAULT_OPTION( "cr589" )

	MCFG_DEVICE_ADD( "cassette", KONAMI573_CASSETTE_SLOT, 0 )
	MCFG_KONAMI573_CASSETTE_DSR_HANDLER(DEVWRITELINE( "maincpu:sio1", psxsio1_device, write_dsr ) )

	// onboard flash
	MCFG_FUJITSU_29F016A_ADD( "29f016a.31m" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.27m" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.31l" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.27l" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.31j" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.27j" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.31h" )
	MCFG_FUJITSU_29F016A_ADD( "29f016a.27h" )

	MCFG_DEVICE_ADD( "pccard1", PCCARD_SLOT, 0 )
	MCFG_DEVICE_ADD( "pccard2", PCCARD_SLOT, 0 )

	MCFG_DEVICE_ADD( "flashbank", ADDRESS_MAP_BANK, 0 )
	MCFG_DEVICE_PROGRAM_MAP( flashbank_map )
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS( ENDIANNESS_LITTLE )
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH( 16 )
	MCFG_ADDRESS_MAP_BANK_STRIDE( 0x400000 )

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x200000, XTAL_53_693175MHz )
	MCFG_PSXGPU_VBLANK_CALLBACK( vblank_state_delegate( FUNC( ksys573_state::sys573_vblank ), (ksys573_state *) owner ) )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO( "lspeaker", "rspeaker" )

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	MCFG_M48T58_ADD( "m48t58" )

	MCFG_DEVICE_ADD( "adc0834", ADC0834, 0 )
	MCFG_ADC083X_INPUT_CB( ksys573_state, analogue_inputs_callback )
MACHINE_CONFIG_END

// Variants with additional digital sound board
static MACHINE_CONFIG_DERIVED( k573d, konami573 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( konami573d_map )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_ADD( "k573dio", XTAL_19_6608MHz )
MACHINE_CONFIG_END

// Variants with additional analogue i/o board
static MACHINE_CONFIG_DERIVED( k573a, konami573 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( konami573a_map )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( pccard1_16mb )
	MCFG_DEVICE_MODIFY( "pccard1" )
	MCFG_SLOT_OPTION_ADD( "16mb", LINEAR_FLASH_PCCARD_16MB )
	MCFG_SLOT_DEFAULT_OPTION( "16mb" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( pccard1_32mb )
	MCFG_DEVICE_MODIFY( "pccard1" )
	MCFG_SLOT_OPTION_ADD( "32mb", LINEAR_FLASH_PCCARD_32MB )
	MCFG_SLOT_DEFAULT_OPTION( "32mb" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( pccard2_32mb )
	MCFG_DEVICE_MODIFY( "pccard2" )
	MCFG_SLOT_OPTION_ADD( "32mb", LINEAR_FLASH_PCCARD_32MB )
	MCFG_SLOT_DEFAULT_OPTION( "32mb" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( pccard2_64mb )
	MCFG_DEVICE_MODIFY( "pccard2" )
	MCFG_SLOT_OPTION_ADD( "64mb", LINEAR_FLASH_PCCARD_64MB )
	MCFG_SLOT_DEFAULT_OPTION( "64mb" )
MACHINE_CONFIG_END

// Security eeprom variants
//
// Suffixes are used to select them
//  x = x76f041
//  y = x76f100
//  z = zs01
//
//  i = also use one or two ds2401
//
// Up to two carts can be used

static MACHINE_CONFIG_FRAGMENT( cassx )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_X )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cassxi )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_XI )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cassy )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_Y )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cassyi )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_YI )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cassyyi )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_YI )
	MCFG_SLOT_OPTION_ADD( "install", KONAMI573_CASSETTE_YI )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( casszi )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_ZI )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cassxzi )
	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_SLOT_OPTION_ADD( "game", KONAMI573_CASSETTE_ZI )
	MCFG_SLOT_OPTION_ADD( "install", KONAMI573_CASSETTE_XI )
	MCFG_SLOT_DEFAULT_OPTION( "game" )
MACHINE_CONFIG_END

// Dance Dance Revolution

static MACHINE_CONFIG_DERIVED( ddr, k573a )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddr2ml, k573a )
	MCFG_DEVICE_ADD( "k573mcr", KONAMI_573_MEMORY_CARD_READER, 0 )

	MCFG_FRAGMENT_ADD( pccard1_16mb )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddr3m, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( pccard2_32mb )
	MCFG_FRAGMENT_ADD( cassyyi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddr3mp, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( pccard2_32mb )
	MCFG_FRAGMENT_ADD( cassxzi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddrusa, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( casszi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddr5m, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( pccard2_32mb )
	MCFG_FRAGMENT_ADD( casszi )
MACHINE_CONFIG_END

// Dancing Stage

static MACHINE_CONFIG_DERIVED( dsfdcta, k573a )
	MCFG_FRAGMENT_ADD( pccard2_32mb )
	MCFG_FRAGMENT_ADD( cassyyi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dsftkd, k573a )
	MCFG_FRAGMENT_ADD( cassyi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dsfdr, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( cassxzi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dsem, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddr_output_callback ) )

	MCFG_FRAGMENT_ADD( cassxi )
MACHINE_CONFIG_END

// Dance Dance Revolution Solo

static MACHINE_CONFIG_DERIVED( ddrsolo, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddrsolo_output_callback ) )

	MCFG_FRAGMENT_ADD( cassyi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddrs2k, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddrsolo_output_callback ) )

	MCFG_FRAGMENT_ADD( cassyyi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ddr4ms, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, ddrsolo_output_callback ) )

	MCFG_FRAGMENT_ADD( pccard2_32mb )
	MCFG_FRAGMENT_ADD( cassxzi )
MACHINE_CONFIG_END

// DrumMania

static MACHINE_CONFIG_DERIVED( drmn, k573a )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( drmn2m, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, drmn_output_callback ) )

	MCFG_FRAGMENT_ADD( cassxzi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( drmn4m, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, drmn_output_callback ) )

	MCFG_FRAGMENT_ADD( casszi )

	MCFG_DEVICE_ADD( "k573msu", KONAMI_573_MULTI_SESSION_UNIT, 0 )
MACHINE_CONFIG_END

// Guitar Freaks

static MACHINE_CONFIG_DERIVED( gtrfrks, k573a )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gtrfrk2m, k573a )
	MCFG_FRAGMENT_ADD( cassyi )
	MCFG_FRAGMENT_ADD( pccard1_32mb ) // HACK: The installation tries to check and erase 32mb but only flashes 16mb.
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gtrfrk3m, k573d )
	MCFG_FRAGMENT_ADD( cassxzi )
	MCFG_FRAGMENT_ADD( pccard1_16mb )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gtrfrk5m, k573d )
	MCFG_FRAGMENT_ADD( casszi )
	MCFG_FRAGMENT_ADD( pccard1_16mb )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gtrfrk7m, k573d )
	MCFG_FRAGMENT_ADD( casszi )
	MCFG_FRAGMENT_ADD( pccard1_32mb )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gtfrk10mb, gtrfrk7m )
	MCFG_DEVICE_ADD( "k573npu", KONAMI_573_NETWORK_PCB_UNIT, 0 )
MACHINE_CONFIG_END

// Miscellaneous

static MACHINE_CONFIG_DERIVED( konami573x, konami573 )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fbaitbc, konami573 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( fbaitbc_map )

	MCFG_UPD4701_ADD( "upd4701" )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hyperbbc, konami573 )
	MCFG_FRAGMENT_ADD( cassy ) // The game doesn't check the security chip

	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "game", hyperbbc_cassette_install )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hypbbc2p, konami573 )
	MCFG_FRAGMENT_ADD( cassy )

	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "game", hypbbc2p_cassette_install )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( salarymc, konami573 )
	MCFG_FRAGMENT_ADD( cassyi )

	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "game", salarymc_cassette_install )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gchgchmp, konami573 )
	MCFG_FRAGMENT_ADD( pccard1_16mb )
	MCFG_FRAGMENT_ADD( cassx )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pnchmn, konami573 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( konami573a_map )

	MCFG_FRAGMENT_ADD( cassxi )
	MCFG_FRAGMENT_ADD( pccard1_32mb )

	MCFG_DEVICE_MODIFY( "cassette" )
	MCFG_DEVICE_CARD_MACHINE_CONFIG( "game", punchmania_cassette_install )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pnchmn2, pnchmn )
	MCFG_FRAGMENT_ADD( pccard2_64mb )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gunmania, konami573 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( gunmania_map )

	MCFG_DS2401_ADD( "gunmania_id" )
	MCFG_FRAGMENT_ADD( pccard2_32mb )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dmx, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, dmx_output_callback ) )

	MCFG_FRAGMENT_ADD( casszi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mamboagg, k573d )
	MCFG_DEVICE_MODIFY( "k573dio" )
	MCFG_KONAMI_573_DIGITAL_IO_BOARD_OUTPUT_CALLBACK( WRITE8( ksys573_state, mamboagg_output_callback ) )

	MCFG_FRAGMENT_ADD( casszi )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mamboagga, mamboagg )
	MCFG_DEVICE_ADD( "k573npu", KONAMI_573_NETWORK_PCB_UNIT, 0 )
MACHINE_CONFIG_END


static INPUT_PORTS_START( konami573 )
	PORT_START( "IN0" )
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "OUT0" )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "adc0834", adc083x_device, cs_write )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "adc0834", adc083x_device, clk_write )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "adc0834", adc083x_device, di_write )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, h8_clk_w )

	PORT_START( "IN1" )
	PORT_DIPNAME( 0x00000001, 0x00000001, "Unused 1" ) PORT_DIPLOCATION( "DIP SW:1" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Screen Flip" ) PORT_DIPLOCATION( "DIP SW:2" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000000, "V-Flip" )
	PORT_DIPNAME( 0x00000004, 0x00000004, "Unused 2" ) PORT_DIPLOCATION( "DIP SW:3" )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000000, "Start Up Device" ) PORT_DIPLOCATION( "DIP SW:4" )
	PORT_DIPSETTING(          0x00000008, "CD-ROM Drive" )
	PORT_DIPSETTING(          0x00000000, "Flash ROM" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, h8_d0_r )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, h8_d1_r )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, h8_d2_r )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, h8_d3_r )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, read_line_adc083x_do )
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, read_line_adc083x_sars )
//  PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x00001000, 0x00001000, "Network?" )
	PORT_CONFSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( On ) )
//  PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, read_line_ds2401 )
//  PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "adc0834", adc083x_device, do_read )
//  PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, read_line_secflash_sda )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* skip hang at startup */
//  PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "pccard1", pccard_slot_device, read_line_inserted )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "pccard2", pccard_slot_device, read_line_inserted )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
//  PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "OUT1" ) // security_w
	PORT_BIT( 0xffffff00, IP_ACTIVE_HIGH, IPT_OUTPUT )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d0 )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d2 )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d3 )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d4 )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d5 )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d6 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_d7 )

	PORT_START( "OUT2" ) // control_w
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( "cassette", konami573_cassette_slot_device, write_line_zs01_sda )

	PORT_START( "IN2" )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) /* skip init? */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 ) /* skip init? */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 ) /* skip init? */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 ) /* skip init? */

	PORT_START( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 2 )
//  PORT_BIT( 0xf0fff0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "LAMPS" )
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_OUTPUT )

	PORT_START( "analog0" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "analog1" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "analog2" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "analog3" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START( "CD" )
	PORT_CONFNAME( 1, 0, "CD" )
	PORT_CONFSETTING( 0, "1" )
	PORT_CONFSETTING( 1, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitbc )
	PORT_INCLUDE( konami573 )

	PORT_START( "uPD4701_y" )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_MINMAX( 0, 0xfff ) PORT_SENSITIVITY( 15 ) PORT_KEYDELTA( 8 ) PORT_RESET

	PORT_START( "uPD4701_switches" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER( 1 )
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitmc )
	PORT_INCLUDE( fbaitbc )

	PORT_MODIFY( "analog0" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX( 0x20,0xdf ) PORT_SENSITIVITY( 30 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 1 ) PORT_REVERSE

	PORT_MODIFY( "analog1" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX( 0x20,0xdf ) PORT_SENSITIVITY( 30 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 1 )
INPUT_PORTS_END

static INPUT_PORTS_START( ddr )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER( DEVICE_SELF, ksys573_state,gn845pwbb_read, NULL )

	PORT_START( "STAGE" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) /* multiplexor */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER( 1 )    /* multiplexor */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER( 1 )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 2 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 2 ) /* multiplexor */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER( 2 )    /* multiplexor */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( ddrsolo )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Left 1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Right 1" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Up 1" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Down 1" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Up-Left 2" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Left 2" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Down 2" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Up-Left 1" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Up-Right 1" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Up 2" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Right 2" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Up-Right 2" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* P2 START */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Select L" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Select R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( gtrfrks )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* SERVICE1 */

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Effect 1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Effect 2" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Pick" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Wailing" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button R" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button G" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button B" )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Effect 1" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Effect 2" )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Pick" )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Wailing" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Button R" )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Button G" )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Button B" )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* P1 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */

	PORT_MODIFY( "LAMPS" )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, gtrfrks_lamps_b7 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, gtrfrks_lamps_b6 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, gtrfrks_lamps_b5 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, gtrfrks_lamps_b4 )
INPUT_PORTS_END

static INPUT_PORTS_START( dmx )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor D1 L" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor D1 R" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Select L" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "P1 Select R" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor U L" ) /* P1 BUTTON1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor U R" ) /* P1 BUTTON2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor D1 L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor D1 R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 2 ) PORT_NAME( "P2 Select L" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 2 ) PORT_NAME( "P2 Select R" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor U L" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor U R" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor D0 L" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "D-Sensor D0 R" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor D0 L" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 2 ) PORT_NAME( "D-Sensor D0 R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */

	PORT_MODIFY( "LAMPS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b0 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b2 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b3 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b4 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, dmx_lamps_b5 )
INPUT_PORTS_END

static INPUT_PORTS_START( drmn )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* COIN2 */

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "High Tom" ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "Low Tom" ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "Hi-Hat" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "Snare" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "Cymbal" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "Bass Drum" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select R" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( gunmania )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "Bullet Tube-1 Sensor" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "Bullet Tube-2 Sensor" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "Safety Sensor Under" )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER( DEVICE_SELF,ksys573_state,gunmania_tank_shutter_sensor, NULL )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x0d000b00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER( DEVICE_SELF,ksys573_state,gunmania_cable_holder_sensor, NULL )

	PORT_START( "GUNX" )
	PORT_BIT( 0x7f, 0x2f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_MINMAX( 0x00,0x5f ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( "gunmania_id", ds2401_device, read )

	PORT_START( "GUNY" )
	PORT_BIT( 0x7f, 0x1f, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR( Y, 1.0, 0.0, 0 ) PORT_MINMAX( 0x00,0x3f ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 )

	PORT_START( "SENSOR" )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER( 1 ) PORT_NAME( "Safety Sensor Front" )

	PORT_START( "ENCODER" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_Q )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_W )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_R )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_T )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_Y )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_U )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER( 4 ) PORT_CODE( KEYCODE_I )
INPUT_PORTS_END

static INPUT_PORTS_START( hndlchmp )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "analog0" )
	PORT_BIT( 0xff, 0xc0, IPT_PEDAL ) PORT_MINMAX( 0xc0, 0xf0 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 20 ) PORT_PLAYER( 2 )

	PORT_MODIFY( "analog1" )
	PORT_BIT( 0xff, 0xc0, IPT_PEDAL ) PORT_MINMAX( 0xc0, 0xf0 ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 20 ) PORT_PLAYER( 1 )

	PORT_MODIFY( "analog2" )
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX( 0x48, 0xb7 ) PORT_SENSITIVITY( 25 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 2 )

	PORT_MODIFY( "analog3" )
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX( 0x48, 0xb7 ) PORT_SENSITIVITY( 25 ) PORT_KEYDELTA( 30 ) PORT_PLAYER( 1 )
INPUT_PORTS_END

static INPUT_PORTS_START( hyperbbc )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )

	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 3 ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 3 ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START3 ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 3 ) /* P1 DOWN */
INPUT_PORTS_END

static INPUT_PORTS_START( hypbbc2p )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 ) /* P1 UP */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */
INPUT_PORTS_END

static INPUT_PORTS_START( mamboagg )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 1 (Top Right)" ) /* COIN2 */

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 3 (Middle Right)" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 1 (Top Right)" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 2 (Bottom Left)" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 1 (Top Left)" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 3 (Bottom Right)" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 2 (Bottom Left)" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 3 (Bottom Right)" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 1 (Top Left)" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "Centre Pad 2 (Middle Left)" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 2 (Bottom Left)" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 1 (Top Left)" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 3 (Bottom Right)" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 1 (Top Right)" ) /* P2 START */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 3 (Middle Right)" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "Left Pad 2 (Middle Left)" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 3 (Middle Right)" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "Right Pad 2 (Middle Left)" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */

	PORT_MODIFY( "LAMPS" )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, mamboagg_lamps_b3 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, mamboagg_lamps_b4 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER( DEVICE_SELF, ksys573_state, mamboagg_lamps_b5 )
INPUT_PORTS_END

static INPUT_PORTS_START( pnchmn )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select L" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select R" ) /* P2 RIGHT */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "Skip Check" )
	PORT_BIT( 0x00005ffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */

	PORT_START( "PADS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "Top Left" )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "Middle Left" )
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "Bottom Left" )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "Top Right" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "Middle Right" )
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "Bottom Right" )
INPUT_PORTS_END

static INPUT_PORTS_START( gchgchmp )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER( 1 ) /* P1 LEFT */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER( 1 ) /* P1 RIGHT */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER( 1 ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER( 1 ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER( 1 )/* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 3 */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

#define SYS573_BIOS_A \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_SYSTEM_BIOS( 0, "std",        "Standard" ) \
	ROMX_LOAD( "700a01.22g",   0x0000000, 0x080000, CRC(11812ef8) SHA1(e1284add4aaddd5337bd7f4e27614460d52b5b48), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 1, "gchgchmp",   "Found on Gachagachamp" ) \
	ROMX_LOAD( "700a01(gchgchmp).22g",  0x000000,  0x080000, CRC(39ebb0ca) SHA1(9aab8c637dd2be84d79007e52f108abe92bf29dd), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 2, "dsem2",      "Found on Dancing Stage Euro Mix 2" ) \
	ROMX_LOAD( "700b01.22g",   0x0000000, 0x080000, CRC(6cf852af) SHA1(a2421d0a494892c0e71003c96995ce8f945064dd), ROM_BIOS(3) ) \
	ROM_REGION( 0x40, "h8_response", 0 ) \
	ROMX_LOAD( "h8a01.bin",    0x000000, 0x000040, CRC(131e0359) SHA1(967f66578ebc0cf6b044d71af09b59bce1f4a1d0), ROM_BIOS(1) ) \
	ROMX_LOAD( "h8a01.bin",    0x000000, 0x000040, CRC(131e0359) SHA1(967f66578ebc0cf6b044d71af09b59bce1f4a1d0), ROM_BIOS(2) ) \
	ROMX_LOAD( "h8b01.bin",    0x000000, 0x000040, CRC(508b057d) SHA1(779177e6312ef272483eeb64a5e84bbae6e301f2), ROM_BIOS(3) )

// BIOS
ROM_START( sys573 )
	SYS573_BIOS_A
ROM_END

// Games
ROM_START( bassangl )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge765ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ee1b32a7) SHA1(c0f6b14b054f5a95ce474e794a3e0ca78faac681) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765jaa02", 0, SHA1(dfcf62581e0d0e994945cc2c37ef86827d511628) )
ROM_END

ROM_START( bassang2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc865ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(095cbfb5) SHA1(529ce0a7b0986cf7e64c37f466d6c2dac95cea7f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865jaa02", 0, BAD_DUMP SHA1(b98d9aa54f13aa73bea580d6494cb6a7f3217be3) )
ROM_END

ROM_START( cr589fw )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700b04", 0, BAD_DUMP SHA1(2f65f62eb7ae202153a8544989675989ed33316f) )
ROM_END

ROM_START( cr589fwa )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700a04", 0, BAD_DUMP SHA1(554481f48eeb5daf8b4e7be2d66840d6c8454a52) )
ROM_END

ROM_START( darkhleg )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx706ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(72b42574) SHA1(79dc959f0ce95ccb9ac0dbf0a72aec973e91bc56) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "706jaa02", 0, SHA1(10101952fad80b7a10b1299158081bf86ce8cbe6) )
ROM_END

ROM_START( ddrextrm )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc36ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c1601287) SHA1(929691a78f7bb6dd830f832f301116df0da1619b) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc36ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c36jaa02", 0, BAD_DUMP SHA1(edeb45fff0e66151b1ba2fd67542064ccddb031e) )
ROM_END

ROM_START( ddru )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(c9e7fced) SHA1(aac4dde100091bc64d397f53484a0ffbf68b8101) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845uaa02", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( ddrj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc845jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(a16f42b8) SHA1(da4f1eb3eb2b28cb3a0bc74bb9b9945970f56ac2) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jba02", 0, SHA1(e4be989f6a655857af8e7336c9a7acf82e51f123) )
ROM_END

ROM_START( ddrja )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jaa02", 0, BAD_DUMP SHA1(37ca16be25bee39a5692dee2fa5f0fa0addfaaca) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jaa01", 0, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddrjb )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jab02", 0, SHA1(bac74acaffd9d00e4105e13f32492f5d0fc5a2e1) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jab01", 0, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddra )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(327c4851) SHA1(f0939224af706fd103a67aae9c96518c1db90ac9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845aaa02", 0, SHA1(9b786de9b1085009c088de0d40425976c1f8df7b) )
ROM_END

ROM_START( ddr2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn896ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "896jaa01", 0, BAD_DUMP SHA1(f802a0e2ba0147eb71c54d92af409c3010a5715f) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "ge984a01(ddr)", 0, SHA1(badd15656f2316f81b0a45026b5ef10287d1480b) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2ml )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "885jab01", 0, SHA1(c2bbb9e2e6f34e07f57e7076726af81df39f55c9) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(f02bb09f41533c6ec496a662d815e85b304fcc72) )
ROM_END

ROM_START( ddr2mla )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(f02bb09f41533c6ec496a662d815e85b304fcc72) )
ROM_END

ROM_START( ddr3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887aaa02", 0, SHA1(6f9a0e9dd046a1fc0c81be9eeb45c136574a4472) )
ROM_END

ROM_START( ddr3mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(3a377cec) SHA1(5bf3107a89547bd7697d9f0ab8f67240e101a559) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(2f633432) SHA1(bce44f20a5a7318af6aea4fdfa8af64ddb76047c) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887jaa02", 0, SHA1(8736818f42822f77e3484ea46a9e63faa7f8517a) )
ROM_END

ROM_START( ddr3mk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kba02", 0, SHA1(9f2c6a4e7ad0de44295dc09b9b054afb044238a9) )
ROM_END

ROM_START( ddr3mka )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kaa02", 0, SHA1(d002f2c98c012d67ad0587553e1d0f45c0ae470e) )
ROM_END

ROM_START( ddr3mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea22ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ef370ff7) SHA1(cb7a043f8bfa535e54ae9af728031d1018ed0734) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca22ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(6883c82c) SHA1(6fef1dc7150066eee427db685b6c5fb350b7768d) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a22jaa02", 0, SHA1(dc3c1223882716d47b4f4db45b5dd2e988cba64c) )
ROM_END

ROM_START( ddr4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea33aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(7bd2a24f) SHA1(62c73a54c4ed7697cf81ddbf3d13d4b0ca827be5) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33aa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6feb2bd) SHA1(dfd5bd532338849289e2e4c155c0ca86e79b9ae5) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aaa02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a33jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e5230867) SHA1(44aea9ccc90d81e7f41e5e9a62b28fcbdd75363b) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a33jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jaa02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4ms )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea33ab.u1",   0x000000, 0x000224, BAD_DUMP CRC(32fb3d13) SHA1(3ca6c77438f96b13d2c05f13a10fcff89a1403a2) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33ab.u1",   0x000000, 0x001014, BAD_DUMP CRC(312ac13f) SHA1(05d733edc03cfc5ea03db6c683f59ed6ff860b5a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aba02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4msj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a33jba.u1",    0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(00e4b531) SHA1(f421fc33642c5a3cd89fb14dc8cd601bdddd1f55) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a33jba.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jba02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea34ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca34ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e9b6ce56) SHA1(f040fba2b2b446baa840026dcd10f9785f8cc0a3) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gca34ja.22h",  0x000000, 0x002000, CRC(80575c1f) SHA1(a0594ca0f75bc7d49b645e835e9fa48a73c3c9c7) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jaa02", 0, SHA1(c33e43192ce49845f8901c505f1c7867bc643a0b) )
ROM_END

ROM_START( ddr4mps )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea34jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca34jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(0c717300) SHA1(00d21f39fe90494ffec2f8799767cc46a9cd2b00) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gca34jb.22h",  0x000000, 0x002000, CRC(bc6c8bd7) SHA1(10ceec5c7bc5ca9fca88f3c083a7d97012982079) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jba02", 0, BAD_DUMP SHA1(c33e43192ce49845f8901c505f1c7867bc643a0b) ) // Check if there was a separate CD created for solo cabinets.
ROM_END

ROM_START( ddr5m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca27ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ec526036) SHA1(f47d94d19268fdfa3ae9d42db9f2e2f9be318f2b) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca27ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a27jaa02", 0, SHA1(70465669dfd48abf806cb58b2410ff4f1781f5f1) )
ROM_END

ROM_START( ddrbocd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "892jaa01", 0, BAD_DUMP SHA1(46ace0feef48a2a6643c3cb4ac9164ba0beeea94) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddrs2k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(36d18e2f) SHA1(e976047dfbee62de9ad9e5de8e7629a24c29d581) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(21073a3e) SHA1(afa12404ceb462b9016a41c40775da87aa09cfeb) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905aaa02", 0, BAD_DUMP SHA1(1fc0f3fcc7d5d23711967023ff02c1fc76479024) )
ROM_END

ROM_START( ddrmax )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb19ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(2255626a) SHA1(cb70c4b551265ffc6cc41f7bd2678696e8067060) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb19ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b19jaa02", 0, SHA1(fe8a6731a2163fe7864cd3c4457034768eb98caa) )
ROM_END

ROM_START( ddrmax2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb20ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(fb7e0f58) SHA1(e6da23257a2a2ba7c69e817a91a0a8864f009386) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb20ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b20jaa02", 0, SHA1(ef6579192b86cfea08debe82f54fc4aae5985c92) )
ROM_END

ROM_START( ddrs2kj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(a077b0a1) SHA1(8f247b38c933a104a325ebf1f1691ef260480e1a) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(b7a104b0) SHA1(0f6901e41640f729f8a084a33148a9b900475594) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge905ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905jaa02", 0, SHA1(a78cf628fb2ba823e1ca35cbd611938273ab82ac) )
ROM_END

ROM_START( ddrsbm )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq894ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(cc3a47de) SHA1(f6e2e101870370b1e295a4a9ed546aa2d8bc2010) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq894ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "894jaa02", 0, SHA1(d6872078a87ee00280a627675540676fb8b10f60) )
ROM_END

ROM_START( ddrusa )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gka44ua.u1",   0x000000, 0x001014, BAD_DUMP CRC(2ef7c4f1) SHA1(9004d27179ece86883d01b3e6bbfeebc1b478d57) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gka44ua.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a44uaa02", 0, BAD_DUMP SHA1(2cdbe1c62d16a2be65adb7e11331fce5c8e45504) )
ROM_END

ROM_START( drmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(7dca0b3f) SHA1(db6d5c527e2a99133b516e01433024d3173848c6) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "881jad01", 0, BAD_DUMP SHA1(7d9d47bef636dbaa8d578f34ea9489e349d3d6df) ) // upgrade or bootleg?

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "881jaa02", 0, NO_DUMP )
ROM_END

ROM_START( drmn2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )
ROM_END

ROM_START( drmn2mpu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "912za01",  0, BAD_DUMP SHA1(033a310006efe164cc6a8276de42a5d555f9fea9) )
ROM_END

ROM_START( drmn3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a23jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(90e544fa) SHA1(1feb617c36bad41aa720a6e5d3ec9e5cb2030567) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca23ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(5af1b5da) SHA1(cf862ef9ab60e8da89af96266943137827e4a261) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a23jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca23ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23jaa02", 0, BAD_DUMP SHA1(89e365f61a4db889621d7d9d9917bcfa2c09704e) )
ROM_END

ROM_START( drmn4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea25jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(356bbbf4) SHA1(a20a8fcaed2dce50451346b1683739c96067feb1) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.31m", 0x000000, 0x200000, CRC(a0dd0ef4) SHA1(be4c1d3f2eb3c484b515be12b692c30cc780c36c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.27m", 0x000000, 0x200000, CRC(118fa45a) SHA1(6bc6129e328f6f97a27b9f524066297b29efff5a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea25jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a25jaa02", 0, BAD_DUMP SHA1(8a0b761d1c282d927e2daf92519654a1c91ee1ab) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "a25jba02", 0, BAD_DUMP SHA1(5f4aae359da610352c1004cfa1a32064d8f55d0e) )
ROM_END

ROM_START( drmn5m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb05jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(6b629d68) SHA1(d01ef0677cd72c05f5f354fc6c4d9022b3506c1e) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb05jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b05jaa02", 0, BAD_DUMP SHA1(7a6e7940d1441cff1d9be1bc3affc029fe6dc9e4) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b05jba02", 0, BAD_DUMP SHA1(822149db553ca78ad8174719a657dbbd2776b922) )
ROM_END

ROM_START( drmn6m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb16jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6933041) SHA1(1839bb99d2db9413c58a2ed95e9039d2c7dd62ba) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.31m",  0x000000, 0x200000, CRC(19de3e53) SHA1(bbb7a247bdd617a124330a946c2e8dd565b2a09c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.27m",  0x000000, 0x200000, CRC(5696e133) SHA1(aad39cc25ce5279adac8a10fb10158f4f4418c0a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb16jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b16jaa02", 0, BAD_DUMP SHA1(fa0862a9bd3a48d4f6e7b44b11ad387acc05037e) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b16jba02", 0, BAD_DUMP SHA1(07de74a3ca384407d99c433110085208a458653e) )
ROM_END

ROM_START( drmn7m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc07jba.u1",   0x000000, 0x001014, BAD_DUMP CRC(8d9bcf10) SHA1(3d486df924ba41669675d62982396aebf8d12052) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.31m",  0x000000, 0x200000, CRC(7120d1ce) SHA1(4df9828150120762b99c5b212bc7a91b0d525bce) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.27m",  0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc07jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c07jca02", 0, SHA1(a81a35360933ab8a7630cf5e8a8c6988714cfa0d) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jda02", 0, BAD_DUMP SHA1(7c22ebbda11bdaf85c3441d7a6f3497994cd957f) )
ROM_END

ROM_START( drmn7ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc07jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(b675b39b) SHA1(9639db913821641cee619d7cc520de5d0c3ae7fa) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.31m",  0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.27m",  0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc07jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c07jaa02", 0, BAD_DUMP SHA1(96c410745d1fd14059bf11987655ed998a9b79dd) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jba02", 0, BAD_DUMP SHA1(25e1a3ff7886c409d16e40ca1798b01b11546755) )
ROM_END

ROM_START( drmn8m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc38jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(aaa03630) SHA1(4976b0c2e1b4458840a165bd889861d62289ad89) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc38jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c38jaa02", 0, BAD_DUMP SHA1(d19ae541557405a4484145f4237f3c868375c72e) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c38jba02", 0, BAD_DUMP SHA1(d963064678978d489474d1ca22c1f249c6f60232) )
ROM_END

ROM_START( drmn9m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd09jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(a1201529) SHA1(4a82f2ee9b049a16c00b7dcd905c43c1a06d60ee) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd09jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d09jaa02", 0, BAD_DUMP SHA1(33f3e48ed5a8becd8c4714413e454328d8d5baae) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d09jba02", 0, NO_DUMP )
ROM_END

ROM_START( drmn10m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd40jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(ef0983a7) SHA1(06127b9fd786eca64eea50c40f7f73717b631e59) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd40jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d40jaa02", 0, BAD_DUMP SHA1(68b2038f0cd2d461f608945d1e243f2b6979efaa) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d40jba02", 0, BAD_DUMP SHA1(0ded9e0a6c77b181e7b6beb1dbdfa17dee4acd90) )
ROM_END

ROM_START( dmx )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge874ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c5536373) SHA1(1492221f7dd9485f7745ecb0a982a88c8e768e53) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge874ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874jaa", 0, BAD_DUMP SHA1(3338a784efdca4f8bdcc83d2c9a6bbe7f7046d5c) )
ROM_END

ROM_START( dmx2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ecc75eb7) SHA1(af66ced28ba5e79ae32ae0ef12d2ebe4207f3822) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a39jaa02", 0, BAD_DUMP SHA1(3d021448df857c12f6d46a20e14ae0fc6d342dcc) )
ROM_END

ROM_START( dmx2majp )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca38ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(99a746b8) SHA1(333236e59a707ecaf840a66f9b947ceade2cf2c9) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.31m",  0x000000, 0x200000, CRC(a0f54ab5) SHA1(a5ae67d7619393779c79a2e227cac0675eeef538) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.27m",  0x000000, 0x200000, CRC(6c3934b8) SHA1(f0e4a692b6caaf60fefaec87fd23da577439f69d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca38ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a38jaa02", 0, SHA1(27fbecefb634ce282ca3bf09500c0c9e8155a7ef) )
ROM_END

ROM_START( dncfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gk874ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(7a6f4672) SHA1(2e009e57760e92f48070a69cff5597c37a4783a2) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gk874ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874kaa", 0, BAD_DUMP SHA1(4d1e843417ea96635eeba0cef944e83fdb72565c) )
ROM_END

ROM_START( dsem )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge936ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(0f5b7ae3) SHA1(646dd49da1216cc2d3d6920bc9b3447d55ebfbf0) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge936ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "936eaa", 0, BAD_DUMP SHA1(7cacc15ae065d47af31f1008374ec8241dba0d55) )
ROM_END

ROM_START( dsem2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gkc23ea.u1",   0x000000, 0x001014, BAD_DUMP CRC(aec2421a) SHA1(5ea9e9ce6161ebc99a50db0b7304385511bd4553) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gkc23ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c23eaa02", 0, BAD_DUMP SHA1(46868c97530db5be1b43ffa32744e3e12495c243) )
ROM_END

ROM_START( dsfdct )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ja_gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(08a60147) SHA1(0d39dca5e9e17fff0e64f296c8416e4ca23fdc1b) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc910jc.u1",   0x000000, 0x000084, BAD_DUMP CRC(3c1ca973) SHA1(32211a72e3ac88b2723f82dac0b26f93031b3a9c) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ja_gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc910jc.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jca02", 0, BAD_DUMP SHA1(0c868f3c9f696d291e8f27687e3ad83e453a4894) )
ROM_END

ROM_START( dsfdcta )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc910ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(59a23808) SHA1(fcff1c68ff6cfbd391ac997a40fb5253fc62de82) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc910ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jaa02", 0, BAD_DUMP SHA1(70851c383e3876c4a697a99706fbaae2dafcb0e0) )
ROM_END

ROM_START( dsfdr )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea37ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(5321055e) SHA1(d06b0dca9caba8249d71340469ad9083b02fd087) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca37ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(b6d9e7f9) SHA1(bc5f491de53a96d46745df09bc94e7853052296c) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea37ja.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca37ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a37jaa02", 0, BAD_DUMP SHA1(c6a23b910e884aa0d4afc388dbc8379e0d09611a) )
ROM_END

ROM_START( dsftkd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "884jaa02", 0, BAD_DUMP SHA1(80f02fcb7ea5b6394a2a58f12b73d87a1826d7f4) )
ROM_END

ROM_START( dstage )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845eb.u1",  0x000000, 0x000084, BAD_DUMP CRC(82b52af5) SHA1(3fb9efe76439fa17a9a759aaebc3dc066b432947) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn884eb.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "gc845eba", 0, BAD_DUMP SHA1(0b7b100ceb37ac30cc1d309e5fe11fde5e1192d0) )
ROM_END

ROM_START( dstagea )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(db643af7) SHA1(881221da640b883302e657b906ea0a4e74555679) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845uaa02", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( fbait2bc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc865ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8f0b4b) SHA1(363b1ea1a520b239ba8bca867366bbe8a9977a43) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865uab02", 0, BAD_DUMP SHA1(d14dc066d4c16fba1e9b31d5f042ad249c4b5137) )
ROM_END

ROM_START( fbaitbc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge765ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(588748c6) SHA1(ea1ead61e0dcb324ef7b6106cae00bcf6702d6c4) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765uab02", 0, BAD_DUMP SHA1(07b09e763e4b90108aa924b518221b16667a7133) )
ROM_END

ROM_START( fbaitmc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(753ad84e) SHA1(e024cefaaee7c9945ccc1f9a3d896b8560adce2e) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ea", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmca )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9c22aae8) SHA1(c107b0bf7fa76708f2d4f6aaf2cf27b3858378a3) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889aa", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(6278603c) SHA1(d6b59e270cfe4016e12565aedec8a4f0702e1a6f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ja", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(67b91e54) SHA1(4d94bfab08e2bf6e34ee606dd3c4e345d8e5d158) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ua", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fghtmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918eaa.u1",  0x000000, 0x000224, CRC(f3342ff5) SHA1(d3d6ecc22396f74b99ad7aab7908cd542c518977) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918eaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918aaa.u1",  0x000000, 0x000224, CRC(1a2c5d53) SHA1(ab7e44a83e8cd271e2bf8580881a3050d35641df) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918aaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918kaa.u1",  0x000000, 0x000224, CRC(cf32990b) SHA1(bf49b8560f008696b45a3f7f03fa7b3395635b0f) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918kaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918uaa.u1",  0x000000, 0x000224, CRC(e1b7e9ef) SHA1(5767f47cb9a689601fb92c6a494563c5ffdde04c) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918uaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( hndlchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31m",    0x000000, 0x200000, CRC(f5f71b1d) SHA1(7d518e5333f44e6ec921a1e882df970953814b6e) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27m",    0x000000, 0x200000, CRC(b3d8c037) SHA1(678b88c37111d1fde8996c7d71b66ec1c4f161fe) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31l",    0x000000, 0x200000, CRC(78e8556c) SHA1(9f6bb651ddeb042ebf1ba057d4932494149f47d6) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27l",    0x000000, 0x200000, CRC(f6a87155) SHA1(269bfdf05ee4ab2e4b87b6e92045e56d0557a576) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31j",    0x000000, 0x200000, CRC(bdc05d16) SHA1(ee397950f7e7e910fdc05737f99604e43d288719) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27j",    0x000000, 0x200000, CRC(ad925ed3) SHA1(e3222308961851cccee2de9da804f74854907451) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31h",    0x000000, 0x200000, CRC(a0293108) SHA1(2e5651a4c1b8e021cc3060db138c9fe7c28caa3b) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27h",    0x000000, 0x200000, CRC(aed26efe) SHA1(20b6fccd0bc5495d8258b976f72d330d6315c6f6) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "710ja.22h",    0x000000, 0x002000, CRC(b784de91) SHA1(048157e9ad6df46656dbac6349b0c821254e1c37) )
ROM_END

ROM_START( gchgchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31m",    0x000000, 0x200000, CRC(f5f71b1d) SHA1(7d518e5333f44e6ec921a1e882df970953814b6e) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27m",    0x000000, 0x200000, CRC(b3d8c037) SHA1(678b88c37111d1fde8996c7d71b66ec1c4f161fe) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31l",    0x000000, 0x200000, CRC(78e8556c) SHA1(9f6bb651ddeb042ebf1ba057d4932494149f47d6) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27l",    0x000000, 0x200000, CRC(f6a87155) SHA1(269bfdf05ee4ab2e4b87b6e92045e56d0557a576) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31j",    0x000000, 0x200000, CRC(bdc05d16) SHA1(ee397950f7e7e910fdc05737f99604e43d288719) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27j",    0x000000, 0x200000, CRC(ad925ed3) SHA1(e3222308961851cccee2de9da804f74854907451) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.31h",    0x000000, 0x200000, CRC(a0293108) SHA1(2e5651a4c1b8e021cc3060db138c9fe7c28caa3b) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "710ja.27h",    0x000000, 0x200000, CRC(aed26efe) SHA1(20b6fccd0bc5495d8258b976f72d330d6315c6f6) )

	ROM_REGION( 0x200000, "pccard1:16mb:1l", 0 )
	ROM_LOAD( "ge877ja.1l",   0x100000, 0x100000, CRC(06b95144) SHA1(870fc99ba6c6b0c314ddc270b8ba0f44412978bd) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:1u", 0 )
	ROM_LOAD( "ge877ja.1u",   0x100000, 0x100000, CRC(2a3b639f) SHA1(c810a16a36c5e3f5a67a760d488d22108b8a35f7) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:2l", 0 )
	ROM_LOAD( "ge877ja.2l",   0x100000, 0x100000, CRC(e2b273ac) SHA1(73eda00d9a32e252e66ad166d35c5bc8a1a1bf97) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:2u", 0 )
	ROM_LOAD( "ge877ja.2u",   0x100000, 0x100000, CRC(247a6c18) SHA1(145a8bbf35f71ebf5c9232ad1a860ee4c10083c1) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:3l", 0 )
	ROM_LOAD( "ge877ja.3l",   0x100000, 0x100000, CRC(174a4551) SHA1(32c24c99824719cd3057281ac1114e624c16df81) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:3u", 0 )
	ROM_LOAD( "ge877ja.3u",   0x100000, 0x100000, CRC(45398c5f) SHA1(ec5f7e83dbd86807fb78e852e31c6f5db187204a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:4l", 0 )
	ROM_LOAD( "ge877ja.4l",   0x100000, 0x100000, CRC(351cbbd6) SHA1(eccb5dc03dc668b0690a6209d57b37fb5cdc200a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:16mb:4u", 0 )
	ROM_LOAD( "ge877ja.4u",   0x100000, 0x100000, CRC(7b28d962) SHA1(27a46e41dc53cb85f83ec4558bc1f88504d725eb) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge877jaa.u1",  0x000000, 0x000224, CRC(06d0e427) SHA1(cf61c421c0ea236b492d49a00b4608062bbe9063) )
ROM_END

ROM_START( gtrfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(06bd6c4f) SHA1(61930e467ad135e2f31393ff5af981ed52f3bef9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(143eaa55) SHA1(51a4fa3693f1cb1646a8986003f9b6cc1ae8b630) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(11ffd43d) SHA1(27f4f4d782604379254fb98c3c57e547aa4b321f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(efa51ee9) SHA1(3374d936de69c287e0161bc526546441c2943555) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrk2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(687868c4) SHA1(1230e74e4cf17953febe501df56d8bbec1de9356) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP SHA1(4f6bb0150ad6ed574dd7583ccd60604028663b2a) )
ROM_END

ROM_START( gtrfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jac01", 0, BAD_DUMP SHA1(ff017dd5c0ecbdb8935d0d4656a45e9fab10ef82) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "949jab02", 0, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jab02", 0, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3mb )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(61f35ee1) SHA1(0a2b66742364d76ec18647b2761590bd49229625) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jaz02", 0, BAD_DUMP SHA1(b0c786ba420a34fcbd16bc36a137f6ae87b7dfa8) )
ROM_END

ROM_START( gtrfrk4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a24jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(29e326fe) SHA1(41a600105b08accc9d7ebd2b8ae08c0863758aa0) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea24ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(d1fccf11) SHA1(6dcd79f3171d6e4bd7e1149901638f8ea58ff623) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a24jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea24ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a24jaa02", 0, BAD_DUMP SHA1(bc0303f5a6a19484cd35890cc9934ee0bcabb2ad) )
ROM_END

ROM_START( gtrfrk5m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea26jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(c2725fca) SHA1(b70a3266c61af5cbe0478a6f3dd850ebcab980dc) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.31m", 0x000000, 0x200000, CRC(1a25e660) SHA1(dbd8fad0bac307723c70d00763cadf4261a7ed73) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.27m", 0x000000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea26jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a26jaa02", 0, BAD_DUMP SHA1(9909e08abff780db6fd7a5fbcc57ffbe14ae08ce) )
ROM_END

ROM_START( gtrfrk6m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb06ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(673c98ab) SHA1(b1d889bf4fc5e425056acb6b72b2c563966fb7d7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb06ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b06jaa02", 0, BAD_DUMP SHA1(2ea53ef492da63183a28c54afde07fce323fe42e) )
ROM_END

ROM_START( gtrfrk7m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb17jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(5a338c31) SHA1(0fd9ee306335858dd6bef680a62557a8bf055cc3) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.31m", 0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.27m", 0x000000, 0x200000, CRC(7e7da9a9) SHA1(1882418779a48b5aefd113895756116379a6a4f7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb17jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b17jaa02", 0, SHA1(daf23982abbab882882f89b3a9d985df36252cae) )
ROM_END

ROM_START( gtrfrk8m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc08jba.u1",   0x000000, 0x001014, BAD_DUMP CRC(db4b3027) SHA1(65ca32fcacda18954a4e8352dbb9bf583dfdd121) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.31m", 0x000000, 0x200000, CRC(ddef5efe) SHA1(7c3a219eacf63f55894e81cb0e41753176191708) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.27m", 0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc08jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c08jba02", 0, BAD_DUMP SHA1(8e352ed8ade581b7c9bb579fc56003ea1831202c) )
ROM_END

ROM_START( gtrfrk8ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc08jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(9c58f22b) SHA1(41ade23bac86e437b1f12c5730b8cce292ffe4f8) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.31m", 0x000000, 0x200000, CRC(aa723d4c) SHA1(5f55ddaf7f21b624deac99cc40b89989cd6f3a3d) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.27m", 0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc08jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c08jaa02", 0, BAD_DUMP SHA1(7a1d97f74ec4d643ff7d3981d66b551cbf9e57f0) )
ROM_END

ROM_START( gtrfrk9m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc39jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(afb75814) SHA1(027dc2ae3444d10c14169f1f354ffcc928f62fb3) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc39jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c39jaa02", 0, BAD_DUMP SHA1(d0696b29976a6bc01c3a1fefe09dbee721ff3ffb) )
ROM_END

ROM_START( gtfrk10m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd10jab.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd10jab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jab01", 0, BAD_DUMP SHA1(c84858b412f0798a65cf3059c743501f32ad7280) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "d10jaa02", 0, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk10ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd10jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd10jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jaa02", 0, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk10mb )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd10jab.u1",   0x000000, 0x001014, BAD_DUMP CRC(43520577) SHA1(a0749e766688032fe6558707b564288b95da9b8d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd10jab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d10jba02", 0, BAD_DUMP SHA1(80893da422268cc1f89688289cdec981c4f9feb2) )
ROM_END

ROM_START( gtfrk11m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(9bd81d0a) SHA1(c95f6d7317bf88177f7217de4ba4376485d5cdbf) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d39jaa02", 0, BAD_DUMP SHA1(7a87ee331ba0301bb8724c398e6c77cfb9c172a7) )
ROM_END

ROM_START( gunmania )
	SYS573_BIOS_A

	ROM_REGION( 0x000008, "gunmania_id", 0 ) /* digital board id */     \
	ROM_LOAD( "ds2401",        0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31m",  0x000000, 0x200000, CRC(6c02d360) SHA1(961bd9df4928a3dead9da6a88641547cae4c4dbd) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27m",  0x000000, 0x200000, CRC(057b5bce) SHA1(979e3fb5496920c3f9eb7111425c08d80c9076a5) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31l",  0x000000, 0x200000, CRC(3f3abf8f) SHA1(9c93e58fad16ccbe4bc4499a1a15af134243c154) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27l",  0x000000, 0x200000, CRC(f2be642d) SHA1(6c46197a0d114ac90824de1fc4df12db561844e5) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31j",  0x000000, 0x200000, CRC(889a4733) SHA1(1f6578d95c0331fdf3235ef7d899d5bd083ff6a0) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27j",  0x000000, 0x200000, CRC(984193a8) SHA1(1a310e22a80cb4854b138f737f679384c98b2e46) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.31h",  0x000000, 0x200000, CRC(202236c1) SHA1(ecd58f2b325fdefe2ac6cdd6f4edd212432e149a) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gl906jaa.27h",  0x000000, 0x200000, CRC(8861b858) SHA1(2a67d465786759a74162ebebc0a44ba9309ffa60) )

	ROM_REGION( 0x200000, "pccard2:32mb:1l", 0 )
	ROM_LOAD( "gl906jaa.1l",   0x100000, 0x100000, BAD_DUMP CRC(4ad00681) SHA1(93fb97bd148c72f13d6d3b713d8bc6eeda7383ef) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:1u", 0 )
	ROM_LOAD( "gl906jaa.1u",   0x100000, 0x100000, BAD_DUMP CRC(6730d49a) SHA1(4f1810c04f078ef6de3a582d1982c6d54223998b) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:2l", 0 )
	ROM_LOAD( "gl906jaa.2l",   0x100000, 0x100000, BAD_DUMP CRC(383c80f6) SHA1(b540aba095526ce956a9a81e43bf46cb3eca6a9e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:2u", 0 )
	ROM_LOAD( "gl906jaa.2u",   0x100000, 0x100000, BAD_DUMP CRC(68a92d52) SHA1(05584cd7e94ac551a82cfb435c637aabe6d4d044) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:3l", 0 )
	ROM_LOAD( "gl906jaa.3l",   0x100000, 0x100000, BAD_DUMP CRC(390b3ff7) SHA1(9ff79043125c11d5338a32443693259c728f8640) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:3u", 0 )
	ROM_LOAD( "gl906jaa.3u",   0x100000, 0x100000, BAD_DUMP CRC(b2ba1f4d) SHA1(1cd9227b99498d3f6bf464d7185fb511babb135e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:4l", 0 )
	ROM_LOAD( "gl906jaa.4l",   0x100000, 0x100000, BAD_DUMP CRC(fed293be) SHA1(9109a18a342f455d7ee6f08c09e494781b6ae400) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:4u", 0 )
	ROM_LOAD( "gl906jaa.4u",   0x100000, 0x100000, BAD_DUMP CRC(ac42d147) SHA1(0dcb9515f6f8c609cc10a73f07683aa132927f18) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:5l", 0 )
	ROM_LOAD( "gl906jaa.5l",   0x100000, 0x100000, BAD_DUMP CRC(8209c1e0) SHA1(9f1f47f49e45bd3c71cd07c6719f8616c2518014) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:5u", 0 )
	ROM_LOAD( "gl906jaa.5u",   0x100000, 0x100000, BAD_DUMP CRC(1e3f0f1a) SHA1(2e6134a1d64ae3367261adfad5af61265d00340a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:6l", 0 )
	ROM_LOAD( "gl906jaa.6l",   0x100000, 0x100000, BAD_DUMP CRC(53ca942e) SHA1(4d82bf406a338e4f96eb28c5c6f2707d73e53086) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:6u", 0 )
	ROM_LOAD( "gl906jaa.6u",   0x100000, 0x100000, BAD_DUMP CRC(82cfd213) SHA1(cd18de5d93541c64bdacc76ab8cd41656827284e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:7l", 0 )
	ROM_LOAD( "gl906jaa.7l",   0x100000, 0x100000, BAD_DUMP CRC(bcf3ed36) SHA1(8c9c97b0c5a21222ce1d680110509231abb58b9e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:7u", 0 )
	ROM_LOAD( "gl906jaa.7u",   0x100000, 0x100000, BAD_DUMP CRC(b5d5da7d) SHA1(000c2db950c3a4ac6296edb45b7c89b4be724071) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:8l", 0 )
	ROM_LOAD( "gl906jaa.8l",   0x100000, 0x100000, BAD_DUMP CRC(96c5e4fe) SHA1(9c7429f0352357b4b370d39b0e0fb9ce4b514a1b) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard2:32mb:8u", 0 )
	ROM_LOAD( "gl906jaa.8u",   0x100000, 0x100000, BAD_DUMP CRC(030fff86) SHA1(5a04fde970fe542b13327ef54b9b6ad6c79a9e3c) )
	ROM_CONTINUE( 0x000000, 0x100000 )
ROM_END

ROM_START( hyperbbc )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31m",    0x000000, 0x200000, CRC(a76043cb) SHA1(1c37034298abf3219d0bba29f4fcea8d83782926) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27m",    0x000000, 0x200000, CRC(689ddd94) SHA1(512ca1529695f4f79ca8c1b8f64bb0067137e430) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31l",    0x000000, 0x200000, CRC(d011c7a5) SHA1(8861b62c8b654b8e719600a37337ae44e6438899) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27l",    0x000000, 0x200000, CRC(950a5267) SHA1(373a7305a090d1e347bfeb62cc2db55cde2a106e) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31j",    0x000000, 0x200000, CRC(ae497ebc) SHA1(ef131e60726db94f0d9ceab70bce02c0de080ede) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27j",    0x000000, 0x200000, CRC(9c156b1b) SHA1(bf07d71cc1f7e9e14beb9f9dfb71667ef2b54f8d) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31h",    0x000000, 0x200000, CRC(368372fb) SHA1(5cc4cb72e182c9e4d0737352e029fd703ba2f516) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27h",    0x000000, 0x200000, CRC(49175f99) SHA1(0154f6332ed210b6f0af20ba622133cde0994b7f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "876ea.22h",    0x000000, 0x002000, CRC(8e11d196) SHA1(e7442fdd611f4290d531b1ebdc9f487e323fd531) )
ROM_END

ROM_START( hyperbbca )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "876aa.31m",    0x000000, 0x200000, CRC(677f8b0a) SHA1(a4c1029a70f5733f64a4f4dde4a568d2cb4dd11d) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "876aa.27m",    0x000000, 0x200000, CRC(0af35a7d) SHA1(086ad70c8bf4bbe5d9748e4d47c639b4250270fc) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31l",    0x000000, 0x200000, CRC(d011c7a5) SHA1(8861b62c8b654b8e719600a37337ae44e6438899) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27l",    0x000000, 0x200000, CRC(950a5267) SHA1(373a7305a090d1e347bfeb62cc2db55cde2a106e) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31j",    0x000000, 0x200000, CRC(ae497ebc) SHA1(ef131e60726db94f0d9ceab70bce02c0de080ede) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27j",    0x000000, 0x200000, CRC(9c156b1b) SHA1(bf07d71cc1f7e9e14beb9f9dfb71667ef2b54f8d) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.31h",    0x000000, 0x200000, CRC(368372fb) SHA1(5cc4cb72e182c9e4d0737352e029fd703ba2f516) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "876ea.27h",    0x000000, 0x200000, CRC(49175f99) SHA1(0154f6332ed210b6f0af20ba622133cde0994b7f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "876aa.22h",    0x000000, 0x002000, CRC(3c17f026) SHA1(8ed33aca99f5d09d5792e136e700e3ac628018e8) )
ROM_END

ROM_START( hypbbc2p )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx908ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(fb6c0635) SHA1(0d974462a0a244ffb1a651adb316242cde427756) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( hypbbc2pk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx908ka.u1",  0x000000, 0x000084, BAD_DUMP CRC(f4f37fe1) SHA1(30f90cdb2d092e4f8d6c14cfd4ca4945e6d352cb) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( konam80a )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9b38b959) SHA1(6b4fca340a9b1c2ae21ad3903c1ac1e39ab08b1a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826aaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80j )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(e9e861e8) SHA1(45841db0b42d096213d9539a8d076d39391dca6d) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826jaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ka.u1", 0x000000, 0x000224, BAD_DUMP CRC(d41f7e38) SHA1(73e2bb132e23be72e72ea5b0686ccad28e47574a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826kaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80s )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(6ce4c619) SHA1(d2be08c213c0a74e30b7ebdd93946374cc64457f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826eaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80u )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(0577379b) SHA1(3988a2a5ef1f1d5981c4767cbed05b73351be903) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826uaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( mamboagg )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa40jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(fd9e7c1f) SHA1(6dd4790589d48803f58328d099c908f0565b2c01) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa40jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(2e4ed217a7e9f7c79abc9a1798556cc3649db30e) )
ROM_END

ROM_START( mamboagga )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa40jrb.u1",  0x000000, 0x001014, BAD_DUMP CRC(367e4c0c) SHA1(bad21aa8818749282dd97cf00c34b7b049096ceb) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa40jrb.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(2e4ed217a7e9f7c79abc9a1798556cc3649db30e) )
ROM_END

ROM_START( mrtlbeat )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "geb47jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(90079ff5) SHA1(8273ee3349dd13207836b0ebf72ad5fa67fef68a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "geb47jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b47jxb02", 0, SHA1(6bbe8d6169ef692bd8995da564bd5a97b6bf0b31) )
ROM_END

ROM_START( powyakex )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx802ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8bdda3) SHA1(780034ab08871631ef0e3e9b779ca89e016c26a8) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "802jab02", 0, SHA1(460cc9f0b2514ec1da06b0a1d7b52fe43220d181) )
ROM_END

ROM_START( pcnfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a23kaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(d71c4b5c) SHA1(3911c5dd933c30e6e44c8cf417bb4c284ecb4b80) )

	ROM_REGION( 0x0001014, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca23ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(f392349c) SHA1(e7eb7979db276de560d5820163a70d97e6c023d8) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a23kaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca23ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23kaa02", 0, BAD_DUMP SHA1(5b853cc25eb583ed36d8cd402235b4f5c9ce065a) )
ROM_END

ROM_START( pnchmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(e4769787) SHA1(d60c6598c7c58b5cd8f86350ebf7f3f32c1ebe9b) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( pnchmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918jab.u1",  0x000000, 0x000224, BAD_DUMP CRC(e4769787) SHA1(d60c6598c7c58b5cd8f86350ebf7f3f32c1ebe9b) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918jab02", 0, SHA1(8b8cb806a4e15b4687456a5a4482ea7e1373bbf6) )
ROM_END

ROM_START( pnchmn2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa09ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(e1e4108f) SHA1(0605e2c7a7dcb2f4928350e96d2ffcc2ede4a762) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.31m",  0x000000, 0x200000, CRC(b1043a91) SHA1(b474439c1a7da7855d9b6d2162d4a522f499d6ab) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.27m",  0x000000, 0x200000, CRC(09b1a70b) SHA1(0f3bcad879e05faaf8130133d774a2071031ee74) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa09ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a09jaa02", 0, BAD_DUMP SHA1(b085fbe76d5ef87578744b45b874b5f79147e586) )
ROM_END

ROM_START( salarymc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca18jaa.u1",  0x000000, 0x000084, CRC(c9197f67) SHA1(8e95a89008f756a79295f2cb557c39efca1351e7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca18jaa.u6",  0x000000, 0x000008, CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a18jaa02", 0, SHA1(740cc93ec65433098153684fdfc418a098a43736) )
ROM_END

ROM_START( stepchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq930ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(de141979) SHA1(fc91a8384852cb940ec1461c8a561118e9850c85) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq930ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.31m", 0x000000, 0x200000, CRC(274f1813) SHA1(ff6053c0889e9b10bf5eeebda68a051bcf8d7430) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.27m", 0x000000, 0x200000, CRC(257f9f8a) SHA1(65f51b1b26809a96798b015c1625f52f7280d9d1) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.31l",  0x000000, 0x200000, CRC(dd3a1821) SHA1(b00ce3e88737f9aa935d0f9e5dc587c28d509483) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.27l",  0x000000, 0x200000, CRC(fff93684) SHA1(a5653bef9ff58bbbb77b6e18c1bbb017ae171426) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.31j",  0x000000, 0x200000, CRC(40cfee5b) SHA1(4dd0bbe9a49b7220d670b2387a7468124cf05938) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.27j",  0x000000, 0x200000, CRC(96ea2ee4) SHA1(ad0c1da7441fb0cc08c917e99b9df48faddd2487) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.31h",  0x000000, 0x200000, CRC(3ddffadd) SHA1(fa2c1289f4813e987bcadf83853627b2e7578978) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq930ja.27h",  0x000000, 0x200000, CRC(256f0794) SHA1(f95d5a8a53dea4d1f4d766124e94ee103cc1e3b2) )
ROM_END

GAME( 1997, sys573,    0,        konami573,  konami573, driver_device, 0,        ROT0, "Konami", "System 573 BIOS", MACHINE_IS_BIOS_ROOT )

GAME( 1997, hndlchmp,  sys573,   konami573,  hndlchmp,  driver_device, 0,        ROT0, "Konami", "Handle Champ (GQ710 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, darkhleg,  sys573,   konami573x, konami573, driver_device, 0,        ROT0, "Konami", "Dark Horse Legend (GX706 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, fbaitbc,   sys573,   fbaitbc,    fbaitbc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait - A Bass Challenge (GE765 VER. UAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, bassangl,  fbaitbc,  fbaitbc,    fbaitbc,   driver_device, 0,        ROT0, "Konami", "Bass Angler (GE765 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, powyakex,  sys573,   konami573x, konami573, driver_device, 0,        ROT0, "Konami", "Jikkyou Powerful Pro Yakyuu EX (GX802 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80s,  sys573,   konami573x, konami573, driver_device, 0,        ROT90, "Konami", "Konami 80's AC Special (GC826 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80u,  konam80s, konami573x, konami573, driver_device, 0,        ROT90, "Konami", "Konami 80's AC Special (GC826 VER. UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80j,  konam80s, konami573x, konami573, driver_device, 0,        ROT90, "Konami", "Konami 80's Gallery (GC826 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80a,  konam80s, konami573x, konami573, driver_device, 0,        ROT90, "Konami", "Konami 80's AC Special (GC826 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80k,  konam80s, konami573x, konami573, driver_device, 0,        ROT90, "Konami", "Konami 80's AC Special (GC826 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, dstage,    sys573,   dsftkd,     ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dancing Stage - Internet Ranking Ver (GC845 VER. EBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, dstagea,   dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dancing Stage (GN845 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddru,      dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution (GN845 VER. UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, ddrj,      dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution - Internet Ranking Ver (GC845 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, ddrja,     dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, ddrjb,     dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution (GC845 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, ddra,      dstage,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution (GN845 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, fbait2bc,  sys573,   fbaitbc,    fbaitbc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, bassang2,  fbait2bc, fbaitbc,    fbaitbc,   driver_device, 0,        ROT0, "Konami", "Bass Angler 2 (GE865 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, hyperbbc,  sys573,   hyperbbc,   hyperbbc,  ksys573_state, hyperbbc, ROT0, "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, hyperbbca, hyperbbc, hyperbbc,   hyperbbc,  ksys573_state, hyperbbc, ROT0, "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, gchgchmp,  sys573,   gchgchmp,   gchgchmp,  driver_device, 0,        ROT0, "Konami", "Gachaga Champ (GE877 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, drmn,      sys573,   drmn,       drmn,      ksys573_state, drmn,     ROT0, "Konami", "DrumMania (GQ881 VER. JAD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, gtrfrks,   sys573,   gtrfrks,    gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks (GQ886 VER. EAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksu,  gtrfrks,  gtrfrks,    gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks (GQ886 VER. UAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksj,  gtrfrks,  gtrfrks,    gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks (GQ886 VER. JAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksa,  gtrfrks,  gtrfrks,    gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks (GQ886 VER. AAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmc,   sys573,   fbaitbc,    fbaitmc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. EA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmcu,  fbaitmc,  fbaitbc,    fbaitmc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. UA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmcj,  fbaitmc,  fbaitbc,    fbaitmc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. JA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmca,  fbaitmc,  fbaitbc,    fbaitmc,   driver_device, 0,        ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. AA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2m,     sys573,   ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution 2nd Mix (GN895 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2ml,    ddr2m,    ddr2ml,     ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mla,   ddr2m,    ddr2ml,     ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddrbocd,   ddr2m,    ddr2ml,     ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mc,    ddr2m,    ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mc2,   ddr2m,    ddr,        ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2m,  sys573,   gtrfrk2m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, dsftkd,    sys573,   dsftkd,     ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, cr589fw,   sys573,   konami573,  konami573, driver_device, 0,        ROT0, "Konami", "CD-ROM Drive Updater 2.0 (700B04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, cr589fwa,  sys573,   konami573,  konami573, driver_device, 0,        ROT0, "Konami", "CD-ROM Drive Updater (700A04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, ddr3mk,    sys573,   ddr3m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 2000, ddr3mka,   ddr3mk,   ddr3m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddr3ma,    ddr3mk,   ddr3m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.1 */
GAME( 1999, ddr3mj,    ddr3mk,   ddr3m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.0 */
GAME( 1999, ddrsbm,    sys573,   ddrsolo,    ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, ddrs2k,    sys573,   ddrs2k,     ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddrs2kj,   ddrs2k,   ddrs2k,     ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.2 */
GAME( 1999, hypbbc2p,  sys573,   hypbbc2p,   hypbbc2p,  ksys573_state, hyperbbc, ROT0, "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, hypbbc2pk, hypbbc2p, hypbbc2p,   hypbbc2p,  ksys573_state, hyperbbc, ROT0, "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, dsfdct,    sys573,   ddr3m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JCA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, dsfdcta,   dsfdct,   dsfdcta,    ddr,       ksys573_state, ddr,      ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, drmn2m,    sys573,   drmn2m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 2nd Mix (GE912 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2mpu,  drmn2m,   drmn2m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, stepchmp,  sys573,   salarymc,   hyperbbc,  ksys573_state, salarymc, ROT0, "Konami", "Step Champ (GQ930 VER. JA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
GAME( 2000, dncfrks,   sys573,   dmx,        dmx,       driver_device, 0,        ROT0, "Konami", "Dance Freaks (G*874 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmx,       dncfrks,  dmx,        dmx,       driver_device, 0,        ROT0, "Konami", "Dance Maniax (G*874 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, gunmania,  sys573,   gunmania,   gunmania,  driver_device, 0,        ROT0, "Konami", "GunMania (GL906 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, fghtmn,    sys573,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Fighting Mania (QG918 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmna,   fghtmn,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Fighting Mania (QG918 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, pnchmn,    fghtmn,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Punch Mania: Hokuto No Ken (GQ918 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, pnchmna,   fghtmn,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Punch Mania: Hokuto No Ken (GQ918 VER. JAB ALT CD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmnk,   fghtmn,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Fighting Mania (QG918 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, fghtmnu,   fghtmn,   pnchmn,     pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Fighting Mania (QG918 VER. UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, dsem,      sys573,   dsem,       ddr,       driver_device, 0,        ROT0, "Konami", "Dancing Stage Euro Mix (G*936 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.7 */
GAME( 2000, gtrfrk3m,  sys573,   gtrfrk3m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3ma,  gtrfrk3m, gtrfrk3m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3mb,  gtrfrk3m, gtrfrk5m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, pnchmn2,   sys573,   pnchmn2,    pnchmn,    ksys573_state, pnchmn,   ROT0, "Konami", "Punch Mania 2: Hokuto No Ken (GQA09 JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* motor/artwork/network */
GAME( 2000, salarymc,  sys573,   salarymc,   hypbbc2p,  ksys573_state, salarymc, ROT0, "Konami", "Salary Man Champ (GCA18 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, ddr3mp,    sys573,   ddr3mp,     ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, pcnfrk3m,  sys573,   drmn2m,     drmn,      driver_device, 0,        ROT0, "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. KAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, drmn3m,    pcnfrk3m, drmn2m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 3rd Mix (G*A23 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, gtrfrk4m,  sys573,   gtrfrk3m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 4th Mix (G*A24 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4m,     sys573,   ddr3mp,     ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. AAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mj,    ddr4m,    ddr3mp,     ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4ms,    sys573,   ddr4ms,     ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. ABA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4msj,   ddr4ms,   ddr4ms,     ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, dsfdr,     sys573,   dsfdr,      ddr,       driver_device, 0,        ROT0, "Konami", "Dancing Stage Featuring Disney's Rave (GCA37JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddrusa,    sys573,   ddrusa,     ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution USA (G*A44 VER. UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mp,    sys573,   ddr3mp,     ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, ddr4mps,   sys573,   ddr4ms,     ddrsolo,   driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, dmx2m,     sys573,   dmx,        dmx,       driver_device, 0,        ROT0, "Konami", "Dance Maniax 2nd Mix (G*A39 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, drmn4m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 4th Mix (G*A25 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2001, gtrfrk5m,  sys573,   gtrfrk5m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 5th Mix (G*A26 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, ddr5m,     sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution 5th Mix (G*A27 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, dmx2majp,  sys573,   dmx,        dmx,       driver_device, 0,        ROT0, "Konami", "Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, mamboagg,  sys573,   mamboagg,   mamboagg,  driver_device, 0,        ROT0, "Konami", "Mambo A Go-Go (GQA40 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, mamboagga, mamboagg, mamboagga,  mamboagg,  driver_device, 0,        ROT0, "Konami", "Mambo A Go-Go e-Amusement (GQA40 VER. JRB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, drmn5m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 5th Mix (G*B05 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk6m,  sys573,   gtrfrk5m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 6th Mix (G*B06 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, drmn6m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 6th Mix (G*B16 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, gtrfrk7m,  sys573,   gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 7th Mix (G*B17 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, ddrmax,    sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "DDR Max - Dance Dance Revolution 6th Mix (G*B19 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, ddrmax2,   sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "DDR Max 2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, mrtlbeat,  sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "Martial Beat (G*B47 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, drmn7m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 7th Mix power-up ver. (G*C07 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, drmn7ma,   drmn7m,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 7th Mix (G*C07 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8m,  sys573,   gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 8th Mix power-up ver. (G*C08 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8ma, gtrfrk8m, gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 8th Mix (G*C08 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, dsem2,     sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dancing Stage Euro Mix 2 (G*C23 VER. EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, ddrextrm,  sys573,   ddr5m,      ddr,       driver_device, 0,        ROT0, "Konami", "Dance Dance Revolution Extreme (G*C36 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn8m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 8th Mix (G*C07 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtrfrk9m,  sys573,   gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 9th Mix (G*C39 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn9m,    sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 9th Mix (G*D09 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10m,  sys573,   gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAB)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10ma, gtfrk10m, gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10mb, gtfrk10m, gtfrk10mb,  gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 10th Mix eAmusement (G*D10 VER. JBA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, gtfrk11m,  sys573,   gtrfrk7m,   gtrfrks,   driver_device, 0,        ROT0, "Konami", "Guitar Freaks 11th Mix (G*D39 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, drmn10m,   sys573,   drmn4m,     drmn,      driver_device, 0,        ROT0, "Konami", "DrumMania 10th Mix (G*D40 VER. JAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
