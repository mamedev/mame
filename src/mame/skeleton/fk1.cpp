// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

FK-1

2009-05-12 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


namespace {

class fk1_state : public driver_device
{
public:
	fk1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_bankr(*this, "bankr1")
		, m_bankw(*this, "bankw1")
		, m_bank2(*this, "bank2")
	{ }

	void fk1(machine_config &config);

private:
	void ppi1_a_w(uint8_t data);
	void ppi1_b_w(uint8_t data);
	void ppi1_c_w(uint8_t data);
	uint8_t ppi1_a_r();
	uint8_t ppi1_b_r();
	uint8_t ppi1_c_r();
	void ppi2_a_w(uint8_t data);
	void ppi2_c_w(uint8_t data);
	uint8_t ppi2_b_r();
	uint8_t ppi2_c_r();
	void ppi3_a_w(uint8_t data);
	void ppi3_b_w(uint8_t data);
	void ppi3_c_w(uint8_t data);
	uint8_t ppi3_a_r();
	uint8_t ppi3_b_r();
	uint8_t ppi3_c_r();
	void pit_out0(int state);
	void pit_out1(int state);
	void pit_out2(int state);
	void intr_w(uint8_t data);
	uint8_t bank_ram_r();
	uint8_t bank_rom_r();
	void disk_w(uint8_t data);
	uint8_t mouse_r();
	void reset_int_w(uint8_t data);
	uint8_t m_video_rol;
	uint8_t m_int_vector;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(vsync_callback);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<ram_device> m_ram;
	required_memory_bank    m_bankr;
	required_memory_bank    m_bankw;
	required_memory_bank    m_bank2;
};


/*
Port A:
        Printer
Port B:
        Keyboard
Port C:
    READING :
        7 - / OBF(buffer overflow for the printer)
        6 - INTE printer
        5 - ERROR printer
        4 - /PEND, lack of paper
        3 - INTR printer
        2 - INTE keyboard
        1 - IBF (data from the keyboard)
        0 - INTR keyboard
    WRITING :
        6 - INTE printer
        2 - INTE keyboard
 */

void fk1_state::ppi1_a_w(uint8_t data)
{
//  logerror("fk1_ppi_1_a_w %02x\n",data);
}

void fk1_state::ppi1_b_w(uint8_t data)
{
//  logerror("fk1_ppi_1_b_w %02x\n",data);
}

void fk1_state::ppi1_c_w(uint8_t data)
{
	//logerror("fk1_ppi_1_c_w %02x\n",data);
}

uint8_t fk1_state::ppi1_a_r()
{
	//logerror("fk1_ppi_1_a_r\n");
	return 0xff;
}

uint8_t fk1_state::ppi1_b_r()
{
//  logerror("fk1_ppi_1_b_r\n");
	return 0;
}

uint8_t fk1_state::ppi1_c_r()
{
//  logerror("fk1_ppi_1_c_r\n");
	return 0;
}

/*
Port A:
    Writing data to disk
Port B:
    Reading data from disk
Port C:
    READING:
        7 - / OF A data write to disk,
        6 - INTE A,
        5 - Select the drive A, B,
        4 - Not connected
        3 - INTR A
        2 - INTE B read data,
        1 - IBF B read data,
        0 - INTR B

    WRITING:
        6 - INTE A - reading data
        2 - INTE B - writing data
*/

void fk1_state::ppi2_a_w(uint8_t data)
{
//  logerror("write to disk %02x\n",data);
}

void fk1_state::ppi2_c_w(uint8_t data)
{
//  logerror("fk1_ppi_2_c_w %02x\n",data);
}

uint8_t fk1_state::ppi2_b_r()
{
//  logerror("read from disk\n");
	return 0;
}

uint8_t fk1_state::ppi2_c_r()
{
//  logerror("fk1_ppi_2_c_r\n");
	return 0;
}


/*

Port A:
    6 - / disk, authorization disk operations,
    3 - INTE MOUSE, permit suspension from mouse,
    2 - INTE RTC from the system clock of 50 Hz,
    1 and 0 - to set the type for hours write to the disk (01 index, 11 address mark, mark the date, 00 other).

Port B:
    Video ROL register
Port C

    READING:
    7 - INDEX ( index mark on the disk)
    6 - I do not know,
    5 - WRITE_PROTECT the protected disk,
    4 - TRAC_00 as a sign of trace

    WRITING:
    3 - HEAD LOAD
    2 - TRACK_43
    1 - DIRC - direction to set the direction of stepping disk
    0 - STEP, move disk (0 1 .. 0.).

*/
void fk1_state::ppi3_a_w(uint8_t data)
{
//  logerror("fk1_ppi_3_a_w %02x\n",data);
}

void fk1_state::ppi3_b_w(uint8_t data)
{
	m_video_rol = data;
}

void fk1_state::ppi3_c_w(uint8_t data)
{
//  logerror("fk1_ppi_3_c_w %02x\n",data);
}

uint8_t fk1_state::ppi3_a_r()
{
//  logerror("fk1_ppi_3_a_r\n");
	return 0;
}

uint8_t fk1_state::ppi3_b_r()
{
	return m_video_rol;
}

uint8_t fk1_state::ppi3_c_r()
{
//  logerror("fk1_ppi_3_c_r\n");
	return 0;
}

void fk1_state::pit_out0(int state)
{
	// System time
	logerror("fk1_pit_out0\n");
}

void fk1_state::pit_out1(int state)
{
	// Timeout for disk operation
	logerror("fk1_pit_out1\n");
}

void fk1_state::pit_out2(int state)
{
	// Overflow for disk operations
	logerror("fk1_pit_out2\n");
}

/*
    0 no interrupt allowed,
    1 allowed INTR-7,
    2 allowed INTR-7 and INTR-6,
    8 any interruption allowed.
*/

void fk1_state::intr_w(uint8_t data)
{
	logerror("fk1_intr_w %02x\n",data);
}

uint8_t fk1_state::bank_ram_r()
{
	m_bankr->set_entry(0);
	m_bank2->set_entry(0);
	return 0;
}

uint8_t fk1_state::bank_rom_r()
{
	m_bankr->set_entry(1);
	m_bank2->set_entry(1);
	return 0;
}

/*
    4 - FORMAT authorization
    3 - READ_ADDRESS_MARK
    2 - READ_DATA_MARK
    1 - WRITE
    0 - READ
    Functions are allowed in one.
*/

void fk1_state::disk_w(uint8_t data)
{
//  logerror("fk1_disk_w %02x\n",data);
}

/*
7 and 6 - 1 to connected mouse and 0 if not connected,
5 - / T2, right-click
4 - / T1, left-click
3 - / BY, Y-axis
2 - / AY, Y-axis
1 - / BX, X-axis
0 - / AX, X-axis
*/

uint8_t fk1_state::mouse_r()
{
//  logerror("fk1_mouse_r\n");
	return 0;
}

/*Write to port 70 resets the interrupt from the system clock of 50 Hz. */

void fk1_state::reset_int_w(uint8_t data)
{
	logerror("fk1_reset_int_w\n");
}

void fk1_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bankr1").bankw("bankw1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xffff).ram();
}

void fk1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x20, 0x23).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x30).rw(FUNC(fk1_state::bank_ram_r), FUNC(fk1_state::intr_w));
	map(0x40, 0x41).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x50, 0x50).rw(FUNC(fk1_state::bank_rom_r), FUNC(fk1_state::disk_w));
	map(0x60, 0x63).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x70, 0x70).rw(FUNC(fk1_state::mouse_r), FUNC(fk1_state::reset_int_w));
}

/* Input ports */
static INPUT_PORTS_START( fk1 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0xA4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(fk1_state::keyboard_callback)
{
	if (ioport("LINE0")->read())
	{
		m_int_vector = 6;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

/*
7 ? DATA_READY
6 ? TIMEOUT
5 ? OVERFLOW
4 ? RTC_50HZ
3 ? MOUSE
2 ? SIO
1 ? KEYBOARD
0 ? PRINTER
*/

IRQ_CALLBACK_MEMBER(fk1_state::irq_callback)
{
	logerror("IRQ %02x\n", m_int_vector*2);
	return m_int_vector * 2;
}

TIMER_DEVICE_CALLBACK_MEMBER(fk1_state::vsync_callback)
{
	m_int_vector = 3;
	m_maincpu->set_input_line(0, HOLD_LINE);
}


void fk1_state::machine_start()
{
	save_item(NAME(m_video_rol));
	save_item(NAME(m_int_vector));
	u8 *r = m_ram->pointer();
	m_bankr->configure_entry(0, r);
	m_bankr->configure_entry(1, m_rom);
	m_bankw->configure_entry(0, r);
	m_bank2->configure_entry(0, r+0x4000);
	m_bank2->configure_entry(1, r+0x8000);
}

void fk1_state::machine_reset()
{
	m_bankr->set_entry(1);
	m_bankw->set_entry(0);
	m_bank2->set_entry(1);
}

uint32_t fk1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const ram = m_ram->pointer();

	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			uint8_t code = ram[x * 0x100 + ((y + m_video_rol) & 0xff) + 0x8000];
			for (int b = 0; b < 8; b++)
				bitmap.pix(y, x*8+b) =  ((code << b) & 0x80) ? 1 : 0;
		}
	}
	return 0;
}

void fk1_state::fk1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &fk1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &fk1_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(fk1_state::irq_callback));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(fk1_state::screen_update));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	pit8253_device &pit8253(PIT8253(config, "pit", 0));
	pit8253.set_clk<0>(50);
	pit8253.out_handler<0>().set(FUNC(fk1_state::pit_out0));
	pit8253.set_clk<1>(1000000);
	pit8253.out_handler<1>().set(FUNC(fk1_state::pit_out1));
	pit8253.set_clk<2>(0);
	pit8253.out_handler<2>().set(FUNC(fk1_state::pit_out2));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set(FUNC(fk1_state::ppi1_a_r));
	ppi1.out_pa_callback().set(FUNC(fk1_state::ppi1_a_w));
	ppi1.in_pb_callback().set(FUNC(fk1_state::ppi1_b_r));
	ppi1.out_pb_callback().set(FUNC(fk1_state::ppi1_b_w));
	ppi1.in_pc_callback().set(FUNC(fk1_state::ppi1_c_r));
	ppi1.out_pc_callback().set(FUNC(fk1_state::ppi1_c_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.out_pa_callback().set(FUNC(fk1_state::ppi2_a_w));
	ppi2.in_pb_callback().set(FUNC(fk1_state::ppi2_b_r));
	ppi2.in_pc_callback().set(FUNC(fk1_state::ppi2_c_r));
	ppi2.out_pc_callback().set(FUNC(fk1_state::ppi2_c_w));

	i8255_device &ppi3(I8255(config, "ppi3"));
	ppi3.in_pa_callback().set(FUNC(fk1_state::ppi3_a_r));
	ppi3.out_pa_callback().set(FUNC(fk1_state::ppi3_a_w));
	ppi3.in_pb_callback().set(FUNC(fk1_state::ppi3_b_r));
	ppi3.out_pb_callback().set(FUNC(fk1_state::ppi3_b_w));
	ppi3.in_pc_callback().set(FUNC(fk1_state::ppi3_c_r));
	ppi3.out_pc_callback().set(FUNC(fk1_state::ppi3_c_w));

	/* uart */
	I8251(config, "uart", 0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("48K");  // 32 for banks1,2 + 16 for vram

	TIMER(config, "keyboard_timer").configure_periodic(FUNC(fk1_state::keyboard_callback), attotime::from_hz(300));
	TIMER(config, "vsync_timer").configure_periodic(FUNC(fk1_state::vsync_callback), attotime::from_hz(50));
}

/* ROM definition */
ROM_START( fk1 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "orig", "Original BIOS" )
	ROMX_LOAD( "fk1.u65",      0x0000, 0x0800, CRC(145561f8) SHA1(a4eb17d773e51b34620c508b6cebcb4531ae99c2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "diag", "Diag BIOS" )
	ROMX_LOAD( "fk1-diag.u65", 0x0000, 0x0800, CRC(e0660ae1) SHA1(6ad609049b28f27126af0a8a6224362351073dee), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                  FULLNAME  FLAGS
COMP( 1989, fk1,  0,      0,      fk1,     fk1,   fk1_state, empty_init, "Statni statek Klicany", "FK-1",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
