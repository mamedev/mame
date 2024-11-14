// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Stuart's Breadboard Project. TMS9995 evaluation kit TMAM6095.

2013-06-02 Skeleton driver.

http://www.stuartconner.me.uk/tms9995_eval_module/tms9995_eval_module.htm

It uses TMS9902 UART for comms, but our implementation of that chip is
not ready for rs232.h as yet.

Press any key to get it started. All input to be in uppercase. Instructions
for Hello World are on the EVMBUG page:

http://www.stuartconner.me.uk/tibug_evmbug/tibug_evmbug.htm#evmbug_example

Pay close attention to "On lines where no label is to be input,
remember to press <Space> to step over the symbol field before
entering the instruction." e.g. a space is needed before XOP.

2022-03-05 TMS9995 Breadboard System added by Chris Swan.

http://www.stuartconner.me.uk/tms9995_breadboard/tms9995_breadboard.htm

****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
//#include "machine/tms9902.h"
#include "machine/terminal.h"


namespace {

class evmbug_state : public driver_device
{
public:
	evmbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void evmbug(machine_config &config);
	void tms9995bb(machine_config &config);

private:
	uint8_t rs232_r(offs_t offset);
	void rs232_w(offs_t offset, uint8_t data);
	void kbd_put(u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void evmbug_mem(address_map &map) ATTR_COLD;
	void tms9995bb_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint8_t m_term_data = 0U;
	uint8_t m_term_out = 0U;
	bool m_rin = 0;
	bool m_rbrl = 0;
	required_device<tms9995_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

void evmbug_state::evmbug_mem(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0xec00, 0xefff).ram();
}

// Breadboard system uses 32K ROM and 32K RAM
void evmbug_state::tms9995bb_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).ram();
}

void evmbug_state::io_map(address_map &map)
{
	map.unmap_value_high();
	//map(0x0000, 0x003f).rw("uart1", FUNC(tms9902_device::cruread), FUNC(tms9902_device::cruwrite));
	map(0x0000, 0x003f).rw(FUNC(evmbug_state::rs232_r), FUNC(evmbug_state::rs232_w));
}

/* Input ports */
static INPUT_PORTS_START( evmbug )
INPUT_PORTS_END

uint8_t evmbug_state::rs232_r(offs_t offset)
{
	if (offset < 8)
		return BIT(m_term_data, offset);
	else if (offset == 21)
		return m_rbrl;
	else if (offset == 22 || offset == 23)
		return 1;
	else if (offset == 15)
	{
		m_rin ^= 1;
		return m_rin;
	}
	else
		return 0;
}

void evmbug_state::rs232_w(offs_t offset, uint8_t data)
{
	if (offset < 8)
	{
		if (offset == 0)
			m_term_out = 0;

		m_term_out |= (data << offset);

		if (offset == 7)
			m_terminal->write(m_term_out & 0x7f);
	}
	else
	if (offset == 18)
		m_rbrl = 0;
}

void evmbug_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_rbrl = data ? 1 : 0;
}

void evmbug_state::machine_reset()
{
	m_rbrl = 0;
	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

void evmbug_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_term_out));
	save_item(NAME(m_rin));
	save_item(NAME(m_rbrl));
}

void evmbug_state::evmbug(machine_config &config)
{
	// basic machine hardware
	// TMS9995 CPU @ 12.0 MHz
	// We have no lines connected yet
	TMS9995(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &evmbug_state::evmbug_mem);
	m_maincpu->set_addrmap(AS_IO, &evmbug_state::io_map);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(evmbug_state::kbd_put));

	//TMS9902(config, "uart1", XTAL(12'000'000) / 4);
}

void evmbug_state::tms9995bb(machine_config &config)
{
	evmbug(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &evmbug_state::tms9995bb_mem);
}

/* ROM definition */
ROM_START( evmbug )
	ROM_REGION( 0x1800, "maincpu", 0 )
	ROM_LOAD( "u8.bin", 0x0000, 0x1000, CRC(ca869a70) SHA1(424d8d61ef15645e3ce3867c64a0cfb69633b5bc) )
	ROM_LOAD( "u9.bin", 0x1000, 0x0800, CRC(7f71c9bf) SHA1(5215892585e5282650209c5ce13a2e4bd6041675) )
ROM_END

/* ROMs from and FF padded to make 32K */
ROM_START( tms9995bb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "evmbug", "EVMBUG system monitor")
	ROMX_LOAD( "evmbug.bin",   0x0000, 0x8000, CRC(a239ec56) SHA1(65b500d7d0f897ce0c320cf3ec32ff4042774599), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "basicram", "EVMBUG system monitor and BASIC in RAM")
	ROMX_LOAD( "basicram.bin", 0x0000, 0x8000, CRC(6ed5aba3) SHA1(76e0e39c0c0028efca339fb3cebaf42351fadb94), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "basicrom", "EVMBUG system monitor and BASIC in ROM")
	ROMX_LOAD( "basicrom.bin", 0x0000, 0x8000, CRC(ded6350b) SHA1(83cf64834e59e216a91065c01ec91be1e10e7244), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "forth", "EVMBUG system monitor and Forth")
	ROMX_LOAD( "forth.bin",    0x0000, 0x8000, CRC(eee8f390) SHA1(59fc2a23c9ac52dce09a519c784db378782242b1), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "test1", "Test EPROM 1")
	ROMX_LOAD( "test1.bin",    0x0000, 0x8000, CRC(9b110ffb) SHA1(58ee990fb17822a879442b98f1b78ccf86b79f00), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "test2", "Test EPROM 2")
	ROMX_LOAD( "test2.bin",    0x0000, 0x8000, CRC(e7a7832d) SHA1(aa8c29097033804d1a0abf2af7cd846edfbd71a3), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "test3", "Test EPROM 3")
	ROMX_LOAD( "test3.bin",    0x0000, 0x8000, CRC(a28579eb) SHA1(477f853970f132592714bcdd048ec932e96c8593), ROM_BIOS(6) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME       PARENT     COMPAT  MACHINE     INPUT   CLASS            INIT        COMPANY              FULLNAME              FLAGS
COMP( 198?, evmbug,    0,         0,      evmbug,     evmbug, evmbug_state,    empty_init, "Texas Instruments", "TMAM 6095",          MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 19??, tms9995bb, evmbug,    0,      tms9995bb,  evmbug, evmbug_state,    empty_init, "Stuart Conner",     "TMS9995 breadboard", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
