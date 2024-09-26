// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Altos 5-15

    ToDo:
    - When running MP/M, dir command crashes the system

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/clock.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

#include "softlist_dev.h"


namespace {

class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio0(*this, "pio0")
		, m_dma (*this, "dma")
		, m_fdc (*this, "fdc")
		, m_p_prom(*this, "proms")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_bankr(*this, "bankr%x", 0)
		, m_bankw(*this, "bankw%x", 0)
	{ }

	void altos5(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);
	uint8_t port08_r();
	uint8_t port09_r();
	void port08_w(uint8_t data);
	void port09_w(uint8_t data);
	void port14_w(uint8_t data);
	void setup_banks(uint8_t source);
	uint8_t convert(offs_t offset, bool state);
	void busreq_w(int state);
	void fdc_intrq_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_port08 = 0U;
	uint8_t m_port09 = 0U;
	bool m_ipl = 0;
	offs_t m_curr_bank = 0;
	floppy_image_device *m_floppy = 0;
	std::unique_ptr<u8[]> m_ram;  // main ram 192k
	std::unique_ptr<u8[]> m_dummy;  // for wrpt
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio0;
	required_device<z80dma_device> m_dma;
	required_device<fd1797_device> m_fdc;
	required_region_ptr<u8> m_p_prom;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_bank_array<16> m_bankr;
	required_memory_bank_array<16> m_bankw;
};


void altos5_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankr(m_bankr[0x0]).bankw(m_bankw[0x0]);
	map(0x1000, 0x1fff).bankr(m_bankr[0x1]).bankw(m_bankw[0x1]);
	map(0x2000, 0x2fff).bankr(m_bankr[0x2]).bankw(m_bankw[0x2]);
	map(0x3000, 0x3fff).bankr(m_bankr[0x3]).bankw(m_bankw[0x3]);
	map(0x4000, 0x4fff).bankr(m_bankr[0x4]).bankw(m_bankw[0x4]);
	map(0x5000, 0x5fff).bankr(m_bankr[0x5]).bankw(m_bankw[0x5]);
	map(0x6000, 0x6fff).bankr(m_bankr[0x6]).bankw(m_bankw[0x6]);
	map(0x7000, 0x7fff).bankr(m_bankr[0x7]).bankw(m_bankw[0x7]);
	map(0x8000, 0x8fff).bankr(m_bankr[0x8]).bankw(m_bankw[0x8]);
	map(0x9000, 0x9fff).bankr(m_bankr[0x9]).bankw(m_bankw[0x9]);
	map(0xa000, 0xafff).bankr(m_bankr[0xa]).bankw(m_bankw[0xa]);
	map(0xb000, 0xbfff).bankr(m_bankr[0xb]).bankw(m_bankw[0xb]);
	map(0xc000, 0xcfff).bankr(m_bankr[0xc]).bankw(m_bankw[0xc]);
	map(0xd000, 0xdfff).bankr(m_bankr[0xd]).bankw(m_bankw[0xd]);
	map(0xe000, 0xefff).bankr(m_bankr[0xe]).bankw(m_bankw[0xe]);
	map(0xf000, 0xffff).bankr(m_bankr[0xf]).bankw(m_bankw[0xf]);
}

void altos5_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x04, 0x07).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x08, 0x0b).rw(m_pio0, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x17).w(FUNC(altos5_state::port14_w));
	map(0x1c, 0x1f).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	//map(0x20, 0x23) // Hard drive
	map(0x2c, 0x2f).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

uint8_t altos5_state::convert(offs_t offset, bool state)
{
	uint8_t data = m_p_prom[offset];

	// if IPL and /A12, point at rom
	if (!state && m_ipl && !BIT(offset, 0))
		data = 0x31;
	else
	// if WPRT point at nothing
	if (state && BIT(data, 7))
		data = 0x30;

	// mask off wprt (no longer needed)
	// normalise bank number (4x becomes 0x; 2x and 1x are already ok)
	return data & 0x3f;
}

void altos5_state::setup_banks(uint8_t source)
{
	offs_t offs,temp;
	// WPRT | template | dma bank / cpu bank

	if (source == 1) // use DMA banks only if BUSACK is asserted
		offs = ((bitswap<8>(m_port09, 0, 0, 0, 5, 1, 2, 7, 6) << 4) & 0x1f0) ^ 0x100;
	else
		offs = ((bitswap<8>(m_port09, 0, 0, 0, 5, 1, 2, 4, 3) << 4) & 0x1f0) ^ 0x100;

	temp = offs;
	if ((source == 2) || (temp != m_curr_bank))
	{
		for (auto &bank : m_bankr)
			bank->set_entry(convert(offs++, 0));

		offs = temp;
		for (auto &bank : m_bankw)
			bank->set_entry(convert(offs++, 1));
	}

	m_curr_bank = temp;
}

void altos5_state::machine_reset()
{
	m_curr_bank = 0;
	m_port08 = 0;
	m_port09 = 0;
	m_ipl = 1;
	setup_banks(2);
	m_floppy = nullptr;
	m_maincpu->reset();
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio0" },
	{ "pio1" },
	{ "ctc" },
	{ "dart" },
	{ "sio" },
	{ nullptr }
};


// turns off IPL mode, removes boot rom from memory map
void altos5_state::port14_w(uint8_t data)
{
	m_ipl = 0;
	setup_banks(2);
}

uint8_t altos5_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void altos5_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

uint8_t altos5_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void altos5_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

void altos5_state::busreq_w(int state)
{
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	setup_banks(state); // adjust banking for dma or cpu
}

/*
d0: L = a HD is present
d1: L = a 2nd hard drive is present
d2: unused configuration input (must be H to skip HD boot)
d3: selected floppy is single(L) or double sided(H)
d7: IRQ from FDC
*/
uint8_t altos5_state::port08_r()
{
	uint8_t data = m_port08 | 0x87;
	if (m_floppy)
		data |= ((uint8_t)m_floppy->twosid_r() << 3); // get number of sides
	return data;
}

/*
d0: HD IRQ
*/
uint8_t altos5_state::port09_r()
{
	return m_port09 & 0xfe;
}

/*
d4: DDEN (H = double density)
d5: DS (H = drive 2)
d6: SS (H = side 2)
*/
void altos5_state::port08_w(uint8_t data)
{
	m_port08 = data & 0x70;

	m_floppy = nullptr;
	if (BIT(data, 5))
		m_floppy = m_floppy1->get_device();
	else
		m_floppy = m_floppy0->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(BIT(data, 6));
		m_fdc->dden_w(!BIT(data, 4));
	}
}

/*
d1, 2: Memory Map template selection (0 = diag; 1 = oasis; 2 = mp/m)
d3, 4: CPU bank select
d5:    H = Write protect of common area
d6, 7: DMA bank select (not emulated)
*/
void  altos5_state::port09_w(uint8_t data)
{
	m_port09 = data;
	setup_banks(2);
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(altos5_state::quickload_cb)
{
	setup_banks(2);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	// Avoid loading a program if CP/M-80 is not in memory
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");
	}

	const int mem_avail = 256 * prog_space.read_byte(7) + prog_space.read_byte(6) - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		prog_space.write_byte(i + 0x100, data);
	}

	// clear out command tail
	prog_space.write_byte(0x80, 0);
	prog_space.write_byte(0x81, 0);

	// Roughly set SP basing on the BDOS position
	m_maincpu->set_state_int(Z80_SP, mem_avail + 384);
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}


static void altos5_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void altos5_state::fdc_intrq_w(int state)
{
	uint8_t data = m_port08 | ((uint8_t)(state) << 7);
	m_pio0->port_a_write(data);
}

void altos5_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x30000);
	m_dummy = std::make_unique<u8[]>(0x1000);

	u8 *r = m_ram.get();
	u8 *d = m_dummy.get();
	u8 *m = memregion("maincpu")->base();

	for (auto &bank : m_bankr)
		bank->configure_entries(0, 48, r, 0x1000);
	for (auto &bank : m_bankw)
		bank->configure_entries(0, 48, r, 0x1000);
	for (auto &bank : m_bankr)
		bank->configure_entry(48, d);
	for (auto &bank : m_bankw)
		bank->configure_entry(48, d);
	for (auto &bank : m_bankr)
		bank->configure_entry(49, m);
	for (auto &bank : m_bankw)
		bank->configure_entry(49, m);

	save_pointer(NAME(m_ram), 0x30000);
	save_item(NAME(m_port08));
	save_item(NAME(m_port09));
	save_item(NAME(m_ipl));
	save_item(NAME(m_curr_bank));
}

void altos5_state::altos5(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &altos5_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &altos5_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	/* devices */
	Z80DMA(config, m_dma, 8_MHz_XTAL / 2);
	m_dma->out_busreq_callback().set(FUNC(altos5_state::busreq_w));
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// BAO, not used
	m_dma->in_mreq_callback().set(FUNC(altos5_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(altos5_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(altos5_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(altos5_state::io_write_byte));

	Z80PIO(config, m_pio0, 8_MHz_XTAL / 2);
	m_pio0->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio0->in_pa_callback().set(FUNC(altos5_state::port08_r));
	m_pio0->out_pa_callback().set(FUNC(altos5_state::port08_w));
	m_pio0->in_pb_callback().set(FUNC(altos5_state::port09_r));
	m_pio0->out_pb_callback().set(FUNC(altos5_state::port09_w));

	z80pio_device& pio1(Z80PIO(config, "pio1", 8_MHz_XTAL / 2));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device& dart(Z80DART(config, "dart", 8_MHz_XTAL / 2));
	// Channel A - console #3
	// Channel B - printer
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device& sio(Z80SIO(config, "sio", 8_MHz_XTAL / 2));
	// Channel A - console #2
	// WRDY connects to (altos5_state, fdc_intrq_w)
	// Channel B - console #1
	sio.out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	sio.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 8_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(8_MHz_XTAL / 4); // 2MHz
	ctc.set_clk<1>(8_MHz_XTAL / 4); // 2MHz
	ctc.set_clk<2>(8_MHz_XTAL / 4); // 2MHz
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::rxtxcb_w));    // SIO Ch B
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));    // Z80DART Ch A, SIO Ch A
	ctc.zc_callback<1>().append("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().append("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<1>().append("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));  // Z80DART Ch B

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232.dcd_handler().set("sio", FUNC(z80sio_device::dcdb_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));

	FD1797(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(altos5_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", altos5_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", altos5_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("altos5");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(altos5_state::quickload_cb));
}


/* ROM definition */
ROM_START( altos5 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("2732.bin",   0x0000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9)) // bios

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("82s141.bin", 0x0000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9)) // banking control
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME      FLAGS */
COMP( 1982, altos5, 0,      0,      altos5,  altos5, altos5_state, empty_init, "Altos", "Altos 5-15", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
