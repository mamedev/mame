// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

Dunfield 6809 Portable

2009-05-12 Skeleton driver.
2011-09-21 connected to terminal, notes added [Robbbert]

Chips used (planned?):
- 6809E CPU
- 6845 CRTC
- 6840 CTC
- 6551 ACIA Console
- 6551 ACIA Aux
- 6850 ACIA Unknown purpose
- uPD765 FDC
- 2764 8K ROM for CPU
- 2732 4K ROM for Chargen (not dumped)
- 6x 6264 RAM
- 3x 5516 RAM
- XTALs: 14.745MHz, 16MHz

So much for the official documentation.

In practice, it reads/writes to a terminal, and doesn't use most of the other
devices.

'maincpu' (F9DD): unmapped program memory write to 00F0 = 05 & FF
'maincpu' (F9E3): unmapped program memory read from 0001 & FF <----- these 2 are CLR 0001
'maincpu' (F9E3): unmapped program memory write to 0001 = 00 & FF
'maincpu' (F9E6): unmapped program memory read from 0005 & FF <----- these 2 are CLR 0005
'maincpu' (F9E6): unmapped program memory write to 0005 = 00 & FF
'maincpu' (F9E9): unmapped program memory write to 0002 = 0B & FF <-- these 2 are STD 0002
'maincpu' (F9E9): unmapped program memory write to 0003 = 1E & FF
'maincpu' (F9EC): unmapped program memory write to 0006 = 0B & FF <-- these 2 are STD 0006
'maincpu' (F9EC): unmapped program memory write to 0007 = 1E & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF <-- the remainder seems to be disk related
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA41): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA46): unmapped program memory write to 00F0 = 04 & FF
'maincpu' (FA82): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF <-- now it gives up & prints an error


Layout of devices differences:
                  Schematic       Actual
6551 1              0000           0000
6551 2              0100           0004
Disk status         0400           00F0
Disk commands       0401           00F0-00F7
Parallel terminal   not there      00FF
CE on UPD765        0400           0200
TC on UPD765        0500           0300

Also, pins 16,17,18 on the UPD765 are incorrect in the schematic.


ToDo:
- Need better documentation
- Need a boot disk image

**********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/mos6551.h"
#include "machine/upd765.h"
#include "machine/terminal.h"


namespace {

class d6809_state : public driver_device
{
public:
	d6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void d6809(machine_config &config);

private:
	u8 term_r();
	void term_w(u8 data);
	void kbd_put(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	u8 m_term_data = 0U;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};

u8 d6809_state::term_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

void d6809_state::term_w(u8 data)
{
	if ((data > 0) && (data < 0x80))
		m_terminal->write(data);
}

void d6809_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	// 00-FF is for various devices.
	map(0x0000, 0x0003).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x0004, 0x0007).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x00f0, 0x00f7).ram(); // for now
	//map(0x00f0, 0x00f0).r(m_fdc, FUNC(upd765a_device::msr_r));
	map(0x00ff, 0x00ff).rw(FUNC(d6809_state::term_r), FUNC(d6809_state::term_w));
	map(0x0200, 0x0201).mirror(0xfe).m(m_fdc, FUNC(upd765a_device::map));
	map(0x0300, 0x0300).mirror(0xff).lw8(NAME([this] (u8 data){ m_fdc->tc_w(1); m_fdc->tc_w(0); }));
	map(0x1000, 0xdfff).ram();
	map(0xe000, 0xffff).rom().region("roms", 0);
}


/* Input ports */
static INPUT_PORTS_START( d6809 )
INPUT_PORTS_END


void d6809_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void d6809_state::machine_start()
{
	save_item(NAME(m_term_data));
}

void d6809_state::machine_reset()
{
	m_fdc->set_ready_line_connected(1);
	m_fdc->set_unscaled_clock(8_MHz_XTAL / 2); // 4MHz for minifloppy
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
}

static void floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


void d6809_state::d6809(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(14'745'600) / 8); // MC68B09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &d6809_state::mem_map);

	MOS6551(config, "acia1", XTAL(14'745'600) / 8); // uses Q clock
	MOS6551(config, "acia2", XTAL(14'745'600) / 8); // uses Q clock

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(d6809_state::kbd_put));

	// Floppy
	UPD765A(config, m_fdc, 8'000'000, true, true);
	//m_fdc->drq_wr_callback().set(m_fdc, FUNC(upd765a_device::dack_w));   // pin not emulated
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( d6809 )
	ROM_REGION( 0x2000, "roms", 0 )
	ROM_LOAD( "d6809.rom", 0x0000, 0x2000, CRC(2ceb40b8) SHA1(780111541234b4f0f781a118d955df61daa56e7e))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME         FLAGS
COMP( 1983, d6809, 0,      0,      d6809,   d6809, d6809_state, empty_init, "Dunfield", "6809 Portable", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
