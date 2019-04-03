// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Adder 5
     (Scorpion 5 Video Board)

    Skeleton Driver - For note keeping.

    This system is not emulated.

*/

#include "emu.h"
#include "includes/bfm_ad5.h"
#include "includes/bfm_sc4.h"
#include "machine/mcf5206e.h"
#include "machine/bfm_sc45_helper.h"
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
//  m_maincpu->set_input_line_and_vector(5, HOLD_LINE, 0x8c);
}

void adder5_state::bfm_ad5(machine_config &config)
{
	MCF5206E(config, m_maincpu, 40000000); /* MCF5206eFT */
	m_maincpu->set_addrmap(AS_PROGRAM, &adder5_state::ad5_map);
	m_maincpu->set_periodic_int(FUNC(adder5_state::ad5_fake_timer_int), attotime::from_hz(1000));
	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	/* unknown sound */
}

#include "bfm_ad5sw.hxx"
