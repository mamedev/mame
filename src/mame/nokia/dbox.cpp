// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * The Dbox-1 is a DVB Satellite and cable digital television set-top box.
 *
 * The CPU board:
 *__________________________________________________________________________________________________________
 *                                                                                                          |
 *                                                                                                          |
 *                                                         _______________                                  |
 *                ___                                     |               |       ________    ________      |
 *               |   |                                    |_______________|      |        |  |        |     |
 *               |   |                                                           | FLASH  |  | FLASH  |     |
 *               |   |                                                           | 29F800B|  | 29F800B|     |
 *               |   |                                                           |        |  |        |     |
 *               |   |                 ____________                              |  1Mb   |  |  1Mb   |     |
 *               |   |                |            |                             |        |  |        |     |
 *               |___|                |   CPU32    |                             |        |  |        |     |
 *                                    | 68340PV16E |                             |________|  |________|     |
 *                                    |            |                                                        |
 *                                    |            |                                                        |
 *             _______________        |____________|                    _____                               |
 *            |               |                                        |     |                              |
 *            |  SCSI         |                                        |_____|         _____       _____    |
 *            |  CONTROLLER   |        +-+                              _____         |     |     |     |   |
 *            | LSI 53CF54-2  |        |X|                             |     |        |DRAM |     |DRAM |   |
 *            |               |        |T|                             |_____|        |42460|     |42460|   |
 *            |               |        |A|                                            |     |     |     |   |
 *            |               |        |L|                  ----------------          |512Kb|     |512Kb|   |
 *            |_______________|        +-+                  |              |          |     |     |     |   |
 *                                    3.6864                ----------------          |_____|     |_____|   |
 *                                                                                                          |
 *__________________________________________________________________________________________________________|
 *
 * History of Nokia Multimedia Division
 *-------------------------------------
 * Luxor AB was a Swedish home electronics and computer manufacturer located in Motala from 1923 and acquired
 * by Nokia 1985. Luxor designed among other things TV sets, Radios and the famous ABC-80. The Nokia Multimedia
 * Division was formed in Linköping as a result of the Luxor acquisition. Their main design was a satellite
 * receiver, the first satellite in Europe was launched in 1988 and the market was growing fast however it took
 * a long time, almost 10 years before the breakthrough came for Nokia, a deal with the Kirch Gruppe was struck and
 * in 1996 the 68340 based Dbox-1 was released in Germany. The original design was expensive, so soon a cost reduced
 * version based on PPC, the Dbox-2, was released. The boxes sold in millions but the margins were negative or very
 * low at best and the Kirch Gruppe went bankrupt in 2002 and Nokia decided to shutdown the facility in Linköping.
 *
 * The heavily subsidised Dbox was very popular in Holland since Kirch Gruppe didn't lock usage to themselves. This was
 * corrected in a forced firmware upgrade leaving the "customers" in Holland without a working box. Pretty soon a
 * shareware software developed by Uli Hermann appeared called DVB98 and later DVB2000 re-enabling the boxes in  Holland
 * and blocking upgrades. Uli's software was by many considered better than the original software.
 *
 * Misc links about Nokia Multimedia Division and this board:
 * http://www.siliconinvestor.com/readmsg.aspx?msgid=5097482
 * http://www.sat-digest.com/SatXpress/Digital/MM/Mediamaster.htm
 * http://www.telecompaper.com/news/beta-research-publishes-dbox-specifications--163443
 * https://de.wikipedia.org/wiki/D-box
 * http://dvb2000.org/dvb2000/
 *
 * Misc findings
 *--------------
 * - Serial port on the back runs at 19200 and issues modem commands when attached to terminal
 * - It is possible to attach a BDM emulator and retrieve the ROM through it.
 * - It is possible to flash new firmware through BDM by adding jumper XP06 (under the modem board)
 * - The bootstrap is based on RTXC 3.2g RTOS
 * - The bootstrap jumps to firmware from 0xb82 to RAM at 0x800000
 *
 * Identified chips/devices
 *-----------------
 * Motorola 68340 CPU
 * Philips SAA7124 Digital Video Encoder
 * LSI 53CF54-2 SCSI controller
 * Crystal CL4922-CL Mpeg Audio Decoder System
 * Rockwell R6653-16 Single Device Data/Fax Modem Data Pump
 * Philips 8020401TM100 <modem related, unknown purpose>
 * C-cube CL9100 MPEG2 decoder
 * Lucent AV6220A MPEG2 demultiplexer w. crypto interface
 * CI Common Interface module
 * LSI L2A0371 Tuner
 * 2 x 29F800B-90 (2Mb FLASH) - schematics shows a 29F400 as second device so firmware checks device ID
 * 2 x 42260-60  (1Mb DRAM)
 * Siemens SDA5708 dot matrix display, SPI like connection
 *  - http://arduinotehniq.blogspot.se/2015/07/sda5708-display-8-character-7x5-dot.html
 *  - charset stored at 0x808404 to 0x808780, 7 bytes per character
 *
 * Known Nokia Receivers
 * -----------------------------
 * D-box SCART, built-in Irdeto CAM, modem, SCSI
 * 9200 SCART, SCSI
 * 9500 SCART, built-in Irdeto CAM, modem, SCSI
 * 9600 SCART
 * 9602 SCART, modem
 * 9610 SCART, modem, SCSI
 * 8200 RF, SCSI
 * 8500 RF, built-in Irdeto CAM, modem, SCSI
 * 8600 RF
 *
 * Known board revs and changes
 * -----------------------------
 * Main Board 55 31893-11 DVB 9500 S
 * Main Board 55 31893-33 DVB 9500 S - GALs named after functions in schematics
 * Main Board 55 31893-46 D-box      - OE* IP09 tied to GND, only one flash IP02
 * Main Board 55 31893-89 DVB 9500 S - OE* IP09 tied to GND
 *
 * Address Map
 * --------------------------------------------------------------------------
 * Address Range     Memory Space (physical)   Notes
 * --------------------------------------------------------------------------
 * 0xffffffff                                  (top of memory)
 * 0x00FFF780-0x00FFF7BF DMA                   offset to SIM40
 * 0x00FFF700-0x00FFF721 Serial devices        offset to SIM40
 * 0x00FFF600-0x00FFF67F Timers                offset to SIM40
 * 0x00FFF000-0x00FFF07F SIM40                 programmed base adress (MCR)
 * 0x00700000-0x0077ffff RAM
 * 0x00780000-0x007807ff I/O area
 * 0x00800000-0x008fffff RAM
 * 0x00000000-0x0001ffff bootstrap
 * --------------------------------------------------------------------------
 *
 * Init sequence
 * -------------
 *  MCR           : 0x6301     Timer/wd disabled, show cycles, ext arbit,
 *                             user access to SIM40, IARB = 1
 *  MBAR          : 0x00FFF101 SIM40 base = 0x00fff000
 *  VBR           : 0x008096F8 VBR - Vector Base Register
 *  SIM40 + 0x0006: 0xE8       AVR - Auto Vector Register
 *  SIM40 + 0x0021: 0x0C       SWIV_SYPCR
 *  SIM40 + 0x001F: 0x00       PPARB - Port B pin assignment
 *  SIM40 + 0x0011: 0xFF       PORTA - Port A Data
 *  SIM40 + 0x0013: 0x16       DDRA  - Port A Data direction
 *  SIM40 + 0x0700: 0x00       Serial port
 *  SIM40 + 0x071F: 0xFF       Serial port
 *  SIM40 + 0x0004: 0x7F03     SYNCR
 *  SIM40 + 0x0040: 0x003FFFF5 CS0 mask 1 - block size = 4194304 (4MB)
 *  SIM40 + 0x0044: 0x00000003 CS0 base 1 - base addr = 0x000000
 *  SIM40 + 0x0048: 0x003FFFF5 CS1 mask 1 - block size = 4194304 (4MB)
 *  SIM40 + 0x004C: 0x00800003 CS1 base 1 - base addr = 0x800000
 *  SIM40 + 0x0050: 0x00007FFF CS2 mask 1 - block size = 65536 (64KB)
 *  SIM40 + 0x0054: 0x00700003 CS2 base 1
 *  SIM40 + 0x0058: 0x000007F2 CS3 mask 1 - block size = 2048 (2KB)
 *  SIM40 + 0x005C: 0x00780003 CS3 base 1
 *  SIM40 + 0x001F: 0x40       PBARB Port B Pin Assignment
 *  -------------------------------------------------------------
 *  The bootstrap copies the firmware to RAM and jumps to it
 *  -------------------------------------------------------------
 *
 *  --- PC > 0x700000
 *  SIM40 + 0x0022: 0x0140     PICR Periodic Interrupt Control Register
 *  SIM40 + 0x0024: 0x0029     PICR Periodic Interrupt Timer Register
 *
 *  Serial port setup
 * ------------------
 *  --- PC < 0x1FFFF so bootstrap code
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x071F: 0xFF     Serial Module - OUTPUT PORT (OP)4 BIT RESET - all cleared
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x0701: 0x8A     Serial Module - MCR Low Byte
 *                            - The serial control registers are only accessible from supervisor mode
 *                            - IARB = 0x0A - serial module has priority level 10d
 *  SIM40 + 0x0704: 0x01     Serial Module - ILR Interrupt Level
 *  SIM40 + 0x0705: 0x44     Serial Module - IVR Interrupt Vector
 *  SIM40 + 0x0714: 0x80     Serial Module - ACR Auxiliary Control
 *                                           - Set 2 of the available baud rates is selected.
 *                                           - CTS state change has no effect
 *  --- setup Channel A
 *  SIM40 + 0x0712: 0x20     Serial Module - CRA Command Register A
 *                                           - Reset the receiver
 *  SIM40 + 0x0712: 0x30     Serial Module - CRA Command Register A
 *                                           - Reset the transmitter
 *  SIM40 + 0x0710: 0x93     Serial Module - MR1A Mode Register 1A
 *                                           - Upon receipt of a valid start bit, RTS≈ is negated if the channel's FIFO is full.
 *                                             RTS≈ is reasserted when the FIFO has an empty position available.
 *                                           - No Parity
 *                                           - Eight bits
 *  SIM40 + 0x0720: 0x07     Serial Module - MR2A Mode Register 2A
 *                                           - No CTS or RTS controll
 *                                           - 1 STOP bit
 *  SIM40 + 0x0711: 0xCC     Serial Module - CSRA Clock Select Register A
 *                                           - 19200 baud TXc
 *                                           - 19200 baud Rxc
 *  SIM40 + 0x0712: 0x41     Serial Module - CRA Command Register A
 *                                           - Reset Error status
 *                                           - Enable Receiver
 *  - Check for charcters in channel A
 *  SIM40 + 0x0711: btst #0  Serial Module - SRA Status Register A
 *  --- if there is
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- setup Channel B (See details as for channel A above)
 *  SIM40 + 0x071A: 0x20     Serial Module - CRB Command Register B
 *  SIM40 + 0x071A: 0x30     Serial Module - CRB Command Register B
 *  SIM40 + 0x0718: 0x93     Serial Module - MR1B Mode Register 1B
 *  SIM40 + 0x0721: 0x07     Serial Module - MR2B Mode Register 2B
 *  SIM40 + 0x0719: 0xCC     Serial Module - CSRB Clock Select Register B
 *  SIM40 + 0x071A: 0x41     Serial Module - CRB Command Register B
 *  - Check for characters in channel B
 *  SIM40 + 0x0719: btst #0  Serial Module - SRB Status Register B
 *  --- if there is
 *        store all characters in buffer at (A6) 0x88FFD0
 *  ---
 *  - Check bit 0 set on Input Port
 *  SIM40 + 0x071D: btst #0  Input Port - IP
 *  --- if bit 0 is set
 *      0x00801208: 0x80
 *  SIM40 + 0x071A: 0x81     Serial Module - CRB Command Register B
 *  ---
 *  SIM40 + 0x0720: 0x41     Serial Module - MR2A Mode register 2A
 *  SIM40 + 0x071D: 0x03     OPCR Output Port Control Register
 *  SIM40 + 0x0715: 0x03     IER Interrupt Enable Register
 *
 *  Timer setup
 * ------------
 *  SIM40 + 0x0640: 0x03     MCR Timer 2
 *  tbc...
 *
 *  -------------------------------------------------------------
 *  The bootstrap copies the firmware to RAM and jumps to it
 *  -------------------------------------------------------------
 *
 *  --- PC > 0x700000 so probably init of the RTXC tick timer setup?!
 *  SIM40 + 0x0022: 0x0140     PICR Periodic Interrupt Control Register
 *  SIM40 + 0x0024: 0x0029     PICR Periodic Interrupt Timer Register
 *
 *  Serial port setup
 * ------------------
 *  --- PC < 0x1FFFF so bootstrap code
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x071F: 0xFF     Serial Module - OUTPUT PORT (OP)4 BIT RESET - all cleared
 *  SIM40 + 0x0700: 0x00     Serial Module - MCR High Byte
 *                            - The serial module is enabled
 *                            - ignore FREEZE
 *                            - The crystal clock is the clear-to-send input capture clock for both channels
 *  SIM40 + 0x0701: 0x8A     Serial Module - MCR Low Byte
 *                            - The serial control registers are only accessible from supervisor mode
 *                            - IARB = 0x0A - serial module has priority level 10d
 *  SIM40 + 0x0704: 0x01     Serial Module - ILR Interrupt Level
 *  SIM40 + 0x0705: 0x44     Serial Module - IVR Interrupt Vector
 *  SIM40 + 0x0714: 0x80     Serial Module - ACR Auxiliary Control
 *                                           - Set 2 of the available baud rates is selected.
 *                                           - CTS state change has no effect
 *  --- setup Channel A
 *  SIM40 + 0x0712: 0x20     Serial Module - CRA Command Register A
 *                                           - Reset the receiver
 *  SIM40 + 0x0712: 0x30     Serial Module - CRA Command Register A
 *                                           - Reset the transmitter
 *  SIM40 + 0x0710: 0x93     Serial Module - MR1A Mode Register 1A
 *                                           - Upon receipt of a valid start bit, RTS≈ is negated if the channel's FIFO is full.
 *                                             RTS≈ is reasserted when the FIFO has an empty position available.
 *                                           - No Parity
 *                                           - Eight bits
 *  SIM40 + 0x0720: 0x07     Serial Module - MR2A Mode Register 2A
 *                                           - No CTS or RTS control
 *                                           - 1 STOP bit
 *  SIM40 + 0x0711: 0xCC     Serial Module - CSRA Clock Select Register A
 *                                           - 19200 baud TXc
 *                                           - 19200 baud Rxc
 *  SIM40 + 0x0712: 0x41     Serial Module - CRA Command Register A
 *                                           - Reset Error status
 *                                           - Enable Receiver
 *  - Check for charcters in channel A
 *  SIM40 + 0x0711: btst #0  Serial Module - SRA Status Register A
 *  --- if there is
 *        store all characters in buffer at (A6) 0x88FFD0
 *  --- setup Channel B (See details as for channel A above)
 *  SIM40 + 0x071A: 0x20     Serial Module - CRB Command Register B
 *  SIM40 + 0x071A: 0x30     Serial Module - CRB Command Register B
 *  SIM40 + 0x0718: 0x93     Serial Module - MR1B Mode Register 1B
 *  SIM40 + 0x0721: 0x07     Serial Module - MR2B Mode Register 2B
 *  SIM40 + 0x0719: 0xCC     Serial Module - CSRB Clock Select Register B
 *  SIM40 + 0x071A: 0x41     Serial Module - CRB Command Register B
 *  - Check for characters in channel B
 *  SIM40 + 0x0719: btst #0  Serial Module - SRB Status Register B
 *  --- if there is
 *        store all characters in buffer at (A6) 0x88FFD0
 *  ---
 *  - Check bit 0 set on Input Port
 *  SIM40 + 0x071D: btst #0  Input Port - IP
 *  --- if bit 0 is set
 *      0x00801208: 0x80
 *  SIM40 + 0x071A: 0x81     Serial Module - CRB Command Register B
 *  ---
 *  SIM40 + 0x0720: 0x41     Serial Module - MR2A Mode register 2A
 *  SIM40 + 0x071D: 0x03     OPCR Output Port Control Register
 *  SIM40 + 0x0715: 0x03     IER Interrupt Enable Register
 *
 *  Timer setup
 * ------------
 *  SIM40 + 0x0640: 0x03     MCR Timer 2
 *  tbc...
 *
 * // Tricks with the CS0 and the GAL:s
 * 008004d8 SIM40 + 0x0044: 0x00000053 CS0 base 1 - base addr = 0x000000, Supervisor Data Space, No CPU Space, Valid CS
 * 008004e2 SIM40 + 0x0040: 0x003FFF05 CS0 mask 1 - block size = 4194304 (4MB),
 * ... strange series of operations between 800834 and 8008D4, suspecting GAL:s to be involved in some magic here
 * 008004ee SIM40 + 0x0040: 0x003FFFF5 CS0 mask 1 - block size = 4194304 (4MB), Mask all accesses
 * 008004f8 SIM40 + 0x0044: 0x0000005b CS0 base 1 - base addr = 0x000000, Supervisor Data Space, Write Protect, No CPU Space, Valid CS
 *
 *  LED Dot Matrix Display hookup
 *  -------------------------------------
 *  "DISPLAY CONN ALT" connected to the SDA5708
 *  pin signal   connected to
 *   1   VCC      +5v
 *   2   LOAD1*   PA2 68340 pin 121 IP01
 *   3   DATA      D8 68340 pin 134 IP01
 *   4   SDCLK     Q6 74138 pin 9   IP12
 *   5   RESET*    Q4 74259 pin 9   IP16
 *   6   GND      GND
 *
 *  IP12 74138 hookup
 *  pin signal   connected to
 *   1   A0       A8 68340 pin 46  IP01
 *   2   A1       A9 68340 pin 47  IP01
 *   3   A2      A10 68340 pin 48  IP01
 *   4   E1*     CS3 68340 pin  5  IP01
 *   5   E2*     CS3 68340 pin  5  IP01
 *   6   E3      +5v
 *   9   Q6    SDCLK SDA5708 pin 4
 *  14   Q1  Enable* 74259 pin 14  IP16
 *
 *  IP16 74259 hookup
 *  pin signal   connected to
 *   1   A0       A0 68340 pin 113 IP01
 *   2   A1       A1 68340 pin  37 IP01
 *   3   A2       A2 68340 pin  38 IP01
 *   9   Q4   Reset* SDA5708 pin 5
 *  13   D        D8 68340 pin 134 IP01
 *  14   Enable*  Q1 74138 pin  14 IP12
 *  15   Clear*   System reset
 *
 *  Address map decoding
 *  --------------------
 *  IP06 GAL16V8 - DRAM-PS8V0.9
 *  pin signal   connected to
 *   1   Q       V-FIFO-CLK inverted SCSI_CLK, origin to be found
 *   2   I1      SCSI_CLK
 *   3   I2      A0   68340 pin 113 IP01
 *   4   I3      CS1  68340 pin   2 IP01
 *   5   I4      A19  68340 pin  62 IP01
 *   6   I5      A21  68340 pin  64 IP01
 *   7   I6      BG   68340 pin 101 IP01
 *   8   I7      SIZ0 68340 pin 105 IP01
 *   9   I8      R/W  68340 pin 107 IP01
 *  11   I9/OE*  GND  via an 100R resistor
 *  12   O0      FC2  68340 pin  71 IP01
 *  13   O1      I8   GAL168V pin 9 IP07 SCSI GAL
 *  14   O2      nc
 *  15   O3      RAS1 514260 pin 14 IP10-11 512KB DRAM each (IP11 is empty socket)
 *  16   O4      LCAS 514260 pin 29 IP09-11 512KB DRAM each (IP11 is empty socket)
 *  17   O5      UCAS 514260 pin 28 IP09-11 512KB DRAM each (IP11 is empty socket)
 *  18   O6      RAS0 514260 pin 14 IP09    512KB DRAM
 *  19   O7      I2 GAL16V8 pin 3 IP07 SCSI GAL (ANDed with !CS1) DMA_REQ
 *              +I1 GAL16V8 pin 2 IP08 LOGC GAL (ANDed with !CS1) DMA_REQ
 *              +G1 74257 pin 1 IP04-05 MUXes
 *
 *  IP07 GAL16V8 - SCSI-PS8V0.8
 *  pin signal   connected to
 *   1   Q       V-FIFO-CLK inverted SCSI_CLK, origin to be found
 *   2   I1      R/W  68340 pin 107 IP01
 *   3   I2      DMA_REQ
 *   4   I3      Q    7474  pin   5 IP25
 *   5   I4      DACK1 68340 pin 15
 *   6   I5      D0   74138 pin  15 IP12
 *   7   I6      !BG  7404<-68340 pin 101 IP01 Inverted Bus Grant
 *   8   I7      3    7408 pin 3 IP23 OE for IP10
 *   9   I8      O1   GAL16V pin 18 IP06 DRAM GAL
 *  11   I9/OE*  GND via an 100R resistor
 *  12   O0      DREQ1 68340 pin 16
 *  13   O1      WE   514260 pin 13 IP10 512KB DRAM
 *  14   O2      DMA_ACK
 *  15   O3      CS_SCSI
 *  16   O4      RD_SCSI
 *  17   O5      WE   514260 pin 13 IP11 empty socket
 *  18   O6      WR_SCSI
 *  19   O7      MWE
 *              +WE   514260 pin 13 IP09 512KB DRAM
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description               Device  Lvl  IRQ
 *                           /Board      Vector
 * ----------------------------------------------------------
 *  IRQ7 CTRL20 IP19 circuit
 *  IRQ6 SCSI/DEMUX INT
 *  IRQ5 CAM module INT
 *  IRQ3 Audio/Video INT
 *
 *  TODO:
 *  - Dump/understand the address decoder GAL:s IP06-IP08 (3 x 16V8)
 *  - Fix debug terminal
 *  - write demuxer
 *
 ****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "machine/68340.h"
#include "machine/74259.h"
#include "machine/intelfsh.h"
#include "video/sda5708.h"

#include "sda5708.lh"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_SETUP   (1U << 1)
#define LOG_DISPLAY (1U << 2)
#define LOG_FLASH   (1U << 3)

#define VERBOSE  (LOG_FLASH)
#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGDISPLAY(...) LOGMASKED(LOG_DISPLAY, __VA_ARGS__)
#define LOGFLASH(...)  LOGMASKED(LOG_FLASH,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

#define LOCALFLASH 0 //  1 = local flash rom implementation 0 = intelflash_device

class dbox_state : public driver_device
{
public:
	dbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_display(*this, "display")
		, m_ip16_74259(*this, "hct259.ip16")
	{ }

	void dbox(machine_config &config);

	void init_dbox();

private:
	required_device<m68340_cpu_device> m_maincpu;
	required_device<sda5708_device> m_display;
	required_device<hct259_device> m_ip16_74259;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void sda5708_reset(uint8_t data);
	void sda5708_clk(uint8_t data);
	void write_pa(uint8_t data);

	void dbox_map(address_map &map) ATTR_COLD;

#if LOCALFLASH
	uint16_t sysflash_r(offs_t offset);
	void sysflash_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
private:
	uint16_t * m_sysflash = nullptr;
	uint32_t m_sf_mode = 0;
	uint32_t m_sf_state = 0;
#endif
};

void dbox_state::machine_start()
{
	LOG("%s\n", FUNCNAME);

#if LOCALFLASH
	save_pointer (NAME (m_sysflash), sizeof(m_sysflash));

	m_sysflash = (uint16_t*)(memregion ("flash0")->base());
	m_sf_mode  = 0;
	m_sf_state = 0;
#endif
}

void dbox_state::machine_reset()
{
	LOG("%s\n", FUNCNAME);
	m_maincpu->tgate2_w(ASSERT_LINE); // Pulled high by resistor when not used as Rx from descrambler device
}

// TODO: Hookup the reset latch correctly
void dbox_state::sda5708_reset(uint8_t data) {
	LOGDISPLAY("%s - not implemented\n", FUNCNAME);
}

void dbox_state::sda5708_clk(uint8_t data) {
	LOGDISPLAY("%s\n", FUNCNAME);
	m_display->sdclk_w(CLEAR_LINE);
	m_display->data_w((0x80 & data) != 0 ? ASSERT_LINE : CLEAR_LINE);
	m_display->sdclk_w(ASSERT_LINE);
}

void dbox_state::write_pa(uint8_t data) {
	LOGDISPLAY("%s\n", FUNCNAME);
	m_display->load_w((0x04 & data) == 0 ? ASSERT_LINE : CLEAR_LINE);
}

#if LOCALFLASH
/* Local emulation of the 29F800B 8Mbit flashes if the intelflsh bugs, relies on a complete command cycle is done per device, not in parallel */
/* TODO: Make a flash device of this and support programming per sector and persistence, as settings etc may be stored in a 8Kb sector  */
void dbox_state::sysflash_w(offs_t offset, uint16_t data, uint16_t mem_mask) {
	LOGFLASH("%s pc:%08x offset:%08x data:%08x mask:%08x\n", FUNCNAME, m_maincpu->pc(), offset, data, mem_mask);

	/*Data bits DQ15–DQ8 are don’t cares for unlock and command cycles.*/
	m_sf_state = ((m_sf_state << 8) & 0xffffff00) | (data & 0xff);
	switch (m_sf_state)
	{
	case 0xf0:// Reset command, to get back to reading flash data
		m_sf_mode = 0;
		m_sf_state = 0;
		LOGFLASH("- Reset command\n");
		break;
	case 0xaa: // Building a multi byte command
		m_sf_mode = 1;
		break;
	case 0xaa55: // Building a multi byte command
	case 0xaa55a0: // Program Data
	case 0xaa5580: // Erase
	case 0xaa5580aa: // Chip or Sector Erase
		break;
	case 0xaa5590: // Autoselect mode
		m_sf_mode = 4;
		m_sf_state = 0;
		LOGFLASH("- Autoselect Mode\n");
		break;
	case 0xb0: // Erase Suspend Mode
		m_sf_mode = 2;
		m_sf_state = 0;
		LOGFLASH("- Erase Suspend Mode\n");
		break;
	case 0x30: // Erase Resume Mode
		m_sf_mode = 3;
		m_sf_state = 0;
		LOGFLASH("- Erase Resume Mode\n");
		break;
	}
}

uint16_t dbox_state::sysflash_r(offs_t offset) {

	if (m_sf_mode == 0)
	{
		return m_sysflash[offset];
	}
	else
	{
		if (m_sf_mode == 4)
		{
			switch (offset & 0xff)
			{
			case 0x00: LOGFLASH("- Manufacturer ID\n"); return 01; break; // Manufacturer ID
			//case 0x01: LOGFLASH("- Device ID\n"); return 0x22d6; break; // Device ID (Top Boot Block) 29F800TA
			case 0x01: LOGFLASH("- Device ID\n"); return 0x2258; break; // Device ID (Bottom Boot Block) 29F800BA
			case 0x02: LOGFLASH("- Sector %02x protection: 1 (hardcoded)\n", offset >> 12); return 01; break;
			default:   LOGFLASH(" - Unhandled Mode:%d State:%08x\n", m_sf_mode, m_sf_state);
			}
		}
	}
	return 0;
}
/* End of flash emulation */
#endif

void dbox_state::dbox_map(address_map &map)
{
	// CS0 - bootrom
	// 008004ee Address mask CS0 00000040, 003ffff5 (ffffffff) - Mask: 003fff00 FCM:0f DD:1 PS: 16-Bit
	// 008004f8 Base address CS0 00000044, 0000005b (ffffffff) - Base: 00000000 BFC:05 WP:1 FTE:0 NCS:1 Valid: Yes
#if LOCALFLASH
	map(0x000000, 0x3fffff).rom().r(FUNC(dbox_state::sysflash_r)).region("flash0", 0);
	map(0x000000, 0x3fffff).w(FUNC(dbox_state::sysflash_w));
#else
	map(0x000000, 0x0fffff).rw("flash0", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x100000, 0x1fffff).rw("flash1", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
#endif
	// CS2 - CS demux
	// 0000009a Address mask CS2 00000050, 00007fff (ffffffff) - Mask: 00007f00 FCM:0f DD:3 PS: External DSACK response
	// 000000a2 Base address CS2 00000054, 00700003 (ffffffff) - Base: 00700000 BFC:00 WP:0 FTE:0 NCS:1 Valid: Yes
	//map(0x700000, 0x77ffff)
	// CS3 - 8 bit devices
	// 000000aa Address mask CS3 00000058, 000007f2 (ffffffff) - Mask: 00000700 FCM:0f DD:0 PS: 8-bit
	// 000000b2 Base address CS3 0000005c, 00780003 (ffffffff) - Base: 00780000 BFC:00 WP:0 FTE:0 NCS:1 Valid: Yes
	// map(0x780000, 0x7807ff)
	map(0x780100, 0x7801ff).w(FUNC(dbox_state::sda5708_reset));
	map(0x780600, 0x7806ff).w(FUNC(dbox_state::sda5708_clk));
	// CS1 - RAM area
	// 0000008a Address mask CS1 00000048, 003ffff5 (ffffffff) - Mask: 003fff00 FCM:0f DD:1 PS: 16-Bit
	// 00000092 Base address CS1 0000004c, 00800003 (ffffffff) - Base: 00800000 BFC:00 WP:0 FTE:0 NCS:1 Valid: Yes
	map(0x800000, 0xcfffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( dbox )
INPUT_PORTS_END

void dbox_state::dbox(machine_config &config)
{
	M68340(config, m_maincpu, 0);       // The 68340 has an internal VCO as clock source, hence need no CPU clock
	m_maincpu->set_crystal(XTAL(32'768)); // The dbox uses the VCO and has a crystal as VCO reference and to synthesize internal clocks from
	m_maincpu->set_addrmap(AS_PROGRAM, &dbox_state::dbox_map);
	m_maincpu->pa_out_callback().set(FUNC(dbox_state::write_pa));

	/* Timer 2 is used to communicate with the descrambler module TODO: Write the descrambler module */
	//m_maincpu->tout2_out_callback().set("dcs", FUNC(descrambler_device::txd_receiver));
	//m_maincpu->tgate2_in_callback().set("dsc", FUNC(descrambler_device::rxd_receiver));

	/* Configure the serial ports */
	subdevice<mc68340_serial_module_device>("maincpu:serial")->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	subdevice<mc68340_serial_module_device>("maincpu:serial")->b_tx_cb().set("modem", FUNC(rs232_port_device::write_txd));
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("maincpu:serial", FUNC(mc68340_serial_module_device::rx_a_w));
	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	modem.rxd_handler().set("maincpu:serial", FUNC(mc68340_serial_module_device::rx_b_w));

	/* Add the boot flash */
	AMD_29F800B_16BIT(config, "flash0");
	AMD_29F800B_16BIT(config, "flash1");

	/* LED Matrix Display */
	SDA5708(config, m_display, 0);
	config.set_default_layout(layout_sda5708);

	/* IP16 74259 8 bit latch */
	HCT259(config, m_ip16_74259);
	m_ip16_74259->q_out_cb<4>().set("display", FUNC(sda5708_device::reset_w));
}

void dbox_state::init_dbox()
{
}

// TODO: Figure out correct ROM address map
// TODO: Figure out what DVB2000 is doing
ROM_START( dbox )
	ROM_REGION16_BE(0x400000, "flash0", ROMREGION_ERASEFF) // should be 0x100000
	ROM_DEFAULT_BIOS("b200uns")

	ROM_SYSTEM_BIOS(0, "b200uns", "Nokia Bootloader B200uns")
	ROMX_LOAD( "b200uns.bin",   0x000000, 0x020000, CRC(0ff53e1f) SHA1(52002ee22c032775dac383d408c44abe9244724f), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "b210uns", "Nokia Bootloader B210uns")
	ROMX_LOAD( "b210uns.bin",   0x000000, 0x020000, CRC(e8de221c) SHA1(db6e20ae73b11e8051f389968803732bd73fc1e4), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "nbc106.bin", "Nokia Bootloader CI v1.06")
	ROMX_LOAD( "bootci106.bin", 0x000000, 0x020000, BAD_DUMP CRC(641762a9) SHA1(7c5233390cc66d3ddf4c730a3418ccfba1dc2905), ROM_BIOS(2) )
ROM_END

} // anonymous namespace


COMP( 1996, dbox, 0, 0, dbox, dbox, dbox_state, init_dbox, "Nokia Multimedia", "D-box 1 (Kirch-Gruppe)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
