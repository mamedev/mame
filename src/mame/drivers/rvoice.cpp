// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Kevin Horton
/******************************************************************************
*
*  Bare bones Realvoice PC driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  Binary supplied by Kevin 'kevtris' Horton
*

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "machine/mos6551.h"
#include "machine/terminal.h"

/* Components */

class rvoice_state : public driver_device
{
public:
	rvoice_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
			{ }

	void rvoicepc(machine_config &config);

	void init_rvoicepc();

private:
	virtual void machine_reset() override;
	void null_kbd_put(u8 data);
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	void hd63701_main_mem(address_map &map);
};


/* Devices */

void rvoice_state::init_rvoicepc()
{
}

void rvoice_state::machine_reset()
{
}


/******************************************************************************
 Address Maps
******************************************************************************/

void rvoice_state::hd63701_main_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6301y_cpu_device::hd6301y_io)); // INTERNAL REGS
	map(0x0040, 0x005f).ram(); // INTERNAL RAM (overlaps acia)
	map(0x0060, 0x007f).rw("acia65c51", FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // ACIA 65C51
	map(0x0080, 0x013f).ram(); // INTERNAL RAM (overlaps acia)
	map(0x2000, 0x7fff).ram(); // EXTERNAL SRAM
	map(0x8000, 0xffff).rom(); // 27512 EPROM
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( rvoicepc )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/
void rvoice_state::null_kbd_put(u8 data)
{
}

void rvoice_state::rvoicepc(machine_config &config)
{
	/* basic machine hardware */
	HD63701Y0(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_main_mem);

	//hd63701_cpu_device &playercpu(HD63701(config "playercpu", XTAL(7'372'800))); // not dumped yet
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_mem);
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_io);
	config.set_maximum_quantum(attotime::from_hz(60));

	mos6551_device &acia(MOS6551(config, "acia65c51", 0));
	acia.set_xtal(1.8432_MHz_XTAL);

	/* video hardware */

	/* sound hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(rvoice_state::null_kbd_put));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(rvoicepc)

	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rv_pc.bin", 0x08000, 0x08000, CRC(4001cd5f) SHA1(d973c6e19e493eedd4f7216bc530ddb0b6c4921e))
	ROM_CONTINUE(0x8000, 0x8000) // first half of 27c512 rom is blank due to stupid address decoder circuit

ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS         INIT           COMPANY                           FULLNAME        FLAGS
COMP( 1988?, rvoicepc, 0,      0,      rvoicepc, rvoicepc, rvoice_state, init_rvoicepc, "Adaptive Communication Systems", "Realvoice PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
