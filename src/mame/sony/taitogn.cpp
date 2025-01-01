// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, smf
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

Not every game uses the Taito Zoom sound hardware, these are: Otenami Haiken, Otenami Haiken Final,
Zoku Otenamihaiken, Zooo, and possibly Space Invaders Anniversary.

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
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz

      Video syncs are 59.8260Hz and 15.4333kHz


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

RC De Go! Controller Info
-------------------------

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
#include "zn.h"

#include "bus/pccard/ataflash.h"
#include "machine/intelfsh.h"
#include "machine/rf5c296.h"

namespace {

//
// Taito GNET specific
//

class taitogn_state : public zn_state
{
public:
	taitogn_state(const machine_config &mconfig, device_type type, const char *tag) :
		zn_state(mconfig, type, tag),
		m_mn10200(*this, "taito_zoom:mn10200"),
		m_pccard(*this, "pccard"),
		m_flashbank(*this, "flashbank"),
		m_mb3773(*this, "mb3773"),
		m_zoom(*this, "taito_zoom"),
		m_pgmflash(*this, "pgmflash"),
		m_sndflash(*this, "sndflash%u", 0U),
		m_jp1(*this, "JP1"),
		m_has_zoom(true)
	{
	}

	void init_nozoom();

	void base_config(machine_config &config);
	void coh3002t_t2_mp(machine_config &config);
	void coh3002t(machine_config &config);
	void coh3002t_t1_mp(machine_config &config);
	void coh3002t_cf(machine_config &config);
	void coh3002t_t2(machine_config &config);
	void coh3002t_t1(machine_config &config);

private:
	uint8_t control_r();
	void control_w(uint8_t data);
	void control2_w(uint16_t data);
	uint8_t control3_r();
	void control3_w(uint8_t data);
	uint16_t gn_1fb70000_r();
	void gn_1fb70000_w(uint16_t data);
	uint16_t hack1_r(offs_t offset);
	void coin_w(uint8_t data);
	uint8_t coin_r();
	uint8_t gnet_mahjong_panel_r();
	uint32_t zsg2_ext_r(offs_t offset);

	void flashbank_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_mp_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_mn10200;
	required_device<pccard_slot_device> m_pccard;
	required_device<address_map_bank_device> m_flashbank;
	required_device<mb3773_device> m_mb3773;
	required_device<taito_zoom_device> m_zoom;
	required_device<intelfsh16_device> m_pgmflash;
	required_device_array<intelfsh16_device, 3> m_sndflash;
	required_ioport m_jp1;

	bool m_has_zoom;
	uint8_t m_control = 0;
	uint16_t m_control2 = 0;
	uint8_t m_control3 = 0;
	int m_v = 0;

	uint8_t m_coin_info = 0;
};

// Misc. controls

uint8_t taitogn_state::control_r()
{
	return m_control;
}

void taitogn_state::control_w(uint8_t data)
{
	// 20 = watchdog
	m_mb3773->write_line_ck((data & 0x20) >> 5);

	// 10 = sound hw reset, but make sure it's only booted on games that use it
	if (m_has_zoom)
	{
		m_mn10200->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		if (~data & m_control & 0x10)
		{
			logerror("control_w Zoom reset\n");

			m_zoom->reset();

			// assume that this also readies the sound flash chips
			m_pgmflash->write(0, 0xff);
			m_sndflash[0]->write(0, 0xff);
			m_sndflash[1]->write(0, 0xff);
			m_sndflash[2]->write(0, 0xff);
		}
	}

	// 04 = select bank
	// According to the rom code, bits 1-0 may be part of the bank
	// selection too, but they're always 0.
	m_flashbank->set_bank(((data>>2) & 1) | (m_jp1->read() << 1));

	m_control = data;
}

void taitogn_state::control2_w(uint16_t data)
{
	m_control2 = data;
}

uint8_t taitogn_state::control3_r()
{
	return m_control3;
}

void taitogn_state::control3_w(uint8_t data)
{
	m_control3 = data;
}

uint16_t taitogn_state::gn_1fb70000_r()
{
	// (1328) 1348 tests mask 0002, 8 times.
	// Called by 1434, exit at 143c
	// f -> 4/1
	// end with 4x1 -> ok
	// end with 4x0 -> configid error
	// so returning 2 always works, strange.

	return 2;
}

void taitogn_state::gn_1fb70000_w(uint16_t data)
{
	// Writes 0 or 1 all the time, it *may* have something to do with
	// i/o port width, but then maybe not
}

uint16_t taitogn_state::hack1_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			if (!machine().side_effects_disabled())
				m_v = m_v ^ 8;
			// Probably something to do with MCU
			return m_v;

		default:
			break;
	}

	return 0;
}



void taitogn_state::coin_w(uint8_t data)
{
	/* 0x01=counter
	   0x02=coin lock 1
	   0x04=mahjong row select
	   0x08=mahjong row select
	   0x10=??
	   0x20=coin lock 2
	   0x40=mahjong row select
	   0x80=mahjong row select
	*/
	m_coin_info = data;
}

uint8_t taitogn_state::coin_r()
{
	return m_coin_info;
}

/* mahjong panel handler (for Usagi & Mahjong Oh) */
uint8_t taitogn_state::gnet_mahjong_panel_r()
{
	switch (m_coin_info & 0xcc)
	{
		case 0x04: return ioport("KEY0")->read();
		case 0x08: return ioport("KEY1")->read();
		case 0x40: return ioport("KEY2")->read();
		case 0x80: return ioport("KEY3")->read();

		default:
			break;
	}

	/* mux disabled */
	return ioport("P4")->read();
}

uint32_t taitogn_state::zsg2_ext_r(offs_t offset)
{
	offset <<= 1;

	switch (offset & 0x300000)
	{
		case 0x000000:
		case 0x100000:
		case 0x200000: return m_sndflash[offset >> 20]->read(offset & 0xfffff) | m_sndflash[offset >> 20]->read((offset & 0xfffff) | 1) << 16;

		default:
			break;
	}
	return 0;
}

void taitogn_state::machine_start()
{
	zn_state::machine_start();
	save_item(NAME(m_control));
	save_item(NAME(m_control2));
	save_item(NAME(m_control3));
	save_item(NAME(m_v));
	save_item(NAME(m_coin_info));
}

void taitogn_state::machine_reset()
{
	// halt sound CPU since it has no valid program at start
	m_mn10200->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_control = 0x10;
	m_flashbank->set_bank(m_jp1->read() << 1);
}

void taitogn_state::init_nozoom()
{
	m_has_zoom = false;
}

void taitogn_state::main_map(address_map &map)
{
	zn_base_map(map);

	map(0x1f000000, 0x1f7fffff).m(m_flashbank, FUNC(address_map_bank_device::amap16));
	map(0x1fa20000, 0x1fa20000).rw(FUNC(taitogn_state::coin_r), FUNC(taitogn_state::coin_w));
	map(0x1fa30000, 0x1fa30000).rw(FUNC(taitogn_state::control3_r), FUNC(taitogn_state::control3_w));
	map(0x1fa51c00, 0x1fa51dff).nopr(); // systematic read at spu_address + 250000, result dropped, maybe other accesses
	map(0x1fa60000, 0x1fa60003).r(FUNC(taitogn_state::hack1_r));
	map(0x1fb00000, 0x1fb0ffff).rw("rf5c296", FUNC(rf5c296_device::io_r), FUNC(rf5c296_device::io_w));
	map(0x1fb40000, 0x1fb40000).rw(FUNC(taitogn_state::control_r), FUNC(taitogn_state::control_w));
	map(0x1fb60000, 0x1fb60001).w(FUNC(taitogn_state::control2_w));
	map(0x1fb70000, 0x1fb70001).rw(FUNC(taitogn_state::gn_1fb70000_r), FUNC(taitogn_state::gn_1fb70000_w));
	map(0x1fb80000, 0x1fb80001).w(m_zoom, FUNC(taito_zoom_device::reg_data_w));
	map(0x1fb80002, 0x1fb80003).w(m_zoom, FUNC(taito_zoom_device::reg_address_w));
	map(0x1fba0000, 0x1fba0001).w(m_zoom, FUNC(taito_zoom_device::sound_irq_w));
	map(0x1fbc0000, 0x1fbc0001).r(m_zoom, FUNC(taito_zoom_device::sound_irq_r));
	map(0x1fbe0000, 0x1fbe01ff).rw(m_zoom, FUNC(taito_zoom_device::shared_ram_r), FUNC(taito_zoom_device::shared_ram_w)).umask32(0x00ff00ff); // M66220FP for comms with the MN10200
}

void taitogn_state::flashbank_map(address_map &map)
{
	// Bank 0 has access to the sub-bios, the mn102 flash and the rf5c296 mem zone
	map(0x00000000, 0x001fffff).rw("biosflash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x00200000, 0x002fffff).rw("rf5c296", FUNC(rf5c296_device::mem_r), FUNC(rf5c296_device::mem_w));
	map(0x00300000, 0x0037ffff).rw(m_pgmflash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));

	// Bank 1 & 3 has access to the 3 samples flashes
	map(0x08000000, 0x081fffff).mirror(0x10000000).rw(m_sndflash[0], FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x08200000, 0x083fffff).mirror(0x10000000).rw(m_sndflash[1], FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x08400000, 0x085fffff).mirror(0x10000000).rw(m_sndflash[2], FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));

	// Bank 2 has access to the sub-bios, the mn102 flash and the mask eprom
	map(0x10000000, 0x100fffff).rom().region("bioseprom", 0);
	map(0x10100000, 0x1017ffff).mirror(0x80000).rw(m_pgmflash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x10200000, 0x103fffff).rw("biosflash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
}

void taitogn_state::main_mp_map(address_map &map)
{
	main_map(map);
	map(0x1fa10100, 0x1fa10100).r(FUNC(taitogn_state::gnet_mahjong_panel_r));
}

void slot_ataflash(device_slot_interface &device)
{
	device.option_add("taitopccard1", TAITO_PCCARD1);
	device.option_add("taitopccard2", TAITO_PCCARD2);
	device.option_add("taitocf", TAITO_COMPACT_FLASH);
	device.option_add("ataflash", ATA_FLASH_PCCARD);
}

void taitogn_state::coh3002t(machine_config &config)
{
	zn2(config);
	gameboard_cat702(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitogn_state::main_map);

	RF5C296(config, "rf5c296", 0).set_pccard("pccard");

	PCCARD_SLOT(config, m_pccard, slot_ataflash, nullptr);

	MB3773(config, "mb3773");

	INTEL_TE28F160(config, "biosflash");
	INTEL_E28F400B(config, "pgmflash");
	INTEL_TE28F160(config, "sndflash0");
	INTEL_TE28F160(config, "sndflash1");
	INTEL_TE28F160(config, "sndflash2");

	ADDRESS_MAP_BANK(config, "flashbank").set_map(&taitogn_state::flashbank_map).set_options(ENDIANNESS_LITTLE, 16, 32, 0x8000000);

	subdevice<spu_device>("spu")->reset_routes();
	subdevice<spu_device>("spu")->add_route(0, "lspeaker", 0.3);
	subdevice<spu_device>("spu")->add_route(1, "rspeaker", 0.3);

	TAITO_ZOOM(config, m_zoom);
	m_zoom->set_use_flash();
	m_zoom->add_route(0, "lspeaker", 1.0);
	m_zoom->add_route(1, "rspeaker", 1.0);

	m_zoom->subdevice<zsg2_device>("zsg2")->ext_read().set(FUNC(taitogn_state::zsg2_ext_r));
}

void taitogn_state::coh3002t_t1(machine_config &config)
{
	coh3002t(config);
	m_pccard->set_default_option("taitopccard1");
}

void taitogn_state::coh3002t_t2(machine_config &config)
{
	coh3002t(config);
	m_pccard->set_default_option("taitopccard2");
}

void taitogn_state::coh3002t_t1_mp(machine_config &config)
{
	coh3002t_t1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitogn_state::main_mp_map);
}

void taitogn_state::coh3002t_t2_mp(machine_config &config)
{
	coh3002t_t2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &taitogn_state::main_mp_map);
}

void taitogn_state::coh3002t_cf(machine_config &config)
{
	coh3002t(config);
	m_pccard->set_default_option("taitocf");
}


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
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "S551:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("S551:2") // bios testmode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "S551:3" )
	PORT_DIPNAME( 0x08, 0x08, "Test Mode" )             PORT_DIPLOCATION("S551:4") // game testmode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "ANALOG1" )
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "ANALOG2" )
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "BIOS Flash" )            PORT_DIPLOCATION("JP1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(gobyrc)
	PORT_INCLUDE(coh3002t)

	PORT_MODIFY( "ANALOG1" )
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX( 0x00, 0xff ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 30 ) PORT_NAME("Wheel")

	PORT_MODIFY( "ANALOG2" )
	PORT_BIT( 0xff, 0x80, IPT_PADDLE_V ) PORT_MINMAX( 0x00, 0xff ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 30 ) PORT_NAME("Trigger") PORT_REVERSE

	//8006FF70
	//PORT_START( "ID" )
	//PORT_CONFNAME( 0x03, 0x00, "ID" )
	//PORT_CONFSETTING( 0, "1" )
	//PORT_CONFSETTING( 1, "2" )
	//PORT_CONFSETTING( 2, "3" )
	//PORT_CONFSETTING( 3, "4" )
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

static INPUT_PORTS_START(coh3002t_jp1)
	PORT_INCLUDE(coh3002t)
	PORT_MODIFY("JP1")
	PORT_DIPNAME( 0x01, 0x01, "BIOS Flash")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

//

#define TAITOGNET_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "m534002c-60.ic353", 0x000000, 0x080000, CRC(03967fa7) SHA1(0e17fec2286e4e25deb23d40e41ce0986f373d49) ) \
	ROM_REGION( 0x8, "cat702_1", 0) \
	ROM_LOAD( "tt10.ic652", 0x000000, 0x000008, CRC(235510b1) SHA1(2cc02113207a8f0b078152d31ce6503411750e70) ) \
	ROM_REGION( 0x8, "cat702_2", 0) \
	ROM_LOAD( "tt16", 0x000000, 0x000008, CRC(6bb167b3) SHA1(9dcba08f10775a9adf2b1f382c947460edd3d239) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */ \
	ROM_REGION16_LE( 0x200000, "biosflash", 0 ) \
	ROM_LOAD( "flash.u30", 0x000000, 0x200000, CRC(c48c8236) SHA1(c6dad60266ce2ff635696bc0d91903c543273559) ) \
	ROM_REGION16_LE( 0x100000, "bioseprom", 0 ) \
	ROM_SYSTEM_BIOS( 0, "v1", "G-NET BIOS v1 flasher" ) \
	ROMX_LOAD( "f35-01_m27c800_v1.bin", 0x000000, 0x100000, CRC(cd15cc30) SHA1(78361f46fa7186d5058937c86c66247a86b1257f), ROM_BIOS(0) ) /* hand made */ \
	ROM_SYSTEM_BIOS( 1, "v2", "G-NET BIOS v2 flasher" ) \
	ROMX_LOAD( "f35-01_m27c800.bin", 0x000000, 0x100000, CRC(6225ec11) SHA1(047852d456b6ff85f8e640887caa03cf3e63ffad), ROM_BIOS(1) ) \
	ROM_REGION( 0x80000, "taito_zoom:mn10200", 0 ) \
	ROM_FILL( 0, 0x80000, 0xff ) \
	ROM_REGION32_LE( 0x600000, "taito_zoom:zsg2", 0 ) \
	ROM_FILL( 0, 0x600000, 0xff )

ROM_START( coh3002t )
	TAITOGNET_BIOS
ROM_END

/* Taito */

ROM_START(raycris)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "raycris", 0, SHA1(9d255710c87c3286542d357820d828807cc6ca07))
ROM_END

ROM_START(raycrisj)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "raycrisj", 0, SHA1(015cb0e6c4421cc38809de28c4793b4491386aee))
ROM_END

ROM_START(gobyrc)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard2" )
	DISK_IMAGE( "gobyrc", 0, SHA1(0bee1f495fc8b033fd56aad9260ae94abb35eb58))
ROM_END

ROM_START(rcdego)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "rcdego", 0, SHA1(9e177f2a3954cfea0c8c5a288e116324d10f5dd1))
ROM_END

ROM_START(chaoshea)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "chaosheat", 0, SHA1(c13b7d7025eee05f1f696d108801c7bafb3f1356))
ROM_END

ROM_START(chaosheaj)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "chaosheatj", 0, SHA1(2f211ac08675ea8ec33c7659a13951db94eaa627))
ROM_END

ROM_START(flipmaze)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "flipmaze", 0, SHA1(423b6c06f4f2d9a608ce20b61a3ac11687d22c40) )
ROM_END

ROM_START(spuzbobl)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard2" )
	DISK_IMAGE( "spuzbobl", 0, SHA1(1b1c72fb7e5656021485fefaef8f2ba48e2b4ea8))
ROM_END

ROM_START(spuzboblj)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard2" )
	DISK_IMAGE( "spuzbobj", 0, SHA1(dac433cf88543d2499bf797d7406b82ae4338726))
ROM_END

ROM_START(soutenry)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "soutenry", 0, SHA1(9204d0be833d29f37b8cd3fbdf09da69b622254b))
ROM_END

ROM_START(shanghss)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "shanghss", 0, SHA1(7964f71ec5c81d2120d83b63a82f97fbad5a8e6d))
ROM_END

ROM_START(sianniv)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "sianniv", 0, SHA1(1e08b813190a9e1baf29bc16884172d6c8da7ae3))
ROM_END

ROM_START(kollon)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "kollon", 0, SHA1(d8ea5b5b0ee99004b16ef89883e23de6c7ddd7ce))
ROM_END

ROM_START(kollonc)
	TAITOGNET_BIOS
	ROM_DEFAULT_BIOS( "v2" )

	DISK_REGION( "pccard:taitocf" )
	DISK_IMAGE( "kollonc", 0, SHA1(ce62181659701cfb8f7c564870ab902be4d8e060)) /* Original Taito Compact Flash version */
ROM_END

ROM_START(shikigam)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "shikigam", 0, SHA1(fa49a0bc47f5cb7c30d7e49e2c3696b21bafb840))
ROM_END

ROM_START(shikigama)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "shikigama", 0, SHA1(a6fe194c86730963301be9710782ca4ac1bf3e8d))
ROM_END


/* Success */

ROM_START(otenamih)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "otenamih", 0, SHA1(b3babe3a1876c43745616ee1e7d87276ce7dad0b) )
ROM_END

ROM_START(psyvaria)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "psyvaria", 0,  SHA1(3c7fca5180356190a8bf94b22a847fdd2e6a4e13))
ROM_END

ROM_START(psyvarij)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "psyvarij", 0,  SHA1(b981a42a10069322b77f7a268beae1d409b4156d))
ROM_END

ROM_START(psyvarrv)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "psyvarrv", 0, SHA1(277c4f52502bcd7acc1889840962ec80d56465f3))
ROM_END

ROM_START(zooo)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "zooo", 0, SHA1(e275b3141b2bc49142990e6b497a5394a314a30b))
ROM_END

ROM_START(zokuoten)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard2" )
	DISK_IMAGE( "zokuoten", 0, SHA1(116e58c90f39a3c18ca6fe0216c998ba02c58814))
ROM_END

ROM_START(zokuotena)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "zokuotena", 0, SHA1(5ce13db00518f96af64935176c71ec68d2a51938))
ROM_END

ROM_START(otenamhf)
	TAITOGNET_BIOS
	ROM_DEFAULT_BIOS( "v2" )

	DISK_REGION( "pccard:taitocf" )
	DISK_IMAGE( "otenamhf", 0, SHA1(5b15c33bf401e5546d78e905f538513d6ffcf562)) /* Original Taito Compact Flash version */
ROM_END


/* Takumi */

ROM_START(nightrai)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "nightrai", 0, SHA1(74d0458f851cbcf10453c5cc4c47bb4388244cdf))
ROM_END

ROM_START(otenki)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "otenki", 0, SHA1(7e745ca4c4570215f452fd09cdd56a42c39caeba))
ROM_END


/* Warashi */

ROM_START(usagi)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard2" )
	DISK_IMAGE( "usagi", 0, SHA1(edf9dd271957f6cb06feed238ae21100514bef8e))
ROM_END

ROM_START(mahjngoh)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "mahjngoh", 0, SHA1(3ef1110d15582d7c0187438d7ad61765dd121cff))
ROM_END

ROM_START(shangtou)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "shanghaito", 0, SHA1(9901db5a9aae77e3af4157aa2c601eaab5b7ca85) )
ROM_END


/* Triangle Service */

ROM_START(xiistag)
	TAITOGNET_BIOS

	DISK_REGION( "pccard:taitopccard1" )
	DISK_IMAGE( "xiistag", 0, SHA1(586e37c8d926293b2bd928e5f0d693910cfb05a2))
ROM_END

} // aononymous namespace


/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-3002t.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1997, coh3002t,  0,        coh3002t,       coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Taito GNET", MACHINE_IS_BIOS_ROOT )

/* Taito */
GAME( 1998, chaoshea,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Chaos Heat (V2.09O 1998/10/02 17:00)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, chaosheaj, chaoshea, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Chaos Heat (V2.08J 1998/09/25 17:00)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, raycris,   coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Ray Crisis (V2.03O 1998/11/15 15:43)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, raycrisj,  raycris,  coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Ray Crisis (V2.03J 1998/11/15 15:43)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, spuzbobl,  coh3002t, coh3002t_t2,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Super Puzzle Bobble (V2.05O 1999/2/24 18:00)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, spuzboblj, spuzbobl, coh3002t_t2,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Super Puzzle Bobble (V2.04J 1999/2/27 02:10)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, gobyrc,    coh3002t, coh3002t_t2,    gobyrc,       taitogn_state, empty_init, ROT0,   "Taito", "Go By RC (V2.03O 1999/05/25 13:31)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, rcdego,    gobyrc,   coh3002t_t1,    gobyrc,       taitogn_state, empty_init, ROT0,   "Taito", "RC De Go (V2.03J 1999/05/22 19:29)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, flipmaze,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "MOSS / Taito", "Flip Maze (V2.04J 1999/09/02 20:00)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, shikigam,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Alfa System / Taito", "Shikigami no Shiro (V2.03J 2001/08/07 18:11)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, shikigama, coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Alfa System / Taito", "Shikigami no Shiro - internal build (V1.02J 2001/09/27 18:45)", MACHINE_IMPERFECT_SOUND )
GAME( 2003, sianniv,   coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, init_nozoom,ROT270, "Taito", "Space Invaders Anniversary (V2.02J)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // IRQ at the wrong time
GAME( 2003, kollon,    coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Taito", "Kollon (V2.04JA 2003/11/01 12:00)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // similar lockup problem as sianniv
GAME( 2003, kollonc,   kollon,   coh3002t_cf,    coh3002t_jp1, taitogn_state, empty_init, ROT0,   "Taito", "Kollon (V2.04JC 2003/11/01 12:00)", MACHINE_IMPERFECT_SOUND )

/* Success */
GAME( 1999, otenamih,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, init_nozoom,ROT0,   "Success", "Otenami Haiken (V2.04J 1999/02/01 18:00:00)", 0 )
GAME( 2000, psyvaria,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Success", "Psyvariar -Medium Unit- (V2.02O 2000/02/22 13:00)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, psyvarij,  psyvaria, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Success", "Psyvariar -Medium Unit- (V2.04J 2000/02/15 11:00)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, psyvarrv,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Success", "Psyvariar -Revision- (V2.04J 2000/08/11 22:00)", MACHINE_IMPERFECT_SOUND )
GAME( 2003, zokuoten,  coh3002t, coh3002t_t2,    coh3002t,     taitogn_state, init_nozoom,ROT0,   "Success", "Zoku Otenamihaiken (V2.05J 2003/05/12 18:00)", 0 )
GAME( 2001, zokuotena, zokuoten, coh3002t_t1,    coh3002t,     taitogn_state, init_nozoom,ROT0,   "Success", "Zoku Otenamihaiken (V2.03J 2001/02/16 16:00)", 0 ) // boots the soundcpu without any valid code, causing an infinite NMI loop (currently circumvented)
GAME( 2004, zooo,      coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, init_nozoom,ROT0,   "Success", "Zooo (V2.01JA 2004/04/13 12:00)", 0 )
GAME( 2005, otenamhf,  coh3002t, coh3002t_cf,    coh3002t_jp1, taitogn_state, init_nozoom,ROT0,   "Success / Warashi", "Otenami Haiken Final (V2.07JC 2005/04/20 15:36)", 0 )

/* Takumi */
GAME( 2001, nightrai,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Takumi", "Night Raid (V2.03J 2001/02/26 17:00)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, otenki,    coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Takumi", "Otenki Kororin (V2.01J 2001/07/02 10:00)", MACHINE_IMPERFECT_SOUND )

/* Warashi */
GAME( 1999, mahjngoh,  coh3002t, coh3002t_t1_mp, coh3002t_mp,  taitogn_state, empty_init, ROT0,   "Warashi / Mahjong Kobo / Taito", "Mahjong Oh (V2.06J 1999/11/23 08:52:22)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, shanghss,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Warashi", "Shanghai Shoryu Sairin (V2.03J 2000/05/26 12:45:28)", MACHINE_IMPERFECT_SOUND )
GAME( 2000, soutenry,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Warashi", "Soutenryu (V2.07J 2000/12/14 11:13:02)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, usagi,     coh3002t, coh3002t_t2_mp, coh3002t_mp,  taitogn_state, empty_init, ROT0,   "Warashi / Mahjong Kobo / Taito", "Usagi (V2.02J 2001/10/02 12:41:19)", MACHINE_IMPERFECT_SOUND ) // missing transparencies, see MT #06258
GAME( 2002, shangtou,  coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT0,   "Warashi / Sunsoft / Taito", "Shanghai Sangokuhai Tougi (Ver 2.01J 2002/01/18 18:26:58)", MACHINE_IMPERFECT_SOUND )

/* Triangle Service */
GAME( 2002, xiistag,   coh3002t, coh3002t_t1,    coh3002t,     taitogn_state, empty_init, ROT270, "Triangle Service", "XII Stag (V2.01J 2002/6/26 22:27)", MACHINE_IMPERFECT_SOUND )
