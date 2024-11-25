// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************

  Self Contained zexall 'Z80 instruction exerciser' test driver
  Zexall originally written by Frank Cringle for ZX Spectrum
  Modularized Spectrum-independent Zexall binary supplied by Blargg
  Serial interface binary/preloader at 0x0000-0x00FF written by Kevin 'kevtris' Horton

  NOTE: there's a modified version of this driver in src/zexall


  Memory map:

  Ram 0000-FFFF (preloaded with binary)
  Special calls take place for three ram values (this interface was designed by kevtris):
  FFFD - 'ack' - shared ram with output device; z80 reads from here and considers the byte at FFFF read if this value incremented
  FFFE - 'req' - shared ram with output device; z80 writes an incrementing value to FFFE to indicate that there is a byte waiting at FFFF
                 and hence requesting the output device on the other end do something about it, until FFFD is incremented by the
                 output device to acknowledge receipt
  FFFF - 'data' - shared ram with output device; z80 writes the data to be sent to output device here
  One i/o port is used, but left unemulated:
  0001 - bit 0 controls whether interrupt timer is enabled (1) or not (0), this is a holdover from a project of kevtris' and can be ignored.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"


namespace {

class zexall_state : public driver_device
{
public:
	zexall_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_main_ram(*this, "main_ram")
	{ }

	void zexall(machine_config &config);

private:
	uint8_t output_ack_r();
	uint8_t output_req_r();
	uint8_t output_data_r();
	void output_ack_w(uint8_t data);
	void output_req_w(uint8_t data);
	void output_data_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint8_t> m_main_ram;
	uint8_t m_out_data = 0U; // byte written to 0xFFFF
	uint8_t m_out_req = 0U; // byte written to 0xFFFE
	uint8_t m_out_req_last = 0U; // old value at 0xFFFE before the most recent write
	uint8_t m_out_ack = 0U; // byte written to 0xFFFC

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};


/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void zexall_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_out_ack));
	save_item(NAME(m_out_req));
	save_item(NAME(m_out_req_last));
	save_item(NAME(m_out_data));
}

void zexall_state::machine_reset()
{
	// zerofill
	m_out_ack = 0;
	m_out_req = 0;
	m_out_req_last = 0;
	m_out_data = 0;

	// program is self-modifying, so need to refresh it on each run
	uint8_t *program = memregion("maincpu")->base();
	memcpy(m_main_ram, program, 0x10000);
}


/******************************************************************************
 I/O Handlers
******************************************************************************/

uint8_t zexall_state::output_ack_r()
{
	// spit out the byte in out_byte if out_req is not equal to out_req_last
	if (m_out_req != m_out_req_last)
	{
		m_terminal->write(m_out_data);
		m_out_req_last = m_out_req;
		m_out_ack++;
	}
	return m_out_ack;
}

void zexall_state::output_ack_w(uint8_t data)
{
	m_out_ack = data;
}

uint8_t zexall_state::output_req_r()
{
	return m_out_req;
}

void zexall_state::output_req_w(uint8_t data)
{
	m_out_req_last = m_out_req;
	m_out_req = data;
}

uint8_t zexall_state::output_data_r()
{
	return m_out_data;
}

void zexall_state::output_data_w(uint8_t data)
{
	m_out_data = data;
}


/******************************************************************************
 Address Maps
******************************************************************************/

void zexall_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("main_ram");
	map(0xfffd, 0xfffd).rw(FUNC(zexall_state::output_ack_r), FUNC(zexall_state::output_ack_w));
	map(0xfffe, 0xfffe).rw(FUNC(zexall_state::output_req_r), FUNC(zexall_state::output_req_w));
	map(0xffff, 0xffff).rw(FUNC(zexall_state::output_data_r), FUNC(zexall_state::output_data_w));
}


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START( zexall )
INPUT_PORTS_END


/******************************************************************************
 Machine Drivers
******************************************************************************/

void zexall_state::zexall(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &zexall_state::mem_map);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START( zexall )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "interface.bin", 0x0000, 0x0051, CRC(4292a574) SHA1(d3ed6d84e2b64e51598f36b4f290972963e1eb6d) ) // written directly in machine code
	ROM_LOAD( "zexall.bin",    0x0100, 0x2189, CRC(b6f869c3) SHA1(14021f75c1bc9f26688969581065a0efff3af59c) )
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                         FULLNAME                            FLAGS
COMP( 2009, zexall, 0,      0,      zexall,  zexall, zexall_state, empty_init, "Frank Cringle / Kevin Horton", "Zexall (FPGA Z80 test interface)", MACHINE_SUPPORTS_SAVE )
