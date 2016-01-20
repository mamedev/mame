// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    TI990/10 driver

    This driver boots the DX10 build tape and build a bootable system disk.
    I have been able to run a few programs (including most games from the
    "fun and games" tape), but I have been unable to perform system generation
    and install BASIC/COBOL/PASCAL.

TODO :
* programmer panel
* emulate TILINE fully: timings, tiline timeout, possibly memory error
* finish tape emulation (write support)
* add additional devices as need appears (931 VDT, FD800, card reader, ASR/KSR, printer)
* emulate 990/10A and 990/12 CPUs?
* find out why the computer locks when executing ALGS and BASIC/COBOL/PASCAL installation
*/

/*
    CRU map:

    990/10 CPU board:
    1fa0-1fbe: map file CRU interface
    1fc0-1fde: error interrupt register
    1fe0-1ffe: control panel

    optional hardware (default configuration):
    0000-001e: 733 ASR
    0020-003e: PROM programmer
    0040-005e: card reader
    0060-007e: line printer
    0080-00be: FD800 floppy disc
    00c0-00ee: 913 VDT, or 911 VDT
    0100-013e: 913 VDT #2, or 911 VDT
    0140-017e: 913 VDT #3, or 911 VDT
    1700-177e (0b00-0b7e, 0f00-0f7e): CI402 serial controller #0 (#1, #2) (for 931/940 VDT)
        (note that CRU base 1700 is used by the integrated serial controller in newer S300,
        S300A, 990/10A (and 990/5?) systems)
    1f00-1f1e: CRU expander #1 interrupt register
    1f20-1f3e: CRU expander #2 interrupt register
    1f40-1f5e: TILINE coupler interrupt control #1-8


    TPCS map:
    1ff800: disk controller #1 (system disk)
    1ff810->1ff870: extra disk controllers #2 through #8
    1ff880 (1ff890): tape controller #1 (#2)
    1ff900->1ff950: communication controllers #1 through #6
    1ff980 (1ff990, 1ff9A0): CI403/404 serial controller #1 (#2, #3) (for 931/940 VDT)
    1ffb00, 1ffb04, etc: ECC memory controller #1, #2, etc, diagnostic
    1ffb10, 1ffb14, etc: cache memory controller #1, #2, etc, diagnostic


    interrupt map (default configuration):
    0,1,2: CPU board
    3: free
    4: card reader
    5: line clock
    6: 733 ASR/KSR
    7: FD800 floppy (or FD1000 floppy)
    8: free
    9: 913 VDT #3
    10: 913 VDT #2
    11: 913 VDT
    12: free
    13: hard disk
    14 line printer
    15: PROM programmer (actually not used)
*/

#include "emu.h"

#include "cpu/tms9900/ti990_10.h"
#include "sound/beep.h"
#include "bus/ti99x/990_hd.h"
#include "bus/ti99x/990_tap.h"
#include "video/911_vdt.h"


class ti990_10_state : public driver_device
{
public:
	ti990_10_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_intlines(0),
		m_ckon_state(0) { }

	device_t *m_terminal;
	DECLARE_DRIVER_INIT(ti990_10);
	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_WRITE_LINE_MEMBER( key_interrupt );
	DECLARE_WRITE_LINE_MEMBER( line_interrupt );
	DECLARE_WRITE_LINE_MEMBER( tape_interrupt );
	void ti990_set_int_line(int line, int state);
	WRITE_LINE_MEMBER(ti990_set_int13);
	TIMER_CALLBACK_MEMBER(clear_load);
	void ti990_hold_load();
	WRITE_LINE_MEMBER(ti990_ckon_ckof_callback);
	READ8_MEMBER( ti990_panel_read );
	WRITE8_MEMBER( ti990_panel_write );

	required_device<cpu_device> m_maincpu;
	UINT16 m_intlines;
	int m_ckon_state;
};


/*
    Interrupt priority encoder.  Actually part of the CPU board.
*/

void ti990_10_state::ti990_set_int_line(int line, int state)
{
	int level;


	if (state)
		m_intlines |= (1 << line);
	else
		m_intlines &= ~ (1 << line);

	if (m_intlines)
	{
		for (level = 0; ! (m_intlines & (1 << level)); level++)
			;
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, level);  /* interrupt it, baby */
	}
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);
}


WRITE_LINE_MEMBER(ti990_10_state::ti990_set_int13)
{
	ti990_set_int_line(13, state);
}

/*
    hold and debounce load line (emulation is inaccurate)
*/

TIMER_CALLBACK_MEMBER(ti990_10_state::clear_load)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void ti990_10_state::ti990_hold_load()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	machine().scheduler().timer_set(attotime::from_msec(100), timer_expired_delegate(FUNC(ti990_10_state::clear_load),this));
}

/*
    line interrupt
*/

/* m_ckon_state: 1 if line clock active (RTCLR flip-flop on TI990/10 schematics -
SMI sheet 4) */
WRITE_LINE_MEMBER(ti990_10_state::line_interrupt)
{
	// set_int10(state);
	if (m_ckon_state)
		ti990_set_int_line(5, 1);
}

WRITE_LINE_MEMBER(ti990_10_state::ti990_ckon_ckof_callback)
{
	m_ckon_state = state;
	if (! m_ckon_state)
		ti990_set_int_line(5, 0);
}



/*
    Control panel emulation

    three panel types
    * operator panel
    * programmer panel
    * MDU (external unit connected instead of the control panel, as seen in
      945401-9701 p. 2-5 though 2-15)

    Operator panel:
    * Power led
    * Fault led
    * Off/On/Load switch

    Programmer panel:
    * 16 status light, 32 switches, IDLE, RUN leds
    * interface to a low-level debugger in ROMs

    * MDU:
    * includes a programmer panel, a tape unit, and a few parts
      (diagnostic tape, diagnostic ROMs, etc.)

    CRU output:
    0-7: lights 0-7
    8: increment scan
    9: clear scan (according to 990 handbook)
    A: run light (additionally sets all data LEDs to 1s, the scan count to 0b10 and enables the HALT/SIE switch)
    B: fault light
    C: Memory Error Interrupt clear
    D: Start panel timer
    E: Set SIE function (interrupt after 2 instructions are executed)
    F: flag (according to 990 handbook)

    input :
    0-7: switches 0-7 (or data from MDU tape)
    8: scan count bit 1
    9: scan count bit 0
    A: timer active
    B: programmer panel not present or locked
    C: char in MDU tape unit buffer?
    D: unused?
    E: if 0, MDU unit present
    F: flag (according to 990 handbook)
*/

READ8_MEMBER( ti990_10_state::ti990_panel_read )
{
	if (offset == 1)
		return 0x48;

	return 0;
}

WRITE8_MEMBER( ti990_10_state::ti990_panel_write )
{
}


void ti990_10_state::machine_reset()
{
	ti990_hold_load();
}

/*
void ti990_10_state::rset_callback)
{
    ti990_cpuboard_reset();

    // clear controller panel and smi fault LEDs
}

void ti990_10_state::lrex_callback)
{
    // right???
    ti990_hold_load();
}
*/

/*
    TI990/10 video emulation.

    We emulate a single VDT911 CRT terminal.
*/

void ti990_10_state::video_start()
{
	m_terminal = machine().device("vdt911");
}

WRITE_LINE_MEMBER(ti990_10_state::key_interrupt)
{
	// set_int10(state);
}

/*
  Memory map - see description above
*/

static ADDRESS_MAP_START(ti990_10_memmap, AS_PROGRAM, 16, ti990_10_state )

	AM_RANGE(0x000000, 0x0fffff) AM_RAM     /* let's say we have 1MB of RAM */
	AM_RANGE(0x100000, 0x1ff7ff) AM_NOP     /* free TILINE space */
	AM_RANGE(0x1ff800, 0x1ff81f) AM_DEVREADWRITE("hdc", ti990_hdc_device, read, write)  /* disk controller TPCS */
	AM_RANGE(0x1ff820, 0x1ff87f) AM_NOP     /* free TPCS */
	AM_RANGE(0x1ff880, 0x1ff89f) AM_DEVREADWRITE("tpc", tap_990_device, read, write) /* tape controller TPCS */
	AM_RANGE(0x1ff8a0, 0x1ffbff) AM_NOP     /* free TPCS */
	AM_RANGE(0x1ffc00, 0x1fffff) AM_ROM     /* LOAD ROM */

ADDRESS_MAP_END


/*
  CRU map
*/

static ADDRESS_MAP_START(ti990_10_io, AS_IO, 8, ti990_10_state )
	AM_RANGE(0x10, 0x11) AM_DEVREAD("vdt911", vdt911_device, cru_r)
	AM_RANGE(0x80, 0x8f) AM_DEVWRITE("vdt911", vdt911_device, cru_w)
	AM_RANGE(0x1fa, 0x1fb) AM_NOP // AM_READ(ti990_10_mapper_cru_r)
	AM_RANGE(0x1fc, 0x1fd) AM_NOP // AM_READ(ti990_10_eir_cru_r)
	AM_RANGE(0x1fe, 0x1ff) AM_READ(ti990_panel_read)
	AM_RANGE(0xfd0, 0xfdf) AM_NOP // AM_WRITE(ti990_10_mapper_cru_w)
	AM_RANGE(0xfe0, 0xfef) AM_NOP // AM_WRITE(ti990_10_eir_cru_w)
	AM_RANGE(0xff0, 0xfff) AM_WRITE(ti990_panel_write)

ADDRESS_MAP_END

/*
    Callback from the tape controller.
*/
WRITE_LINE_MEMBER(ti990_10_state::tape_interrupt)
{
	// set_int9(state);
}

static MACHINE_CONFIG_START( ti990_10, ti990_10_state )
	/* basic machine hardware */
	/* TI990/10 CPU @ 4.0(???) MHz */
	MCFG_TMS99xx_ADD("maincpu", TI990_10, 4000000, ti990_10_memmap, ti990_10_io )

	// VDT 911 terminal
	MCFG_DEVICE_ADD("vdt911", VDT911, 0)
	MCFG_VDT911_KEYINT_HANDLER(WRITELINE(ti990_10_state, key_interrupt))
	MCFG_VDT911_LINEINT_HANDLER(WRITELINE(ti990_10_state, line_interrupt))

	// Hard disk
	MCFG_DEVICE_ADD("hdc", TI990_HDC, 0)
	MCFG_TI990_HDC_INT_CALLBACK(WRITELINE(ti990_10_state, ti990_set_int13))

	// Tape controller
	MCFG_DEVICE_ADD("tpc", TI990_TAPE_CTRL, 0)
	MCFG_TI990_TAPE_INT_HANDLER(WRITELINE(ti990_10_state, tape_interrupt))
MACHINE_CONFIG_END


/*
  ROM loading
*/
ROM_START(ti990_10)

	/*CPU memory space*/
#if 0

	ROM_REGION16_BE(0x200000, "maincpu",0)

	/* TI990/10 : older boot ROMs for floppy-disk */
	ROM_LOAD16_BYTE("975383.31", 0x1FFC00, 0x100, CRC(64fcd040))
	ROM_LOAD16_BYTE("975383.32", 0x1FFC01, 0x100, CRC(64277276))
	ROM_LOAD16_BYTE("975383.29", 0x1FFE00, 0x100, CRC(af92e7bf))
	ROM_LOAD16_BYTE("975383.30", 0x1FFE01, 0x100, CRC(b7b40cdc))

#elif 1

	ROM_REGION16_BE(0x200000, "maincpu",0)

	/* TI990/10 : newer "universal" boot ROMs  */
	ROM_LOAD16_BYTE("975383.45", 0x1FFC00, 0x100, CRC(391943c7) SHA1(bbd4da60b221d146542a6b547ae1570024e41b8a))
	ROM_LOAD16_BYTE("975383.46", 0x1FFC01, 0x100, CRC(f40f7c18) SHA1(03613bbf2263a4335c25dfc63cf2878c06bfe280))
	ROM_LOAD16_BYTE("975383.47", 0x1FFE00, 0x100, CRC(1ba571d8) SHA1(adaa18f149b643cc842fea8d7107ee868d6ffaf4))
	ROM_LOAD16_BYTE("975383.48", 0x1FFE01, 0x100, CRC(8852b09e) SHA1(f0df2abb438716832c16ab111e475da3ae612673))

#else

	ROM_REGION16_BE(0x202000, "maincpu",0)

	/* TI990/12 ROMs - actually incompatible with TI990/10, but I just wanted to disassemble them. */
	ROM_LOAD16_BYTE("ti2025-7", 0x1FFC00, 0x1000, CRC(4824f89c))
	ROM_LOAD16_BYTE("ti2025-8", 0x1FFC01, 0x1000, CRC(51fef543))
	/* the other half of this ROM is not loaded - it makes no sense as TI990/12 machine code, as
	it is microcode... */

#endif


	/* VDT911 character definitions */
	ROM_REGION(vdt911_chr_region_len, vdt911_chr_region, ROMREGION_ERASEFF)

ROM_END

DRIVER_INIT_MEMBER(ti990_10_state,ti990_10)
{
#if 0
	/* load specific ti990/12 rom page */
	const int page = 3;

	memmove(memregion("maincpu")->base()+0x1FFC00, memregion("maincpu")->base()+0x1FFC00+(page*0x400), 0x400);
#endif
}

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY                 FULLNAME */
COMP( 1975, ti990_10,   0,      0,      ti990_10,   0, ti990_10_state,   ti990_10,   "Texas Instruments",    "TI Model 990/10 Minicomputer System" , MACHINE_NOT_WORKING )
