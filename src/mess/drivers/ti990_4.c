// license:MAME|LGPL-2.1+
// copyright-holders:Michael Zapf
/*
    ti990/4 driver

    We emulate a basic ti990/4 board, with a FD800 disk controller and an 733
    ASR terminal.  A little floppy-based software for this computer is
    available thanks to efforts by Dave Pitts: mostly, Forth and TX990.


    Board setup options:
    8kb of DRAM (onboard option, with optional parity): base 0x0000 or 0x2000
    4 banks of 512 bytes of ROM or SRAM: base 0x0000, 0x0800, 0xF000 or 0xF800
    power-up vector: 0x0000 (level 0) or 0xFFFC (load)
    optional memerr interrupt (level 2)
    optional power fail interrupt (level 1)
    optional real-time clock interrupt (level 5 or 7)


    Setup for the emulated system:
    0x0000: 8kb on-board DRAM + 24kb extension RAM (total 32kb)
    0xF800: 512 bytes SRAM
    0xFA00: 512 bytes SRAM (or empty?)
    0xFC00: 512 bytes self-test ROM
    0xFE00: 512 bytes loader ROM
    power-up vector: 0xFFFC (load)

    Note that only interrupt levels 3-7 are supported by the board (8-15 are not wired).

TODO:
* Split into two machines for either ASR or VDT
* finish ASR emulation
* programmer panel
* emulate other devices: card reader, printer

    Original implementation: Raphael Nabet
    Rewritten by Michael Zapf 2013
*/

#include "emu.h"
#include "cpu/tms9900/tms9900.h"

#include "video/911_vdt.h"
#include "sound/beep.h"
#include "video/733_asr.h"

#include "imagedev/flopdrv.h"
#include "machine/ti99/990_dk.h"


class ti990_4_state : public driver_device
{
public:
	ti990_4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fd800(*this, "fd800") { }

	device_t *m_terminal;
	DECLARE_READ8_MEMBER( panel_read );
	DECLARE_WRITE8_MEMBER( panel_write );
	DECLARE_WRITE8_MEMBER( external_operation );
	DECLARE_READ8_MEMBER( interrupt_level );
	DECLARE_WRITE_LINE_MEMBER( fd_interrupt );

	DECLARE_DRIVER_INIT(ti990_4);
	DECLARE_DRIVER_INIT(ti990_4v);

	DECLARE_MACHINE_RESET(ti990_4);

	virtual void video_start();
	UINT32 screen_update_ti990_4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ti990_4_line_interrupt);
	void idle_callback(int state);

private:
	void        hold_load();
	void        device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	int         m_intlines;
	int         m_int_level;
	emu_timer*  m_nmi_timer;
	void        reset_int_lines();
	void        set_int_line(int line, int state);
	void        line_interrupt();

	void        set_int3(int state);
	void        set_int6(int state);
	void        set_int7(int state);
	bool        m_ckon_state;

	bool        m_video;

	// Connected devices
	required_device<tms9900_device>     m_maincpu;
	required_device<fd800_legacy_device> m_fd800;
};

enum
{
	NMI_TIMER_ID = 1
};

void ti990_4_state::hold_load()
{
	m_maincpu->set_input_line(INT_9900_LOAD, ASSERT_LINE);
	logerror("ti990_4: Triggering LOAD interrupt\n");
	m_nmi_timer->adjust(attotime::from_msec(100));
}

/*
    LOAD interrupt trigger callback
*/
void ti990_4_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_maincpu->set_input_line(INT_9900_LOAD, CLEAR_LINE);
	logerror("ti990_4: Released LOAD interrupt\n");
}

READ8_MEMBER( ti990_4_state::panel_read )
{
	if (offset == 1)
		return 0x48;

	return 0;
}

WRITE8_MEMBER( ti990_4_state::panel_write )
{
}

void ti990_4_state::set_int_line(int line, int state)
{
	if (state)
		m_intlines |= (1 << line);
	else
		m_intlines &= ~(1 << line);

	if (m_intlines != 0)
	{
		m_int_level = 0;
		while ((m_intlines & (1 << m_int_level))==0) m_int_level++;
		logerror("ti990_4: Setting int level to %x\n", m_int_level);
		m_maincpu->set_input_line(INT_9900_INTREQ, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INT_9900_INTREQ, CLEAR_LINE);
}

void ti990_4_state::set_int3(int state)
{
	set_int_line(3, state);
}

void ti990_4_state::set_int6(int state)
{
	set_int_line(6, state);
}

void ti990_4_state::set_int7(int state)
{
	set_int_line(7, state);
}

void ti990_4_state::reset_int_lines()
{
	m_intlines = 0;
}

void ti990_4_state::line_interrupt()
{
	if (m_ckon_state) set_int_line(5, ASSERT_LINE);
}

/*
    Callback from the floppy controller.
*/
WRITE_LINE_MEMBER(ti990_4_state::fd_interrupt)
{
	set_int7(state);
}

INTERRUPT_GEN_MEMBER(ti990_4_state::ti990_4_line_interrupt)
{
	if (m_video)
		vdt911_keyboard(m_terminal);
	else
		asr733_keyboard(m_terminal);

	line_interrupt();
}

/*
    TI990/4 video emulation.

    We emulate a single VDT911 CRT terminal.
*/

void ti990_vdt_int(running_machine &machine, int state)
{
	// set_int3
}

static const vdt911_init_params_t vdt911_intf =
{
	char_1920,
	vdt911_model_US,
	ti990_vdt_int
};

void ti990_4_state::video_start()
{
	if (m_video)
		m_terminal = machine().device("vdt911");
	else
		m_terminal = machine().device("asr733");
}

UINT32 ti990_4_state::screen_update_ti990_4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video)
		vdt911_refresh(m_terminal, bitmap, cliprect, 0, 0);
	else
		asr733_refresh(m_terminal, bitmap, 0, 0);

	return 0;
}

static const asr733_init_params_t asr733_intf =
{
	// set_int6
};

WRITE8_MEMBER( ti990_4_state::external_operation )
{
	static const char* extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	switch (offset)
	{
	case IDLE_OP:
		return;
	case RSET_OP:
		m_ckon_state = false;
		// clear controller panel and smi fault LEDs
		break;
	case CKON_OP:
		m_ckon_state = true;
		break;
	case CKOF_OP:
		m_ckon_state = false;
		set_int_line(5, 0);
		break;
	case LREX_OP:
		// TODO: Verify this
		hold_load();
		break;

	default:
		logerror("ti99_4x: External operation %s not implemented on TI-990/4 board\n", extop[offset]);
	}
}

READ8_MEMBER( ti990_4_state::interrupt_level )
{
	return m_int_level;
}

/*
    Memory map - see description above
*/

static ADDRESS_MAP_START(memmap, AS_PROGRAM, 16, ti990_4_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM /* dynamic RAM */
	AM_RANGE(0x8000, 0xf7ff) AM_NOP /* reserved for expansion */
	AM_RANGE(0xf800, 0xfbff) AM_RAM /* static RAM? */
	AM_RANGE(0xfc00, 0xffff) AM_ROM /* LOAD ROM */
ADDRESS_MAP_END


/*
    CRU map

    0x000-0xF7F: user devices
    0xF80-0xF9F: CRU interrupt + expansion control
    0xFA0-0xFAF: TILINE coupler interrupt control
    0xFB0-0xFCF: reserved
    0xFD0-0xFDF: memory mapping and memory protect
    0xFE0-0xFEF: internal interrupt control
    0xFF0-0xFFF: front panel

    Default user map:
    0x000-0x00f: 733 ASR (int 6)
    0x010-0x01f: PROM programmer (wired to int 15, unused)
    0x020-0x02f: 804 card reader (int 4)
    0x030-0x03f: line printer (wired to int 14, unused)
    0x040-0x05f: FD800 floppy controller (int 7)
    0x060-0x07f: VDT1 (int 3 - wired to int 11, unused)
    0x080-0x09f: VDT2, or CRU expansion (int ??? - wired to int 10, unused)
    0x0a0-0x0bf: VDT3 (int ??? - wired to int 9, unused)
*/

static ADDRESS_MAP_START(cru_map, AS_IO, 8, ti990_4_state )
	AM_RANGE(0x00, 0x01) AM_DEVREAD_LEGACY("asr733", asr733_cru_r)
	AM_RANGE(0x00, 0x0f) AM_DEVWRITE_LEGACY("asr733", asr733_cru_w)

	AM_RANGE(0x08, 0x0b) AM_DEVREAD( "fd800", fd800_legacy_device, cru_r )
	AM_RANGE(0x40, 0x5f) AM_DEVWRITE( "fd800", fd800_legacy_device, cru_w )

	AM_RANGE(0x1fe, 0x1ff) AM_READ( panel_read )
	AM_RANGE(0xff0, 0xfff) AM_WRITE( panel_write )
ADDRESS_MAP_END

static ADDRESS_MAP_START(cru_map_v, AS_IO, 8, ti990_4_state )
	AM_RANGE(0x10, 0x11) AM_DEVREAD_LEGACY("vdt911", vdt911_cru_r)
	AM_RANGE(0x80, 0x8f) AM_DEVWRITE_LEGACY("vdt911", vdt911_cru_w)

	AM_RANGE(0x08, 0x0b) AM_DEVREAD( "fd800", fd800_legacy_device, cru_r )
	AM_RANGE(0x40, 0x5f) AM_DEVWRITE( "fd800", fd800_legacy_device, cru_w )

	AM_RANGE(0x1fe, 0x1ff) AM_READ( panel_read )
	AM_RANGE(0xff0, 0xfff) AM_WRITE( panel_write )
ADDRESS_MAP_END


static const floppy_interface ti990_4_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_8_DSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(fd800),
	NULL,
	NULL
};

static TMS99xx_CONFIG( cpuconf )
{
	DEVCB_DRIVER_MEMBER(ti990_4_state, external_operation),
	DEVCB_DRIVER_MEMBER(ti990_4_state, interrupt_level),
	DEVCB_NULL,     // Instruction acquisition
	DEVCB_NULL,     // Clock out
	DEVCB_NULL,     // wait
	DEVCB_NULL,      // Hold acknowledge
	DEVCB_NULL     // data bus in
};

MACHINE_RESET_MEMBER(ti990_4_state,ti990_4)
{
	hold_load();
	reset_int_lines();
	m_ckon_state = false;
	m_maincpu->set_ready(ASSERT_LINE);
}

DRIVER_INIT_MEMBER(ti990_4_state, ti990_4)
{
	m_video = false;
	m_nmi_timer = timer_alloc(NMI_TIMER_ID);
	asr733_init(machine());
}

DRIVER_INIT_MEMBER(ti990_4_state, ti990_4v)
{
	m_video = true;
	m_nmi_timer = timer_alloc(NMI_TIMER_ID);
	vdt911_init(machine());
}

static INPUT_PORTS_START(ti990_4)
	ASR733_KEY_PORTS
INPUT_PORTS_END

static INPUT_PORTS_START(ti990_4v)
	VDT911_KEY_PORTS
INPUT_PORTS_END

static MACHINE_CONFIG_START( ti990_4, ti990_4_state )
	/* basic machine hardware */
	/* TMS9900 CPU @ 3.0(???) MHz */
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map, cpuconf)

	MCFG_CPU_PERIODIC_INT_DRIVER(ti990_4_state, ti990_4_line_interrupt,  120/*or TIME_IN_HZ(100) in Europe*/)
	MCFG_MACHINE_RESET_OVERRIDE(ti990_4_state, ti990_4 )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(ti990_4_state, screen_update_ti990_4)

	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_PALETTE("asr733:palette")
	MCFG_ASR733_VIDEO_ADD("asr733", asr733_intf)

	// Floppy controller
	MCFG_FD800_ADD("fd800", WRITELINE(ti990_4_state, fd_interrupt))
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(ti990_4_floppy_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ti990_4v, ti990_4_state )
	/* basic machine hardware */
	/* TMS9900 CPU @ 3.0(???) MHz */
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map_v, cpuconf)

	MCFG_CPU_PERIODIC_INT_DRIVER(ti990_4_state, ti990_4_line_interrupt,  120/*or TIME_IN_HZ(100) in Europe*/)
	MCFG_MACHINE_RESET_OVERRIDE(ti990_4_state, ti990_4 )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(ti990_4_state, screen_update_ti990_4)

	MCFG_SCREEN_SIZE(560, 280)
	MCFG_SCREEN_VISIBLE_AREA(0, 560-1, 0, /*250*/280-1)
	MCFG_SCREEN_PALETTE("vdt911:palette")
	MCFG_VDT911_VIDEO_ADD("vdt911", vdt911_intf)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// Floppy controller
	MCFG_FD800_ADD("fd800", WRITELINE(ti990_4_state, fd_interrupt))
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(ti990_4_floppy_interface)
MACHINE_CONFIG_END

/*
  ROM loading
*/
ROM_START(ti990_4)
	/*CPU memory space*/
	ROM_REGION16_BE(0x10000, "maincpu",0)

#if 0
	/* ROM set 945121-5: "733 ASR ROM loader with self test (prototyping)"
	(cf 945401-9701 pp. 1-19) */

	/* test ROM */
	ROMX_LOAD("94519209.u39", 0xFC00, 0x100, CRC(0a0b0c42), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("94519210.u55", 0xFC00, 0x100, CRC(d078af61), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD("94519211.u61", 0xFC01, 0x100, CRC(6cf7d4a0), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("94519212.u78", 0xFC01, 0x100, CRC(d9522458), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))

	/* LOAD ROM */
	ROMX_LOAD("94519113.u3", 0xFE00, 0x100, CRC(8719b04e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("94519114.u4", 0xFE00, 0x100, CRC(72a040e0), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD("94519115.u6", 0xFE01, 0x100, CRC(9ccf8cca), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("94519116.u7", 0xFE01, 0x100, CRC(fa387bf3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))

#else
	/* ROM set 945121-4(?): "Floppy disc loader with self test" (cf 945401-9701 pp. 1-19) */
	ROM_LOAD16_WORD("ti9904.rom", 0xFC00, 0x400, CRC(691e7d19) SHA1(58d9bed80490fdf71c743bfd3077c70840b7df8c))
#endif

	ROM_REGION(asr733_chr_region_len, asr733_chr_region, ROMREGION_ERASEFF)
ROM_END

ROM_START(ti990_4v)
	/*CPU memory space*/
	ROM_REGION16_BE(0x10000, "maincpu",0)
	ROM_LOAD16_WORD("ti9904.rom", 0xFC00, 0x400, CRC(691e7d19) SHA1(58d9bed80490fdf71c743bfd3077c70840b7df8c))

	/* VDT911 character definitions */
	ROM_REGION(vdt911_chr_region_len, vdt911_chr_region, ROMREGION_ERASEFF)
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY                 FULLNAME */
COMP( 1976, ti990_4,  0,      0,      ti990_4,    ti990_4, ti990_4_state, ti990_4,    "Texas Instruments",    "TI 990/4 Minicomputer System" , GAME_NOT_WORKING | GAME_NO_SOUND )
COMP( 1976, ti990_4v, ti990_4,      0,      ti990_4v,   ti990_4v, ti990_4_state, ti990_4v,    "Texas Instruments",    "TI 990/4 Minicomputer System with Video Display Terminal" , GAME_NOT_WORKING | GAME_NO_SOUND )
