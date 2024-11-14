// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    P8000

    09/2009 Skeleton driver based on Matt Knoth's emulator.
    P8000emu (http://knothusa.net/Home.php) has been a great source of info.
    Other info from
      * http://www.pofo.de/P8000/notes/books/
      * http://www.pofo.de/P8000/

    P8000 memory layout
      * divided into 3 banks of 64k
      * bank A is for roms, only 0000-1FFF is populated
      * bank B is for static ram, only 2000-2FFF exists
      * bank C is for dynamic ram, all 64k is available.
      * selection is done with OUT(c), code
      * code = 0 for do nothing; 1 = bank A; 2 = bank B; 4 = bank C.
      * Reg C = 0; Reg B = start address of memory that is being switched,
        for example B = 20 indicates "bank2" in memory map, and also the
        corresponding address in bank A/B/C.

    P8000 monitor commands
      * B : ?
      * D : display and modify memory
      * F : fill memory
      * G : go to
      * M : move (copy) memory
      * N : dump registers
      * O : boot from floppy
      * P : ?
      * Q : ?
      * R : dump registers
      * S : boot from floppy
      * T : jump to ROM at CEF0
      * X : jump to ROM at DB00
      * return : boot from floppy disk

    P8000_16 : All input must be in uppercase.

    TODO:
      * properly implement Z80 daisy chain in 16 bit board
      * Find out how to enter hardware check on 16 bit board

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "cpu/z8000/z8000.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "speaker.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

class p8k_16_daisy_device : public device_t, public z80_daisy_chain_interface
{
public:
	p8k_16_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint16_t viack_r() {
		device_z80daisy_interface *intf = daisy_get_irq_device();
		return intf ? intf->z80daisy_irq_ack() : 0;
	}
	void reti_w(uint8_t data) { if(data == 0x4d) daisy_call_reti_device(); }
protected:
	void device_start() override {}
};

DEFINE_DEVICE_TYPE(P8K_16_DAISY, p8k_16_daisy_device, "p8k_16_daisy", "P8000 16-bit daisy chain device")

p8k_16_daisy_device::p8k_16_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P8K_16_DAISY, tag, owner, clock)
	, z80_daisy_chain_interface(mconfig, *this) {}


namespace {

class p8k_state : public driver_device
{
public:
	p8k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_daisy(*this, "p8k_16_daisy")
		, m_pio2(*this, "pio2")
		, m_i8272(*this, "i8272")
	{ }

	void p8k(machine_config &config);
	void p8k_16(machine_config &config);

	void init_p8k();

private:
	uint8_t port0_r(offs_t offset);
	void port0_w(offs_t offset, uint8_t data);
	DECLARE_MACHINE_RESET(p8k);

	[[maybe_unused]] void fdc_irq(int state);
	void p8k_daisy_interrupt(int state);
	void p8k_dma_irq_w(int state);
	void p8k_16_daisy_interrupt(int state);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	void p8k_16_datamap(address_map &map) ATTR_COLD;
	void p8k_16_iomap(address_map &map) ATTR_COLD;
	void p8k_16_memmap(address_map &map) ATTR_COLD;
	void p8k_iomap(address_map &map) ATTR_COLD;
	void p8k_memmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu; // Z80 or Z8001 depending on the machine
	optional_device<p8k_16_daisy_device> m_daisy;
	optional_device<z80pio_device> m_pio2;
	optional_device<i8272a_device> m_i8272;
};

/***************************************************************************

    P8000 8bit

****************************************************************************/

void p8k_state::p8k_memmap(address_map &map)
{
	map(0x0000, 0x0FFF).bankrw("bank0");
	map(0x1000, 0x1FFF).bankrw("bank1");
	map(0x2000, 0x2FFF).bankrw("bank2");
	map(0x3000, 0x3FFF).bankrw("bank3");
	map(0x4000, 0x4FFF).bankrw("bank4");
	map(0x5000, 0x5FFF).bankrw("bank5");
	map(0x6000, 0x6FFF).bankrw("bank6");
	map(0x7000, 0x7FFF).bankrw("bank7");
	map(0x8000, 0x8FFF).bankrw("bank8");
	map(0x9000, 0x9FFF).bankrw("bank9");
	map(0xA000, 0xAFFF).bankrw("bank10");
	map(0xB000, 0xBFFF).bankrw("bank11");
	map(0xC000, 0xCFFF).bankrw("bank12");
	map(0xD000, 0xDFFF).bankrw("bank13");
	map(0xE000, 0xEFFF).bankrw("bank14");
	map(0xF000, 0xFFFF).bankrw("bank15");
}

void p8k_state::p8k_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw(FUNC(p8k_state::port0_r), FUNC(p8k_state::port0_w)); // MH7489
	map(0x08, 0x0b).rw("ctc0", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).rw("pio0", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x18, 0x1b).rw("pio1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x1c, 0x1f).rw(m_pio2, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x20, 0x21).m(m_i8272, FUNC(i8272a_device::map));
	map(0x24, 0x27).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x28, 0x2b).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x2c, 0x2f).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x3c, 0x3c).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
}



uint8_t p8k_state::port0_r(offs_t offset)
{
	return 0;
}

// see memory explanation above
void p8k_state::port0_w(offs_t offset, uint8_t data)
{
	uint8_t breg = m_maincpu->state_int(Z80_B) >> 4;
	if ((data==1) || (data==2) || (data==4))
	{
		char banknum[8];
		sprintf(banknum,"bank%d", breg);

		offset = 0;
		if (data==2)
			offset = 16;
		else
		if (data==4)
			offset = 32;

		offset += breg;

		membank(banknum)->set_entry(offset);
	}
	else
	if (data)
		printf("Invalid data %X for bank %d\n",data,breg);
}


/***************************************************************************

    P8000 8bit Peripherals

****************************************************************************/

void p8k_state::p8k_daisy_interrupt(int state)
{
	m_maincpu->set_input_line(0, state);
}

/* Z80 DMA */

void p8k_state::p8k_dma_irq_w(int state)
{
	m_i8272->tc_w(state);
	p8k_daisy_interrupt(state);
}

uint8_t p8k_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void p8k_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

uint8_t p8k_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void p8k_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}


/* Z80 Daisy Chain */

static const z80_daisy_config p8k_daisy_chain[] =
{
	{ "dma" },   /* FDC related */
	{ "pio2" },
	{ "ctc0" },
	{ "sio" },
	{ "sio1" },
	{ "pio0" },
	{ "pio1" },
	{ "ctc1" },
	{ nullptr }
};

/* Intel 8272 Interface */

void p8k_state::fdc_irq(int state)
{
	m_pio2->port_b_write(state ? 0x10 : 0x00);
}

static void p8k_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

/* Input ports */
static INPUT_PORTS_START( p8k )
	PORT_START("DSW")
	PORT_BIT( 0x7f, 0x7f, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Hardware Test")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x80, DEF_STR(On))
INPUT_PORTS_END


MACHINE_RESET_MEMBER(p8k_state,p8k)
{
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	membank("bank3")->set_entry(0);
	membank("bank4")->set_entry(0);
	membank("bank5")->set_entry(0);
	membank("bank6")->set_entry(0);
	membank("bank7")->set_entry(0);
	membank("bank8")->set_entry(0);
	membank("bank9")->set_entry(0);
	membank("bank10")->set_entry(0);
	membank("bank11")->set_entry(0);
	membank("bank12")->set_entry(0);
	membank("bank13")->set_entry(0);
	membank("bank14")->set_entry(0);
	membank("bank15")->set_entry(0);
}

void p8k_state::init_p8k()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("bank0")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank1")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank2")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank3")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank4")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank5")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank6")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank7")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank8")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank9")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank10")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank11")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank12")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank13")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank14")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
	membank("bank15")->configure_entries(0, 48, &RAM[0x0000], 0x1000);
}


/***************************************************************************

    P8000 16bit

****************************************************************************/

void p8k_state::p8k_16_memmap(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("maincpu",0);
	map(0x04000, 0x07fff).ram().share("share1");
	map(0x08000, 0xfffff).ram().share("share2");
}

void p8k_state::p8k_16_datamap(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("maincpu",0);
	map(0x04000, 0x07fff).ram().share("share1");
	map(0x08000, 0xfffff).ram().share("share2");
}


void p8k_state::p8k_16_iomap(address_map &map)
{
//  map(0x0fef0, 0x0feff) // clock
	map(0x0ff80, 0x0ff87).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0x0ff88, 0x0ff8f).rw("sio1", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0x0ff90, 0x0ff97).rw("pio0", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)).umask16(0x00ff);
	map(0x0ff98, 0x0ff9f).rw("pio1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)).umask16(0x00ff);
	map(0x0ffa0, 0x0ffa7).rw(m_pio2, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)).umask16(0x00ff);
	map(0x0ffa8, 0x0ffaf).rw("ctc0", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0x0ffb0, 0x0ffb7).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
//  map(0x0ffc0, 0x0ffc1) // SCR
//  map(0x0ffc8, 0x0ffc9) // SBR
//  map(0x0ffd0, 0x0ffd1) // NBR
//  map(0x0ffd8, 0x0ffd9) // SNVR
	map(0x0ffe1, 0x0ffe1).w(m_daisy, FUNC(p8k_16_daisy_device::reti_w));
//  map(0x0fff0, 0x0fff1) // TRPL
//  map(0x0fff8, 0x0fff9) // IF1L
}


/***************************************************************************

    P8000 16bit Peripherals

****************************************************************************/

void p8k_state::p8k_16_daisy_interrupt(int state)
{
	m_maincpu->set_input_line(z8001_device::VI_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


/* Z80 Daisy Chain */

static const z80_daisy_config p8k_16_daisy_chain[] =
{
	{ "ctc0" },
	{ "ctc1" },
	{ "sio" },
	{ "sio1" },
	{ "pio0" },
	{ "pio1" },
	{ "pio2" },
	{ nullptr }
};



#if 0
/* F4 Character Displayer */
static const gfx_layout p8k_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( p8k )
	GFXDECODE_ENTRY( "chargen", 0x0000, p8k_charlayout, 0, 1 )
GFXDECODE_END
#endif


/***************************************************************************

    Machine Drivers

****************************************************************************/

void p8k_state::p8k(machine_config &config)
{
	/* basic machine hardware */
	z80_device& maincpu(Z80(config, "maincpu", 16_MHz_XTAL / 4));
	maincpu.set_daisy_config(p8k_daisy_chain);
	maincpu.set_addrmap(AS_PROGRAM, &p8k_state::p8k_memmap);
	maincpu.set_addrmap(AS_IO, &p8k_state::p8k_iomap);

	MCFG_MACHINE_RESET_OVERRIDE(p8k_state,p8k)

	/* peripheral hardware */
	z80dma_device& dma(Z80DMA(config, "dma", 16_MHz_XTAL / 4));
	dma.out_busreq_callback().set(FUNC(p8k_state::p8k_dma_irq_w));
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dma.in_mreq_callback().set(FUNC(p8k_state::memory_read_byte));
	dma.out_mreq_callback().set(FUNC(p8k_state::memory_write_byte));
	dma.in_iorq_callback().set(FUNC(p8k_state::io_read_byte));
	dma.out_iorq_callback().set(FUNC(p8k_state::io_write_byte));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 307200));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txcb_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxcb_w));

	z80ctc_device& ctc0(Z80CTC(config, "ctc0", 1229000));    /* 1.22MHz clock */
	// to implement: callbacks!
	// manual states the callbacks should go to
	// Baud Gen 3, FDC, System-Kanal
	ctc0.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device& ctc1(Z80CTC(config, "ctc1", 1229000));    /* 1.22MHz clock */
	// to implement: callbacks!
	// manual states the callbacks should go to
	// Baud Gen 0, Baud Gen 1, Baud Gen 2,
	ctc1.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device& sio(Z80SIO(config, "sio", 16_MHz_XTAL / 4));
	sio.out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));

	z80sio_device& sio1(Z80SIO(config, "sio1", 16_MHz_XTAL / 4));
	sio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio0(Z80PIO(config, "pio0", 1229000));
	pio0.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio1(Z80PIO(config, "pio1", 1229000));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio2, 1229000);
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio2->in_pa_callback().set_ioport("DSW");

	I8272A(config, m_i8272, 16_MHz_XTAL / 2, true);
	m_i8272->drq_wr_callback().set("dma", FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, "i8272:0", p8k_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "i8272:1", p8k_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	BEEP(config, "beeper", 3250).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void p8k_state::p8k_16(machine_config &config)
{
	/* basic machine hardware */
	z8001_device &maincpu(Z8001(config, m_maincpu, XTAL(4'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &p8k_state::p8k_16_memmap);
	maincpu.set_addrmap(AS_DATA, &p8k_state::p8k_16_datamap);
	maincpu.set_addrmap(AS_IO, &p8k_state::p8k_16_iomap);
	maincpu.viack().set("p8k_16_daisy", FUNC(p8k_16_daisy_device::viack_r));

	P8K_16_DAISY(config, m_daisy, 0);
	m_daisy->set_daisy_config(p8k_16_daisy_chain);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 307200));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txcb_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxcb_w));

	/* peripheral hardware */
	z80ctc_device& ctc0(Z80CTC(config, "ctc0", XTAL(4'000'000)));
	ctc0.intr_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	z80ctc_device& ctc1(Z80CTC(config, "ctc1", XTAL(4'000'000)));
	ctc1.intr_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'000'000)));
	sio.out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	sio.out_int_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));

	z80sio_device& sio1(Z80SIO(config, "sio1", XTAL(4'000'000)));
	sio1.out_int_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	z80pio_device& pio0(Z80PIO(config, "pio0", XTAL(4'000'000)));
	pio0.out_int_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	z80pio_device& pio1(Z80PIO(config, "pio1", XTAL(4'000'000)));
	pio1.out_int_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	Z80PIO(config, m_pio2, XTAL(4'000'000));
	m_pio2->out_int_callback().set(FUNC(p8k_state::p8k_16_daisy_interrupt));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 3250).add_route(ALL_OUTPUTS, "mono", 0.5);
}

/* ROM definition */
ROM_START( p8000 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD("mon8_1_3.1",  0x0000, 0x1000, CRC(ad1bb118) SHA1(2332963acd74d5d1a009d9bce8a2b108de01d2a5))
	ROM_LOAD("mon8_2_3.1",  0x1000, 0x1000, CRC(daced7c2) SHA1(f1f778e72568961b448020fc543ed6e81bbe81b1))

	// this is for the p8000's terminal, not the p8000 itself
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("p8t_zs",    0x0000, 0x0800, CRC(f9321251) SHA1(a6a796b58d50ec4a416f2accc34bd76bc83f18ea))
	ROM_LOAD("p8tdzs.2",  0x0800, 0x0800, CRC(32736503) SHA1(6a1d7c55dddc64a7d601dfdbf917ce1afaefbb0a))

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "wdc4.2_1-2c43.bin", 0x0000, 0x1000, CRC(2646f1ee) SHA1(f62574ad57a2c8ac55c5df89256a707c0cafc0eb) )
	ROM_LOAD( "wdc4.2_2-5d66.bin", 0x1000, 0x1000, CRC(5d496b65) SHA1(42166d7ec51fce086c65ea829d8a3d63088815ca) )
ROM_END

ROM_START( p8000_16 )
	ROM_REGION16_BE( 0x4000, "maincpu", 0 )
	ROM_LOAD16_BYTE("mon16_1h_3.1_udos",   0x0000, 0x1000, CRC(0c3c28da) SHA1(0cd35444c615b404ebb9cf80da788593e573ddb5))
	ROM_LOAD16_BYTE("mon16_1l_3.1_udos",   0x0001, 0x1000, CRC(e8857bdc) SHA1(f89c65cbc479101130c71806fd3ddc28e6383f12))
	ROM_LOAD16_BYTE("mon16_2h_3.1_udos",   0x2000, 0x1000, CRC(cddf58d5) SHA1(588bad8df75b99580459c7a8e898a3396907e3a4))
	ROM_LOAD16_BYTE("mon16_2l_3.1_udos",   0x2001, 0x1000, CRC(395ee7aa) SHA1(d72fadb1608cd0915cd5ce6440897303ac5a12a6))

	// this is for the p8000's terminal, not the p8000 itself
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD("p8t_zs",    0x0000, 0x0800, CRC(f9321251) SHA1(a6a796b58d50ec4a416f2accc34bd76bc83f18ea))
	ROM_LOAD("p8tdzs.2",  0x0800, 0x0800, CRC(32736503) SHA1(6a1d7c55dddc64a7d601dfdbf917ce1afaefbb0a))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                   FULLNAME               FLAGS
COMP( 1989, p8000,    0,      0,      p8k,     p8k,   p8k_state, init_p8k,   "EAW electronic Treptow", "P8000 (8bit Board)",  MACHINE_NOT_WORKING)
COMP( 1989, p8000_16, p8000,  0,      p8k_16,  p8k,   p8k_state, empty_init, "EAW electronic Treptow", "P8000 (16bit Board)", MACHINE_NOT_WORKING)
