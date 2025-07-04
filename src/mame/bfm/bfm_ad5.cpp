// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Adder 5
     (Scorpion 5 Video Board)

    Skeleton Driver - For note keeping.

    This system is not emulated.

ISS 3 ADDER 5 VIDEO CARD

       COMPONENT SIDE                           SOLDER SIDE
  ___________________________            ___________________________
 | _______  _______ _______ |            |                          |
 ||      | |__CN2_||__CN1_| |            |                          |
 || IC3  |     MAX202 MAX202|            |                          |
 ||______|  XTL3            |            |                          |
 |  _________________       |___      ___|                          |
 | |                | ___   |__|      |__|                          |
 | | ROM 2 IC7      | |IC8  |__|      |__|                          |
 | |________________|       |__| PLG1 |__|                          |
 |  _________________       |__|      |__|                          |
 | |                |       |__|      |__|                          |
 | | ROM 1 IC6      | XTL2  |__|      |__|                          |
 | |________________|       |__|      |__|                          |
 | _______________________  |__|      |__|  ____                    |
 ||_|_|_|_|__PLG3__|_|_|_|  |__|      |__|  |IC | ____________      |
 |    ____                  |__|      |__|  |11 | |     ______|     |
 |   |IC5|                  |__|      |__|  |___| |    |__          |
 |   |___|              XTL1|__|      |__|  ____  |    |__          |
 |  __________________      |__|      |__|  |IC | |    |__          |
 |  |                 |     |__|      |__|  | 9 | |PL1 |__          |
 |  | IC1             |     |__|      |__|  |___| |    |__          |
 |  | YAMAHA          |     |__|      |__|  ____  |    |__          |
 |  | YGV619-V        |     |__|      |__|  |IC | |    |__          |
 |  |                 |     |__|      |__|  |10 | |    |__          |
 |  |                 |     |            |  |___| |    |______      |
 |  |_________________|     |            |        |___________|     |
 | _____    _____       ____|            |____                      |
 | |    |   |    |     |                      |                     |
 | | IC2|   | IC4|     |                      |                     |
 | |    |   |    |     |                      |                     |
 | |____|   |____|     |                      |                     |
 |             ______  |                      |  ______             |
 |_____________|     |_|                      |_|     |_____________|
              | PLG2  |                        |       |
              |_______|                        |_______|

XTL1 = 20.00 MHz
XTL2 = 20.00 MHz
XTL3 = 3.686 MHz
IC2 = Samsung K4S641632H-UC75
IC3 = Philips / NXP SCC6861C1A44
IC4 = Samsung K4S641632H-UC75
IC5 = GAL16V8D
IC8 = PIC (for external Serial EEPROM)
IC9 = LCX245
IC10 = LCX245
IC11 = 74HCT2730
PLG1 = To mother board
PLG2 = VGA OUT (HD15)
PLG3 = Flash (for pluging flash ROMs)
CN1 = RS232 Port 3
CN2 = RS232 Port 4
PL1 = Compact Flash Slot

Flash ROMs are plugged into PLG3 on a small sub-board with the following pinout:

+----------------+
| +--+           |
| |01|           |
| |::|  +------+ |
| |::|  |      | |
| |::|  |28F128| |
| |::|  |      | |
| |::|  |      | |
| |::|  +------+ |
| |30|           |
| +--+           |
+----------------+

GND |01| GND
GND |02| GND
WE  |03| A23
STS |04| OE
D07 |05| D15
D06 |06| D14
D13 |07| GND
D12 |08| D05
VCCq|09| D04
D11 |10| GND
D10 |11| D03
VCC |12| D02
D01 |13| D09
D00 |14| D08
BYTE|15| NC
CE2 |16| A22
A01 |17| A00
A03 |18| A02
A05 |19| A04
GND |20| A06
A08 |21| A07
A10 |22| A09
Vpen|23| RP
A11 |24| CE0
A13 |25| A12
VCC |26| A14
A16 |27| A15
A18 |28| A17
A20 |29| A19
A21 |30| CE1

*/

#include "emu.h"
#include "bfm_ad5.h"
#include "machine/mcf5206e.h"
#include "bfm_sc45_helper.h"
#include "speaker.h"

void adder5_state::init_ad5()
{
	// sc5 roms always start with SC5
	uint8_t *src = memregion( "maincpu" )->base();
//  printf("%02x %02x %02x %02x\n", src[0], src[1], src[2], src[3]);
	if (((src[0] == 0x20) && (src[2] == 0x43)) || ((src[1] == 0x35) && (src[3] == 0x53)))
	{
		printf("Confirmed SC5 ROM\n");
	}
	else
	{
		printf("NOT AN SC5 ROM!!!!!\n");
	}


	// there is usually a string in the rom with identification info, often also saying which sound rom should be used!
	// find it.
	int found = find_project_string(machine(), 3, 0);
	if (!found)
	{
		printf("Normal rom pair string not found, checking mismatched / missing rom string\n");
	}

	// help identify roms where one of the pair is missing too
	if (!found)
	{
		found = find_project_string(machine(), 3, 1);
	}

	if (!found)
	{
		found = find_project_string(machine(), 3, 2);
	}

	if (!found)
	{
		printf("No suitable string found\n");
	}

}

void adder5_state::ad5_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom();
	map(0x01000000, 0x0100ffff).ram();
	map(0x40000000, 0x40000fff).ram();
	map(0x80000000, 0x8000ffff).ram();
	map(0x80800000, 0x8080ffff).ram();

	map(0xffff0000, 0xffff03ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::dev_r), FUNC(mcf5206e_peripheral_device::dev_w)); // technically this can be moved with MBAR
}

INPUT_PORTS_START( bfm_ad5 )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(adder5_state::ad5_fake_timer_int)
{
	// this should be coming from the Timer / SIM modules of the Coldfire
//  m_maincpu->set_input_line_and_vector(5, HOLD_LINE, 0x8c); // MCF5206E but disabled
}

void adder5_state::bfm_ad5(machine_config &config)
{
	MCF5206E(config, m_maincpu, 40000000); /* MCF5206eFT */
	m_maincpu->set_addrmap(AS_PROGRAM, &adder5_state::ad5_map);
	m_maincpu->set_periodic_int(FUNC(adder5_state::ad5_fake_timer_int), attotime::from_hz(1000));
	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0, m_maincpu);

	SPEAKER(config, "speaker", 2).front();
	/* unknown sound */
}
