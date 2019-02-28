// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha,Jon Sturm
/***************************************************************************
TI-85 and TI-86 drivers by Krzysztof Strzecha
TI-83 Plus, TI-84 Plus, and Siliver Edition support by Jon Sturm

Notes:
1. After start TI-85 waits for ON key interrupt, so press ON key to start
   calculator. ************* PRESS THE "Q" KEY TO TURN IT ON. ********************
2. Only difference between all TI-85 drivers is ROM version.
3. TI-86 is TI-85 with more RAM and ROM.
4. Only difference between all TI-86 drivers is ROM version.
5. Video engine (with grayscale support) based on the idea found in VTI source
   emulator written by Rusty Wagner.
6. NVRAM is saved properly only when calculator is turned off before exiting MAME.
7. To receive data from TI press "R" immediately after TI starts to send data.
8. To request screen dump from calculator press "S".
9. TI-81 does not have a serial link.

Needed:
1. Info about ports 3 (bit 2 seems to be always 0) and 4.
2. Any info on TI-81 hardware.
3. ROM dumps of unemulated models.
4. Artworks.

New:
05/10/2002 TI-85 serial link works again.
17/09/2002 TI-85 snapshots loading fixed. Few code cleanups.
       TI-86 SNAPSHOT LOADING DOESNT WORK.
       TI-85, TI-86 SERIAL LINK DOESNT WORK.
08/09/2001 TI-81, TI-85, TI-86 modified to new core.
       TI-81, TI-85, TI-86 reset corrected.
21/08/2001 TI-81, TI-85, TI-86 NVRAM corrected.
20/08/2001 TI-81 ON/OFF fixed.
       TI-81 ROM bank switching added (port 5).
       TI-81 NVRAM support added.
15/08/2001 TI-81 kayboard is now mapped as it should be.
14/08/2001 TI-81 preliminary driver added.
05/07/2001 Serial communication corrected (transmission works now after reset).
02/07/2001 Many source cleanups.
       PCR added.
01/07/2001 Possibility to request screen dump from TI (received dumps are saved
       as t85i file).
29/06/2001 Received variables can be saved now.
19/06/2001 Possibility to receive variables from calculator (they are nor saved
       yet).
17/06/2001 TI-86 reset fixed.
15/06/2001 Possibility to receive memory backups from calculator.
07/06/2001 TI-85 reset fixed.
       Work on receiving data from calculator started.
04/06/2001 TI-85 is able to receive variables and memory backups.
14/05/2001 Many source cleanups.
11/05/2001 Release years corrected. Work on serial link started.
26/04/2001 NVRAM support added.
25/04/2001 Video engine totally rewritten so grayscale works now.
17/04/2001 TI-86 snapshots loading added.
       ti86grom driver added.
16/04/2001 Sound added.
       Five TI-86 drivers added (all features of TI-85 drivers without
       snapshot loading).
13/04/2001 Snapshot loading (VTI 2.0 save state files).
18/02/2001 Palette (not perfect).
       Contrast control (port 2) implemented.
       LCD ON/OFF implemented (port 3).
       Interrupts corrected (port 3) - ON/OFF and APD works now.
       Artwork added.
09/02/2001 Keypad added.
       200Hz timer interrupts implemented.
       ON key and its interrupts implemented.
       Calculator is now fully usable.
02/02/2001 Preliminary driver

To do:
- port 7 (TI-86)
- port 4 (all models)
- artwork (all models)
- port 0 link (TI-82 and TI-83)
- add TI-73, TI-83+ and T84+ drivers


TI-81 memory map

    CPU: Z80 2MHz
        0000-7fff ROM
        8000-ffff RAM (?)

TI-82 memory map

    CPU: Z80 6MHz
        0000-3fff ROM 0
        4000-7fff ROM 1-7 (switched)
        8000-ffff RAM

TI-83 memory map

    CPU: Z80 6MHz
        0000-3fff ROM 0
        4000-7fff ROM 1-15 (switched)
        8000-ffff RAM

TI-83Plus memory map

    CPU: Z80 8MHz (running at 6 MHz)
        0000-3fff ROM 0
        4000-7fff ROM 0-31 or RAM 0-1 (switched)
        7000-bfff ROM 0-31 or RAM 0-1 (switched)
        c000-ffff RAM 0-31 or RAM 0-1 (switched)

TI-85 memory map

    CPU: Z80 6MHz
        0000-3fff ROM 0
        4000-7fff ROM 1-7 (switched)
        8000-ffff RAM

TI-86 memory map

    CPU: Z80 6MHz
        0000-3fff ROM 0
        4000-7fff ROM 0-15 or RAM 0-7 (switched)
        7000-bfff ROM 0-15 or RAM 0-7 (switched)
        c000-ffff RAM 0

Interrupts:

    IRQ: 200Hz timer
         ON key

TI-81 ports:
    0: Video buffer offset (write only)
    1: Keypad
    2: Contrast (write only)
    3: ON status, LCD power
    4: Video buffer width, interrupt control (write only)
    5: ?
    6:
    7: ?

TI-82 ports:
    0: Link
    1: Keypad
    2: Memory page
    3: ON status, LCD power
    4: Video buffer width, interrupt control (write only)
    10: Control port for the display controller
    11: Data port for the display controller

TI-83 ports:
    0: Link + Memory page
    1: Keypad
    2: Memory page
    3: ON status, LCD power
    4: Video buffer width, interrupt control (write only)
    10: Control port for the display controller
    11: Data port for the display controller
    14: Battery Status

TI-83Plus ports:
    0: Link
    1: Keypad
    2: ?
    3: ON status, LCD power
    4: Interrupt status
    6: Memory page 1
    7: Memory page 2
    10: Control port for the display controller
    11: Data port for the display controller

TI-83PlusSE ports:
    0: Link
    1: Keypad
    2: ?
    3: ON status, LCD power
    4: Interrupt status
    5: Memory page 3
    6: Memory page 1
    7: Memory page 2
    10: Controll port for the display controller
    11: Data port for the display controller
    15: Asic Version

TI-85 ports:
    0: Video buffer offset (write only)
    1: Keypad
    2: Contrast (write only)
    3: ON status, LCD power
    4: Video buffer width, interrupt control (write only)
    5: Memory page
    6: Power mode
    7: Link

TI-86 ports:
    0: Video buffer offset (write only)
    1: Keypad
    2: Contrast (write only)
    3: ON status, LCD power
    4: Power mode
    5: Memory page
    6: Memory page
    7: Link

***************************************************************************/

#include "emu.h"
#include "includes/ti85.h"

#include "cpu/z80/z80.h"
#include "imagedev/snapquik.h"
#include "machine/bankdev.h"
#include "screen.h"


/* port i/o functions */

void ti85_state::ti81_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti85_port_0000_r), FUNC(ti85_state::ti85_port_0000_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti85_port_0002_r), FUNC(ti85_state::ti85_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti85_port_0003_r), FUNC(ti85_state::ti85_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0004_r), FUNC(ti85_state::ti85_port_0004_w));
	map(0x0005, 0x0005).rw(FUNC(ti85_state::ti85_port_0005_r), FUNC(ti85_state::ti85_port_0005_w));
	map(0x0007, 0x0007).w(FUNC(ti85_state::ti81_port_0007_w));
}

void ti85_state::ti85_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti85_port_0000_r), FUNC(ti85_state::ti85_port_0000_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti85_port_0002_r), FUNC(ti85_state::ti85_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti85_port_0003_r), FUNC(ti85_state::ti85_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0004_r), FUNC(ti85_state::ti85_port_0004_w));
	map(0x0005, 0x0005).rw(FUNC(ti85_state::ti85_port_0005_r), FUNC(ti85_state::ti85_port_0005_w));
	map(0x0006, 0x0006).rw(FUNC(ti85_state::ti85_port_0006_r), FUNC(ti85_state::ti85_port_0006_w));
	map(0x0007, 0x0007).rw(FUNC(ti85_state::ti8x_serial_r), FUNC(ti85_state::ti8x_serial_w));
}

void ti85_state::ti82_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti8x_serial_r), FUNC(ti85_state::ti8x_serial_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti82_port_0002_r), FUNC(ti85_state::ti82_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti85_port_0003_r), FUNC(ti85_state::ti85_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0004_r), FUNC(ti85_state::ti85_port_0004_w));
	map(0x0010, 0x0010).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0011, 0x0011).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
}

void ti85_state::ti81v2_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti82_port_0002_r), FUNC(ti85_state::ti82_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti85_port_0003_r), FUNC(ti85_state::ti85_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0004_r), FUNC(ti85_state::ti85_port_0004_w));
	map(0x0010, 0x0010).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0011, 0x0011).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
}

void ti85_state::ti83_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti83_port_0000_r), FUNC(ti85_state::ti83_port_0000_w));  //TODO
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti83_port_0002_r), FUNC(ti85_state::ti83_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti83_port_0003_r), FUNC(ti85_state::ti83_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0004_r), FUNC(ti85_state::ti85_port_0004_w));
	map(0x0010, 0x0010).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0011, 0x0011).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
	map(0x0014, 0x0014).portr("BATTERY");
}

void ti85_state::ti83p_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti8x_plus_serial_r), FUNC(ti85_state::ti8x_plus_serial_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).r(FUNC(ti85_state::ti83p_port_0002_r));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti83_port_0003_r), FUNC(ti85_state::ti83p_int_mask_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti83p_port_0004_r), FUNC(ti85_state::ti83p_port_0004_w));
	map(0x0006, 0x0006).rw(FUNC(ti85_state::ti86_port_0005_r), FUNC(ti85_state::ti83p_port_0006_w));
	map(0x0007, 0x0007).rw(FUNC(ti85_state::ti86_port_0006_r), FUNC(ti85_state::ti83p_port_0007_w));
	map(0x0010, 0x0010).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0011, 0x0011).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
//  AM_RANGE(0x0014, 0x0014) AM_WRITE(ti83p_port_0014_w )
}

void ti85_state::ti83pse_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti8x_plus_serial_r), FUNC(ti85_state::ti8x_plus_serial_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti83pse_port_0002_r), FUNC(ti85_state::ti83pse_int_ack_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti83_port_0003_r), FUNC(ti85_state::ti83p_int_mask_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti83p_port_0004_r), FUNC(ti85_state::ti83pse_port_0004_w));
	map(0x0005, 0x0005).rw(FUNC(ti85_state::ti83pse_port_0005_r), FUNC(ti85_state::ti83pse_port_0005_w));
	map(0x0006, 0x0006).rw(FUNC(ti85_state::ti86_port_0005_r), FUNC(ti85_state::ti83pse_port_0006_w));
	map(0x0007, 0x0007).rw(FUNC(ti85_state::ti86_port_0006_r), FUNC(ti85_state::ti83pse_port_0007_w));
	map(0x0009, 0x0009).r(FUNC(ti85_state::ti83pse_port_0009_r));
	map(0x0010, 0x0010).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0011, 0x0011).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
	map(0x0012, 0x0012).rw("t6a04", FUNC(t6a04_device::control_read), FUNC(t6a04_device::control_write));
	map(0x0013, 0x0013).rw("t6a04", FUNC(t6a04_device::data_read), FUNC(t6a04_device::data_write));
	map(0x0014, 0x0014).w(FUNC(ti85_state::ti83p_port_0014_w));
	map(0x0015, 0x0015).r(FUNC(ti85_state::ti83pse_port_0015_r));
	map(0x0020, 0x0020).rw(FUNC(ti85_state::ti83pse_port_0020_r), FUNC(ti85_state::ti83pse_port_0020_w));
	map(0x0021, 0x0021).rw(FUNC(ti85_state::ti83pse_port_0021_r), FUNC(ti85_state::ti83pse_port_0021_w));

	map(0x0030, 0x0030).rw(FUNC(ti85_state::ti83pse_ctimer1_setup_r), FUNC(ti85_state::ti83pse_ctimer1_setup_w));
	map(0x0031, 0x0031).rw(FUNC(ti85_state::ti83pse_ctimer1_loop_r), FUNC(ti85_state::ti83pse_ctimer1_loop_w));
	map(0x0032, 0x0032).rw(FUNC(ti85_state::ti83pse_ctimer1_count_r), FUNC(ti85_state::ti83pse_ctimer1_count_w));
	map(0x0033, 0x0033).rw(FUNC(ti85_state::ti83pse_ctimer2_setup_r), FUNC(ti85_state::ti83pse_ctimer2_setup_w));
	map(0x0034, 0x0034).rw(FUNC(ti85_state::ti83pse_ctimer2_loop_r), FUNC(ti85_state::ti83pse_ctimer2_loop_w));
	map(0x0035, 0x0035).rw(FUNC(ti85_state::ti83pse_ctimer2_count_r), FUNC(ti85_state::ti83pse_ctimer2_count_w));
	map(0x0036, 0x0036).rw(FUNC(ti85_state::ti83pse_ctimer3_setup_r), FUNC(ti85_state::ti83pse_ctimer3_setup_w));
	map(0x0037, 0x0037).rw(FUNC(ti85_state::ti83pse_ctimer3_loop_r), FUNC(ti85_state::ti83pse_ctimer3_loop_w));
	map(0x0038, 0x0038).rw(FUNC(ti85_state::ti83pse_ctimer3_count_r), FUNC(ti85_state::ti83pse_ctimer3_count_w));

	map(0x0055, 0x0055).r(FUNC(ti85_state::ti84pse_port_0055_r));
	map(0x0056, 0x0056).r(FUNC(ti85_state::ti84pse_port_0056_r));
}

void ti85_state::ti86_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0x0000).rw(FUNC(ti85_state::ti85_port_0000_r), FUNC(ti85_state::ti85_port_0000_w));
	map(0x0001, 0x0001).rw(FUNC(ti85_state::ti8x_keypad_r), FUNC(ti85_state::ti8x_keypad_w));
	map(0x0002, 0x0002).rw(FUNC(ti85_state::ti85_port_0002_r), FUNC(ti85_state::ti85_port_0002_w));
	map(0x0003, 0x0003).rw(FUNC(ti85_state::ti85_port_0003_r), FUNC(ti85_state::ti85_port_0003_w));
	map(0x0004, 0x0004).rw(FUNC(ti85_state::ti85_port_0006_r), FUNC(ti85_state::ti85_port_0006_w));
	map(0x0005, 0x0005).rw(FUNC(ti85_state::ti86_port_0005_r), FUNC(ti85_state::ti86_port_0005_w));
	map(0x0006, 0x0006).rw(FUNC(ti85_state::ti86_port_0006_r), FUNC(ti85_state::ti86_port_0006_w));
	map(0x0007, 0x0007).rw(FUNC(ti85_state::ti8x_serial_r), FUNC(ti85_state::ti8x_serial_w));
}

/* memory w/r functions */

void ti85_state::ti81_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankr("bank2");
	map(0x8000, 0xffff).ram().share("nvram");
}

void ti85_state::ti86_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void ti85_state::ti83pse_banked_mem(address_map &map)
{
	map(0x0000, 0x1fffff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x200000, 0x21BFFF).ram().share("nvram");
}


void ti85_state::ti84p_banked_mem(address_map &map)
{
	map(0x0000, 0xfffff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x200000, 0x21BFFF).ram().share("nvram");
}

void ti85_state::ti83p_banked_mem(address_map &map)
{
	map(0x00000, 0x7ffff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x100000, 0x107fff).ram().share("nvram");
}

void ti85_state::ti83p_asic_mem(address_map &map)
{
	map(0x0000, 0x3fff).rw(m_membank[0], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0x4000, 0x7fff).w(m_membank[1], FUNC(address_map_bank_device::write8)).r(FUNC(ti85_state::ti83p_membank2_r));
	map(0x8000, 0xbfff).w(m_membank[2], FUNC(address_map_bank_device::write8)).r(FUNC(ti85_state::ti83p_membank3_r));
	map(0xc000, 0xffff).rw(m_membank[3], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

/* keyboard input */

static INPUT_PORTS_START (ti81)
	PORT_START("BIT0")   /* bit 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_F5)
	PORT_START("BIT1")   /* bit 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRACE") PORT_CODE(KEYCODE_F4)
	PORT_START("BIT2")   /* bit 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LN") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ZOOM") PORT_CODE(KEYCODE_F3)
	PORT_START("BIT3")   /* bit 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOG") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RANGE") PORT_CODE(KEYCODE_F2)
	PORT_START("BIT4")   /* bit 4 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EE") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^2") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y=") PORT_CODE(KEYCODE_F1)
	PORT_START("BIT5")   /* bit 5 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SIN") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^-1") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
	PORT_START("BIT6")   /* bit 6 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_PGDN)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARS") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRGM") PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MATRX") PORT_CODE(KEYCODE_F7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MATH") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_TILDE)
	PORT_START("BIT7")   /* bit 7 */
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X|T") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALPHA") PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_START("ON")   /* ON */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/OFF") PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START (ti85)
	PORT_START("BIT0")   /* bit 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER   (ENTRY)") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(-)     (ANS     |_|)") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".       (:       Z)") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0       (CHAR    Y)") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5      (M5)") PORT_CODE(KEYCODE_F5)
	PORT_START("BIT1")   /* bit 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+       (MEM     X)") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3       (VARS    W)") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2       (TEST    V)") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1       (BASE    U)") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STORE   (RCL     =)") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4      (M4)") PORT_CODE(KEYCODE_F4)
	PORT_START("BIT2")   /* bit 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-       (LIST    T)") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6       (STRNG   S)") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5       (CONV    R)") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4       (CONS    Q)") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",       (ANGLE   P)") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3      (M3)") PORT_CODE(KEYCODE_F3)
	PORT_START("BIT3")   /* bit 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*       (MATH    O)") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9       (CPLX    N)") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8       (VECTR   M)") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7       (MATRX   L)") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^2     (SQRT    K)") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2      (M2)") PORT_CODE(KEYCODE_F2)
	PORT_START("BIT4")   /* bit 4 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/       (CALC    J)") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")       (]       I)") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(       ([       H)") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EE      (X^-1    G)") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LN      (e^x     F)") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1      (M1)") PORT_CODE(KEYCODE_F1)
	PORT_START("BIT5")   /* bit 5 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^       (PI      E)") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN     (TAN^-1  D)") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS     (COS^-1  C)") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SIN     (SIN^-1  B)") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOG     (10^x    A)") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
	PORT_START("BIT6")   /* bit 6 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR   (TOLER)") PORT_CODE(KEYCODE_PGDN)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CUSTOM  (CATALOG)") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRGM    (POLY)") PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STAT    (SIMULT)") PORT_CODE(KEYCODE_F7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GRAPH   (SOLVER)") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EXIT    (QUIT)") PORT_CODE(KEYCODE_ESC)
	PORT_START("BIT7")   /* bit 7 */
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL     (INS)") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x-VAR   (LINK    x)") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALPHA   (alpha)") PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MORE    (MODE)") PORT_CODE(KEYCODE_TILDE)
	PORT_START("ON")   /* ON */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/OFF") PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START (ti82)
	PORT_START("BIT0")   /* bit 0 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_F5)
	PORT_START("BIT1")   /* bit 1 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TRACE") PORT_CODE(KEYCODE_F4)
	PORT_START("BIT2")   /* bit 2 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LN") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ZOOM") PORT_CODE(KEYCODE_F3)
	PORT_START("BIT3")   /* bit 3 */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOG") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WINDOW") PORT_CODE(KEYCODE_F2)
	PORT_START("BIT4")   /* bit 4 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_END)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^2") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y=") PORT_CODE(KEYCODE_F1)
	PORT_START("BIT5")   /* bit 5 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SIN") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x^-1") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
	PORT_START("BIT6")   /* bit 6 */
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_PGDN)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("VARS") PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRGM") PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MATRX") PORT_CODE(KEYCODE_F7)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MATH") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_ESC)
	PORT_START("BIT7")   /* bit 7 */
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STAT") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("x-VAR") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALPHA") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_START("ON")   /* ON */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/OFF") PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START (ti83)
	PORT_INCLUDE( ti82 )

	PORT_START("BATTERY")
		PORT_DIPNAME( 0x01, 0x01, "Battery Status" )
		PORT_DIPSETTING( 0x01, DEF_STR( Normal ) )
		PORT_DIPSETTING( 0x00, "Low Battery" )
INPUT_PORTS_END

/* machine definition */
void ti85_state::ti81(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2000000);        /* 2 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &ti85_state::ti81_mem);
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti81_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(0);
	screen.set_size(96, 64);
	screen.set_visarea(0, 96-1, 0, 64-1);
	screen.set_screen_update(FUNC(ti85_state::screen_update_ti85));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ti85_state::ti85_palette), 224, 224);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void ti85_state::ti85(machine_config &config)
{
	ti81(config);
	m_maincpu->set_clock(6000000);        /* 6 MHz */
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti85_io);

	MCFG_MACHINE_RESET_OVERRIDE(ti85_state, ti85 )

	subdevice<screen_device>("screen")->set_size(128, 64);
	subdevice<screen_device>("screen")->set_visarea(0, 128-1, 0, 64-1);

	TI8X_LINK_PORT(config, m_link_port, default_ti8x_link_devices, nullptr);
}


MACHINE_CONFIG_START(ti85_state::ti85d)
	ti85(config);
	MCFG_SNAPSHOT_ADD("snapshot", ti85_state, ti8x, "sav")
	//TI85SERIAL(config, "tiserial");
MACHINE_CONFIG_END


void ti85_state::ti82(machine_config &config)
{
	ti81(config);
	m_maincpu->set_clock(6000000);        /* 6 MHz */
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti82_io);

	MCFG_MACHINE_RESET_OVERRIDE(ti85_state, ti85 )

	subdevice<screen_device>("screen")->set_screen_update("t6a04", FUNC(t6a04_device::screen_update));

	subdevice<palette_device>("palette")->set_entries(2).set_init(FUNC(ti85_state::ti82_palette));

	T6A04(config, "t6a04", 0).set_size(96, 64);

	TI8X_LINK_PORT(config, m_link_port, default_ti8x_link_devices, nullptr);
}

void ti85_state::ti81v2(machine_config &config)
{
	ti82(config);
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti81v2_io);

	config.device_remove("linkport");
}

void ti85_state::ti83(machine_config &config)
{
	ti81(config);
	m_maincpu->set_clock(6000000);        /* 6 MHz */
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti83_io);

	MCFG_MACHINE_RESET_OVERRIDE(ti85_state, ti85 )

	subdevice<screen_device>("screen")->set_screen_update("t6a04", FUNC(t6a04_device::screen_update));

	subdevice<palette_device>("palette")->set_entries(2).set_init(FUNC(ti85_state::ti82_palette));

	T6A04(config, "t6a04", 0).set_size(96, 64);
}

MACHINE_CONFIG_START(ti85_state::ti86)
	ti85(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti85_state::ti86_mem);
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti86_io);

	MCFG_MACHINE_START_OVERRIDE(ti85_state, ti86 )
	MCFG_MACHINE_RESET_OVERRIDE(ti85_state, ti85 )

	MCFG_SNAPSHOT_ADD("snapshot", ti85_state, ti8x, "sav")
MACHINE_CONFIG_END

void ti85_state::ti83p(machine_config &config)
{
	ti81(config);
	m_maincpu->set_clock(6000000);        /* 8 MHz running at 6 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &ti85_state::ti83p_asic_mem);
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti83p_io);

	MCFG_MACHINE_START_OVERRIDE(ti85_state, ti83p )
	MCFG_MACHINE_RESET_OVERRIDE(ti85_state, ti83p )

	subdevice<screen_device>("screen")->set_screen_update("t6a04", FUNC(t6a04_device::screen_update));

	subdevice<palette_device>("palette")->set_entries(2).set_init(FUNC(ti85_state::ti82_palette));

	ADDRESS_MAP_BANK(config, m_membank[0]).set_map(&ti85_state::ti83p_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, m_membank[1]).set_map(&ti85_state::ti83p_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, m_membank[2]).set_map(&ti85_state::ti83p_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, m_membank[3]).set_map(&ti85_state::ti83p_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	T6A04(config, "t6a04", 0).set_size(96, 64);

	TI8X_LINK_PORT(config, m_link_port, default_ti8x_link_devices, nullptr);

	AMD_29F400T(config, m_flash);
}

void ti85_state::ti83pse(machine_config &config)
{
	ti83p(config);
	m_maincpu->set_clock(15000000);
	m_maincpu->set_addrmap(AS_IO, &ti85_state::ti83pse_io);

	m_membank[0]->set_map(&ti85_state::ti83pse_banked_mem);

	m_membank[1]->set_map(&ti85_state::ti83pse_banked_mem);

	m_membank[2]->set_map(&ti85_state::ti83pse_banked_mem);

	m_membank[3]->set_map(&ti85_state::ti83pse_banked_mem);

	MCFG_MACHINE_START_OVERRIDE(ti85_state, ti83pse )
	FUJITSU_29F160T(config.replace(), m_flash, 0);
}

void ti85_state::ti84p(machine_config &config)
{
	ti83pse(config);
	m_membank[0]->set_map(&ti85_state::ti84p_banked_mem);

	m_membank[1]->set_map(&ti85_state::ti84p_banked_mem);

	m_membank[2]->set_map(&ti85_state::ti84p_banked_mem);

	m_membank[3]->set_map(&ti85_state::ti84p_banked_mem);

	MCFG_MACHINE_START_OVERRIDE(ti85_state, ti84p )
	AMD_29F800T(config.replace(), m_flash, 0);
}

void ti85_state::ti84pse(machine_config &config)
{
	ti83pse(config);
	MCFG_MACHINE_START_OVERRIDE(ti85_state, ti84pse )
}

void ti85_state::ti73(machine_config &config)
{
	ti83p(config);
	config.device_remove("linkport");
	//TI73SERIAL(config, "tiserial");
}

ROM_START (ti73)
	ROM_REGION (0x80000, "flash",0)
	ROM_DEFAULT_BIOS("v160")
	ROM_SYSTEM_BIOS( 0, "v160", "V 1.60" )
	ROMX_LOAD( "ti73v160.bin", 0x00000, 0x80000, CRC(bb0e3a16) SHA1(d62c2c7532698962818a747a7f32e35e41dfe338), ROM_BIOS(0) )
ROM_END

ROM_START (ti73b)
	ROM_REGION (0x80000, "flash",0)
	ROM_DEFAULT_BIOS("v191")
	ROM_SYSTEM_BIOS( 0, "v13004", "V 1.3004" )
	ROMX_LOAD( "ti73bv13004.bin", 0x00000, 0x80000, CRC(453701d8) SHA1(371d1f74a5e26ed749e12baac104f0069f329f44), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "V 1.40" )
	ROMX_LOAD( "ti73bv140.bin", 0x00000, 0x80000, CRC(057e85ae) SHA1(4c45c8b26190e887bb9cdc3b185fd7e703922cbc), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v150", "V 1.50" )
	ROMX_LOAD( "ti73bv150.bin", 0x00000, 0x80000, CRC(c0edfb53) SHA1(1049363587b6d7985356aa2467a0118e6cc6dc37), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v160", "V 1.60" )
	ROMX_LOAD( "ti73bv160.bin", 0x00000, 0x80000, CRC(28d07d9d) SHA1(7795720a68ca7017e682a8f2fe617b0cd758c008), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v180", "V 1.80" )
	ROMX_LOAD( "ti73bv180.bin", 0x00000, 0x80000, CRC(7d3b9ee6) SHA1(93bfc8d951c526e1be7c0e1bebc43dd20cd4c3b1), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v185", "V 1.85" )
	ROMX_LOAD( "ti73bv185.bin", 0x00000, 0x80000, CRC(4e7d68e7) SHA1(52a8b71fee7cda11935d6e89825842b4aad046dd), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v190", "V 1.90" )
	ROMX_LOAD( "ti73bv190.bin", 0x00000, 0x80000, CRC(8726a8db) SHA1(636551d75fd0bccbbc89ea6749bb1153e9545e26), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v191", "V 1.91" )
	ROMX_LOAD( "ti73bv191.bin", 0x00000, 0x80000, CRC(f3785d57) SHA1(ad73d0c61ef6a51a04902a9b30a58992a2d860c4), ROM_BIOS(7) )
ROM_END

ROM_START (ti81)
	ROM_REGION (0x08000, "bios",0)
	ROM_DEFAULT_BIOS("v18")
	ROM_SYSTEM_BIOS( 0, "v11", "V 1.1K" )
	ROMX_LOAD( "ti81v11k.bin", 0x00000, 0x8000, CRC(0b860a63) SHA1(84a71cfc8818ca4b7d0caa76ffbf6d0463eaf7c6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v16", "V 1.6K" )
	ROMX_LOAD( "ti81v16k.bin", 0x00000, 0x8000, CRC(452ca838) SHA1(92649f0f3bce7d8829d950cecd6532d7f7db1297), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v18", "V 1.8K" )
	ROMX_LOAD( "ti81v18k.bin", 0x00000, 0x8000, CRC(94ac58e2) SHA1(ba915cfe2fe50a452ef8287db8f2244e29056d54), ROM_BIOS(2) )
	//No dumps 1.0, and 2.0 from ticalc.org, less sure about 1.6K
ROM_END

ROM_START (ti81v2)
	ROM_REGION (0x08000, "bios",0)
	ROM_DEFAULT_BIOS("v20")
	ROM_SYSTEM_BIOS( 0, "v20", "V 2.0V" )
	ROMX_LOAD( "ti81v20v.bin", 0x00000, 0x8000, CRC(cfbd12da) SHA1(d2a923526d98f1046fcb583e46951939ba66bdb9), ROM_BIOS(0) )
ROM_END

ROM_START (ti82)
	ROM_REGION (0x20000, "bios",0)
	ROM_DEFAULT_BIOS("v19")
	ROM_SYSTEM_BIOS( 0, "v16", "V 16.0" )
	ROMX_LOAD( "ti82v16.bin", 0x00000, 0x20000, CRC(e2f5721c) SHA1(df300ae52e105faf2785a8ae9f42e84e4308d460), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v17", "V 17.0" )
	ROMX_LOAD( "ti82v17.bin", 0x00000, 0x20000, CRC(0fc956d4) SHA1(77eef7d2db5ad1fb5de9129086a18428ddf66195), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v18", "V 18.0" )
	ROMX_LOAD( "ti82v18.bin", 0x00000, 0x20000, CRC(6a320f03) SHA1(9ee15ebf0a1f8bde5bef982b5db4ce120c605d29), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v19", "V 19.0" )
	ROMX_LOAD( "ti82v19.bin", 0x00000, 0x20000, CRC(ed4cf9ff) SHA1(10dc2d01c62b4e971a6ed7ebc75ca0f2e3dc4f95), ROM_BIOS(3) )
	//Rom versions according to ticalc.org 3*, 4*, 7*, 8.0, 10.0, 12.0, 15.0, 16.0, 17.0, 18.0, 19.0, 19.006
ROM_END

ROM_START (ti83)
	ROM_REGION (0x40000, "bios",0)
	ROM_DEFAULT_BIOS("v110")
	ROM_SYSTEM_BIOS( 0, "v102", "V 1.02" )
	ROMX_LOAD( "ti83v102.bin", 0x00000, 0x40000, CRC(7ee5d27b) SHA1(ce08f6a808701fc6672230a790167ee485157561), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v103", "V 1.03" )
	ROMX_LOAD( "ti83v103.bin", 0x00000, 0x40000, CRC(926f72a4) SHA1(8399e384804d8d29866caa4c8763d7a61946a467), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v104", "V 1.04" )
	ROMX_LOAD( "ti83v104.bin", 0x00000, 0x40000, CRC(dccb73d3) SHA1(33877ff637dc5f4c5388799fd7e2159b48e72893), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v106", "V 1.06" )
	ROMX_LOAD( "ti83v106.bin", 0x00000, 0x40000, CRC(2eae1cf0) SHA1(3d65c2a1b771ce8e5e5a0476ec1aa9c9cdc0e833), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v107", "V 1.07" )
	ROMX_LOAD( "ti83v107.bin", 0x00000, 0x40000, CRC(4bf05697) SHA1(ef66dad3e7b2b6a86f326765e7dfd7d1a308ad8f), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v108", "V 1.08" )
	ROMX_LOAD( "ti83v108.bin", 0x00000, 0x40000, CRC(0c6aafcc) SHA1(9c74f0b61655e9e160e92164db472ad7ee02b0f8), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v110", "V 1.10" )
	ROMX_LOAD( "ti83v110.bin", 0x00000, 0x40000, CRC(7faee2d2) SHA1(25b373b58523647bb7b904001d391615e0b79bee), ROM_BIOS(6) )
	//Rom versions according to ticalc.org 1.02, 1.03, 1.04, 1.06, 1.07, 1.08, 1.10
ROM_END

ROM_START (ti83p)
	ROM_REGION (0x80000, "flash",0)
	ROM_DEFAULT_BIOS("v116")
	ROM_SYSTEM_BIOS( 0, "v103", "V 1.03" )
	ROMX_LOAD( "ti83pv103.bin", 0x00000, 0x80000, CRC(da466be0) SHA1(37eaeeb9fb5c18fb494e322b75070e80cc4d858e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v110", "V 1.10" )
	ROMX_LOAD( "ti83pv110.bin", 0x00000, 0x80000, CRC(62683990) SHA1(f86cdefe4ed5ef9965cd9eb667cb859e2cb10e19), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v112", "V 1.12" )
	ROMX_LOAD( "ti83pv112.bin", 0x00000, 0x80000, CRC(ddca5026) SHA1(6615df5554076b6b81bd128bf847d2ff046e556b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v113", "V 1.13" )
	ROMX_LOAD( "ti83pv113.bin", 0x00000, 0x80000, CRC(30a243aa) SHA1(9b79e994ea1ce7af05b68f8ecee8b1b1fc3f0810), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v114", "V 1.14" )
	ROMX_LOAD( "ti83pv114.bin", 0x00000, 0x80000, CRC(b32059c7) SHA1(46c66ba0421c03fc42f5afb06c7d3af812786140), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v115", "V 1.15" )
	ROMX_LOAD( "ti83pv115.bin", 0x00000, 0x80000, CRC(9288029b) SHA1(8bd05fd47cab4028f275d1cc5383fd4f0e193474), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v116", "V 1.16" )
	ROMX_LOAD( "ti83pv116.bin", 0x00000, 0x80000, CRC(0b7cd006) SHA1(290bc81159ea061d8ccb56a6f63e042f150afb32), ROM_BIOS(6) )
	//Missing 1.17, 1.18, and 1.19
ROM_END

ROM_START (ti83pb)
	ROM_REGION (0x80000, "flash",0)
	ROM_DEFAULT_BIOS("v119")
	ROM_SYSTEM_BIOS( 0, "v103", "V 1.03" )
	ROMX_LOAD( "ti83pbv103.bin", 0x00000, 0x80000, CRC(745472fa) SHA1(e1707e0b56e72bb126fa1dda430c659a726beaf7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v110", "V 1.10" )
	ROMX_LOAD( "ti83pbv110.bin", 0x00000, 0x80000, CRC(edf9a1d9) SHA1(edbb725f12c10dd1dd8d5c4a4f836bf03659411d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v112", "V 1.12" )
	ROMX_LOAD( "ti83pbv112.bin", 0x00000, 0x80000, CRC(ce3f9427) SHA1(b8b8cd806ceac68f2d35ef34e6695fa9ea2d8ad1), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v113", "V 1.13" )
	ROMX_LOAD( "ti83pbv113.bin", 0x00000, 0x80000, CRC(3327c8c0) SHA1(07830de8efc99ea6ceab388e6c0603c28a23454f), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v114", "V 1.14" )
	ROMX_LOAD( "ti83pbv114.bin", 0x00000, 0x80000, CRC(408134b9) SHA1(791ff9fc2e184d5048e349fb5b65830719d5199b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v115", "V 1.15" )
	ROMX_LOAD( "ti83pbv115.bin", 0x00000, 0x80000, CRC(a16a4bff) SHA1(a0374a5d5f25e3f9dc1c241447233cf3a23e7946), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v116", "V 1.16" )
	ROMX_LOAD( "ti83pbv116.bin", 0x00000, 0x80000, CRC(b5e00ef6) SHA1(23b131263b696c03f778eb5d37411be9a86cf752), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v118", "V 1.18" )
	ROMX_LOAD( "ti83pbv118.bin", 0x00000, 0x80000, CRC(0915b0a0) SHA1(48c270c383c2d05058693a5bf58d462936bbb335), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "v119", "V 1.19" )
	ROMX_LOAD( "ti83pbv119.bin", 0x00000, 0x80000, CRC(58f14c79) SHA1(1fddd44d54f3ff12bfb548fcb03ce36b5a4f295a), ROM_BIOS(8) )
	//Missing 1.17
ROM_END

ROM_START (ti85)
	ROM_REGION (0x20000, "bios",0)
	ROM_DEFAULT_BIOS("v100")
	ROM_SYSTEM_BIOS( 0, "v30a", "V 3.0A" )
	ROMX_LOAD( "ti85v30a.bin", 0x00000, 0x20000, CRC(de4c0b1a) SHA1(f4cf4b8309372dbe26187bb279545f5d4bd48fc1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v40",  "V 4.0" )
	ROMX_LOAD( "ti85v40.bin",  0x00000, 0x20000, CRC(a1723a17) SHA1(ff5866636bb3f206a6bf39cc9c9dc8308332aaf0), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v50",  "V 5.0" )
	ROMX_LOAD( "ti85v50.bin",  0x00000, 0x20000, CRC(781fa403) SHA1(bf20d520d8efd7e5ae269789ca4b3c71848ac32a), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v60",  "V 6.0" )
	ROMX_LOAD( "ti85v60.bin",  0x00000, 0x20000, CRC(b694a117) SHA1(36d58e2723e5ae4ffe0f8da691fa9a83bfe9e06b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v80",  "V 8.0" )
	ROMX_LOAD( "ti85v80.bin",  0x00000, 0x20000, CRC(7f296338) SHA1(765d5c612b6ffc0d1ded8f79bcbe880b1b562a98), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v90",  "V 9.0" )
	ROMX_LOAD( "ti85v90.bin",  0x00000, 0x20000, CRC(6a0a94d0) SHA1(7742bf8a6929a21d06f306b494fc03b1fbdfe3e4), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v100", "V 10.0" )
	ROMX_LOAD( "ti85v100.bin", 0x00000, 0x20000, CRC(053325b0) SHA1(36da1080c34e7b53cbe8463be5804e30e4a50dc8), ROM_BIOS(6) )
	//No_dumps 1.0, 2.0 and 7.0 according to ticalc.org
ROM_END

ROM_START (ti86)
	ROM_REGION (0x40000, "bios",0)
	ROM_DEFAULT_BIOS("v16")
	ROM_SYSTEM_BIOS( 0, "v12", "V 1.2" )
	ROMX_LOAD( "ti86v12.bin", 0x00000, 0x40000, CRC(bdf16105) SHA1(e40b22421c31bf0af104518b748ae79cd21d9c57), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v13", "V 1.3" )
	ROMX_LOAD( "ti86v13.bin", 0x00000, 0x40000, CRC(073ef70f) SHA1(5702d4bb835bdcbfa8075ffd620fca0eaf3a1592), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v14", "V 1.4" )
	ROMX_LOAD( "ti86v14.bin", 0x00000, 0x40000, CRC(fe6e2986) SHA1(23e0fb9a1763d5b9a7b0e593f09c2ff30c760866), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v15", "V 1.5" )
	ROMX_LOAD( "ti86v15.bin", 0x00000, 0x40000, BAD_DUMP CRC(e6e10546) SHA1(5ca63fdfc965ae3fb8e0695263cf9da41f6ecb90), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v16", "V 1.6" )
	ROMX_LOAD( "ti86v16.bin", 0x00000, 0x40000, CRC(37e02acc) SHA1(b5ad204885e5dde23a22f18f8d5eaffca69d638d), ROM_BIOS(4) )
	//Rom versions according to ticalc.org 1.2, 1.3, 1.4, 1.5, 1.6
ROM_END

ROM_START (ti83pse)
	ROM_REGION (0x200000, "flash", 0)
	ROM_SYSTEM_BIOS( 0, "v116", "V 1.16" )
	ROM_DEFAULT_BIOS("v116")
	ROMX_LOAD( "ti83psev116.bin", 0x00000, 0x200000, CRC(d2570863) SHA1(d4214b3c0ebb26e10fe95294ac72a90d2ba99537), ROM_BIOS(0) )
ROM_END

ROM_START (ti83pseb)
	ROM_REGION (0x200000, "flash", 0)
	ROM_DEFAULT_BIOS("v112")
	ROM_SYSTEM_BIOS( 0, "v112", "V 1.12" )
	ROMX_LOAD( "ti83psebv112.bin", 0x00000, 0x200000, CRC(e8cfcdb7) SHA1(322929d289c17c247da7da3674d6115f1740fa49), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v113", "V 1.13" )
	ROMX_LOAD( "ti83psebv113.bin", 0x00000, 0x200000, CRC(cf90e998) SHA1(29b92a32e3ceae7d918fc404fec50a53f35b574c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v114", "V 1.14" )
	ROMX_LOAD( "ti83psebv114.bin", 0x00000, 0x200000, CRC(4842c167) SHA1(2a1938474df970f92ee2349912b7824685a4da99), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v115", "V 1.15" )
	ROMX_LOAD( "ti83psebv115.bin", 0x00000, 0x200000, CRC(79a7bcbb) SHA1(1c47f2299eedde8db21b9ec469ba01a1b14533db), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v116", "V 1.16" )
	ROMX_LOAD( "ti83psebv116.bin", 0x00000, 0x200000, CRC(f75b896f) SHA1(75c8356ee89f35cb197684f3581cbfa3904c2f0a), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v118", "V 1.18" )
	ROMX_LOAD( "ti83psebv118.bin", 0x00000, 0x200000, CRC(0ad0a741) SHA1(cb83a6f1517fc5d34a29cdf4b1d30ea2762b2a95), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v119", "V 1.19" )
	ROMX_LOAD( "ti83psebv119.bin", 0x00000, 0x200000, CRC(200dd7d0) SHA1(8177bc6d5489d575cbfa9a004d097fc08c6f8c86), ROM_BIOS(6) )
ROM_END

ROM_START (ti84pse)
	ROM_REGION (0x200000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v221", "V 2.21" )
	ROMX_LOAD( "ti84psev221.bin", 0x00000, 0x200000, CRC(da8b3c8e) SHA1(736fae1929089167a3af6290e04e9278b0a3d1a6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v222", "V 2.22" )
	ROMX_LOAD( "ti84psev222.bin", 0x00000, 0x200000, CRC(dc2931db) SHA1(319f1ec6accdbe2309f9ffcd8d9970fa2a422c4d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v230", "V 2.30" )
	ROMX_LOAD( "ti84psev230.bin", 0x00000, 0x200000, CRC(8800c73a) SHA1(cb9ad540137ede275ff22e293ae0f7cc31b6663d), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v240", "V 2.40" )
	ROMX_LOAD( "ti84psev240.bin", 0x00000, 0x200000, CRC(2aed41c4) SHA1(6886f4c07718f0dfa43b397ff492a6b4b06ded15), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v241", "V 2.41" )
	ROMX_LOAD( "ti84psev241.bin", 0x00000, 0x200000, CRC(3dcb18ba) SHA1(728834cb426c09f6b00d1fd89e81eb154488854c), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v243", "V 2.43" )
	ROMX_LOAD( "ti84psev243.bin", 0x00000, 0x200000, CRC(1e9707f8) SHA1(767a5238882d97fac550971adbfbe48f82f2772f), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v253mp", "V 2.53MP" )
	ROMX_LOAD( "ti84psev253mp.bin", 0x00000, 0x200000, CRC(3e52683a) SHA1(80050ae2a8f128b291d3a8973ab32e879172f2b9), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84psev255mp.bin", 0x00000, 0x200000, CRC(70439fdd) SHA1(201e585caa64836829ea57c1291c6136c778ef55), ROM_BIOS(7) )
ROM_END

ROM_START (ti84psev3)
	ROM_REGION (0x200000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84psev3v255mp.bin", 0x00000, 0x200000, CRC(daa7cb89) SHA1(eabdc9b46a1cb7fef60b0fabf36ab7d484cdb3bf), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pseb)
	ROM_REGION (0x200000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v221", "V 2.21" )
	ROMX_LOAD( "ti84psebv221.bin", 0x00000, 0x200000, CRC(cff9a231) SHA1(132ff36d4ebed04452fc0b54341b29db882d1292), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v222", "V 2.22" )
	ROMX_LOAD( "ti84psebv222.bin", 0x00000, 0x200000, CRC(c95baf64) SHA1(167a8bc911fadd62e0b9eb2c4f3c96009795fb2f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v230", "V 2.30" )
	ROMX_LOAD( "ti84psebv230.bin", 0x00000, 0x200000, CRC(f7e19a09) SHA1(7d800eb350d6c7dd9fd6aaab44de6c2de70f6f49), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v240", "V 2.40" )
	ROMX_LOAD( "ti84psebv240.bin", 0x00000, 0x200000, CRC(550c1cf7) SHA1(4cfcb232a310d42d252d5481ad4417ea1f55288e), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v241", "V 2.41" )
	ROMX_LOAD( "ti84psebv241.bin", 0x00000, 0x200000, CRC(422a4589) SHA1(c70b5c58ba723e60787f8a5b0caef94ee9cec087), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v243", "V 2.43" )
	ROMX_LOAD( "ti84psebv243.bin", 0x00000, 0x200000, CRC(61765acb) SHA1(a725f58532706deff2f60d700030da0e99a2c21d), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v253mp", "V 2.53MP" )
	ROMX_LOAD( "ti84psebv253mp.bin", 0x00000, 0x200000, CRC(41b33509) SHA1(92ef7dd17d8998f21a652c5d0c3f631fc993677f), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84psebv255mp.bin", 0x00000, 0x200000, CRC(0fa2c2ee) SHA1(ac01dbe4c5fb0f5c83ddc2b7907647992717fbce), ROM_BIOS(7) )
ROM_END

ROM_START (ti84p)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v221", "V 2.21" )
	ROMX_LOAD( "ti84pv221.bin", 0x00000, 0x100000, CRC(2f55c4d9) SHA1(04e91c5d089b4a56f12e949f0bd6936df6539d2a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v222", "V 2.22" )
	ROMX_LOAD( "ti84pv222.bin", 0x00000, 0x100000, CRC(bdb80f06) SHA1(7c6e1557acd90bb649a353616d3fc96fcbb59b10), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v230", "V 2.30" )
	ROMX_LOAD( "ti84pv230.bin", 0x00000, 0x100000, CRC(ce3229f6) SHA1(abbb228cb6efbca724008d15ac724919821163d8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v240", "V 2.40" )
	ROMX_LOAD( "ti84pv240.bin", 0x00000, 0x100000, CRC(4d0b1e08) SHA1(53ae06d4c492402ce5ae999c9d8be7437c0797ed), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v241", "V 2.41" )
	ROMX_LOAD( "ti84pv241.bin", 0x00000, 0x100000, CRC(5879e99a) SHA1(8e7651fe10b05e870ab5e211c1ff800e60f2a933), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v243", "V 2.43" )
	ROMX_LOAD( "ti84pv243.bin", 0x00000, 0x100000, CRC(0234caea) SHA1(d50b70fd2e141e920807e083b53056f6bbe2870c), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v253mp", "V 2.53MP" )
	ROMX_LOAD( "ti84pv253mp.bin", 0x00000, 0x100000, CRC(84ab2f4f) SHA1(4083f416c8698af4e1c512b21c526bc7e1aa1c23), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84pv255mp.bin", 0x00000, 0x100000, CRC(4af31251) SHA1(8f67269346644b87e7cd0f353f5f4030e787cf57), ROM_BIOS(7) )
ROM_END

ROM_START (ti84pv2)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v221", "V 2.21" )
	ROMX_LOAD( "ti84pv2v221.bin", 0x00000, 0x100000, CRC(5a23a22e) SHA1(2e1dcf163c0a0ea725ed9e264ba12c3cae67c969), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v222", "V 2.22" )
	ROMX_LOAD( "ti84pv2v222.bin", 0x00000, 0x100000, CRC(c8ce69f1) SHA1(aba6291dd020d375093bd56174416462f0e44130), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v230", "V 2.30" )
	ROMX_LOAD( "ti84pv2v230.bin", 0x00000, 0x100000, CRC(bb444f01) SHA1(f75c1866f27ff29ce8e113ff676ccdc6a53553d6), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v240", "V 2.40" )
	ROMX_LOAD( "ti84pv2v240.bin", 0x00000, 0x100000, CRC(387d78ff) SHA1(4ea9c7d56ce545fc3a4fb19f13dcc53638bf439f), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v241", "V 2.41" )
	ROMX_LOAD( "ti84pv2v241.bin", 0x00000, 0x100000, CRC(2d0f8f6d) SHA1(5048cebac3814ed56e82c9d6f094be8ffaa15e10), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v243", "V 2.43" )
	ROMX_LOAD( "ti84v2pv243.bin", 0x00000, 0x100000, CRC(7742ac1d) SHA1(dc02d658412e7f00205906bdf8ba6b252a193506), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v253mp", "V 2.53MP" )
	ROMX_LOAD( "ti84pv2v253mp.bin", 0x00000, 0x100000, CRC(f1dd49b8) SHA1(c9b592f3451778df1a4ada76cdd2f859c6c5df26), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84pv2v255mp.bin", 0x00000, 0x100000, CRC(3f8574a6) SHA1(0f88e719512f2691fff6c8bcc89292158086f841), ROM_BIOS(7) )
ROM_END

ROM_START (ti84pov2)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255orn")
	ROM_SYSTEM_BIOS( 0, "v255orn", "V 2.55MP/ORn" )
	ROMX_LOAD( "ti84pov2v255orn.bin", 0x00000, 0x100000, CRC(13d7d311) SHA1(c0fd22cd822d77dde0b037c33123178e30275d27), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pv3)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84pv3v255mp.bin", 0x00000, 0x100000, CRC(a9b5d5a6) SHA1(d500540feca974f6e8fa269981cfb25dc951c338), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pob)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255orn")
	ROM_SYSTEM_BIOS( 0, "v255orn", "V 2.55/ORn" )
	ROMX_LOAD( "ti84pobv255orn.bin", 0x00000, 0x100000, CRC(b8992646) SHA1(643c2f03d0743ffe61d9ca5ed813bc748add7c44), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pov3)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255orn")
	ROM_SYSTEM_BIOS( 0, "v255orn", "V 2.55/ORn" )
	ROMX_LOAD( "ti84pov3v255orn.bin", 0x00000, 0x100000, CRC(85e77211) SHA1(32e1962f33ec1c3b76921cda2a96e95fd0a6c805), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pb)
	ROM_REGION (0x100000, "flash",0)
	ROM_DEFAULT_BIOS("v255mp")
	ROM_SYSTEM_BIOS( 0, "v221", "V 2.21" )
	ROMX_LOAD( "ti84pbv221.bin", 0x00000, 0x100000, CRC(f16d5779) SHA1(8c81ea6046863a91ab50222f2dc4c4fa73b08e8f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v222", "V 2.22" )
	ROMX_LOAD( "ti84pbv222.bin", 0x00000, 0x100000, CRC(63809ca6) SHA1(71a859ee1b8c23b8c09c718d1d96623e14a2728f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v230", "V 2.30" )
	ROMX_LOAD( "ti84pbv230.bin", 0x00000, 0x100000, CRC(100aba56) SHA1(d0a34121dcc437f2df60a646b879442800912fd9), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v240", "V 2.40" )
	ROMX_LOAD( "ti84pbv240.bin", 0x00000, 0x100000, CRC(93338da8) SHA1(37ffe852928124fab1bd61d66c44a4bb356b60d6), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v241", "V 2.41" )
	ROMX_LOAD( "ti84pbv241.bin", 0x00000, 0x100000, CRC(86417a3a) SHA1(3be9456268b9c0b2e5cf3a65af2b148ef74ce89b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v243", "V 2.43" )
	ROMX_LOAD( "ti84pbv243.bin", 0x00000, 0x100000, CRC(dc0c594a) SHA1(2779db4987e22b3e3d946cecf5eb3942a0478eaa), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v253mp", "V 2.53MP" )
	ROMX_LOAD( "ti84pbv253mp.bin", 0x00000, 0x100000, CRC(5a93bcef) SHA1(3535a0bcbed9d7949c2791695a26e4b2db1af8ba), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v255mp", "V 2.55MP" )
	ROMX_LOAD( "ti84pbv255mp.bin", 0x00000, 0x100000, CRC(94cb81f1) SHA1(5bf30a7ebbebfa90f221cdddc931ae0b96c419db), ROM_BIOS(7) )
ROM_END

ROM_START (ti84pcse)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v42")
	ROM_SYSTEM_BIOS( 0, "v40", "V 4.0" )
	ROMX_LOAD( "ti84pcsev40.bin", 0x00000, 0x400000, CRC(e0b8ec78) SHA1(a4ffdfa0d2a8fc1b1356429675efc96b4f25fbc5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v42", "V 4.2" )
	ROMX_LOAD( "ti84pcsev42.bin", 0x00000, 0x400000, CRC(57d5373d) SHA1(06acbd22c9cb31320e022791ac03ba695f058654), ROM_BIOS(1) )
ROM_END

ROM_START (ti84pcsev2)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v42")
	ROM_SYSTEM_BIOS( 0, "v42", "V 4.2" )
	ROMX_LOAD( "ti84pcsev42.bin", 0x00000, 0x400000, CRC(4b6c2342) SHA1(e2a9d0124f852af79643438c994f13abc47e07af), ROM_BIOS(0) )
ROM_END

ROM_START (ti84pce)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v530")
	ROM_SYSTEM_BIOS( 0, "v500", "V 5.00" )
	ROMX_LOAD( "ti84pcev500.bin", 0x00000, 0x400000, CRC(e31ecdf9) SHA1(7f93a2e17b75debdeb5704e07092c48b3abfec9e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v510", "V 5.10" )
	ROMX_LOAD( "ti84pcev510.bin", 0x00000, 0x400000, CRC(042f7031) SHA1(6754edc7aefc9f74247bf5ff60e7546f77cd2898), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v515", "V 5.15" )
	ROMX_LOAD( "ti84pcev515.bin", 0x00000, 0x400000, CRC(2a958b6a) SHA1(6302ab3b4e3fca1ee6a05a3441b086ef7a57bee8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v520", "V 5.20" )
	ROMX_LOAD( "ti84pcev520.bin", 0x00000, 0x400000, CRC(a59c6633) SHA1(d02f20aa3c895254a0974db7e424dd91d075f859), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v521", "V 5.21" )
	ROMX_LOAD( "ti84pcev521.bin", 0x00000, 0x400000, CRC(89bc2ae1) SHA1(b0b83b2b0158e5b382fc12af95aa2a2e41f3ce6d), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v522", "V 5.22" )
	ROMX_LOAD( "ti84pcev522.bin", 0x00000, 0x400000, CRC(49ce1768) SHA1(f949c8f2832edd33a1b0dd4da0ab4c1f23e47b21), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "v530", "V 5.30" )
	ROMX_LOAD( "ti84pcev530.bin", 0x00000, 0x400000, CRC(c72f36b8) SHA1(6856fb2a9d0a2e338a89b91bb7680180a69482d3), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "v531", "V 5.31" )
	ROMX_LOAD( "ti84pcev531.bin", 0x00000, 0x400000, CRC(6d269f68) SHA1(9f9321a0cff17c331c92be127ec67ef67317968b), ROM_BIOS(7) )
ROM_END

ROM_START (ti83pcev15)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v530") // 5.30 is the default because 5.31 disables some features
	ROM_SYSTEM_BIOS( 0, "v515", "V 5.15" )
	ROMX_LOAD( "ti83pcev15v515.bin", 0x00000, 0x400000, CRC(f924d8e6) SHA1(ffb100c0d0478c414e7ba4dd9d73791d026b40ca), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v520", "V 5.20" )
	ROMX_LOAD( "ti83pcev15v520.bin", 0x00000, 0x400000, CRC(a403db1a) SHA1(a565d96e75bed354483c6904b9ee2b8054adc31e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v521", "V 5.21" )
	ROMX_LOAD( "ti83pcev15v521.bin", 0x00000, 0x400000, CRC(1dc2e3e3) SHA1(d8e44e1a8a6591b289766a881a81d33eca9a0ecc), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v522", "V 5.22" )
	ROMX_LOAD( "ti83pcev15v522.bin", 0x00000, 0x400000, CRC(6971746d) SHA1(321c7aeda6af8b42742d080e802979188421e06a), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v530", "V 5.30" )
	ROMX_LOAD( "ti83pcev15v530.bin", 0x00000, 0x400000, CRC(08ae7388) SHA1(6d9d98d090ac1b250d1f8ba8ef7c26eb448e7f8c), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v531", "V 5.31" )
	ROMX_LOAD( "ti83pcev15v531.bin", 0x00000, 0x400000, CRC(6643adb3) SHA1(b380e15946c1749a56600d18fee7d9d3c658dee3), ROM_BIOS(5) )
ROM_END

ROM_START (ti84pcev15)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v530") // 5.30 is the default because 5.31 disables some features
	ROM_SYSTEM_BIOS( 0, "v515", "V 5.15" )
	ROMX_LOAD( "ti84pcev15v515.bin", 0x00000, 0x400000, CRC(0318a913) SHA1(44256e23708c71b8f63660d3100e767d597c377f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v520", "V 5.20" )
	ROMX_LOAD( "ti84pcev15v520.bin", 0x00000, 0x400000, CRC(c2029323) SHA1(827c1e7ead58f4eabe5b7b942d0f24abd46f1633), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v521", "V 5.21" )
	ROMX_LOAD( "ti84pcev15v521.bin", 0x00000, 0x400000, CRC(880eefc9) SHA1(3449363553c093b6bb555306d91a4dd6dac8e891), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v522", "V 5.22" )
	ROMX_LOAD( "ti84pcev15v522.bin", 0x00000, 0x400000, CRC(b95fefd9) SHA1(db1515b3bd4eebeb5ee3c29597991fc7f1e8b84e), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v530", "V 5.30" )
	ROMX_LOAD( "ti84pcev15v530.bin", 0x00000, 0x400000, CRC(0148cc26) SHA1(72a10379bbd9d427c6e73afa9fe316cbd502f53c), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "v531", "V 5.31" )
	ROMX_LOAD( "ti84pcev15v531.bin", 0x00000, 0x400000, CRC(86511ea0) SHA1(ff14ec454fd1e0a2c436b4eed1eefca0d16aabfb), ROM_BIOS(5) )
ROM_END

ROM_START (ti84pcev30)
	ROM_REGION (0x400000, "flash",0)
	ROM_DEFAULT_BIOS("v530") // 5.30 is the default because 5.31 disables some features
	ROM_SYSTEM_BIOS( 0, "v530", "V 5.30" )
	ROMX_LOAD( "ti84pcev30v530.bin", 0x00000, 0x400000, CRC(cc7a7047) SHA1(0d348e60dc57276b1f8d5ff87935e47cdd27455c), ROM_BIOS(0) )
ROM_END

//    YEAR  NAME        PARENT   COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY              FULLNAME                                           FLAGS
COMP( 201?, ti84pob,    ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "Orion TI-84 Plus (bootleg)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 201?, ti84pov2,   ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "Orion TI-84 Plus (Boot Code 1.02)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 201?, ti84pov3,   ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "Orion TI-84 Plus (Boot Code 1.03)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
COMP( 1998, ti73,       0,       0,      ti73,    ti82,  ti85_state, empty_init, "Texas Instruments", "TI-73 Explorer",                                  MACHINE_NO_SOUND_HW )
COMP( 20??, ti73b,      ti73,    0,      ti73,    ti82,  ti85_state, empty_init, "Texas Instruments", "TI-73 Explorer (bootleg)",                        MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 1990, ti81,       0,       0,      ti81,    ti81,  ti85_state, empty_init, "Texas Instruments", "TI-81",                                           MACHINE_NO_SOUND_HW )
COMP( 1994, ti81v2,     ti81,    0,      ti81v2,  ti81,  ti85_state, empty_init, "Texas Instruments", "TI-81 v2.0",                                      MACHINE_NO_SOUND_HW )
COMP( 1993, ti82,       0,       0,      ti82,    ti82,  ti85_state, empty_init, "Texas Instruments", "TI-82",                                           MACHINE_NO_SOUND_HW )
COMP( 1996, ti83,       0,       0,      ti83,    ti83,  ti85_state, empty_init, "Texas Instruments", "TI-83",                                           MACHINE_NO_SOUND_HW )
COMP( 1999, ti83p,      0,       0,      ti83p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-83 Plus (Boot Code 1.00)",                     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti83pb,     ti83p,   0,      ti83p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-83 Plus (bootleg)",                            MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2001, ti83pse,    0,       0,      ti83pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-83 Plus Silver Edition (Boot Code 1.00)",      MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti83pseb,   ti83pse, 0,      ti83pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-83 Plus Silver Edition (bootleg)",             MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 201?, ti83pcev15, ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-83 Premium CE (Boot Code 5.1.5.0014)",         MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2004, ti84p,      0,       0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus (Boot Code 1.00)",                     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 200?, ti84pv2,    ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus (Boot Code 1.02)",                     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2011, ti84pv3,    ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus (Boot Code 1.03)",                     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti84pb,     ti84p,   0,      ti84p,   ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus (bootleg)",                            MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti84pcse,   ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus C Silver Edition (Boot Code 4.0)",     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti84pcsev2, ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus C Silver Edition (Boot Code 4.2)",     MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2015, ti84pce,    ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus CE (Boot Code 5.0.0.0089)",            MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2016, ti84pcev15, ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus CE (Boot Code 5.1.5.0014)",            MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2017, ti84pcev30, ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus CE (Boot Code 5.3.0.0037)",            MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2004, ti84pse,    0,       0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus Silver Edition (Boot Code 1.00)",      MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 2011, ti84psev3,  ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus Silver Edition (Boot Code 1.03)",      MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 20??, ti84pseb,   ti84pse, 0,      ti84pse, ti82,  ti85_state, empty_init, "Texas Instruments", "TI-84 Plus Silver Edition (bootleg)",             MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
COMP( 1992, ti85,       0,       0,      ti85d,   ti85,  ti85_state, empty_init, "Texas Instruments", "TI-85",                                           MACHINE_NO_SOUND_HW )
COMP( 1997, ti86,       0,       0,      ti86,    ti85,  ti85_state, empty_init, "Texas Instruments", "TI-86",                                           MACHINE_NO_SOUND_HW )
