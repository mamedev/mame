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
  GunMania (and probably GunMania Zone Plus) has no CDROM drive.
  There is a slot for a security cart (cart is installed in CN14) and also a PCMCIA card slot.
  The main board and CDROM drive are housed in a black metal box. GunMania doesn't have that box.
  The games can be swapped by exchanging the CDROM disc and the security cart, whereby the main-board
  FlashROMs are re-programmed after a small wait. On subsequent power-ups, there is a check to test if the
  contents of the FlashROMs matches the CDROM, then the game boots up immediately.

  PCMCIA card slot is used by Dance Dance Revolution (2ndMIX link ver. and later),
  GUITARFREAKS (2ndMIX link ver. and later), drummania (7thMIX and later), and GunMania.
  DDR and GF: 32M flash card is used to store edit data (players can edit data by PlayStation
  console, and send data to arcade machine via PS memory card).
  DM and GF: network PCB unit (for e-AMUSEMENT) is connected via PCMCIA slot.
  GM: unknown (program data is stored in flash card?)

  The games that run on this system include...

  Game                                                Year       Hardware Code     CD Code
  --------------------------------------------------------------------------------------------
P Anime Champ                                         2000.12    GCA07 JA          (no CD)
P Bass Angler                                         1998.03    GE765 JA          765 JA A02
P Bass Angler 2                                       1998.07    GC865 JA          865 JA A02
K Dance Dance Revolution Karaoke MIX (DAM-DDR)        1999.11    GQ921 JB          921 JB B02
K Dance Dance Revolution Karaoke MIX (DAM-DDR) 2nd    2000.07    GQ947 JA          947 JA A02
A Dance Dance Revolution                              1998.09    GC845 JA          845 JA(missing)/UA A01 / 845 JA A02
A Dance Dance Revolution Internet Ranking ver.        1998.11    GC845 JB          845 JB A01 / 845 JA/UA A02
A Dance Dance Revolution 2ndMIX                       1999.01    GC895 JA          895 JA A02
A Dance Dance Revolution 2ndMIX LINK version          1999.04    GE885 JA          885 JA A02
A DDR 2ndMIX with bmIIDX CLUB ver.                    1999.05    GN896 JA          896 JA A01
A DDR 2ndMIX AND bmIIDX substream CLUB ver. 2         1999.05    GE984 JA          984 JA A01
D Dance Dance Revolution Solo BASS MIX                1999.08    GQ894 JA          894 JA A02
D Dance Dance Revolution 3rdMIX                       1999.10    GN887 AA/JA/KA    887 AA/JA/KA A02
D Dance Dance Revolution Solo 2000                    1999.12    GC905 AA/JA       905 AA/JA A02
D Dance Dance Revolution 3rdMIX PLUS                  2000.06    GCA22 JA          A22 JA A02
D Dance Dance Revolution 4thMIX                       2000.08    GCA33 JA          A33 JA A02
D Dance Dance Revolution 4thMIX PLUS                  2000.12    GCA34 JA          A34 JA A02
D Dance Dance Revolution 5thMIX                       2001.03    GCA27 JA          A27 JA A02
D DDRMAX Dance Dance Revolution 6thMIX                2001.10    GCB19 JA          B19 JA A02
D DDRMAX2 Dance Dance Revolution 7thMIX               2002.03    GCB20 JA          B20 JA A02
D Dance Dance Revolution EXTREME                      2002.12    GCC36 JA          C36 JA A02
D Dance Maniax                                        2000.06    GE874 JA          874 JA A(needs redump)
D Dance Maniax 2ndMIX                                 2000.12    GCA39 JA          A39 JA A02
D Dance Maniax 2ndMIX APPEND J PARADISE               2001.04
A Dancing Stage                                       1999.08    GN845 EA          845 EA(needs redump)
D Dancing Stage Euro Mix                              2000       GE936 EA          936 EA A(needs redump)
D Dancing Stage Euro Mix 2                            2000       G*C23 EA          C23 EA A02
D Dancing Stage featuring Disney's Rave               2000.11    GCA37 JA          A37 JA A02
D Dancing Stage featuring DREAMS COME TRUE            1999.12    GC910 JA          910 JA/JC A02
A Dancing Stage featuring TRUE KiSS DESTiNATiON       1999.07    G*884 JA          884 JA A02
P Dark Horse Legend                                   1998.03    GX706 JA          706 JA A02
A drummania                                           1999.07    GQ881 JA          881 JA D01 / 881 JA A02
D drummania 2ndMIX                                    2000.03    GE912 JA          912 JA B02
D drummania 3rdMIX                                    2000.09    GCA23 JA          A23 JA A02
D drummania 4thMIX                                    2001.03    GEA25 JA          A25 JA A02
D drummania 5thMIX                                    2001.09    GCB05 JA          B05 JA A02
D drummania 6thMIX                                    2002.02    GCB16 JA          B16 JA A02
N drummania 7thMIX                                    2002.08    GCC07 JA          C07 JA A02
N drummania 7thMIX power-up ver.                      2002.08    GEC07 JB          C07 JC A02
N drummania 8thMIX                                    2003.04    GCC38 JA          C38 JA A02
N drummania 9thMIX                                    2003.10    GCD09 JA          D09 JA A02
N drummania 10thMIX                                   2004.04
? Fighting Mania                                      2000
P Fisherman's Bait                                    1998.06    GE765 UA          765 UA B02
P Fisherman's Bait 2                                  1998       GC865 UA          865 UA B02
P Fisherman's Bait Marlin Challenge                   1999       GX889             889 AA/EA/JA/UA(needs redump)
P Gacha Gachamp                                       1999.01    GQ877 JA          GE877-JA(PCMCIA card)
P Great Bishi Bashi Champ                             2002.??    GBA48 JA          (no CD)
A GUITARFREAKS                                        1999.02    GQ886 EA/JA/UA/AA 886 ** A02/C02/D02
A GUITARFREAKS 2ndMIX                                 1999.07    GQ883 JA          883 ** A02
A GUITARFREAKS 2ndMIX Link Kit 1                      1999.09    GE929 JA          929 JA A02
A GUITARFREAKS 2ndMIX Link Kit 2                      1999.11    GC929 JB          929 JB B02
D GUITARFREAKS 3rdMIX                                 2000.04    GE949 JA          949 JA C01 / 949 JA C02
D GUITARFREAKS 4thMIX                                 2000.08    GEA24 JA          A24 JA A02
D GUITARFREAKS 5thMIX                                 2001.03    GCA26 JA          A26 JA A02
D GUITARFREAKS 6thMIX                                 2001.09    GCB06 JA          B06 JA A02
D GUITARFREAKS 7thMIX                                 2002.02    GCB17 JA          B17 JA A02
N GUITARFREAKS 8thMIX                                 2002.08    GCC08 JA          C08 JA A02
N GUITARFREAKS 8thMIX power-up ver.                   2002.11    GEC08 JB          C08 JB A02
N GUITARFREAKS 9thMIX                                 2003.04    GCC39 JA          C39 JA A02
N GUITARFREAKS 10thMIX                                2003.10    GCD10 JA          D10 JA A02
N GUITARFREAKS 11thMIX                                2004.04
G GunMania                                            2000.07    G?906 JA          (no CD)
? *GunMania Zone Plus                                 2000.10    GCA15 JA
P Handle Champ                                        1997.12    GQ710 JA          (no CD)
P Hyper Bishi Bashi Champ                             1998.07    GC876 EA          (no CD)
P Hyper Bishi Bashi Champ - 2 Player                  1999.08    GC908 JA          908    A02
P Jikkyou Pawafuru Puro Yakyu EX                      1998.04    GX802 JA          802 JA B02
P Jikkyou Pawafuru Puro Yakyu EX 98                   1998.08
AA Kick & Kick                                        2001       GNA36 EA          (no CD)
P Konami 80's Arcade Gallery                          1998.11    GC826 JA          826 JA A01
P Konami 80's AC Special                              1998       GC826 UA          826 UA A01
D Mambo a GoGo                                        2001.06
D Punchmania Hokuto no Ken                            2000.03                      918 JA B02
D Punchmania Hokuto no Ken 2                          2000.12                      A09 JA A02
P *Salary Man Champ
P Salary Man Champ - 2 Player                         2001.02    GCA18 JA          A18 JA(needs redump)
P Step Champ                                          1999.12

P: plain System573
A: uses ext. analog I/O board GX700-PWB(F)
AA: uses alt. ext. analog I/O board GX700-PWB(K)
D: uses ext. digital sound and I/O board GX894-PWB(B)
N: uses network PCB unit GUC07 + ext. digital sound and I/O board GX894-PWB(B)
G: gun mania only, drives air soft gun (this game uses real BB bullet)
K: uses karaoke I/O board GX921-PWB(B)


  Note:
       Not all games listed above are confirmed to run on System 573.
       * - denotes undumped.

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
  SM5877    - Nippon Precision Circuits SM5877 2-channel D/A converter (SSOP24, @32D)
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


  Auxiliary Controls PCB
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
 Notes: This PCB is used for Gacha Gachamp. No ICs.

        CN5 - To control lever unit (1P). uses 9 pins out of 15 pins of B15P-SHF-1AA
        CN6 - To control lever unit (2P). uses 9 pins out of 14 pins of B14P-SHF-1AA
        (CN4, CN3, CN2 is printed pattern only, no actual connector)


GX700-PWB(K)A (C)2000 KONAMI
|-----------------------------|
|    CN9        CN4         |
|         CN3          CN5    |
|                             |
|   U2 U1                     |
|                             |-----------------|
|                                    CN6        |
|         U3   U4 U5   U6                       |
|   U7    U8                         U9-U16     |
|               U17  U18                        |
|                                               |
|    U19                                        |
|         U20   U21            CN1              |
|        CN7   CN8             CN2              |
|-----------------------------------------------|
Notes: (all ICs shown)

        CN1        - (bottom) Connector joining this PCB to the MAIN PCB
        CN2        - (unpopulated custom 80-pin)
        CN3        - JST ??12
        CN4        - JST ??12
        CN5        - JST ??12
        CN6        - JST ??10
        CN7        - (unpopulated 5-pin)
        CN8        - (unpopulated 8-pin)
        CN9        - (unpopulated 4-pin)
        U1         - Maxim DS2401 (SOIC6)
        U2         - (unpopulated SOIC8)
        U3         - Motorola 74LS74A (SOIC14)
        U4,U5,U6   - Motorola 74LS244 (SOIC20)
        U7         - (unpopulated 4-pad)
        U8         - AMD PALCE16V8Q-15, stamped 'X700K01' (DIP20)
        U9-16      - Sharp PC817XF (DIP4)
        U17        - TI 74LV245A (SOIC20)
        U18        - Motorola 74LS273 (SOIC20)
        U19        - (unpopulated PLCC44 socket)
        U20        - (unpopulated SOIC16)
        U21        - (unpopulated SOIC16)

*/

#include "emu.h"

#include "k573cass.h"
#include "k573dio.h"
#include "k573kara.h"
#include "k573mcal.h"
#include "k573mcr.h"
#include "k573msu.h"

#include "cpu/psx/psx.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/cr589.h"
#include "bus/pccard/k573npu.h"
#include "bus/pccard/konami_dual.h"
#include "bus/pccard/linflash.h"
#include "machine/adc083x.h"
#include "machine/bankdev.h"
#include "machine/ds2401.h"
#include "machine/jvshost.h"
#include "machine/mb89371.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "machine/upd4701.h"
#include "sound/spu.h"
#include "sound/cdda.h"
#include "video/psx.h"

#include "screen.h"
#include "speaker.h"

#include "cdrom.h"

#include <algorithm>

#include "pnchmn.lh"

#define LOG_CDROM    (1U << 1)
#define LOG_CONTROL  (1U << 2)
#define LOG_SECURITY (1U << 3)
#define LOG_JVS      (1U << 4)
#define LOG_IOBOARD  (1U << 5)
// #define VERBOSE      (LOG_GENERAL | LOG_CDROM | LOG_CONTROL | LOG_SECURITY | LOG_JVS | LOG_IOBOARD)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCDROM(...)    LOGMASKED(LOG_CDROM, __VA_ARGS__)
#define LOGCONTROL(...)  LOGMASKED(LOG_CONTROL, __VA_ARGS__)
#define LOGSECURITY(...) LOGMASKED(LOG_SECURITY, __VA_ARGS__)
#define LOGJVS(...)      LOGMASKED(LOG_JVS, __VA_ARGS__)
#define LOGIOBOARD(...)  LOGMASKED(LOG_IOBOARD, __VA_ARGS__)

#define ATAPI_CYCLES_PER_SECTOR ( 30000 )  // plenty of time to allow DMA setup etc.  BIOS requires this be at least 2000, individual games may vary.

/*
 * Class declaration for sys573_jvs_host
 */

DECLARE_DEVICE_TYPE(SYS573_JVS_HOST, sys573_jvs_host)

class sys573_jvs_host : public jvs_host
{
public:
	// construction/destruction
	sys573_jvs_host(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void send_packet(uint8_t *data, int length);
	int received_packet(uint8_t *buffer);

	int jvs_sense_r();

private:
	int output_buffer_size = 0;
};

DEFINE_DEVICE_TYPE(SYS573_JVS_HOST, sys573_jvs_host, "sys573_jvs_host", "JVS Host (System 573)")

sys573_jvs_host::sys573_jvs_host(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: jvs_host(mconfig, SYS573_JVS_HOST, tag, owner, clock)
{
	output_buffer_size = 0;
}

int sys573_jvs_host::jvs_sense_r()
{
	return !get_address_set_line();
}

int sys573_jvs_host::received_packet(uint8_t *buffer)
{
	if (jvs_sense_r()) {
		// The game will send the command twice to reset, but the command
		// shouldn't return any data or else a "JVS SUBS RESET ERROR" appears
		return 0;
	}

	uint32_t length;
	const uint8_t *data;

	get_raw_reply(data, length);

	if (length > 0) {
		// The games don't unescape the data in memory.
		// This causes issues any time 0xe0 or 0xd0 shows up in
		// the original response data and were escaped.
		// Sending an unescaped "encoded" packet works perfectly
		// in-game.
		uint8_t checksum = std::accumulate(data, data + length, 0);

		buffer[0] = 0xe0;
		memcpy(buffer + 1, data, length);
		buffer[length+1] = checksum;
		buffer[length+2] = 0;
		length += 2;

		commit_encoded();
	}

	return (int)length;
}

void sys573_jvs_host::send_packet(uint8_t *data, int length)
{
	while (length > 0)
	{
		push(*data);
		data++;
		length--;
	}

	commit_raw();
}


namespace {

class ksys573_state : public driver_device
{
public:
	ksys573_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sys573_jvs_host(*this, "sys573_jvs_host"),
		m_k573dio(*this, "k573dio"),
		m_lamps(*this, "lamp%u", 0U),
		m_analog0(*this, "analog0"),
		m_analog1(*this, "analog1"),
		m_analog2(*this, "analog2"),
		m_analog3(*this, "analog3"),
		m_psxirq(*this, "maincpu:irq"),
		m_ata(*this, "ata"),
		m_image(*this, "ata:0:cr589"),
		m_pccard1(*this, "pccard1"),
		m_pccard2(*this, "pccard2"),
		m_pccard_cd{ 1, 1 },
		m_h8_response(*this, "h8_response"),
		m_ram(*this, "maincpu:ram"),
		m_flashbank(*this, "flashbank"),
		m_in2(*this, "IN2"),
		m_out1(*this, "OUT1"),
		m_out2(*this, "OUT2"),
		m_upd4701(*this, "upd4701"),
		m_gunx(*this, "GUNX"),
		m_sensor(*this, "SENSOR"),
		m_encoder(*this, "ENCODER"),
		m_kicksensor1(*this, "KICK_SENSOR1"),
		m_kicksensor2(*this, "KICK_SENSOR2"),
		m_kicksensor3(*this, "KICK_SENSOR3"),
		m_ds2401_id(*this, "ds2401_id"),
		m_duart(*this, "mb89371")
	{ }

	void drmn9m(machine_config &config);
	void drmn10m(machine_config &config);
	void gtfrk10m(machine_config &config);
	void gtfrk11m(machine_config &config);
	void gtrfrk7m(machine_config &config);
	void hyperbbc(machine_config &config);
	[[maybe_unused]] void ddrsolo(machine_config &config);
	void ddrsbm(machine_config &config);
	void mamboagga(machine_config &config);
	void gunmania(machine_config &config);
	void hypbbc2p(machine_config &config);
	void gtrfrk2m(machine_config &config);
	void gtrfrk2ml(machine_config &config);
	void gtrfrk5m(machine_config &config);
	void ddrs2k(machine_config &config);
	void stepchmp(machine_config &config);
	void animechmp(machine_config &config);
	void salarymc(machine_config &config);
	void gbbchmp(machine_config &config);
	void konami573(machine_config &config, bool no_cdrom = false);
	void konami573n(machine_config &config);
	void drmn2m(machine_config &config);
	void gtrfrk3m(machine_config &config);
	void mamboagg(machine_config &config);
	void gtrfrks(machine_config &config);
	void gchgchmp(machine_config &config);
	void drmn4m(machine_config &config);
	void fbaitbc(machine_config &config);
	void ddr4ms(machine_config &config);
	void konami573x(machine_config &config);
	void dmx(machine_config &config);
	void drmn(machine_config &config);
	void kicknkick(machine_config &config);
	void k573d(machine_config &config);
	void k573k(machine_config &config);
	void k573a(machine_config &config);
	void k573ak(machine_config &config);
	void pccard1_16mb(machine_config &config);
	void pccard1_32mb(machine_config &config);
	void pccard2_32mb(machine_config &config);
	void pccard2_64mb(machine_config &config);
	void cassx(machine_config &config);
	void cassxi(machine_config &config);
	void cassy(machine_config &config);
	void cassyi(machine_config &config);
	void cassyyi(machine_config &config);
	void casszi(machine_config &config);
	void cassxzi(machine_config &config);

	void init_serlamp();
	void init_hyperbbc();
	void init_drmn();

	int gunmania_tank_shutter_sensor();
	int gunmania_cable_holder_sensor();

	int h8_d0_r();
	int h8_d1_r();
	int h8_d2_r();
	int h8_d3_r();

	template<int N> int pccard_cd_r();

	void gtrfrks_lamps_b7(int state);
	void gtrfrks_lamps_b6(int state);
	void gtrfrks_lamps_b5(int state);
	void gtrfrks_lamps_b4(int state);
	void dmx_lamps_b0(int state);
	void dmx_lamps_b1(int state);
	void dmx_lamps_b2(int state);
	void dmx_lamps_b3(int state);
	void dmx_lamps_b4(int state);
	void dmx_lamps_b5(int state);
	void mamboagg_lamps_b3(int state);
	void mamboagg_lamps_b4(int state);
	void mamboagg_lamps_b5(int state);
	void serial_lamp_reset(int state);
	void serial_lamp_data(int state);
	void stepchmp_lamp_clock(int state);
	void animechmp_lamp_clock(int state);
	void salarymc_lamp_clock(int state);
	void hyperbbc_lamp_red(int state);
	void hyperbbc_lamp_green(int state);
	void hyperbbc_lamp_blue(int state);
	void hyperbbc_lamp_start(int state);
	void hyperbbc_lamp_strobe1(int state);
	void hyperbbc_lamp_strobe2(int state);
	void hyperbbc_lamp_strobe3(int state);

	void h8_clk_w(int state);

	int jvs_rx_r();

protected:
	using gx700pwfbf_output_delegate = delegate<void (offs_t, uint8_t)>;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void driver_start() override;

	void gx700pwfbf_init(gx700pwfbf_output_delegate &&output_callback_func);

	void konami573a_map(address_map &map) ATTR_COLD;

	required_device<psxcpu_device> m_maincpu;
	required_device<sys573_jvs_host> m_sys573_jvs_host;
	optional_device<k573dio_device> m_k573dio;

	output_finder<2> m_lamps;

private:
	bool jvs_is_valid_packet();

	void jvs_input_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t jvs_input_r(offs_t offset, uint16_t mem_mask = ~0);

	uint16_t port_in2_jvs_r(offs_t offset, uint16_t mem_mask = ~0);

	uint16_t control_r(offs_t offset, uint16_t mem_mask = ~0);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void atapi_reset_w(uint16_t data);
	void security_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t security_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t ge765pwbba_r(offs_t offset, uint16_t mem_mask = ~0);
	void ge765pwbba_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gx700pwbf_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void gx700pwbf_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gx700pwbk_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void gx700pwbk_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gunmania_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gunmania_r(offs_t offset, uint16_t mem_mask = ~0);
	void ata_interrupt(int state);

	TIMER_CALLBACK_MEMBER( atapi_xfer_end );
	void ddrsolo_output_callback(offs_t offset, uint8_t data);
	void drmn_output_callback(offs_t offset, uint8_t data);
	void dmx_output_callback(offs_t offset, uint8_t data);
	void mamboagg_output_callback(offs_t offset, uint8_t data);
	double analogue_inputs_callback(uint8_t input);

	void cdrom_dma_read( uint32_t *ram, uint32_t n_address, int32_t n_size );
	void cdrom_dma_write( uint32_t *ram, uint32_t n_address, int32_t n_size );

	void stepchmp_cassette_install(device_t* device);
	void animechmp_cassette_install(device_t *device);
	void salarymc_cassette_install(device_t *device);
	void hyperbbc_cassette_install(device_t *device);
	void hyprbbc2_cassette_install(device_t *device);
	void hypbbc2p_cassette_install(device_t *device);
	static void cr589_config(device_t *device);
	void fbaitbc_map(address_map &map) ATTR_COLD;
	void flashbank_map(address_map &map) ATTR_COLD;
	void gunmania_map(address_map &map) ATTR_COLD;
	void gbbchmp_map(address_map &map) ATTR_COLD;
	void konami573_map(address_map &map) ATTR_COLD;
	void konami573ak_map(address_map &map) ATTR_COLD;
	void konami573d_map(address_map &map) ATTR_COLD;
	void konami573k_map(address_map &map) ATTR_COLD;

	required_ioport m_analog0;
	required_ioport m_analog1;
	required_ioport m_analog2;
	required_ioport m_analog3;

	void gx700pwbf_output( int offset, uint8_t data );

	required_device<psxirq_device> m_psxirq;

	required_device<ata_interface_device> m_ata;
	optional_device<atapi_hle_device> m_image;
	required_device<pccard_slot_device> m_pccard1;
	required_device<pccard_slot_device> m_pccard2;
	emu_timer *m_atapi_timer;
	int m_atapi_xferbase;
	int m_atapi_xfersize;

	int m_pccard_cd[2];

	uint32_t m_control;
	uint16_t m_n_security_control;

	required_region_ptr<uint8_t> m_h8_response;
	int m_h8_index;
	int m_h8_clk;

	uint8_t m_gx700pwbf_output_data[ 4 ];
	gx700pwfbf_output_delegate m_gx700pwfbf_output_callback;

	int m_serial_lamp_bits;
	int m_serial_lamp_shift;
	int m_serial_lamp_data;
	int m_serial_lamp_clock;

	int m_hyperbbc_lamp_red;
	int m_hyperbbc_lamp_green;
	int m_hyperbbc_lamp_blue;
	int m_hyperbbc_lamp_start;
	int m_hyperbbc_lamp_strobe1;
	int m_hyperbbc_lamp_strobe2;
	int m_hyperbbc_lamp_strobe3;

	uint32_t *m_p_n_psxram;

	int m_tank_shutter_position;
	int m_cable_holder_release;

	int m_jvs_input_idx_r, m_jvs_input_idx_w;
	int m_jvs_output_idx_w, m_jvs_output_len_w;
	uint8_t m_jvs_input_buffer[512];
	uint8_t m_jvs_output_buffer[512];

	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_flashbank;
	required_ioport m_in2;
	required_ioport m_out1;
	required_ioport m_out2;
	optional_device<upd4701_device> m_upd4701;
	optional_ioport m_gunx;
	optional_ioport m_sensor;
	optional_ioport m_encoder;
	optional_ioport m_kicksensor1;
	optional_ioport m_kicksensor2;
	optional_ioport m_kicksensor3;
	optional_device<ds2401_device> m_ds2401_id;
	optional_device<mb89371_device> m_duart;
};


class ddr_state : public ksys573_state
{
public:
	ddr_state(const machine_config &mconfig, device_type type, const char *tag) :
		ksys573_state(mconfig, type, tag),
		m_stage(*this, "STAGE")
	{
	}

	// DDR analog
	void ddr(machine_config &config);
	void ddrk(machine_config &config);
	void ddr2mc2(machine_config &config);
	void ddr2ml(machine_config &config);
	void ddrbocd(machine_config &config);

	// DDR digital
	void ddr3m(machine_config &config);
	void ddr3mp(machine_config &config);
	void ddrusa(machine_config &config);
	void ddr5m(machine_config &config);

	// Dancing Stage analog
	void dsfdcta(machine_config &config);
	void dsftkd(machine_config &config);

	// Dancing Stage digital
	void dsfdct(machine_config &config);
	void dsfdr(machine_config &config);
	void dsem(machine_config &config);
	void dsem2(machine_config &config);

	ioport_value gn845pwbb_read();

	void init_ddr();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	struct stage_state
	{
		int DO = 0;
		int clk = 0;
		int shift = 0;
		int state = 0;
		int bit = 0;
	};

	void ddr_output_callback(offs_t offset, uint8_t data);

	void gn845pwbb_do_w(int offset, int data);
	void gn845pwbb_clk_w(int offset, int data);

	required_ioport m_stage;

	uint32_t m_stage_mask;
	stage_state m_stage_state[2];
};


class pnchmn_state : public ksys573_state
{
public:
	pnchmn_state(const machine_config &mconfig, device_type type, const char *tag) :
		ksys573_state(mconfig, type, tag),
		m_pads(*this, "PADS")
	{
	}

	void pnchmn(machine_config &config);
	void pnchmn2(machine_config &config);

	void init_pnchmn();

	// FIXME: leaking because a konami573_cassette_xi_device member uses it
	double m_pad_position[6] = { };
	int m_pad_motor_direction[6] = { };
	attotime m_last_pad_update;
	required_ioport m_pads;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void punchmania_cassette_install(device_t *device);
	void punchmania_output_callback(offs_t offset, uint8_t data);
};


void ksys573_state::konami573_map(address_map &map)
{
	map(0x1f000000, 0x1f3fffff).m(m_flashbank, FUNC(address_map_bank_device::amap16));
	map(0x1f400000, 0x1f400003).portr("IN0").portw("OUT0");
	map(0x1f400004, 0x1f400007).portr("IN1");
	map(0x1f400008, 0x1f40000b).r(FUNC(ksys573_state::port_in2_jvs_r));
	map(0x1f40000c, 0x1f40000f).portr("IN3");
	map(0x1f480000, 0x1f48000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x1f4c0000, 0x1f4c000f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x1f500000, 0x1f500001).rw(FUNC(ksys573_state::control_r), FUNC(ksys573_state::control_w));    // Konami can't make a game without a "control" register.
	map(0x1f560000, 0x1f560001).w(FUNC(ksys573_state::atapi_reset_w));
	map(0x1f5c0000, 0x1f5c0003).nopw();                // watchdog?
	map(0x1f600000, 0x1f600003).portw("LAMPS");
	map(0x1f620000, 0x1f623fff).rw("m48t58", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask32(0x00ff00ff);
	map(0x1f680000, 0x1f680001).rw(FUNC(ksys573_state::jvs_input_r), FUNC(ksys573_state::jvs_input_w));
	map(0x1f6a0000, 0x1f6a0001).rw(FUNC(ksys573_state::security_r), FUNC(ksys573_state::security_w));
}

void ksys573_state::flashbank_map(address_map &map)
{
	map(0x0000000, 0x03fffff).rw("29f016a.31m", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x0000000, 0x03fffff).rw("29f016a.27m", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x0400000, 0x07fffff).rw("29f016a.31l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x0400000, 0x07fffff).rw("29f016a.27l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x0800000, 0x0bfffff).rw("29f016a.31j", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x0800000, 0x0bfffff).rw("29f016a.27j", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x0c00000, 0x0ffffff).rw("29f016a.31h", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x0c00000, 0x0ffffff).rw("29f016a.27h", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x4000000, 0x7ffffff).rw("pccard1", FUNC(pccard_slot_device::read_memory), FUNC(pccard_slot_device::write_memory));
	map(0x8000000, 0xbffffff).rw("pccard2", FUNC(pccard_slot_device::read_memory), FUNC(pccard_slot_device::write_memory));
}

void ksys573_state::konami573d_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).m(m_k573dio, FUNC(k573dio_device::amap));
}

void ksys573_state::konami573k_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).m("k573kara", FUNC(k573kara_device::amap));
}

void ksys573_state::konami573a_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).rw(FUNC(ksys573_state::gx700pwbf_io_r), FUNC(ksys573_state::gx700pwbf_io_w));
}

void ksys573_state::konami573ak_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).rw(FUNC(ksys573_state::gx700pwbk_io_r), FUNC(ksys573_state::gx700pwbk_io_w));
}

void ksys573_state::fbaitbc_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).rw(FUNC(ksys573_state::ge765pwbba_r), FUNC(ksys573_state::ge765pwbba_w));
}

void ksys573_state::gunmania_map(address_map &map)
{
	konami573_map(map);
	map(0x1f640000, 0x1f6400ff).rw(FUNC(ksys573_state::gunmania_r), FUNC(ksys573_state::gunmania_w));
}

void ksys573_state::gbbchmp_map(address_map &map)
{
	konami573_map(map);
	// The game waits until transmit is ready, but the chip may not actually be present.
	map(0x1f640000, 0x1f640007).rw(m_duart, FUNC(mb89371_device::read), FUNC(mb89371_device::write)).umask32(0x00ff00ff);
}

bool ksys573_state::jvs_is_valid_packet()
{
	if (m_jvs_input_idx_w < 5) {
		// A valid packet will have at the very least
		//  - sync (0xe0)
		//  - node number (non-zero)
		//  - size
		//  - at least 1 byte in the request message
		//  - checksum
		return false;
	}

	if (m_jvs_input_buffer[0] != 0xe0  || m_jvs_input_buffer[1] == 0x00) {
		return false;
	}

	int command_size = m_jvs_input_buffer[2] + 3;
	if (m_jvs_input_idx_w < command_size) {
		return false;
	}

	uint8_t checksum = 0;
	for (int i = 1; i < command_size - 1; i++) {
		checksum += m_jvs_input_buffer[i];
	}

	return checksum == m_jvs_input_buffer[command_size - 1];
}

void ksys573_state::jvs_input_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_jvs_input_buffer[m_jvs_input_idx_w++] = data & 0xff;
	m_jvs_input_buffer[m_jvs_input_idx_w++] = data >> 8;

	if (m_jvs_input_buffer[0] != 0xe0) {
		m_jvs_input_idx_w = 0;
	}

	if (jvs_is_valid_packet()) {
		LOGJVS("jvs_input_w( %08x, %08x, %02x %02x )\n", offset, mem_mask, data & 0xff, data >> 8 );
		for (int i = 0; i < m_jvs_input_idx_w; i++)
			LOGJVS("%02x ", m_jvs_input_buffer[i]);
		LOGJVS("\n");

		int command_size = m_jvs_input_buffer[2] + 3;
		m_sys573_jvs_host->send_packet(m_jvs_input_buffer + 1, command_size - 2); // jvshost doesn't actually check the checksum, so don't send it

		m_jvs_input_idx_w = 0;
		m_jvs_input_idx_r = 0;

		m_jvs_input_buffer[0] = 0;
	}
}

uint16_t ksys573_state::jvs_input_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_jvs_input_buffer[m_jvs_input_idx_r++];
	data |= m_jvs_input_buffer[m_jvs_input_idx_r++] << 8;

	return data;
}

uint16_t ksys573_state::port_in2_jvs_r(offs_t offset, uint16_t mem_mask)
{
	if (offset == 0) {
		// 0x1f400008-0x1f400009 are for inputs
		return m_in2->read();
	}

	if (m_jvs_output_len_w <= 0) {
		return 0;
	}

	uint16_t data = m_jvs_output_buffer[m_jvs_output_idx_w] | (m_jvs_output_buffer[m_jvs_output_idx_w+1] << 8);
	m_jvs_output_idx_w += 2;

	if (m_jvs_output_idx_w >= m_jvs_output_len_w) {
		m_jvs_output_idx_w = 0;
		m_jvs_output_len_w = 0;
	}

	LOGJVS("m_jvs_output_r %08x %08x | %02x %02x | %02x\n", offset, mem_mask, data & 0xff, data >> 8, m_jvs_output_idx_w);

	return data;
}

int ksys573_state::jvs_rx_r()
{
	if (m_jvs_output_len_w <= 0) {
		m_jvs_output_len_w = m_sys573_jvs_host->received_packet(m_jvs_output_buffer);
	}

	return m_jvs_output_len_w > 0;
}

uint16_t ksys573_state::control_r(offs_t offset, uint16_t mem_mask)
{
	LOGCONTROL( "control_r( %08x, %08x ) %08x\n", offset, mem_mask, m_control );

	return m_control;
}

void ksys573_state::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_control );

	LOGCONTROL( "control_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	m_out2->write( data, mem_mask );

	m_flashbank->set_bank( m_control & 0x3f );
}

TIMER_CALLBACK_MEMBER( ksys573_state::atapi_xfer_end )
{
	/// TODO: respect timing of data from ATAPI device.

	m_atapi_timer->adjust( attotime::never );

	for( int i = 0; i < m_atapi_xfersize; i++ )
	{
		uint32_t d = m_ata->cs0_r(0) << 0;
		d |= m_ata->cs0_r(0) << 16;

		m_p_n_psxram[ m_atapi_xferbase / 4 ] = d;
		m_atapi_xferbase += 4;
	}

	/// HACK: konami80s only works if you dma more data than requested
	if( ( m_ata->cs1_r(6) & 8 ) != 0 )
	{
		m_atapi_timer->adjust( m_maincpu->cycles_to_attotime( ( ATAPI_CYCLES_PER_SECTOR * ( m_atapi_xfersize / 64 ) ) ) );
	}
}

void ksys573_state::ata_interrupt(int state)
{
	m_psxirq->intin10( state );
}

void ksys573_state::atapi_reset_w(uint16_t data)
{
	if( !( data & 1 ) )
	{
		m_ata->reset();
	}
}

void ksys573_state::cdrom_dma_read( uint32_t *ram, uint32_t n_address, int32_t n_size )
{
	LOGCDROM( "cdrom_dma_read( %08x, %08x )\n", n_address, n_size );
//  osd_printf_debug( "DMA read: address %08x size %08x\n", n_address, n_size );
}

void ksys573_state::cdrom_dma_write( uint32_t *ram, uint32_t n_address, int32_t n_size )
{
	m_p_n_psxram = ram;

	LOGCDROM( "cdrom_dma_write( %08x, %08x )\n", n_address, n_size );
//  osd_printf_debug( "DMA write: address %08x size %08x\n", n_address, n_size );

	m_atapi_xferbase = n_address;
	m_atapi_xfersize = n_size;
	// set a transfer complete timer ( Note: CYCLES_PER_SECTOR can't be lower than 2000 or the BIOS ends up "out of order" )
	m_atapi_timer->adjust( m_maincpu->cycles_to_attotime( ( ATAPI_CYCLES_PER_SECTOR * ( n_size / 512 ) ) ) );
}

void ksys573_state::security_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_n_security_control );

	LOGSECURITY( "security_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

	m_out1->write( data, mem_mask );
}

uint16_t ksys573_state::security_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_n_security_control;
	LOGSECURITY( "security_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

template<int N>
int ksys573_state::pccard_cd_r()
{
	return m_pccard_cd[N];
}

void ksys573_state::driver_start()
{
	m_atapi_timer = timer_alloc( FUNC( ksys573_state::atapi_xfer_end ), this );
	m_atapi_timer->adjust( attotime::never );

	save_item( NAME( m_n_security_control ) );
	save_item( NAME( m_control ) );
	save_item( NAME( m_pccard_cd ) );

	m_h8_index = 0;
}

void ksys573_state::machine_start()
{
	m_lamps.resolve();
}

void ksys573_state::machine_reset()
{
	m_n_security_control = 0;
	m_control = 0;

	m_h8_index = 0;
	m_h8_clk = 0;

	m_jvs_input_idx_r = m_jvs_input_idx_w = 0;
	m_jvs_output_idx_w = m_jvs_output_len_w = 0;

	std::fill_n( m_jvs_input_buffer, sizeof( m_jvs_input_buffer ), 0 );
	std::fill_n( m_jvs_output_buffer, sizeof( m_jvs_output_buffer ), 0 );
}

// H8 check at startup (JVS related)

void ksys573_state::h8_clk_w(int state)
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

int ksys573_state::h8_d0_r()
{
	return ( m_h8_response[ m_h8_index ] >> 0 ) & 1;
}

int ksys573_state::h8_d1_r()
{
	return ( m_h8_response[ m_h8_index ] >> 1 ) & 1;
}

int ksys573_state::h8_d2_r()
{
	return ( m_h8_response[ m_h8_index ] >> 2 ) & 1;
}

int ksys573_state::h8_d3_r()
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

uint16_t ksys573_state::ge765pwbba_r(offs_t offset, uint16_t mem_mask)
{
	switch( offset )
	{
	case 0x4c:
	case 0x4d:
		return m_upd4701->read_y(offset & 1);

	default:
		LOGIOBOARD( "ge765pwbba_r: unhandled offset %08x %08x\n", offset, mem_mask );
		break;
	}

	LOGIOBOARD( "ge765pwbba_r( %08x, %08x )\n", offset, mem_mask );
	return 0;
}

void ksys573_state::ge765pwbba_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch( offset )
	{
	case 0x08:
		break;

	case 0x40:
		output().set_value( "motor", data & 0xff );
		break;

	case 0x44:
		output().set_value( "brake", data & 0xff );
		break;

	case 0x50:
		m_upd4701->resety_w( 1 );
		m_upd4701->resety_w( 0 );
		break;

	default:
		LOGIOBOARD( "ge765pwbba_w: unhandled offset %08x %08x %08x\n", offset, mem_mask, data );
		break;
	}

	LOGIOBOARD( "ge765pwbba_w( %08x, %08x, %08x )\n", offset, mem_mask, data );
}

/*

GX700-PWB(F)

Analogue I/O board

*/

uint16_t ksys573_state::gx700pwbf_io_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t data = 0;
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

	LOGIOBOARD( "gx700pwbf_io_r( %08x, %08x ) %08x\n", offset, mem_mask, data );

	return data;
}

void ksys573_state::gx700pwbf_output( int offset, uint8_t data )
{
	if( !m_gx700pwfbf_output_callback.isnull() )
	{
		int i;
		static const int shift[] = { 7, 6, 1, 0, 5, 4, 3, 2 };
		for( i = 0; i < 8; i++ )
		{
			int oldbit = ( m_gx700pwbf_output_data[ offset ] >> shift[ i ] ) & 1;
			int newbit = ( data >> shift[ i ] ) & 1;
			if( oldbit != newbit )
			{
				m_gx700pwfbf_output_callback( ( offset * 8 ) + i, newbit );
			}
		}
	}
	m_gx700pwbf_output_data[ offset ] = data;
}

void ksys573_state::gx700pwbf_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGIOBOARD( "gx700pwbf_io_w( %08x, %08x, %08x )\n", offset, mem_mask, data );

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

void ksys573_state::gx700pwfbf_init(gx700pwfbf_output_delegate &&output_callback_func)
{
	std::fill_n(m_gx700pwbf_output_data, std::size(m_gx700pwbf_output_data), 0);

	m_gx700pwfbf_output_callback = std::move(output_callback_func);

	save_item(NAME(m_gx700pwbf_output_data ));
}

/*

GX700-PWB(K)

Analogue I/O board

*/

uint16_t ksys573_state::gx700pwbk_io_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t data = 0;

	switch (offset)
	{
	case 0x40:
		data = m_kicksensor1->read();
		break;

	case 0x48:
		data = m_kicksensor2->read();
		break;

	case 0x50:
		data = m_kicksensor3->read();
		break;
	}

	return data;
}

void ksys573_state::gx700pwbk_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x60:
		m_ds2401_id->write(!BIT(data, 15));
		break;
	}
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

void ddr_state::gn845pwbb_do_w( int offset, int data )
{
	m_stage_state[ offset ].DO = !data;
}

void ddr_state::gn845pwbb_clk_w( int offset, int data )
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

	LOGIOBOARD( "stage: %dp data clk=%d state=%d d0=%d shift=%08x bit=%d stage_mask=%08x\n", offset + 1, clk,
		m_stage_state[ offset ].state, m_stage_state[ offset ].DO, m_stage_state[ offset ].shift, m_stage_state[ offset ].bit, m_stage_mask );
}

ioport_value ddr_state::gn845pwbb_read()
{
	return m_stage->read() & m_stage_mask;
}

void ddr_state::ddr_output_callback(offs_t offset, uint8_t data)
{
	switch( offset )
	{
	case 0:
		output().set_value( "foot 1p up", !data );
		break;

	case 1:
		output().set_value( "foot 1p left", !data );
		break;

	case 2:
		output().set_value( "foot 1p right", !data );
		break;

	case 3:
		output().set_value( "foot 1p down", !data );
		break;

	case 4:
		gn845pwbb_do_w( 0, !data );
		break;

	case 7:
		gn845pwbb_clk_w( 0, !data );
		break;

	case 8:
		output().set_value( "foot 2p up", !data );
		break;

	case 9:
		output().set_value( "foot 2p left", !data );
		break;

	case 10:
		output().set_value( "foot 2p right", !data );
		break;

	case 11:
		output().set_value( "foot 2p down", !data );
		break;

	case 12:
		gn845pwbb_do_w( 1, !data );
		break;

	case 15:
		gn845pwbb_clk_w( 1, !data );
		break;

	case 17:
		m_lamps[0] = data ? 0 : 1; // start 1
		break;

	case 18:
		m_lamps[1] = data ? 0 : 1; // start 2
		break;

	case 20:
		output().set_value( "body right low", !data );
		break;

	case 21:
		output().set_value( "body left low", !data );
		break;

	case 22:
		output().set_value( "body left high", !data );
		break;

	case 23:
		output().set_value( "body right high", !data );
		break;

	case 28: // digital
	case 30: // analogue
		output().set_value( "speaker", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

void ddr_state::machine_start()
{
	ksys573_state::machine_start();

	m_stage_mask = 0xffffffff;

	save_item(NAME(m_stage_mask));
	save_item(STRUCT_MEMBER(m_stage_state, DO));
	save_item(STRUCT_MEMBER(m_stage_state, clk));
	save_item(STRUCT_MEMBER(m_stage_state, shift));
	save_item(STRUCT_MEMBER(m_stage_state, state));
	save_item(STRUCT_MEMBER(m_stage_state, bit));
}

void ddr_state::init_ddr()
{
	gx700pwfbf_init(gx700pwfbf_output_delegate(&ddr_state::ddr_output_callback, this));
}

/* Guitar freaks */

void ksys573_state::gtrfrks_lamps_b7(int state)
{
	output().set_value( "spot left", state );
}

void ksys573_state::gtrfrks_lamps_b6(int state)
{
	output().set_value( "spot right", state );
}

void ksys573_state::gtrfrks_lamps_b5(int state)
{
	m_lamps[0] = state ? 1 : 0; // start left
}

void ksys573_state::gtrfrks_lamps_b4(int state)
{
	m_lamps[1] = state ? 1 : 0; // start right
}

/* ddr solo */

void ksys573_state::ddrsolo_output_callback(offs_t offset, uint8_t data)
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
		output().set_value( "extra 4", !data );
		break;

	case 9:
		output().set_value( "extra 2", !data );
		break;

	case 10:
		output().set_value( "extra 1", !data );
		break;

	case 11:
		output().set_value( "extra 3", !data );
		break;

	case 16:
		output().set_value( "speaker", !data );
		break;

	case 20:
		m_lamps[0] = data ? 0 : 1; // start
		break;

	case 21:
		output().set_value( "body center", !data );
		break;

	case 22:
		output().set_value( "body right", !data );
		break;

	case 23:
		output().set_value( "body left", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

/* drummania */

void ksys573_state::drmn_output_callback(offs_t offset, uint8_t data)
{
	switch( offset )
	{
	case 0: // drmn2+
	case 16: // drmn
		output().set_value( "hi-hat", !data );
		break;

	case 1: // drmn2+
	case 17: // drmn
		output().set_value( "high tom", !data );
		break;

	case 2: // drmn2+
	case 18: // drmn
		output().set_value( "low tom", !data );
		break;

	case 3: // drmn2+
	case 19: // drmn
		output().set_value( "snare", !data );
		break;

	case 8: // drmn2+
	case 30: // drmn
		output().set_value( "spot left & right", !data );
		break;

	case 9: // drmn2+
	case 31: // drmn
		output().set_value( "neon top", data );
		break;

	case 11: // drmn2+
	case 27: // drmn
		output().set_value( "neon woofer", data );
		break;

	case 12: // drmn2+
	case 20: // drmn
		output().set_value( "cymbal", !data );
		break;

	case 13: // drmn2+
	case 21: // drmn
		m_lamps[0] = data ? 1 : 0; // start
		break;

	case 14: // drmn2+
	case 22: // drmn
		output().set_value( "select button", data );
		break;

	case 23: // drmn
	case 26: // drmn
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

void ksys573_state::init_drmn()
{
	gx700pwfbf_init(gx700pwfbf_output_delegate(&ksys573_state::drmn_output_callback, this));
}

/* dance maniax */

void ksys573_state::dmx_output_callback(offs_t offset, uint8_t data)
{
	switch( offset )
	{
	case 0:
		output().set_value( "blue io 8", !data );
		break;

	case 1:
		output().set_value( "blue io 9", !data );
		break;

	case 2:
		output().set_value( "red io 9", !data );
		break;

	case 3:
		output().set_value( "red io 8", !data );
		break;

	case 4:
		output().set_value( "blue io 6", !data );
		break;

	case 5:
		output().set_value( "blue io 7", !data );
		break;

	case 6:
		output().set_value( "red io 7", !data );
		break;

	case 7:
		output().set_value( "red io 6", !data );
		break;

	case 8:
		output().set_value( "blue io 4", !data );
		break;

	case 9:
		output().set_value( "blue io 5", !data );
		break;

	case 10:
		output().set_value( "red io 5", !data );
		break;

	case 11:
		output().set_value( "red io 4", !data );
		break;

	case 12:
		output().set_value( "blue io 10", !data );
		break;

	case 13:
		output().set_value( "blue io 11", !data );
		break;

	case 14:
		output().set_value( "red io 11", !data );
		break;

	case 15:
		output().set_value( "red io 10", !data );
		break;

	case 16:
		output().set_value( "blue io 0", !data );
		break;

	case 17:
		output().set_value( "blue io 1", !data );
		break;

	case 18:
		output().set_value( "red io 1", !data );
		break;

	case 19:
		output().set_value( "red io 0", !data );
		break;

	case 20:
		output().set_value( "blue io 2", !data );
		break;

	case 21:
		output().set_value( "blue io 3", !data );
		break;

	case 22:
		output().set_value( "red io 3", !data );
		break;

	case 23:
		output().set_value( "red io 2", !data );
		break;

	case 28:
		output().set_value( "yellow spot light", !data );
		break;

	case 29:
		output().set_value( "blue spot light", !data );
		break;

	case 31:
		output().set_value( "pink spot light", !data );
		break;

	default:
//      printf( "%d=%d\n", offset, data );
		break;
	}
}

void ksys573_state::dmx_lamps_b0(int state)
{
	output().set_value( "left 2p", state );
}

void ksys573_state::dmx_lamps_b1(int state)
{
	m_lamps[1] = state ? 1 : 0; // start 1p
}

void ksys573_state::dmx_lamps_b2(int state)
{
	output().set_value( "right 2p", state );
}

void ksys573_state::dmx_lamps_b3(int state)
{
	output().set_value( "left 1p", state );
}

void ksys573_state::dmx_lamps_b4(int state)
{
	m_lamps[0] = state ? 1 : 0; // start 2p
}

void ksys573_state::dmx_lamps_b5(int state)
{
	output().set_value( "right 1p", state );
}

/* step champ */
void ksys573_state::stepchmp_lamp_clock(int state)
{
	if( state && !m_serial_lamp_clock )
	{
		m_serial_lamp_bits++;

		m_serial_lamp_shift <<= 1;
		m_serial_lamp_shift |= m_serial_lamp_data;

		if( m_serial_lamp_bits == 8 )
		{
			output().set_value( "halogen 1", ( m_serial_lamp_shift >> 3 ) & 1 );
			output().set_value( "halogen 2", ( m_serial_lamp_shift >> 2 ) & 1 );
			output().set_value( "halogen 3", ( m_serial_lamp_shift >> 1 ) & 1 );

			output().set_value( "player 1 start", ( m_serial_lamp_shift >> 7 ) & 1 );
			output().set_value( "player 2 start", ( m_serial_lamp_shift >> 6 ) & 1 );
			output().set_value( "player 3 start", ( m_serial_lamp_shift >> 5 ) & 1 );

			m_serial_lamp_bits = 0;
			m_serial_lamp_shift = 0;
		}
	}

	m_serial_lamp_clock = state;
}

void ksys573_state::stepchmp_cassette_install(device_t* device)
{
	konami573_cassette_y_device &cassette = downcast<konami573_cassette_y_device&>(*device);

	cassette.d5_handler().set(*this, FUNC(ksys573_state::stepchmp_lamp_clock));
	cassette.d6_handler().set(*this, FUNC(ksys573_state::serial_lamp_reset));
	cassette.d7_handler().set(*this, FUNC(ksys573_state::serial_lamp_data));
}

/* anime champ */
void ksys573_state::animechmp_lamp_clock(int state)
{
	if( state && !m_serial_lamp_clock )
	{
		m_serial_lamp_bits++;

		m_serial_lamp_shift <<= 1;
		m_serial_lamp_shift |= m_serial_lamp_data;

		if( m_serial_lamp_bits == 16 )
		{
			if( ( m_serial_lamp_shift & ~0xfff ) != 0 )
			{
				LOG( "unknown bits in serial_lamp_shift %08x\n", m_serial_lamp_shift & ~0xfff );
			}

			output().set_value( "player 1 red", ( m_serial_lamp_shift >> 11 ) & 1 );
			output().set_value( "player 1 green", ( m_serial_lamp_shift >> 10 ) & 1 );
			output().set_value( "player 1 blue", ( m_serial_lamp_shift >> 9 ) & 1 );

			output().set_value( "player 2 red", ( m_serial_lamp_shift >> 8 ) & 1 );
			output().set_value( "player 2 green", ( m_serial_lamp_shift >> 7 ) & 1 );
			output().set_value( "player 2 blue", ( m_serial_lamp_shift >> 6 ) & 1 );

			output().set_value( "player 3 red", ( m_serial_lamp_shift >> 5 ) & 1 );
			output().set_value( "player 3 green", ( m_serial_lamp_shift >> 4 ) & 1 );
			output().set_value( "player 3 blue", ( m_serial_lamp_shift >> 3 ) & 1 );

			output().set_value( "player 1 start", ( m_serial_lamp_shift >> 2 ) & 1 );
			output().set_value( "player 2 start", ( m_serial_lamp_shift >> 1 ) & 1 );
			output().set_value( "player 3 start", ( m_serial_lamp_shift >> 0 ) & 1 );

			m_serial_lamp_bits = 0;
			m_serial_lamp_shift = 0;
		}
	}

	m_serial_lamp_clock = state;
}

void ksys573_state::animechmp_cassette_install(device_t *device)
{
	konami573_cassette_y_device &cassette = downcast<konami573_cassette_y_device &>(*device);

	cassette.d5_handler().set(*this, FUNC(ksys573_state::animechmp_lamp_clock));
	cassette.d6_handler().set(*this, FUNC(ksys573_state::serial_lamp_reset));
	cassette.d7_handler().set(*this, FUNC(ksys573_state::serial_lamp_data));
}

/* salary man champ */
void ksys573_state::serial_lamp_reset(int state)
{
	if( state )
	{
		m_serial_lamp_bits = 0;
		m_serial_lamp_shift = 0;
	}
}

void ksys573_state::serial_lamp_data(int state)
{
	m_serial_lamp_data = state;
}

void ksys573_state::salarymc_lamp_clock(int state)
{
	if( state && !m_serial_lamp_clock )
	{
		m_serial_lamp_bits++;

		m_serial_lamp_shift <<= 1;
		m_serial_lamp_shift |= m_serial_lamp_data;

		if( m_serial_lamp_bits == 16 )
		{
			if( ( m_serial_lamp_shift & ~0xe38 ) != 0 )
			{
				LOG( "unknown bits in serial_lamp_shift %08x\n", m_serial_lamp_shift & ~0xe38 );
			}

			output().set_value( "player 1 red", ( m_serial_lamp_shift >> 11 ) & 1 );
			output().set_value( "player 1 green", ( m_serial_lamp_shift >> 10 ) & 1 );
			output().set_value( "player 1 blue", ( m_serial_lamp_shift >> 9 ) & 1 );

			output().set_value( "player 2 red", ( m_serial_lamp_shift >> 5 ) & 1 );
			output().set_value( "player 2 green", ( m_serial_lamp_shift >> 4 ) & 1 );
			output().set_value( "player 2 blue", ( m_serial_lamp_shift >> 3 ) & 1 );

			m_serial_lamp_bits = 0;
			m_serial_lamp_shift = 0;
		}
	}

	m_serial_lamp_clock = state;
}

void ksys573_state::salarymc_cassette_install(device_t *device)
{
	konami573_cassette_y_device &cassette = downcast<konami573_cassette_y_device &>(*device);

	cassette.d5_handler().set(*this, FUNC(ksys573_state::salarymc_lamp_clock));
	cassette.d6_handler().set(*this, FUNC(ksys573_state::serial_lamp_reset));
	cassette.d7_handler().set(*this, FUNC(ksys573_state::serial_lamp_data));
}

void ksys573_state::init_serlamp()
{
	m_serial_lamp_bits = 0;
	m_serial_lamp_shift = 0;
	m_serial_lamp_data = 0;
	m_serial_lamp_clock = 0;

	save_item( NAME( m_serial_lamp_bits ) );
	save_item( NAME( m_serial_lamp_shift ) );
	save_item( NAME( m_serial_lamp_data ) );
	save_item( NAME( m_serial_lamp_clock ) );
}

/* Hyper Bishi Bashi Champ */

void ksys573_state::hyperbbc_lamp_red(int state)
{
	m_hyperbbc_lamp_red = state;
}

void ksys573_state::hyperbbc_lamp_green(int state)
{
	m_hyperbbc_lamp_green = state;
}

void ksys573_state::hyperbbc_lamp_blue(int state)
{
	m_hyperbbc_lamp_blue = state;
}

void ksys573_state::hyperbbc_lamp_start(int state)
{
	m_hyperbbc_lamp_start = state;
}

void ksys573_state::hyperbbc_lamp_strobe1(int state)
{
	if( state && !m_hyperbbc_lamp_strobe1 )
	{
		output().set_value( "player 1 red", m_hyperbbc_lamp_red );
		output().set_value( "player 1 green", m_hyperbbc_lamp_green );
		output().set_value( "player 1 blue", m_hyperbbc_lamp_blue );
		output().set_value( "player 1 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe1 = state;
}

void ksys573_state::hyperbbc_lamp_strobe2(int state)
{
	if( state && !m_hyperbbc_lamp_strobe2 )
	{
		output().set_value( "player 2 red", m_hyperbbc_lamp_red );
		output().set_value( "player 2 green", m_hyperbbc_lamp_green );
		output().set_value( "player 2 blue", m_hyperbbc_lamp_blue );
		output().set_value( "player 2 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe2 = state;
}

void ksys573_state::hyperbbc_lamp_strobe3(int state)
{
	if( state && !m_hyperbbc_lamp_strobe3 )
	{
		output().set_value( "player 3 red", m_hyperbbc_lamp_red );
		output().set_value( "player 3 green", m_hyperbbc_lamp_green );
		output().set_value( "player 3 blue", m_hyperbbc_lamp_blue );
		output().set_value( "player 3 start", m_hyperbbc_lamp_start );
	}

	m_hyperbbc_lamp_strobe3 = state;
}

void ksys573_state::hyperbbc_cassette_install(device_t *device)
{
	konami573_cassette_y_device &cassette = downcast<konami573_cassette_y_device &>(*device);

	cassette.d0_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_strobe3)); // line shared with x76f100 sda
	cassette.d1_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_strobe2)); // line shared with x76f100 scl
	cassette.d3_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_strobe1)); // line shared with x76f100 rst
	cassette.d4_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_green));
	cassette.d5_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_blue));
	cassette.d6_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_red));
	cassette.d7_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_start));
}

void ksys573_state::hypbbc2p_cassette_install(device_t *device)
{
	konami573_cassette_y_device &cassette = downcast<konami573_cassette_y_device &>(*device);

	cassette.d0_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_strobe2)); // line shared with x76f100 sda
	cassette.d3_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_strobe1)); // line shared with x76f100 rst
	cassette.d4_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_green));
	cassette.d5_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_blue));
	cassette.d6_handler().set(*this, FUNC(ksys573_state::hyperbbc_lamp_red));
}

void ksys573_state::init_hyperbbc()
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

void ksys573_state::mamboagg_output_callback(offs_t offset, uint8_t data)
{
	switch( offset )
	{
	case 4:
		output().set_value( "fire lamp left", !data );
		break;
	case 5:
		output().set_value( "fire fan left", !data );
		break;
	case 6:
		output().set_value( "fire fan right", !data );
		break;
	case 7:
		output().set_value( "fire lamp right", !data );
		break;
	case 28:
		output().set_value( "conga left", !data );
		break;
	case 29:
		output().set_value( "conga right", !data );
		break;
	case 31:
		output().set_value( "conga centre", !data );
		break;
	}
}

void ksys573_state::mamboagg_lamps_b3(int state)
{
	m_lamps[0] = state ? 1 : 0; // start 1p
}

void ksys573_state::mamboagg_lamps_b4(int state)
{
	output().set_value( "select right", state );
}

void ksys573_state::mamboagg_lamps_b5(int state)
{
	output().set_value( "select left", state );
}


/* punch mania */


void pnchmn_state::punchmania_cassette_install(device_t *device)
{
	auto game = downcast<konami573_cassette_xi_device *>(device);
	auto adc0838 = device->subdevice<adc083x_device>("adc0838");
	adc0838->set_input_callback(*game, FUNC(konami573_cassette_xi_device::punchmania_inputs_callback));
}

void pnchmn_state::punchmania_output_callback(offs_t offset, uint8_t data)
{
	switch( offset )
	{
	case 8:
		output().set_value( "select left right", !data );
		break;
	case 9:
		output().set_value( "left bottom lamp", !data );
		break;
	case 10:
		output().set_value( "left middle lamp", !data );
		break;
	case 11:
		output().set_value( "start lamp", !data );
		break;
	case 12:
		output().set_value( "left top lamp", !data );
		break;
	case 13:
		output().set_value( "right middle lamp", !data );
		break;
	case 14:
		output().set_value( "right top lamp", !data );
		break;
	case 15:
		output().set_value( "right bottom lamp", !data );
		break;
	case 16:
		m_pad_motor_direction[ 0 ] = data ? 1 : 0; // left top motor +
		break;
	case 17:
		m_pad_motor_direction[ 1 ] = data ? 1 : 0; // left middle motor +
		break;
	case 18:
		m_pad_motor_direction[ 1 ] = data ? -1 : 0; // left middle motor -
		break;
	case 19:
		m_pad_motor_direction[ 0 ] = data ? -1 : 0; // left top motor -
		break;
	case 20:
		m_pad_motor_direction[ 2 ] = data ? 1 : 0; // left bottom motor +
		break;
	case 21:
		m_pad_motor_direction[ 3 ] = data ? -1 : 0; // right top motor -
		break;
	case 22:
		m_pad_motor_direction[ 3 ] = data ? 1 : 0; // right top motor +
		break;
	case 23:
		m_pad_motor_direction[ 2 ] = data ? -1 : 0; // left bottom motor -
		break;
	case 26:
		m_pad_motor_direction[ 5 ] = data ? 1 : 0; // right bottom motor +
		break;
	case 27:
		m_pad_motor_direction[ 4 ] = data ? 1 : 0; // right middle motor +
		break;
	case 30:
		m_pad_motor_direction[ 4 ] = data ? -1 : 0; // right middle motor -
		break;
	case 31:
		m_pad_motor_direction[ 5 ] = data ? -1 : 0; // right bottom motor -
		break;
	}
}

void pnchmn_state::machine_start()
{
	ksys573_state::machine_start();

	std::fill(std::begin(m_pad_position), std::end(m_pad_position), 0.0);
	std::fill(std::begin(m_pad_motor_direction), std::end(m_pad_motor_direction), 0);

	save_item(NAME(m_pad_position));
	save_item(NAME(m_pad_motor_direction));
}

void pnchmn_state::machine_reset()
{
	ksys573_state::machine_reset();

	std::fill(std::begin(m_pad_motor_direction), std::end(m_pad_motor_direction), 0);
	m_last_pad_update = machine().time();
}

void pnchmn_state::init_pnchmn()
{
	gx700pwfbf_init(gx700pwfbf_output_delegate(&pnchmn_state::punchmania_output_callback, this));
}

/* GunMania */

void ksys573_state::gunmania_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	std::string message;

	switch( offset )
	{
	case 0x4c:
		m_ds2401_id->write( ( data >> 5 ) & 1 );
		break;

	case 0x54:
		switch( data & 0xa0 )
		{
		case 0x20:
			message += "cable holder motor release ";
			m_cable_holder_release = 1;
			break;

		case 0x80:
			message += "cable holder motor catch ";

			m_cable_holder_release = 0;
			break;

		case 0xa0:
			message += "cable holder motor stop ";
			break;
		}

		switch( data & 0x50 )
		{
		case 0x10:
			message += "bullet supply motor rotate ";
			break;

		case 0x40:
			message += "bullet supply motor reverse ";
			break;

		case 0x50:
			message += "bullet shutter motor unknown ";
			break;
		}

		switch( data & 0x0a )
		{
		case 0x02:
			message += "tank shutter motor close ";

			if( m_tank_shutter_position > 0 )
			{
				m_tank_shutter_position--;
			}

			break;

		case 0x08:
			message += "tank shutter motor open ";

			if( m_tank_shutter_position < 100 )
			{
				m_tank_shutter_position++;
			}

			break;

		case 0x0a:
			message += "tank shutter motor unknown ";
			break;
		}

		if( ( data & ~0xfa ) != 0 )
		{
			message += util::string_format("unknown bits %08x", data & ~0xfa);
		}

		if( !message.empty() )
		{
			LOG( message );
		}

		break;
	}

	LOGIOBOARD( "gunmania_w %08x %08x %08x\n", offset, mem_mask, data );
}

int ksys573_state::gunmania_tank_shutter_sensor()
{
	if( m_tank_shutter_position == 0 )
	{
		return 1;
	}

	return 0;
}

int ksys573_state::gunmania_cable_holder_sensor()
{
	return m_cable_holder_release;
}

uint16_t ksys573_state::gunmania_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t data = 0;

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

	LOGIOBOARD( "gunmania_r %08x %08x %08x\n", offset, mem_mask, data );
	return data;
}

/* ADC0834 Interface */

double ksys573_state::analogue_inputs_callback(uint8_t input)
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

void ksys573_state::cr589_config(device_t *device)
{
	auto cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^speaker", 1.0, 0);
	cdda->add_route(1, "^^speaker", 1.0, 1);

	auto cdrom = device->subdevice<cdrom_image_device>("image");
	cdrom->add_region("install");
	cdrom->add_region("install2");
	cdrom->add_region("runtime", true);
}

void ksys573_state::konami573(machine_config &config, bool no_cdrom)
{
	/* basic machine hardware */
	CXD8530CQ(config, m_maincpu, XTAL(67'737'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::konami573_map);
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&ksys573_state::cdrom_dma_read, this));
	m_maincpu->subdevice<psxdma_device>("dma")->install_write_handler(5, psxdma_device::write_delegate(&ksys573_state::cdrom_dma_write, this));

	subdevice<ram_device>("maincpu:ram")->set_default_size("4M");

	ATA_INTERFACE(config, m_ata, 0);
	m_ata->irq_handler().set(FUNC(ksys573_state::ata_interrupt));
	if(!no_cdrom)
	{
		m_ata->slot(0).option_add("cr589", CR589);
		m_ata->slot(0).set_option_machine_config("cr589", cr589_config);
		m_ata->slot(0).set_default_option("cr589");
	}

	konami573_cassette_slot_device &cassette(KONAMI573_CASSETTE_SLOT(config, "cassette", 0));
	cassette.dsr_handler().set("maincpu:sio1", FUNC(psxsio1_device::write_dsr));

	// onboard flash
	FUJITSU_29F016A(config, "29f016a.31m");
	FUJITSU_29F016A(config, "29f016a.27m");
	FUJITSU_29F016A(config, "29f016a.31l");
	FUJITSU_29F016A(config, "29f016a.27l");
	FUJITSU_29F016A(config, "29f016a.31j");
	FUJITSU_29F016A(config, "29f016a.27j");
	FUJITSU_29F016A(config, "29f016a.31h");
	FUJITSU_29F016A(config, "29f016a.27h");

	PCCARD_SLOT(config, m_pccard1, 0);
	m_pccard1->cd1().set([this](int state) { m_pccard_cd[0] = state; });

	PCCARD_SLOT(config, m_pccard2, 0);
	m_pccard2->cd1().set([this](int state) { m_pccard_cd[1] = state; });

	ADDRESS_MAP_BANK(config, m_flashbank ).set_map( &ksys573_state::flashbank_map ).set_options( ENDIANNESS_LITTLE, 16, 32, 0x400000);

	/* video hardware */
	CXD8561Q(config, "gpu", XTAL(53'693'175), 0x200000, m_maincpu.target()).set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	spu_device &spu(SPU(config, "spu", XTAL(67'737'600)/2, m_maincpu.target()));
	spu.add_route(0, "speaker", 1.0, 0);
	spu.add_route(1, "speaker", 1.0, 1);

	M48T58(config, "m48t58", 0);

	adc0834_device &adc(ADC0834(config, "adc0834"));
	adc.set_input_callback(FUNC(ksys573_state::analogue_inputs_callback));

	SYS573_JVS_HOST(config, m_sys573_jvs_host, 0);

	// Uncomment for generating new security cartridges
	// Warning: Does not play well with memory card reader (JVS chaining issue?)
	//KONAMI_573_MASTER_CALENDAR(config, "k573mcal", 0, m_sys573_jvs_host);
}

// Variants with additional digital sound board
void ksys573_state::k573d(machine_config &config)
{
	konami573(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::konami573d_map);
	KONAMI_573_DIGITAL_IO_BOARD(config, m_k573dio, XTAL(19'660'800));
}

// Variants with additional karaoke I/O board
void ksys573_state::k573k(machine_config &config)
{
	konami573(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::konami573k_map);
	KONAMI_573_KARAOKE_IO_BOARD(config, "k573kara", XTAL(36'864'000));
}

// Variants with additional analogue i/o board
void ksys573_state::k573a(machine_config &config)
{
	konami573(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::konami573a_map);
}

void ksys573_state::k573ak(machine_config &config)
{
	konami573(config, true);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::konami573ak_map);
}

void ksys573_state::pccard1_16mb(machine_config &config)
{
	m_pccard1->option_add("16mb", FUJITSU_16MB_FLASH_CARD);
	m_pccard1->set_default_option("16mb");
}

void ksys573_state::pccard1_32mb(machine_config &config)
{
	m_pccard1->option_add("32mb", FUJITSU_32MB_FLASH_CARD);
	m_pccard1->option_add("id245p01", ID245P01);
	m_pccard1->set_default_option("32mb");
}

void ksys573_state::pccard2_32mb(machine_config &config)
{
	m_pccard2->option_add("32mb", FUJITSU_32MB_FLASH_CARD);
	m_pccard2->option_add("id245p01", ID245P01);
	m_pccard2->set_default_option("32mb");
}

void ksys573_state::pccard2_64mb(machine_config &config)
{
	m_pccard2->option_add("konami_dual", KONAMI_DUAL_PCCARD);
	m_pccard2->set_default_option("konami_dual");
}

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

void ksys573_state::cassx(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_X );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::cassxi(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_XI );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::cassy(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_Y );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::cassyi(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_YI );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::cassyyi(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_YI );
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "install", KONAMI573_CASSETTE_YI );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::casszi(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_ZI );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

void ksys573_state::cassxzi(machine_config &config)
{
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "game", KONAMI573_CASSETTE_ZI );
	subdevice<konami573_cassette_slot_device>("cassette")->option_add( "install", KONAMI573_CASSETTE_XI );
	subdevice<konami573_cassette_slot_device>("cassette")->set_default_option( "game" );
}

// Dance Dance Revolution

void ddr_state::ddr(machine_config &config)
{
	k573a(config);
	cassx(config);
}

void ddr_state::ddrk(machine_config &config)
{
	k573k(config);
	cassxi(config);
}

void ddr_state::ddr2mc2(machine_config &config)
{
	k573a(config);
	cassx(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ddr_state::ddr2ml(machine_config &config)
{
	k573a(config);
	pccard1_16mb(config);
	cassx(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ddr_state::ddrbocd(machine_config &config)
{
	k573a(config);
	pccard1_16mb(config);
	cassx(config);
}

void ddr_state::ddr3m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	pccard2_32mb(config);
	cassyyi(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ddr_state::ddr3mp(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	pccard2_32mb(config);
	cassxzi(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ddr_state::ddrusa(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	casszi(config);
}

void ddr_state::ddr5m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	pccard2_32mb(config);
	casszi(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

// Dancing Stage

void ddr_state::dsfdct(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	pccard2_32mb(config);
	cassyyi(config);
}

void ddr_state::dsfdcta(machine_config &config)
{
	k573a(config);
	pccard2_32mb(config);
	cassyyi(config);
}

void ddr_state::dsftkd(machine_config &config)
{
	k573a(config);
	cassyi(config);
}

void ddr_state::dsfdr(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	cassxzi(config);
}

void ddr_state::dsem(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	cassxi(config);
}

void ddr_state::dsem2(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ddr_state::ddr_output_callback));

	pccard2_32mb(config);
	casszi(config);
}

// Dance Dance Revolution Solo

void ksys573_state::ddrsolo(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::ddrsolo_output_callback));

	cassyi(config);
}

void ksys573_state::ddrsbm(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::ddrsolo_output_callback));
	m_k573dio->set_ddrsbm_fpga(true);

	cassyi(config);
}

void ksys573_state::ddrs2k(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::ddrsolo_output_callback));

	cassyyi(config);
}

void ksys573_state::ddr4ms(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::ddrsolo_output_callback));

	pccard2_32mb(config);
	cassxzi(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

// DrumMania

void ksys573_state::drmn(machine_config &config)
{
	k573a(config);
	cassx(config);
}

void ksys573_state::drmn2m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::drmn_output_callback));

	cassxzi(config);
}

void ksys573_state::drmn4m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::drmn_output_callback));

	casszi(config);

	KONAMI_573_MULTI_SESSION_UNIT(config, "k573msu", 0);
}

void ksys573_state::drmn9m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::drmn_output_callback));

	casszi(config);

	KONAMI_573_MULTI_SESSION_UNIT(config, "k573msu", 0);

	// KONAMI_573_NETWORK_PCB_UNIT(config, "k573npu", 0);
}

void ksys573_state::drmn10m(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::drmn_output_callback));

	casszi(config);

	KONAMI_573_MULTI_SESSION_UNIT(config, "k573msu", 0);

	// KONAMI_573_NETWORK_PCB_UNIT(config, "k573npu", 0);
}

// Guitar Freaks

void ksys573_state::gtrfrks(machine_config &config)
{
	k573a(config);
	cassx(config);
}

void ksys573_state::gtrfrk2m(machine_config &config)
{
	k573a(config);
	cassyi(config);
	pccard1_32mb(config); // HACK: The installation tries to check and erase 32mb but only flashes 16mb.
}

void ksys573_state::gtrfrk2ml(machine_config &config)
{
	k573a(config);
	cassyi(config);
	pccard1_32mb(config); // HACK: The installation tries to check and erase 32mb but only flashes 16mb.

	// For Guitar Freaks 2nd Mix Link Ver 1 (memory cards) and Link Ver 2 (memory cards + controllers)
	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ksys573_state::gtrfrk3m(machine_config &config)
{
	k573d(config);
	cassxzi(config);
	pccard1_16mb(config);

	KONAMI_573_MEMORY_CARD_READER(config, "k573mcr", 0, m_sys573_jvs_host);
}

void ksys573_state::gtrfrk5m(machine_config &config)
{
	k573d(config);
	casszi(config);
	pccard1_32mb(config);
}

void ksys573_state::gtrfrk7m(machine_config &config)
{
	k573d(config);
	casszi(config);
	pccard1_32mb(config);
}

void ksys573_state::gtfrk10m(machine_config &config)
{
	k573d(config);
	casszi(config);
	pccard1_32mb(config);

	// KONAMI_573_NETWORK_PCB_UNIT(config, "k573npu", 0);
}

void ksys573_state::gtfrk11m(machine_config &config)
{
	k573d(config);
	casszi(config);
	pccard1_32mb(config);

	// KONAMI_573_NETWORK_PCB_UNIT(config, "k573npu", 0);
}

// Miscellaneous

void ksys573_state::konami573n(machine_config &config)
{
	konami573(config, true);
}


void ksys573_state::konami573x(machine_config &config)
{
	konami573(config);
	cassx(config);
}

void ksys573_state::fbaitbc(machine_config & config)
{
	konami573(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::fbaitbc_map);

	UPD4701A(config, m_upd4701);
	m_upd4701->set_porty_tag("uPD4701_y");

	cassx(config);
}

void ksys573_state::hyperbbc(machine_config &config)
{
	konami573(config, true);
	cassy(config); // The game doesn't check the security chip

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config( "game", [this] (device_t *device) { hyperbbc_cassette_install(device); } );
}

void ksys573_state::hypbbc2p(machine_config &config)
{
	konami573(config);
	cassy(config);

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config( "game", [this] (device_t *device) { hypbbc2p_cassette_install(device); } );
}

void ksys573_state::animechmp(machine_config &config)
{
	konami573(config, true);
	cassyi(config);

	pccard1_32mb(config);

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config("game", [this](device_t* device) { animechmp_cassette_install(device); });
}

void ksys573_state::stepchmp(machine_config &config)
{
	konami573(config, true);
	cassyi(config);

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config("game", [this](device_t* device) { stepchmp_cassette_install(device); });
}

void ksys573_state::salarymc(machine_config &config)
{
	konami573(config);
	cassyi(config);

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config( "game", [this] (device_t *device) { salarymc_cassette_install(device); } );
}

void ksys573_state::gbbchmp(machine_config &config)
{
	animechmp(config);
	MB89371(config, m_duart, 0);

	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::gbbchmp_map);
}

void ksys573_state::gchgchmp(machine_config &config)
{
	konami573(config, true);
	pccard1_16mb(config);
	cassx(config);
}

void pnchmn_state::pnchmn(machine_config &config)
{

	konami573(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pnchmn_state::konami573a_map);

	cassxi(config);
	pccard1_32mb(config);

	subdevice<konami573_cassette_slot_device>("cassette")->set_option_machine_config( "game", [this] (device_t *device) { punchmania_cassette_install(device); } );
}

void pnchmn_state::pnchmn2(machine_config &config)
{
	pnchmn(config);
	pccard2_64mb(config);
}

void ksys573_state::gunmania(machine_config &config)
{
	konami573(config, true);
	m_maincpu->set_addrmap(AS_PROGRAM, &ksys573_state::gunmania_map);

	DS2401( config, "ds2401_id" );
	pccard2_32mb(config);
}

void ksys573_state::dmx(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::dmx_output_callback));

	casszi(config);
}

void ksys573_state::mamboagg(machine_config &config)
{
	k573d(config);
	m_k573dio->output_callback().set(FUNC(ksys573_state::mamboagg_output_callback));

	casszi(config);
}

void ksys573_state::mamboagga(machine_config &config)
{
	mamboagg(config);

	// TODO: Rental serial device needs to be attached here
}

void ksys573_state::kicknkick(machine_config &config)
{
	DS2401(config, "ds2401_id");
	k573ak(config);
}

static INPUT_PORTS_START( konami573 )
	PORT_START( "IN0" )
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "OUT0" )
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0834", FUNC(adc083x_device::cs_write))
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0834", FUNC(adc083x_device::clk_write))
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("adc0834", FUNC(adc083x_device::di_write))
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::h8_clk_w))

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
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::h8_d0_r))
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::h8_d1_r))
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::h8_d2_r))
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::h8_d3_r))
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::read_line_adc083x_do))
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::read_line_adc083x_sars))
//  PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x00001000, 0x00001000, "Network?" )
	PORT_CONFSETTING(          0x00001000, DEF_STR( Off ) )
	PORT_CONFSETTING(          0x00000000, DEF_STR( On ) )
//  PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::read_line_ds2401))
//  PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc0834", FUNC(adc083x_device::do_read))
//  PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::read_line_secflash_sda))
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("sys573_jvs_host", FUNC(sys573_jvs_host::jvs_sense_r))
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::jvs_rx_r))
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // JVS-related
//  PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::pccard_cd_r<0>))
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::pccard_cd_r<1>))
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
//  PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "OUT1" ) // security_w
	PORT_BIT( 0xffffff00, IP_ACTIVE_HIGH, IPT_OUTPUT )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d0))
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d1))
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d2))
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d3))
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d4))
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d5))
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d6))
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_d7))

	PORT_START( "OUT2" ) // control_w
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("cassette", FUNC(konami573_cassette_slot_device::write_line_zs01_sda))

	PORT_START( "IN2" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( fbaitbc )
	PORT_INCLUDE( konami573 )

	PORT_START( "uPD4701_y" )
	PORT_BIT( 0x0fff, 0, IPT_MOUSE_Y ) PORT_MINMAX( 0, 0xfff ) PORT_SENSITIVITY( 15 ) PORT_KEYDELTA( 8 )

	PORT_START( "uPD4701_switches" )
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", FUNC(upd4701_device::middle_w))
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", FUNC(upd4701_device::right_w))
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", FUNC(upd4701_device::left_w))
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
	PORT_BIT( 0x00000f0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ddr_state::gn845pwbb_read))

	/* IN3 bit 0x02000000 is used by ddr4mp and ddr4mps to specify that it's a rental cabinet type which requires a rental security cartridge to boot, unused by other DDR games */

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
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* P2 START */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Select L" ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Select R" ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( ddrkara )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY("IN1")
	PORT_BIT(0x10000000, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Service/Select")

	PORT_MODIFY("IN2")
	PORT_BIT(0xffff6000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x00000f0f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(ddr_state::gn845pwbb_read))
	PORT_BIT(0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Down 2")
	PORT_BIT(0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_16WAY PORT_PLAYER(1) PORT_NAME("P1 Left 2")
	PORT_BIT(0x00000040, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Telop")
	PORT_BIT(0x00000080, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mode")
	PORT_BIT(0x00001000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Level")
	PORT_BIT(0x00008000, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Start")

	PORT_MODIFY("IN3")
	PORT_BIT(0xfffffbff, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_SERVICE_NO_TOGGLE(0x00000400, IP_ACTIVE_LOW) PORT_NAME("Test")

	PORT_START("STAGE")
	PORT_BIT(0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_16WAY PORT_PLAYER(1)
	PORT_BIT(0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_16WAY PORT_PLAYER(1) /* multiplexor */
	PORT_BIT(0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_16WAY PORT_PLAYER(1) /* multiplexor */
	PORT_BIT(0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_16WAY PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( gtrfrks )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* SERVICE1 */

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000300, 0x0000, IPT_DIAL ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Effect Knob" ) PORT_MINMAX( 0x0000, 0x0300 ) PORT_SENSITIVITY( 1 ) PORT_KEYDELTA( 8 ) PORT_REVERSE
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Pick" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Wailing" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button R" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button G" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "P1 Button B" )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000003, 0x0000, IPT_DIAL ) PORT_PLAYER( 2 ) PORT_NAME( "P2 Effect Knob" ) PORT_MINMAX( 0x00, 0x03 ) PORT_SENSITIVITY( 1 ) PORT_KEYDELTA( 8 ) PORT_REVERSE
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
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::gtrfrks_lamps_b7))
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::gtrfrks_lamps_b6))
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::gtrfrks_lamps_b5))
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::gtrfrks_lamps_b4))
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
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED ) /* Used by dmx2 and dmx2majp to specify it's a rental cabinet type which requires a rental security cartridge to boot, unused by other DMX games */
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
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b0))
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b1))
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b2))
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b3))
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b4))
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::dmx_lamps_b5))
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
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::gunmania_tank_shutter_sensor))

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x0d000b00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(ksys573_state::gunmania_cable_holder_sensor))

	PORT_START( "GUNX" )
	PORT_BIT( 0x7f, 0x2f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_MINMAX( 0x00,0x5f ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 15 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ds2401_id", FUNC(ds2401_device::read))

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
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 3 ) PORT_NAME( "3P Red" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 3 ) PORT_NAME( "3P Blue" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 3 ) PORT_NAME( "3P Green" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Red" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Green" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Blue" )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Red" )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Green" )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Blue" )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( hypbbc2p )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 ) /* P1 UP */
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 START */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Red" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Green" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "1P Blue" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Red" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Blue" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 2 ) PORT_NAME( "2P Green" )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
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
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::mamboagg_lamps_b3))
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::mamboagg_lamps_b4))
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ksys573_state::mamboagg_lamps_b5))
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
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON 2 */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON 3 */
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 LEFT */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 RIGHT */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER( 1 ) /* P2 UP */
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER( 1 )/* P2 DOWN */
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 1 */
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 2 */
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON 3 */

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON4 */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER( 2 ) /* P1 BUTTON5 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED ) /* P1 BUTTON6 */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON4 */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER( 2 ) /* P2 BUTTON5 */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED ) /* P2 BUTTON6 */
INPUT_PORTS_END

static INPUT_PORTS_START( kicknkick )
	PORT_INCLUDE( konami573 )

	PORT_MODIFY( "IN2" )
	PORT_BIT( 0xffff5ffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED ) // Skips data verification on boot
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select L" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER( 1 ) PORT_NAME( "Select R" )

	PORT_MODIFY( "IN3" )
	PORT_BIT( 0xfffffbff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "KICK_SENSOR1" )
	PORT_BIT( 0xffff02ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "F1" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "F2" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "F3" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "F4" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER( 1 ) PORT_NAME( "H10" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER( 1 ) PORT_NAME( "H9" )
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ds2401_id", FUNC(ds2401_device::read))

	PORT_START( "KICK_SENSOR2" )
	PORT_BIT( 0xffff00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER( 1 ) PORT_NAME( "H8" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "H7" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "H6" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "H5" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "H4" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "H3" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "H2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "H1" )

	PORT_START( "KICK_SENSOR3" )
	PORT_BIT( 0xffff00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER( 1 ) PORT_NAME( "V8" )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER( 1 ) PORT_NAME( "V7" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER( 1 ) PORT_NAME( "V6" )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER( 1 ) PORT_NAME( "V5" )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER( 1 ) PORT_NAME( "V4" )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER( 1 ) PORT_NAME( "V3" )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER( 1 ) PORT_NAME( "V2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 ) PORT_NAME( "V1" )
INPUT_PORTS_END

#define SYS573_BIOS_A \
	ROM_SYSTEM_BIOS( 0, "std",        "Standard" ) \
	ROM_SYSTEM_BIOS( 1, "gchgchmp",   "Found on Gacha Gachamp" ) \
	ROM_SYSTEM_BIOS( 2, "dsem2",      "Found on Dancing Stage Euro Mix 2" ) \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROMX_LOAD( "700a01.22g",           0x0000000, 0x080000, CRC(11812ef8) SHA1(e1284add4aaddd5337bd7f4e27614460d52b5b48), ROM_BIOS(0) ) \
	ROMX_LOAD( "700a01,gchgchmp.22g",  0x000000,  0x080000, CRC(39ebb0ca) SHA1(9aab8c637dd2be84d79007e52f108abe92bf29dd), ROM_BIOS(1) ) \
	ROMX_LOAD( "700b01.22g",           0x0000000, 0x080000, CRC(6cf852af) SHA1(a2421d0a494892c0e71003c96995ce8f945064dd), ROM_BIOS(2) ) \
	ROM_REGION( 0x8000, "mcu", 0 ) \
	ROM_LOAD( "hd6473644h.18e", 0, 0x8000, NO_DUMP) \
	ROM_REGION( 0x40, "h8_response", 0 ) \
	ROMX_LOAD( "h8a01.bin",    0x000000, 0x000040, CRC(131e0359) SHA1(967f66578ebc0cf6b044d71af09b59bce1f4a1d0), ROM_BIOS(0) ) \
	ROMX_LOAD( "h8a01.bin",    0x000000, 0x000040, CRC(131e0359) SHA1(967f66578ebc0cf6b044d71af09b59bce1f4a1d0), ROM_BIOS(1) ) \
	ROMX_LOAD( "h8b01.bin",    0x000000, 0x000040, CRC(508b057d) SHA1(779177e6312ef272483eeb64a5e84bbae6e301f2), ROM_BIOS(2) )

// BIOS
ROM_START( sys573 )
	SYS573_BIOS_A
ROM_END

// Games
ROM_START( animechmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "pccard1:32mb:1l", 0 )
	ROM_LOAD( "ca07jaa02.1l", 0x100000, 0x100000, BAD_DUMP CRC(7be507ae) SHA1(3eee2e46a9d16662f6897d3c50841933a1fdbddb) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:1u", 0 )
	ROM_LOAD( "ca07jaa02.1u", 0x100000, 0x100000, BAD_DUMP CRC(5cca6cb3) SHA1(b8bad3e8b37712a464a582a796676cffeb1ca953) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:2l", 0 )
	ROM_LOAD( "ca07jaa02.2l", 0x100000, 0x100000, BAD_DUMP CRC(035f96b0) SHA1(dcd74bac370c65edd597f7331888ed714c081704) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:2u", 0 )
	ROM_LOAD( "ca07jaa02.2u", 0x100000, 0x100000, BAD_DUMP CRC(fce9defd) SHA1(c3ae258fc8afdbacfc718b2d4251c6f478e70c77) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:3l", 0 )
	ROM_LOAD( "ca07jaa02.3l", 0x100000, 0x100000, BAD_DUMP CRC(6fa3c80a) SHA1(8c84a29f382a85f8235848bc5dad5cfe33eb85f8) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:3u", 0 )
	ROM_LOAD( "ca07jaa02.3u", 0x100000, 0x100000, BAD_DUMP CRC(dedc20b7) SHA1(289766eb2c01214102fd177b70a5422cbf11a615) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:4l", 0 )
	ROM_LOAD( "ca07jaa02.4l", 0x100000, 0x100000, BAD_DUMP CRC(1781eac1) SHA1(01e7d71e885d786aab46a7f37e23719279320b37) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:4u", 0 )
	ROM_LOAD( "ca07jaa02.4u", 0x100000, 0x100000, BAD_DUMP CRC(04b717a2) SHA1(730fd39623f72b0fec8eb2553e82ee0fb9262f99) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:5l", 0 )
	ROM_LOAD( "ca07jaa02.5l", 0x100000, 0x100000, BAD_DUMP CRC(16e568b5) SHA1(d4627ff0eca6b0a3c4c67d429bc897039c7d7743) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:5u", 0 )
	ROM_LOAD( "ca07jaa02.5u", 0x100000, 0x100000, BAD_DUMP CRC(1cd747d2) SHA1(9b9250f6fe6ff20e2c8951610b253ce3f56265e7) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:6l", 0 )
	ROM_LOAD( "ca07jaa02.6l", 0x100000, 0x100000, BAD_DUMP CRC(cf0ef666) SHA1(d8788763301ae456412e694fcdc05eee236201fb) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:6u", 0 )
	ROM_LOAD( "ca07jaa02.6u", 0x100000, 0x100000, BAD_DUMP CRC(b74e1a51) SHA1(b0a30e706d88701f6622167e5e4534b1f2e7bb7e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:7l", 0 )
	ROM_LOAD( "ca07jaa02.7l", 0x100000, 0x100000, BAD_DUMP CRC(1ca3a2bf) SHA1(e0bcce586167b3107836f1c4aa2807871a34ff68) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:7u", 0 )
	ROM_LOAD( "ca07jaa02.7u", 0x100000, 0x100000, BAD_DUMP CRC(680d2651) SHA1(94659c5188e31acb75882597a75b7e5f29175d37) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:8l", 0 )
	ROM_LOAD( "ca07jaa02.8l", 0x100000, 0x100000, BAD_DUMP CRC(0b6c2a8e) SHA1(3871ea584f987f14e73dbcd99f29c94d4e0e6cb6) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:8u", 0 )
	ROM_LOAD( "ca07jaa02.8u", 0x100000, 0x100000, BAD_DUMP CRC(08ac7edb) SHA1(ddbd900134dfff220ef833507ef67a4883cac0f1) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca07ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(845e2907) SHA1(bf278d3eb35eaa233ccedf54bfee313b6ae772e0) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca07ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )
ROM_END

ROM_START( bassangl )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge765ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(98141cc7) SHA1(79ed944c556671a00af995e054b9aeafee245015) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "765jaa02", 0, SHA1(dfcf62581e0d0e994945cc2c37ef86827d511628) )
ROM_END

ROM_START( bassang2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc865ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(0a25aec3) SHA1(7b3c4da67f862d8d17e84cf5b9fbb3f7554cc409) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "865jaa02", 0, BAD_DUMP SHA1(b98d9aa54f13aa73bea580d6494cb6a7f3217be3) )
ROM_END

ROM_START( cr589fw )
	SYS573_BIOS_A

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "700b04", 0, BAD_DUMP SHA1(2f65f62eb7ae202153a8544989675989ed33316f) )
ROM_END

ROM_START( cr589fwa )
	SYS573_BIOS_A

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "700a04", 0, BAD_DUMP SHA1(554481f48eeb5daf8b4e7be2d66840d6c8454a52) )
ROM_END

ROM_START( darkhleg )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx706ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(04bb0b14) SHA1(6cdedee6b1e169e0be6e065f93263abd3285a21a) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "706jaa02", 0, SHA1(10101952fad80b7a10b1299158081bf86ce8cbe6) )
ROM_END

ROM_START( ddrextrm )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc36ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(197845c2) SHA1(c6c1c192f69331232a93ec6c690257e57a244e25) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc36ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c36jaa02", 0, BAD_DUMP SHA1(edeb45fff0e66151b1ba2fd67542064ccddb031e) )
ROM_END

ROM_START( ddru )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(a248d576) SHA1(081bcc86c72314159c6560681b4d03cef9e231a7) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "845uaa02", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( ddrj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc845jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(cac06b23) SHA1(6df13364d07308c19d4d4c3c92730637989619f1) )

	DISK_REGION( "runtime" )
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

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "845jaa02", 0, BAD_DUMP SHA1(37ca16be25bee39a5692dee2fa5f0fa0addfaaca) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "845jaa01", 0, NO_DUMP ) // if this even exists
ROM_END

ROM_START( ddrjb )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 ) // game doesn't try reading at all. not needed?
	ROM_LOAD( "gc845ja.u1",   0x000000, 0x000224, NO_DUMP )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31m",  0x000000, 0x200000, CRC(7acbc2ef) SHA1(64cd25ee3060399bf072346e4e6383f77755188b) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27m",  0x000000, 0x200000, CRC(67608bb7) SHA1(42f0203342b913b6ff433bcaf6cf44a471294032) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31l",  0x000000, 0x200000, CRC(c15ad83c) SHA1(79b401ff58ed84a60647531f94852b70cd86aea0) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27l",  0x000000, 0x200000, CRC(4a712a01) SHA1(e9716c6750adff78bd6c85f406fa61ae2981a1c0) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31j",  0x000000, 0x200000, CRC(4fb5437e) SHA1(21a892d0c63fa146a89bcc0e83244549f55fe4fb) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27j",  0x000000, 0x200000, CRC(3b281fa3) SHA1(05cfa8d09d6da56e4438b6d19f9bbdb5c17b651b) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.31h",  0x000000, 0x200000, CRC(724b373b) SHA1(157f879fc4f4d3f4021d40aa8e2fdd464fc2b0e3) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gc845jab.27h",  0x000000, 0x200000, CRC(43294c4b) SHA1(ebaa32495a50c78560eb4988587220ed535d0dc2) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "845jab02", 0, SHA1(bfaeff41ec0dac281fd40567309e0bb1c22354fa) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "845jab01", 0, NO_DUMP ) // if this even exists

	ROM_REGION( 0x002000, "m48t58", 0 ) // dummy file with required header to allow game to boot
	ROM_LOAD( "gc845jab.22h",  0x000000, 0x002000, BAD_DUMP CRC(b20d341e) SHA1(ed423e2ec66924ac4fada83d70dda6be75e5e85c) )
ROM_END

ROM_START( ddra )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(59d361ca) SHA1(7014768c660e591f63622442c842f442c0d8e0ea) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "845aaa02", 0, SHA1(9b786de9b1085009c088de0d40425976c1f8df7b) )
ROM_END

ROM_START( ddrkara )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq921jbb.u1", 0x000000, 0x000224, BAD_DUMP CRC(56fe552c) SHA1(51ccf6a64b478fd44a8e2d64c7a0f34138b680d2) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq921jbb.u6", 0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "921jbb02", 0, SHA1(32849007fe5527245a6cc2de38e73d60ae74eff0) )
ROM_END

ROM_START( ddrkara2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc947ja.u1", 0x000000, 0x000224, CRC(5bea7b68) SHA1(46d3ae1ffa97474cc2e91cb5982dcf4ca9cc9a1d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc947ja.u6", 0x000000, 0x000008, CRC(6d76f4a9) SHA1(4fe5d9151ca0a0a67d64657c9a82776b928f429b) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "947jaa02", 0, SHA1(c1e9ad0386b52867bdabd550b95e1651d07d972c) )
ROM_END

ROM_START( ddr2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(5d906be5) SHA1(4ea9c5506aaaf1726f2a39d0a37a8df35a6aad47) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(a066ad5e) SHA1(1783c62d7396e4e7f8d723b7bc07e45285dc122d) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "896jaa01", 0, BAD_DUMP SHA1(f802a0e2ba0147eb71c54d92af409c3010a5715f) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddr2mc2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge984ja.u1",  0x000000, 0x000224, BAD_DUMP CRC(a066ad5e) SHA1(1783c62d7396e4e7f8d723b7bc07e45285dc122d) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "ge984a01,ddr", 0, SHA1(badd15656f2316f81b0a45026b5ef10287d1480b) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(f02bb09f41533c6ec496a662d815e85b304fcc72) )
ROM_END

ROM_START( ddr2ml )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(a066ad5e) SHA1(1783c62d7396e4e7f8d723b7bc07e45285dc122d) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "885jac01", 0, SHA1(ed864096ee99aa813f40642b9467fe2cbb07d669) )
ROM_END

ROM_START( ddr2mla )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(a066ad5e) SHA1(1783c62d7396e4e7f8d723b7bc07e45285dc122d) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "885jab01", 0, SHA1(c2bbb9e2e6f34e07f57e7076726af81df39f55c9) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(f02bb09f41533c6ec496a662d815e85b304fcc72) )
ROM_END

ROM_START( ddr2mlb )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge885jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(a066ad5e) SHA1(1783c62d7396e4e7f8d723b7bc07e45285dc122d) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "885jaa02", 0, SHA1(f02bb09f41533c6ec496a662d815e85b304fcc72) )
ROM_END

ROM_START( ddr3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(68e01a7b) SHA1(a0094ae0f78ef4246957f998b1493973b3bb67ec) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(0c095e3d) SHA1(0a5b3d4ba917f55b29ad80d989c20675c14b3cad) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "887aaa02", 0, SHA1(6f9a0e9dd046a1fc0c81be9eeb45c136574a4472) )
ROM_END

ROM_START( ddr3mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(cc8838ee) SHA1(b4db610103c0c5ba473f725b3b8b7f987ac3eaec) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(987e93b2) SHA1(9892b479fdc2c54b2e23783c69e270aa60596673) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "887jaa02", 0, SHA1(8736818f42822f77e3484ea46a9e63faa7f8517a) )
ROM_END

ROM_START( ddr3mk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(68e01a7b) SHA1(a0094ae0f78ef4246957f998b1493973b3bb67ec) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887kb.u1",   0x000000, 0x000084, BAD_DUMP CRC(0c095e3d) SHA1(0a5b3d4ba917f55b29ad80d989c20675c14b3cad) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887kb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "887kba02", 0, SHA1(9f2c6a4e7ad0de44295dc09b9b054afb044238a9) )
ROM_END

ROM_START( ddr3mka )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(68e01a7b) SHA1(a0094ae0f78ef4246957f998b1493973b3bb67ec) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn887ka.u1",   0x000000, 0x000084, BAD_DUMP CRC(0c095e3d) SHA1(0a5b3d4ba917f55b29ad80d989c20675c14b3cad) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn887ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "887kaa02", 0, SHA1(d002f2c98c012d67ad0587553e1d0f45c0ae470e) )
ROM_END

ROM_START( ddr3mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea22ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(adfc9021) SHA1(bb161b9c1bd358cb72e61197a9fafd58c44adb7c) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca22ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(02f8ba64) SHA1(3d1b660ff7c98de7f6386fc4d3300dc4205b5a3a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca22ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a22jaa02", 0, SHA1(dc3c1223882716d47b4f4db45b5dd2e988cba64c) )
ROM_END

ROM_START( ddr4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea33aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(a9bf9a0e) SHA1(7210ff9606746bc94bfba28e3522ee79222da32c) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33aa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(284effe1) SHA1(bdaeef1b47b643d9ad0ebac82651e60752565fcb) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a33aaa02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4mj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a33jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(adfc9021) SHA1(bb161b9c1bd358cb72e61197a9fafd58c44adb7c) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(3a291fea) SHA1(15e777d2137313e925d922e7289dd46c00838211) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a33jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a33jaa02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4ms )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea33ab.u1",   0x000000, 0x000224, BAD_DUMP CRC(4ca14f40) SHA1(90cdcaea12f1305c1068496df80ef1069625e398) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33ab.u1",   0x000000, 0x00008c, BAD_DUMP CRC(15d7ccf0) SHA1(2bda973c83cf2b042753492201a51db2e415338e) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33ab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a33aba02", 0, BAD_DUMP SHA1(cc7349cbee37bfb101480497e99f1f52acb4ffa1) )
ROM_END

ROM_START( ddr4msj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a33jba.u1",    0x000000, 0x000224, BAD_DUMP CRC(b458d46a) SHA1(791b938e0a65131238e66fe86efa5f83d9225332) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca33jb.u1",   0x000000, 0x00008c, BAD_DUMP CRC(07b02cfb) SHA1(291d654d9dec8180f48a9da1cb71f4a4f7e86c26) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a33jba.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca33jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a33jba02", 0, BAD_DUMP SHA1(9d9fb5e65f1532f358e9c273c56d11389d11fd79) )
ROM_END

ROM_START( ddr4mp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea34ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(adfc9021) SHA1(bb161b9c1bd358cb72e61197a9fafd58c44adb7c) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca34ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(e7f5f795) SHA1(cf942ed741d9605aaa1d99bfa848cc13f3f87b1b) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca34ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gca34ja.22h",  0x000000, 0x002000, CRC(80575c1f) SHA1(a0594ca0f75bc7d49b645e835e9fa48a73c3c9c7) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a34jaa02", 0, SHA1(c33e43192ce49845f8901c505f1c7867bc643a0b) )
ROM_END

ROM_START( ddr4mps )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea34jb.u1",   0x000000, 0x000224, BAD_DUMP CRC(b458d46a) SHA1(791b938e0a65131238e66fe86efa5f83d9225332) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca34jb.u1",   0x000000, 0x00008c, BAD_DUMP CRC(da6cc484) SHA1(1fe1aa95bbee1de7fd4930e6883f99768486da2a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca34jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gca34jb.22h",  0x000000, 0x002000, CRC(bc6c8bd7) SHA1(10ceec5c7bc5ca9fca88f3c083a7d97012982079) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a34jba02", 0, BAD_DUMP SHA1(c33e43192ce49845f8901c505f1c7867bc643a0b) ) // Check if there was a separate CD created for solo cabinets.
ROM_END

ROM_START( ddr5m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca27ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(0044c64d) SHA1(ac25b51125675ccca992d60b12396b3b370c6368) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca27ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a27jaa02", 0, SHA1(70465669dfd48abf806cb58b2410ff4f1781f5f1) )
ROM_END

ROM_START( ddrbocd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn895jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(5d906be5) SHA1(4ea9c5506aaaf1726f2a39d0a37a8df35a6aad47) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "892jaa01", 0, BAD_DUMP SHA1(46ace0feef48a2a6643c3cb4ac9164ba0beeea94) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "895jaa02", 0, BAD_DUMP SHA1(cfe3a6f3ed62ba388b07045e29e22472d17dcfe4) )
ROM_END

ROM_START( ddrs2k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(b7a75390) SHA1(8ad65f4d9fa949296b5b21e7a12be0bde8ffec38) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc905aa.u1",   0x000000, 0x000084, BAD_DUMP CRC(961a9dbe) SHA1(c112af1da26eee8da237655ed28d9bae2bc5728a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "905aaa02", 0, BAD_DUMP SHA1(1fc0f3fcc7d5d23711967023ff02c1fc76479024) )
ROM_END

ROM_START( ddrmax )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb19ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(680a3288) SHA1(b413c6c43c4a18c5c713049a9c2fbde2d98e36bc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb19ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b19jaa02", 0, SHA1(fe8a6731a2163fe7864cd3c4457034768eb98caa) )
ROM_END

ROM_START( ddrmax2 )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb20ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(169be875) SHA1(de40794ebf9190fe3676326d21150fd6aa72dc25) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb20ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b20jaa02", 0, SHA1(ef6579192b86cfea08debe82f54fc4aae5985c92) )
ROM_END

ROM_START( ddrs2kj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(f3b65e55) SHA1(24656affd3fe3a0a594e2cd0aeb91eb6ceb996b8) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc905ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(00bca330) SHA1(c83bd38846698cdfba9d7d40c51a59720a75bfee) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge905ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc905aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "905jaa02", 0, SHA1(a78cf628fb2ba823e1ca35cbd611938273ab82ac) )
ROM_END

ROM_START( ddrsbm )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq894ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(10b85f6b) SHA1(c15ec333c8bb0c33edb8d31bf4f309aa0db9ff55) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq894ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "894jaa02", 0, SHA1(d6872078a87ee00280a627675540676fb8b10f60) )
ROM_END

ROM_START( ddrusa )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gka44ua.u1",   0x000000, 0x00008c, BAD_DUMP CRC(ca12febe) SHA1(3bd01332e5b20420ec195ab125671cb212a8f5f2) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gka44ua.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a44uaa02", 0, BAD_DUMP SHA1(2cdbe1c62d16a2be65adb7e11331fce5c8e45504) )
ROM_END

ROM_START( drmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ff3a2868) SHA1(3949fe928cc435ceca8bb2fe8b5a558d91542dfd) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "881xxb02", 0, BAD_DUMP SHA1(9252ff1841584c06506f58c9a9cefbc82b32187d) ) // drummania/percussion freaks hybrid cd
ROM_END

ROM_START( drmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(ff3a2868) SHA1(3949fe928cc435ceca8bb2fe8b5a558d91542dfd) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ja.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ja.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "881jad01", 0, BAD_DUMP SHA1(7d9d47bef636dbaa8d578f34ea9489e349d3d6df) ) // upgrade cd

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "881jaa02", 0, SHA1(dad3eab14bb4535dd52885fef022720caf280e2b) )
ROM_END

ROM_START( drmn2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(508d618d) SHA1(d30ba54ccc86072c10962632603028206ff08d4b) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(952f4681) SHA1(667fc5ef07517801496444a91b6f1d88c8d0b9d2) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )
ROM_END

ROM_START( drmn2mpu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(508d618d) SHA1(d30ba54ccc86072c10962632603028206ff08d4b) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(952f4681) SHA1(667fc5ef07517801496444a91b6f1d88c8d0b9d2) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "912jab02", 0, BAD_DUMP SHA1(19dfae94b63468d3e16d3cc4a3eeae60d5dff1d7) )

	// How to install:
	// 1. Set dipswitch 4 to CD-ROM mode, switch to install security cart, and restart machine to install using the runtime CD
	// 2. Switch to game security cart, switch to install CD, and restart machine to install power up version (will have a graphical installer)
	// 3. Set dipswitch 4 to Flash ROM mode, switch to runtime CD again, and restart machine
	// You should see "SESSION POWERUP KIT INSTALLED" in top left corner after initial boot (will always show up if you go into operator menu and select game mode as an easy test)
	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "912za01",  0, BAD_DUMP SHA1(033a310006efe164cc6a8276de42a5d555f9fea9) )
ROM_END

ROM_START( drmn3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a23jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(508d618d) SHA1(d30ba54ccc86072c10962632603028206ff08d4b) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca23ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(e78225db) SHA1(0870cfb01cd3f940812b842b315bc6329e956d88) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a23jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca23ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a23jaa02", 0, BAD_DUMP SHA1(89e365f61a4db889621d7d9d9917bcfa2c09704e) )
ROM_END

ROM_START( drmn4m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea25jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(8ee64348) SHA1(9c02096c94da68d096efc225f19e420cb9445820) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.31m", 0x000000, 0x200000, CRC(a0dd0ef4) SHA1(be4c1d3f2eb3c484b515be12b692c30cc780c36c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25jaa.27m", 0x000000, 0x200000, CRC(118fa45a) SHA1(6bc6129e328f6f97a27b9f524066297b29efff5a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea25jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a25jaa02", 0, BAD_DUMP SHA1(8a0b761d1c282d927e2daf92519654a1c91ee1ab) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "a25jba02", 0, BAD_DUMP SHA1(5f4aae359da610352c1004cfa1a32064d8f55d0e) )
ROM_END

ROM_START( drmn5m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb05jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(1f0e9f51) SHA1(64f9ae6c2d9a6264b09c1b66ae2acac9db9ada32) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb05jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b05jaa02", 0, BAD_DUMP SHA1(7a6e7940d1441cff1d9be1bc3affc029fe6dc9e4) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b05jba02", 0, BAD_DUMP SHA1(822149db553ca78ad8174719a657dbbd2776b922) )
ROM_END

ROM_START( drmn6m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb16jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(a8ab305d) SHA1(7815ec63fa419174d689f237bcb3ec2372fb6baa) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.31m",  0x000000, 0x200000, CRC(19de3e53) SHA1(bbb7a247bdd617a124330a946c2e8dd565b2a09c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16jaa.27m",  0x000000, 0x200000, CRC(5696e133) SHA1(aad39cc25ce5279adac8a10fb10158f4f4418c0a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb16jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b16jaa02", 0, BAD_DUMP SHA1(fa0862a9bd3a48d4f6e7b44b11ad387acc05037e) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "b16jba02", 0, BAD_DUMP SHA1(07de74a3ca384407d99c433110085208a458653e) )
ROM_END

ROM_START( drmn7m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc07jba.u1",   0x000000, 0x00008c, BAD_DUMP CRC(9b85fd8a) SHA1(34fc42389fc5eb5d09a0a7b6b4c4b4597b1f6849) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.31m",  0x000000, 0x200000, CRC(7120d1ce) SHA1(4df9828150120762b99c5b212bc7a91b0d525bce) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jba.27m",  0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc07jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c07jca02", 0, SHA1(a81a35360933ab8a7630cf5e8a8c6988714cfa0d) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jda02", 0, BAD_DUMP SHA1(7c22ebbda11bdaf85c3441d7a6f3497994cd957f) )
ROM_END

ROM_START( drmn7ma )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc07jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(a5477405) SHA1(f532bba5f089a263bbfd07bb66f028663c659a84) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.31m",  0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07jaa.27m",  0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc07jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c07jaa02", 0, BAD_DUMP SHA1(96c410745d1fd14059bf11987655ed998a9b79dd) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c07jba02", 0, BAD_DUMP SHA1(25e1a3ff7886c409d16e40ca1798b01b11546755) )
ROM_END

ROM_START( drmn8m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc38jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(c2895619) SHA1(49dcebd44435760dde02638b0e0a89b743064e69) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc38jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c38jaa02", 0, SHA1(9115252e6cc13ff90e73cd1a864e0d99e3c8b5ea) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "c38jba02", 0, SHA1(2a31335277929b2231b12ad950ab69e35b37d973) )
ROM_END

ROM_START( drmn9m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd09jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(f73f2f6b) SHA1(30abe964fcef3901b2098e32946568f4a7c617d7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd09jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d09jab01", 0, SHA1(f3962f77b96a48bf0195700f4b72bf02d75a7e03) )

	DISK_REGION( "install2" )
	DISK_IMAGE_READONLY( "d09jca02", 0, SHA1(4583ce07fca67660f9f1928589f39d3f551206ff) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d09jaa02", 0, BAD_DUMP SHA1(33f3e48ed5a8becd8c4714413e454328d8d5baae) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d09jba02", 0, BAD_DUMP SHA1(68324d474d89a9ddf5cadc9ab4a8d615b3739879) )
ROM_END

ROM_START( drmn9ma )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd09jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(f73f2f6b) SHA1(30abe964fcef3901b2098e32946568f4a7c617d7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd09jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d09jca02", 0, SHA1(4583ce07fca67660f9f1928589f39d3f551206ff) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d09jaa02", 0, BAD_DUMP SHA1(33f3e48ed5a8becd8c4714413e454328d8d5baae) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d09jba02", 0, BAD_DUMP SHA1(68324d474d89a9ddf5cadc9ab4a8d615b3739879) )
ROM_END

ROM_START( drmn10m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd40jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(d98d4aa5) SHA1(7142c1e6291fdaac726477662487c7600c048e0a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd40jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d40jca02", 0, SHA1(f0208a62e2d15773961e89383ae8d4bd2e8f6b47) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d40jaa02", 0, BAD_DUMP SHA1(68b2038f0cd2d461f608945d1e243f2b6979efaa) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d40jba02", 0, BAD_DUMP SHA1(0ded9e0a6c77b181e7b6beb1dbdfa17dee4acd90) )
ROM_END

ROM_START( dmx )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge874ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(f27cfd58) SHA1(4d8a61bdf2c5f3bebcb37cb2bac1e3de90a9a30e) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge874ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "874jaa", 0, BAD_DUMP SHA1(3338a784efdca4f8bdcc83d2c9a6bbe7f7046d5c) )
ROM_END

ROM_START( dmxa )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge874aa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(cf2fc30c) SHA1(1d2688f8fe2d8b51dd73815dedb3e99f53439846) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge874aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "874aaa02", 0, SHA1(6985445a492e9d8ccd9938c7c47402c3886e2d62) )
ROM_END

ROM_START( dmx2m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca39ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(7f1e7a66) SHA1(65afe81544e096769c4b7ecf9ffde760f69dc331) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a39jaa02", 0, BAD_DUMP SHA1(3d021448df857c12f6d46a20e14ae0fc6d342dcc) )
ROM_END

ROM_START( dmx2majp )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca38ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(34d8db6f) SHA1(7936b7e1e464ce1738f38f9a0960507404735e86) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.31m",  0x000000, 0x200000, CRC(a0f54ab5) SHA1(a5ae67d7619393779c79a2e227cac0675eeef538) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gca38ja.27m",  0x000000, 0x200000, CRC(6c3934b8) SHA1(f0e4a692b6caaf60fefaec87fd23da577439f69d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca38ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a38jaa02", 0, SHA1(27fbecefb634ce282ca3bf09500c0c9e8155a7ef) )
ROM_END

ROM_START( dncfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gk874ka.u1",   0x000000, 0x00008c, BAD_DUMP CRC(43244ec6) SHA1(07040d4bc491274d39d6c5d39e346c0905892cb4) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gk874ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "874kaa", 0, BAD_DUMP SHA1(4d1e843417ea96635eeba0cef944e83fdb72565c) )
ROM_END

ROM_START( dsem )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge936ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(c7e3a7f9) SHA1(7120c0ef3c3dac23713922f9447d5eac1b91c54d) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge936ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "936eaa", 0, BAD_DUMP SHA1(7cacc15ae065d47af31f1008374ec8241dba0d55) )
ROM_END

ROM_START( dsem2 )
	SYS573_BIOS_A
	ROM_DEFAULT_BIOS("dsem2")

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gkc23ea.u1",   0x000000, 0x00008c, BAD_DUMP CRC(107e9868) SHA1(02a0f92fc3167d37818818332bf189f39e3c25fc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gkc23ea.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c23eaa02", 0, BAD_DUMP SHA1(46868c97530db5be1b43ffa32744e3e12495c243) )
ROM_END

ROM_START( dsfdct )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge887ja_gn887ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(bfbba6c7) SHA1(eb082fe7bb1bbc3656391a0617075010281b8726) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc910jc.u1",   0x000000, 0x000084, BAD_DUMP CRC(e09eb1c6) SHA1(046d5552c774ea2b9141fff45c13a926c18f705c) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge887ja_gn887ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc910jc.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "910jca02", 0, BAD_DUMP SHA1(0c868f3c9f696d291e8f27687e3ad83e453a4894) )
ROM_END

ROM_START( dsfdcta )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(12e9807b) SHA1(4bbd4696b9345030daf8922775463cea3ee792c5) )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc910ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(eebf9f88) SHA1(afc5f63f4b4251edfe12e9bbc6a788ddb76c0985) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gc910ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "910jaa02", 0, BAD_DUMP SHA1(70851c383e3876c4a697a99706fbaae2dafcb0e0) )
ROM_END

ROM_START( dsfdr )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "gea37ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(adfc9021) SHA1(bb161b9c1bd358cb72e61197a9fafd58c44adb7c) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca37ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(d7df793c) SHA1(3b85ef84edda34dce13fcafb7f6c77f4154eee5d) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "gea37ja.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca37ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a37jaa02", 0, BAD_DUMP SHA1(c6a23b910e884aa0d4afc388dbc8379e0d09611a) )
ROM_END

ROM_START( dsftkd )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn884ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(12e9807b) SHA1(4bbd4696b9345030daf8922775463cea3ee792c5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn884ja.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "884jaa02", 0, BAD_DUMP SHA1(80f02fcb7ea5b6394a2a58f12b73d87a1826d7f4) )
ROM_END

ROM_START( dstage )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845eb.u1",  0x000000, 0x000084, BAD_DUMP CRC(35a88d75) SHA1(122317074a96f36cdfc33cd26851bab3d514da3c) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn884eb.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "gc845eba", 0, BAD_DUMP SHA1(0b7b100ceb37ac30cc1d309e5fe11fde5e1192d0) )
ROM_END

ROM_START( dstagea )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn845ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(b0cb136c) SHA1(ff9471517d94caaf6f8e07c0aa4f70762cbcd6ee) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "845uaa02", 0, BAD_DUMP SHA1(d3f9290d4dadb5e9b82ebe77abf7b99d1a89f716) )
ROM_END

ROM_START( fbait2bc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc865ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(e9f61a3d) SHA1(367914f142811ca592700c43570b4498a32434cd) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "865uab02", 0, BAD_DUMP SHA1(d14dc066d4c16fba1e9b31d5f042ad249c4b5137) )
ROM_END

ROM_START( fbaitbc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge765ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(5bfe59b0) SHA1(a15ae8e95a4fe6a3fa212297af8324b075b315c9) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "765uab02", 0, BAD_DUMP SHA1(07b09e763e4b90108aa924b518221b16667a7133) )
ROM_END

ROM_START( fbaitmc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(ceadb821) SHA1(12da44b0c4f4d4af133d1e8e0089602c0a3d8411) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "889ea", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmca )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(27b5ca87) SHA1(5c3c822b1bd8c03aabdebcd927cf4479fdfc3bff) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "889aa", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(d9ef0053) SHA1(56503e668693af092e680fada47314ca4dcfea5f) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "889ja", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fbaitmcu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx889ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(dc2e7e3b) SHA1(461910918392037f9e33eb7ed2ac8e494c141fe1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "889ua", 0, BAD_DUMP SHA1(0b567bf2f03ee8089e0b021ea502a53b3f6fe7ac) )
ROM_END

ROM_START( fghtmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918eaa.u1",  0x000000, 0x000224, CRC(4bf384d9) SHA1(094d541efb27ad4f2faf799b141069b78db6ad6b) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918eaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918aaa.u1",  0x000000, 0x000224, CRC(a2ebf67f) SHA1(e845ffd517c23f5d577898267bec1572bb9559ef) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918aaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918kaa.u1",  0x000000, 0x000224, CRC(77f53227) SHA1(649d3fbf47369487f366a8d6192e79612be4ec83) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918kaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( fghtmnu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918uaa.u1",  0x000000, 0x000224, CRC(597042c3) SHA1(5bc16259c0c0d55bbb2142bae79cc9be3dc52304) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918uaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( gbbchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "pccard1:32mb:1l", 0 )
	ROM_LOAD( "cb48jab02.1l", 0x100000, 0x100000, BAD_DUMP CRC(c461f9d8) SHA1(739adaafc121a2978802e0a2e1551954e34e60c6) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:1u", 0 )
	ROM_LOAD( "cb48jab02.1u", 0x100000, 0x100000, BAD_DUMP CRC(a909447e) SHA1(03ddd1a34bd51a11a4a838b75a8885b6acb4daff) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:2l", 0 )
	ROM_LOAD( "cb48jab02.2l", 0x100000, 0x100000, BAD_DUMP CRC(c67b8134) SHA1(632a02f5c35906f6f4512a68caf98a70dc4d0d98) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:2u", 0 )
	ROM_LOAD( "cb48jab02.2u", 0x100000, 0x100000, BAD_DUMP CRC(e3f5a88b) SHA1(d9103810e5c9d64d73525c5c2176a5e6c5fd4be4) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:3l", 0 )
	ROM_LOAD( "cb48jab02.3l", 0x100000, 0x100000, BAD_DUMP CRC(d8a58e21) SHA1(5a58a6759aa4bca7e35033cc411a2058e2f2e31f) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:3u", 0 )
	ROM_LOAD( "cb48jab02.3u", 0x100000, 0x100000, BAD_DUMP CRC(6a26bcc0) SHA1(92bedd98a28ebb04e2e3c1a9f16f6d4c7a5be29e) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:4l", 0 )
	ROM_LOAD( "cb48jab02.4l", 0x100000, 0x100000, BAD_DUMP CRC(d61d6e20) SHA1(121360976d515a2539f1b1d508591b70dd375095) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:4u", 0 )
	ROM_LOAD( "cb48jab02.4u", 0x100000, 0x100000, BAD_DUMP CRC(d0babf51) SHA1(929f2e940c9639c9fcf7bb6a7ba5e15c43a343b4) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:5l", 0 )
	ROM_LOAD( "cb48jab02.5l", 0x100000, 0x100000, BAD_DUMP CRC(5848bdd0) SHA1(14ea255adc644fa49ca6967ba36087e6ac9046dc) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:5u", 0 )
	ROM_LOAD( "cb48jab02.5u", 0x100000, 0x100000, BAD_DUMP CRC(e18e2e43) SHA1(8a460d86fcc0713b46bf2786aa3bb40faa8a2f23) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:6l", 0 )
	ROM_LOAD( "cb48jab02.6l", 0x100000, 0x100000, BAD_DUMP CRC(8b6da035) SHA1(1993d8f9c68dc5fea19f3d9a9348c6ab55cda9cf) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:6u", 0 )
	ROM_LOAD( "cb48jab02.6u", 0x100000, 0x100000, BAD_DUMP CRC(84968845) SHA1(64f66fa377388305047dccb2f9c6ab1881788da6) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:7l", 0 )
	ROM_LOAD( "cb48jab02.7l", 0x100000, 0x100000, BAD_DUMP CRC(a36fc186) SHA1(5bb93bbb41729b64bcb32cf5b6d572d71fcd4437) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:7u", 0 )
	ROM_LOAD( "cb48jab02.7u", 0x100000, 0x100000, BAD_DUMP CRC(dd6b3c8c) SHA1(1350f4d8287105f18e108f2687f51371e20396cd) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:8l", 0 )
	ROM_LOAD( "cb48jab02.8l", 0x100000, 0x100000, BAD_DUMP CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x200000, "pccard1:32mb:8u", 0 )
	ROM_LOAD( "cb48jab02.8u", 0x100000, 0x100000, BAD_DUMP CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb48ja.u1",   0x000000, 0x000084, BAD_DUMP CRC(879ff48d) SHA1(c548173c5adfda08a82a04e0d0fbcfc529ed41a0) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb48ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )
ROM_END

ROM_START( gchgchmp )
	SYS573_BIOS_A
	ROM_DEFAULT_BIOS("gchgchmp")

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
	ROM_LOAD( "ge877jaa.u1",  0x000000, 0x000224, CRC(6e2f06ff) SHA1(79901dfcc5b1bd89cf464246c27fc9657aff3f05) )
ROM_END

ROM_START( gtrfrks )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(3b38dccf) SHA1(c313d025b4faed73ab021b76dbf0c51c9d701eb3) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__d02", 0, BAD_DUMP SHA1(8d6681d6cacd054a047ad984184fa0dfa383ecb9) )
ROM_END

ROM_START( gtrfrksu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(29bb1ad5) SHA1(797ba61b6407fc0d0531e58129a9230e1484dc11) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__d02", 0, BAD_DUMP SHA1(8d6681d6cacd054a047ad984184fa0dfa383ecb9) )
ROM_END

ROM_START( gtrfrksj )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(2c7a64bd) SHA1(01bd46989a897dbd023603e5cabbd7deb85825ff) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__d02", 0, BAD_DUMP SHA1(8d6681d6cacd054a047ad984184fa0dfa383ecb9) )
ROM_END

ROM_START( gtrfrksa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(d220ae69) SHA1(dfdd29197222cb621b05f9b9091cff9a20cd846e) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__d02", 0, BAD_DUMP SHA1(8d6681d6cacd054a047ad984184fa0dfa383ecb9) )
ROM_END

ROM_START( gtrfrksc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(3b38dccf) SHA1(c313d025b4faed73ab021b76dbf0c51c9d701eb3) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksuc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(29bb1ad5) SHA1(797ba61b6407fc0d0531e58129a9230e1484dc11) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksjc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(2c7a64bd) SHA1(01bd46989a897dbd023603e5cabbd7deb85825ff) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksac )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(d220ae69) SHA1(dfdd29197222cb621b05f9b9091cff9a20cd846e) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__c02", 0, BAD_DUMP SHA1(80293512c4b914ef98acb1bbc7e3a2ed944a0dad) )
ROM_END

ROM_START( gtrfrksea )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886eac.u1",  0x000000, 0x000224, BAD_DUMP CRC(3b38dccf) SHA1(c313d025b4faed73ab021b76dbf0c51c9d701eb3) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__a02", 0, SHA1(f34f5678cfc0292e65e114480f167866664f4173) )
ROM_END

ROM_START( gtrfrksua )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886uac.u1",  0x000000, 0x000224, BAD_DUMP CRC(29bb1ad5) SHA1(797ba61b6407fc0d0531e58129a9230e1484dc11) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__a02", 0, SHA1(f34f5678cfc0292e65e114480f167866664f4173) )
ROM_END

ROM_START( gtrfrksja )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886jac.u1",  0x000000, 0x000224, BAD_DUMP CRC(2c7a64bd) SHA1(01bd46989a897dbd023603e5cabbd7deb85825ff) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__a02", 0, SHA1(f34f5678cfc0292e65e114480f167866664f4173) )
ROM_END

ROM_START( gtrfrksaa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq886aac.u1",  0x000000, 0x000224, BAD_DUMP CRC(d220ae69) SHA1(dfdd29197222cb621b05f9b9091cff9a20cd846e) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "886__a02", 0, SHA1(f34f5678cfc0292e65e114480f167866664f4173) )
ROM_END

ROM_START( gtrfrk2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(3b4f65b0) SHA1(ddec1da89feb848ba412b70064bf45ca4ba50426) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP SHA1(4f6bb0150ad6ed574dd7583ccd60604028663b2a) )
ROM_END

ROM_START( gtrfrk2ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883eaa.u1",  0x000000, 0x000084, BAD_DUMP CRC(1777a6eb) SHA1(6d5fd8fd63bf96de31a1b2d6e372aa728b5deaab) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "883__a01", 0, BAD_DUMP SHA1(913c55a41d313e6197e0a176a2fba1461cda17f2) )
ROM_END

ROM_START( gtrfrk2mua )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883uaa.u1",  0x000000, 0x000084, BAD_DUMP CRC(6ca19ad2) SHA1(5ef4a1c65966b646a8808ec1d97893d3df40819f) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "883__a01", 0, BAD_DUMP SHA1(913c55a41d313e6197e0a176a2fba1461cda17f2) )
ROM_END

ROM_START( gtrfrk2mja )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883jaa.u1",  0x000000, 0x000084, BAD_DUMP CRC(3b4f65b0) SHA1(ddec1da89feb848ba412b70064bf45ca4ba50426) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "883__a01", 0, BAD_DUMP SHA1(913c55a41d313e6197e0a176a2fba1461cda17f2) )
ROM_END

ROM_START( gtrfrk2mka )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883kaa.u1",  0x000000, 0x000084, BAD_DUMP CRC(34d01c64) SHA1(98fa0ff0d1f7f179648d4c37dd89d0cf5a322e64) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "883__a01", 0, BAD_DUMP SHA1(913c55a41d313e6197e0a176a2fba1461cda17f2) )
ROM_END

ROM_START( gtrfrk2maa )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883aaa.u1",  0x000000, 0x000084, BAD_DUMP CRC(7f5e6875) SHA1(44a217e674c82443a7876dab46dc56af75010ccf) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "883__a01", 0, BAD_DUMP SHA1(913c55a41d313e6197e0a176a2fba1461cda17f2) )
ROM_END

ROM_START( gtrfrk2ml1 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(3b4f65b0) SHA1(ddec1da89feb848ba412b70064bf45ca4ba50426) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "929jaa02", 0, BAD_DUMP SHA1(7667f6d8eee055a12ed032ab2087db10f1d21bd4) )
ROM_END

ROM_START( gtrfrk2ml2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq883jad.u1",  0x000000, 0x000084, BAD_DUMP CRC(3b4f65b0) SHA1(ddec1da89feb848ba412b70064bf45ca4ba50426) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq883jad.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "929jbb02", 0, BAD_DUMP SHA1(4f6bb0150ad6ed574dd7583ccd60604028663b2a) )
ROM_END

ROM_START( gtrfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(e98b0389) SHA1(bbc5d54946625d140b1b431cbbb09118b49d39f1) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x00008c, BAD_DUMP CRC(e7bd2ccc) SHA1(93721ef9fcd65073438c60fc4403293a7670811a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "949jac01", 0, BAD_DUMP SHA1(ff017dd5c0ecbdb8935d0d4656a45e9fab10ef82) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "949jab02", 0, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3ma )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "949jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(e98b0389) SHA1(bbc5d54946625d140b1b431cbbb09118b49d39f1) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jab.u1",  0x000000, 0x00008c, BAD_DUMP CRC(e7bd2ccc) SHA1(93721ef9fcd65073438c60fc4403293a7670811a) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "949jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "949jab02", 0, BAD_DUMP SHA1(ad629c9bafbdc4bf6c679918a5fae2bcfdb39332) )
ROM_END

ROM_START( gtfrk3mb )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "ge949jaa.u1",  0x000000, 0x00008c, BAD_DUMP CRC(e7bd2ccc) SHA1(93721ef9fcd65073438c60fc4403293a7670811a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "ge949jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "949jaz02", 0, BAD_DUMP SHA1(b0c786ba420a34fcbd16bc36a137f6ae87b7dfa8) )
ROM_END

ROM_START( gtrfrk4m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a24jaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(e98b0389) SHA1(bbc5d54946625d140b1b431cbbb09118b49d39f1) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea24ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(6a6bb4b5) SHA1(a7ae107e42201f694a99e7c58b0f5b26d147b3d4) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a24jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea24ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a24jaa02", 0, BAD_DUMP SHA1(bc0303f5a6a19484cd35890cc9934ee0bcabb2ad) )
ROM_END

ROM_START( gtrfrk5m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea26jaa.u1",  0x000000, 0x00008c, BAD_DUMP CRC(5e16a3f1) SHA1(629dce132588035cea20d1fe92e0f17f90d0ef60) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.31m", 0x000000, 0x200000, CRC(1a25e660) SHA1(dbd8fad0bac307723c70d00763cadf4261a7ed73) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26jaa.27m", 0x000000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea26jaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a26jaa02", 0, BAD_DUMP SHA1(9909e08abff780db6fd7a5fbcc57ffbe14ae08ce) )
ROM_END

ROM_START( gtrfrk5ma )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea26aaa.u1",  0x000000, 0x00008c, BAD_DUMP CRC(09539c73) SHA1(a92a9c41121d98e06890323564cf731c437d9286) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26aaa.31m", 0x000000, 0x200000, CRC(59a9a1a0) SHA1(f673d8f60a77e2257ffc9c7e486fc836733bc14c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea26aaa.27m", 0x000000, 0x200000, CRC(345dc5f2) SHA1(61af3fcfe6119c1e8e18b92693855ab4fe708b30) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea26aaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a26aaa02", 0, SHA1(79c430664b79fb8f8d78b61b0192b73e70d3f206) )
ROM_END

ROM_START( gtrfrk6m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb06ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(6a4b2436) SHA1(c9ad3b9b1c1b58c70b7ceb0332e8904f62cd56f9) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb06ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b06jaa02", 0, BAD_DUMP SHA1(2ea53ef492da63183a28c54afde07fce323fe42e) )
ROM_END

ROM_START( gtrfrk7m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb17jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(3e08140c) SHA1(8ba94ced641edab17a61e25b890985280bf5d30b) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.31m", 0x000000, 0x200000, CRC(1e1cbfe3) SHA1(6c942820f915ea0e01f0e736d70780ad8408aa69) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb17jaa.27m", 0x000000, 0x200000, CRC(7e7da9a9) SHA1(1882418779a48b5aefd113895756116379a6a4f7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb17jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b17jaa02", 0, SHA1(daf23982abbab882882f89b3a9d985df36252cae) )
ROM_END

ROM_START( gtrfrk8m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc08jba.u1",   0x000000, 0x00008c, BAD_DUMP CRC(b7da7085) SHA1(19702458b22dc49e5475ee6e39b02cca9b626122) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.31m", 0x000000, 0x200000, CRC(ddef5efe) SHA1(7c3a219eacf63f55894e81cb0e41753176191708) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jba.27m", 0x000000, 0x200000, CRC(9393fe8e) SHA1(f60752e3e397121f3d4856a634e1c8ce5fc465b5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc08jba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c08jba02", 0, BAD_DUMP SHA1(8e352ed8ade581b7c9bb579fc56003ea1831202c) )
ROM_END

ROM_START( gtrfrk8ma )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc08jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(e35a6b58) SHA1(a10f670adae91efbef09b2a1c8c29e1ccb1036bc) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.31m", 0x000000, 0x200000, CRC(aa723d4c) SHA1(5f55ddaf7f21b624deac99cc40b89989cd6f3a3d) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc08jaa.27m", 0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc08jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c08jaa02", 0, BAD_DUMP SHA1(7a1d97f74ec4d643ff7d3981d66b551cbf9e57f0) )
ROM_END

ROM_START( gtrfrk9m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc39jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(865d2191) SHA1(9f5226d01164d83c36fde315de2d15a2f3b61604) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc39jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c39jaa02", 0, SHA1(fef1202457b6bae2e10be6ecea35369820ffded5) )
ROM_END

ROM_START( gtfrk10m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd10jab.u1",   0x000000, 0x00008c, BAD_DUMP CRC(4147d6fb) SHA1(35877a6c295369f3c6857f6e33fad80abf111156) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd10jab.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d10jab01", 0, BAD_DUMP SHA1(c84858b412f0798a65cf3059c743501f32ad7280) )

	DISK_REGION( "install2" )
	DISK_IMAGE_READONLY( "d10jba02", 0, BAD_DUMP SHA1(80893da422268cc1f89688289cdec981c4f9feb2) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d10jaa02", 0, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk10ma )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd10jaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(97b5afc8) SHA1(ac57ac0dfb0ee73ad5e0819505153fe9d7da9dc9) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd10jaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d10jba02", 0, BAD_DUMP SHA1(80893da422268cc1f89688289cdec981c4f9feb2) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d10jaa02", 0, BAD_DUMP SHA1(d4e4460ca3edc1b365af593757557c6cf5b7b3ec) )
ROM_END

ROM_START( gtfrk11m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd39ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(809301a6) SHA1(718e646bc6e72d89e78b771fb64374f77c5662a0) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd39ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d39jba02", 0, SHA1(a10225d1dd6cfd22382970099927aeba5e0c03e7) ) // e-Amusement song data installer for HDD, requires NPU

	// Supports both JA/JB and AA/AB regions, the only difference seems to be the warning screen and some online network-related details
	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d39jaa02", 0, BAD_DUMP SHA1(7a87ee331ba0301bb8724c398e6c77cfb9c172a7) )
ROM_END

ROM_START( gunmania )
	SYS573_BIOS_A

	ROM_REGION( 0x000008, "ds2401_id", 0 ) /* digital board id */
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

ROM_START( hndlchmp )
	SYS573_BIOS_A
	ROM_DEFAULT_BIOS("gchgchmp")

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.31m",   0x000000, 0x200000, CRC(0eec3edf) SHA1(9624bbf207e81cdbc2754cce81aeb57d4042ddce) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.27m",   0x000000, 0x200000, CRC(7571daa8) SHA1(5f07b894a954d1eb13c4da1533a634d8185f4e54) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.31l",   0x000000, 0x200000, CRC(1dc20303) SHA1(46cc531977865e718ed2049651baef2eccbc233e) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.27l",   0x000000, 0x200000, CRC(5109ca33) SHA1(11c0ec8773240a002083205dceff21dac2fb7a1a) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.31j",   0x000000, 0x200000, CRC(bdc05d16) SHA1(ee397950f7e7e910fdc05737f99604e43d288719) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.27j",   0x000000, 0x200000, CRC(ad925ed3) SHA1(e3222308961851cccee2de9da804f74854907451) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.31h",   0x000000, 0x200000, CRC(dfcef2cd) SHA1(5b5025ae5d64767a17523d7e1559cf03c0609b85) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "710saa.27h",   0x000000, 0x200000, CRC(eda05636) SHA1(d63ccb52946b1f70874d81ba53e09ca08b0d7f71) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "710saa.22h",   0x000000, 0x002000, CRC(6a8b13b6) SHA1(10218242ea14cd355c139d47598a195d71db6352) )
ROM_END

ROM_START( hndlchmpj )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.31m",   0x000000, 0x200000, CRC(f5f71b1d) SHA1(7d518e5333f44e6ec921a1e882df970953814b6e) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.27m",   0x000000, 0x200000, CRC(b3d8c037) SHA1(678b88c37111d1fde8996c7d71b66ec1c4f161fe) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.31l",   0x000000, 0x200000, CRC(78e8556c) SHA1(9f6bb651ddeb042ebf1ba057d4932494149f47d6) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.27l",   0x000000, 0x200000, CRC(f6a87155) SHA1(269bfdf05ee4ab2e4b87b6e92045e56d0557a576) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.31j",   0x000000, 0x200000, CRC(bdc05d16) SHA1(ee397950f7e7e910fdc05737f99604e43d288719) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.27j",   0x000000, 0x200000, CRC(ad925ed3) SHA1(e3222308961851cccee2de9da804f74854907451) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.31h",   0x000000, 0x200000, CRC(a0293108) SHA1(2e5651a4c1b8e021cc3060db138c9fe7c28caa3b) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "710jab.27h",   0x000000, 0x200000, CRC(aed26efe) SHA1(20b6fccd0bc5495d8258b976f72d330d6315c6f6) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "710jab.22h",   0x000000, 0x002000, CRC(b784de91) SHA1(048157e9ad6df46656dbac6349b0c821254e1c37) )
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

ROM_START( hyperbbck )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.31m",    0x000000, 0x200000, CRC(b2f5ea67) SHA1(205416c2954cfc303f164bb74f66356c393db294) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.27m",    0x000000, 0x200000, CRC(d5f32438) SHA1(3bc8598af2e8817bbcb381f90a9b12d5736abed7) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.31l",    0x000000, 0x200000, CRC(628cd211) SHA1(5c2d5f95bf3e7995ad32dc432c81e69e42ba9b88) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.27l",    0x000000, 0x200000, CRC(4a860adf) SHA1(02aea8c205ea5b094d1a52dadc751c11d6b8aab7) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.31j",    0x000000, 0x200000, CRC(4d572e90) SHA1(ac06a4f4efcee2729b131da8634eced85338196a) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.27j",    0x000000, 0x200000, CRC(f80953f7) SHA1(c82bea38a8dc19ed99e5fd5c97cbffd7669581a7) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.31h",    0x000000, 0x200000, CRC(4f99ef5b) SHA1(df02cdc61455a470cadada16c43e7f153d9d48c7) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "876ka.27h",    0x000000, 0x200000, CRC(21586113) SHA1(a563e383961b8e2421869070fe384ed910ed2fe4) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "876ka.22h",    0x000000, 0x002000, CRC(b4705bde) SHA1(3005982b3c237181c6a03b42bf37ffe79f68dc79) )
ROM_END

ROM_START( hypbbc2p )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx908ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(8900eaff) SHA1(aea4449c2453875694dfa92451d46dace710faf2) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( hypbbc2pk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx908ka.u1",  0x000000, 0x000084, BAD_DUMP CRC(869f932b) SHA1(db5f96536ede2c16fc5fd5ab6a8efdf0f03be43b) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "908a02", 0, BAD_DUMP SHA1(573194ca9938c30415fc88dcc0c0152dd3024d71) )
ROM_END

ROM_START( jppyex98 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc811ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(fd70efbe) SHA1(206fd4a6424723f082856c19617461897d127638) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "811jaa02", 0, BAD_DUMP SHA1(e9580172d58b38841f643651ae0bcaf24fd5f118) )
ROM_END

ROM_START( konam80a )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826aa.u1", 0x000000, 0x000224, BAD_DUMP CRC(da4cff17) SHA1(9a7b074d15b7d861132d9588c753cd18a469590f) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "826aaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80j )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(0479ef71) SHA1(043c01f115dd1434273729d06be3ba9c2d7b7a86) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "826jaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80k )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ka.u1", 0x000000, 0x000224, BAD_DUMP CRC(b4d57749) SHA1(8edbc08bc160350aa45dc4eb75dff29bcfd3d8f0) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "826kaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80s )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ea.u1", 0x000000, 0x000224, BAD_DUMP CRC(7c12ec3f) SHA1(7989ed47fe551116a821c7d4bc4de6bbb22eff29) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "826eaa01", 0, BAD_DUMP SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( konam80u )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gc826ua.u1", 0x000000, 0x000224, BAD_DUMP CRC(077fefab) SHA1(2b3db957b6b767854ffeb99edd767040cb981489) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "826uaa01", 0, SHA1(be5f8b31fd18ba631fe98c2132c56abf20193419) )
ROM_END

ROM_START( mamboagg )
	SYS573_BIOS_A
	ROM_DEFAULT_BIOS("dsem2")

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa40jab.u1",  0x000000, 0x00008c, BAD_DUMP CRC(bbb6033d) SHA1(22f3d0e9541be7c24338e97a8c29f0baadf40557) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa40jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(2e4ed217a7e9f7c79abc9a1798556cc3649db30e) )
ROM_END

ROM_START( mamboagga )
	SYS573_BIOS_A
	ROM_DEFAULT_BIOS("dsem2")

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa40jrb.u1",  0x000000, 0x00008c, BAD_DUMP CRC(313b5685) SHA1(8df9cab660af8f9d057a1360ec4dc9f9ed50e185) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa40jrb.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a40jab02", 0, SHA1(2e4ed217a7e9f7c79abc9a1798556cc3649db30e) )
ROM_END

ROM_START( mrtlbeat )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "geb47jb.u1",   0x000000, 0x00008c, BAD_DUMP CRC(83de7dd1) SHA1(cc9afd1f8f93829e845dbbb9e27eb786525e9d66) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "geb47jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b47jxb02", 0, SHA1(6bbe8d6169ef692bd8995da564bd5a97b6bf0b31) )
ROM_END

ROM_START( mrtlbeata )
	// Small differences compared to the standard mrtlbeat JBA version:
	// - Lacks the "original program" mode
	// - Select menu screen layout is different due to lack of "original program" mode
	// - Ending screen lacks one line of text
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "geb47ja.u1",   0x000000, 0x00008c, BAD_DUMP CRC(be474ec0) SHA1(328d352847cdddcf04a6179e45f2f273bee713dd) ) // hand crafted

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "geb47jb.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b47jxb02", 0, SHA1(6bbe8d6169ef692bd8995da564bd5a97b6bf0b31) )
ROM_END

ROM_START( powyakex )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gx802ja.u1", 0x000000, 0x000224, BAD_DUMP CRC(82743f7b) SHA1(9019de5f38e79477327e4dde23e1e572e8450d67) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "802jab02", 0, SHA1(460cc9f0b2514ec1da06b0a1d7b52fe43220d181) )
ROM_END

ROM_START( pcnfrk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ea.u1",   0x000000, 0x000224, BAD_DUMP CRC(e878901a) SHA1(5c58c6b580b6d864d42e1fc0dd4852cfe4bbe02b) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ea.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ea.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ea.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY("881xxb02", 0, BAD_DUMP SHA1(9252ff1841584c06506f58c9a9cefbc82b32187d))
ROM_END

ROM_START( pcnfrka )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(0160e2bc) SHA1(d18cb24c1ebf4ab4492ab2a2d863e522a7b47692) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881aa.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881aa.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881aa.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY("881xxb02", 0, BAD_DUMP SHA1(9252ff1841584c06506f58c9a9cefbc82b32187d))
ROM_END

ROM_START( pcnfrkk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ka.u1",   0x000000, 0x000224, BAD_DUMP CRC(d47e26e4) SHA1(c74209be867eeb4196e2c3a6cf2ef6c89ba9faf4) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ka.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ka.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ka.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY("881xxb02", 0, BAD_DUMP SHA1(9252ff1841584c06506f58c9a9cefbc82b32187d))
ROM_END

ROM_START( drmnu )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq881ua.u1",   0x000000, 0x000224, BAD_DUMP CRC(fafb5600) SHA1(b18c0ca09e563be710f3c3fcfc408a62131c6d18) )

	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ua.31h",  0x000000, 0x200000, CRC(a5b86ece) SHA1(9696f0c512501574bae6e436306675894bb2352e) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "gq881ua.27h",  0x000000, 0x200000, CRC(fc0b94c1) SHA1(967d374288db757d161d0e9e8e396a1176071c5f) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "gq881ua.22h",  0x000000, 0x002000, CRC(e834d5ec) SHA1(1c845811e43d7dfec657da288b5a38b8bc9c8366) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY("881xxb02", 0, BAD_DUMP SHA1(9252ff1841584c06506f58c9a9cefbc82b32187d))
ROM_END

ROM_START( pcnfrk2m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912aa.u1",   0x000000, 0x000224, BAD_DUMP CRC(aed7ab59) SHA1(41103d383c35dab0b074e336b1b07dd346a855ee) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912aa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(89928c8b) SHA1(5999e5366ae12c24d7a68fad4f1e12dbee6ef7b2) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "912aaa02", 0, BAD_DUMP SHA1(0abc1c32c71c535ee2deb3e1fa574f1e723b97b0) )
ROM_END

ROM_START( pcnfrk2mk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "ge912ka.u1",   0x000000, 0x000224, BAD_DUMP CRC(7bc96f01) SHA1(ea6a51a6b10c56beb68c949b6939e9d2fef3f44e) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gn912ka.u1",   0x000000, 0x00008c, BAD_DUMP CRC(bd67eb6b) SHA1(35e3434effaae1222b9e72886fa6ae28a74b9a42) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "ge912ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gn912ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "912kaa02", 0, BAD_DUMP SHA1(f817054453b7d66260813fe823f348280b4e9902) )
ROM_END

ROM_START( pcnfrk3m )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a23aaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(aed7ab59) SHA1(41103d383c35dab0b074e336b1b07dd346a855ee) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca23aa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(f5e5c5d0) SHA1(8a27fbb3ac32bc4be9dad97bb3c428bb4bc60b24) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a23aaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca23aa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a23aaa02", 0, BAD_DUMP SHA1(aab3b2e1167f0b4ab101d77fa3bb25db8ab01437) )
ROM_END

ROM_START( pcnfrk3mk )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:install:eeprom", 0 )
	ROM_LOAD( "a23kaa.u1",    0x000000, 0x000224, BAD_DUMP CRC(7bc96f01) SHA1(ea6a51a6b10c56beb68c949b6939e9d2fef3f44e) )

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca23ka.u1",   0x000000, 0x00008c, BAD_DUMP CRC(ee247c6f) SHA1(e6aa6b45e8151ea6d84834be93c831c6394864e7) )

	ROM_REGION( 0x000008, "cassette:install:id", 0 )
	ROM_LOAD( "a23kaa.u6",    0x000000, 0x000008, BAD_DUMP CRC(af09e43c) SHA1(d8372f2d6e0ae07061b496a2242a63e5bc2e54dc) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca23ka.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a23kaa02", 0, BAD_DUMP SHA1(5b853cc25eb583ed36d8cd402235b4f5c9ce065a) )
ROM_END

ROM_START( pcnfrk4m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gea25aaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(9c81a343) SHA1(1ad243e4905e5c7ee1355221759f337bdec5bb58) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25aaa.31m", 0x000000, 0x200000, CRC(557093c2) SHA1(80e8eecf3248e890f9cce70859b6092725c4918c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gea25aaa.27m", 0x000000, 0x200000, CRC(118fa45a) SHA1(6bc6129e328f6f97a27b9f524066297b29efff5a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gea25aaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a25aaa02", 0, BAD_DUMP SHA1(cea168d38a4052ef5f30dc00a80529bbd8a31097) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "a25aba02", 0, BAD_DUMP SHA1(eb8eed41c715f39a426433224671adc36d4b0262) )
ROM_END

ROM_START( pcnfrk4mk )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gka25kaa.u1",   0x000000, 0x00008c, CRC(2b0efc90) SHA1(84bfc8dd365e9d51d248d4dbc09274aa3dff345f) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gka25kaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a25kaa02", 0, BAD_DUMP SHA1(6c9084773c6964e794bd096822da4413aa7305ef) )
ROM_END

ROM_START( pcnfrk5m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb05aaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(484ba0d3) SHA1(4d24a38e0975449dd0f155021d9c804b7547354a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb05aaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b05aaa02", 0, SHA1(71e244ba03d6e761921c24f2a2ae5835f0a69021) )
ROM_END

ROM_START( pcnfrk5mk )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb05kaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(6fd59b3f) SHA1(5118a56f4303dce78a806b49ff9e834be85528a7) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb05kaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b05kaa02", 0, SHA1(10e7ca2b0d8ed29f5882cccd735b6d1cb13aca21) )
ROM_END

ROM_START( pcnfrk6m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcb16aaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(95f80e09) SHA1(14e7998baf42bc8610638a4e5aa13ba1afb361e2) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16aaa.31m",  0x000000, 0x200000, CRC(52c9334b) SHA1(2288f729611aefe93a470f8ef88a211582136e86) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcb16aaa.27m",  0x000000, 0x200000, CRC(5696e133) SHA1(aad39cc25ce5279adac8a10fb10158f4f4418c0a) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcb16aaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "b16aaa02", 0, BAD_DUMP SHA1(d268548e7cbfb22a8127509aeb84b4487b0e7460) )
ROM_END

ROM_START( pcnfrk7m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc07aaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(98144a51) SHA1(376d265a0f897ad116cdd88a673f6e20d880204a) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07aaa.31m",  0x000000, 0x200000, CRC(e32a680d) SHA1(fcdf8bc9ff2290350b2653588047c78ec6c5f4a8) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gcc07aaa.27m",  0x000000, 0x200000, CRC(49d27b57) SHA1(e62737fe8665d837c2cebd1dcf4577a021d8cdb1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc07aaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c07aaa02", 0, BAD_DUMP SHA1(3edb219d5fd7cf25c851fa02c7c1926c9e2baa02) )
ROM_END

ROM_START( pcnfrk8m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcc38aaa.u1",   0x000000, 0x00008c, BAD_DUMP CRC(5a3611fe) SHA1(279dbad199abdaa947f8c1e39cf08385d9e102ab) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcc38aaa.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "c38aaa02", 0, SHA1(df1699e6216cfccf5ff0cad8ad2b66a8c4c8cfc9) )
ROM_END

ROM_START( pcnfrk9m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd09aba.u1",   0x000000, 0x00008c, BAD_DUMP CRC(1fe73b91) SHA1(b3c49b84295b9dfc946b12a53bdec5cd24541c98) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd09aba.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d09aba02", 0, BAD_DUMP SHA1(a817d1c7fdb354b7d2d5c08f92a352c76a2b1a72) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d09aaa02", 0, SHA1(593706872e3a541acfe4d9527c3a5e89cc193e98) )
ROM_END

ROM_START( pcnfrk10m )
	SYS573_BIOS_A

	ROM_REGION( 0x000008c, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gcd40aca.u1",   0x000000, 0x00008c, BAD_DUMP CRC(cbeaaaae) SHA1(ccc6c47b20ab847859ec37c9c45dc90b24a232b5) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gcd40aca.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "install" )
	DISK_IMAGE_READONLY( "d40aca02", 0, BAD_DUMP SHA1(3a23808e13b689f3ed2a1fa1ce541a4b82765d97) ) // e-Amusement song data installer for HDD, requires NPU

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "d40aaa02", 0, SHA1(638c19588c5f9967eb5623a3a979ac68e7c96dae) )

	DISK_REGION( "multisession" )
	DISK_IMAGE_READONLY( "d40aba02", 0, NO_DUMP )
ROM_END

ROM_START( pnchmn )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918jaa.u1",  0x000000, 0x000224, BAD_DUMP CRC(5cb13cab) SHA1(7b4e084df560c317f5530d6d5f4052ec51ecc683) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918jaa.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918xxb02", 0, BAD_DUMP SHA1(8ced8952fff3e70ce0621a491f0973af5a6ccd82) )
ROM_END

ROM_START( pnchmna )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq918jab.u1",  0x000000, 0x000224, BAD_DUMP CRC(5cb13cab) SHA1(7b4e084df560c317f5530d6d5f4052ec51ecc683) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.31m", 0x000000, 0x200000, CRC(3653b5d7) SHA1(1deb44335b7a38506fb30da40e0ca61b96aea7bb) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gq918xxb.27m", 0x000000, 0x200000, CRC(27d48c97) SHA1(c140d4bdfa869fbcae1133bbfe73a346e6f46cb8) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gq918jab.u6",  0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "918jab02", 0, SHA1(8b8cb806a4e15b4687456a5a4482ea7e1373bbf6) )
ROM_END

ROM_START( pnchmn2 )
	SYS573_BIOS_A

	ROM_REGION( 0x0000224, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gqa09ja.u1",   0x000000, 0x000224, BAD_DUMP CRC(5923bba3) SHA1(97b099779f20f4169c3171c73f55048fb5314c50) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.31m",  0x000000, 0x200000, CRC(b1043a91) SHA1(b474439c1a7da7855d9b6d2162d4a522f499d6ab) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "gqa09ja.27m",  0x000000, 0x200000, CRC(09b1a70b) SHA1(0f3bcad879e05faaf8130133d774a2071031ee74) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gqa09ja.u6",   0x000000, 0x000008, BAD_DUMP CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a09jaa02", 0, BAD_DUMP SHA1(b085fbe76d5ef87578744b45b874b5f79147e586) )
ROM_END

ROM_START( salarymc )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gca18jaa.u1",  0x000000, 0x000084, CRC(c4e827e3) SHA1(9429eedcc793d84223c5494b50e64b906e2fa8e1) )

	ROM_REGION( 0x000008, "cassette:game:id", 0 )
	ROM_LOAD( "gca18jaa.u6",  0x000000, 0x000008, CRC(ce84419e) SHA1(839e8ee080ecfc79021a06417d930e8b32dfc6a1) )

	DISK_REGION( "runtime" )
	DISK_IMAGE_READONLY( "a18jaa02", 0, SHA1(740cc93ec65433098153684fdfc418a098a43736) )
ROM_END

ROM_START( stepchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x0000084, "cassette:game:eeprom", 0 )
	ROM_LOAD( "gq930ja.u1",  0x000000, 0x000084, BAD_DUMP CRC(d3e541fd) SHA1(157812a2afeb287804c659115a2a7f8bb7fd2ee9) )

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

ROM_START( strgchmp )
	SYS573_BIOS_A

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.31m",   0x000000, 0x200000, CRC(389f8acb) SHA1(db80af29d53f737a6affd1afd18cce050c294fa6) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.27m",   0x000000, 0x200000, CRC(5b5c6e4d) SHA1(a5ad9a459504dfb7ada0148f590f777a604549e9) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.31l",   0x000000, 0x200000, CRC(fd9fd9b1) SHA1(f1536f62f68d80cabd5d58e8dfa14a8b3e9a6ae3) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.27l",   0x000000, 0x200000, CRC(a0e9b72b) SHA1(7272c54c5dc1d1df2427e301e5a10083c9db1967) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.31j",   0x000000, 0x200000, CRC(bf6ebc47) SHA1(59a4812e79c9695a0d161fd62534bbb4ae84a4bc) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.27j",   0x000000, 0x200000, CRC(79669ebd) SHA1(45828fd6c0b63666963c8f3b7cc5d508bc06b845) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.31h",   0x000000, 0x200000, CRC(2a41c844) SHA1(ffa36fef3abc90f2e25e79ccbda199d614edf034) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "710uaa.27h",   0x000000, 0x200000, CRC(b30556bb) SHA1(b771114c6520fe17e4256b9217c72243ac97f4b5) )

	ROM_REGION( 0x002000, "m48t58", 0 )
	ROM_LOAD( "710uaa.22h",   0x000000, 0x002000, CRC(a3e93d49) SHA1(630daa1a02320433a068eb5214f6b30acc06df76) )
ROM_END

ROM_START( kicknkick )
	SYS573_BIOS_A

	ROM_REGION( 0x000008, "ds2401_id", 0 ) /* digital board id */
	ROM_LOAD( "ds2401",        0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )

	ROM_REGION( 0x200000, "29f016a.31m", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.31m",   0x000000, 0x200000, CRC(f7461ee1) SHA1(60898894237ef2c478eb91c1d11e0f2beda7d55c) )
	ROM_REGION( 0x200000, "29f016a.27m", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.27m",   0x000000, 0x200000, CRC(80379c73) SHA1(9e226c258570efc6c45b76b277010c23527ce480) )
	ROM_REGION( 0x200000, "29f016a.31l", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.31l",   0x000000, 0x200000, CRC(a1129729) SHA1(50b8134b2fd6fd82ad1e95db39633f1e338174ea) )
	ROM_REGION( 0x200000, "29f016a.27l", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.27l",   0x000000, 0x200000, CRC(8f489306) SHA1(d5a27ed048139fd46404cb11e5d42a813989e0ab) )
	ROM_REGION( 0x200000, "29f016a.31j", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.31j",   0x000000, 0x200000, CRC(4608ab06) SHA1(033e30f0a866bcedd9b718d234e788c8919f7f3a) )
	ROM_REGION( 0x200000, "29f016a.27j", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.27j",   0x000000, 0x200000, CRC(1f75eb84) SHA1(c54c4221a2fa0668b688ec2479abf1838461493a) )
	ROM_REGION( 0x200000, "29f016a.31h", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.31h",   0x000000, 0x200000, CRC(6475da5f) SHA1(6b1de2ed06504583bfa3e5b11851c459d227e6d8) )
	ROM_REGION( 0x200000, "29f016a.27h", 0 ) /* onboard flash */
	ROM_LOAD( "a36eaa.27h",   0x000000, 0x200000, CRC(1179ab7b) SHA1(19a316cacb6eb87b905884091820e6b53aef64b7) )
ROM_END

} // anonymous namespace


// FIXME: dependency hell strikes again
// this shouldn't be here at all, but the spaghetti is so tangled
double konami573_cassette_xi_device::punchmania_inputs_callback(uint8_t input)
{
	// The values 50 and 150 come from the game's internal I/O simulation mode.
	// Set DIPSW 2 ("Screen Flip") to ON and press select left + start on the I/O test screen to see the simulated I/O in action.
	constexpr double POT_MIN = 50;
	constexpr double POT_MAX = 150;
	constexpr double POT_RANGE = POT_MAX - POT_MIN;
	constexpr int MOTOR_SPEED_MUL = 2;

	pnchmn_state *state = machine().driver_data<pnchmn_state>();
	double *pad_position = state->m_pad_position;
	int *pad_motor_direction = state->m_pad_motor_direction;
	int pads = state->m_pads->read();
	attotime curtime = machine().time();
	double elapsed = ( curtime - state->m_last_pad_update ).as_double();
	double diff = POT_RANGE * elapsed * MOTOR_SPEED_MUL;

	for( int i = 0; i < 6; i++ )
	{
		if( BIT( pads, i ) )
		{
			pad_position[ i ] = POT_MIN;
		}
		else
		{
			pad_position[ i ] = std::clamp( pad_position[ i ] + ( diff * pad_motor_direction[ i ] ), POT_MIN, POT_MAX );
		}
	}

	machine().output().set_value( "left top pad", pad_position[ 0 ] );
	machine().output().set_value( "left middle pad", pad_position[ 1 ] );
	machine().output().set_value( "left bottom pad", pad_position[ 2 ] );
	machine().output().set_value( "right top pad", pad_position[ 3 ] );
	machine().output().set_value( "right middle pad", pad_position[ 4 ] );
	machine().output().set_value( "right bottom pad", pad_position[ 5 ] );

	state->m_last_pad_update = curtime;

	switch( input )
	{
	case ADC083X_CH0: /* Left Top */
	case ADC083X_CH1: /* Left Middle */
	case ADC083X_CH2: /* Left Bottom */
	case ADC083X_CH3: /* Right Top */
	case ADC083X_CH4: /* Right Middle */
	case ADC083X_CH5: /* Right Bottom */
		return 5.0 - ( 5.0 * ( pad_position[ input ] / 255.0 ) );
	case ADC083X_COM:
		return 0;
	case ADC083X_VREF:
		return 5;
	}
	return 5;
}


GAME( 1997, sys573,    0,        konami573n, konami573, ksys573_state, empty_init,    ROT0,  "Konami", "System 573 BIOS", MACHINE_IS_BIOS_ROOT )

GAME( 1997, strgchmp,  sys573,   konami573n, hndlchmp,  ksys573_state, empty_init,    ROT0,  "Konami", "Steering Champ (GQ710 97/12/18 VER. UAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, hndlchmp,  strgchmp, konami573n, hndlchmp,  ksys573_state, empty_init,    ROT0,  "Konami", "Handle Champ (GQ710 97/12/18 VER. SAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, hndlchmpj, strgchmp, konami573n, hndlchmp,  ksys573_state, empty_init,    ROT0,  "Konami", "Handle Champ (GQ710 1997/12/08 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, darkhleg,  sys573,   konami573x, konami573, ksys573_state, empty_init,    ROT0,  "Konami", "Dark Horse Legend (GX706 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, fbaitbc,   sys573,   fbaitbc,    fbaitbc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait - A Bass Challenge (GE765 VER. UAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, bassangl,  fbaitbc,  fbaitbc,    fbaitbc,   ksys573_state, empty_init,    ROT0,  "Konami", "Bass Angler (GE765 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, powyakex,  sys573,   konami573x, konami573, ksys573_state, empty_init,    ROT0,  "Konami", "Jikkyou Pawafuru Puro Yakyu EX (GX802 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, jppyex98,  sys573,   konami573x, konami573, ksys573_state, empty_init,    ROT0,  "Konami", "Jikkyou Pawafuru Puro Yakyu EX '98 (GC811 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80s,  sys573,   konami573x, konami573, ksys573_state, empty_init,    ROT90, "Konami", "Konami 80's AC Special (GC826 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80u,  konam80s, konami573x, konami573, ksys573_state, empty_init,    ROT90, "Konami", "Konami 80's AC Special (GC826 VER. UAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80j,  konam80s, konami573x, konami573, ksys573_state, empty_init,    ROT90, "Konami", "Konami 80's Gallery (GC826 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80a,  konam80s, konami573x, konami573, ksys573_state, empty_init,    ROT90, "Konami", "Konami 80's AC Special (GC826 VER. AAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, konam80k,  konam80s, konami573x, konami573, ksys573_state, empty_init,    ROT90, "Konami", "Konami 80's AC Special (GC826 VER. KAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, dstage,    sys573,   dsftkd,     ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dancing Stage - Internet Ranking Ver (GC845 VER. EBA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, dstagea,   dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dancing Stage (GN845 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddru,      dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution (GN845 VER. UAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, ddrj,      dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution - Internet Ranking Ver (GC845 VER. JBA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, ddrja,     dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution (GC845 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, ddrjb,     dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution (GC845 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddra,      dstage,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution (GN845 VER. AAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddrkara,   sys573,   ddrk,       ddrkara,   ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution Karaoke Mix (GQ921 VER. JBB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, fbait2bc,  sys573,   fbaitbc,    fbaitbc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait 2 - A Bass Challenge (GE865 VER. UAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, bassang2,  fbait2bc, fbaitbc,    fbaitbc,   ksys573_state, empty_init,    ROT0,  "Konami", "Bass Angler 2 (GE865 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, hyperbbc,  sys573,   hyperbbc,   hyperbbc,  ksys573_state, init_hyperbbc, ROT0,  "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, hyperbbca, hyperbbc, hyperbbc,   hyperbbc,  ksys573_state, init_hyperbbc, ROT0,  "Konami", "Hyper Bishi Bashi Champ (GQ876 VER. AAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, hyperbbck, hyperbbc, hyperbbc,   hyperbbc,  ksys573_state, init_hyperbbc, ROT0,  "Konami", "Hyper Bishi Bashi Champ (GE876 VER. KAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gchgchmp,  sys573,   gchgchmp,   gchgchmp,  ksys573_state, empty_init,    ROT0,  "Konami", "Gacha Gachamp (GE877 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrks,   sys573,   gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks Ver 1.01 (GQ886 VER. EAD)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksu,  gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks Ver 1.01 (GQ886 VER. UAD)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksj,  gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks Ver 1.01 (GQ886 VER. JAD)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksa,  gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks Ver 1.01 (GQ886 VER. AAD)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksc,  gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. EAC)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksuc, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. UAC)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksjc, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. JAC)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksac, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. AAC)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksea, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksua, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. UAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksja, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrksaa, gtrfrks,  gtrfrks,    gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks (GQ886 VER. AAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmc,   sys573,   fbaitbc,    fbaitmc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. EA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmcu,  fbaitmc,  fbaitbc,    fbaitmc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. UA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmcj,  fbaitmc,  fbaitbc,    fbaitmc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. JA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, fbaitmca,  fbaitmc,  fbaitbc,    fbaitmc,   ksys573_state, empty_init,    ROT0,  "Konami", "Fisherman's Bait - Marlin Challenge (GX889 VER. AA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2m,     sys573,   ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix (GN895 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2ml,    ddr2m,    ddr2ml,     ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAC)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mla,   ddr2m,    ddr2ml,     ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mlb,   ddr2m,    ddr2ml,     ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix - Link Ver (GE885 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddrbocd,   ddr2m,    ddrbocd,    ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution Best of Cool Dancers (GE892 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mc,    ddr2m,    ddr,        ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX CLUB VERSiON (GE896 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, ddr2mc2,   ddr2m,    ddr2mc2,    ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution 2nd Mix with beatmaniaIIDX substream CLUB VERSiON 2 (GE984 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, pcnfrk,    sys573,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "Percussion Freaks (GQ881 VER. EAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, pcnfrka,   pcnfrk,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "Percussion Freaks (GQ881 VER. AAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, pcnfrkk,   pcnfrk,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "Percussion Freaks (GQ881 VER. KAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, drmnu,     pcnfrk,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "DrumMania (GQ881 VER. UAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, drmn,      pcnfrk,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "DrumMania (GQ881 VER. JAD)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, drmna,     pcnfrk,   drmn,       drmn,      ksys573_state, init_drmn,     ROT0,  "Konami", "DrumMania (GQ881 VER. JAD ALT CD)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, gtrfrk2m,  sys573,   gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix Ver 1.01 (GQ883 VER. JAD)", MACHINE_IMPERFECT_SOUND ) // Link Kit 2 without the memcard readers boots into GQ883 JAD
GAME( 1999, gtrfrk2ma, gtrfrk2m, gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix (GQ883 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2mua,gtrfrk2m, gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix (GQ883 VER. UAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2mja,gtrfrk2m, gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix (GQ883 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2mka,gtrfrk2m, gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix (GQ883 VER. KAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2maa,gtrfrk2m, gtrfrk2m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix (GQ883 VER. AAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2ml1,gtrfrk2m, gtrfrk2ml,  gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix Link Kit 1 (GE929 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gtrfrk2ml2,gtrfrk2m, gtrfrk2ml,  gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 2nd Mix Link Kit 2 (GC929 VER. JBB)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, dsftkd,    sys573,   dsftkd,     ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dancing Stage featuring TRUE KiSS DESTiNATiON (G*884 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, cr589fw,   sys573,   konami573,  konami573, ksys573_state, empty_init,    ROT0,  "Konami", "CD-ROM Drive Updater 2.0 (700B04)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, cr589fwa,  sys573,   konami573,  konami573, ksys573_state, empty_init,    ROT0,  "Konami", "CD-ROM Drive Updater (700A04)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, ddr3mk,    sys573,   ddr3m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea2 (GN887 VER. KBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 2000, ddr3mka,   ddr3mk,   ddr3m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 3rd Mix - Ver.Korea (GN887 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddr3ma,    ddr3mk,   ddr3m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.1 */
GAME( 1999, ddr3mj,    ddr3mk,   ddr3m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 3rd Mix (GN887 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.0 */
GAME( 1999, ddrsbm,    sys573,   ddrsbm,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution Solo Bass Mix (GQ894 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, ddrs2k,    sys573,   ddrs2k,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.3 */
GAME( 1999, ddrs2kj,   ddrs2k,   ddrs2k,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution Solo 2000 (GC905 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.2 */
GAME( 1999, hypbbc2p,  sys573,   hypbbc2p,   hypbbc2p,  ksys573_state, init_hyperbbc, ROT0,  "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, hypbbc2pk, hypbbc2p, hypbbc2p,   hypbbc2p,  ksys573_state, init_hyperbbc, ROT0,  "Konami", "Hyper Bishi Bashi Champ - 2 Player (GX908 1999/08/24 VER. KAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, dsfdct,    sys573,   dsfdct,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JCA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, dsfdcta,   dsfdct,   dsfdcta,    ddr,       ddr_state,     init_ddr,      ROT0,  "Konami", "Dancing Stage featuring Dreams Come True (GC910 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, pcnfrk2m,  sys573,   drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 2nd Mix (GE912 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 2000, pcnfrk2mk, pcnfrk2m, drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 2nd Mix (GE912 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2m,    pcnfrk2m, drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 2nd Mix (GE912 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, drmn2mpu,  pcnfrk2m, drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 2nd Mix Session Power Up Kit (GE912 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.5 */
GAME( 1999, stepchmp,  sys573,   stepchmp,   hyperbbc,  ksys573_state, init_serlamp,  ROT0,  "Konami", "Step Champ (GQ930 VER. JA)", MACHINE_NO_SOUND )
GAME( 2000, dncfrks,   sys573,   dmx,        dmx,       ksys573_state, empty_init,    ROT0,  "Konami", "Dance Freaks (G*874 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmx,       dncfrks,  dmx,        dmx,       ksys573_state, empty_init,    ROT0,  "Konami", "Dance Maniax (G*874 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, dmxa,      dncfrks,  dmx,        dmx,       ksys573_state, empty_init,    ROT0,  "Konami", "Dance Maniax (G*874 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, gunmania,  sys573,   gunmania,   gunmania,  ksys573_state, empty_init,    ROT0,  "Konami", "GunMania (GL906 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAMEL(2000, fghtmn,    sys573,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Fighting Mania (QG918 VER. EAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAMEL(2000, fghtmna,   fghtmn,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Fighting Mania (QG918 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAMEL(2000, pnchmn,    fghtmn,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Punch Mania: Hokuto no Ken (GQ918 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAMEL(2000, pnchmna,   fghtmn,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Punch Mania: Hokuto no Ken (GQ918 VER. JAB ALT CD)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAMEL(2000, fghtmnk,   fghtmn,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Fighting Mania (QG918 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAMEL(2000, fghtmnu,   fghtmn,   pnchmn,     pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Fighting Mania (QG918 VER. UAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAME( 2000, dsem,      sys573,   dsem,       ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dancing Stage Euro Mix (G*936 VER. EAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.7 */
GAME( 2000, ddrkara2,  sys573,   ddrk,       ddrkara,   ddr_state,     init_ddr,      ROT0,  "Konami", "Dance Dance Revolution Karaoke Mix 2 (GQ947 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, gtrfrk3m,  sys573,   gtrfrk3m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAC)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3ma,  gtrfrk3m, gtrfrk3m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 3rd Mix (GE949 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAME( 2000, gtfrk3mb,  gtrfrk3m, gtrfrk5m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 3rd Mix - security cassette versionup (949JAZ02)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.4 */
GAMEL(2000, pnchmn2,   sys573,   pnchmn2,    pnchmn,    pnchmn_state,  init_pnchmn,   ROT0,  "Konami", "Punch Mania 2: Hokuto no Ken (GQA09 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_pnchmn ) /* artwork/network */
GAME( 2000, animechmp, sys573,   animechmp,  hyperbbc,  ksys573_state, init_serlamp,  ROT0,  "Konami", "Anime Champ (GCA07 VER. JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, salarymc,  sys573,   salarymc,   hypbbc2p,  ksys573_state, init_serlamp,  ROT0,  "Success / Konami", "Salary Man Champ - Tatakau Salary Man (GCA18 VER. JAA)", MACHINE_IMPERFECT_SOUND ) // Co-developed? with Praime (sic) Systems https://gdri.smspower.org/wiki/index.php/Prime_System_Development
GAME( 2000, ddr3mp,    sys573,   ddr3mp,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 3rd Mix Plus (G*A22 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.6 */
GAME( 2000, pcnfrk3m,  sys573,   drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, pcnfrk3mk, pcnfrk3m, drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 3rd Mix (G*A23 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, drmn3m,    pcnfrk3m, drmn2m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 3rd Mix (G*A23 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, gtrfrk4m,  sys573,   gtrfrk3m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 4th Mix (G*A24 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4m,     sys573,   ddr3mp,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mj,    ddr4m,    ddr3mp,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix (G*A33 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4ms,    sys573,   ddr4ms,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. ABA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4msj,   ddr4ms,   ddr4ms,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix Solo (G*A33 VER. JBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, dsfdr,     sys573,   dsfdr,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dancing Stage Featuring Disney's Rave (GCA37JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddrusa,    sys573,   ddrusa,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution USA (G*A44 VER. UAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.8 */
GAME( 2000, ddr4mp,    sys573,   ddr3mp,     ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix Plus (G*A34 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, ddr4mps,   sys573,   ddr4ms,     ddrsolo,   ksys573_state, empty_init,    ROT0,  "Konami", "Dance Dance Revolution 4th Mix Plus Solo (G*A34 VER. JBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2000, dmx2m,     sys573,   dmx,        dmx,       ksys573_state, empty_init,    ROT0,  "Konami", "Dance Maniax 2nd Mix (G*A39 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, pcnfrk4m,  sys573,   drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 4th Mix (G*A25 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, pcnfrk4mk, pcnfrk4m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 4th Mix (G*A25 VER. KAA)", MACHINE_IMPERFECT_SOUND ) /* BOOT VER 1.9 */
GAME( 2001, drmn4m,    pcnfrk4m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 4th Mix (G*A25 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk5m,  sys573,   gtrfrk5m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 5th Mix (G*A26 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk5ma, gtrfrk5m, gtrfrk5m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 5th Mix (G*A26 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, ddr5m,     sys573,   ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution 5th Mix (G*A27 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, kicknkick, sys573,   kicknkick,  kicknkick, ksys573_state, empty_init,    ROT0,  "Konami", "Kick & Kick (GNA36 VER. EAA)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, dmx2majp,  sys573,   dmx,        dmx,       ksys573_state, empty_init,    ROT0,  "Konami", "Dance Maniax 2nd Mix Append J-Paradise (G*A38 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, mamboagg,  sys573,   mamboagg,   mamboagg,  ksys573_state, empty_init,    ROT0,  "Konami", "Mambo A Go-Go (GQA40 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, mamboagga, mamboagg, mamboagga,  mamboagg,  ksys573_state, empty_init,    ROT0,  "Konami", "Mambo A Go-Go (GQA40 VER. JRB, Rental)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, pcnfrk5m,  sys573,   drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 5th Mix (G*B05 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, pcnfrk5mk, pcnfrk5m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 5th Mix (G*B05 VER. KAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, drmn5m,    pcnfrk5m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 5th Mix (G*B05 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, gtrfrk6m,  sys573,   gtrfrk5m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 6th Mix (G*B06 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2001, pcnfrk6m,  sys573,   drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 6th Mix (G*B16 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, drmn6m,    pcnfrk6m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 6th Mix (G*B16 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, gtrfrk7m,  sys573,   gtrfrk7m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 7th Mix (G*B17 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2001, ddrmax,    sys573,   ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "DDRMAX - Dance Dance Revolution 6th Mix (G*B19 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, ddrmax2,   sys573,   ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "DDRMAX2 - Dance Dance Revolution 7th Mix (G*B20 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, mrtlbeat,  sys573,   ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Martial Beat (G*B47 VER. JBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, mrtlbeata, mrtlbeat, ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Martial Beat (G*B47 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.9 */
GAME( 2002, gbbchmp,   sys573,   gbbchmp,    hyperbbc,  ksys573_state, init_serlamp,  ROT0,  "Konami", "Great Bishi Bashi Champ (GBA48 VER. JAB)", MACHINE_IMPERFECT_SOUND )
GAME( 2002, pcnfrk7m,  sys573,   drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 7th Mix (G*C07 VER. AAA)", MACHINE_IMPERFECT_SOUND ) /* BOOT VER 1.95 */
GAME( 2002, drmn7m,    pcnfrk7m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 7th Mix power-up ver. (G*C07 VER. JBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, drmn7ma,   pcnfrk7m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 7th Mix (G*C07 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8m,  sys573,   gtrfrk7m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 8th Mix power-up ver. (G*C08 VER. JBA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, gtrfrk8ma, gtrfrk8m, gtrfrk7m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 8th Mix (G*C08 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, dsem2,     sys573,   dsem2,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dancing Stage Euro Mix 2 (G*C23 VER. EAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2002, ddrextrm,  sys573,   ddr5m,      ddr,       ddr_state,     empty_init,    ROT0,  "Konami", "Dance Dance Revolution Extreme (G*C36 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, pcnfrk8m,  sys573,   drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 8th Mix (G*C38 VER. AAA)", MACHINE_IMPERFECT_SOUND ) /* BOOT VER 1.95 */
GAME( 2003, drmn8m,    pcnfrk8m, drmn4m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 8th Mix (G*C38 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtrfrk9m,  sys573,   gtrfrk7m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 9th Mix (G*C39 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, pcnfrk9m,  sys573,   drmn9m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 9th Mix (G*D09 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn9m,    pcnfrk9m, drmn9m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 9th Mix (G*D09 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, drmn9ma,   pcnfrk9m, drmn9m,     drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 9th Mix (G*D09 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10m,  sys573,   gtfrk10m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2003, gtfrk10ma, gtfrk10m, gtfrk10m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 10th Mix (G*D10 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, gtfrk11m,  sys573,   gtfrk11m,   gtrfrks,   ksys573_state, empty_init,    ROT0,  "Konami", "Guitar Freaks 11th Mix (G*D39 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, pcnfrk10m, sys573,   drmn10m,    drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "Percussion Freaks 10th Mix (G*D40 VER. AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
GAME( 2004, drmn10m,   pcnfrk10m,drmn10m,    drmn,      ksys573_state, empty_init,    ROT0,  "Konami", "DrumMania 10th Mix (G*D40 VER. JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) /* BOOT VER 1.95 */
