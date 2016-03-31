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

#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

#define TERMINAL_TAG "terminal"

class besta_state : public driver_device
{
public:
	besta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pit1 (*this, "pit1"),
		m_pit2 (*this, "pit2"),
		m_terminal(*this, TERMINAL_TAG),
		m_p_ram(*this, "p_ram")
	{
	}

	DECLARE_READ8_MEMBER( mpcc_reg_r );
	DECLARE_WRITE8_MEMBER( mpcc_reg_w );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	UINT8 m_mpcc_regs[32];

	required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;
	virtual void machine_reset() override;

	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<UINT32> m_p_ram;
};

READ8_MEMBER( besta_state::mpcc_reg_r )
{
	UINT8 ret;

	if (!(offset == 0 && !m_mpcc_regs[0])) {
	DBG_LOG(1,"mpcc_reg_r",("(%d) = %02X at %s\n", offset,
		(offset > 31 ? -1 : m_mpcc_regs[offset]), machine().describe_context()));
	}

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
	device_t *devconf = machine().device(TERMINAL_TAG);

	DBG_LOG(1,"mpcc_reg_w",("(%d) <- %02X at %s\n", offset, data, machine().describe_context()));

	switch (offset) {
		case 2:
			m_term_data = data;
			break;
		case 10:
			dynamic_cast<generic_terminal_device *>(devconf)->write(generic_space(), 0, data);
		default:
			m_mpcc_regs[offset] = data;
			break;
	}
}

WRITE8_MEMBER( besta_state::kbd_put )
{
	mpcc_reg_w(space, (offs_t)2, data, mem_mask);
}

static ADDRESS_MAP_START(besta_mem, AS_PROGRAM, 32, besta_state)
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("p_ram")       // local bus DRAM, 4MB
//  AM_RANGE(0x08010000, 0x08011fff) AM_RAM                         // unknown -- accessed by cp31dssp
//  AM_RANGE(0xfca03500, 0xfca0350f) AM_READWRITE8(iscsi_reg_r, iscsi_reg_w, 0xffffffff)
	AM_RANGE(0xff000000, 0xff00ffff) AM_ROM AM_REGION("user1", 0)   // actual mapping is up to 0xff03ffff
	AM_RANGE(0xff040000, 0xff07ffff) AM_RAM                         // onboard SRAM
//  AM_RANGE(0xff800000, 0xff80001f) AM_DEVREADWRITE8("mpcc", mpcc68561_t, reg_r, reg_w, 0xffffffff)
	AM_RANGE(0xff800000, 0xff80001f) AM_READWRITE8(mpcc_reg_r, mpcc_reg_w, 0xffffffff) // console
	AM_RANGE(0xff800200, 0xff800237) AM_DEVREADWRITE8 ("pit2", pit68230_device, read, write, 0xffffffff)
//  AM_RANGE(0xff800400, 0xff800xxx) // ??? -- shows up in cp31dssp log
//  AM_RANGE(0xff800800, 0xff800xxx) // 68153 BIM
//  AM_RANGE(0xff800a00, 0xff800xxx) // 62421 RTC
	AM_RANGE(0xff800c00, 0xff800c37) AM_DEVREADWRITE8 ("pit1", pit68230_device, read, write, 0xffffffff)
//  AM_RANGE(0xff800e00, 0xff800xxx) // PIT3?
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( besta )
INPUT_PORTS_END


void besta_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x10000); // not really what happens but...
	memset(m_mpcc_regs, 0, sizeof(m_mpcc_regs));    // should initialize to defined values
	m_mpcc_regs[8] = 0x80;              // always ready to transmit

	m_maincpu->reset();
}

/* CP31 processor board */
static MACHINE_CONFIG_START( besta, besta_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 2*16670000)
	MCFG_CPU_PROGRAM_MAP(besta_mem)

	MCFG_DEVICE_ADD ("pit1", PIT68230, 16670000 / 2)    // XXX verify clock

	MCFG_DEVICE_ADD ("pit2", PIT68230, 16670000 / 2)    // XXX verify clock

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(besta_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( besta88 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS(0, "cp31dbg", "CP31 Debug")
	ROMX_LOAD( "cp31dbgboot.27c512",  0x0000, 0x10000, CRC(9bf057de) SHA1(b13cb16042e4c6ca63ae26058a78259c0849d0b6), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "cp31dssp", "CP31 DSSP")
	ROMX_LOAD( "cp31dsspboot.27c512", 0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "cp31os9", "CP31 OS9")
	ROMX_LOAD( "cp31os9.27c512",      0x0000, 0x10000, CRC(607a0a55) SHA1(c257a88672ab39d2f3fad681d22e062182b0236d), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT                 INIT    COMPANY         FULLNAME       FLAGS */
COMP( 1988, besta88,  0,      0,     besta,     besta,   driver_device,  0,  "Sapsan", "Besta-88", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
