// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/****************************************************************************************************************

Ithaca Intersystems DPS-1

The last commercial release of a computer fitted with a front panel.

It needs to boot from floppy before anything appears on screen.

ToDo:
- Need artwork of the front panel switches and LEDs, and port FF.

***************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/am9519.h"
#include "machine/scn_pci.h"
#include "machine/upd765.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "softlist_dev.h"


namespace {

class dps1_state : public driver_device
{
public:
	dps1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		//, m_floppy1(*this, "fdc:1")
	{ }

	void dps1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void portb2_w(u8 data);
	void portb4_w(u8 data);
	void portb6_w(u8 data);
	void portb8_w(u8 data);
	void portba_w(u8 data);
	void portbc_w(u8 data);
	void portbe_w(u8 data);
	u8 portff_r();
	void portff_w(u8 data);
	void fdc_drq_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	bool m_dma_dir = 0;
	u16 m_dma_adr = 0U;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	//required_device<floppy_connector> m_floppy1;
};

void dps1_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share(m_ram);
	map(0x0000, 0x03ff).bankr(m_bank1);
}

void dps1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("uart", FUNC(scn2651_device::read), FUNC(scn2651_device::write));
	map(0xb0, 0xb1).m(m_fdc, FUNC(upd765_family_device::map));
	map(0xb2, 0xb3).w(FUNC(dps1_state::portb2_w)); // set dma fdc->memory
	map(0xb4, 0xb5).w(FUNC(dps1_state::portb4_w)); // set dma memory->fdc
	map(0xb6, 0xb7).w(FUNC(dps1_state::portb6_w)); // enable eprom
	map(0xb8, 0xb9).w(FUNC(dps1_state::portb8_w)); // set A16-23
	map(0xba, 0xbb).w(FUNC(dps1_state::portba_w)); // set A8-15
	map(0xbc, 0xbd).w(FUNC(dps1_state::portbc_w)); // set A0-7
	map(0xbe, 0xbf).w(FUNC(dps1_state::portbe_w)); // disable eprom
	map(0xff, 0xff).rw(FUNC(dps1_state::portff_r), FUNC(dps1_state::portff_w));
	// other allocated ports, optional
	// map(0x04, 0x07).rw("uart2", FUNC(scn2651_device::read), FUNC(scn2651_device::write));
	// map(0x08, 0x0b) parallel ports
	// map(0x10, 0x11) // interrupt response
	map(0x14, 0x14).rw("am9519a", FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x15, 0x15).rw("am9519a", FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x16, 0x16).rw("am9519b", FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x17, 0x17).rw("am9519b", FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	// map(0x18, 0x1f) control lines 0 to 7
	map(0xe0, 0xe3).noprw(); //unknown device
}


// read from disk, to memory
void dps1_state::portb2_w(u8 data)
{
	m_dma_dir = 1;
}

// write to disk, from memory
void dps1_state::portb4_w(u8 data)
{
	m_dma_dir = 0;
}

// enable eprom
void dps1_state::portb6_w(u8 data)
{
	m_bank1->set_entry(1);
}

// set A16-23
void dps1_state::portb8_w(u8 data)
{
}

// set A8-15
void dps1_state::portba_w(u8 data)
{
	m_dma_adr = (data << 8) | (m_dma_adr & 0xff);
}

// set A0-7
void dps1_state::portbc_w(u8 data)
{
	m_dma_adr = (m_dma_adr & 0xff00) | data;
}

// disable eprom
void dps1_state::portbe_w(u8 data)
{
	m_bank1->set_entry(0);
}

// read 8 front-panel switches
u8 dps1_state::portff_r()
{
	return 0x0e;
}

// write to 8 leds
void dps1_state::portff_w(u8 data)
{
}

// do dma
void dps1_state::fdc_drq_w(int state)
{
	if (state)
	{
		// acknowledge drq by taking /dack low (unsupported)
		// then depending on direction, transfer a byte
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		if (m_dma_dir)
		{ // disk to mem
			mem.write_byte(m_dma_adr, m_fdc->dma_r());
		}
		else
		{ // mem to disk
			m_fdc->dma_w(mem.read_byte(m_dma_adr));
		}
		m_dma_adr++;
	}
	// else take /dack high (unsupported)
}

void dps1_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, m_rom);

	save_item(NAME(m_dma_dir));
	save_item(NAME(m_dma_adr));
}

void dps1_state::machine_reset()
{
	m_bank1->set_entry(1);
	// set fdc for 8 inch floppies
	m_fdc->set_rate(500000);
	// turn on the motor
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
}

static INPUT_PORTS_START( dps1 )
INPUT_PORTS_END

static void floppies(device_slot_interface &device)
{
	device.option_add("floppy0", FLOPPY_8_DSDD);
}

void dps1_state::dps1(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dps1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dps1_state::io_map);

	/* video hardware */
	scn2651_device &uart(SCN2651(config, "uart", 5.0688_MHz_XTAL)); // Signetics 2651N
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(uart, FUNC(scn2651_device::rxd_w));
	rs232.dsr_handler().set(uart, FUNC(scn2651_device::dsr_w));
	rs232.cts_handler().set(uart, FUNC(scn2651_device::cts_w));

	AM9519(config, "am9519a", 0);
	AM9519(config, "am9519b", 0);

	// floppy
	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, false, true);
	//m_fdc->intrq_wr_callback().set(FUNC(dps1_state::fdc_int_w)); // doesn't appear to be used
	m_fdc->drq_wr_callback().set(FUNC(dps1_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "floppy0", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	//FLOPPY_CONNECTOR(config, "fdc:1", floppies, "floppy1", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("dps1");
}

ROM_START( dps1 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "boot 1280", 0x0000, 0x0400, CRC(9c2e98fa) SHA1(78e6c9d00aa6e8f6c4d3c65984cfdf4e99434c66) ) // actually on the FDC-2 board
ROM_END

} // anonymous namespace


COMP( 1979, dps1, 0, 0, dps1, dps1, dps1_state, empty_init, "Ithaca InterSystems", "DPS-1", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
