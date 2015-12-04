// license:???
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
#include "cpu/z8000/z8000.h"
#include "cpu/i86/i86.h"
#include "video/mc6845.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "formats/m20_dsk.h"
#include "formats/pc_dsk.h"
#include "machine/m20_kbd.h"
#include "bus/rs232/rs232.h"
#include "machine/m20_8086.h"
#include "softlist.h"

class m20_state : public driver_device
{
public:
	m20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_p_videoram(*this, "p_videoram"),
		m_palette(*this, "palette")
	{
	}

	required_device<z8001_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<i8251_device> m_kbdi8251;
	required_device<i8251_device> m_ttyi8251;
	required_device<i8255_device> m_i8255;
	required_device<pic8259_device> m_i8259;
	required_device<fd1797_t> m_fd1797;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	optional_device<m20_8086_device> m_apb;

	required_shared_ptr<UINT16> m_p_videoram;
	required_device<palette_device> m_palette;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ16_MEMBER(m20_i8259_r);
	DECLARE_WRITE16_MEMBER(m20_i8259_w);
	DECLARE_READ16_MEMBER(port21_r);
	DECLARE_WRITE16_MEMBER(port21_w);
	DECLARE_WRITE_LINE_MEMBER(tty_clock_tick_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_clock_tick_w);
	DECLARE_WRITE_LINE_MEMBER(timer_tick_w);
	DECLARE_WRITE_LINE_MEMBER(halt_apb_w);
	DECLARE_WRITE_LINE_MEMBER(int_w);
	MC6845_UPDATE_ROW(update_row);

private:
	offs_t m_memsize;
	UINT8 m_port21;
	void install_memory();

public:
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	IRQ_CALLBACK_MEMBER(m20_irq_callback);
};


#define MAIN_CLOCK 4000000 /* 4 MHz */
#define PIXEL_CLOCK XTAL_4_433619MHz


MC6845_UPDATE_ROW( m20_state::update_row )
{
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i, j;

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ((ma | (ra << 1)) << 4) + i;
		UINT16 data = m_p_videoram[ offset ];

		for ( j = 15; j >= 0; j-- )
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

READ16_MEMBER(m20_state::port21_r)
{
	//printf("port21 read: offset 0x%x\n", offset);
	return m_port21;
}

WRITE16_MEMBER(m20_state::port21_w)
{
	//printf("port21 write: offset 0x%x, data 0x%x\n", offset, data);
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

READ16_MEMBER(m20_state::m20_i8259_r)
{
	return m_i8259->read(space, offset)<<1;
}

WRITE16_MEMBER(m20_state::m20_i8259_w)
{
	m_i8259->write(space, offset, (data>>1));
}

WRITE_LINE_MEMBER( m20_state::tty_clock_tick_w )
{
	m_ttyi8251->write_txc(state);
	m_ttyi8251->write_rxc(state);
}

WRITE_LINE_MEMBER( m20_state::kbd_clock_tick_w )
{
	m_kbdi8251->write_txc(state);
	m_kbdi8251->write_rxc(state);
}

WRITE_LINE_MEMBER( m20_state::timer_tick_w )
{
	/* Using HOLD_LINE is not completely correct:
	 * The output of the 8253 is connected to a 74LS74 flop chip.
	 * The output of the flop chip is connected to NVI CPU input.
	 * The flop is reset by a 1:8 decoder which compares CPU ST0-ST3
	 * outputs to detect an interrupt acknowledge transaction.
	 * 8253 is programmed in square wave mode, not rate
	 * generator mode.
	 */
	if(m_apb)
		m_apb->nvi_w(state);
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? HOLD_LINE /*ASSERT_LINE*/ : CLEAR_LINE);
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


static ADDRESS_MAP_START(m20_program_mem, AS_PROGRAM, 16, m20_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x30000, 0x33fff ) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE( 0x40000, 0x41fff ) AM_ROM AM_REGION("maincpu", 0x00000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(m20_data_mem, AS_DATA, 16, m20_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x30000, 0x33fff ) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE( 0x40000, 0x41fff ) AM_ROM AM_REGION("maincpu", 0x00000)
ADDRESS_MAP_END


void m20_state::install_memory()
{
	m_memsize = m_ram->size();
	UINT8 *memptr = m_ram->pointer();
	address_space& pspace = m_maincpu->space(AS_PROGRAM);
	address_space& dspace = m_maincpu->space(AS_DATA);

	/* install mainboard memory (aka DRAM0) */

	/* <0>0000 */
	pspace.install_readwrite_bank(0x0000, 0x3fff, 0x3fff, 0, "dram0_4000");
	dspace.install_readwrite_bank(0x0000, 0x3fff, 0x3fff, 0, "dram0_4000");
	/* <0>4000 */
	pspace.install_readwrite_bank(0x4000, 0x7fff, 0x3fff, 0, "dram0_8000");
	/* <0>8000 */
	pspace.install_readwrite_bank(0x8000, 0xbfff, 0x3fff, 0, "dram0_c000");
	/* <0>C000 */
	pspace.install_readwrite_bank(0xc000, 0xcfff, 0x3fff, 0, "dram0_10000");
	/* <1>0000 */
	pspace.install_readwrite_bank(0x10000, 0x13fff, 0x3fff, 0, "dram0_8000");
	dspace.install_readwrite_bank(0x10000, 0x13fff, 0x3fff, 0, "dram0_14000");
	/* <1>4000 */
	pspace.install_readwrite_bank(0x14000, 0x17fff, 0x3fff, 0, "dram0_c000");
	dspace.install_readwrite_bank(0x14000, 0x17fff, 0x3fff, 0, "dram0_18000");
	/* <1>8000 */
	pspace.install_readwrite_bank(0x18000, 0x1bfff, 0x3fff, 0, "dram0_10000");
	dspace.install_readwrite_bank(0x18000, 0x1bfff, 0x3fff, 0, "dram0_1c000");
	/* <1>c000 empty*/
	/* <2>0000 */
	pspace.install_readwrite_bank(0x20000, 0x23fff, 0x3fff, 0, "dram0_14000");
	dspace.install_readwrite_bank(0x20000, 0x23fff, 0x3fff, 0, "dram0_14000");
	/* <2>4000 */
	pspace.install_readwrite_bank(0x24000, 0x27fff, 0x3fff, 0, "dram0_18000");
	dspace.install_readwrite_bank(0x24000, 0x27fff, 0x3fff, 0, "dram0_18000");
	/* <2>8000 */
	pspace.install_readwrite_bank(0x28000, 0x2bfff, 0x3fff, 0, "dram0_1c000");
	dspace.install_readwrite_bank(0x28000, 0x2bfff, 0x3fff, 0, "dram0_1c000");
	/* <2>c000 empty*/
	/* <3>0000 (video buffer)
	pspace.install_readwrite_bank(0x30000, 0x33fff, 0x3fff, 0, "dram0_0000");
	dspace.install_readwrite_bank(0x30000, 0x33fff, 0x3fff, 0, "dram0_0000");
	*/

	/* <5>0000 */
	dspace.install_readwrite_bank(0x50000, 0x53fff, 0x3fff, 0, "dram0_8000");
	/* <5>4000 */
	dspace.install_readwrite_bank(0x54000, 0x57fff, 0x3fff, 0, "dram0_c000");
	/* <5>8000 */
	dspace.install_readwrite_bank(0x58000, 0x5bfff, 0x3fff, 0, "dram0_10000");
	/* <5>c000 expansion bus */
	/* <6>0000 */
	pspace.install_readwrite_bank(0x60000, 0x63fff, 0x3fff, 0, "dram0_8000");
	dspace.install_readwrite_bank(0x60000, 0x63fff, 0x3fff, 0, "dram0_8000");
	/* <6>4000 */
	pspace.install_readwrite_bank(0x64000, 0x67fff, 0x3fff, 0, "dram0_c000");
	dspace.install_readwrite_bank(0x64000, 0x67fff, 0x3fff, 0, "dram0_c000");
	/* <6>8000 */
	pspace.install_readwrite_bank(0x68000, 0x6bfff, 0x3fff, 0, "dram0_10000");
	dspace.install_readwrite_bank(0x68000, 0x6bfff, 0x3fff, 0, "dram0_10000");
	/* <6>c000 empty*/
	/* segment <7> expansion ROM? */
	/* <8>0000 */
	pspace.install_readwrite_bank(0x80000, 0x83fff, 0x3fff, 0, "dram0_8000");
	dspace.install_readwrite_bank(0x80000, 0x83fff, 0x3fff, 0, "dram0_18000");
	/* <8>4000 */
	pspace.install_readwrite_bank(0x84000, 0x87fff, 0x3fff, 0, "dram0_c000");
	dspace.install_readwrite_bank(0x84000, 0x87fff, 0x3fff, 0, "dram0_1c000");
	/* <9>0000 */
	pspace.install_readwrite_bank(0x90000, 0x93fff, 0x3fff, 0, "dram0_18000");
	dspace.install_readwrite_bank(0x90000, 0x93fff, 0x3fff, 0, "dram0_18000");
	/* <9>4000 */
	pspace.install_readwrite_bank(0x94000, 0x97fff, 0x3fff, 0, "dram0_1c000");
	dspace.install_readwrite_bank(0x94000, 0x97fff, 0x3fff, 0, "dram0_1c000");
	/* <A>0000 */
	pspace.install_readwrite_bank(0xa0000, 0xa3fff, 0x3fff, 0, "dram0_8000");
	dspace.install_readwrite_bank(0xa0000, 0xa3fff, 0x3fff, 0, "dram0_8000");
	/* <A>4000 */
	pspace.install_readwrite_bank(0xa4000, 0xa7fff, 0x3fff, 0, "dram0_c000");
	dspace.install_readwrite_bank(0xa4000, 0xa7fff, 0x3fff, 0, "dram0_c000");

	//membank("dram0_0000")->set_base(memptr);
	membank("dram0_4000")->set_base(memptr + 0x4000);
	membank("dram0_8000")->set_base(memptr + 0x8000);
	membank("dram0_c000")->set_base(memptr + 0xc000);
	membank("dram0_10000")->set_base(memptr + 0x10000);
	membank("dram0_14000")->set_base(memptr + 0x14000);
	membank("dram0_18000")->set_base(memptr + 0x18000);
	membank("dram0_1c000")->set_base(memptr + 0x1c000);

	if (m_memsize > 128 * 1024) {
		/* install memory expansions (DRAM1..DRAM3) */

		if (m_memsize < 256 * 1024) {
			/* 32K expansion cards */

			/* DRAM1, 32K */

			/* prog
			   AM_RANGE( 0x2c000, 0x2ffff ) AM_RAM AM_SHARE("dram1_0000")
			   AM_RANGE( 0x88000, 0x8bfff ) AM_RAM AM_SHARE("dram1_4000")
			   AM_RANGE( 0xa8000, 0xabfff ) AM_RAM AM_SHARE("dram1_4000")
			*/
			pspace.install_readwrite_bank(0x2c000, 0x2ffff, 0x3fff, 0, "dram1_0000");
			pspace.install_readwrite_bank(0x88000, 0x8bfff, 0x3fff, 0, "dram1_4000");
			pspace.install_readwrite_bank(0xa8000, 0xabfff, 0x3fff, 0, "dram1_4000");

			/*
			  data
			  AM_RANGE( 0x04000, 0x07fff ) AM_RAM AM_SHARE("dram1_4000")
			  AM_RANGE( 0x1c000, 0x1ffff ) AM_RAM AM_SHARE("dram1_0000")
			  AM_RANGE( 0x2c000, 0x2ffff ) AM_RAM AM_SHARE("dram1_0000")
			  AM_RANGE( 0xa8000, 0xabfff ) AM_RAM AM_SHARE("dram1_4000")
			*/
			dspace.install_readwrite_bank(0x4000, 0x7fff, 0x3fff, 0, "dram1_4000");
			dspace.install_readwrite_bank(0x1c000, 0x1ffff, 0x3fff, 0, "dram1_0000");
			dspace.install_readwrite_bank(0x2c000, 0x2ffff, 0x3fff, 0, "dram1_0000");
			dspace.install_readwrite_bank(0xa8000, 0xabfff, 0x3fff, 0, "dram1_4000");

			membank("dram1_0000")->set_base(memptr + 0x20000);
			membank("dram1_4000")->set_base(memptr + 0x24000);

			if (m_memsize > 128 * 1024 + 32768) {
				/* DRAM2, 32K */

				/* prog
				   AM_RANGE( 0x8c000, 0x8ffff ) AM_RAM AM_SHARE("dram2_0000")
				   AM_RANGE( 0x98000, 0x9bfff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0xac000, 0xaffff ) AM_RAM AM_SHARE("dram2_0000")
				*/
				pspace.install_readwrite_bank(0x8c000, 0x8ffff, 0x3fff, 0, "dram2_0000");
				pspace.install_readwrite_bank(0x98000, 0x9bfff, 0x3fff, 0, "dram2_4000");
				pspace.install_readwrite_bank(0xac000, 0xaffff, 0x3fff, 0, "dram2_0000");

				/* data
				   AM_RANGE( 0x08000, 0x0bfff ) AM_RAM AM_SHARE("dram2_0000")
				   AM_RANGE( 0x0c000, 0x0ffff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0x88000, 0x8bfff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0x98000, 0x9bfff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0xac000, 0xaffff ) AM_RAM AM_SHARE("dram2_0000")
				 */
				dspace.install_readwrite_bank(0x8000, 0xbfff, 0x3fff, 0, "dram2_0000");
				dspace.install_readwrite_bank(0xc000, 0xffff, 0x3fff, 0, "dram2_4000");
				dspace.install_readwrite_bank(0x88000, 0x8bfff, 0x3fff, 0, "dram2_4000");
				dspace.install_readwrite_bank(0x98000, 0x9bfff, 0x3fff, 0, "dram2_4000");
				dspace.install_readwrite_bank(0xac000, 0xaffff, 0x3fff, 0, "dram2_0000");

				membank("dram2_0000")->set_base(memptr + 0x28000);
				membank("dram2_4000")->set_base(memptr + 0x2c000);
			}
			if (m_memsize > 128 * 1024 + 2 * 32768) {
				/* DRAM3, 32K */

				/* prog
				   AM_RANGE( 0x9c000, 0x9ffff ) AM_RAM AM_SHARE("dram3_0000")
				   AM_RANGE( 0xb0000, 0xb3fff ) AM_RAM AM_SHARE("dram3_4000")
				*/
				pspace.install_readwrite_bank(0x9c000, 0x9ffff, 0x3fff, 0, "dram3_0000");
				pspace.install_readwrite_bank(0xb0000, 0xb3fff, 0x3fff, 0, "dram3_4000");

				/* data
				   AM_RANGE( 0x44000, 0x47fff ) AM_RAM AM_SHARE("dram3_0000")
				   AM_RANGE( 0x48000, 0x4bfff ) AM_RAM AM_SHARE("dram3_4000")
				   AM_RANGE( 0x8c000, 0x8ffff ) AM_RAM AM_SHARE("dram3_0000")
				   AM_RANGE( 0x9c000, 0x9ffff ) AM_RAM AM_SHARE("dram3_0000")
				   AM_RANGE( 0xb0000, 0xb3fff ) AM_RAM AM_SHARE("dram3_4000")
				   AM_RANGE( 0xc0000, 0xc3fff ) AM_RAM AM_SHARE("dram3_4000")
				 */
				dspace.install_readwrite_bank(0x44000, 0x47fff, 0x3fff, 0, "dram3_0000");
				dspace.install_readwrite_bank(0x48000, 0x4bfff, 0x3fff, 0, "dram3_4000");
				dspace.install_readwrite_bank(0x8c000, 0x8ffff, 0x3fff, 0, "dram3_0000");
				dspace.install_readwrite_bank(0x9c000, 0x9ffff, 0x3fff, 0, "dram3_0000");
				dspace.install_readwrite_bank(0xb0000, 0xb3fff, 0x3fff, 0, "dram3_4000");
				dspace.install_readwrite_bank(0xc0000, 0xc3fff, 0x3fff, 0, "dram3_4000");

				membank("dram3_0000")->set_base(memptr + 0x30000);
				membank("dram3_4000")->set_base(memptr + 0x34000);
			}
		}
		else {
			/* 128K expansion cards */

			/* DRAM1, 128K */

			/* prog
			   AM_RANGE( 0x2c000, 0x2ffff ) AM_RAM AM_SHARE("dram1_0000")
			   AM_RANGE( 0x88000, 0x8bfff ) AM_RAM AM_SHARE("dram1_4000")
			   AM_RANGE( 0x8c000, 0x8ffff ) AM_RAM AM_SHARE("dram1_8000")
			   AM_RANGE( 0x98000, 0x9bfff ) AM_RAM AM_SHARE("dram1_c000")
			   AM_RANGE( 0x9c000, 0x9ffff ) AM_RAM AM_SHARE("dram1_10000")
			   AM_RANGE( 0xa8000, 0xabfff ) AM_RAM AM_SHARE("dram1_4000")
			   AM_RANGE( 0xac000, 0xaffff ) AM_RAM AM_SHARE("dram1_8000")
			   AM_RANGE( 0xb0000, 0xb3fff ) AM_RAM AM_SHARE("dram1_14000")
			   AM_RANGE( 0xb4000, 0xb7fff ) AM_RAM AM_SHARE("dram1_18000")
			   AM_RANGE( 0xb8000, 0xbbfff ) AM_RAM AM_SHARE("dram1_1c000")
			*/
			pspace.install_readwrite_bank(0x2c000, 0x2ffff, 0x3fff, 0, "dram1_0000");
			pspace.install_readwrite_bank(0x88000, 0x8bfff, 0x3fff, 0, "dram1_4000");
			pspace.install_readwrite_bank(0x8c000, 0x8ffff, 0x3fff, 0, "dram1_8000");
			pspace.install_readwrite_bank(0x98000, 0x9bfff, 0x3fff, 0, "dram1_c000");
			pspace.install_readwrite_bank(0x9c000, 0x9ffff, 0x3fff, 0, "dram1_10000");
			pspace.install_readwrite_bank(0xa8000, 0xabfff, 0x3fff, 0, "dram1_4000");
			pspace.install_readwrite_bank(0xac000, 0xaffff, 0x3fff, 0, "dram1_8000");
			pspace.install_readwrite_bank(0xb0000, 0xb3fff, 0x3fff, 0, "dram1_14000");
			pspace.install_readwrite_bank(0xb4000, 0xb7fff, 0x3fff, 0, "dram1_18000");
			pspace.install_readwrite_bank(0xb8000, 0xbbfff, 0x3fff, 0, "dram1_1c000");

			/* data
			   AM_RANGE( 0x04000, 0x07fff ) AM_RAM AM_SHARE("dram1_4000")
			   AM_RANGE( 0x1c000, 0x1ffff ) AM_RAM AM_SHARE("dram1_0000")
			   AM_RANGE( 0x2c000, 0x2ffff ) AM_RAM AM_SHARE("dram1_0000")
			   AM_RANGE( 0x88000, 0x8bfff ) AM_RAM AM_SHARE("dram1_c000")
			   AM_RANGE( 0x8c000, 0x8ffff ) AM_RAM AM_SHARE("dram1_10000")
			   AM_RANGE( 0x98000, 0x9bfff ) AM_RAM AM_SHARE("dram1_c000")
			   AM_RANGE( 0x9c000, 0x9ffff ) AM_RAM AM_SHARE("dram1_10000")
			   AM_RANGE( 0xa8000, 0xabfff ) AM_RAM AM_SHARE("dram1_4000")
			   AM_RANGE( 0xac000, 0xaffff ) AM_RAM AM_SHARE("dram1_8000")
			   AM_RANGE( 0xb0000, 0xb3fff ) AM_RAM AM_SHARE("dram1_14000")
			   AM_RANGE( 0xb4000, 0xb7fff ) AM_RAM AM_SHARE("dram1_18000")
			   AM_RANGE( 0xb8000, 0xbbfff ) AM_RAM AM_SHARE("dram1_1c000")
			 */
			dspace.install_readwrite_bank(0x4000, 0x7fff, 0x3fff, 0, "dram1_4000");
			dspace.install_readwrite_bank(0x1c000, 0x1ffff, 0x3fff, 0, "dram1_0000");
			dspace.install_readwrite_bank(0x2c000, 0x2ffff, 0x3fff, 0, "dram1_0000");
			dspace.install_readwrite_bank(0x88000, 0x8bfff, 0x3fff, 0, "dram1_c000");
			dspace.install_readwrite_bank(0x8c000, 0x8ffff, 0x3fff, 0, "dram1_10000");
			dspace.install_readwrite_bank(0x98000, 0x9bfff, 0x3fff, 0, "dram1_c000");
			dspace.install_readwrite_bank(0x9c000, 0x9ffff, 0x3fff, 0, "dram1_10000");
			dspace.install_readwrite_bank(0xa8000, 0xabfff, 0x3fff, 0, "dram1_4000");
			dspace.install_readwrite_bank(0xac000, 0xaffff, 0x3fff, 0, "dram1_8000");
			dspace.install_readwrite_bank(0xb0000, 0xb3fff, 0x3fff, 0, "dram1_14000");
			dspace.install_readwrite_bank(0xb4000, 0xb7fff, 0x3fff, 0, "dram1_18000");
			dspace.install_readwrite_bank(0xb8000, 0xbbfff, 0x3fff, 0, "dram1_1c000");

			membank("dram1_0000")->set_base(memptr + 0x20000);
			membank("dram1_4000")->set_base(memptr + 0x24000);
			membank("dram1_8000")->set_base(memptr + 0x28000);
			membank("dram1_c000")->set_base(memptr + 0x2c000);
			membank("dram1_10000")->set_base(memptr + 0x30000);
			membank("dram1_14000")->set_base(memptr + 0x34000);
			membank("dram1_18000")->set_base(memptr + 0x38000);
			membank("dram1_1c000")->set_base(memptr + 0x3c000);

			if (m_memsize > 256 * 1024) {
				/* DRAM2, 128K */

				/* prog
				   AM_RANGE( 0xbc000, 0xbffff ) AM_RAM AM_SHARE("dram2_0000")

				   AM_RANGE( 0xc0000, 0xc3fff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0xc4000, 0xc7fff ) AM_RAM AM_SHARE("dram2_8000")
				   AM_RANGE( 0xc8000, 0xcbfff ) AM_RAM AM_SHARE("dram2_c000")
				   AM_RANGE( 0xcc000, 0xcffff ) AM_RAM AM_SHARE("dram2_10000")

				   AM_RANGE( 0xd0000, 0xd3fff ) AM_RAM AM_SHARE("dram2_14000")
				   AM_RANGE( 0xd4000, 0xd7fff ) AM_RAM AM_SHARE("dram2_18000")
				   AM_RANGE( 0xd8000, 0xdbfff ) AM_RAM AM_SHARE("dram2_1c000")
				 */
				pspace.install_readwrite_bank(0xbc000, 0xbffff, 0x3fff, 0, "dram2_0000");
				pspace.install_readwrite_bank(0xc0000, 0xc3fff, 0x3fff, 0, "dram2_4000");
				pspace.install_readwrite_bank(0xc4000, 0xc7fff, 0x3fff, 0, "dram2_8000");
				pspace.install_readwrite_bank(0xc8000, 0xcbfff, 0x3fff, 0, "dram2_c000");
				pspace.install_readwrite_bank(0xcc000, 0xcffff, 0x3fff, 0, "dram2_10000");
				pspace.install_readwrite_bank(0xd0000, 0xd3fff, 0x3fff, 0, "dram2_14000");
				pspace.install_readwrite_bank(0xd4000, 0xd7fff, 0x3fff, 0, "dram2_18000");
				pspace.install_readwrite_bank(0xd8000, 0xdbfff, 0x3fff, 0, "dram2_1c000");

				/* data
				   AM_RANGE( 0x08000, 0x0bfff ) AM_RAM AM_SHARE("dram2_0000")
				   AM_RANGE( 0x0c000, 0x0ffff ) AM_RAM AM_SHARE("dram2_4000")

				   AM_RANGE( 0xbc000, 0xbffff ) AM_RAM AM_SHARE("dram2_0000")

				   AM_RANGE( 0xc0000, 0xc3fff ) AM_RAM AM_SHARE("dram2_4000")
				   AM_RANGE( 0xc4000, 0xc7fff ) AM_RAM AM_SHARE("dram2_8000")
				   AM_RANGE( 0xc8000, 0xcbfff ) AM_RAM AM_SHARE("dram2_c000")
				   AM_RANGE( 0xcc000, 0xcffff ) AM_RAM AM_SHARE("dram2_10000")

				   AM_RANGE( 0xd0000, 0xd3fff ) AM_RAM AM_SHARE("dram2_14000")
				   AM_RANGE( 0xd4000, 0xd7fff ) AM_RAM AM_SHARE("dram2_18000")
				   AM_RANGE( 0xd8000, 0xdbfff ) AM_RAM AM_SHARE("dram2_1c000")
				*/
				dspace.install_readwrite_bank(0x8000, 0xbfff, 0x3fff, 0, "dram2_0000");
				dspace.install_readwrite_bank(0xc000, 0xffff, 0x3fff, 0, "dram2_4000");
				dspace.install_readwrite_bank(0xbc000, 0xbffff, 0x3fff, 0, "dram2_0000");
				dspace.install_readwrite_bank(0xc0000, 0xc3fff, 0x3fff, 0, "dram2_4000");
				dspace.install_readwrite_bank(0xc4000, 0xc7fff, 0x3fff, 0, "dram2_8000");
				dspace.install_readwrite_bank(0xc8000, 0xcbfff, 0x3fff, 0, "dram2_c000");
				dspace.install_readwrite_bank(0xcc000, 0xcffff, 0x3fff, 0, "dram2_10000");
				dspace.install_readwrite_bank(0xd0000, 0xd3fff, 0x3fff, 0, "dram2_14000");
				dspace.install_readwrite_bank(0xd4000, 0xd7fff, 0x3fff, 0, "dram2_18000");
				dspace.install_readwrite_bank(0xd8000, 0xdbfff, 0x3fff, 0, "dram2_1c000");

				membank("dram2_0000")->set_base(memptr + 0x40000);
				membank("dram2_4000")->set_base(memptr + 0x44000);
				membank("dram2_8000")->set_base(memptr + 0x48000);
				membank("dram2_c000")->set_base(memptr + 0x4c000);
				membank("dram2_10000")->set_base(memptr + 0x50000);
				membank("dram2_14000")->set_base(memptr + 0x54000);
				membank("dram2_18000")->set_base(memptr + 0x58000);
				membank("dram2_1c000")->set_base(memptr + 0x5c000);
			}
			if (m_memsize > 384 * 1024) {
				/* DRAM3, 128K */

				/* prog
				   AM_RANGE( 0xdc000, 0xdffff ) AM_RAM AM_SHARE("dram3_0000")

				   AM_RANGE( 0xe0000, 0xe3fff ) AM_RAM AM_SHARE("dram3_4000")
				   AM_RANGE( 0xe4000, 0xe7fff ) AM_RAM AM_SHARE("dram3_8000")
				   AM_RANGE( 0xe8000, 0xebfff ) AM_RAM AM_SHARE("dram3_c000")
				   AM_RANGE( 0xec000, 0xeffff ) AM_RAM AM_SHARE("dram3_10000")

				   AM_RANGE( 0xf0000, 0xf3fff ) AM_RAM AM_SHARE("dram3_14000")
				   AM_RANGE( 0xf4000, 0xf7fff ) AM_RAM AM_SHARE("dram3_18000")
				   AM_RANGE( 0xf8000, 0xfbfff ) AM_RAM AM_SHARE("dram3_1c000")
				   AM_RANGE( 0xfc000, 0xfffff ) AM_RAM AM_SHARE("dram3_0000")
				*/
				pspace.install_readwrite_bank(0xdc000, 0xdffff, 0x3fff, 0, "dram3_0000");
				pspace.install_readwrite_bank(0xe0000, 0xe3fff, 0x3fff, 0, "dram3_4000");
				pspace.install_readwrite_bank(0xe4000, 0xe7fff, 0x3fff, 0, "dram3_8000");
				pspace.install_readwrite_bank(0xe8000, 0xebfff, 0x3fff, 0, "dram3_c000");
				pspace.install_readwrite_bank(0xec000, 0xeffff, 0x3fff, 0, "dram3_10000");
				pspace.install_readwrite_bank(0xf0000, 0xf3fff, 0x3fff, 0, "dram3_14000");
				pspace.install_readwrite_bank(0xf4000, 0xf7fff, 0x3fff, 0, "dram3_18000");
				pspace.install_readwrite_bank(0xf8000, 0xfbfff, 0x3fff, 0, "dram3_1c000");
				pspace.install_readwrite_bank(0xfc000, 0xfffff, 0x3fff, 0, "dram3_0000");

				/* data
				   AM_RANGE( 0x44000, 0x47fff ) AM_RAM AM_SHARE("dram3_0000")
				   AM_RANGE( 0x48000, 0x4bfff ) AM_RAM AM_SHARE("dram3_4000")
				   AM_RANGE( 0xdc000, 0xdffff ) AM_RAM AM_SHARE("dram3_0000")

				   AM_RANGE( 0xe0000, 0xe3fff ) AM_RAM AM_SHARE("dram3_4000")
				   AM_RANGE( 0xe4000, 0xe7fff ) AM_RAM AM_SHARE("dram3_8000")
				   AM_RANGE( 0xe8000, 0xebfff ) AM_RAM AM_SHARE("dram3_c000")
				   AM_RANGE( 0xec000, 0xeffff ) AM_RAM AM_SHARE("dram3_10000")

				   AM_RANGE( 0xf0000, 0xf3fff ) AM_RAM AM_SHARE("dram3_14000")
				   AM_RANGE( 0xf4000, 0xf7fff ) AM_RAM AM_SHARE("dram3_18000")
				   AM_RANGE( 0xf8000, 0xfbfff ) AM_RAM AM_SHARE("dram3_1c000")
				   AM_RANGE( 0xfc000, 0xfffff ) AM_RAM AM_SHARE("dram3_0000")
				*/
				dspace.install_readwrite_bank(0x44000, 0x47fff, 0x3fff, 0, "dram3_0000");
				dspace.install_readwrite_bank(0x48000, 0x4bfff, 0x3fff, 0, "dram3_4000");
				dspace.install_readwrite_bank(0xdc000, 0xdffff, 0x3fff, 0, "dram3_0000");
				dspace.install_readwrite_bank(0xe0000, 0xe3fff, 0x3fff, 0, "dram3_4000");
				dspace.install_readwrite_bank(0xe4000, 0xe7fff, 0x3fff, 0, "dram3_8000");
				dspace.install_readwrite_bank(0xe8000, 0xebfff, 0x3fff, 0, "dram3_c000");
				dspace.install_readwrite_bank(0xec000, 0xeffff, 0x3fff, 0, "dram3_10000");
				dspace.install_readwrite_bank(0xf0000, 0xf3fff, 0x3fff, 0, "dram3_14000");
				dspace.install_readwrite_bank(0xf4000, 0xf7fff, 0x3fff, 0, "dram3_18000");
				dspace.install_readwrite_bank(0xf8000, 0xfbfff, 0x3fff, 0, "dram3_1c000");
				dspace.install_readwrite_bank(0xfc000, 0xfffff, 0x3fff, 0, "dram3_0000");

				membank("dram3_0000")->set_base(memptr + 0x60000);
				membank("dram3_4000")->set_base(memptr + 0x64000);
				membank("dram3_8000")->set_base(memptr + 0x68000);
				membank("dram3_c000")->set_base(memptr + 0x6c000);
				membank("dram3_10000")->set_base(memptr + 0x70000);
				membank("dram3_14000")->set_base(memptr + 0x74000);
				membank("dram3_18000")->set_base(memptr + 0x78000);
				membank("dram3_1c000")->set_base(memptr + 0x7c000);
			}
		}
	}
}

static ADDRESS_MAP_START(m20_io, AS_IO, 16, m20_state)
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x00, 0x07) AM_DEVREADWRITE8("fd1797", fd1797_t, read, write, 0x00ff)

	AM_RANGE(0x20, 0x21) AM_READWRITE(port21_r, port21_w);

	AM_RANGE(0x60, 0x61) AM_DEVWRITE8("crtc", mc6845_device, address_w, 0x00ff)
	AM_RANGE(0x62, 0x63) AM_DEVWRITE8("crtc", mc6845_device, address_w, 0xff00) // FIXME
	AM_RANGE(0x62, 0x63) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0x00ff)
	AM_RANGE(0x64, 0x65) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)

	AM_RANGE(0x80, 0x87) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0x00ff)

	AM_RANGE(0xa0, 0xa1) AM_DEVREADWRITE8("i8251_1", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xa2, 0xa3) AM_DEVREADWRITE8("i8251_1", i8251_device, status_r, control_w, 0x00ff)

	AM_RANGE(0xc0, 0xc1) AM_DEVREADWRITE8("i8251_2", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xc2, 0xc3) AM_DEVREADWRITE8("i8251_2", i8251_device, status_r, control_w, 0x00ff)

	AM_RANGE(0x120, 0x127) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)

	AM_RANGE(0x140, 0x143) AM_READWRITE(m20_i8259_r, m20_i8259_w)

	AM_RANGE(0x3ffa, 0x3ffd) AM_DEVWRITE("apb", m20_8086_device, handshake_w)
ADDRESS_MAP_END

IRQ_CALLBACK_MEMBER(m20_state::m20_irq_callback)
{
	if (! irqline)
		return 0xff; // NVI, value ignored
	else
		return m_i8259->acknowledge();
}

WRITE_LINE_MEMBER(m20_state::int_w)
{
	if(m_apb && !m_apb->halted())
		m_apb->vi_w(state);
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, state ? ASSERT_LINE : CLEAR_LINE);
}

void m20_state::machine_start()
{
	install_memory();
}

void m20_state::machine_reset()
{
	UINT8 *ROM = memregion("maincpu")->base();
	UINT8 *RAM = (UINT8 *)(m_ram->pointer() + 0x4000);

	if (m_memsize >= 256 * 1024)
		m_port21 = 0xdf;
	else
		m_port21 = 0xff;

	if(system_bios() > 0)  // bits have different meanings?
		m_port21 &= ~8;

	m_fd1797->reset();

	memcpy(RAM, ROM, 8);  // we need only the reset vector
	m_maincpu->reset();     // reset the CPU to ensure it picks up the new vector
	if(m_apb)
		m_apb->m_8086->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}


static SLOT_INTERFACE_START( m20_floppies )
	SLOT_INTERFACE( "5dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( m20_state::floppy_formats )
	FLOPPY_M20_FORMAT,
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START(keyboard)
	SLOT_INTERFACE("m20", M20_KEYBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( m20, m20_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z8001, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(m20_program_mem)
	MCFG_CPU_DATA_MAP(m20_data_mem)
	MCFG_CPU_IO_MAP(m20_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(m20_state,m20_irq_callback)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("160K")
	MCFG_RAM_DEFAULT_VALUE(0)
	MCFG_RAM_EXTRA_OPTIONS("128K,192K,224K,256K,384K,512K")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* Devices */
	MCFG_FD1797_ADD("fd1797", 1000000)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("i8259", pic8259_device, ir0_w))
	MCFG_FLOPPY_DRIVE_ADD("fd1797:0", m20_floppies, "5dd", m20_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1797:1", m20_floppies, "5dd", m20_state::floppy_formats)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", PIXEL_CLOCK/8) /* hand tuned to get ~50 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(16)
	MCFG_MC6845_UPDATE_ROW_CB(m20_state, update_row)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)

	MCFG_DEVICE_ADD("i8251_1", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("kbd", rs232_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("i8259", pic8259_device, ir4_w))

	MCFG_DEVICE_ADD("i8251_2", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("i8259", pic8259_device, ir3_w))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("i8259", pic8259_device, ir5_w))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(1230782)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(m20_state, tty_clock_tick_w))
	MCFG_PIT8253_CLK1(1230782)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(m20_state, kbd_clock_tick_w))
	MCFG_PIT8253_CLK2(1230782)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(m20_state, timer_tick_w))

	MCFG_PIC8259_ADD("i8259", WRITELINE(m20_state, int_w), VCC, NULL)

	MCFG_RS232_PORT_ADD("kbd", keyboard, "m20")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251_1", i8251_device, write_rxd))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251_2", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("apb", M20_8086, 0)

	MCFG_SOFTWARE_LIST_ADD("flop_list","m20")
MACHINE_CONFIG_END

ROM_START(m20)
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "m20", "M20 1.0" )
	ROMX_LOAD("m20.bin", 0x0000, 0x2000, CRC(5c93d931) SHA1(d51025e087a94c55529d7ee8fd18ff4c46d93230), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "m20-20d", "M20 2.0d" )
	ROMX_LOAD("m20-20d.bin", 0x0000, 0x2000, CRC(cbe265a6) SHA1(c7cb9d9900b7b5014fcf1ceb2e45a66a91c564d0), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "m20-20f", "M20 2.0f" )
	ROMX_LOAD("m20-20f.bin", 0x0000, 0x2000, CRC(db7198d8) SHA1(149d8513867081d31c73c2965dabb36d5f308041), ROM_BIOS(3))
ROM_END

ROM_START(m40)
	ROM_REGION(0x14000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "m40-81", "M40 15.dec.81" )
	ROMX_LOAD( "m40rom-15-dec-81", 0x0000, 0x2000, CRC(e8e7df84) SHA1(e86018043bf5a23ff63434f9beef7ce2972d8153), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "m40-82", "M40 17.dec.82" )
	ROMX_LOAD( "m40rom-17-dec-82", 0x0000, 0x2000, CRC(cf55681c) SHA1(fe4ae14a6751fef5d7bde49439286f1da3689437), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "m40-41", "M40 4.1" )
	ROMX_LOAD( "m40rom-4.1", 0x0000, 0x2000, CRC(cf55681c) SHA1(fe4ae14a6751fef5d7bde49439286f1da3689437), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "m40-60", "M40 6.0" )
	ROMX_LOAD( "m40rom-6.0", 0x0000, 0x4000, CRC(8114ebec) SHA1(4e2c65b95718c77a87dbee0288f323bd1c8837a3), ROM_BIOS(4))

	ROM_REGION(0x4000, "apb_bios", ROMREGION_ERASEFF) // Processor board with 8086
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   INIT COMPANY     FULLNAME        FLAGS */
COMP( 1981, m20,   0,      0,      m20,    0,   driver_device,    0, "Olivetti", "Olivetti L1 M20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1981, m40,   m20,    0,      m20,    0,   driver_device,    0, "Olivetti", "Olivetti L1 M40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
