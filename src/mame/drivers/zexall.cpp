// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Robbbert
/******************************************************************************
*
*  Self Contained zexall 'Z80 instruction exerciser' test driver
*  Zexall originally written by Frank Cringle for ZX Spectrum
*  Modularized Spectrum-independent Zexall binary supplied by Blargg
*  Serial interface binary/preloader at 0x0000-0x00FF written by Kevin 'kevtris' Horton
*
*
* mem map:
Ram 0000-FFFF (preloaded with binary)
Special calls take place for three ram values (this interface was designed by kevtris):
FFFD - 'ack' - shared ram with output device; z80 reads from here and considers the byte at FFFF read if this value incremented
FFFE - 'req' - shared ram with output device; z80 writes an incrementing value to FFFE to indicate that there is a byte waiting at FFFF and hence requesting the output device on the other end do something about it, until FFFD is incremented by the output device to acknowledge receipt
FFFF - 'data' - shared ram with output device; z80 writes the data to be sent to output device here
One i/o port is used:
0001 - bit 0 controls whether interrupt timer is enabled (1) or not (0), this is a holdover from a project of kevtris' and can be ignored.

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class zexall_state : public driver_device
{
public:
	zexall_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_main_ram(*this, "main_ram")
	{
	}

	DECLARE_READ8_MEMBER( zexall_output_ack_r );
	DECLARE_READ8_MEMBER( zexall_output_req_r );
	DECLARE_READ8_MEMBER( zexall_output_data_r );
	DECLARE_WRITE8_MEMBER( zexall_output_ack_w );
	DECLARE_WRITE8_MEMBER( zexall_output_req_w );
	DECLARE_WRITE8_MEMBER( zexall_output_data_w );
	DECLARE_DRIVER_INIT(zexall);
private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<UINT8> m_main_ram;
	UINT8 m_out_data; // byte written to 0xFFFF
	UINT8 m_out_req; // byte written to 0xFFFE
	UINT8 m_out_req_last; // old value at 0xFFFE before the most recent write
	UINT8 m_out_ack; // byte written to 0xFFFC
	virtual void machine_reset() override;
};

DRIVER_INIT_MEMBER(zexall_state,zexall)
{
	m_out_ack = 0;
	m_out_req = 0;
	m_out_req_last = 0;
	m_out_data = 0;
}

void zexall_state::machine_reset()
{
// rom is self-modifying, so need to refresh it on each run
	UINT8 *rom = memregion("romcode")->base();
	UINT8 *ram = m_main_ram;
	/* fill main ram with zexall code */
	memcpy(ram, rom, 0x228a);
}

READ8_MEMBER( zexall_state::zexall_output_ack_r )
{
// spit out the byte in out_byte if out_req is not equal to out_req_last
	if (m_out_req != m_out_req_last)
	{
		m_terminal->write(space,0,m_out_data);
		fprintf(stderr,"%c",m_out_data);
		m_out_req_last = m_out_req;
		m_out_ack++;
	}
	return m_out_ack;
}

WRITE8_MEMBER( zexall_state::zexall_output_ack_w )
{
	m_out_ack = data;
}

READ8_MEMBER( zexall_state::zexall_output_req_r )
{
	return m_out_req;
}

WRITE8_MEMBER( zexall_state::zexall_output_req_w )
{
	m_out_req_last = m_out_req;
	m_out_req = data;
}

READ8_MEMBER( zexall_state::zexall_output_data_r )
{
	return m_out_data;
}

WRITE8_MEMBER( zexall_state::zexall_output_data_w )
{
	m_out_data = data;
}

/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(z80_mem, AS_PROGRAM, 8, zexall_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xfffc) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0xfffd, 0xfffd) AM_READWRITE(zexall_output_ack_r,zexall_output_ack_w)
	AM_RANGE(0xfffe, 0xfffe) AM_READWRITE(zexall_output_req_r,zexall_output_req_w)
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(zexall_output_data_r,zexall_output_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io, AS_IO, 8, zexall_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0001, 0x0001) AM_NOP // really a disable/enable for some sort of interrupt timer on kev's hardware, which is completely irrelevant for the zexall test
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( zexall )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( zexall, zexall_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3_579545MHz*10)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(zexall)
	ROM_REGION(0x22ff, "romcode", 0)
	ROM_LOAD("zex.bin", 0x0000, 0x2289, CRC(77e0a1df) SHA1(cc8f84724e3837783816d92a6dfb8e5975232c66))
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                                                    FLAGS */
COMP( 2009, zexall,   0,          0,      zexall,   zexall, zexall_state, zexall,      "Frank Cringle & MESSDEV",   "ZEXALL Z80 instruction set exerciser (modified for MESS)", MACHINE_NO_SOUND_HW )
