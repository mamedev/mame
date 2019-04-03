// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Besta-88 and Besta-90 engineering workstations.

    Derived (OEMd?) from Force Computers' SYS68K series.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/terminal.h"


class besta_state : public driver_device
{
public:
	besta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit1 (*this, "pit1")
		, m_pit2 (*this, "pit2")
		, m_terminal(*this, "terminal")
		, m_p_ram(*this, "p_ram")
	{ }

	void besta(machine_config &config);

protected:
	void besta_mem(address_map &map);
	DECLARE_READ8_MEMBER( mpcc_reg_r );
	DECLARE_WRITE8_MEMBER( mpcc_reg_w );
	void kbd_put(u8 data);
	virtual void machine_reset() override;

	uint8_t m_term_data;
	uint8_t m_mpcc_regs[32];

	required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint32_t> m_p_ram;
};

READ8_MEMBER( besta_state::mpcc_reg_r )
{
	uint8_t ret;

	switch (offset) {
		case 0: /* r_stat aka ... */
			return (m_term_data) ? 0x80 : 0;
		case 2: /* r_data aka ... */
			ret = m_term_data;
			m_term_data = 0;
			return ret;
		default:
			return m_mpcc_regs[offset];
	}
}

WRITE8_MEMBER( besta_state::mpcc_reg_w )
{
	switch (offset) {
		case 2:
			kbd_put(data);
			break;
		case 10:
			m_terminal->write(data);
		default:
			m_mpcc_regs[offset] = data;
			break;
	}
}

void besta_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void besta_state::besta_mem(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("p_ram");       // local bus DRAM, 4MB
//  AM_RANGE(0x08010000, 0x08011fff) AM_RAM                         // unknown -- accessed by cp31dssp
//  AM_RANGE(0xfca03500, 0xfca0350f) AM_READWRITE8(iscsi_reg_r, iscsi_reg_w, 0xffffffff)
	map(0xff000000, 0xff00ffff).rom().region("user1", 0);   // actual mapping is up to 0xff03ffff
	map(0xff040000, 0xff07ffff).ram();                         // onboard SRAM
//  AM_RANGE(0xff800000, 0xff80001f) AM_DEVREADWRITE8("mpcc", mpcc68561_t, reg_r, reg_w, 0xffffffff)
	map(0xff800000, 0xff80001f).rw(FUNC(besta_state::mpcc_reg_r), FUNC(besta_state::mpcc_reg_w)); // console
	map(0xff800200, 0xff800237).rw(m_pit2, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
//  AM_RANGE(0xff800400, 0xff800xxx) // ??? -- shows up in cp31dssp log
//  AM_RANGE(0xff800800, 0xff800xxx) // 68153 BIM
//  AM_RANGE(0xff800a00, 0xff800xxx) // 62421 RTC
	map(0xff800c00, 0xff800c37).rw(m_pit1, FUNC(pit68230_device::read), FUNC(pit68230_device::write));
//  AM_RANGE(0xff800e00, 0xff800xxx) // PIT3?
}

/* Input ports */
static INPUT_PORTS_START( besta )
INPUT_PORTS_END


void besta_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x10000); // not really what happens but...
	memset(m_mpcc_regs, 0, sizeof(m_mpcc_regs));    // should initialize to defined values
	m_mpcc_regs[8] = 0x80;              // always ready to transmit

	m_maincpu->reset();
}

/* CP31 processor board */
void besta_state::besta(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 2*16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &besta_state::besta_mem);

	PIT68230(config, m_pit1, 16670000 / 2);    // XXX verify clock

	PIT68230(config, m_pit2, 16670000 / 2);    // XXX verify clock

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(besta_state::kbd_put));
}

/* ROM definition */

ROM_START( besta88 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS(0, "cp31dbg", "CP31 Debug")
	ROMX_LOAD( "cp31dbgboot.27c512",  0x0000, 0x10000, CRC(9bf057de) SHA1(b13cb16042e4c6ca63ae26058a78259c0849d0b6), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "cp31dssp", "CP31 DSSP")
	ROMX_LOAD( "cp31dsspboot.27c512", 0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "cp31os9", "CP31 OS9")
	ROMX_LOAD( "cp31os9.27c512",      0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME    FLAGS
COMP( 1988, besta88, 0,      0,      besta,   besta, besta_state, empty_init, "Sapsan", "Besta-88", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
