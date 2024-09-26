// license:BSD-3-Clause
// copyright-holders:Christian Grossler
/*

 Olivetti M20 skeleton driver, by incog (19/05/2009)

Needs a proper Z8001 CPU core, check also

ftp://ftp.groessler.org/pub/chris/olivetti_m20/misc/bios/rom.s

---

APB notes:

0xfc903 checks for the string TEST at 0x3f4-0x3f6, does an int 0xfe if so, unknown purpose

Error codes:
Triangle    Test CPU registers and instructions
Square      Test ROM
4 vertical lines    CPU call or trap instructions failed
Diamond     Test system RAM
E C0     8255 parallel interface IC test failed
E C1     6845 CRT controller IC test failed
E C2     1797 floppy disk controller chip failed
E C3     8253 timer IC failed
E C4     8251 keyboard interface failed
E C5     8251 keyboard test failed
E C6     8259 PIC IC test failed
E K0     Keyboard did not respond
E K1     Keyboard responds, but self test failed
E D1     Disk drive 1 test failed
E D0     Disk drive 0 test failed
E I0     Non-vectored interrupt error
E I1     Vectored interrupt error

*************************************************************************************************/


#include "emu.h"
#include "m20_8086.h"
#include "m20_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/z8000/z8000.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#include "formats/m20_dsk.h"


namespace {

class m20_state : public driver_device
{
public:
	m20_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_kbdi8251(*this, "i8251_1"),
		m_ttyi8251(*this, "i8251_2"),
		m_i8255(*this, "ppi8255"),
		m_i8259(*this, "i8259"),
		m_fd1797(*this, "fd1797"),
		m_floppy0(*this, "fd1797:0:5dd"),
		m_floppy1(*this, "fd1797:1:5dd"),
		m_apb(*this, "apb"),
		m_palette(*this, "palette")
	{
	}

	void m20(machine_config &config);

private:
	required_device<z8001_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<i8251_device> m_kbdi8251;
	required_device<i8251_device> m_ttyi8251;
	required_device<i8255_device> m_i8255;
	required_device<pic8259_device> m_i8259;
	required_device<fd1797_device> m_fd1797;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	optional_device<m20_8086_device> m_apb;

	required_device<palette_device> m_palette;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t i8259_r(offs_t offset);
	void i8259_w(offs_t offset, uint16_t data);
	uint16_t port21_r();
	void port21_w(uint16_t data);
	void tty_clock_tick_w(int state);
	void kbd_clock_tick_w(int state);
	void timer_tick_w(int state);
	void int_w(int state);
	MC6845_UPDATE_ROW(update_row);

	void m20_data_mem(address_map &map) ATTR_COLD;
	void m20_io(address_map &map) ATTR_COLD;
	void m20_program_mem(address_map &map) ATTR_COLD;

	offs_t m_memsize = 0;
	uint8_t m_port21 = 0;
	void install_memory();

	static void floppy_formats(format_registration &fr);
	uint16_t viack_r();
	uint16_t nviack_r();
};


#define MAIN_CLOCK 4000000 /* 4 MHz */
#define PIXEL_CLOCK 4.433619_MHz_XTAL


MC6845_UPDATE_ROW( m20_state::update_row )
{
	uint32_t *p = &bitmap.pix(y);
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint16_t *vram = (uint16_t *)m_ram->pointer();
	uint16_t offset = ((ma | (ra << 1)) << 4);

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t data = vram[ offset + i ];

		for ( int j = 15; j >= 0; j-- )
		{
			*p = palette[( data & 1 << j ) ? 1 : 0];
			p++;
		}
	}
}

/*
port21      =   0x21        !TTL latch
!   (see hw1 document, fig 2-33, pg 2-47)
!       Output                      Input
!       ======                      =====
!   B0  0 selects floppy 0
!       1 deselects floppy 0
!   B1  0 selects floppy 1
!       1 deselects floppy 1
!   B2  Motor On (Not Used)
!   B3  0 selects double density    0 => Skip basic tests
!       1 selects single density    1 => Perform basic tests
!                                   Latched copy when B7 is written to Port21
!   B4  Uncommitted output          0 => 128K card(s) present
!                                   1 => 32K card(s) present
!                                   Cannot mix types!
!   B5  Uncommitted output          0 => 8-colour card present
!                                   1 => 4-colour card present
!   B6  Uncommitted output          0 => RAM
!                                   1 => ROM (???)
!   B7  See B3 input                0 => colour card present
*/

uint16_t m20_state::port21_r()
{
	//printf("port21 read\n");
	return m_port21;
}

void m20_state::port21_w(uint16_t data)
{
	//printf("port21 write: data 0x%x\n", data);
	m_port21 = (m_port21 & 0xf8) | (data & 0x7);

	// floppy drive select
	if (data & 1) {
		m_floppy0->mon_w(0);
		m_fd1797->set_floppy(m_floppy0);
	}
	else
		m_floppy0->mon_w(1);

	if (data & 2) {
		m_floppy1->mon_w(0);
		m_fd1797->set_floppy(m_floppy1);
	}
	else
		m_floppy1->mon_w(1);

	if(!(data & 3))
		m_fd1797->set_floppy(nullptr);

	// density select 1 - sd, 0 - dd
	m_fd1797->dden_w(data & 8);
}

uint16_t m20_state::i8259_r(offs_t offset)
{
	return m_i8259->read(offset)<<1;
}

void m20_state::i8259_w(offs_t offset, uint16_t data)
{
	m_i8259->write(offset, (data>>1));
}

void m20_state::tty_clock_tick_w(int state)
{
	m_ttyi8251->write_txc(state);
	m_ttyi8251->write_rxc(state);
}

void m20_state::kbd_clock_tick_w(int state)
{
	m_kbdi8251->write_txc(state);
	m_kbdi8251->write_rxc(state);
}

void m20_state::timer_tick_w(int state)
{
	/* The output of the 8253 is connected to a 74LS74 flop chip.
	 * The output of the flop chip is connected to NVI CPU input.
	 * The flop is reset by a 1:8 decoder which compares CPU ST0-ST3
	 * outputs to detect an interrupt acknowledge transaction.
	 * 8253 is programmed in square wave mode, not rate
	 * generator mode.
	 */
	if(m_apb)
		m_apb->nvi_w(state);
	if(state)
		m_maincpu->set_input_line(z8001_device::NVI_LINE, ASSERT_LINE);
}


/* Memory map description (by courtesy of Dwight Elvey)

    DRAM0 = motherboard (128K)
    DRAM1 = first slot from keyboard end
    DRAM2 = second slot from keyboard end
    DRAM3 = third slot from keyboard end
    SRAM0 = memory window for expansion slot
    ROM0  = ROM

Expansion cards are either 32K or 128K. They cannot be mixed, all installed
cards need to be the same type.

B/W, 32K cards, 3 cards => 224K of memory:
<0>0000 D DRAM0  4000 I DRAM0  4000  <8>0000 D DRAM0 18000 I DRAM0  8000
<0>4000 D DRAM1  4000 I DRAM0  8000  <8>4000 D DRAM0 1C000 I DRAM0  C000
<0>8000 D DRAM2  0000 I DRAM0  C000  <8>8000 D DRAM2  4000 I DRAM1  4000
<0>C000 D DRAM2  4000 I DRAM0 10000  <8>C000 D DRAM3  0000 I DRAM2  0000
<1>0000 D DRAM0 14000 I DRAM0  8000  <9>0000 D DRAM0 18000 I DRAM0 18000
<1>4000 D DRAM0 18000 I DRAM0  C000  <9>4000 D DRAM0 1C000 I DRAM0 1C000
<1>8000 D DRAM0 1C000 I DRAM0 10000  <9>8000 D DRAM2  4000 I DRAM2  4000
<1>C000 D DRAM1  0000 I None         <9>C000 D DRAM3  0000 I DRAM3  0000
<2>0000 D DRAM0 14000 I DRAM0 14000  <A>0000 D DRAM0  8000 I DRAM0  8000
<2>4000 D DRAM0 18000 I DRAM0 18000  <A>4000 D DRAM0  C000 I DRAM0  C000
<2>8000 D DRAM0 1C000 I DRAM0 1C000  <A>8000 D DRAM1  4000 I DRAM1  4000
<2>C000 D DRAM1  0000 I DRAM1  0000  <A>C000 D DRAM2  0000 I DRAM2  0000
<3>0000 D DRAM0  0000 I DRAM0  0000  <B>0000 D DRAM3  4000 I DRAM3  4000
<3>4000 D None        I None         <B>4000 D None        I None
<3>8000 D None        I None         <B>8000 D None        I None
<3>C000 D None        I None         <B>C000 D None        I None
<4>0000 D  ROM0  0000 I  ROM0  0000  <C>0000 D DRAM3  4000 I None
<4>4000 D DRAM3  0000 I  ROM0 10000  <C>4000 D None        I None
<4>8000 D DRAM3  4000 I  ROM0 14000  <C>8000 D None        I None
<4>C000 D None        I  ROM0 18000  <C>C000 D None        I None
<5>0000 D DRAM0  8000 I  ROM0 10000  <D>0000 D None        I None
<5>4000 D DRAM0  C000 I  ROM0 14000  <D>4000 D None        I None
<5>8000 D DRAM0 10000 I  ROM0 18000  <D>8000 D None        I None
<5>C000 D SRAM0  0000 I SRAM0  0000  <D>C000 D None        I None
<6>0000 D DRAM0  8000 I DRAM0  8000  <E>0000 D None        I None
<6>4000 D DRAM0  C000 I DRAM0  C000  <E>4000 D None        I None
<6>8000 D DRAM0 10000 I DRAM0 10000  <E>8000 D None        I None
<6>C000 D None        I None         <E>C000 D None        I None
<7>0000 D  ROM0  0000 I  ROM0  0000  <F>0000 D None        I None
<7>4000 D  ROM0 10000 I  ROM0 10000  <F>4000 D None        I None
<7>8000 D  ROM0 14000 I  ROM0 14000  <F>8000 D None        I None
<7>C000 D  ROM0 18000 I  ROM0 18000  <F>C000 D None        I None

B/W, 128K cards, 3 cards => 512K of memory:
<0>0000 D DRAM0  4000 I DRAM0  4000  <8>0000 D DRAM0 18000 I DRAM0  8000
<0>4000 D DRAM1  4000 I DRAM0  8000  <8>4000 D DRAM0 1C000 I DRAM0  C000
<0>8000 D DRAM2  0000 I DRAM0  C000  <8>8000 D DRAM1  C000 I DRAM1  4000
<0>C000 D DRAM2  4000 I DRAM0 10000  <8>C000 D DRAM1 10000 I DRAM1  8000
<1>0000 D DRAM0 14000 I DRAM0  8000  <9>0000 D DRAM0 18000 I DRAM0 18000
<1>4000 D DRAM0 18000 I DRAM0  C000  <9>4000 D DRAM0 1C000 I DRAM0 1C000
<1>8000 D DRAM0 1C000 I DRAM0 10000  <9>8000 D DRAM1  C000 I DRAM1  C000
<1>C000 D DRAM1  0000 I None         <9>C000 D DRAM1 10000 I DRAM1 10000
<2>0000 D DRAM0 14000 I DRAM0 14000  <A>0000 D DRAM0  8000 I DRAM0  8000
<2>4000 D DRAM0 18000 I DRAM0 18000  <A>4000 D DRAM0  C000 I DRAM0  C000
<2>8000 D DRAM0 1C000 I DRAM0 1C000  <A>8000 D DRAM1  4000 I DRAM1  4000
<2>C000 D DRAM1  0000 I DRAM1  0000  <A>C000 D DRAM1  8000 I DRAM1  8000
<3>0000 D DRAM0  0000 I DRAM0  0000  <B>0000 D DRAM1 14000 I DRAM1 14000
<3>4000 D None        I None         <B>4000 D DRAM1 18000 I DRAM1 18000
<3>8000 D None        I None         <B>8000 D DRAM1 1C000 I DRAM1 1C000
<3>C000 D None        I None         <B>C000 D DRAM2  0000 I DRAM2  0000
<4>0000 D  ROM0  0000 I  ROM0  0000  <C>0000 D DRAM2  4000 I DRAM2  4000
<4>4000 D DRAM3  0000 I None         <C>4000 D DRAM2  8000 I DRAM2  8000
<4>8000 D DRAM3  4000 I None         <C>8000 D DRAM2  C000 I DRAM2  C000
<4>C000 D None        I None         <C>C000 D DRAM2 10000 I DRAM2 10000
<5>0000 D DRAM0  8000 I  ROM0 10000  <D>0000 D DRAM2 14000 I DRAM2 14000
<5>4000 D DRAM0  C000 I  ROM0 14000  <D>4000 D DRAM2 18000 I DRAM2 18000
<5>8000 D DRAM0 10000 I  ROM0 18000  <D>8000 D DRAM2 1C000 I DRAM2 1C000
<5>C000 D SRAM0  0000 I SRAM0  0000  <D>C000 D DRAM3  0000 I DRAM3  0000
<6>0000 D DRAM0  8000 I DRAM0  8000  <E>0000 D DRAM3  4000 I DRAM3  4000
<6>4000 D DRAM0  C000 I DRAM0  C000  <E>4000 D DRAM3  8000 I DRAM3  8000
<6>8000 D DRAM0 10000 I DRAM0 10000  <E>8000 D DRAM3  C000 I DRAM3  C000
<6>C000 D None        I None         <E>C000 D DRAM3 10000 I DRAM3 10000
<7>0000 D  ROM0  0000 I  ROM0  0000  <F>0000 D DRAM3 14000 I DRAM3 14000
<7>4000 D  ROM0 10000 I  ROM0 10000  <F>4000 D DRAM3 18000 I DRAM3 18000
<7>8000 D  ROM0 14000 I  ROM0 14000  <F>8000 D DRAM3 1C000 I DRAM3 1C000
<7>C000 D  ROM0 18000 I  ROM0 18000  <F>C000 D DRAM3  0000 I DRAM3  0000
*/


void m20_state::m20_program_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x40000, 0x41fff).rom().region("maincpu", 0x00000);
}

void m20_state::m20_data_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x40000, 0x41fff).rom().region("maincpu", 0x00000);
}


void m20_state::install_memory()
{
	m_memsize = m_ram->size();
	uint8_t *memptr = m_ram->pointer();
	address_space& pspace = m_maincpu->space(AS_PROGRAM);
	address_space& dspace = m_maincpu->space(AS_DATA);

	/* install mainboard memory (aka DRAM0) */

	/* <0>0000 */
	pspace.install_ram(0x0000, 0x3fff, 0, memptr + 0x4000);
	dspace.install_ram(0x0000, 0x3fff, 0, memptr + 0x4000);
	/* <0>4000 */
	pspace.install_ram(0x4000, 0x7fff, 0, memptr + 0x8000);
	/* <0>8000 */
	pspace.install_ram(0x8000, 0xbfff, 0, memptr + 0xc000);
	/* <0>C000 */
	pspace.install_ram(0xc000, 0xcfff, 0, memptr + 0x10000);
	/* <1>0000 */
	pspace.install_ram(0x10000, 0x13fff, 0, memptr + 0x8000);
	dspace.install_ram(0x10000, 0x13fff, 0, memptr + 0x14000);
	/* <1>4000 */
	pspace.install_ram(0x14000, 0x17fff, 0, memptr + 0xc000);
	dspace.install_ram(0x14000, 0x17fff, 0, memptr + 0x18000);
	/* <1>8000 */
	pspace.install_ram(0x18000, 0x1bfff, 0, memptr + 0x10000);
	dspace.install_ram(0x18000, 0x1bfff, 0, memptr + 0x1c000);
	/* <1>c000 empty*/
	/* <2>0000 */
	pspace.install_ram(0x20000, 0x23fff, 0, memptr + 0x14000);
	dspace.install_ram(0x20000, 0x23fff, 0, memptr + 0x14000);
	/* <2>4000 */
	pspace.install_ram(0x24000, 0x27fff, 0, memptr + 0x18000);
	dspace.install_ram(0x24000, 0x27fff, 0, memptr + 0x18000);
	/* <2>8000 */
	pspace.install_ram(0x28000, 0x2bfff, 0, memptr + 0x1c000);
	dspace.install_ram(0x28000, 0x2bfff, 0, memptr + 0x1c000);
	/* <2>c000 empty*/
	/* <3>0000 (video buffer) */
	pspace.install_ram(0x30000, 0x33fff, 0, memptr + 0x0000);
	dspace.install_ram(0x30000, 0x33fff, 0, memptr + 0x0000);


	/* <5>0000 */
	dspace.install_ram(0x50000, 0x53fff, 0, memptr + 0x8000);
	/* <5>4000 */
	dspace.install_ram(0x54000, 0x57fff, 0, memptr + 0xc000);
	/* <5>8000 */
	dspace.install_ram(0x58000, 0x5bfff, 0, memptr + 0x10000);
	/* <5>c000 expansion bus */
	/* <6>0000 */
	pspace.install_ram(0x60000, 0x63fff, 0, memptr + 0x8000);
	dspace.install_ram(0x60000, 0x63fff, 0, memptr + 0x8000);
	/* <6>4000 */
	pspace.install_ram(0x64000, 0x67fff, 0, memptr + 0xc000);
	dspace.install_ram(0x64000, 0x67fff, 0, memptr + 0xc000);
	/* <6>8000 */
	pspace.install_ram(0x68000, 0x6bfff, 0, memptr + 0x10000);
	dspace.install_ram(0x68000, 0x6bfff, 0, memptr + 0x10000);
	/* <6>c000 empty*/
	/* segment <7> expansion ROM? */
	/* <8>0000 */
	pspace.install_ram(0x80000, 0x83fff, 0, memptr + 0x8000);
	dspace.install_ram(0x80000, 0x83fff, 0, memptr + 0x18000);
	/* <8>4000 */
	pspace.install_ram(0x84000, 0x87fff, 0, memptr + 0xc000);
	dspace.install_ram(0x84000, 0x87fff, 0, memptr + 0x1c000);
	/* <9>0000 */
	pspace.install_ram(0x90000, 0x93fff, 0, memptr + 0x18000);
	dspace.install_ram(0x90000, 0x93fff, 0, memptr + 0x18000);
	/* <9>4000 */
	pspace.install_ram(0x94000, 0x97fff, 0, memptr + 0x1c000);
	dspace.install_ram(0x94000, 0x97fff, 0, memptr + 0x1c000);
	/* <A>0000 */
	pspace.install_ram(0xa0000, 0xa3fff, 0, memptr + 0x8000);
	dspace.install_ram(0xa0000, 0xa3fff, 0, memptr + 0x8000);
	/* <A>4000 */
	pspace.install_ram(0xa4000, 0xa7fff, 0, memptr + 0xc000);
	dspace.install_ram(0xa4000, 0xa7fff, 0, memptr + 0xc000);

	if (m_memsize > 128 * 1024) {
		/* install memory expansions (DRAM1..DRAM3) */

		if (m_memsize < 256 * 1024) {
			/* 32K expansion cards */

			/* DRAM1, 32K */

			/* prog
			   map( 0x2c000, 0x2ffff ).ram().share("dram1_0000");
			   map( 0x88000, 0x8bfff ).ram().share("dram1_4000");
			   map( 0xa8000, 0xabfff ).ram().share("dram1_4000");
			*/
			pspace.install_ram(0x2c000, 0x2ffff, 0, memptr + 0x20000);
			pspace.install_ram(0x88000, 0x8bfff, 0, memptr + 0x24000);
			pspace.install_ram(0xa8000, 0xabfff, 0, memptr + 0x24000);

			/*
			  data
			  map( 0x04000, 0x07fff ).ram().share("dram1_4000");
			  map( 0x1c000, 0x1ffff ).ram().share("dram1_0000");
			  map( 0x2c000, 0x2ffff ).ram().share("dram1_0000");
			  map( 0xa8000, 0xabfff ).ram().share("dram1_4000");
			*/
			dspace.install_ram(0x4000, 0x7fff, 0, memptr + 0x24000);
			dspace.install_ram(0x1c000, 0x1ffff, 0, memptr + 0x20000);
			dspace.install_ram(0x2c000, 0x2ffff, 0, memptr + 0x20000);
			dspace.install_ram(0xa8000, 0xabfff, 0, memptr + 0x24000);

			if (m_memsize > 128 * 1024 + 32768) {
				/* DRAM2, 32K */

				/* prog
				   map( 0x8c000, 0x8ffff ).ram().share("dram2_0000");
				   map( 0x98000, 0x9bfff ).ram().share("dram2_4000");
				   map( 0xac000, 0xaffff ).ram().share("dram2_0000");
				*/
				pspace.install_ram(0x8c000, 0x8ffff, 0, memptr + 0x28000);
				pspace.install_ram(0x98000, 0x9bfff, 0, memptr + 0x2c000);
				pspace.install_ram(0xac000, 0xaffff, 0, memptr + 0x28000);

				/* data
				   map( 0x08000, 0x0bfff ).ram().share("dram2_0000");
				   map( 0x0c000, 0x0ffff ).ram().share("dram2_4000");
				   map( 0x88000, 0x8bfff ).ram().share("dram2_4000");
				   map( 0x98000, 0x9bfff ).ram().share("dram2_4000");
				   map( 0xac000, 0xaffff ).ram().share("dram2_0000");
				 */
				dspace.install_ram(0x8000, 0xbfff, 0, memptr + 0x28000);
				dspace.install_ram(0xc000, 0xffff, 0, memptr + 0x2c000);
				dspace.install_ram(0x88000, 0x8bfff, 0, memptr + 0x2c000);
				dspace.install_ram(0x98000, 0x9bfff, 0, memptr + 0x2c000);
				dspace.install_ram(0xac000, 0xaffff, 0, memptr + 0x28000);
			}
			if (m_memsize > 128 * 1024 + 2 * 32768) {
				/* DRAM3, 32K */

				/* prog
				   map( 0x9c000, 0x9ffff ).ram().share("dram3_0000");
				   map( 0xb0000, 0xb3fff ).ram().share("dram3_4000");
				*/
				pspace.install_ram(0x9c000, 0x9ffff, 0, memptr + 0x30000);
				pspace.install_ram(0xb0000, 0xb3fff, 0, memptr + 0x34000);

				/* data
				   map( 0x44000, 0x47fff ).ram().share("dram3_0000");
				   map( 0x48000, 0x4bfff ).ram().share("dram3_4000");
				   map( 0x8c000, 0x8ffff ).ram().share("dram3_0000");
				   map( 0x9c000, 0x9ffff ).ram().share("dram3_0000");
				   map( 0xb0000, 0xb3fff ).ram().share("dram3_4000");
				   map( 0xc0000, 0xc3fff ).ram().share("dram3_4000");
				 */
				dspace.install_ram(0x44000, 0x47fff, 0, memptr + 0x30000);
				dspace.install_ram(0x48000, 0x4bfff, 0, memptr + 0x34000);
				dspace.install_ram(0x8c000, 0x8ffff, 0, memptr + 0x30000);
				dspace.install_ram(0x9c000, 0x9ffff, 0, memptr + 0x30000);
				dspace.install_ram(0xb0000, 0xb3fff, 0, memptr + 0x34000);
				dspace.install_ram(0xc0000, 0xc3fff, 0, memptr + 0x34000);
			}
		}
		else {
			/* 128K expansion cards */

			/* DRAM1, 128K */

			/* prog
			   map( 0x2c000, 0x2ffff ).ram().share("dram1_0000");
			   map( 0x88000, 0x8bfff ).ram().share("dram1_4000");
			   map( 0x8c000, 0x8ffff ).ram().share("dram1_8000");
			   map( 0x98000, 0x9bfff ).ram().share("dram1_c000");
			   map( 0x9c000, 0x9ffff ).ram().share("dram1_10000");
			   map( 0xa8000, 0xabfff ).ram().share("dram1_4000");
			   map( 0xac000, 0xaffff ).ram().share("dram1_8000");
			   map( 0xb0000, 0xb3fff ).ram().share("dram1_14000");
			   map( 0xb4000, 0xb7fff ).ram().share("dram1_18000");
			   map( 0xb8000, 0xbbfff ).ram().share("dram1_1c000");
			*/
			pspace.install_ram(0x2c000, 0x2ffff, 0, memptr + 0x20000);
			pspace.install_ram(0x88000, 0x8bfff, 0, memptr + 0x24000);
			pspace.install_ram(0x8c000, 0x8ffff, 0, memptr + 0x28000);
			pspace.install_ram(0x98000, 0x9bfff, 0, memptr + 0x2c000);
			pspace.install_ram(0x9c000, 0x9ffff, 0, memptr + 0x30000);
			pspace.install_ram(0xa8000, 0xabfff, 0, memptr + 0x24000);
			pspace.install_ram(0xac000, 0xaffff, 0, memptr + 0x28000);
			pspace.install_ram(0xb0000, 0xb3fff, 0, memptr + 0x34000);
			pspace.install_ram(0xb4000, 0xb7fff, 0, memptr + 0x38000);
			pspace.install_ram(0xb8000, 0xbbfff, 0, memptr + 0x3c000);

			/* data
			   map( 0x04000, 0x07fff ).ram().share("dram1_4000");
			   map( 0x1c000, 0x1ffff ).ram().share("dram1_0000");
			   map( 0x2c000, 0x2ffff ).ram().share("dram1_0000");
			   map( 0x88000, 0x8bfff ).ram().share("dram1_c000");
			   map( 0x8c000, 0x8ffff ).ram().share("dram1_10000");
			   map( 0x98000, 0x9bfff ).ram().share("dram1_c000");
			   map( 0x9c000, 0x9ffff ).ram().share("dram1_10000");
			   map( 0xa8000, 0xabfff ).ram().share("dram1_4000");
			   map( 0xac000, 0xaffff ).ram().share("dram1_8000");
			   map( 0xb0000, 0xb3fff ).ram().share("dram1_14000");
			   map( 0xb4000, 0xb7fff ).ram().share("dram1_18000");
			   map( 0xb8000, 0xbbfff ).ram().share("dram1_1c000");
			 */
			dspace.install_ram(0x4000, 0x7fff, 0, memptr + 0x24000);
			dspace.install_ram(0x1c000, 0x1ffff, 0, memptr + 0x20000);
			dspace.install_ram(0x2c000, 0x2ffff, 0, memptr + 0x20000);
			dspace.install_ram(0x88000, 0x8bfff, 0, memptr + 0x2c000);
			dspace.install_ram(0x8c000, 0x8ffff, 0, memptr + 0x30000);
			dspace.install_ram(0x98000, 0x9bfff, 0, memptr + 0x2c000);
			dspace.install_ram(0x9c000, 0x9ffff, 0, memptr + 0x30000);
			dspace.install_ram(0xa8000, 0xabfff, 0, memptr + 0x24000);
			dspace.install_ram(0xac000, 0xaffff, 0, memptr + 0x28000);
			dspace.install_ram(0xb0000, 0xb3fff, 0, memptr + 0x34000);
			dspace.install_ram(0xb4000, 0xb7fff, 0, memptr + 0x38000);
			dspace.install_ram(0xb8000, 0xbbfff, 0, memptr + 0x3c000);

			if (m_memsize > 256 * 1024) {
				/* DRAM2, 128K */

				/* prog
				   map( 0xbc000, 0xbffff ).ram().share("dram2_0000");

				   map( 0xc0000, 0xc3fff ).ram().share("dram2_4000");
				   map( 0xc4000, 0xc7fff ).ram().share("dram2_8000");
				   map( 0xc8000, 0xcbfff ).ram().share("dram2_c000");
				   map( 0xcc000, 0xcffff ).ram().share("dram2_10000");

				   map( 0xd0000, 0xd3fff ).ram().share("dram2_14000");
				   map( 0xd4000, 0xd7fff ).ram().share("dram2_18000");
				   map( 0xd8000, 0xdbfff ).ram().share("dram2_1c000");
				 */
				pspace.install_ram(0xbc000, 0xbffff, 0, memptr + 0x40000);
				pspace.install_ram(0xc0000, 0xc3fff, 0, memptr + 0x44000);
				pspace.install_ram(0xc4000, 0xc7fff, 0, memptr + 0x48000);
				pspace.install_ram(0xc8000, 0xcbfff, 0, memptr + 0x4c000);
				pspace.install_ram(0xcc000, 0xcffff, 0, memptr + 0x50000);
				pspace.install_ram(0xd0000, 0xd3fff, 0, memptr + 0x54000);
				pspace.install_ram(0xd4000, 0xd7fff, 0, memptr + 0x58000);
				pspace.install_ram(0xd8000, 0xdbfff, 0, memptr + 0x5c000);

				/* data
				   map( 0x08000, 0x0bfff ).ram().share("dram2_0000");
				   map( 0x0c000, 0x0ffff ).ram().share("dram2_4000");

				   map( 0xbc000, 0xbffff ).ram().share("dram2_0000");

				   map( 0xc0000, 0xc3fff ).ram().share("dram2_4000");
				   map( 0xc4000, 0xc7fff ).ram().share("dram2_8000");
				   map( 0xc8000, 0xcbfff ).ram().share("dram2_c000");
				   map( 0xcc000, 0xcffff ).ram().share("dram2_10000");

				   map( 0xd0000, 0xd3fff ).ram().share("dram2_14000");
				   map( 0xd4000, 0xd7fff ).ram().share("dram2_18000");
				   map( 0xd8000, 0xdbfff ).ram().share("dram2_1c000");
				*/
				dspace.install_ram(0x8000, 0xbfff, 0, memptr + 0x40000);
				dspace.install_ram(0xc000, 0xffff, 0, memptr + 0x44000);
				dspace.install_ram(0xbc000, 0xbffff, 0, memptr + 0x40000);
				dspace.install_ram(0xc0000, 0xc3fff, 0, memptr + 0x44000);
				dspace.install_ram(0xc4000, 0xc7fff, 0, memptr + 0x48000);
				dspace.install_ram(0xc8000, 0xcbfff, 0, memptr + 0x4c000);
				dspace.install_ram(0xcc000, 0xcffff, 0, memptr + 0x50000);
				dspace.install_ram(0xd0000, 0xd3fff, 0, memptr + 0x54000);
				dspace.install_ram(0xd4000, 0xd7fff, 0, memptr + 0x58000);
				dspace.install_ram(0xd8000, 0xdbfff, 0, memptr + 0x5c000);
			}
			if (m_memsize > 384 * 1024) {
				/* DRAM3, 128K */

				/* prog
				   map( 0xdc000, 0xdffff ).ram().share("dram3_0000");

				   map( 0xe0000, 0xe3fff ).ram().share("dram3_4000");
				   map( 0xe4000, 0xe7fff ).ram().share("dram3_8000");
				   map( 0xe8000, 0xebfff ).ram().share("dram3_c000");
				   map( 0xec000, 0xeffff ).ram().share("dram3_10000");

				   map( 0xf0000, 0xf3fff ).ram().share("dram3_14000");
				   map( 0xf4000, 0xf7fff ).ram().share("dram3_18000");
				   map( 0xf8000, 0xfbfff ).ram().share("dram3_1c000");
				   map( 0xfc000, 0xfffff ).ram().share("dram3_0000");
				*/
				pspace.install_ram(0xdc000, 0xdffff, 0, memptr + 0x60000);
				pspace.install_ram(0xe0000, 0xe3fff, 0, memptr + 0x64000);
				pspace.install_ram(0xe4000, 0xe7fff, 0, memptr + 0x68000);
				pspace.install_ram(0xe8000, 0xebfff, 0, memptr + 0x6c000);
				pspace.install_ram(0xec000, 0xeffff, 0, memptr + 0x70000);
				pspace.install_ram(0xf0000, 0xf3fff, 0, memptr + 0x74000);
				pspace.install_ram(0xf4000, 0xf7fff, 0, memptr + 0x78000);
				pspace.install_ram(0xf8000, 0xfbfff, 0, memptr + 0x7c000);
				pspace.install_ram(0xfc000, 0xfffff, 0, memptr + 0x60000);

				/* data
				   map( 0x44000, 0x47fff ).ram().share("dram3_0000");
				   map( 0x48000, 0x4bfff ).ram().share("dram3_4000");
				   map( 0xdc000, 0xdffff ).ram().share("dram3_0000");

				   map( 0xe0000, 0xe3fff ).ram().share("dram3_4000");
				   map( 0xe4000, 0xe7fff ).ram().share("dram3_8000");
				   map( 0xe8000, 0xebfff ).ram().share("dram3_c000");
				   map( 0xec000, 0xeffff ).ram().share("dram3_10000");

				   map( 0xf0000, 0xf3fff ).ram().share("dram3_14000");
				   map( 0xf4000, 0xf7fff ).ram().share("dram3_18000");
				   map( 0xf8000, 0xfbfff ).ram().share("dram3_1c000");
				   map( 0xfc000, 0xfffff ).ram().share("dram3_0000");
				*/
				dspace.install_ram(0x44000, 0x47fff, 0, memptr + 0x60000);
				dspace.install_ram(0x48000, 0x4bfff, 0, memptr + 0x64000);
				dspace.install_ram(0xdc000, 0xdffff, 0, memptr + 0x60000);
				dspace.install_ram(0xe0000, 0xe3fff, 0, memptr + 0x64000);
				dspace.install_ram(0xe4000, 0xe7fff, 0, memptr + 0x68000);
				dspace.install_ram(0xe8000, 0xebfff, 0, memptr + 0x6c000);
				dspace.install_ram(0xec000, 0xeffff, 0, memptr + 0x70000);
				dspace.install_ram(0xf0000, 0xf3fff, 0, memptr + 0x74000);
				dspace.install_ram(0xf4000, 0xf7fff, 0, memptr + 0x78000);
				dspace.install_ram(0xf8000, 0xfbfff, 0, memptr + 0x7c000);
				dspace.install_ram(0xfc000, 0xfffff, 0, memptr + 0x60000);
			}
		}
	}
}

void m20_state::m20_io(address_map &map)
{
	map.unmap_value_high();

	map(0x00, 0x07).rw(m_fd1797, FUNC(fd1797_device::read), FUNC(fd1797_device::write)).umask16(0x00ff);

	map(0x20, 0x21).rw(FUNC(m20_state::port21_r), FUNC(m20_state::port21_w));

	map(0x61, 0x61).w("crtc", FUNC(mc6845_device::address_w));
	map(0x62, 0x62).w("crtc", FUNC(mc6845_device::address_w)); // FIXME
	map(0x63, 0x63).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x64, 0x64).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x80, 0x87).rw(m_i8255, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);

	map(0xa0, 0xa3).rw(m_kbdi8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);

	map(0xc0, 0xc3).rw(m_ttyi8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);

	map(0x120, 0x127).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);

	map(0x140, 0x143).rw(FUNC(m20_state::i8259_r), FUNC(m20_state::i8259_w));

	map(0x3ffa, 0x3ffd).w(m_apb, FUNC(m20_8086_device::handshake_w));
}

uint16_t m20_state::viack_r()
{
	return m_i8259->acknowledge()<<1;
}

uint16_t m20_state::nviack_r()
{
	m_maincpu->set_input_line(z8001_device::NVI_LINE, CLEAR_LINE);
	return 0xffff;
}

void m20_state::int_w(int state)
{
	if(m_apb && !m_apb->halted())
		m_apb->vi_w(state);
	m_maincpu->set_input_line(z8001_device::VI_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void m20_state::machine_start()
{
	install_memory();
}

void m20_state::machine_reset()
{
	uint8_t *ROM = memregion("maincpu")->base();
	uint8_t *RAM = (uint8_t *)(m_ram->pointer() + 0x4000);

	if (m_memsize >= 256 * 1024)
		m_port21 = 0xdf;
	else
		m_port21 = 0xff;

	if(system_bios() > 0)  // bits have different meanings?
		m_port21 &= ~8;

	m_fd1797->reset();

	memcpy(RAM, ROM, 8);  // we need only the reset vector
	m_kbdi8251->write_cts(0);
	if (m_apb)
		m_apb->halt();
}


static void m20_floppies(device_slot_interface &device)
{
	device.option_add("5dd", FLOPPY_525_DD);
}

void m20_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_M20_FORMAT);
}

static void keyboard(device_slot_interface &device)
{
	device.option_add("m20", M20_KEYBOARD);
}

void m20_state::m20(machine_config &config)
{
	/* basic machine hardware */
	Z8001(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m20_state::m20_program_mem);
	m_maincpu->set_addrmap(AS_DATA, &m20_state::m20_data_mem);
	m_maincpu->set_addrmap(AS_IO, &m20_state::m20_io);
	m_maincpu->viack().set(FUNC(m20_state::viack_r));
	m_maincpu->nviack().set(FUNC(m20_state::nviack_r));

	RAM(config, RAM_TAG).set_default_size("160K").set_default_value(0).set_extra_options("128K,192K,224K,256K,384K,512K");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Devices */
	FD1797(config, m_fd1797, 1000000);
	m_fd1797->intrq_wr_callback().set(m_i8259, FUNC(pic8259_device::ir0_w));
	FLOPPY_CONNECTOR(config, "fd1797:0", m20_floppies, "5dd", m20_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1797:1", m20_floppies, "5dd", m20_state::floppy_formats);

	mc6845_device &crtc(MC6845(config, "crtc", PIXEL_CLOCK/8)); /* hand tuned to get ~50 fps */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(16);
	crtc.set_update_row_callback(FUNC(m20_state::update_row));

	I8255A(config, m_i8255, 0);

	I8251(config, m_kbdi8251, 0);
	m_kbdi8251->txd_handler().set("kbd", FUNC(rs232_port_device::write_txd));
	m_kbdi8251->rxrdy_handler().set(m_i8259, FUNC(pic8259_device::ir4_w));

	I8251(config, m_ttyi8251, 0);
	m_ttyi8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_ttyi8251->rxrdy_handler().set(m_i8259, FUNC(pic8259_device::ir3_w));
	m_ttyi8251->txrdy_handler().set(m_i8259, FUNC(pic8259_device::ir5_w));

	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(1230782);
	pit8253.out_handler<0>().set(FUNC(m20_state::tty_clock_tick_w));
	pit8253.set_clk<1>(1230782);
	pit8253.out_handler<1>().set(FUNC(m20_state::kbd_clock_tick_w));
	pit8253.set_clk<2>(1230782);
	pit8253.out_handler<2>().set(FUNC(m20_state::timer_tick_w));

	PIC8259(config, m_i8259, 0);
	m_i8259->out_int_callback().set(FUNC(m20_state::int_w));

	rs232_port_device &kbd(RS232_PORT(config, "kbd", keyboard, "m20"));
	kbd.rxd_handler().set(m_kbdi8251, FUNC(i8251_device::write_rxd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_ttyi8251, FUNC(i8251_device::write_rxd));

	M20_8086(config, m_apb, m_maincpu, m_i8259, RAM_TAG);

	SOFTWARE_LIST(config, "flop_list").set_original("m20");
}

ROM_START(m20)
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "m20", "M20 1.0" )
	ROMX_LOAD("m20.bin", 0x0000, 0x2000, CRC(5c93d931) SHA1(d51025e087a94c55529d7ee8fd18ff4c46d93230), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "m20-20d", "M20 2.0d" )
	ROMX_LOAD("m20-20d.bin", 0x0000, 0x2000, CRC(cbe265a6) SHA1(c7cb9d9900b7b5014fcf1ceb2e45a66a91c564d0), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "m20-20f", "M20 2.0f" )
	ROMX_LOAD("m20-20f.bin", 0x0000, 0x2000, CRC(db7198d8) SHA1(149d8513867081d31c73c2965dabb36d5f308041), ROM_BIOS(2))
ROM_END

ROM_START(m40)
	ROM_REGION(0x14000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "m40-81", "M40 15.dec.81" )
	ROMX_LOAD( "m40rom-15-dec-81", 0x0000, 0x2000, CRC(e8e7df84) SHA1(e86018043bf5a23ff63434f9beef7ce2972d8153), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "m40-82", "M40 17.dec.82" )
	ROMX_LOAD( "m40rom-17-dec-82", 0x0000, 0x2000, CRC(cf55681c) SHA1(fe4ae14a6751fef5d7bde49439286f1da3689437), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "m40-41", "M40 4.1" )
	ROMX_LOAD( "m40rom-4.1", 0x0000, 0x2000, CRC(cf55681c) SHA1(fe4ae14a6751fef5d7bde49439286f1da3689437), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "m40-60", "M40 6.0" )
	ROMX_LOAD( "m40rom-6.0", 0x0000, 0x4000, CRC(8114ebec) SHA1(4e2c65b95718c77a87dbee0288f323bd1c8837a3), ROM_BIOS(3))

	ROM_REGION(0x4000, "apb_bios", ROMREGION_ERASEFF) // Processor board with 8086
ROM_END

// CPU board: Z8001BPS CPU, Z8010BPS MMU, 32MHz XTAL, P8253 PIT, MC68B50P ACIA, 1 4-dip bank, 512K RAM
// FDC board: D765AC-2 FDC, D8237AC-5 DMAC, GA04-CF11051, TMP8253P-5, 1 4-dip bank, 1 barely readable chip (AMI8520JFT or something resembling it)
// there are other undocumented PCBs. It uses 2x 8 inch floppy drives
ROM_START(m44) // TODO: implement different hardware. Split to another driver?
	ROM_REGION( 0x14000, "maincpu", 0 ) // 14 MAR. 86 REL B.1
	ROM_LOAD16_BYTE( "pd30.128.c06", 0x0000, 0x4000, CRC(8155dc69) SHA1(ed65f842e2857ad10170c697d945745fd7d47f9c) )
	ROM_LOAD16_BYTE( "pd29.128.a06", 0x0001, 0x4000, CRC(74d7de4b) SHA1(dd3a69ff29a2f1292f3a7db73bd2447bd664e54b) )

	ROM_REGION( 0x114, "plds", 0 )
	ROM_LOAD( "pl46.j09", 0x000, 0x114, NO_DUMP ) // PLD, chip type unknown
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME           FLAGS
COMP( 1981, m20,  0,      0,      m20,     0,     m20_state, empty_init, "Olivetti", "Olivetti L1 M20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1981, m40,  m20,    0,      m20,     0,     m20_state, empty_init, "Olivetti", "Olivetti L1 M40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1986, m44,  0,      0,      m20,     0,     m20_state, empty_init, "Olivetti", "Olivetti L1 M44", MACHINE_IS_SKELETON )
