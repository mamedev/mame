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
  * emulate memory card board GE885-PWB(A)A ( contains Toshiba tmpr3904af, ram, rom, tranceiver and glue ).

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
P *Gachaga Champ                                1999.01
P Hyper Bishi Bashi Champ                       1998.07    GC908 JA          908    A02
P Jikkyou Powerful Pro Yakyuu EX                1998.04    GX802 JA          802 JA B02
P *Jikkyou Powerful Pro Yakyuu EX 98            1998.08
? *Kick & Kick                                  2001
P Konami 80's Arcade Gallery                    1998.11    GC826 JA          826 JA A01
P Konami 80's AC Special                        1998       GC826 UA          826 UA A01
D *Mambo a GoGo                                 2001.06
D Punchmania Hokuto no Ken                      2000.03                      918 JA B02
D Punchmania Hokuto no Ken 2                    2000.12                      A09 JA A02
P Salary Man Champ                              2001.02    GCA18 JA          A18 JA(needs redump)
P *Step Champ                                   1999.12

P: plain System573
A: uses ext. analog I/O board
D: uses ext. digital sound and I/O board
N: uses network PCB unit + ext. digital sound and I/O board
G: gun mania only, drives air soft gun (this game uses real BB bullet)

  Note:
       Not all games listed above are confirmed to run on System 573.
       * - denotes not dumped yet. If you can help with the remaining undumped System 573 games,
       please contact http://guru.mameworld.info/


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


  Digital I/O PCB
  ---------------

  GX894-PWB(B)A (C)1999 KONAMI CO. LTD.

             |-------------|
             |        CN12 |
             |             |
             | PC847 PC847 |
             |             |
             |        CN11 |
             |             |
             | PC847 PC847 |
             |             |
             | DS2401 CN10 |
             |             |
             | PC847 PC847 |
             |             |
             |  CN14  CN13 |
  |----------|             |----------|
  |                  PC847            |
  | ADM232 CN17              XC9536   |
  |                                   |
  |                    19.6608MHz     |-----------|
  | ADM232 CN15  CY7C109                          |
  |                       HY51V65164A HY51V65164A |
  |                            HY51V65164A        |
  |      CN16    XCS40XL                          |
  |                                               |
  | AK4309B   CN18         29.450MHz  MAS3507D    |
  |                                               |
  |                           CN3                 |
  | HYC24855  RCA-L/R                             |
  |-----------------------------------------------|

  Notes:

  PC847       - High Density Mounting Type Photocoupler
  CN12        - 13 pin connector with 8 wires to external connectors
  CN11        - 12 pin connector with 8 wires to external connectors
  DS2401      - DS2401 911C2  Silicon serial number
  CN10        - 10 pin connector with 8 wires to external connectors
  CN14        - 7 pin connector
  CN13        - 5 pin connector with 2 wires to external connectors
  ADM232      - ADM232AARN 9933 H48475  High Speed, 5 V, 0.1 uF CMOS RS-232 Drivers/Receivers
  CN17        - 3 pin connector
  XC9536      - XILINX XC9536 PC44AEM9933 F1096429A 15C
  CN15        - 8 pin connector
  CY7C109     - CY7C109-25VC 931 H 04 404825  128k x 8 Static RAM
  HY51V65164A - 64M bit dynamic EDO RAM
  CN16        - 4 pin connector joining this PCB to the CD-DA IN on the MAIN PCB.
  XCS40XL     - XILINX XCS40XL PQ208AKP9929 A2033251A 4C
  AK4309B     - AKM AK4309B 3N932N  16bit SCF DAC
  CN18        - 6 pin connector
  MAS3507D    - IM MAS3507D D8 9173 51 HM U 072953.000 ES  MPEG 1/2 Layer 2/3 Audio Decoder
  CN3         - Connector joining this PCB to the MAIN PCB
  HYC24855    - ?
  RCA-L/R     - RCA connectors for left/right audio output

  */

#include "emu.h"
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

class ksys573_state : public psx_state
{
public:
	ksys573_state(running_machine &machine, const driver_device_config_base &config)
		: psx_state(machine, config) { }

	int flash_bank;
	fujitsu_29f016a_device *flash_device[5][16];
	int security_cart_number;

	int chiptype[ 2 ];
	int has_ds2401[ 2 ];

	nvram_handler_func nvram_handler_security_cart_0;
	nvram_handler_func nvram_handler_security_cart_1;

	UINT32 control;

	UINT8 *atapi_regs;
	emu_timer *atapi_timer;
	SCSIInstance *inserted_cdrom;
	SCSIInstance *available_cdroms[ 2 ];
	UINT8 *atapi_data;
	int atapi_data_ptr;
	int atapi_data_len;
	int atapi_xferlen;
	int atapi_xferbase;
	int atapi_cdata_wait;
	int atapi_xfermod;

	UINT32 m_n_security_control;
	void (*security_callback)( running_machine *machine, int data );

	emu_timer *m_p_timer_root[ 3 ];
	UINT16 m_p_n_root_count[ 3 ];
	UINT16 m_p_n_root_mode[ 3 ];
	UINT16 m_p_n_root_target[ 3 ];
	UINT64 m_p_n_root_start[ 3 ];

	UINT8 gx700pwbf_output_data[ 4 ];
	void (*gx700pwfbf_output_callback)( running_machine *machine, int offset, int data );
	UINT16 gx894pwbba_output_data[ 8 ];
	void (*gx894pwbba_output_callback)( running_machine *machine, int offset, int data );

	UINT32 stage_mask;
	struct
	{
		int DO;
		int clk;
		int shift;
		int state;
		int bit;
	} stage[ 2 ];

	UINT32 gx894_ram_write_offset;
	UINT32 gx894_ram_read_offset;
	UINT16 *gx894_ram;
	int f, o, s;
	UINT32 a,b,c,d;

	int salarymc_lamp_bits;
	int salarymc_lamp_shift;
	int salarymc_lamp_clk;

	int hyperbbc_lamp_strobe1;
	int hyperbbc_lamp_strobe2;
};

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

static const char *const diskregions[] = { "cdrom0", "cdrom1" };

/* EEPROM handlers */

static NVRAM_HANDLER( konami573 )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( state->nvram_handler_security_cart_0 != NULL )
	{
		state->nvram_handler_security_cart_0(machine, file, read_or_write);
	}

	if( state->nvram_handler_security_cart_1 != NULL )
	{
		state->nvram_handler_security_cart_1(machine, file, read_or_write);
	}
}

static WRITE32_HANDLER( mb89371_w )
{
	verboselog( space->machine, 2, "mb89371_w %08x %08x %08x\n", offset, mem_mask, data );
}

static READ32_HANDLER( mb89371_r )
{
	UINT32 data = 0xffffffff;
	verboselog( space->machine, 2, "mb89371_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

static READ32_HANDLER( jamma_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	running_machine *machine = space->machine;
	int security_cart_number = state->security_cart_number;
	UINT32 data = input_port_read(machine, "IN1");
	data |= 0x000000c0;

	if( state->has_ds2401[ security_cart_number ] )
	{
		data |= ds2401_read( machine, security_cart_number ) << 14;
	}

	switch( state->chiptype[ security_cart_number ] )
	{
	case 1:
		data |= x76f041_sda_read( machine, security_cart_number ) << 18;
		break;

	case 2:
		data |= x76f100_sda_read( machine, security_cart_number ) << 18;
		break;

	case 3:
		data |= zs01_sda_read( machine, security_cart_number ) << 18;
		break;
	}

	if( state->flash_device[1][0] == NULL )
	{
		data |= ( 1 << 26 );
	}
	if( state->flash_device[2][0] == NULL )
	{
		data |= ( 1 << 27 );
	}

	verboselog( machine, 2, "jamma_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

static READ32_HANDLER( control_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();

	verboselog( space->machine, 2, "control_r( %08x, %08x ) %08x\n", offset, mem_mask, state->control );

	return state->control;
}

static WRITE32_HANDLER( control_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	UINT32 control;
	int old_bank = state->flash_bank;

	COMBINE_DATA(&state->control);
	control = state->control;

	verboselog( space->machine, 2, "control_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	state->flash_bank = -1;

	switch( state->chiptype[ state->security_cart_number ] )
	{
	case 3:
		zs01_sda_write( space->machine, state->security_cart_number, !( ( control >> 6 ) & 1 ) ); /* 0x40 */
		break;
	}

	if( state->flash_device[0][0] != NULL && ( control & ~0x43 ) == 0x00 )
	{
		state->flash_bank = (0 << 8) + ( ( control & 3 ) * 2 );
		if( state->flash_bank != old_bank ) verboselog( space->machine, 1, "onboard %d\n", control & 3 );
	}
	else if( state->flash_device[1][0] != NULL && ( control & ~0x47 ) == 0x10 )
	{
		state->flash_bank = (1 << 8) + ( ( control & 7 ) * 2 );
		if( state->flash_bank != old_bank ) verboselog( space->machine, 1, "pccard1 %d\n", control & 7 );
	}
	else if( state->flash_device[2][0] != NULL && ( control & ~0x47 ) == 0x20 )
	{
		state->flash_bank = (2 << 8) + ( ( control & 7 ) * 2 );
		if( state->flash_bank != old_bank ) verboselog( space->machine, 1, "pccard2 %d\n", control & 7 );
	}
	else if( state->flash_device[3][0] != NULL && ( control & ~0x47 ) == 0x20 )
	{
		state->flash_bank = (3 << 8) + ( ( control & 7 ) * 2 );
		if( state->flash_bank != old_bank ) verboselog( space->machine, 1, "pccard3 %d\n", control & 7 );
	}
	else if( state->flash_device[4][0] != NULL && ( control & ~0x47 ) == 0x28 )
	{
		state->flash_bank = (4 << 8) + ( ( control & 7 ) * 2 );
		if( state->flash_bank != old_bank ) verboselog( space->machine, 1, "pccard4 %d\n", control & 7 );
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

#define MAX_TRANSFER_SIZE ( 63488 )

static TIMER_CALLBACK( atapi_xfer_end )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	UINT32 *p_n_psxram = state->p_n_psxram;
	UINT8 *atapi_regs = state->atapi_regs;
	int i, n_this;
	UINT8 sector_buffer[ 4096 ];

	timer_adjust_oneshot(state->atapi_timer, attotime_never, 0);

//  verboselog( machine, 2, "atapi_xfer_end( %d ) atapi_xferlen = %d, atapi_xfermod=%d\n", x, atapi_xfermod, atapi_xferlen );

//  mame_printf_debug("ATAPI: xfer_end.  xferlen = %d, atapi_xfermod = %d\n", atapi_xferlen, atapi_xfermod);

	while( state->atapi_xferlen > 0 )
	{
		// get a sector from the SCSI device
		SCSIReadData( state->inserted_cdrom, sector_buffer, 2048 );

		state->atapi_xferlen -= 2048;

		i = 0;
		n_this = 2048 / 4;
		while( n_this > 0 )
		{
			p_n_psxram[ state->atapi_xferbase / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			state->atapi_xferbase += 4;
			i += 4;
			n_this--;
		}
	}

	if (state->atapi_xfermod > MAX_TRANSFER_SIZE)
	{
		state->atapi_xferlen = MAX_TRANSFER_SIZE;
		state->atapi_xfermod = state->atapi_xfermod - MAX_TRANSFER_SIZE;
	}
	else
	{
		state->atapi_xferlen = state->atapi_xfermod;
		state->atapi_xfermod = 0;
	}


	if (state->atapi_xferlen > 0)
	{
		//mame_printf_debug("ATAPI: starting next piece of multi-part transfer\n");
		atapi_regs[ATAPI_REG_COUNTLOW] = state->atapi_xferlen & 0xff;
		atapi_regs[ATAPI_REG_COUNTHIGH] = (state->atapi_xferlen>>8)&0xff;

		timer_adjust_oneshot(state->atapi_timer, machine->device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (state->atapi_xferlen/2048))), 0);
	}
	else
	{
		//mame_printf_debug("ATAPI: Transfer completed, dropping DRQ\n");
		atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRDY;
		atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO | ATAPI_INTREASON_COMMAND;
	}

	psx_irq_set(machine, 0x400);

	verboselog( machine, 2, "atapi_xfer_end: %d %d\n", state->atapi_xferlen, state->atapi_xfermod );
}

static READ32_HANDLER( atapi_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	UINT8 *atapi_regs = state->atapi_regs;
	running_machine *machine = space->machine;
	int reg, data;

	if (mem_mask == 0x0000ffff)	// word-wide command read
	{
//      mame_printf_debug("ATAPI: packet read = %04x\n", atapi_data[atapi_data_ptr]);

		// assert IRQ and drop DRQ
		if (state->atapi_data_ptr == 0 && state->atapi_data_len == 0)
		{
			// get the data from the device
			if( state->atapi_xferlen > 0 )
			{
				SCSIReadData( state->inserted_cdrom, state->atapi_data, state->atapi_xferlen );
				state->atapi_data_len = state->atapi_xferlen;
			}

			if (state->atapi_xfermod > MAX_TRANSFER_SIZE)
			{
				state->atapi_xferlen = MAX_TRANSFER_SIZE;
				state->atapi_xfermod = state->atapi_xfermod - MAX_TRANSFER_SIZE;
			}
			else
			{
				state->atapi_xferlen = state->atapi_xfermod;
				state->atapi_xfermod = 0;
			}

			verboselog( machine, 2, "atapi_r: atapi_xferlen=%d\n", state->atapi_xferlen );
			if( state->atapi_xferlen != 0 )
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

			atapi_regs[ATAPI_REG_COUNTLOW] = state->atapi_xferlen & 0xff;
			atapi_regs[ATAPI_REG_COUNTHIGH] = (state->atapi_xferlen>>8)&0xff;

			psx_irq_set(space->machine, 0x400);
		}

		if( state->atapi_data_ptr < state->atapi_data_len )
		{
			data = state->atapi_data[state->atapi_data_ptr++];
			data |= ( state->atapi_data[state->atapi_data_ptr++] << 8 );
			if( state->atapi_data_ptr >= state->atapi_data_len )
			{
//              verboselog( machine, 2, "atapi_r: read all bytes\n" );
				state->atapi_data_ptr = 0;
				state->atapi_data_len = 0;

				if( state->atapi_xferlen == 0 )
				{
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_IO;
					psx_irq_set(space->machine, 0x400);
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
		if (mem_mask == 0x00ff0000)
		{
			reg += 1;
			shift = 16;
		}

		data = atapi_regs[reg];

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( machine, 1, "atapi_r: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( machine, 1, "atapi_r: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( machine, 1, "atapi_r: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( machine, 1, "atapi_r: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( machine, 1, "atapi_r: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( machine, 1, "atapi_r: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( machine, 1, "atapi_r: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( machine, 1, "atapi_r: cmdstatus=%02x\n", data );
			break;
		}

//      mame_printf_debug("ATAPI: read reg %d = %x (PC=%x)\n", reg, data, cpu_get_pc(space->cpu));

		data <<= shift;
	}

	verboselog( machine, 2, "atapi_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( atapi_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	UINT8 *atapi_regs = state->atapi_regs;
	UINT8 *atapi_data = state->atapi_data;
	running_machine *machine = space->machine;
	int reg;

	verboselog( machine, 2, "atapi_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (mem_mask == 0x0000ffff)	// word-wide command write
	{
		verboselog( machine, 2, "atapi_w: data=%04x\n", data );

//      mame_printf_debug("ATAPI: packet write %04x\n", data);
		atapi_data[state->atapi_data_ptr++] = data & 0xff;
		atapi_data[state->atapi_data_ptr++] = data >> 8;

		if (state->atapi_cdata_wait)
		{
//          mame_printf_debug("ATAPI: waiting, ptr %d wait %d\n", state->atapi_data_ptr, state->atapi_cdata_wait);
			if (state->atapi_data_ptr == state->atapi_cdata_wait)
			{
				// send it to the device
				SCSIWriteData( state->inserted_cdrom, atapi_data, state->atapi_cdata_wait );

				// assert IRQ
				psx_irq_set(space->machine, 0x400);

				// not sure here, but clear DRQ at least?
				atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
			}
		}

		else if ( state->atapi_data_ptr == 12 )
		{
			int phase;

			verboselog( machine, 2, "atapi_w: command %02x\n", atapi_data[0]&0xff );

			// reset data pointer for reading SCSI results
			state->atapi_data_ptr = 0;
			state->atapi_data_len = 0;

			// send it to the SCSI device
			SCSISetCommand( state->inserted_cdrom, state->atapi_data, 12 );
			SCSIExecCommand( state->inserted_cdrom, &state->atapi_xferlen );
			SCSIGetPhase( state->inserted_cdrom, &phase );

			if (state->atapi_xferlen != -1)
			{
//              mame_printf_debug("ATAPI: SCSI command %02x returned %d bytes from the device\n", atapi_data[0]&0xff, state->atapi_xferlen);

				// store the returned command length in the ATAPI regs, splitting into
				// multiple transfers if necessary
				state->atapi_xfermod = 0;
				if (state->atapi_xferlen > MAX_TRANSFER_SIZE)
				{
					state->atapi_xfermod = state->atapi_xferlen - MAX_TRANSFER_SIZE;
					state->atapi_xferlen = MAX_TRANSFER_SIZE;
				}

				atapi_regs[ATAPI_REG_COUNTLOW] = state->atapi_xferlen & 0xff;
				atapi_regs[ATAPI_REG_COUNTHIGH] = (state->atapi_xferlen>>8)&0xff;

				if (state->atapi_xferlen == 0)
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
					state->atapi_cdata_wait = state->atapi_xferlen;
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
						timer_adjust_oneshot( state->atapi_timer, downcast<cpu_device *>(space->cpu)->cycles_to_attotime( ATAPI_CYCLES_PER_SECTOR ), 0 );
						break;
				}

				// assert IRQ
				psx_irq_set(space->machine, 0x400);
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
		if (mem_mask == 0x00ff0000)
		{
			reg += 1;
			data >>= 16;
		}

		switch( reg )
		{
		case ATAPI_REG_DATA:
			verboselog( machine, 1, "atapi_w: data=%02x\n", data );
			break;
		case ATAPI_REG_ERRFEAT:
			verboselog( machine, 1, "atapi_w: errfeat=%02x\n", data );
			break;
		case ATAPI_REG_INTREASON:
			verboselog( machine, 1, "atapi_w: intreason=%02x\n", data );
			break;
		case ATAPI_REG_SAMTAG:
			verboselog( machine, 1, "atapi_w: samtag=%02x\n", data );
			break;
		case ATAPI_REG_COUNTLOW:
			verboselog( machine, 1, "atapi_w: countlow=%02x\n", data );
			break;
		case ATAPI_REG_COUNTHIGH:
			verboselog( machine, 1, "atapi_w: counthigh=%02x\n", data );
			break;
		case ATAPI_REG_DRIVESEL:
			verboselog( machine, 1, "atapi_w: drivesel=%02x\n", data );
			break;
		case ATAPI_REG_CMDSTATUS:
			verboselog( machine, 1, "atapi_w: cmdstatus=%02x\n", data );
			break;
		}

		atapi_regs[reg] = data;
//      mame_printf_debug("ATAPI: reg %d = %x (offset %x mask %x PC=%x)\n", reg, data, offset, mem_mask, cpu_get_pc(space->cpu));

		if (reg == ATAPI_REG_CMDSTATUS)
		{
//          mame_printf_debug("ATAPI command %x issued! (PC=%x)\n", data, cpu_get_pc(space->cpu));

			switch (data)
			{
				case 0xa0:	// PACKET
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;
					atapi_regs[ATAPI_REG_INTREASON] = ATAPI_INTREASON_COMMAND;

					state->atapi_data_ptr = 0;
					state->atapi_data_len = 0;

					/* we have no data */
					state->atapi_xferlen = 0;
					state->atapi_xfermod = 0;

					state->atapi_cdata_wait = 0;
					break;

				case 0xa1:	// IDENTIFY PACKET DEVICE
					atapi_regs[ATAPI_REG_CMDSTATUS] = ATAPI_STAT_DRQ;

					state->atapi_data_ptr = 0;
					state->atapi_data_len = 512;

					/* we have no data */
					state->atapi_xferlen = 0;
					state->atapi_xfermod = 0;

					memset( atapi_data, 0, state->atapi_data_len );

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

					psx_irq_set(space->machine, 0x400);
					break;

				case 0xef:	// SET FEATURES
					atapi_regs[ATAPI_REG_CMDSTATUS] = 0;

					state->atapi_data_ptr = 0;
					state->atapi_data_len = 0;

					psx_irq_set(space->machine, 0x400);
					break;

				default:
					mame_printf_debug("ATAPI: Unknown IDE command %x\n", data);
					break;
			}
		}
	}
}

static void atapi_exit(running_machine& machine)
{
	ksys573_state *state = machine.driver_data<ksys573_state>();
	int i;

	for( i = 0; i < 2; i++ )
	{
		if( get_disk_handle( &machine, diskregions[i] ) != NULL )
		{
			SCSIDeleteInstance( state->available_cdroms[ i ] );
		}
	}
}

static void atapi_init(running_machine *machine)
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	UINT8 *atapi_regs;
	int i;

	atapi_regs = state->atapi_regs = auto_alloc_array( machine, UINT8, ATAPI_REG_MAX );

	atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
	atapi_regs[ATAPI_REG_ERRFEAT] = 1;
	atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
	atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

	state->atapi_data_ptr = 0;
	state->atapi_data_len = 0;
	state->atapi_cdata_wait = 0;

	state->atapi_timer = timer_alloc(machine, atapi_xfer_end, NULL);
	timer_adjust_oneshot(state->atapi_timer, attotime_never, 0);

	for( i = 0; i < 2; i++ )
	{
		if( get_disk_handle( machine, diskregions[i] ) != NULL )
		{
			SCSIAllocInstance( machine, &SCSIClassCr589, &state->available_cdroms[ i ], diskregions[i] );
		}
		else
		{
			state->available_cdroms[ i ] = NULL;
		}
	}
	machine->add_notifier(MACHINE_NOTIFY_EXIT, atapi_exit);

	state->atapi_data = auto_alloc_array( machine, UINT8, ATAPI_DATA_SIZE );

	state_save_register_global_pointer( machine, state->atapi_regs, ATAPI_REG_MAX );
	state_save_register_global_pointer( machine, state->atapi_data, ATAPI_DATA_SIZE / 2 );
	state_save_register_global( machine, state->atapi_data_ptr );
	state_save_register_global( machine, state->atapi_data_len );
	state_save_register_global( machine, state->atapi_xferlen );
	state_save_register_global( machine, state->atapi_xferbase );
	state_save_register_global( machine, state->atapi_cdata_wait );
	state_save_register_global( machine, state->atapi_xfermod );
}

static WRITE32_HANDLER( atapi_reset_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	UINT8 *atapi_regs = state->atapi_regs;

	verboselog( space->machine, 2, "atapi_reset_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if (data)
	{
		verboselog( space->machine, 2, "atapi_reset_w: reset\n" );

//      mame_printf_debug("ATAPI reset\n");

		atapi_regs[ATAPI_REG_CMDSTATUS] = 0;
		atapi_regs[ATAPI_REG_ERRFEAT] = 1;
		atapi_regs[ATAPI_REG_COUNTLOW] = 0x14;
		atapi_regs[ATAPI_REG_COUNTHIGH] = 0xeb;

		state->atapi_data_ptr = 0;
		state->atapi_data_len = 0;
		state->atapi_cdata_wait = 0;

		state->atapi_xferlen = 0;
		state->atapi_xfermod = 0;
	}
}

static void cdrom_dma_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	verboselog( machine, 2, "cdrom_dma_read( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA read: address %08x size %08x\n", n_address, n_size);
}

static void cdrom_dma_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	verboselog( machine, 2, "cdrom_dma_write( %08x, %08x )\n", n_address, n_size );
//  mame_printf_debug("DMA write: address %08x size %08x\n", n_address, n_size);

	state->atapi_xferbase = n_address;

	verboselog( machine, 2, "atapi_xfer_end: %d %d\n", state->atapi_xferlen, state->atapi_xfermod );

	// set a transfer complete timer (Note: CYCLES_PER_SECTOR can't be lower than 2000 or the BIOS ends up "out of order")
	timer_adjust_oneshot(state->atapi_timer, machine->device<cpu_device>("maincpu")->cycles_to_attotime((ATAPI_CYCLES_PER_SECTOR * (state->atapi_xferlen/2048))), 0);
}

static WRITE32_HANDLER( security_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	int security_cart_number = state->security_cart_number;
	running_machine *machine = space->machine;
	COMBINE_DATA( &state->m_n_security_control );

	verboselog( space->machine, 2, "security_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( ACCESSING_BITS_0_15 )
	{
		switch( state->chiptype[ security_cart_number ] )
		{
		case 1:
			x76f041_sda_write( machine, security_cart_number, ( data >> 0 ) & 1 );
			x76f041_scl_write( machine, security_cart_number, ( data >> 1 ) & 1 );
			x76f041_cs_write( machine, security_cart_number, ( data >> 2 ) & 1 );
			x76f041_rst_write( machine, security_cart_number, ( data >> 3 ) & 1 );
			break;

		case 2:
			x76f100_sda_write( machine, security_cart_number, ( data >> 0 ) & 1 );
			x76f100_scl_write( machine, security_cart_number, ( data >> 1 ) & 1 );
			x76f100_cs_write( machine, security_cart_number, ( data >> 2 ) & 1 );
			x76f100_rst_write( machine, security_cart_number, ( data >> 3 ) & 1 );
			break;

		case 3:
			zs01_scl_write( machine, security_cart_number, ( data >> 1 ) & 1 );
			zs01_cs_write( machine, security_cart_number, ( data >> 2 ) & 1 );
			zs01_rst_write( machine, security_cart_number, ( data >> 3 ) & 1 );
			break;
		}

		if( state->has_ds2401[ security_cart_number ] )
		{
			ds2401_write( machine, security_cart_number, !( ( data >> 4 ) & 1 ) );
		}

		if( state->security_callback != NULL )
		{
			(*state->security_callback)( machine, data & 0xff );
		}
	}
}

static READ32_HANDLER( security_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();

	UINT32 data = state->m_n_security_control;
	verboselog( space->machine, 2, "security_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static READ32_HANDLER( flash_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	UINT32 data = 0;

	if( state->flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_r( %08x, %08x ) no bank selected %08x\n", cpu_get_pc(space->cpu), offset, mem_mask, state->control );
		data = 0xffffffff;
	}
	else
	{
		fujitsu_29f016a_device **flash_base = &state->flash_device[state->flash_bank >> 8][state->flash_bank & 0xff];
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			data |= ( flash_base[0]->read( adr + 0 ) & 0xff ) << 0; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_8_15 )
		{
			data |= ( flash_base[1]->read( adr + 0 ) & 0xff ) << 8; // 27m/27l/27j/27h
		}
		if( ACCESSING_BITS_16_23 )
		{
			data |= ( flash_base[0]->read( adr + 1 ) & 0xff ) << 16; // 31m/31l/31j/31h
		}
		if( ACCESSING_BITS_24_31 )
		{
			data |= ( flash_base[1]->read( adr + 1 ) & 0xff ) << 24; // 27m/27l/27j/27h
		}
	}

	verboselog( space->machine, 2, "flash_r( %08x, %08x, %08x) bank = %08x\n", offset, mem_mask, data, state->flash_bank );

	return data;
}

static WRITE32_HANDLER( flash_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();

	verboselog( space->machine, 2, "flash_w( %08x, %08x, %08x\n", offset, mem_mask, data );

	if( state->flash_bank < 0 )
	{
		mame_printf_debug( "%08x: flash_w( %08x, %08x, %08x ) no bank selected %08x\n", cpu_get_pc(space->cpu), offset, mem_mask, data, state->control );
	}
	else
	{
		fujitsu_29f016a_device **flash_base = &state->flash_device[state->flash_bank >> 8][state->flash_bank & 0xff];
		int adr = offset * 2;

		if( ACCESSING_BITS_0_7 )
		{
			flash_base[0]->write( adr + 0, ( data >> 0 ) & 0xff );
		}
		if( ACCESSING_BITS_8_15 )
		{
			flash_base[1]->write( adr + 0, ( data >> 8 ) & 0xff );
		}
		if( ACCESSING_BITS_16_23 )
		{
			flash_base[0]->write( adr + 1, ( data >> 16 ) & 0xff );
		}
		if( ACCESSING_BITS_24_31 )
		{
			flash_base[1]->write( adr + 1, ( data >> 24 ) & 0xff );
		}
	}
}

/* Root Counters */

static UINT64 psxcpu_gettotalcycles( running_machine *machine )
{
	/* TODO: should return the start of the current tick. */
	return machine->firstcpu->total_cycles() * 2;
}

static int root_divider( running_machine *machine, int n_counter )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( n_counter == 0 && ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_CLC ) != 0 )
	{
		/* TODO: pixel clock, probably based on resolution */
		return 5;
	}
	else if( n_counter == 1 && ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_CLC ) != 0 )
	{
		return 2150;
	}
	else if( n_counter == 2 && ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_DIV ) != 0 )
	{
		return 8;
	}
	return 1;
}

static UINT16 root_current( running_machine *machine, int n_counter )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_STOP ) != 0 )
	{
		return state->m_p_n_root_count[ n_counter ];
	}
	else
	{
		UINT64 n_current;
		n_current = psxcpu_gettotalcycles(machine) - state->m_p_n_root_start[ n_counter ];
		n_current /= root_divider( machine, n_counter );
		n_current += state->m_p_n_root_count[ n_counter ];
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			state->m_p_n_root_count[ n_counter ] = n_current;
			state->m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(machine);
		}
		return n_current;
	}
}

static int root_target( running_machine *machine, int n_counter )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_COUNTTARGET ) != 0 ||
		( state->m_p_n_root_mode[ n_counter ] & PSX_RC_IRQTARGET ) != 0 )
	{
		return state->m_p_n_root_target[ n_counter ];
	}
	return 0x10000;
}

static void root_timer_adjust( running_machine *machine, int n_counter )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_STOP ) != 0 )
	{
		timer_adjust_oneshot( state->m_p_timer_root[ n_counter ], attotime_never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( machine, n_counter ) - root_current( machine, n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( machine, n_counter );

		timer_adjust_oneshot( state->m_p_timer_root[ n_counter ], attotime_mul(ATTOTIME_IN_HZ(33868800), n_duration), n_counter);
	}
}

static TIMER_CALLBACK( root_finished )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int n_counter = param;

//  if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as PSX_RC_COUNTTARGET & PSX_RC_IRQTARGET don't have to be the same. */
		state->m_p_n_root_count[ n_counter ] = 0;
		state->m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(machine);
	}
	if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_REPEAT ) != 0 )
	{
		root_timer_adjust( machine, n_counter );
	}
	if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_IRQOVERFLOW ) != 0 ||
		( state->m_p_n_root_mode[ n_counter ] & PSX_RC_IRQTARGET ) != 0 )
	{
		psx_irq_set( machine, 0x10 << n_counter );
	}
}

static WRITE32_HANDLER( k573_counter_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	int n_counter;

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		state->m_p_n_root_count[ n_counter ] = data;
		state->m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(space->machine);
		break;
	case 1:
		state->m_p_n_root_count[ n_counter ] = root_current( space->machine, n_counter );
		state->m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(space->machine);
		state->m_p_n_root_mode[ n_counter ] = data;

		if( ( state->m_p_n_root_mode[ n_counter ] & PSX_RC_RESET ) != 0 )
		{
			/* todo: check this is correct */
			state->m_p_n_root_count[ n_counter ] = 0;
			state->m_p_n_root_mode[ n_counter ] &= ~( PSX_RC_STOP | PSX_RC_RESET );
		}
#if 0
		if( ( data & 0xfca6 ) != 0 ||
			( ( data & 0x0100 ) != 0 && n_counter != 0 && n_counter != 1 ) ||
			( ( data & 0x0200 ) != 0 && n_counter != 2 ) )
		{
			printf( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
		}
#endif
		break;
	case 2:
		state->m_p_n_root_target[ n_counter ] = data;
		break;
	default:
		return;
	}

	root_timer_adjust( space->machine, n_counter );
}

static READ32_HANDLER( k573_counter_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	int n_counter;
	UINT32 data;

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( space->machine, n_counter );
		break;
	case 1:
		data = state->m_p_n_root_mode[ n_counter ];
		break;
	case 2:
		data = state->m_p_n_root_target[ n_counter ];
		break;
	default:
		return 0;
	}
	return data;
}

static ADDRESS_MAP_START( konami573_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f000000, 0x1f3fffff) AM_READWRITE( flash_r, flash_w )
	AM_RANGE(0x1f400000, 0x1f400003) AM_READ_PORT( "IN0" ) AM_WRITE_PORT( "OUT0" )
	AM_RANGE(0x1f400004, 0x1f400007) AM_READ( jamma_r )
	AM_RANGE(0x1f400008, 0x1f40000b) AM_READ_PORT( "IN2" )
	AM_RANGE(0x1f40000c, 0x1f40000f) AM_READ_PORT( "IN3" )
	AM_RANGE(0x1f480000, 0x1f48000f) AM_READWRITE( atapi_r, atapi_w )	// IDE controller, used mostly in ATAPI mode (only 3 pure IDE commands seen so far)
	AM_RANGE(0x1f500000, 0x1f500003) AM_READWRITE( control_r, control_w )	// Konami can't make a game without a "control" register.
	AM_RANGE(0x1f560000, 0x1f560003) AM_WRITE( atapi_reset_w )
	AM_RANGE(0x1f5c0000, 0x1f5c0003) AM_WRITENOP				// watchdog?
	AM_RANGE(0x1f620000, 0x1f623fff) AM_DEVREADWRITE8("m48t58", timekeeper_r, timekeeper_w, 0x00ff00ff)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f6a0000, 0x1f6a0003) AM_READWRITE( security_r, security_w )
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_DEVREAD("spu", psx_spu_delay_r)
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(k573_counter_r, k573_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_DEVREADWRITE("spu", psx_spu_r, psx_spu_w)
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("bios", 0)
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END



static void flash_init( running_machine *machine )
{
	// find onboard flash devices
	ksys573_state *state = machine->driver_data<ksys573_state>();
	astring tempstr;
	for (int index = 0; index < 8; index++)
		state->flash_device[0][index] = machine->device<fujitsu_29f016a_device>(tempstr.format("onboard.%d", index));

	// find pccard flash devices
	for (int card = 1; card <= 4; card++)
		for (int index = 0; index < 16; index++)
			state->flash_device[card][index] = machine->device<fujitsu_29f016a_device>(tempstr.format("pccard%d.%d", card, index));

	state_save_register_global( machine, state->flash_bank );
	state_save_register_global( machine, state->control );
}

static void *atapi_get_device(running_machine *machine)
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	void *ret;
	SCSIGetDevice( state->inserted_cdrom, &ret );
	return ret;
}

static void security_cart_init( running_machine *machine, int cart, const char *eeprom_region, const char *ds2401_region )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	UINT8 *eeprom_rom = machine->region( eeprom_region )->base();
	int eeprom_length = machine->region( eeprom_region )->bytes();
	UINT8 *ds2401_rom = machine->region( ds2401_region )->base();

	if( eeprom_rom != NULL )
	{
		switch( eeprom_length )
		{
		case 0x224:
			x76f041_init( machine, cart, eeprom_rom );
			state->chiptype[ cart ] = 1;

			switch( cart )
			{
			case 0:
				state->nvram_handler_security_cart_0 = NVRAM_HANDLER_NAME(x76f041_0);
				break;
			case 1:
				state->nvram_handler_security_cart_1 = NVRAM_HANDLER_NAME(x76f041_1);
				break;
			}

			break;

		case 0x84:
			x76f100_init( machine, cart, eeprom_rom );
			state->chiptype[ cart ] = 2;

			switch( cart )
			{
			case 0:
				state->nvram_handler_security_cart_0 = NVRAM_HANDLER_NAME(x76f100_0);
				break;
			case 1:
				state->nvram_handler_security_cart_1 = NVRAM_HANDLER_NAME(x76f100_1);
				break;
			}

			break;

		case 0x1014:
			zs01_init( machine, cart, eeprom_rom, NULL, NULL, ds2401_rom );
			state->chiptype[ cart ] = 3;

			switch( cart )
			{
			case 0:
				state->nvram_handler_security_cart_0 = NVRAM_HANDLER_NAME(zs01_0);
				break;
			case 1:
				state->nvram_handler_security_cart_1 = NVRAM_HANDLER_NAME(zs01_1);
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
		state->chiptype[ cart ] = 0;
	}

	if( state->chiptype[ cart ] != 3 && ds2401_rom != NULL )
	{
		ds2401_init( machine, cart, ds2401_rom );
		state->has_ds2401[ cart ] = 1;
	}
	else
	{
		state->has_ds2401[ cart ] = 0;
	}
}

static DRIVER_INIT( konami573 )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int i;

	psx_driver_init(machine);
	atapi_init(machine);
	psx_dma_install_read_handler(machine, 5, cdrom_dma_read);
	psx_dma_install_write_handler(machine, 5, cdrom_dma_write);

	for (i = 0; i < 3; i++)
	{
		state->m_p_timer_root[i] = timer_alloc(machine, root_finished, NULL);
	}

	state_save_register_global( machine, state->m_n_security_control );

	security_cart_init( machine, 0, "user2", "user9" );
	security_cart_init( machine, 1, "user8", "user10" );

	state_save_register_item_array( machine, "KSYS573", NULL, 0, state->m_p_n_root_count );
	state_save_register_item_array( machine, "KSYS573", NULL, 0, state->m_p_n_root_mode );
	state_save_register_item_array( machine, "KSYS573", NULL, 0, state->m_p_n_root_target );
	state_save_register_item_array( machine, "KSYS573", NULL, 0, state->m_p_n_root_start );

	flash_init(machine);
}

static MACHINE_RESET( konami573 )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	psx_machine_init(machine);

	if( state->chiptype[ 0 ] != 0 )
	{
		/* security cart */
		psx_sio_input( machine, 1, PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );
	}

	state->flash_bank = -1;
}

static void spu_irq(device_t *device, UINT32 data)
{
	psx_irq_set(device->machine, data);
}

static const psx_spu_interface konami573_psxspu_interface =
{
	spu_irq,
	psx_dma_install_read_handler,
	psx_dma_install_write_handler
};

static void update_mode( running_machine *machine )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int cart = input_port_read(machine, "CART");
	int cd = input_port_read( machine, "CD" );
	SCSIInstance *new_cdrom;

	if( state->chiptype[ 1 ] != 0 )
	{
		state->security_cart_number = cart;
	}
	else
	{
		state->security_cart_number = 0;
	}

	if( state->available_cdroms[ 1 ] != NULL )
	{
		new_cdrom = state->available_cdroms[ cd ];
	}
	else
	{
		new_cdrom = state->available_cdroms[ 0 ];
	}

	if( state->inserted_cdrom != new_cdrom )
	{
		state->inserted_cdrom = new_cdrom;
		cdda_set_cdrom(machine->device("cdda"), atapi_get_device(machine));
	}
}

static INTERRUPT_GEN( sys573_vblank )
{
	ksys573_state *state = device->machine->driver_data<ksys573_state>();
	UINT32 *p_n_psxram = state->p_n_psxram;

	update_mode(device->machine);

	if( strcmp( device->machine->gamedrv->name, "ddr2ml" ) == 0 )
	{
		/* patch out security-plate error */

		/* 8001f850: jal $8003221c */
		if( p_n_psxram[ 0x1f850 / 4 ] == 0x0c00c887 )
		{
			/* 8001f850: j $8001f888 */
			p_n_psxram[ 0x1f850 / 4 ] = 0x08007e22;
		}
	}

	psx_vblank(device);
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
	device_t *upd4701 = space->machine->device("upd4701");
	UINT32 data = 0;

	switch (offset)
	{
	case 0x26:
		upd4701_y_add(upd4701, 0, input_port_read_safe(space->machine, "uPD4701_y", 0), 0xffff);
		upd4701_switches_set(upd4701, 0, input_port_read_safe(space->machine, "uPD4701_switches", 0));

		upd4701_cs_w(upd4701, 0, 0);
		upd4701_xy_w(upd4701, 0, 1);

		if (ACCESSING_BITS_0_7)
		{
			upd4701_ul_w(upd4701, 0, 0);
			data |= upd4701_d_r(upd4701, 0, 0xffff) << 0;
		}

		if (ACCESSING_BITS_16_23)
		{
			upd4701_ul_w(upd4701, 0, 1);
			data |= upd4701_d_r(upd4701, 0, 0xffff) << 16;
		}

		upd4701_cs_w(upd4701, 0, 1);
		break;

	default:
		verboselog(space->machine, 0, "ge765pwbba_r: unhandled offset %08x %08x\n", offset, mem_mask);
		break;
	}

	verboselog(space->machine, 2, "ge765pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data);
	return data;
}

static WRITE32_HANDLER( ge765pwbba_w )
{
	device_t *upd4701 = space->machine->device("upd4701");
	switch (offset)
	{
	case 0x04:
		break;

	case 0x20:
		if (ACCESSING_BITS_0_7)
		{
			output_set_value("motor", data & 0xff);
		}
		break;

	case 0x22:
		if (ACCESSING_BITS_0_7)
		{
			output_set_value("brake", data & 0xff);
		}
		break;

	case 0x28:
		if (ACCESSING_BITS_0_7)
		{
			upd4701_resety_w(upd4701, 0, 1);
			upd4701_resety_w(upd4701, 0, 0);
		}
		break;

	default:
		verboselog(space->machine, 0, "ge765pwbba_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data);
		break;
	}

	verboselog(space->machine, 2, "ge765pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data);
}

static DRIVER_INIT( ge765pwbba )
{
	DRIVER_INIT_CALL(konami573);
	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f640000, 0x1f6400ff, 0, 0, ge765pwbba_r, ge765pwbba_w );
}

/*

GX700-PWB(F)

Analogue I/O board

*/

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

	verboselog( space->machine, 2, "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

static void gx700pwbf_output( running_machine *machine, int offset, UINT8 data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( state->gx700pwfbf_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 7, 6, 1, 0, 5, 4, 3, 2 };
		for( i = 0; i < 8; i++ )
		{
			int oldbit = ( state->gx700pwbf_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				(*state->gx700pwfbf_output_callback)( machine, ( offset * 8 ) + i, newbit );
			}
		}
	}
	state->gx700pwbf_output_data[ offset ] = data;
}

static WRITE32_HANDLER( gx700pwbf_io_w )
{
	verboselog( space->machine, 2, "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0x20:

		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( space->machine, 0, data & 0xff );
		}
		break;

	case 0x22:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( space->machine, 1, data & 0xff );
		}
		break;

	case 0x24:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( space->machine, 2, data & 0xff );
		}
		break;

	case 0x26:
		if( ACCESSING_BITS_0_15 )
		{
			gx700pwbf_output( space->machine, 3, data & 0xff );
		}
		break;

	default:
//      printf( "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
		break;
	}
}

static void gx700pwfbf_init( running_machine *machine, void (*output_callback_func)( running_machine *machine, int offset, int data ) )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	memset( state->gx700pwbf_output_data, 0, sizeof( state->gx700pwbf_output_data ) );

	state->gx700pwfbf_output_callback = output_callback_func;

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f640000, 0x1f6400ff, 0, 0, gx700pwbf_io_r, gx700pwbf_io_w );

	state_save_register_global_array( machine, state->gx700pwbf_output_data );
}

/*

GN845-PWB(B)

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

static void gn845pwbb_do_w( running_machine *machine, int offset, int data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	state->stage[ offset ].DO = !data;
}

static void gn845pwbb_clk_w( running_machine *machine, int offset, int data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int clk = !data;

	if( clk != state->stage[ offset ].clk )
	{
		state->stage[ offset ].clk = clk;

		if( clk )
		{
			state->stage[ offset ].shift = ( state->stage[ offset ].shift >> 1 ) | ( state->stage[ offset ].DO << 12 );

			switch( state->stage[ offset ].state )
			{
			case DDR_STAGE_IDLE:
				if( state->stage[ offset ].shift == 0xc90 )
				{
					state->stage[ offset ].state = DDR_STAGE_INIT;
					state->stage[ offset ].bit = 0;
					state->stage_mask = 0xfffff9f9;
				}
				break;

			case DDR_STAGE_INIT:
				state->stage[ offset ].bit++;
				if( state->stage[ offset ].bit < 22 )
				{
					int a = ( ( ( ( ~0x06 ) | mask[ state->stage[ 0 ].bit ] ) & 0xff ) << 8 );
					int b = ( ( ( ( ~0x06 ) | mask[ state->stage[ 1 ].bit ] ) & 0xff ) << 0 );

					state->stage_mask = 0xffff0000 | a | b;
				}
				else
				{
					state->stage[ offset ].bit = 0;
					state->stage[ offset ].state = DDR_STAGE_IDLE;

					state->stage_mask = 0xffffffff;
				}
				break;
			}
		}
	}

	verboselog( machine, 2, "stage: %dp data clk=%d state=%d d0=%d shift=%08x bit=%d stage_mask=%08x\n", offset + 1, clk,
		state->stage[ offset ].state, state->stage[ offset ].DO, state->stage[ offset ].shift, state->stage[ offset ].bit, state->stage_mask );
}

static CUSTOM_INPUT( gn845pwbb_read )
{
	ksys573_state *state = field->port->machine->driver_data<ksys573_state>();

	return input_port_read(field->port->machine, "STAGE") & state->stage_mask;
}

static void gn845pwbb_output_callback( running_machine *machine, int offset, int data )
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
		gn845pwbb_do_w( machine, 0, !data );
		break;

	case 7:
		gn845pwbb_clk_w( machine, 0, !data );
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
		gn845pwbb_do_w( machine, 1, !data );
		break;

	case 15:
		gn845pwbb_clk_w( machine, 1, !data );
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
	ksys573_state *state = machine->driver_data<ksys573_state>();

	DRIVER_INIT_CALL(konami573);

	state->stage_mask = 0xffffffff;
	gx700pwfbf_init( machine, gn845pwbb_output_callback );

	state_save_register_global( machine, state->stage_mask );
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
		verboselog( space->machine, 0, "gtrfrks_io_r: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}

	verboselog( space->machine, 2, "gtrfrks_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

static WRITE32_HANDLER( gtrfrks_io_w )
{
	verboselog( space->machine, 2, "gtrfrks_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

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
		verboselog( space->machine, 0, "gtrfrks_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

static DRIVER_INIT( gtrfrks )
{
	DRIVER_INIT_CALL(konami573);

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_r, gtrfrks_io_w );
}

/* GX894 digital i/o */

static const UINT8 ds2401_xid[] =
{
	0x3d, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0x01
};

static READ32_HANDLER( gx894pwbba_r )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
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
			data |= state->gx894_ram[ state->gx894_ram_read_offset / 2 ];
//          printf( "reading %08x %04x\r", state->gx894_ram_read_offset, state->gx894_ram[ state->gx894_ram_read_offset / 2 ] );
			state->gx894_ram_read_offset += 2;
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
			data |= ds2401_read( space->machine, 2 ) << 28;
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

	verboselog( space->machine, 2, "gx894pwbba_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
//  printf( "%08x: gx894pwbba_r( %08x, %08x ) %08x\n", cpu_get_pc(space->cpu), offset, mem_mask, data );
	return data;
}

static char *binary( char *s, UINT32 data )
{
	int i;
	for( i = 0; i < 32; i++ )
	{
		s[ i ] = '0' + ( ( data >> ( 31 - i ) ) & 1 );
	}
	s[ i ] = 0;
	return s;
}

static void gx894pwbba_output( running_machine *machine, int offset, UINT8 data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	if( state->gx894pwbba_output_callback != NULL )
	{
		int i;
		static const int shift[] = { 0, 2, 3, 1 };
		for( i = 0; i < 4; i++ )
		{
			int oldbit = ( state->gx894pwbba_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				(*state->gx894pwbba_output_callback)( machine, ( offset * 4 ) + i, newbit );
			}
		}
	}
	state->gx894pwbba_output_data[ offset ] = data;
}

static WRITE32_HANDLER( gx894pwbba_w )
{
	ksys573_state *state = space->machine->driver_data<ksys573_state>();
	char buff[33];
	UINT32 olda=state->a,oldb=state->b,oldc=state->c,oldd=state->d;

//  printf( "gx894pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	if( offset == 4 )
	{
		return;
	}

	verboselog( space->machine, 2, "gx894pwbba_w( %08x, %08x, %08x) %s\n", offset, mem_mask, data, binary( buff, data ) );

	switch( offset )
	{
	case 0x2b:
		/* sound? */
		break;
	case 0x2c:
		if( ACCESSING_BITS_0_15 )
		{
			state->gx894_ram_write_offset &= 0x0000ffff;
			state->gx894_ram_write_offset |= ( data & 0x0000ffff ) << 16;
		}
		if( ACCESSING_BITS_16_31 )
		{
			state->gx894_ram_write_offset &= 0xffff0000;
			state->gx894_ram_write_offset |= ( data & 0xffff0000 ) >> 16;
		}
		break;
	case 0x2d:
		if( ACCESSING_BITS_0_15 )
		{
			state->gx894_ram[ state->gx894_ram_write_offset / 2 ] = data & 0xffff;
//          printf( "writing %08x %04x\r", state->gx894_ram_write_offset, state->gx894_ram[ state->gx894_ram_write_offset / 2 ] );
			state->gx894_ram_write_offset += 2;
		}
		if( ACCESSING_BITS_16_31 )
		{
			state->gx894_ram_read_offset &= 0x0000ffff;
			state->gx894_ram_read_offset |= ( data & 0xffff0000 ) << 0;
		}
		break;
	case 0x2e:
		if( ACCESSING_BITS_0_15 )
		{
			state->gx894_ram_read_offset &= 0xffff0000;
			state->gx894_ram_read_offset |= ( data & 0x0000ffff ) >> 0;
		}
		if( ACCESSING_BITS_16_31 )
		{
//          printf( "write offset 2e msw32\n" );
		}
		break;
	case 0x38:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( space->machine, 0, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( space->machine, 1, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &state->a );
		break;
	case 0x39:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( space->machine, 7, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( space->machine, 3, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &state->b );
		break;
	case 0x3b:
		if( ACCESSING_BITS_16_31 )
		{
			ds2401_write( space->machine, 2, !( ( data >> 28 ) & 1 ) );
		}
		break;
	case 0x3e:
		if( ACCESSING_BITS_0_15 )
		{
			/* 12 */
			/* 13 */
			/* 14 */
			/* 15 */

			/* fpga */
			state->s = ( state->s >> 1 ) | ( ( data & 0x8000 ) >> 8 );
			state->f++;
			if( state->f == 8 )
			{
//              printf( "%04x %02x\n", o, s );
				state->c = 0;
				state->f = 0;
				state->o++;
			}
		}

		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( space->machine, 4, ( data >> 28 ) & 0xf );
		}
		COMBINE_DATA( &state->c );
		break;
	case 0x3f:
		if( ACCESSING_BITS_16_31 )
		{
			gx894pwbba_output( space->machine, 2, ( data >> 28 ) & 0xf );
		}
		if( ACCESSING_BITS_0_15 )
		{
			gx894pwbba_output( space->machine, 5, ( data >> 12 ) & 0xf );
		}
		COMBINE_DATA( &state->d );
		break;
	default:
//      printf( "write offset %08x\n", offset );
		break;
	}
	if( state->a != olda || state->b != oldb || state->c != oldc || state->d != oldd )
	{
//      printf( "%08x %08x %08x %08x\n", a, b, c, d );
	}
}

static void gx894pwbba_init( running_machine *machine, void (*output_callback_func)( running_machine *machine, int offset, int data ) )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int gx894_ram_size = 24 * 1024 * 1024;

	state->gx894pwbba_output_callback = output_callback_func;

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f640000, 0x1f6400ff, 0, 0, gx894pwbba_r, gx894pwbba_w );

	state->gx894_ram_write_offset = 0;
	state->gx894_ram_read_offset = 0;
	state->gx894_ram = auto_alloc_array( machine, UINT16, gx894_ram_size/2 );

	ds2401_init( machine, 2, ds2401_xid ); /* todo: load this from roms */

	state_save_register_global_array( machine, state->gx894pwbba_output_data );
	state_save_register_global_pointer( machine, state->gx894_ram, gx894_ram_size / 4 );
}

/* ddr digital */

static DRIVER_INIT( ddrdigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine, gn845pwbb_output_callback );
}

/* guitar freaks digital */

static DRIVER_INIT( gtrfrkdigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine, NULL );

	memory_install_readwrite32_handler( cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f600000, 0x1f6000ff, 0, 0, gtrfrks_io_r, gtrfrks_io_w );
}

/* ddr solo */

static void ddrsolo_output_callback( running_machine *machine, int offset, int data )
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

	gx894pwbba_init( machine, ddrsolo_output_callback );
}

/* drummania */

static void drmn_output_callback( running_machine *machine, int offset, int data )
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

	gx700pwfbf_init( machine, drmn_output_callback );
}

static DRIVER_INIT( drmndigital )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine, drmn_output_callback );
}

/* dance maniax */

static void dmx_output_callback( running_machine *machine, int offset, int data )
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
	verboselog( space->machine, 2, "dmx_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

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
		verboselog( space->machine, 0, "dmx_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

static DRIVER_INIT( dmx )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine, dmx_output_callback );

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f600000, 0x1f6000ff, 0, 0, dmx_io_w );
}

/* salary man champ */

static void salarymc_lamp_callback( running_machine *machine, int data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int d = ( data >> 7 ) & 1;
	int rst = ( data >> 6 ) & 1;
	int clk = ( data >> 5 ) & 1;

	if( rst )
	{
		state->salarymc_lamp_bits = 0;
		state->salarymc_lamp_shift = 0;
	}

	if( state->salarymc_lamp_clk != clk )
	{
		state->salarymc_lamp_clk = clk;

		if( state->salarymc_lamp_clk )
		{
			state->salarymc_lamp_shift <<= 1;

			state->salarymc_lamp_shift |= d;

			state->salarymc_lamp_bits++;
			if( state->salarymc_lamp_bits == 16 )
			{
				if( ( state->salarymc_lamp_shift & ~0xe38 ) != 0 )
				{
					verboselog( machine, 0, "unknown bits in salarymc_lamp_shift %08x\n", state->salarymc_lamp_shift & ~0xe38 );
				}

				output_set_value( "player 1 red", ( state->salarymc_lamp_shift >> 11 ) & 1 );
				output_set_value( "player 1 green", ( state->salarymc_lamp_shift >> 10 ) & 1 );
				output_set_value( "player 1 blue", ( state->salarymc_lamp_shift >> 9 ) & 1 );

				output_set_value( "player 2 red", ( state->salarymc_lamp_shift >> 5 ) & 1 );
				output_set_value( "player 2 green", ( state->salarymc_lamp_shift >> 4 ) & 1 );
				output_set_value( "player 2 blue", ( state->salarymc_lamp_shift >> 3 ) & 1 );

				state->salarymc_lamp_bits = 0;
				state->salarymc_lamp_shift = 0;
			}
		}
	}
}

static DRIVER_INIT( salarymc )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	DRIVER_INIT_CALL(konami573);

	state->security_callback = salarymc_lamp_callback;

	state_save_register_global( machine, state->salarymc_lamp_bits );
	state_save_register_global( machine, state->salarymc_lamp_shift );
	state_save_register_global( machine, state->salarymc_lamp_clk );
}

/* Hyper Bishi Bashi Champ */

static void hyperbbc_lamp_callback( running_machine *machine, int data )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();
	int red = ( data >> 6 ) & 1;
	int blue = ( data >> 5 ) & 1;
	int green = ( data >> 4 ) & 1;
	int strobe1 = ( data >> 3 ) & 1;
	int strobe2 = ( data >> 0 ) & 1;

	if( strobe1 && !state->hyperbbc_lamp_strobe1 )
	{
		output_set_value( "player 1 red", red );
		output_set_value( "player 1 green", green );
		output_set_value( "player 1 blue", blue );
	}

	state->hyperbbc_lamp_strobe1 = strobe1;

	if( strobe2 && !state->hyperbbc_lamp_strobe2 )
	{
		output_set_value( "player 2 red", red );
		output_set_value( "player 2 green", green );
		output_set_value( "player 2 blue", blue );
	}

	state->hyperbbc_lamp_strobe2 = strobe2;
}

static DRIVER_INIT( hyperbbc )
{
	ksys573_state *state = machine->driver_data<ksys573_state>();

	DRIVER_INIT_CALL(konami573);

	state->security_callback = hyperbbc_lamp_callback;

	state_save_register_global( machine, state->hyperbbc_lamp_strobe1 );
	state_save_register_global( machine, state->hyperbbc_lamp_strobe2 );
}

/* Mambo A Go Go */

static void mamboagg_output_callback( running_machine *machine, int offset, int data )
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

static WRITE32_HANDLER( mamboagg_io_w )
{
	verboselog( space->machine, 2, "mamboagg_io_w( %08x, %08x ) %08x\n", offset, mem_mask, data );

	switch( offset )
	{
	case 0:
		output_set_led_value( 0, !( ( data >> 3 ) & 1 ) ); // start 1p
		output_set_value( "select right", !( ( data >> 4 ) & 1 ) );
		output_set_value( "select left", !( ( data >> 5 ) & 1 ) );
		break;

	default:
		verboselog( space->machine, 0, "mamboagg_io_w: unhandled offset %08x, %08x\n", offset, mem_mask );
		break;
	}
}

static DRIVER_INIT( mamboagg )
{
	DRIVER_INIT_CALL(konami573);

	gx894pwbba_init( machine, mamboagg_output_callback );

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1f600000, 0x1f6000ff, 0, 0, mamboagg_io_w );
}


/* ADC0834 Interface */

static double analogue_inputs_callback( device_t *device, UINT8 input )
{
	switch (input)
	{
	case ADC083X_CH0:
		return (double)( 5 * input_port_read_safe( device->machine, "analog0", 0 ) ) / 255.0;
	case ADC083X_CH1:
		return (double)( 5 * input_port_read_safe( device->machine, "analog1", 0 ) ) / 255.0;
	case ADC083X_CH2:
		return (double)( 5 * input_port_read_safe( device->machine, "analog2", 0 ) ) / 255.0;
	case ADC083X_CH3:
		return (double)( 5 * input_port_read_safe( device->machine, "analog3", 0 ) ) / 255.0;
	case ADC083X_AGND:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 0;
}


static const adc083x_interface konami573_adc_interface = {
	analogue_inputs_callback
};

static MACHINE_CONFIG_START( konami573, ksys573_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", PSXCPU, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konami573_map)
	MCFG_CPU_VBLANK_INT("screen", sys573_vblank)

	MCFG_MACHINE_RESET( konami573 )
	MCFG_NVRAM_HANDLER( konami573 )

	// onboard flash
	MCFG_FUJITSU_29F016A_ADD("onboard.0")
	MCFG_FUJITSU_29F016A_ADD("onboard.1")
	MCFG_FUJITSU_29F016A_ADD("onboard.2")
	MCFG_FUJITSU_29F016A_ADD("onboard.3")
	MCFG_FUJITSU_29F016A_ADD("onboard.4")
	MCFG_FUJITSU_29F016A_ADD("onboard.5")
	MCFG_FUJITSU_29F016A_ADD("onboard.6")
	MCFG_FUJITSU_29F016A_ADD("onboard.7")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC( 0 ))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 1024, 1024 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )

	MCFG_PALETTE_LENGTH( 65536 )

	MCFG_PALETTE_INIT( psx )
	MCFG_VIDEO_START( psx_type2 )
	MCFG_VIDEO_UPDATE( psx )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD( "spu", PSXSPU, 0 )
	MCFG_SOUND_CONFIG( konami573_psxspu_interface )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.0 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.0 )

	MCFG_M48T58_ADD( "m48t58" )

	MCFG_ADC0834_ADD( "adc0834", konami573_adc_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard1, konami573 )
	// flash for pccard 1
	MCFG_FUJITSU_29F016A_ADD("pccard1.0")
	MCFG_FUJITSU_29F016A_ADD("pccard1.1")
	MCFG_FUJITSU_29F016A_ADD("pccard1.2")
	MCFG_FUJITSU_29F016A_ADD("pccard1.3")
	MCFG_FUJITSU_29F016A_ADD("pccard1.4")
	MCFG_FUJITSU_29F016A_ADD("pccard1.5")
	MCFG_FUJITSU_29F016A_ADD("pccard1.6")
	MCFG_FUJITSU_29F016A_ADD("pccard1.7")
	MCFG_FUJITSU_29F016A_ADD("pccard1.8")
	MCFG_FUJITSU_29F016A_ADD("pccard1.9")
	MCFG_FUJITSU_29F016A_ADD("pccard1.10")
	MCFG_FUJITSU_29F016A_ADD("pccard1.11")
	MCFG_FUJITSU_29F016A_ADD("pccard1.12")
	MCFG_FUJITSU_29F016A_ADD("pccard1.13")
	MCFG_FUJITSU_29F016A_ADD("pccard1.14")
	MCFG_FUJITSU_29F016A_ADD("pccard1.15")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pccard2, konami573 )
	// flash for pccard 2
	MCFG_FUJITSU_29F016A_ADD("pccard2.0")
	MCFG_FUJITSU_29F016A_ADD("pccard2.1")
	MCFG_FUJITSU_29F016A_ADD("pccard2.2")
	MCFG_FUJITSU_29F016A_ADD("pccard2.3")
	MCFG_FUJITSU_29F016A_ADD("pccard2.4")
	MCFG_FUJITSU_29F016A_ADD("pccard2.5")
	MCFG_FUJITSU_29F016A_ADD("pccard2.6")
	MCFG_FUJITSU_29F016A_ADD("pccard2.7")
	MCFG_FUJITSU_29F016A_ADD("pccard2.8")
	MCFG_FUJITSU_29F016A_ADD("pccard2.9")
	MCFG_FUJITSU_29F016A_ADD("pccard2.10")
	MCFG_FUJITSU_29F016A_ADD("pccard2.11")
	MCFG_FUJITSU_29F016A_ADD("pccard2.12")
	MCFG_FUJITSU_29F016A_ADD("pccard2.13")
	MCFG_FUJITSU_29F016A_ADD("pccard2.14")
	MCFG_FUJITSU_29F016A_ADD("pccard2.15")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( k573bait, konami573 )

	/* Additional NEC Encoder */
	MCFG_UPD4701_ADD( "upd4701" )
MACHINE_CONFIG_END

static INPUT_PORTS_START( konami573 )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OUT0")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_cs_write)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_clk_write)
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("adc0834", adc083x_di_write)

	PORT_START("IN1")
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
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("adc0834", adc083x_do_read)
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

	PORT_START("IN2")
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

	PORT_START("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
//  PORT_BIT( 0xf0fff0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("CART")
	PORT_CONFNAME( 1, 0, "Security Cart" )
	PORT_CONFSETTING( 0, "Install" )
	PORT_CONFSETTING( 1, "Game" )

	PORT_START("CD")
	PORT_CONFNAME( 1, 0, "CD" )
	PORT_CONFSETTING( 0, "1" )
	PORT_CONFSETTING( 1, "2" )
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitbc )
	PORT_INCLUDE( konami573 )

	PORT_START( "uPD4701_y" )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_MINMAX( 0, 0xfff ) PORT_SENSITIVITY( 15 ) PORT_KEYDELTA( 8 ) PORT_RESET

	PORT_START( "uPD4701_switches" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitmc )
	PORT_INCLUDE( fbaitbc )

	PORT_START( "analog0" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1) PORT_REVERSE

	PORT_START( "analog1" )
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xdf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( ddr )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000f0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(gn845pwbb_read, NULL)

	PORT_START( "STAGE" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) /* multiplexor */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1)    /* multiplexor */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) /* multiplexor */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(2)    /* multiplexor */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( ddrsolo )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 1" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Right 1" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up 1" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 1" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 2" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Left 2" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Down 2" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Left 1" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_16WAY PORT_PLAYER(1) PORT_NAME( "P1 Up-Right 1" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME( "P1 Up 2" ) /* P2 BUTTON1 */
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

static INPUT_PORTS_START( hyperbbc )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 ) /* P1 UP */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */
INPUT_PORTS_END

static INPUT_PORTS_START( mamboagg )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 1 (Top Right)" ) /* COIN2 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 3 (Middle Right)" ) /* P1 UP */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 1 (Top Right)" ) /* P1 DOWN */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 2 (Bottom Left)" ) /* P1 BUTTON 1 */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 1 (Top Left)" ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 3 (Bottom Right)" ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 2 (Bottom Left)" ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 3 (Bottom Right)" ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 1 (Top Left)" ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Centre Pad 2 (Middle Left)" ) /* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 2 (Bottom Left)" ) /* P2 BUTTON1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 1 (Top Left)" ) /* P2 BUTTON2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 3 (Bottom Right)" ) /* P2 BUTTON3 */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 1 (Top Right)" ) /* P2 START */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 3 (Middle Right)" ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME( "Left Pad 2 (Middle Left)" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 3 (Middle Right)" ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Right Pad 2 (Middle Left)" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

#define SYS573_BIOS_A \
	ROM_REGION32_LE( 0x080000, "bios", 0 ) \
	ROM_LOAD( "700a01.22g",   0x0000000, 0x080000, CRC(11812ef8) SHA1(e1284add4aaddd5337bd7f4e27614460d52b5b48))

// BIOS
ROM_START( sys573 )
	SYS573_BIOS_A
ROM_END

// Games
ROM_START( bassangl )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ee1b32a7) SHA1(c0f6b14b054f5a95ce474e794a3e0ca78faac681) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765jaa02", 0, SHA1(4291711b1025733cb97f6da5dc3b03c189fcc37c) )
ROM_END

ROM_START( bassang2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc865ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(095cbfb5) SHA1(529ce0a7b0986cf7e64c37f466d6c2dac95cea7f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865jaa02", 0, SHA1(b98d9aa54f13aa73bea580d6494cb6a7f3217be3) )
ROM_END

ROM_START( cr589fw )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700b04", 0, SHA1(2f65f62eb7ae202153a8544989675989ed33316f) )
ROM_END

ROM_START( cr589fwa )
	SYS573_BIOS_A

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "700a04", 0, SHA1(554481f48eeb5daf8b4e7be2d66840d6c8454a52) )
ROM_END

ROM_START( darkhleg )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx706ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(72b42574) SHA1(79dc959f0ce95ccb9ac0dbf0a72aec973e91bc56) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "706jaa02", 0, SHA1(58bd06855988250028086cba6b3670372b9d96a0) )
ROM_END

ROM_START( fmania )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_FILL( 0x000000, 0x000224, 0xff )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "918xxb02", 0, SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( ddrextrm )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcc36ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c1601287) SHA1(929691a78f7bb6dd830f832f301116df0da1619b) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcc36ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c36jaa02", 0, SHA1(edeb45fff0e66151b1ba2fd67542064ccddb031e) )
ROM_END

ROM_START( ddru )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(c9e7fced) SHA1(aac4dde100091bc64d397f53484a0ffbf68b8101) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845uaa02", 0, SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( ddrj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(a16f42b8) SHA1(da4f1eb3eb2b28cb3a0bc74bb9b9945970f56ac2) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jba02", 0, SHA1(2d10378c89fe85682f262f0987f8366b9ea72f11) )
ROM_END

ROM_START( ddrja )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jaa.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jaa02", 0, SHA1(37ca16be25bee39a5692dee2fa5f0fa0addfaaca) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jaa01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddrjb )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27m",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.2", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.3", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27l",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.4", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.5", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27j",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31h",  0x000000, 0x200000, NO_DUMP )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27h",  0x000000, 0x200000, NO_DUMP )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845jab02", 0, SHA1(7bdcef37bf376c23153dfd1580de5666cc681335) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "845jab01", 1, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddra )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(327c4851) SHA1(f0939224af706fd103a67aae9c96518c1db90ac9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845aaa02", 0, SHA1(839e2f8698a1561ac364998b8b3158ef0dee6998) )
ROM_END

ROM_START( ddr2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "895jaa02", 0, SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn896ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "896jaa01", 0, SHA1(f802a0e2ba0147eb71c54d92af409c3010a5715f) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "984jaa01", 0, SHA1(5505c28be27bfa9648060fd799bcf0c2c5f608ed) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2ml )
	SYS573_BIOS_A

	ROM_REGION( 0x080000, "cpu2", 0 ) /* memory card reader */
	ROM_LOAD( "885a01.bin",   0x000000, 0x080000, CRC(e22d093f) SHA1(927f62f63b5caa7899392decacd12fea0e6fdbea) )

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(cbc984c5) SHA1(6c0cd78a41000999b4ffbd9fb3707738b50a9b50) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(5d187aea247eefc5c065566ab277acd8c942ba27) )
ROM_END

ROM_START( ddr3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887aaa02", 0, SHA1(c4136305b97123f5dfe3ecd34a10ddda0180da3d) )
ROM_END

ROM_START( ddr3mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(3a377cec) SHA1(5bf3107a89547bd7697d9f0ab8f67240e101a559) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(2f633432) SHA1(bce44f20a5a7318af6aea4fdfa8af64ddb76047c) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887jaa02", 0, SHA1(2d1bf2a1566292dc869afaa6486f5ecd3973ff62) )
ROM_END

ROM_START( ddr3mk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kba02", 0, SHA1(92a3844fab24f46c16dd96f9474d95fd001df603) )
ROM_END

ROM_START( ddr3mka )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(4ce86d32) SHA1(94cdb9873a7f7503acc3b763e9b49ec6af53533f) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(bb14f9bd) SHA1(9d0adf5a32d8bbcaaea2f701f5c7a5d51ee0b8bf) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "887kaa02", 0, SHA1(a80930dd66c2e2326e8792f2e7cf9116d9cd752c) )
ROM_END

ROM_START( ddr3mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea22ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ef370ff7) SHA1(cb7a043f8bfa535e54ae9af728031d1018ed0734) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca22ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(6883c82c) SHA1(6fef1dc7150066eee427db685b6c5fb350b7768d) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a22jaa02", 0, SHA1(2bf07d08f6acee562024b418b453d654fc40f8dd) )
ROM_END

ROM_START( ddr4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(7bd2a24f) SHA1(62c73a54c4ed7697cf81ddbf3d13d4b0ca827be5) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33aa.u1",   0x000000, 0x001014, BAD_DUMP CRC(f6feb2bd) SHA1(dfd5bd532338849289e2e4c155c0ca86e79b9ae5) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aaa02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e5230867) SHA1(44aea9ccc90d81e7f41e5e9a62b28fcbdd75363b) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "a33jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca33ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jaa02", 0, SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4ms )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea33ab.u1",   0x000000, 0x000224, BAD_DUMP CRC(32fb3d13) SHA1(3ca6c77438f96b13d2c05f13a10fcff89a1403a2) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33ab.u1",   0x000000, 0x001014, BAD_DUMP CRC(312ac13f) SHA1(05d733edc03cfc5ea03db6c683f59ed6ff860b5a) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33aba02", 0, SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4msj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a33jba.u1",    0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca33jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(00e4b531) SHA1(f421fc33642c5a3cd89fb14dc8cd601bdddd1f55) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "a33jba.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca33jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a33jba02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(10f1e9b8) SHA1(985bd26638964beebba5de4c7cda772b402d2e59) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(e9b6ce56) SHA1(f040fba2b2b446baa840026dcd10f9785f8cc0a3) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gca34ja.22h",  0x000000, 0x002000, CRC(80575c1f) SHA1(a0594ca0f75bc7d49b645e835e9fa48a73c3c9c7) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jaa02", 0, SHA1(1d5f9eb633f054ddbf9fba55d53e4ee263ba91dd) )
ROM_END

ROM_START( ddr4mps )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea34jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(babf6fdb) SHA1(a2ef6b855d42072f0d3c72c8de9aff1f867de3f7) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca34jb.u1",   0x000000, 0x001014, BAD_DUMP CRC(0c717300) SHA1(00d21f39fe90494ffec2f8799767cc46a9cd2b00) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gca34jb.22h",  0x000000, 0x002000, CRC(bc6c8bd7) SHA1(10ceec5c7bc5ca9fca88f3c083a7d97012982079) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a34jba02", 0, BAD_DUMP SHA1(1d5f9eb633f054ddbf9fba55d53e4ee263ba91dd) )
ROM_END

ROM_START( ddr5m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca27ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ec526036) SHA1(f47d94d19268fdfa3ae9d42db9f2e2f9be318f2b) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gca27ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a27jaa02", 0, SHA1(0324973c98b82b72b22d2f0cd43e1924b83be667) )
ROM_END

ROM_START( ddrbocd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(363f427e) SHA1(adec886a07b9bd91f142f286b04fc6582205f037) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "892jaa01", 0, SHA1(46ace0feef48a2a6643c3cb4ac9164ba0beeea94) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "895jaa02", 1, SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddrs2k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(36d18e2f) SHA1(e976047dfbee62de9ad9e5de8e7629a24c29d581) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(21073a3e) SHA1(afa12404ceb462b9016a41c40775da87aa09cfeb) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905aaa02", 0, SHA1(1fc0f3fcc7d5d23711967023ff02c1fc76479024) )
ROM_END

ROM_START( ddrs2kj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(a077b0a1) SHA1(8f247b38c933a104a325ebf1f1691ef260480e1a) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(b7a104b0) SHA1(0f6901e41640f729f8a084a33148a9b900475594) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge905ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "905jaa02", 0, SHA1(84931345611574afd53976a0807f4163348e3c15) )
ROM_END

ROM_START( ddrmax )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb19ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(2255626a) SHA1(cb70c4b551265ffc6cc41f7bd2678696e8067060) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcb19ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b19jaa02", 0, SHA1(a156ebdef395747c64e1829237e4e7932ae251a8) )
ROM_END

ROM_START( ddrmax2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb20ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(fb7e0f58) SHA1(e6da23257a2a2ba7c69e817a91a0a8864f009386) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcb20ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b20jaa02", 0, SHA1(3f378e922e3182f980d07d6b2b524e33c5a00549) )
ROM_END

ROM_START( ddrsbm )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq894ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(cc3a47de) SHA1(f6e2e101870370b1e295a4a9ed546aa2d8bc2010) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gq894ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "894jaa02", 0, SHA1(3b2e061996d12f0e7367a579208eb746d849e070) )
ROM_END

ROM_START( ddrusa )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gka44ua.u1",   0x000000, 0x001014, BAD_DUMP CRC(2ef7c4f1) SHA1(9004d27179ece86883d01b3e6bbfeebc1b478d57) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gka44ua.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a44uaa02", 0, SHA1(2cdbe1c62d16a2be65adb7e11331fce5c8e45504) )
ROM_END

ROM_START( drmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(7dca0b3f) SHA1(db6d5c527e2a99133b516e01433024d3173848c6) )

	ROM_REGION( 0x200000, "onboard.6", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "onboard.7", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 ) /* timekeeper */
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "881jad01", 0, SHA1(7d9d47bef636dbaa8d578f34ea9489e349d3d6df) ) // upgrade or bootleg?

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "881jaa02", 1, NO_DUMP )
ROM_END

ROM_START( drmn2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )
ROM_END

ROM_START( drmn2mpu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(1246fe5b) SHA1(b58d4f4c95e13abf639d645223565544bd79a58a) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(34deea99) SHA1(f179e22eaf30453bb94177ed9c25d7996f020c99) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "912jab02", 0, SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "912za01",  1, SHA1(033a310006efe164cc6a8276de42a5d555f9fea9) )
ROM_END

ROM_START( drmn3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(90e544fa) SHA1(1feb617c36bad41aa720a6e5d3ec9e5cb2030567) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(5af1b5da) SHA1(cf862ef9ab60e8da89af96266943137827e4a261) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "a23jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca23ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23jaa02", 0, SHA1(89e365f61a4db889621d7d9d9917bcfa2c09704e) )
ROM_END

ROM_START( dmx )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge874ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(c5536373) SHA1(1492221f7dd9485f7745ecb0a982a88c8e768e53) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "ge874ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874jaa", 0, BAD_DUMP SHA1(3338a784efdca4f8bdcc83d2c9a6bbe7f7046d5c) )
ROM_END

ROM_START( dmx2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(ecc75eb7) SHA1(af66ced28ba5e79ae32ae0ef12d2ebe4207f3822) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gca39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a39jaa02", 0, SHA1(3d021448df857c12f6d46a20e14ae0fc6d342dcc) )
ROM_END

ROM_START( dmx2majp )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca38ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(99a746b8) SHA1(333236e59a707ecaf840a66f9b947ceade2cf2c9) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.31m",  0x000000, 0x200000, CRC(a0f54ab5) SHA1(a5ae67d7619393779c79a2e227cac0675eeef538) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.27m",  0x000000, 0x200000, CRC(6c3934b8) SHA1(f0e4a692b6caaf60fefaec87fd23da577439f69d) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gca38ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a38jaa02", 0, SHA1(d26c481ef8a70bba75bcdf41f9ceb3a49c245986) )
ROM_END

ROM_START( dncfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gk874ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(7a6f4672) SHA1(2e009e57760e92f48070a69cff5597c37a4783a2) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gk874ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "874kaa", 0, BAD_DUMP SHA1(4d1e843417ea96635eeba0cef944e83fdb72565c) )
ROM_END

ROM_START( dsem )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge936ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(0f5b7ae3) SHA1(646dd49da1216cc2d3d6920bc9b3447d55ebfbf0) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "ge936ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "936eaa", 0, BAD_DUMP SHA1(7cacc15ae065d47af31f1008374ec8241dba0d55) )
ROM_END

ROM_START( dsem2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gkc23ea.u1",   0x000000, 0x001014, BAD_DUMP CRC(aec2421a) SHA1(5ea9e9ce6161ebc99a50db0b7304385511bd4553) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gkc23ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c23eaa02", 0, SHA1(46868c97530db5be1b43ffa32744e3e12495c243) )
ROM_END

ROM_START( dsfdct )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "ge887ja_gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(08a60147) SHA1(0d39dca5e9e17fff0e64f296c8416e4ca23fdc1b) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910jc.u1",   0x000000, 0x000084, BAD_DUMP CRC(3c1ca973) SHA1(32211a72e3ac88b2723f82dac0b26f93031b3a9c) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "ge887ja_gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gc910jc.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jca02", 0, SHA1(0c868f3c9f696d291e8f27687e3ad83e453a4894) )
ROM_END

ROM_START( dsfdcta )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x0000084, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gc910ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(59a23808) SHA1(fcff1c68ff6cfbd391ac997a40fb5253fc62de82) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gc910ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "910jaa02", 0, SHA1(70851c383e3876c4a697a99706fbaae2dafcb0e0) )
ROM_END

ROM_START( dsfdr )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "gea37ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(5321055e) SHA1(d06b0dca9caba8249d71340469ad9083b02fd087) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca37ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(b6d9e7f9) SHA1(bc5f491de53a96d46745df09bc94e7853052296c) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "gea37ja.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca37ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a37jaa02", 0, SHA1(c6a23b910e884aa0d4afc388dbc8379e0d09611a) )
ROM_END

ROM_START( dsftkd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(ce6b98ce) SHA1(75549d9470345ce06d2706d373b19416d97e5b9a) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "884jaa02", 0, SHA1(80f02fcb7ea5b6394a2a58f12b73d87a1826d7f4) )
ROM_END

ROM_START( dstage )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gn845ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(db643af7) SHA1(881221da640b883302e657b906ea0a4e74555679) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "845ea", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( fbait2bc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc865ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8f0b4b) SHA1(363b1ea1a520b239ba8bca867366bbe8a9977a43) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "865uab02", 0, SHA1(d14dc066d4c16fba1e9b31d5f042ad249c4b5137) )
ROM_END

ROM_START( fbaitbc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "ge765ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(588748c6) SHA1(ea1ead61e0dcb324ef7b6106cae00bcf6702d6c4) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "765uab02", 0, SHA1(07b09e763e4b90108aa924b518221b16667a7133) )
ROM_END

ROM_START( fbaitmc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(753ad84e) SHA1(e024cefaaee7c9945ccc1f9a3d896b8560adce2e) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ea", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmca )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9c22aae8) SHA1(c107b0bf7fa76708f2d4f6aaf2cf27b3858378a3) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889aa", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(6278603c) SHA1(d6b59e270cfe4016e12565aedec8a4f0702e1a6f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ja", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx889ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(67b91e54) SHA1(4d94bfab08e2bf6e34ee606dd3c4e345d8e5d158) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "889ua", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( gtrfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(06bd6c4f) SHA1(61930e467ad135e2f31393ff5af981ed52f3bef9) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886ea", 0, BAD_DUMP SHA1(c0118b5539902e75853403a4979869d18c3d1b86) )
ROM_END

ROM_START( gtrfrksu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(143eaa55) SHA1(51a4fa3693f1cb1646a8986003f9b6cc1ae8b630) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886ua", 0, BAD_DUMP SHA1(c0118b5539902e75853403a4979869d18c3d1b86) )
ROM_END

ROM_START( gtrfrksj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(11ffd43d) SHA1(27f4f4d782604379254fb98c3c57e547aa4b321f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886ja", 0, BAD_DUMP SHA1(c0118b5539902e75853403a4979869d18c3d1b86) )
ROM_END

ROM_START( gtrfrksa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(efa51ee9) SHA1(3374d936de69c287e0161bc526546441c2943555) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "886aa", 0, BAD_DUMP SHA1(c0118b5539902e75853403a4979869d18c3d1b86) )
ROM_END

ROM_START( gtrfrk2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(687868c4) SHA1(1230e74e4cf17953febe501df56d8bbec1de9356) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP SHA1(4f6bb0150ad6ed574dd7583ccd60604028663b2a) )
ROM_END

ROM_START( gtrfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jac01", 0, SHA1(ff017dd5c0ecbdb8935d0d4656a45e9fab10ef82) )

	DISK_REGION( "cdrom1" )
	DISK_IMAGE_READONLY( "949jab02", 1, SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(96c21d71) SHA1(871f1f0429154a486e547e182534db1557008dd6) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(8645e17f) SHA1(e8a833384cb6bdb05870fcd44e7c8ed48a03c852) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jab02", 0, SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3mb )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* game security cart eeprom */
	ROM_LOAD( "ge949jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(61f35ee1) SHA1(0a2b66742364d76ec18647b2761590bd49229625) )

	ROM_REGION( 0x000008, "user9", 0 ) /* game security cart id */
	ROM_LOAD( "ge949jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "949jaz02", 0, SHA1(b0c786ba420a34fcbd16bc36a137f6ae87b7dfa8) )
ROM_END

ROM_START( gtrfrk4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a24jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(29e326fe) SHA1(41a600105b08accc9d7ebd2b8ae08c0863758aa0) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gea24ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(d1fccf11) SHA1(6dcd79f3171d6e4bd7e1149901638f8ea58ff623) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "a24jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gea24ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a24jaa02", 0, SHA1(bc0303f5a6a19484cd35890cc9934ee0bcabb2ad) )
ROM_END

ROM_START( gtrfrk5m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gea26jaa.u1",  0x000000, 0x001014, BAD_DUMP CRC(c2725fca) SHA1(b70a3266c61af5cbe0478a6f3dd850ebcab980dc) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.31m", 0x000000, 0x200000, CRC(1a25e660) SHA1(dbd8fad0bac307723c70d00763cadf4261a7ed73) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.27m", 0x000000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gea26jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a26jaa02", 0, SHA1(9909e08abff780db6fd7a5fbcc57ffbe14ae08ce) )
ROM_END

ROM_START( gtrfrk6m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb06ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(673c98ab) SHA1(b1d889bf4fc5e425056acb6b72b2c563966fb7d7) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcb06ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b06jaa02", 0, SHA1(2ea53ef492da63183a28c54afde07fce323fe42e) )
ROM_END

ROM_START( gtrfrk7m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcb17jaa.u1",   0x000000, 0x001014, BAD_DUMP CRC(5a338c31) SHA1(0fd9ee306335858dd6bef680a62557a8bf055cc3) )

	ROM_REGION( 0x200000, "onboard.0", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.31m", 0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "onboard.1", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.27m", 0x000000, 0x200000, CRC(7e7da9a9) SHA1(1882418779a48b5aefd113895756116379a6a4f7) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcb17jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b17jaa02", 0, SHA1(d38dc22011b71b0e4167f1728a8794ea4b9c5396) )
ROM_END

ROM_START( gtfrk11m )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gcd39ja.u1",   0x000000, 0x001014, BAD_DUMP CRC(9bd81d0a) SHA1(c95f6d7317bf88177f7217de4ba4376485d5cdbf) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gcd39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "d39jaa02", 0, SHA1(7a87ee331ba0301bb8724c398e6c77cfb9c172a7) )
ROM_END

ROM_START( hyperbbc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx908ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(fb6c0635) SHA1(0d974462a0a244ffb1a651adb316242cde427756) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( hyperbbck )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx908ka.u1",  0x000000, 0x000084, BAD_DUMP CRC(f4f37fe1) SHA1(30f90cdb2d092e4f8d6c14cfd4ca4945e6d352cb) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "908a02", 0, SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( konam80a )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(9b38b959) SHA1(6b4fca340a9b1c2ae21ad3903c1ac1e39ab08b1a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826aaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80j )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(e9e861e8) SHA1(45841db0b42d096213d9539a8d076d39391dca6d) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826jaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ka.u1", 0x000000, 0x000224, BAD_DUMP CRC(d41f7e38) SHA1(73e2bb132e23be72e72ea5b0686ccad28e47574a) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826kaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80s )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(6ce4c619) SHA1(d2be08c213c0a74e30b7ebdd93946374cc64457f) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826eaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80u )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gc826ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(0577379b) SHA1(3988a2a5ef1f1d5981c4767cbed05b73351be903) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "826uaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( mamboagg )
	SYS573_BIOS_A

	ROM_REGION( 0x0001014, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gqa40jab.u1",  0x000000, 0x001014, BAD_DUMP CRC(fd9e7c1f) SHA1(6dd4790589d48803f58328d099c908f0565b2c01) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gqa40jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(a914842442b2f63465e16f979f06da0b78a5f13e) )
ROM_END

ROM_START( pbballex )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gx802ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(ea8bdda3) SHA1(780034ab08871631ef0e3e9b779ca89e016c26a8) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "802jab02", 0, SHA1(bcc2b6c3515e2420eef9fdf8b28115368a428a92) )
ROM_END

ROM_START( pcnfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "user2", 0 ) /* install security cart eeprom */
	ROM_LOAD( "a23kaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(d71c4b5c) SHA1(3911c5dd933c30e6e44c8cf417bb4c284ecb4b80) )

	ROM_REGION( 0x0001014, "user8", 0 ) /* game security cart eeprom */
	ROM_LOAD( "gca23ka.u1",   0x000000, 0x001014, BAD_DUMP CRC(f392349c) SHA1(e7eb7979db276de560d5820163a70d97e6c023d8) )

	ROM_REGION( 0x000008, "user9", 0 ) /* install security cart id */
	ROM_LOAD( "a23kaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "user10", 0 ) /* game security cart id */
	ROM_LOAD( "gca23ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a23kaa02", 0, SHA1(5b853cc25eb583ed36d8cd402235b4f5c9ce065a) )
ROM_END

ROM_START( salarymc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "user2", 0 ) /* security cart eeprom */
	ROM_LOAD( "gca18jaa.u1",  0x000000, 0x000084, CRC(c9197f67) SHA1(8e95a89008f756a79295f2cb557c39efca1351e7) )

	ROM_REGION( 0x000008, "user9", 0 ) /* security cart id */
	ROM_LOAD( "gca18jaa.u6",  0x000000, 0x000008, CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a18ja", 0, BAD_DUMP SHA1(bb147b9a4871f1ddd108e3e503479221d87ec545) )
ROM_END

// System 573 BIOS (we're missing the later version that boots up with a pseudo-GUI)
GAME( 1998, sys573,   0,        konami573, konami573, konami573,  ROT0, "Konami", "System 573 BIOS", GAME_IS_BIOS_ROOT )

GAME( 1998, darkhleg, sys573,   konami573, konami573, konami573,  ROT0, "Konami", "Dark Horse Legend (GX706 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, fbaitbc,  sys573,   k573bait,  fbaitbc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - A Bass Challenge (GE765 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, bassangl, fbaitbc,  k573bait,  fbaitbc,   ge765pwbba, ROT0, "Konami", "Bass Angler (GE765 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
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
GAME( 1998, fbait2bc, sys573,   k573bait,  fbaitbc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, bassang2, fbait2bc, k573bait,  fbaitbc,   ge765pwbba, ROT0, "Konami", "Bass Angler 2 (GE865 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn,     sys573,   konami573, drmn,      drmn,       ROT0, "Konami", "DrumMania (GQ881 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, gtrfrks,  sys573,   konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. EAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksu, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. UAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksj, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrksa, gtrfrks,  konami573, gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks (GQ886 VER. AAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmc,  sys573,   k573bait,  fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. EA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcu, fbaitmc,  k573bait,  fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. UA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmcj, fbaitmc,  k573bait,  fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. JA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, fbaitmca, fbaitmc,  k573bait,  fbaitmc,   ge765pwbba, ROT0, "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. AA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2m,    sys573,   konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix (GN895 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddrbocd,  ddr2m,    pccard1,   ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2ml,   ddr2m,    pccard1,   ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc,   ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, ddr2mc2,  ddr2m,    konami573, ddr,       ddr,        ROT0, "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2m, sys573,   pccard1,   gtrfrks,   gtrfrks,    ROT0, "Konami", "Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dsftkd,   sys573,   konami573, ddr,       ddr,        ROT0, "Konami", "Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fw,  sys573,   konami573, konami573, konami573,  ROT0, "Konami", "CD-ROM Drive Updater 2.0 (700B04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, cr589fwa, sys573,   konami573, konami573, konami573,  ROT0, "Konami", "CD-ROM Drive Updater (700A04)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mk,   sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 2000, ddr3mka,  ddr3mk,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddr3ma,   ddr3mk,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.1 */
GAME( 1999, ddr3mj,   ddr3mk,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.0 */
GAME( 1999, ddrsbm,   sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, ddrs2k,   sys573,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddrs2kj,  ddrs2k,   konami573, ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.2 */
GAME( 1999, hyperbbc, sys573,   konami573, hyperbbc,  hyperbbc,   ROT0, "Konami", "Hyper Bishi Bashi Champ (GX908 1999/08/24 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, hyperbbck,hyperbbc, konami573, hyperbbc,  hyperbbc,   ROT0, "Konami", "Hyper Bishi Bashi Champ (GX908 1999/08/24 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, dsfdct,   sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JCA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1999, dsfdcta,  dsfdct,   pccard2,   ddr,       ddr,        ROT0, "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1999, drmn2m,   sys573,   konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2mpu, drmn2m,   konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 2000, dncfrks,  sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Freaks (G*874 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmx,      dncfrks,  konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax (G*874 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dsem,     sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage Euro Mix (G*936 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.7 */
GAME( 2000, gtrfrk3m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAC)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3ma, gtrfrk3m, pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3mb, gtrfrk3m, pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, salarymc, sys573,   konami573, hyperbbc,  salarymc,   ROT0, "Konami", "Salary Man Champ (GCA18 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 2000, ddr3mp,   sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, pcnfrk3m, sys573,   konami573, drmn,      drmndigital,ROT0, "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. KAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, drmn3m,   pcnfrk3m, konami573, drmn,      drmndigital,ROT0, "Konami", "DrumMania 3rd Mix (G*A23 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, gtrfrk4m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 4th Mix (G*A24 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4m,    sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. AAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mj,   ddr4m,    pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4ms,   sys573,   pccard2,   ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. ABA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4msj,  ddr4ms,   pccard2,   ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. JBA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, dsfdr,    sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage Featuring Disney's Rave (GCA37JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddrusa,   sys573,   konami573, ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution USA (G*A44 VER. UAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mp,   sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, ddr4mps,  sys573,   pccard2,   ddrsolo,   ddrsolo,    ROT0, "Konami", "Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, dmx2m,    sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix (G*A39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, fmania,   sys573,   konami573, konami573, konami573,  ROT0, "Konami", "Fighting Mania (918 xx B02)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 2001, gtrfrk5m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 5th Mix (G*A26 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, ddr5m,    sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution 5th Mix (G*A27 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, dmx2majp, sys573,   konami573, dmx,       dmx,        ROT0, "Konami", "Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, mamboagg, sys573,   konami573, mamboagg,  mamboagg,   ROT0, "Konami", "Mambo A Go-Go (GQA40 VER. JAB)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, gtrfrk6m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 6th Mix (G*B06 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk7m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 7th Mix (G*B17 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, ddrmax,   sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "DDR Max - Dance Dance Revolution 6th Mix (G*B19 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, ddrmax2,  sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "DDR Max 2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, dsem2,    sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dancing Stage Euro Mix 2 (G*C23 VER. EAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, ddrextrm, sys573,   pccard2,   ddr,       ddrdigital, ROT0, "Konami", "Dance Dance Revolution Extreme (G*C36 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, gtfrk11m, sys573,   pccard1,   gtrfrks,   gtrfrkdigital,ROT0, "Konami", "Guitar Freaks 11th Mix (G*D39 VER. JAA)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NOT_WORKING ) /* BOOT VER 1.95 */
