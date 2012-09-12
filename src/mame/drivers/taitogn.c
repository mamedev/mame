/*

GNET Motherboard
Taito, 1998

The Taito GNET System comprises the following main parts....
- Sony ZN-2 Motherboard (Main CPU/GPU/SPU, RAM, BIOS, EEPROM & peripheral interfaces)
- Taito FC PCB (Sound hardware & FLASHROMs for storage of PCMCIA cart contents)
- Taito CD PCB (PCMCIA cart interface)

Also available are...
- Optional Communication Interface PCB
- Optional Save PCB

On power-up, the system checks for a PCMCIA cart. If the cart matches the contents of the flashROMs,
the game boots immediately with no delay. If the cart doesn't match, it re-flashes the flashROMs with _some_
of the information contained in the cart, which takes approximately 2-3 minutes. The game then resets
and boots up.

If no cart is present on power-up, the Taito GNET logo is displayed, then a message 'SYSTEM ERROR'
Since the logo is shown on boot even without a cart, there must be another sub-BIOS for the initial booting,
which I suspect is one of the flashROMs that is acting like a standard ROM and is not flashed at all.
Upon inspecting the GNET top board, it appears flash.u30 is the sub-BIOS and perhaps U27 is something sound related.
The flashROMs at U55, U56 & U29 appear to be the ones that are re-flashed when swapping game carts.

PCB Layouts
-----------
(Standard ZN2 Motherboard)

ZN-2 COH-3000 (sticker says COH-3002T denoting Taito GNET BIOS version)
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       S301           COH3002T.353                      |
|                                                        |
|                                                        |
|J                                                       |
|                                                        |
|A              814260    CXD2925Q     EPM7064           |
|                                                        |
|M                                     67.73MHz          |
|                                                        |
|M                                                       |
|            S551    KM4132G271BQ-8                      |
|A                                                       |
|                                CXD8654Q    CXD8661R    |
|                    KM4132G271BQ-8                      |
|CN505  CN506                   53.693MHz    100MHz      |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|            MC44200FT                                   |
|  NEC_78081G503        KM416V1204BT-L5  KM416V1204BT-L5 |
|                                                        |
|CN651  CN652                 *                 *        |
|                CN654                                   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      S301  - Slide switch for stereo or mono sound output
      S551  - Dip switch (4 position, defaults all OFF)

      COH3002T.353   - GNET BIOS 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM
      814260-70      - 256K x16 (4MBit) DRAM
      KM4132G271BQ-8 - 128K x 32Bit x 2 Banks SGRAM
      KM416V1204BT-L5- 1M x16 EDO DRAM
      EPM7064        - Altera EPM7064QC100 CPLD (QFP100)
      CAT702         - Protection chip labelled 'TT10' (DIP20)
      *              - Unpopulated position for additional KM416V1204BT-L5 RAMs


FC PCB  K91X0721B  M43X0337B
|--------------------------------------------|
|   |---------------------------|            |
|   |---------------------------|            |
| NJM2100  NJM2100                           |
| MB87078                                    |
| *MB3773     XC95108         DIP40   CAT702 |
| *ADM708AR                                  |
| *UPD6379GR                                 |
|             FLASH.U30                      |
|                                            |
| DIP24                                      |
|                  *RF5C296                  |
| -------CD-PCB------- _                     |
| |                   | |                    |
| |                   | |                    |
| |                   | |                    |
| |                   | |                    |
| |                   | |                    |
| |                   | |                    |
| |                   | |                    |
| |                   |-|                    |
| --------------------                       |
|          M66220FP   FLASH.U55   FLASH16.U29|
|      FLASH.U27             FLASH.U56       |
|*LC321664                                   |
| TMS57002DPHA                *ZSG-2         |
|           LH52B256      25MHz              |
|   MN1020012A                               |
|--------------------------------------------|
Notes:
      DIP40           - Unpopulated socket for 8MBit DIP40 EPROM type AM27C800
      DIP24           - Unpopulated position for FM1208 DIP24 IC
      FLASH.U30       - Intel TE28F160 16MBit FLASHROM (TSOP56)
      FLASH.U29/55/56 - Intel TE28F160 16MBit FLASHROM (TSOP56)
      FLASH.U27       - Intel E28F400 4MBit FLASHROM (TSOP48)
      LH52B256        - Sharp 32K x8 SRAM (SOP28)
      LC321664        - Sanyo 64K x16 EDO DRAM (SOP40)
      XC95108         - XILINX XC95108 CPLD labelled 'E65-01' (QFP100)
      MN1020012A      - Panasonic MN1020012A Sound CPU (QFP128)
      ZSG-2           - Zoom Corp ZSG-2 Sound DSP (QFP100)
      TMS57002DPHA    - Texas Instruments TMS57002DPHA Sound DSP (QFP80)
      RF5C296         - Ricoh RF5C296 PCMCIA controller (TQFP144)
      M66220FP        - 256 x8bit Mail-Box (Inter-MPU data transfer)
      CAT702          - Protection chip labelled 'TT16' (DIP20)
      CD PCB          - A PCMCIA cart slot connector mounted onto a small daughterboard
      *               - These parts located under the PCB


Taito G-Net card info
---------------------

The G-Net system uses a custom PCMCIA card for game software storage. The card is
locked with a password and can't be read by conventional means.
Some of the cards are made in separate pieces and can be opened. However some
are encased in a single-piece steel shell and opening it up destroys the card.
Some of the later games came packaged as a Compact Flash card and a PCMCIA to CF
adapter, however these cards were also locked the same as the older type.
The game uses an analog wheel (5k potentiometer) in the shape of a hand-held
control unit and a trigger (another 5k potentiometer) used for acceleration
and brake. The trigger and wheel are self centering. If the trigger is pulled back
(like firing a gun) the car goes faster. If the trigger is pushed forward the car
slows down. The controller looks a lot like the old Scalextric controllers (remember those?  :-)

The controller is connected to the ZN2 main board to the 10 pin connector labelled
'ANALOG'. Using two 5k-Ohm potentiometers, power (+5V) and ground are taken from the JAMMA
edge connector or directly from the power supply. The output of the steering pot is
connected to pin 2 and the output of the acceleration pot is connected to pin 3.
The ANALOG connector output pins are tied directly to a chip next to the connector marked
'NEC 78081G503 9810KX189'. This is a NEC 8-bit 78K0-family microcontroller with on-chip 8k ROM,
256 bytes RAM, 33 I/O ports, 8-bit resolution 8-channel A/D converter, 3-channel timer, 1-channel
3-wire serial interface interrupt control (USART) and other peripheral hardware.


Card PCB Layouts
----------------

Type 1 (standard 'Taito' type, as found on most G-Net games)
------ (This type has separate top and bottom pieces which are glued together and
       (can be opened if done 'carefully'. But be careful the edges of the top lid are SHARP!)

Top
---

RO-055A AI AM-1
|-|-------------------------------------|
| |        |---------|     |--------|   |
| |        |S2812A150|     |CXK58257|   |
| |        |---------|     |--------|   |
| |                                     |
| |        |-------|     |-----------|  |
| |        |       |     |           |  |
| |        |ML-101 |     |           |  |
| |        |       |     |  F1PACK   |  |
| |        |-------|     |           |  |
| |                      |           |  |
| |        |-----|       |-----------|  |
| |        |ROM1 |                      |
| |        |-----|                      |
| |                                     |
|-|-------------------------------------|
Notes:
      F1PACK    - TEL F1PACK(tm) TE6350B 9744 E0B (TQFP176)
      S2812A150 - Seiko Instruments 2k x8 parallel EEPROM (TSOP28)
      ML-101    - ML-101 24942-6420 9833 Z03 JAPAN (TQFP100, NOTE! This chip looks like it is Fujitsu-manufactured)
      CXK58257  - SONY CXK58257 32k x8 SRAM (TSOP28)
      ROM1      - TOSHIBA TC58V32FT 4M x8 (32MBit) CMOS NAND Flash EEPROM 3.3Volt (TSOP44)
                  The ROMs use a non-standard format and can not be read by conventional methods.

Bottom
------

|-|-------------------------------------|
| |                                     |
| |  |-----|      |-----|     |-----|   |
| |  |ROM2 |      |ROM3 |     |ROM4 |   |
| |  |-----|      |-----|     |-----|   |
| |                                     |
| |                                     |
| |  |-----|      |-----|     |-----|   |
| |  |ROM5 |      |ROM6 |     |ROM7 |   |
| |  |-----|      |-----|     |-----|   |
| |                                     |
| |                                     |
| |  |-----|      |-----|     |-----|   |
| |  |ROM8 |      |ROM9 |     |ROM10|   |
| |  |-----|      |-----|     |-----|   |
| |                                     |
|-|-------------------------------------|
Notes:
      ROM2-10   - TOSHIBA TC58V32FT 4M x8 (32MBit) CMOS NAND Flash EEPROM 3.3Volt (TSOP44)
                  The ROMs use a non-standard format and can not be read by conventional methods.

      Note: All ROMs have no markings (except part number) and no labels. There are also
      no PCB location marks. The numbers I've assigned to the ROMs are made up for
      simplicity. The actual cards have been dumped as a single storage device.

      Confirmed usage on.... (not all games listed)
      Chaos Heat
      Flip Maze
      Kollon
      Mahjong OH
      Nightraid
      Otenki Kororin / Weather Tales
      Psyvariar Medium Unit
      Psyvariar Revision
      Ray Crisis
      RC de Go
      Shanghai Shoryu Sairin
      Shikigami no Shiro
      Souten Ryu
      Space Invaders Anniversary
      Super Puzzle Bobble (English)
      XIIStag
      Zoku Otenami Haiken
      Zooo


Type 2 (3rd party type 'sealed' cards)
------

Note only 1 card was sacrificed and opened.

Top
---

|-|-------------------------------------|
| |             18.00         |-----|   |
| |                           |U15  |   |
| |        |---------|        |-----|   |
| |        |20H2877  |                  |
| |        |IBM0398  |        |-----|   |
| |        |1B37001TQ|        |U13  |   |
| |        |KOREA    |        |-----|   |
| |        |---------|                  |
| |                                     |
| |                                     |
| |   |-----|     |-------|             |
| |   |U10  |     |D431000|             |
| |   |-----|     |-------|             |
| |                                     |
|-|-------------------------------------|
Notes:
      IBM0398 - Custom IC marked 20H2877 IBM0398 1B37001TQ KOREA (TQFP176)
      D431000 - NEC D431000 128k x8 SRAM (TSOP32)
      18.00   - Small square white 'thing', may be an oscillator at 18MHz?
      U*      - TOSHIBA TC58V32FT 4M x8 (32MBit) CMOS NAND Flash EEPROM 3.3Volt (TSOP44)
                The ROMs use a non-standard format and can not be read by conventional methods.

Bottom
------

|-|-------------------------------------|
| |                                     |
| |  |-----|      |-----|     |-----|   |
| |  |U1   |      |U2   |     |U3   |   |
| |  |-----|      |-----|     |-----|   |
| |                                     |
| |                                     |
| |  |-----|      |-----|     |-----|   |
| |  |U4   |      |U5   |     |U6   |   |
| |  |-----|      |-----|     |-----|   |
| |                                     |
| |                                     |
| |               |-----|     |-----|   |
| |               |U7   |     |U8   |   |
| |               |-----|     |-----|   |
| |                                     |
|-|-------------------------------------|
Notes:
      U*  - TOSHIBA TC58V32FT 4M x8 (32MBit) CMOS NAND Flash EEPROM 3.3Volt (TSOP44)
            The ROMs use a non-standard format and can not be read by conventional methods.
            U8 not populated in Nightraid card, but may be populated in other cards.

      Note: All ROMs have no markings (except part number) and no labels. There are PCB
            location marks. The actual cards have been dumped as a single storage device.

      Confirmed usage on.... (not all games listed)
      Nightraid (another one, not the same as listed above)

      Based on card type (made with single sealed steel shell) these are also using the same PCB...
      XIIStag (another one, not the same as listed above)
      Go By RC
      Space Invaders Anniversary (another one, not the same as listed above)
      Super Puzzle Bobble (Japan)
      Usagi

Type 3 (PCMCIA Compact Flash Adaptor + Compact Flash card, sealed together with the game?s label)
------

       The Compact Flash card is read protected, it is a custom Sandisk SDCFB-64 Card (64MByte)

       Confirmed usage on.... (not all games listed)
       Otenami Haiken Final
       Kollon
       Zooo
*/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "includes/psx.h"
#include "machine/at28c16.h"
#include "machine/intelfsh.h"
#include "machine/znsec.h"
#include "machine/idectrl.h"
#include "machine/mb3773.h"
#include "sound/spu.h"
#include "audio/taito_zm.h"

class taitogn_state : public psx_state
{
public:
	taitogn_state(const machine_config &mconfig, device_type type, const char *tag)
		: psx_state(mconfig, type, tag) { }

	intel_te28f160_device *m_biosflash;
	intel_e28f400_device *m_pgmflash;
	intel_te28f160_device *m_sndflash[3];

	unsigned char m_cis[512];
	int m_locked;

	unsigned char m_rf5c296_reg;

	UINT32 m_control;
	UINT32 m_control2;
	UINT32 m_control3;
	int m_v;

	UINT32 m_n_znsecsel;
	UINT32 m_b_znsecport;
	int m_n_dip_bit;
	int m_b_lastclock;
	emu_timer *m_dip_timer;

	UINT32 m_coin_info;
	UINT32 m_mux_data;
	DECLARE_WRITE32_MEMBER(rf5c296_io_w);
	DECLARE_READ32_MEMBER(rf5c296_io_r);
	DECLARE_READ32_MEMBER(rf5c296_mem_r);
	DECLARE_WRITE32_MEMBER(rf5c296_mem_w);
	DECLARE_READ32_MEMBER(flash_subbios_r);
	DECLARE_WRITE32_MEMBER(flash_subbios_w);
	DECLARE_READ32_MEMBER(flash_mn102_r);
	DECLARE_WRITE32_MEMBER(flash_mn102_w);
	DECLARE_READ32_MEMBER(flash_s1_r);
	DECLARE_WRITE32_MEMBER(flash_s1_w);
	DECLARE_READ32_MEMBER(flash_s2_r);
	DECLARE_WRITE32_MEMBER(flash_s2_w);
	DECLARE_READ32_MEMBER(flash_s3_r);
	DECLARE_WRITE32_MEMBER(flash_s3_w);
	DECLARE_READ32_MEMBER(control_r);
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_WRITE32_MEMBER(control2_w);
	DECLARE_READ32_MEMBER(control3_r);
	DECLARE_WRITE32_MEMBER(control3_w);
	DECLARE_READ32_MEMBER(gn_1fb70000_r);
	DECLARE_WRITE32_MEMBER(gn_1fb70000_w);
	DECLARE_READ32_MEMBER(hack1_r);
	DECLARE_READ32_MEMBER(znsecsel_r);
	DECLARE_WRITE32_MEMBER(znsecsel_w);
	DECLARE_READ32_MEMBER(boardconfig_r);
	DECLARE_WRITE32_MEMBER(coin_w);
	DECLARE_READ32_MEMBER(coin_r);
	DECLARE_READ32_MEMBER(gnet_mahjong_panel_r);
	DECLARE_DRIVER_INIT(coh3002t_mp);
	DECLARE_DRIVER_INIT(coh3002t);
};


// rf5c296 is very inaccurate at that point, it hardcodes the gnet config

static void rf5c296_reg_w(ATTR_UNUSED running_machine &machine, UINT8 reg, UINT8 data)
{
	taitogn_state *state = machine.driver_data<taitogn_state>();
	//  fprintf(stderr, "rf5c296_reg_w %02x, %02x (%s)\n", reg, data, machine.describe_context());
	switch (reg)
	{
		// Interrupt and General Control Register
		case 0x03:
			// Check for card reset
			if (!(data & 0x40))
			{
				devtag_reset(machine, ":card");
				state->m_locked = 0x1ff;
				ide_set_gnet_readlock (machine.device(":card"), 1);
			}
		break;

		default:
		break;
	}
}

static UINT8 rf5c296_reg_r(ATTR_UNUSED running_machine &machine, UINT8 reg)
{
	//  fprintf(stderr, "rf5c296_reg_r %02x (%s)\n", reg, machine.describe_context());
	return 0x00;
}

WRITE32_MEMBER(taitogn_state::rf5c296_io_w)
{
	if(offset < 2) {
		ide_controller32_pcmcia_w(machine().device(":card"), offset, data, mem_mask);
		return;
	}

	if(offset == 0x3e0/4) {
		if(ACCESSING_BITS_0_7)
			m_rf5c296_reg = data;
		if(ACCESSING_BITS_8_15)
			rf5c296_reg_w(machine(), m_rf5c296_reg, data >> 8);
	}
}

READ32_MEMBER(taitogn_state::rf5c296_io_r)
{
	if(offset < 2)
		return ide_controller32_pcmcia_r(machine().device(":card"), offset, mem_mask);

	offset *= 4;

	if(offset == 0x3e0/4) {
		UINT32 res = 0xffff0000;
		if(ACCESSING_BITS_0_7)
			res |= m_rf5c296_reg;
		if(ACCESSING_BITS_8_15)
			res |= rf5c296_reg_r(machine(), m_rf5c296_reg) << 8;
		return res;
	}

	return 0xffffffff;
}

// Hardcoded to reach the pcmcia CIS

READ32_MEMBER(taitogn_state::rf5c296_mem_r)
{
	if(offset < 0x80)
		return (m_cis[offset*2+1] << 16) | m_cis[offset*2];

	switch(offset) {
	case 0x080: return 0x00800041;
	case 0x081: return 0x0000002e;
	case 0x100: return m_locked ? 0x00010000 : 0;
	default:
		return 0;
	}
}

WRITE32_MEMBER(taitogn_state::rf5c296_mem_w)
{
	if(offset >= 0x140 && offset <= 0x144) {
		dynamic_buffer key(get_disk_handle(machine(), ":drive_0")->hunk_bytes());

		int pos = (offset - 0x140)*2;
		UINT8 v, k;
		if(ACCESSING_BITS_16_23) {
			v = data >> 16;
			pos++;
		} else
			v = data;
		get_disk_handle(machine(), ":drive_0")->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, key);
		k = pos < key.count() ? key[pos] : 0;
		if(v == k)
			m_locked &= ~(1 << pos);
		else
			m_locked |= 1 << pos;
		if (!m_locked) {
			ide_set_gnet_readlock (machine().device(":card"), 0);
		}
	}
}


// Flash handling

static UINT32 gen_flash_r(intelfsh16_device *device, offs_t offset, UINT32 mem_mask)
{
	UINT32 res = 0;
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		res |= device->read(offset);
	if(ACCESSING_BITS_16_31)
		res |= device->read(offset+1) << 16;
	return res;
}

static void gen_flash_w(intelfsh16_device *device, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		device->write(offset, data);
	if(ACCESSING_BITS_16_31)
	    device->write(offset+1, data >> 16);
}


READ32_MEMBER(taitogn_state::flash_subbios_r)
{
	return gen_flash_r(m_biosflash, offset, mem_mask);
}

WRITE32_MEMBER(taitogn_state::flash_subbios_w)
{
	gen_flash_w(m_biosflash, offset, data, mem_mask);
}

READ32_MEMBER(taitogn_state::flash_mn102_r)
{
	return gen_flash_r(m_pgmflash, offset, mem_mask);
}

WRITE32_MEMBER(taitogn_state::flash_mn102_w)
{
	gen_flash_w(m_pgmflash, offset, data, mem_mask);
}

READ32_MEMBER(taitogn_state::flash_s1_r)
{
	return gen_flash_r(m_sndflash[0], offset, mem_mask);
}

WRITE32_MEMBER(taitogn_state::flash_s1_w)
{
	gen_flash_w(m_sndflash[0], offset, data, mem_mask);
}

READ32_MEMBER(taitogn_state::flash_s2_r)
{
	return gen_flash_r(m_sndflash[1], offset, mem_mask);
}

WRITE32_MEMBER(taitogn_state::flash_s2_w)
{
	gen_flash_w(m_sndflash[1], offset, data, mem_mask);
}

READ32_MEMBER(taitogn_state::flash_s3_r)
{
	return gen_flash_r(m_sndflash[2], offset, mem_mask);
}

WRITE32_MEMBER(taitogn_state::flash_s3_w)
{
	gen_flash_w(m_sndflash[2], offset, data, mem_mask);
}

static void install_handlers(running_machine &machine, int mode)
{
	taitogn_state *state = machine.driver_data<taitogn_state>();
	address_space *a = machine.device("maincpu")->memory().space(AS_PROGRAM);
	if(mode == 0) {
		// Mode 0 has access to the subbios, the mn102 flash and the rf5c296 mem zone
		a->install_readwrite_handler(0x1f000000, 0x1f1fffff, read32_delegate(FUNC(taitogn_state::flash_subbios_r),state), write32_delegate(FUNC(taitogn_state::flash_subbios_w),state));
		a->install_readwrite_handler(0x1f200000, 0x1f2fffff, read32_delegate(FUNC(taitogn_state::rf5c296_mem_r),state), write32_delegate(FUNC(taitogn_state::rf5c296_mem_w),state));
		a->install_readwrite_handler(0x1f300000, 0x1f37ffff, read32_delegate(FUNC(taitogn_state::flash_mn102_r),state), write32_delegate(FUNC(taitogn_state::flash_mn102_w),state));
		a->nop_readwrite(0x1f380000, 0x1f5fffff);

	} else {
		// Mode 1 has access to the 3 samples flashes
		a->install_readwrite_handler(0x1f000000, 0x1f1fffff, read32_delegate(FUNC(taitogn_state::flash_s1_r),state), write32_delegate(FUNC(taitogn_state::flash_s1_w),state));
		a->install_readwrite_handler(0x1f200000, 0x1f3fffff, read32_delegate(FUNC(taitogn_state::flash_s2_r),state), write32_delegate(FUNC(taitogn_state::flash_s2_w),state));
		a->install_readwrite_handler(0x1f400000, 0x1f5fffff, read32_delegate(FUNC(taitogn_state::flash_s3_r),state), write32_delegate(FUNC(taitogn_state::flash_s3_w),state));
	}
}

// Misc. controls

READ32_MEMBER(taitogn_state::control_r)
{
	//      fprintf(stderr, "gn_r %08x @ %08x (%s)\n", 0x1fb00000+4*offset, mem_mask, machine().describe_context());
	return m_control;
}

WRITE32_MEMBER(taitogn_state::control_w)
{
	// 20 = watchdog
	// 04 = select bank

	// According to the rom code, bits 1-0 may be part of the bank
	// selection too, but they're always 0.

	UINT32 p = m_control;
	device_t *mb3773 = machine().device("mb3773");

	COMBINE_DATA(&m_control);

	mb3773_set_ck(mb3773, (m_control & 0x20) >> 5);

#if 0
	if((p ^ control) & ~0x20)
		fprintf(stderr, "control = %c%c.%c %c%c%c%c (%s)\n",
				control & 0x80 ? '1' : '0',
				control & 0x40 ? '1' : '0',
				control & 0x10 ? '1' : '0',
				control & 0x08 ? '1' : '0',
				control & 0x04 ? 'f' : '-',
				control & 0x02 ? '1' : '0',
				control & 0x01 ? '1' : '0',
				machine().describe_context());
#endif

	if((p ^ m_control) & 0x04)
		install_handlers(machine(), m_control & 4 ? 1 : 0);
}

WRITE32_MEMBER(taitogn_state::control2_w)
{
	COMBINE_DATA(&m_control2);
}

READ32_MEMBER(taitogn_state::control3_r)
{
	return m_control3;
}

WRITE32_MEMBER(taitogn_state::control3_w)
{
	COMBINE_DATA(&m_control3);
}

READ32_MEMBER(taitogn_state::gn_1fb70000_r)
{
	// (1328) 1348 tests mask 0002, 8 times.
	// Called by 1434, exit at 143c
	// f -> 4/1
	// end with 4x1 -> ok
	// end with 4x0 -> configid error
	// so returning 2 always works, strange.

	return 2;
}

WRITE32_MEMBER(taitogn_state::gn_1fb70000_w)
{
	// Writes 0 or 1 all the time, it *may* have somthing to do with
	// i/o port width, but then maybe not
}

READ32_MEMBER(taitogn_state::hack1_r)
{
	m_v = m_v ^ 8;
	// Probably something to do with sound
	return m_v;
}



// Lifted from zn.c

static const UINT8 tt10[ 8 ] = { 0x80, 0x20, 0x38, 0x08, 0xf1, 0x03, 0xfe, 0xfc };
static const UINT8 tt16[ 8 ] = { 0xc0, 0x04, 0xf9, 0xe1, 0x60, 0x70, 0xf2, 0x02 };

READ32_MEMBER(taitogn_state::znsecsel_r)
{
	return m_n_znsecsel;
}

static void sio_znsec0_handler( running_machine &machine, int n_data )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	if( ( n_data & PSX_SIO_OUT_CLOCK ) == 0 )
        {
			if( state->m_b_lastclock )
				psx_sio_input( machine, 0, PSX_SIO_IN_DATA, ( znsec_step( 0, ( n_data & PSX_SIO_OUT_DATA ) != 0 ) != 0 ) * PSX_SIO_IN_DATA );
			state->m_b_lastclock = 0;
        }
	else
        {
			state->m_b_lastclock = 1;
        }
}

static void sio_znsec1_handler( running_machine &machine, int n_data )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	if( ( n_data & PSX_SIO_OUT_CLOCK ) == 0 )
        {
			if( state->m_b_lastclock )
				psx_sio_input( machine, 0, PSX_SIO_IN_DATA, ( znsec_step( 1, ( n_data & PSX_SIO_OUT_DATA ) != 0 ) != 0 ) * PSX_SIO_IN_DATA );
			state->m_b_lastclock = 0;
        }
	else
        {
			state->m_b_lastclock = 1;
        }
}

static void sio_pad_handler( running_machine &machine, int n_data )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	if( ( n_data & PSX_SIO_OUT_DTR ) != 0 )
        {
			state->m_b_znsecport = 1;
        }
	else
        {
			state->m_b_znsecport = 0;
        }

	psx_sio_input( machine, 0, PSX_SIO_IN_DATA | PSX_SIO_IN_DSR, PSX_SIO_IN_DATA | PSX_SIO_IN_DSR );
}

static void sio_dip_handler( running_machine &machine, int n_data )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	if( ( n_data & PSX_SIO_OUT_CLOCK ) == 0 )
	{
		if( state->m_b_lastclock )
		{
			int bit = ( ( state->ioport("DSW")->read() >> state->m_n_dip_bit ) & 1 );
			psx_sio_input( machine, 0, PSX_SIO_IN_DATA, bit * PSX_SIO_IN_DATA );
			state->m_n_dip_bit++;
			state->m_n_dip_bit &= 7;
		}
		state->m_b_lastclock = 0;
	}
	else
	{
		state->m_b_lastclock = 1;
	}
}

WRITE32_MEMBER(taitogn_state::znsecsel_w)
{
	COMBINE_DATA( &m_n_znsecsel );

	if( ( m_n_znsecsel & 0x80 ) == 0 )
        {
			psx_sio_install_handler( machine(), 0, sio_pad_handler );
			psx_sio_input( machine(), 0, PSX_SIO_IN_DSR, 0 );
        }
	else if( ( m_n_znsecsel & 0x08 ) == 0 )
        {
			znsec_start( 1 );
			psx_sio_install_handler( machine(), 0, sio_znsec1_handler );
			psx_sio_input( machine(), 0, PSX_SIO_IN_DSR, 0 );
        }
	else if( ( m_n_znsecsel & 0x04 ) == 0 )
        {
			znsec_start( 0 );
			psx_sio_install_handler( machine(), 0, sio_znsec0_handler );
			psx_sio_input( machine(), 0, PSX_SIO_IN_DSR, 0 );
        }
	else
        {
			m_n_dip_bit = 0;
			m_b_lastclock = 1;

			psx_sio_install_handler( machine(), 0, sio_dip_handler );
			psx_sio_input( machine(), 0, PSX_SIO_IN_DSR, 0 );

			m_dip_timer->adjust( downcast<cpu_device *>(&space.device())->cycles_to_attotime( 100 ), 1 );
        }
}

static TIMER_CALLBACK( dip_timer_fired )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	psx_sio_input( machine, 0, PSX_SIO_IN_DSR, param * PSX_SIO_IN_DSR );

	if( param )
	{
		state->m_dip_timer->adjust(machine.device<cpu_device>("maincpu")->cycles_to_attotime(50));
	}
}


READ32_MEMBER(taitogn_state::boardconfig_r)
{
	/*
    ------00 mem=4M
    ------01 mem=4M
    ------10 mem=8M
    ------11 mem=16M
    -----0-- smem=hM
    -----1-- smem=2M
    ----0--- vmem=1M
    ----1--- vmem=2M
    000----- rev=-2
    001----- rev=-1
    010----- rev=0
    011----- rev=1
    100----- rev=2
    101----- rev=3
    110----- rev=4
    111----- rev=5
    */

	return 64|32|8;
}


WRITE32_MEMBER(taitogn_state::coin_w)
{
	/* 0x01=counter
       0x02=coin lock 1
       0x08=??
       0x20=coin lock 2
       0x80=??
    */
	COMBINE_DATA (&m_coin_info);
}

READ32_MEMBER(taitogn_state::coin_r)
{
	return m_coin_info;
}

/* mahjong panel handler (for Usagi & Mahjong Oh) */
READ32_MEMBER(taitogn_state::gnet_mahjong_panel_r)
{
	m_mux_data = m_coin_info;
	m_mux_data &= 0xcc;

	switch(m_mux_data)
	{
		case 0x04: return ioport("KEY0")->read();
		case 0x08: return ioport("KEY1")->read();
		case 0x40: return ioport("KEY2")->read();
		case 0x80: return ioport("KEY3")->read();
	}

	/* mux disabled */
	return ioport("P4")->read();
}

// Init and reset

DRIVER_INIT_MEMBER(taitogn_state,coh3002t)
{
	m_biosflash = machine().device<intel_te28f160_device>("biosflash");
	m_pgmflash = machine().device<intel_e28f400_device>("pgmflash");
	m_sndflash[0] = machine().device<intel_te28f160_device>("sndflash0");
	m_sndflash[1] = machine().device<intel_te28f160_device>("sndflash1");
	m_sndflash[2] = machine().device<intel_te28f160_device>("sndflash2");

	psx_driver_init(machine());
	znsec_init(0, tt10);
	znsec_init(1, tt16);
	psx_sio_install_handler(machine(), 0, sio_pad_handler);
	m_dip_timer = machine().scheduler().timer_alloc( FUNC(dip_timer_fired), NULL );

	UINT32 metalength;
	memset(m_cis, 0xff, 512);
	if (get_disk_handle(machine(), ":drive_0") != NULL)
		get_disk_handle(machine(), ":drive_0")->read_metadata(PCMCIA_CIS_METADATA_TAG, 0, m_cis, 512, metalength);
}

DRIVER_INIT_MEMBER(taitogn_state,coh3002t_mp)
{
	DRIVER_INIT_CALL(coh3002t);
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_read_handler(0x1fa10100, 0x1fa10103, read32_delegate(FUNC(taitogn_state::gnet_mahjong_panel_r),this));
}

static MACHINE_RESET( coh3002t )
{
	taitogn_state *state = machine.driver_data<taitogn_state>();

	state->m_b_lastclock = 1;
	state->m_locked = 0x1ff;
	install_handlers(machine, 0);
	state->m_control = 0;
	devtag_reset(machine, ":card");
	ide_set_gnet_readlock(machine.device(":card"), 1);

	// halt sound CPU since it has no valid program at start
	machine.device("mn10200")->execute().set_input_line(INPUT_LINE_RESET,ASSERT_LINE); /* MCU */
}

static ADDRESS_MAP_START( taitogn_map, AS_PROGRAM, 32, taitogn_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("share1") /* ram */
	AM_RANGE(0x00400000, 0x007fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x1f000000, 0x1f1fffff) AM_READWRITE(flash_s1_r, flash_s1_w)
	AM_RANGE(0x1f200000, 0x1f3fffff) AM_READWRITE(flash_s2_r, flash_s2_w)
	AM_RANGE(0x1f400000, 0x1f5fffff) AM_READWRITE(flash_s3_r, flash_s3_w)
	AM_RANGE(0x1fa00000, 0x1fa00003) AM_READ_PORT("P1")
	AM_RANGE(0x1fa00100, 0x1fa00103) AM_READ_PORT("P2")
	AM_RANGE(0x1fa00200, 0x1fa00203) AM_READ_PORT("SERVICE")
	AM_RANGE(0x1fa00300, 0x1fa00303) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1fa10000, 0x1fa10003) AM_READ_PORT("P3")
	AM_RANGE(0x1fa10100, 0x1fa10103) AM_READ_PORT("P4")
	AM_RANGE(0x1fa10200, 0x1fa10203) AM_READ(boardconfig_r)
	AM_RANGE(0x1fa10300, 0x1fa10303) AM_READWRITE(znsecsel_r, znsecsel_w)
	AM_RANGE(0x1fa20000, 0x1fa20003) AM_READWRITE(coin_r, coin_w)
	AM_RANGE(0x1fa30000, 0x1fa30003) AM_READWRITE(control3_r, control3_w)
	AM_RANGE(0x1fa51c00, 0x1fa51dff) AM_READWRITE16_LEGACY(spu_r, spu_w, 0xffffffff) // systematic read at spu_address + 250000, result dropped, maybe other accesses
	AM_RANGE(0x1fa60000, 0x1fa60003) AM_READ(hack1_r)
	AM_RANGE(0x1faf0000, 0x1faf07ff) AM_DEVREADWRITE8_LEGACY("at28c16", at28c16_r, at28c16_w, 0xffffffff) /* eeprom */
	AM_RANGE(0x1fb00000, 0x1fb0ffff) AM_READWRITE(rf5c296_io_r, rf5c296_io_w)
	AM_RANGE(0x1fb40000, 0x1fb40003) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x1fb60000, 0x1fb60003) AM_WRITE(control2_w)
	AM_RANGE(0x1fb70000, 0x1fb70003) AM_READWRITE(gn_1fb70000_r, gn_1fb70000_w)
	AM_RANGE(0x1fbe0000, 0x1fbe01ff) AM_RAM // 256 bytes com zone with the mn102, low bytes of words only, with additional comm at 1fb80000
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("mainbios", 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x80400000, 0x807fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_WRITENOP AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END


static void spu_irq(device_t *device, UINT32 data)
{
	if (data)
	{
		psx_irq_set(device->machine(), 1<<9);
	}
}

static const ide_config ide_intf = 
{
	NULL, 
	NULL, 
	0
};

static MACHINE_CONFIG_START( coh3002t, taitogn_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8661R, XTAL_100MHz )
	MCFG_CPU_PROGRAM_MAP(taitogn_map)

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8654Q, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)

	MCFG_MACHINE_RESET( coh3002t )

	MCFG_AT28C16_ADD( "at28c16", 0 )
	MCFG_IDE_CONTROLLER_ADD( "card", ide_intf, ide_devices, "hdd", NULL, true)

	MCFG_MB3773_ADD("mb3773")

	MCFG_INTEL_TE28F160_ADD("biosflash")
	MCFG_INTEL_E28F400_ADD("pgmflash")
	MCFG_INTEL_TE28F160_ADD("sndflash0")
	MCFG_INTEL_TE28F160_ADD("sndflash1")
	MCFG_INTEL_TE28F160_ADD("sndflash2")

	MCFG_FRAGMENT_ADD( taito_zoom_sound )
MACHINE_CONFIG_END

static INPUT_PORTS_START( coh3002t )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* input port define for the mahjong panel (standard type) */
static INPUT_PORTS_START( coh3002t_mp )
	PORT_INCLUDE( coh3002t )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //rate button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//

#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define TAITOGNET_BIOS \
	ROM_REGION32_LE( 0x080000, "mainbios", 0 ) \
	ROM_LOAD( "coh-3002t.353", 0x000000, 0x080000, CRC(03967fa7) SHA1(0e17fec2286e4e25deb23d40e41ce0986f373d49) ) \
	ROM_REGION( 0x200000, "biosflash", 0 ) \
	ROM_SYSTEM_BIOS( 0, "v1",   "G-NET Bios v1" ) \
    	ROM_LOAD16_WORD_BIOS(0, "flash.u30", 0x000000, 0x200000, CRC(c48c8236) SHA1(c6dad60266ce2ff635696bc0d91903c543273559) ) \
	ROM_SYSTEM_BIOS( 1, "v2",   "G-NET Bios v2" ) \
    	ROM_LOAD16_WORD_BIOS(1, "flashv2.u30", 0x000000, 0x200000, CRC(CAE462D3) SHA1(f1b10846a8423d9fe021191c5876190857c3d2a4) ) \
	ROM_REGION32_LE( 0x80000,  "mn10200", 0) \
	ROM_FILL( 0, 0x80000, 0xff) \
	ROM_REGION32_LE( 0x600000, "zsg1", 0) \
	ROM_FILL( 0, 0x600000, 0xff)

ROM_START( taitogn )
	TAITOGNET_BIOS
ROM_END

/* Taito */

ROM_START(raycris)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "raycris", 0, SHA1(015cb0e6c4421cc38809de28c4793b4491386aee))
ROM_END


ROM_START(gobyrc)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "gobyrc", 0, SHA1(0bee1f495fc8b033fd56aad9260ae94abb35eb58))
ROM_END

ROM_START(rcdego)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "rcdego", 0, SHA1(9e177f2a3954cfea0c8c5a288e116324d10f5dd1))
ROM_END

ROM_START(chaoshea)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "chaosheat", 0, SHA1(c13b7d7025eee05f1f696d108801c7bafb3f1356))
ROM_END

ROM_START(chaosheaj)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "chaosheatj", 0, SHA1(2f211ac08675ea8ec33c7659a13951db94eaa627))
ROM_END


ROM_START(flipmaze)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "flipmaze", 0, SHA1(423b6c06f4f2d9a608ce20b61a3ac11687d22c40) )
ROM_END


ROM_START(spuzbobl)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "spuzbobl", 0, SHA1(1b1c72fb7e5656021485fefaef8f2ba48e2b4ea8))
ROM_END

ROM_START(spuzboblj)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "spuzbobj", 0, SHA1(dac433cf88543d2499bf797d7406b82ae4338726))
ROM_END

ROM_START(soutenry)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "soutenry", 0, SHA1(9204d0be833d29f37b8cd3fbdf09da69b622254b))
ROM_END

ROM_START(shanghss)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "shanghss", 0, SHA1(7964f71ec5c81d2120d83b63a82f97fbad5a8e6d))
ROM_END

ROM_START(sianniv)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "sianniv", 0, SHA1(1e08b813190a9e1baf29bc16884172d6c8da7ae3))
ROM_END

ROM_START(kollon)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "kollon", 0, SHA1(d8ea5b5b0ee99004b16ef89883e23de6c7ddd7ce))
ROM_END

ROM_START(kollonc)
	TAITOGNET_BIOS
	ROM_DEFAULT_BIOS( "v2" )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "kollonc", 0, SHA1(ce62181659701cfb8f7c564870ab902be4d8e060)) /* Original Taito Compact Flash version */
ROM_END

ROM_START(shikigam)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "shikigam", 0, SHA1(fa49a0bc47f5cb7c30d7e49e2c3696b21bafb840))
ROM_END


/* Success */

ROM_START(otenamih)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "otenamih", 0, SHA1(b3babe3a1876c43745616ee1e7d87276ce7dad0b) )
ROM_END


ROM_START(psyvaria)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "psyvaria", 0,  SHA1(b981a42a10069322b77f7a268beae1d409b4156d))
ROM_END

ROM_START(psyvarrv)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "psyvarrv", 0, SHA1(277c4f52502bcd7acc1889840962ec80d56465f3))
ROM_END

ROM_START(zooo)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "zooo", 0, SHA1(e275b3141b2bc49142990e6b497a5394a314a30b))
ROM_END

ROM_START(zokuoten)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "zokuoten", 0, SHA1(5ce13db00518f96af64935176c71ec68d2a51938))
ROM_END

ROM_START(otenamhf)
	TAITOGNET_BIOS
	ROM_DEFAULT_BIOS( "v2" )

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "otenamhf", 0, SHA1(5b15c33bf401e5546d78e905f538513d6ffcf562)) /* Original Taito Compact Flash version */
ROM_END




/* Takumi */

ROM_START(nightrai)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "nightrai", 0, SHA1(74d0458f851cbcf10453c5cc4c47bb4388244cdf))
ROM_END

ROM_START(otenki)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "otenki", 0, SHA1(7e745ca4c4570215f452fd09cdd56a42c39caeba))
ROM_END

/* Warashi */

ROM_START(usagi)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "usagi", 0, SHA1(edf9dd271957f6cb06feed238ae21100514bef8e))
ROM_END

ROM_START(mahjngoh)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "mahjngoh", 0, SHA1(3ef1110d15582d7c0187438d7ad61765dd121cff))
ROM_END

ROM_START(shangtou)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "shanghaito", 0, SHA1(9901db5a9aae77e3af4157aa2c601eaab5b7ca85) )
ROM_END


/* Triangle Service */

ROM_START(xiistag)
	TAITOGNET_BIOS

	DISK_REGION( "drive_0" )
	DISK_IMAGE( "xiistag", 0, SHA1(586e37c8d926293b2bd928e5f0d693910cfb05a2))
ROM_END


/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-3002t.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1997, taitogn,  0,        coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Taito GNET", GAME_IS_BIOS_ROOT )

GAME( 1998, chaoshea, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Chaos Heat (V2.09O)", GAME_IMPERFECT_SOUND )
GAME( 1998, chaosheaj,chaoshea, coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Chaos Heat (V2.08J)", GAME_IMPERFECT_SOUND )
GAME( 1998, raycris,  taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Ray Crisis (V2.03J)", GAME_IMPERFECT_SOUND )
GAME( 1999, spuzbobl, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Super Puzzle Bobble (V2.05O)", GAME_IMPERFECT_SOUND )
GAME( 1999, spuzboblj,spuzbobl, coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Super Puzzle Bobble (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 1999, gobyrc,   taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Go By RC (V2.03O)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // custom inputs need calibrating
GAME( 1999, rcdego,   gobyrc,   coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "RC De Go (V2.03J)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // custom inputs need calibrating
GAME( 1999, flipmaze, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito / Moss", "Flip Maze (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 2001, shikigam, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT270, "Alfa System / Taito", "Shikigami no Shiro (V2.03J)", GAME_IMPERFECT_SOUND )
GAME( 2003, sianniv,  taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT270, "Taito", "Space Invaders Anniversary (V2.02J)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // IRQ at the wrong time
GAME( 2003, kollon,   taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Kollon (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 2003, kollonc,  kollon,   coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Taito", "Kollon (V2.04JC)", GAME_IMPERFECT_SOUND )

GAME( 1999, otenamih, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Success", "Otenami Haiken (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 2005, otenamhf, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Success / Warashi", "Otenami Haiken Final (V2.07JC)", GAME_IMPERFECT_SOUND )
GAME( 2000, psyvaria, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT270, "Success", "Psyvariar -Medium Unit- (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 2000, psyvarrv, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT270, "Success", "Psyvariar -Revision- (V2.04J)", GAME_IMPERFECT_SOUND )
GAME( 2000, zokuoten, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Success", "Zoku Otenamihaiken (V2.03J)", GAME_IMPERFECT_SOUND )
GAME( 2004, zooo,     taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Success", "Zooo (V2.01J)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // missing most of the playfield

GAME( 1999, mahjngoh, taitogn,  coh3002t, coh3002t_mp, taitogn_state, coh3002t_mp, ROT0, "Warashi / Mahjong Kobo / Taito", "Mahjong Oh (V2.06J)", GAME_IMPERFECT_SOUND )
GAME( 2001, usagi,    taitogn,  coh3002t, coh3002t_mp, taitogn_state, coh3002t_mp, ROT0, "Warashi / Mahjong Kobo / Taito", "Usagi (V2.02J)", GAME_IMPERFECT_SOUND )
GAME( 2000, soutenry, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Warashi", "Soutenryu (V2.07J)", GAME_IMPERFECT_SOUND )
GAME( 2000, shanghss, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Warashi", "Shanghai Shoryu Sairin (V2.03J)", GAME_IMPERFECT_SOUND )
GAME( 2002, shangtou, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Warashi / Sunsoft / Taito", "Shanghai Sangokuhai Tougi (Ver 2.01J)", GAME_IMPERFECT_SOUND )

GAME( 2001, nightrai, taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Takumi", "Night Raid (V2.03J)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND ) // no background / enemy sprites
GAME( 2001, otenki,   taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT0,   "Takumi", "Otenki Kororin (V2.01J)", GAME_IMPERFECT_SOUND )

GAME( 2002, xiistag,  taitogn,  coh3002t, coh3002t, taitogn_state, coh3002t, ROT270, "Triangle Service", "XII Stag (V2.01J)", GAME_IMPERFECT_SOUND )
