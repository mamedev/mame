// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Imsai MPU-B. One of the earliest single-board computers on a S100 card.

    2013-09-11 Skeleton driver.

    Chips used: i8085, i8251, i8253, 3622 fusable prom. XTAL 6MHz

    Press any key to start the monitor program.

    ToDo:
    - Banking
    - Dipswitches

****************************************************************************/

#include "emu.h"

//#include "bus/s100/s100.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/terminal.h"


namespace {

class imsai_state : public driver_device
{
public:
	imsai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_pit(*this, "pit")
	{ }

	void imsai(machine_config &config);

private:
	void kbd_put(u8 data);
	u8 keyin_r();
	u8 status_r();
	void control_w(u8 data);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_term_data = 0U;
	void machine_reset() override;
	void machine_start() override;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<generic_terminal_device> m_terminal;
	required_device<pit8253_device> m_pit;
};


void imsai_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0xd000, 0xd0ff).ram();
	map(0xd100, 0xd103).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd800, 0xdfff).rom().region("maincpu", 0);
}

void imsai_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x02, 0x02).r(FUNC(imsai_state::keyin_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x03, 0x03).r(FUNC(imsai_state::status_r));
	map(0x04, 0x05).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x12, 0x13).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x14, 0x14).r(FUNC(imsai_state::keyin_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x15, 0x15).r(FUNC(imsai_state::status_r));
	map(0xf3, 0xf3).w(FUNC(imsai_state::control_w));
}

/* Input ports */
static INPUT_PORTS_START( imsai )
INPUT_PORTS_END

u8 imsai_state::keyin_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

u8 imsai_state::status_r()
{
	return (m_term_data) ? 3 : 1;
}

void imsai_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void imsai_state::control_w(u8 data)
{
}

void imsai_state::machine_reset()
{
	m_term_data = 0;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xd800, 0xdfff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// remove from the memory map
					m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x0000, 0x07ff);
				}
			},
			&m_rom_shadow_tap);
}

void imsai_state::machine_start()
{
	save_item(NAME(m_term_data));
}

void imsai_state::imsai(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(6'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &imsai_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &imsai_state::io_map);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(imsai_state::kbd_put));

	/* Devices */
	I8251(config, "uart", 0);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(6_MHz_XTAL / 3); // Timer 0: baud rate gen for 8251
	m_pit->out_handler<0>().set("uart", FUNC(i8251_device::write_txc));
	m_pit->out_handler<0>().append("uart", FUNC(i8251_device::write_rxc));
	m_pit->set_clk<1>(6_MHz_XTAL / 3); // Timer 1: user
	m_pit->set_clk<2>(6_MHz_XTAL / 3); // Timer 2: user
}

/* ROM definition */
ROM_START( imsai )
	ROM_REGION( 0x800, "maincpu", 0 ) // 2716 or 2708 program PROM
	ROM_LOAD( "vdb-80.rom",   0x0000, 0x0800, CRC(0afc4683) SHA1(a5419aaee00badf339d7c627f50ef8b2538e42e2) )

	ROM_REGION( 0x200, "decode", 0 ) // 512x4 address decoder ROM
	ROM_LOAD( "3622.u31", 0x000, 0x200, NO_DUMP )

	ROM_REGION( 0x20, "status", 0 ) // PROM for decoding 8085 status signals
	ROM_LOAD( "74s288.u38", 0x00, 0x20, NO_DUMP )
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1978, imsai, 0,      0,      imsai,   imsai, imsai_state, empty_init, "Imsai", "MPU-B",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
