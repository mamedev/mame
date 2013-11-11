/***************************************************************************

    DEC Rainbow 100

    Driver-in-progress by R. Belmont and Miodrag Milanovic with additions by Karl-Ludwig Deisenhofer.

    STATE AS OF NOVEMBER 2013
    --------------------------
    - FATAL: keyboard emulation needs love (inhibits the system from booting with ERROR 50 on cold or ERROR 13 on warm boot).
    - NOT WORKING: serial (ERROR 60)
    - NOT WORKING: printer interface (ERROR 40).

    - NON-CRITICAL: no code for W18 (DSR) jumper. W90 jumper probably not relevant for emulation (VBIAS pin 2 => to unknown register).
    - NON-CRITICAL: watchdog logic not implemented. MHFLU - ERROR 16 indicated hardware problems or (most often) software crashes on real hardware.

    - SHOULD BE IMPLEMENTED AS SLOT DEVICES (for now, DIP settings affect 'system_parameter_r' only and are disabled):
            * Color graphics option (uses NEC upd7220 GDC)
            * Extended communication option (same as BUNDLE_OPTION ?)

    - OTHER UPGRADES (NEC_V20 should be easy, the TURBOW is harder to come by)
			* Suitable Solutions TURBOW286 (12 Mhz, 68-pin, low power AMD N80L286-12 and WAYLAND/EDSUN EL286-88-10-B ( 80286 to 8088 Processor Signal Converter )
			  / replacement for main cpu with on-board DC 7174 or DT 7174 (?) RTC and changed BOOT ROM labeled 'TBSS1.3 - 3ED4').

			* NEC_V20 (requires modded BOOT ROM because of - at least 2 - hard coded timing loops): 
                 100A:         100B/100+:						100B+ ALTERNATE RECOMMENDATION (fixes RAM size auto-detection problems when V20 is in place.
				                                                                                Tested on a 30+ year old live machine. Your mileage may vary)
                 Location Data	Location Data                   Loc.|Data								
																00C6 46 [ increases 'wait for Z80' from approx. 27,5 ms (old value 40) to 30,5 ms ]
																0303 00 [ disable CHECKSUM ]
                 043F     64    072F     64	<----------------->	072F 73 [ increases minimum cycle time from 2600 (64) to 3000 ms (73) ]
                 067D	  20	0B36     20	<-----------------> 0B36 20 [ use a value of 20 for NEC_V20 - as in the initial patch. Changes cause VFR - ERROR 10. ]
                 1FFE     2B	3FFE     1B  (BIOS CHECKSUM)    
                 1FFF     70	3FFF     88  (BIOS CHECKSUM)    

    Meaning of Diagnostics LEDs (from PC100ESV1.PDF found, e.g.,
    on ftp://ftp.update.uu.se/pub/rainbow/doc/rainbow-docs/

    Internal Diagnostic Messages                               F
    Msg Message                               Lights Display   A
    No.                                       * = on o = off   T
                                              - = on or off    A
                                              1 2 3 4 5 6 7    L
    --------------------------------------------------------------
     1  Main Board (Video)                    o * * o * o *   Yes
     2  Main Board* (unsolicited interrupt)   * * * * o * o   Yes
     3  Drive A or B (index)                  o o * o o * *
     4  Drive A or B (motor)                  * * o o o * *
     5  Drive A or B (seek)                   o * o o o * *
     6  Drive A or B (read)                   * o o o o * *
     7  Drive A or B (restore)                o * * o o * *
     8  Drive A or B (step)                   * o * o o * *
     9  System Load incomplete+ (System Load) o o o o o o o
    10  Main Board (video, vfr)               * * * o * o *   Yes
    11  System Load incomplete+ (Boot Load)   o o o o o o o
    12  Drive A or B (not ready)              o o o o o * *
    13  Keyboard                              * * o * o * o   Yes
    14  Main Board (nvm data)                 * * * * o * *
    15  (no msg. 15 in that table)
    16  Interrupts off*                       * * * o o o o   Cond.
    17  Main Board (video RAM)                * * * o * * o   Yes
    18  Main Board (Z80 crc)                  * * * * o o *   Yes
    19  Main Board RAM (0-64K)                - - - * * o *   Yes
    20  Main Board (unsolicited int., Z80)    * * * o o o *   Yes
    21  Drive Not Ready+                      o o o o o o o
    22  Remove Card or Diskette               o * * o o o *
    23  Non-System Diskette+                  o o o o o o o
    24  new memory size = nnnK                o o o o o o o
    25  Set Up Defaults stored                o o o o o o o
    26  Main Board (RAM arbitration)          * * * o * o o   Yes
    27  Main Board (RAM option)               - - - * * o o
    28  RX50 controller board                 * * * o o * *
    29  Main Board* (Z80 response)            * * * * o o o
    30  Main Board (ROM crc, ROM 0)           * * * * * * *   Yes
    31  Main Board (ROM crc, ROM 1)           * * * * * * o   Yes
    -   Main Board (ROM crc, ROM 2)           * * * o * * *   Yes
    33  Main Board (contention)               o o o o o * o   Yes
    40  Main Board (printer port)             * o * * o * o
    50  Main Board (keyboard port)            o o * * o * o   Yes
    60  Main Board (comm port)                o * * * o * o

    --------------------------------------------------------------
    *   These errors can occur at any time because the circuits
        are monitored constantly
    +   These messages may occur during power-up if auto boot is
        selected

PCB layout
----------

DEC-100 model B
= part no.70-19974-02 according to document EK-RB100-TM_001

PCB # 5416206 / 5016205-01C1:

        7-6-5-4 |3-2-1
        DIAGNOSTIC-LEDs |J3   | |J2     | |J1    |
|------|----8088|Z80-|--|VIDEO|-|PRINTER|-|SERIAL|---|
|  2 x 64 K             |/KBD.|                      |
|  R  A  M             NEC D7201C            |P|[W90]|
|                                            |O|     |
|   [W6]   ROM 1       INTEL 8088            |W|     |
|          (23-020e5-00)                     |E|     |
|                                            |R|     |
| ...J5..  BOOT ROM 0      ...J4...          =J8     |
| ...J6... (23-022e5-00)                             |
| [W5]                                               |
|                                                    |
|     INTEL 8251A   ZILOG Z 80A                      |
|                [W18]                               |
| A  4x                74 LS 244                     |
| M  S           [W15]                               |
| 9  -   DEC-DC011     74 LS 245                     |
| 1  R           [W14]                               |
| 2  A                  [W13]                        |
| 8  M   CHARGEN.-                                   |
|        ROM (4K)           ...J7...  | ...J9 = RX50 |
|------------PCB# 5416206 / 5016205-01C1-------------|
NOTES
W5 + W6 are out when 16K x 8 EPROMS are used  
/ W5 + W6 installed => 32 K x 8 EPROMs (pin 27 = A14)

W13, W14, W15, W18, W90 = for manufacturing tests.
=> W13 - W15 affect diagnostic read register (port $0a)
=> W18 pulls DSR to ground and affects 8251A - port $11 (bit 7)
   NOTE: THERE IS NO CODE TO PULL DSR YET.
=> W90 connects pin 2 (Voltage Bias on PWR connector J8) with the communications control register 
(SH1 - IOWR4 L line according to page 21 of the DEC-100-B field manual)

SEEN ON SCHEMATICS - NOT PRESENT ON THIS PCB:
W16 pulls J2 printer port pin 1 to GND when set (otherwise pin unconnected)
W17 pulls J1 serial  port pin 1 to GND when set (otherwise pin unconnected)
****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "video/vtvideo.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "machine/i8251.h"
#include "machine/dec_lk201.h"
#include "sound/beep.h"
#include "machine/nvram.h"

#include "rainbow.lh" // BEZEL - LAYOUT with LEDs for diag 1-7, keyboard 8-11 and floppy 20-21

#define DEC_B_NVMEM_SIZE (256)

class rainbow_state : public driver_device
{
public:
	rainbow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_inp1(*this, "W13"),
			m_inp2(*this, "W14"),
			m_inp3(*this, "W15"),
			m_inp4(*this, "W18"),
			m_inp5(*this, "BUNDLE OPTION"),
			m_inp6(*this, "FLOPPY CONTROLLER"),
			m_inp7(*this, "GRAPHICS OPTION"),
			m_inp8(*this, "MEMORY PRESENT"),

		m_beep(*this, "beeper"),
		m_crtc(*this, "vt100_video"),
		m_i8088(*this, "maincpu"),
		m_z80(*this, "subcpu"),
		m_fdc(*this, "wd1793"),
		m_kbd8251(*this, "kbdser"),
		m_lk201(*this, LK201_TAG),
		m_p_ram(*this, "p_ram"),

		m_p_vol_ram(*this, "vol_ram"),
		m_p_nvram(*this, "nvram"),

		m_shared(*this, "sh_ram"),
		m_maincpu(*this, "maincpu") { }

	required_ioport m_inp1;
	required_ioport m_inp2;
	required_ioport m_inp3;
	required_ioport m_inp4;
	required_ioport m_inp5;
	required_ioport m_inp6;
	required_ioport m_inp7;
	required_ioport m_inp8;

	required_device<beep_device> m_beep;

	required_device<rainbow_video_device> m_crtc;
	required_device<cpu_device> m_i8088;
	required_device<cpu_device> m_z80;
	required_device<fd1793_device> m_fdc;
	required_device<i8251_device> m_kbd8251;
	required_device<lk201_device> m_lk201;
	required_shared_ptr<UINT8> m_p_ram;
	required_shared_ptr<UINT8> m_p_vol_ram;
	required_shared_ptr<UINT8> m_p_nvram;
	required_shared_ptr<UINT8> m_shared;
	UINT8 m_diagnostic;

	virtual void machine_start();

	DECLARE_READ8_MEMBER(read_video_ram_r);
	DECLARE_WRITE8_MEMBER(clear_video_interrupt);

	DECLARE_READ8_MEMBER(diagnostic_r);
	DECLARE_WRITE8_MEMBER(diagnostic_w);

	DECLARE_READ8_MEMBER(comm_control_r);
	DECLARE_WRITE8_MEMBER(comm_control_w);

	DECLARE_READ8_MEMBER(share_z80_r);
	DECLARE_WRITE8_MEMBER(share_z80_w);

	DECLARE_READ8_MEMBER(floating_bus_r);
	DECLARE_WRITE8_MEMBER(floating_bus_w);

		// EMULATOR TRAP TO INTERCEPT KEYBOARD cmd in AH and PARAMETER in AL (port 90 = AL / port 91 = AH)
		// TODO: beeper and led handling should better be handled by LK201 code.
	DECLARE_WRITE8_MEMBER(PORT90_W);
	DECLARE_WRITE8_MEMBER(PORT91_W);

	DECLARE_READ8_MEMBER(i8088_latch_r);
	DECLARE_WRITE8_MEMBER(i8088_latch_w);
	DECLARE_READ8_MEMBER(z80_latch_r);
	DECLARE_WRITE8_MEMBER(z80_latch_w);

	DECLARE_WRITE8_MEMBER(z80_diskdiag_read_w);
	DECLARE_WRITE8_MEMBER(z80_diskdiag_write_w);

	DECLARE_WRITE8_MEMBER(z80_diskcontrol_write_w);
	DECLARE_READ8_MEMBER(system_parameter_r);

	DECLARE_READ_LINE_MEMBER(kbd_rx);
	DECLARE_WRITE_LINE_MEMBER(kbd_tx);
	DECLARE_WRITE_LINE_MEMBER(kbd_rxready_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_txready_w);

	bool m_SCREEN_BLANK;
	bool m_COLDBOOT;

	bool m_zflip;                   // Z80 alternate memory map with A15 inverted
	bool m_z80_halted;
	bool m_kbd_tx_ready, m_kbd_rx_ready;

	int m_KBD;
	int m_beep_counter;

private:
	UINT8 m_z80_private[0x800];     // Z80 private 2K
	UINT8 m_z80_mailbox, m_8088_mailbox;

	void update_kbd_irq();
	virtual void machine_reset();
public:
	UINT32 screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	required_device<cpu_device> m_maincpu;
};

void rainbow_state::machine_start()
{  
	m_SCREEN_BLANK = false;
	m_COLDBOOT = true;

	save_item(NAME(m_z80_private));
	save_item(NAME(m_z80_mailbox));
	save_item(NAME(m_8088_mailbox));
	save_item(NAME(m_zflip));
	save_item(NAME(m_kbd_tx_ready));
	save_item(NAME(m_kbd_rx_ready));

	UINT8 *rom = memregion("maincpu")->base();

	// Enables PORT90_W + PORT91_W via BIOS call (offset +$21 in HIGH ROM)
	// F8 / FC ROM REGION (CHECK + PATCH)
	if(rom[0xfc000 + 0x0022] == 0x22 && rom[0xfc000 + 0x0023] == 0x28)
	{
			rom[0xf4303]=0x00; // Disable CRC CHECK (F0 / F4 ROM)

			rom[0xfc000 + 0x0022] =0xe2;  // jmp to offset $3906
			rom[0xfc000 + 0x0023] =0x38;

			rom[0xfc000 + 0x3906] =0xe6;  // out 90,al
			rom[0xfc000 + 0x3907] =0x90;

			rom[0xfc000 + 0x3908] =0x86;  //  xchg al,ah
			rom[0xfc000 + 0x3909] =0xc4;

			rom[0xfc000 + 0x390a] =0xe6;  // out 91,al
			rom[0xfc000 + 0x390b] =0x91;

			rom[0xfc000 + 0x390c] =0x86;  // xchg al,ah
			rom[0xfc000 + 0x390d] =0xc4;

			rom[0xfc000 + 0x390e] =0xe9;  // jmp (original jump offset $2846)
			rom[0xfc000 + 0x390f] =0x35;
			rom[0xfc000 + 0x3910] =0xef;
	}
}

static ADDRESS_MAP_START( rainbow8088_map, AS_PROGRAM, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x0ffff) AM_RAM AM_SHARE("sh_ram")
	AM_RANGE(0x10000, 0x1ffff) AM_RAM

	// test at f4e00 - f4e1c
	AM_RANGE(0x20000, 0xdffff) AM_READWRITE(floating_bus_r,floating_bus_w)
	AM_RANGE(0x20000, 0xdffff) AM_RAM

	// Documentation claims there is a 256 x 4 bit NVRAM from 0xed000 to 0xed040 (*)
	//   shadowed at $ec000 - $ecfff and from $ed040 - $edfff.

	//  - PC-100 A might have had a smaller (NV-)RAM (*)
	//  - ED000 - ED0FF is the area the _DEC-100-B BIOS_ accesses - and checks.

	//  - Specs claim that the CPU has direct access to volatile RAM only.
	//    So NVRAM is hidden now and loads & saves are triggered within the 
	//    'diagnostic_w' handler (like on real hardware).

	//  - Address bits 8-12 are ignored (-> AM_MIRROR). Remove for debugging.
	AM_RANGE(0xed000, 0xed0ff) AM_RAM AM_SHARE("vol_ram") AM_MIRROR(0x1f00) 
	AM_RANGE(0xed100, 0xed1ff) AM_RAM AM_SHARE("nvram") 

	AM_RANGE(0xee000, 0xeffff) AM_RAM AM_SHARE("p_ram")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rainbow8088_io , AS_IO, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE (0x00, 0x00) AM_READWRITE(i8088_latch_r, i8088_latch_w)

	// 0x02 Communication status / control register (8088)
	AM_RANGE (0x02, 0x02) AM_READWRITE(comm_control_r, comm_control_w)

	// 0x04 Video processor DC011
	AM_RANGE (0x04, 0x04) AM_DEVWRITE("vt100_video", rainbow_video_device, dc011_w)

	AM_RANGE (0x08, 0x08) AM_READ(system_parameter_r)

	AM_RANGE (0x0a, 0x0a) AM_READWRITE(diagnostic_r, diagnostic_w)
	// 0x0C Video processor DC012
	AM_RANGE (0x0c, 0x0c) AM_DEVWRITE("vt100_video", rainbow_video_device, dc012_w)

	AM_RANGE(0x10, 0x10) AM_DEVREADWRITE("kbdser", i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_DEVREADWRITE("kbdser", i8251_device, status_r, control_w)

	AM_RANGE (0x90, 0x90) AM_WRITE(PORT90_W)
	AM_RANGE (0x91, 0x91) AM_WRITE(PORT91_W)
ADDRESS_MAP_END

static ADDRESS_MAP_START(rainbowz80_mem, AS_PROGRAM, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(share_z80_r, share_z80_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rainbowz80_io, AS_IO, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(z80_latch_r, z80_latch_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(z80_diskdiag_read_w)
	AM_RANGE(0x21, 0x21) AM_WRITE(z80_diskdiag_write_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(z80_diskcontrol_write_w)
	AM_RANGE(0x60, 0x60) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_status_r, wd17xx_command_w)
	AM_RANGE(0x61, 0x61) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_track_r, wd17xx_track_w)
	AM_RANGE(0x62, 0x62) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_sector_r, wd17xx_sector_w)
	AM_RANGE(0x63, 0x63) AM_DEVREADWRITE_LEGACY("wd1793", wd17xx_data_r, wd17xx_data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( rainbow )
		PORT_START("FLOPPY CONTROLLER")
		PORT_DIPNAME( 0x02, 0x02, "FLOPPY CONTROLLER") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_START("MEMORY PRESENT")
		PORT_DIPNAME( 0xF000, 0x2000, "MEMORY PRESENT")
		PORT_DIPSETTING(    0x2000, "128 K (BOARD DEFAULT)" ) // NOTE: 0x2000 hard coded in 'system_parameter_r'
		PORT_DIPSETTING(    0x3000, "192 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x4000, "256 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x5000, "320 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x6000, "384 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x7000, "448 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x8000, "512 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x9000, "576 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xA000, "640 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xB000, "704 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xC000, "768 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xD000, "832 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xE000, "896 K (MEMORY OPTION)" )

		PORT_START("GRAPHICS OPTION")
		PORT_DIPNAME( 0x00, 0x00, "GRAPHICS OPTION") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x04, DEF_STR( On ) )
		PORT_START("BUNDLE OPTION")
		PORT_DIPNAME( 0x00, 0x00, "BUNDLE OPTION") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_START("W13")
		PORT_DIPNAME( 0x02, 0x02, "W13") PORT_TOGGLE
		PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("W14")
		PORT_DIPNAME( 0x04, 0x04, "W14") PORT_TOGGLE
		PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("W15")
		PORT_DIPNAME( 0x08, 0x08, "W15") PORT_TOGGLE
		PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("W18")
		PORT_DIPNAME( 0x00, 0x04, "W18") PORT_TOGGLE
		PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void rainbow_state::machine_reset()
{
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_zflip = true;
	m_z80_halted = true;
	m_kbd_tx_ready = m_kbd_rx_ready = false;

	m_kbd8251->input_callback(SERIAL_STATE_CTS); // raise clear to send

	m_KBD = 0;

	m_beep->set_frequency(2000);
		m_beep->set_state(0);

	// RESET ALL LEDs
	output_set_value("led1", 1);
	output_set_value("led2", 1);
	output_set_value("led3", 1);
	output_set_value("led4", 1);
	output_set_value("led5", 1);
	output_set_value("led6", 1);
	output_set_value("led7", 1);
	output_set_value("led8", 1);
	output_set_value("led9", 1);
	output_set_value("led10", 1);
	output_set_value("led11", 1);

	output_set_value("led20", 1); // DRIVE 0 (A)
	output_set_value("led21", 1); // DRIVE 1 (B)
}

UINT32 rainbow_state::screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_SCREEN_BLANK)
		bitmap.fill(get_black_pen(machine()), cliprect);
 	else 
		m_crtc->video_update(bitmap, cliprect);
	return 0;
}

// Simulate floating bus for initial RAM detection in low ROM.
// WANTED: is a cleaner, more compatible way feasible?
READ8_MEMBER(rainbow_state::floating_bus_r)
{
	if ( m_maincpu->state_int(I8086_CS) != 0xF400)
		return space.read_byte(offset);

	if  ( m_maincpu->state_int(I8086_DS) < m_inp8->read() )
	{
		return space.read_byte(offset);
	} else
	{
		return (offset>>16) + 2;
	}
}

WRITE8_MEMBER(rainbow_state::floating_bus_w)
{
	space.write_byte(offset,data);
}


READ8_MEMBER(rainbow_state::share_z80_r)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			return m_shared[offset + 0x8000];
		}
		else if (offset < 0x8800)
		{
			return m_z80_private[offset & 0x7ff];
		}

		return m_shared[offset ^ 0x8000];
	}
	else
	{
		if (offset < 0x800)
		{
			return m_z80_private[offset];
		}

		return m_shared[offset];
	}

	return 0xff;
}

WRITE8_MEMBER(rainbow_state::share_z80_w)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			m_shared[offset + 0x8000] = data;
		}
		else if (offset < 0x8800)
		{
			m_z80_private[offset & 0x7ff] = data;
		}

		m_shared[offset ^ 0x8000] = data;
	}
	else
	{
		if (offset < 0x800)
		{
			m_z80_private[offset] = data;
		}
		else
		{
			m_shared[offset] = data;
		}
	}
}

READ8_MEMBER(rainbow_state::system_parameter_r)
{
/*  Info about option boards is in bits 0 - 3:
    Bundle card (1) | Floppy (2) | Graphics (4) | Memory option (8)

    0 1 2 3 4 5 6 7
    B F G M
   ( 1 means NOT present )
*/
	// Hard coded value 0x2000 - see DIP switch setup!
	return 0x0f - m_inp5->read() - m_inp6->read() - m_inp7->read() - (
										(m_inp8->read() > 0x2000) ? 8 : 0
										);
}

READ8_MEMBER(rainbow_state::comm_control_r)
{
	// Our simple COLDBOOT flag is adequate for the initial MHFU test (at BIOS location 00A8).

	// TODO: on real hardware, MHFU detection is disabled BY WRITING TO 0x10c (=> BIOS assumes power-up reset)
	//       MHFU is enabled by writing to 0x0c.
	if (m_COLDBOOT)
	{    m_COLDBOOT = 0;
		return ( 0x20 );  // bit 5 = watchdog detect.
	} else {
		return ( 0x00 );  // ERROR 16 is displayed = watchdog triggered
	}
}

WRITE8_MEMBER(rainbow_state::comm_control_w)
{
/* Communication control register of -COMM- port (when written):

   8088 LEDs:
   5  7  6  4    <- BIT POSITION
   D6 -D5-D4-D3  <- INTERNAL LED NUMBER (DEC PDF)
   -4--5--6--7-  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
*/
	output_set_value("led4", BIT(data, 5)); // LED "D6"
	output_set_value("led5", BIT(data, 7)); // LED "D5"
	output_set_value("led6", BIT(data, 6)); // LED "D4"
	output_set_value("led7", BIT(data, 4)); // LED "D3"

//  printf("%02x to COMM.CONTROL REGISTER\n", data);
}

// EMULATOR TRAP (patched into ROM @ machine_start) via BIOS : call / offset +$21  (AL / AH)
WRITE8_MEMBER(rainbow_state::PORT90_W)
{
	//printf("KBD COMMAND : %02x to AL (90)\n", data);

	m_KBD = 0; // reset previous command.

	if (data == 0xfd) {      // Powerup (beep)
		m_beep->set_state(1);
		m_beep_counter=600;  // BELL = 125 ms
	}

	if (data == 0xa7) {
			m_beep->set_state(1);
		m_beep_counter=600;  // BELL = 125 ms
	}

	if (data == 0x9f) {    // emit a keyclick (2ms)
		m_beep->set_state(1);
		m_beep_counter=25; // longer than calculated ( 9,6 )
	}

	if (data == 0x13) {  // light LEDs -
		m_KBD = 0x13;
	}
	if (data == 0x11) {  // switch off LEDs -
			m_KBD = 0x11;
	}
}

WRITE8_MEMBER(rainbow_state::PORT91_W)
{
	//printf("KBD PARAM %02x to AH (91) \n", data);

	// 4 leds, represented in the low 4 bits of a byte
	if (m_KBD == 0x13) {  // light LEDs -
		if (data & 1) { output_set_value("led8", 0); } //   KEYBOARD :  "Wait" LED
		if (data & 2) { output_set_value("led9", 0); } //   KEYBOARD :  "Compose" LED
		if (data & 4) { output_set_value("led10", 0); } //  KEYBOARD :  "Lock" LED
		if (data & 8) { output_set_value("led11", 0); } //  KEYBOARD :  "Hold" LED
		m_KBD = 0; // reset previous command.
	}
	if (m_KBD == 0x11) {  // switch off LEDs -
		if (data & 1) { output_set_value("led8", 1); } //   KEYBOARD :  "Wait" LED
		if (data & 2) { output_set_value("led9", 1); } //   KEYBOARD :  "Compose" LED
		if (data & 4) { output_set_value("led10", 1); } //  KEYBOARD :  "Lock" LED
		if (data & 8) { output_set_value("led11", 1); } //  KEYBOARD :  "Hold" LED
		m_KBD = 0; // reset previous command.
	}

	if (m_KBD == 0x1b) {   /* enable the keyclick */
							/* max volume is 0, lowest is 0x7 */
		m_KBD = 0; // reset previous command.
	}
}

READ8_MEMBER(rainbow_state::i8088_latch_r)
{
//    printf("Read %02x from 8088 mailbox\n", m_8088_mailbox);
	m_i8088->set_input_line(INPUT_LINE_INT0, CLEAR_LINE);
	return m_8088_mailbox;
}

WRITE8_MEMBER(rainbow_state::i8088_latch_w)
{
//    printf("%02x to Z80 mailbox\n", data);
	m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf7);
	m_z80_mailbox = data;
}

READ8_MEMBER(rainbow_state::z80_latch_r)
{
//    printf("Read %02x from Z80 mailbox\n", m_z80_mailbox);
	m_z80->set_input_line(0, CLEAR_LINE);
	return m_z80_mailbox;
}

WRITE8_MEMBER(rainbow_state::z80_latch_w)
{
//    printf("%02x to 8088 mailbox\n", data);
	m_i8088->set_input_line_and_vector(INPUT_LINE_INT0, ASSERT_LINE, 0x27);
	m_8088_mailbox = data;
}

WRITE8_MEMBER(rainbow_state::z80_diskdiag_read_w)
{
	m_zflip = true;
}

WRITE8_MEMBER(rainbow_state::z80_diskdiag_write_w)
{
/*   Z80 LEDs:
     4   5   6  <- bit #
    D11 D10 -D9 <- INTERNAL LED NUMBER (see PDF)
    -1 --2-- 3  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
*/
	output_set_value("led1", BIT(data, 4)); // LED "D11"
	output_set_value("led2", BIT(data, 5)); // LED "D10"
	output_set_value("led3", BIT(data, 6)); // LED "D9"

	m_zflip = false;
}

WRITE8_MEMBER(rainbow_state::z80_diskcontrol_write_w)
{
	//printf("%02x to z80 DISK CONTROL (W)\n", data);

	// TODO: this logic is a bit primitive. According to the spec, the RX-50 drive LED only turns on if
	//       (a) spindle motor runs (b) disk is in drive (c) door closed (d) drive side is selected
	if ( (data & 1) && (data & 8) )
		output_set_value("led20", 0); // DISKETTE 0 SELECTED & MOTOR 0 ON => LIGHT "DRIVE A"
	else
		output_set_value("led20", 1);

	if ( (data & 2) && (data & 8) )
		output_set_value("led21", 0); // DISKETTE 1 SELECTED & MOTOR 0 ON => LIGHT "DRIVE B"
	else
		output_set_value("led21", 1);
}

READ8_MEMBER( rainbow_state::read_video_ram_r )
{
	return m_p_ram[offset];
}

INTERRUPT_GEN_MEMBER(rainbow_state::vblank_irq)
{
	device.execute().set_input_line_and_vector(INPUT_LINE_INT0, ASSERT_LINE, 0x20);
}

WRITE8_MEMBER( rainbow_state::clear_video_interrupt )
{
	m_i8088->set_input_line(INPUT_LINE_INT0, CLEAR_LINE);
}

READ8_MEMBER( rainbow_state::diagnostic_r )
{
//    printf("%02x DIP value ORed to diagnostic\n", ( m_inp1->read() | m_inp2->read() | m_inp3->read()   )  );

	return ( (m_diagnostic & (0xf1)) | (    m_inp1->read() |
											m_inp2->read() |
											m_inp3->read()   
									   )
			);
}

WRITE8_MEMBER( rainbow_state::diagnostic_w )
{
//    printf("%02x to diag port (PC=%x)\n", data, space.device().safe_pc());
	m_SCREEN_BLANK = (data & 2) ? false : true;

	if ( !(data & 0x40)  && (m_diagnostic & 0x40) ) // if set to 1 (first) and later set to 0...
	{  
		//  SAVE / PROGRAM NVM: transfer data from volatile memory to NVM 
		memcpy( m_p_nvram, m_p_vol_ram, 256); 
	}

	if ( (data & 0x80)  && !(m_diagnostic & 0x80) ) // if set to 0 (first) and later set to 1...
	{
	    // READ / RECALL NVM: transfer data from NVM to volatile memory 
		memcpy( m_p_vol_ram, m_p_nvram, 256);
	}
		
	if (!(data & 1))
	{
		m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z80_halted = true;
	}

	if ((data & 1) && (m_z80_halted))
	{
		m_zflip = true;
		m_z80_halted = false;
		m_z80->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z80->reset();
	}

	m_diagnostic = data;
}


// KEYBOARD
void rainbow_state::update_kbd_irq()
{
	if ((m_kbd_rx_ready) || (m_kbd_tx_ready))
	{
		m_i8088->set_input_line_and_vector(INPUT_LINE_INT0, ASSERT_LINE, 0x26);
	}
	else
	{
		m_i8088->set_input_line(INPUT_LINE_INT0, CLEAR_LINE);
	}
}


READ_LINE_MEMBER(rainbow_state::kbd_rx)
{
//    printf("read keyboard\n");
	return 0x00;
}

WRITE_LINE_MEMBER(rainbow_state::kbd_tx)
{
//    printf("%02x to keyboard\n", state);
}

WRITE_LINE_MEMBER(rainbow_state::kbd_rxready_w)
{
//    printf("rxready %d\n", state);
	m_kbd_rx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

WRITE_LINE_MEMBER(rainbow_state::kbd_txready_w)
{
//    printf("txready %d\n", state);
	m_kbd_tx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

TIMER_DEVICE_CALLBACK_MEMBER(rainbow_state::keyboard_tick)
{
	m_kbd8251->transmit_clock();
	m_kbd8251->receive_clock();

	if (m_beep_counter > 1)
			m_beep_counter--;
	else
		if ( m_beep_counter == 1 )
		{   m_beep->set_state(0);
			m_beep_counter = 0;
		}
}

static const vt_video_interface video_interface =
{
	"chargen",
	DEVCB_DRIVER_MEMBER(rainbow_state, read_video_ram_r),
	DEVCB_DRIVER_MEMBER(rainbow_state, clear_video_interrupt)
};

/* F4 Character Displayer */
static const gfx_layout rainbow_charlayout =
{
	8, 10,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 15*8, 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( rainbow )
	GFXDECODE_ENTRY( "chargen", 0x0000, rainbow_charlayout, 0, 1 )
GFXDECODE_END

// Rainbow Z80 polls only, no IRQ/DRQ are connected
const wd17xx_interface rainbow_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{FLOPPY_0, FLOPPY_1}
};

static const floppy_interface floppy_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	"floppy_5_25",
	NULL
};

static const i8251_interface i8251_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(rainbow_state, kbd_rx),         // rxd in
	DEVCB_DRIVER_LINE_MEMBER(rainbow_state, kbd_tx),         // txd out
	DEVCB_NULL,         // dsr
	DEVCB_NULL,         // dtr
	DEVCB_NULL,         // rts
	DEVCB_DRIVER_LINE_MEMBER(rainbow_state, kbd_rxready_w),
	DEVCB_DRIVER_LINE_MEMBER(rainbow_state, kbd_txready_w),
	DEVCB_NULL,         // tx empty
	DEVCB_NULL          // syndet
};

static MACHINE_CONFIG_START( rainbow, rainbow_state )
	MCFG_DEFAULT_LAYOUT(layout_rainbow)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8088, XTAL_24_0734MHz / 5)
	MCFG_CPU_PROGRAM_MAP(rainbow8088_map)
	MCFG_CPU_IO_MAP(rainbow8088_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rainbow_state,  vblank_irq)

	MCFG_CPU_ADD("subcpu",Z80, XTAL_24_0734MHz / 6)
	MCFG_CPU_PROGRAM_MAP(rainbowz80_mem)
	MCFG_CPU_IO_MAP(rainbowz80_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(132*10, 49*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*10-1, 0, 25*10-1)
	MCFG_SCREEN_UPDATE_DRIVER(rainbow_state, screen_update_rainbow)
	MCFG_GFXDECODE(rainbow)
	MCFG_PALETTE_LENGTH(3)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, monochrome_amber)
	MCFG_RAINBOW_VIDEO_ADD("vt100_video", video_interface)

		/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)

	MCFG_FD1793_ADD("wd1793", rainbow_wd17xx_interface )
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(floppy_intf)
	MCFG_SOFTWARE_LIST_ADD("flop_list","rainbow")

	MCFG_I8251_ADD("kbdser", i8251_intf)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", rainbow_state, keyboard_tick, attotime::from_hz(4800))

	MCFG_LK201_ADD()
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( rainbow )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "23-022e5-00.bin",  0xf0000, 0x4000, CRC(9d1332b4) SHA1(736306d2a36bd44f95a39b36ebbab211cc8fea6e))
	ROM_RELOAD(0xf4000,0x4000)
	ROM_LOAD( "23-020e5-00.bin", 0xf8000, 0x4000, CRC(8638712f) SHA1(8269b0d95dc6efbe67d500dac3999df4838625d8)) // German, French, English
	//ROM_LOAD( "23-015e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Dutch, French, English
	//ROM_LOAD( "23-016e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Finish, Swedish, English
	//ROM_LOAD( "23-017e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Danish, Norwegian, English
	//ROM_LOAD( "23-018e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Spanish, Italian, English
	ROM_RELOAD(0xfc000,0x4000)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE          INIT COMPANY                         FULLNAME       FLAGS */
COMP( 1982, rainbow, 0,      0,       rainbow,   rainbow, driver_device, 0,  "Digital Equipment Corporation", "Rainbow 100B", GAME_NOT_WORKING | GAME_IMPERFECT_COLORS)
