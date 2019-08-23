// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Scorpion 5

    Skeleton Driver - For note keeping.

    This system is not emulated.

 SCORPION 5 PCB
 _________________________________________________________________________
 | ______________________  _____________  _____________  _________    __  |
 | |____PLUG_J_LAMPS_____| |___PLUG_K___| |___PLUG_L___| |_PLUG_M_|   |P| |
 |                                                                    |L| |
 | D20NF D20NF D20NF D20NF D20NF D20NF D20NF                          |N| |
 | D20NF D20NF D20NF D20NF D20NF D20NF D20NF                          __  |
 | __          D20NF  D20NF                    MM74HCT                |P| |
 | |P|                                                                |L| |
 | |L| ______  ______ ______  ______  ULN28030AG  CS4341AKSZ          |P| |
 | |U| ULN28030AG x 2 UDN2982LWT x 2                                  __  |
 | |G|               ____ ____ ____   ULN28030AG                      |P| |
 | | |               HC14 HC14 HC14  _________  __________  ___       |L| |
 | |F|               ______  ______  |AMI S   | |YAMAHA   | |P |      |Q| |
 | |_|               UDN2982LWT x 2  |0728DC0 | |YMZ280B-F| |L |      __  |
 | __   _____  _____                 |BFG206ML| |_________| |U | IC20 |P| |
 | |P|  |    | |    |                 62215-001      XTL1   |G |      |L| |
 | |L|  |DB9 | |DB9 |                                       |  | IC19 |R| |
 | |E|  |MALE| |FEMALE                 ______   ___________ |Z |      |_| |
 | |_|  |____| |____| 74HCT00          |NXP  |  |COLDFIRE  ||  |  __  __  |
 | __            ___      XTL3  SCC68681C1A44|  |MCF5206EAB40  |  |P| |P| |
 | |P|           |SW|  o <- LED        |_____|  |          ||  |  |L| |L| |
 | |L|           | 4| __                        |          ||  |  |U| |U| |
 | |U|           |__| |_| SW RESET              |__________||  |  |G| |G| |
 | |G|           |SW| |_| SW TEST      ____  ______  ______ |  |  | | | | |
 | | |           | 1|                  |GAL| IS62C256AL x 2 |__|  |U| |S| |
 | |D|           |__|                  |___|                          __  |
 | |_|                 o o o o o o o <- LEDS                     XTL2 |P| |
 |  _______ _______ ____________________                   _______    |L| |
 | |PLUG C||PLUG B| |______PLUG A_______|    BATTERY 3.6V  |PLUG V|   |T| |
 |________________________________________________________________________|

SW1 = 8 dipswitches
SW4 = 8 dipswitches

IC20 = MC1489DG
IC19 = SN75188

XTL1 = 16.9344 MHz
XTL2 = 40.0000 MHz
XTL3 = 3.6864 MHz

PLUG A = POWER IN
PLUG B = CAB SWITCHES
PLUG C = REEL INPUTS
PLUG D = REEL OUTPUTS
PLE = PLUG E = GAME SWITCHES
PLUG F = L.E.D.S
PLUG J = LAMPS
PLUG K = REEL LAMPS
PLUG L = GENERAL I/O
PLUG M = AUDIO OUTPUT
PLN = PLUG N = ALPHA
PLP = PLUG P = SEC METER
PLQ = PLUG Q = CCTalk I/F
PLR = PLUG R = RS232 PORT 2
PLUG S = RS232 PORT 1 (DB25 female)
PLT = PLUG T = I2C I/F
PLUG U = BDM I/F
PLUG V = I2C I/F
PLUG Z = GAME CARD / VIDEO CARD


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

*/


#include "emu.h"
#include "includes/bfm_sc5.h"

#include "machine/bfm_sc45_helper.h"
#include "machine/mcf5206e.h"
#include "video/awpvid.h"
#include "speaker.h"

#include "bfm_sc5.lh"



WRITE16_MEMBER( bfm_sc5_state::sc5_duart_w )
{
	// clearly a duart of some kind, write patterns are the same as SC4 games
//  logerror("%s: duart_w %1x %04x %04x\n", machine().describe_context(), offset, data, mem_mask);

	if (ACCESSING_BITS_8_15)
	{
		m_duart->write(offset,(data>>8)&0x00ff);
	}
	else
	{
		logerror("%s: duart_w %1x %04x %04x\n", machine().describe_context(), offset, data, mem_mask);
	}

}

READ8_MEMBER( bfm_sc5_state::sc5_mux1_r )
{
	switch (offset)
	{
		case 0x20:
			return machine().rand();
	}

	logerror("%s: sc5_mux1_r %1x\n", machine().describe_context(), offset);

	return 0x00;
}


WRITE8_MEMBER( bfm_sc5_state::sc5_mux1_w )
{
	if ((offset&0xf)==0)
	{
		mux_output_w(space, (offset & 0x01f0)>>4, data);
	}
	else
	{
		logerror("%s: sc5_mux1_w %1x %04x\n", machine().describe_context(), offset, data);
	}
}



WRITE8_MEMBER( bfm_sc5_state::sc5_mux2_w )
{
	if ((offset&0xf)==0)
	{
		mux_output2_w(space, (offset & 0x01f0)>>4, data);
	}
	else
	{
		logerror("%s: sc5_mux2_w %1x %04x\n", machine().describe_context(), offset, data);
	}
}


void bfm_sc5_state::sc5_map(address_map &map)
{
	// ROM (max size?)
	map(0x00000000, 0x002fffff).rom();
	// ?
	map(0x01000000, 0x0100ffff).ram();

#if 1
	// dev1
	map(0x01010000, 0x010101ff).rw(FUNC(bfm_sc5_state::sc5_mux1_r), FUNC(bfm_sc5_state::sc5_mux1_w)); // guess
#endif

#if 0

	map(0x01010200, 0x01010203).nopw();
	map(0x01010210, 0x01010213).nopw();
	map(0x01010220, 0x01010223).nopw();
	map(0x01010230, 0x01010233).nopw();

	map(0x01010280, 0x01010283).nopw();

	map(0x010102a0, 0x010102a3).nopw();

	map(0x010102c0, 0x010102c3).nopw();

	map(0x010102f0, 0x010102f3).nopw();

	map(0x01010300, 0x01010303).nopw();

	map(0x01010330, 0x01010333).nopw();

	map(0x01010360, 0x01010363).nopw();

	map(0x01010380, 0x01010383).nopw();
	map(0x01010390, 0x01010393).nopw();
#endif

#if 1
	// dev2
	map(0x01020000, 0x010201ff).w(FUNC(bfm_sc5_state::sc5_mux2_w)); // guess
#endif

#if 0

	map(0x01020200, 0x01020203).nopw();
	map(0x01020210, 0x01020213).nopw();
	map(0x01020220, 0x01020223).nopw();
	map(0x01020230, 0x01020233).nopw();

	map(0x01020280, 0x01020283).nopw();

	map(0x010202a0, 0x010202a3).nopw();
	map(0x010202b0, 0x010202b3).nopw();
	map(0x010202c0, 0x010202c3).nopw();
#endif
	map(0x010202F0, 0x010202F3).rw(FUNC(bfm_sc5_state::sc5_10202F0_r), FUNC(bfm_sc5_state::sc5_10202F0_w));
#if 1
	map(0x01020330, 0x01020333).nopw();

	map(0x01020350, 0x01020353).nopw();
	map(0x01020360, 0x01020363).nopw();
	map(0x01020370, 0x01020373).nopw();

	map(0x01020390, 0x01020393).nopw();
#endif
	map(0x02000000, 0x0200001f).w(FUNC(bfm_sc5_state::sc5_duart_w));

	// ram
	map(0x40000000, 0x4000ffff).ram();

	// peripherals
	map(0xffff0000, 0xffff03ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::dev_r), FUNC(mcf5206e_peripheral_device::dev_w)); // technically this can be moved with MBAR
}

INPUT_PORTS_START( bfm_sc5 )
INPUT_PORTS_END

READ8_MEMBER( bfm_sc5_state::sc5_10202F0_r )
{
	switch (offset)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
			logerror("%s: sc5_10202F0_r %d\n", machine().describe_context(), offset);
			return machine().rand();
	}

	return 0;
}

WRITE8_MEMBER( bfm_sc5_state::sc5_10202F0_w )
{
	switch (offset)
	{
		case 0x0:
			bfm_sc45_write_serial_vfd((data &0x4)?1:0, (data &0x1)?1:0, (data&0x2) ? 0:1);
			if (data&0xf8) logerror("%s: sc5_10202F0_w %d - %02x\n", machine().describe_context(), offset, data);
			break;
		case 0x1:
		case 0x2:
		case 0x3:
			logerror("%s: sc5_10202F0_w %d - %02x\n", machine().describe_context(), offset, data);
			break;
	}
}


WRITE_LINE_MEMBER(bfm_sc5_state::bfm_sc5_duart_irq_handler)
{
	logerror("bfm_sc5_duart_irq_handler\n");
}

WRITE_LINE_MEMBER(bfm_sc5_state::bfm_sc5_duart_txa)
{
	logerror("bfm_sc5_duart_tx\n");
}

READ8_MEMBER(bfm_sc5_state::bfm_sc5_duart_input_r)
{
	logerror("bfm_sc5_duart_input_r\n");
	return 0xff;
}

WRITE8_MEMBER(bfm_sc5_state::bfm_sc5_duart_output_w)
{
	logerror("bfm_sc5_duart_output_w\n");
}

void bfm_sc5_state::bfm_sc5(machine_config &config)
{
	MCF5206E(config, m_maincpu, 40000000); /* MCF5206eFT */
	m_maincpu->set_addrmap(AS_PROGRAM, &bfm_sc5_state::sc5_map);
	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MC68681(config, m_duart, 16000000/4); // ?? Mhz
	m_duart->set_clocks(16000000/2/8, 16000000/2/16, 16000000/2/16, 16000000/2/8);
	m_duart->irq_cb().set(FUNC(bfm_sc5_state::bfm_sc5_duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(bfm_sc5_state::bfm_sc5_duart_txa));
	m_duart->inport_cb().set(FUNC(bfm_sc5_state::bfm_sc5_duart_input_r));;
	m_duart->outport_cb().set(FUNC(bfm_sc5_state::bfm_sc5_duart_output_w));;

	BFM_BDA(config, m_vfd0, 60, 0);

	config.set_default_layout(layout_bfm_sc5);

	YMZ280B(config, m_ymz, 16000000); // ?? Mhz
	m_ymz->add_route(ALL_OUTPUTS, "mono", 1.0);
}

#include "bfm_sc5sw.hxx"
