// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  Votrax Type 'N Talk Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with loads of help (dumping, code disassembly, schematics, and other
*  documentation) from Kevin 'kevtris' Horton
*
*  The Votrax TNT was sold from some time in early 1981 until at least 1983.
*
*  2011-02-27 The TNT communicates with its host via serial (RS-232) by
*             using a 6850 ACIA. It will echo whatever is sent, back to the
*             host. In order that we can examine it, I have connected the
*             'terminal' so we can enter data, and see what gets sent back.
*             I've also connected the 'votrax' device, but it is far from
*             complete (the sounds are not understandable).
*
*             ACIA status code is set to E2 for no data and E3 for data.
*
*             On the CPU, A12 and A15 are not connected. A13 and A14 are used
*             to select devices. A0-A9 address RAM. A0-A11 address ROM.
*             A0 switches the ACIA between status/command, and data in/out.
*
******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "machine/votraxtnt.h"

#include "votrtnt.lh"


namespace {

class votrtnt_state : public driver_device
{
public:
	votrtnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void votrtnt(machine_config &config);
};


/******************************************************************************
 Machine Drivers
******************************************************************************/

void votrtnt_state::votrtnt(machine_config &config)
{
	/* basic machine hardware */
	votraxtnt_device &votrax(VOTRAXTNT(config, "votrax"));
	votrax.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	votrax.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	/* video hardware */
	//config.set_default_layout(layout_votrtnt);

	/* serial hardware */
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("votrax", FUNC(votraxtnt_device::write_rxd));
	rs232.cts_handler().set("votrax", FUNC(votraxtnt_device::write_cts));
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrtnt)
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY   FULLNAME        FLAGS
COMP( 1980, votrtnt, 0,      0,      votrtnt, 0,       votrtnt_state, empty_init, "Votrax", "Type 'N Talk", MACHINE_SUPPORTS_SAVE )
