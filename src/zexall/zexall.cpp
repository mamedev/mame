// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Robbbert, Miodrag Milanovic
/******************************************************************************

  This is a simplified version of the zexall driver, merely as an example for a standalone
  emulator build. Video terminal and user interface is removed. For full notes and proper
  emulation driver, see src/mame/homebrew/zexall.cpp.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "zexall.h"
#include "interface.h"

class zexall_state : public driver_device
{
public:
	zexall_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_main_ram(*this, "main_ram")
	{
	}

	uint8_t output_ack_r();
	uint8_t output_req_r();
	uint8_t output_data_r();
	void output_ack_w(uint8_t data);
	void output_req_w(uint8_t data);
	void output_data_w(uint8_t data);

	void z80_mem(address_map &map) ATTR_COLD;
	void zexall(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_main_ram;
	uint8_t m_out_data; // byte written to 0xFFFF
	uint8_t m_out_req; // byte written to 0xFFFE
	uint8_t m_out_req_last; // old value at 0xFFFE before the most recent write
	uint8_t m_out_ack; // byte written to 0xFFFC
	std::string terminate_string;

	virtual void machine_reset() override ATTR_COLD;
};


/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void zexall_state::machine_reset()
{
	// zerofill
	m_out_ack = 0;
	m_out_req = 0;
	m_out_req_last = 0;
	m_out_data = 0;
	terminate_string = "";

	// program is self-modifying, so need to refresh it on each run
	memset(m_main_ram, 0xff, 0x10000);
	memcpy(m_main_ram, interface_binary, 0x51);
	memcpy(m_main_ram + 0x0100, zexall_binary, 0x2189);
}


/******************************************************************************
 I/O Handlers
******************************************************************************/

uint8_t zexall_state::output_ack_r()
{
	// spit out the byte in out_byte if out_req is not equal to out_req_last
	if (m_out_req != m_out_req_last)
	{
		osd_printf_info("%c",m_out_data);
		if (m_out_data != 10 && m_out_data != 13)
			terminate_string += m_out_data;
		else
			terminate_string = "";

		if (terminate_string == "Tests complete")
			machine().schedule_exit();

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

void zexall_state::z80_mem(address_map &map)
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
	m_maincpu->set_addrmap(AS_PROGRAM, &zexall_state::z80_mem);
}


/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(zexall)
	ROM_REGION(0x0, "maincpu", 0)
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT      COMPAT  MACHINE   INPUT   STATE         INIT        COMPANY                         FULLNAME                            FLAGS */
COMP( 2009, zexall,   0,          0,      zexall,   zexall, zexall_state, empty_init, "Frank Cringle / Kevin Horton", "Zexall (FPGA Z80 test interface)", MACHINE_NO_SOUND_HW )
