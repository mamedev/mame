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
#include "machine/upd765.h"
#include "machine/mc2661.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "softlist.h"

class dps1_state : public driver_device
{
public:
	dps1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		//, m_floppy1(*this, "fdc:1")
	{ }

	void dps1(machine_config &config);

	void init_dps1();

protected:
	virtual void machine_reset() override;

private:
	DECLARE_WRITE8_MEMBER(portb2_w);
	DECLARE_WRITE8_MEMBER(portb4_w);
	DECLARE_WRITE8_MEMBER(portb6_w);
	DECLARE_WRITE8_MEMBER(portb8_w);
	DECLARE_WRITE8_MEMBER(portba_w);
	DECLARE_WRITE8_MEMBER(portbc_w);
	DECLARE_WRITE8_MEMBER(portbe_w);
	DECLARE_READ8_MEMBER(portff_r);
	DECLARE_WRITE8_MEMBER(portff_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	void io_map(address_map &map);
	void mem_map(address_map &map);
	bool m_dma_dir;
	uint16_t m_dma_adr;
	required_device<cpu_device> m_maincpu;
	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	//required_device<floppy_connector> m_floppy1;
};

void dps1_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).bankr("bankr0").bankw("bankw0");
	map(0x0400, 0xffff).ram();
}

void dps1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("uart", FUNC(mc2661_device::read), FUNC(mc2661_device::write)); // S2651
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
	// AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("uart2", mc2661_device, read, write) // S2651
	// AM_RANGE(0x08, 0x0b) parallel ports
	// AM_RANGE(0x10, 0x11) // interrupt response
	map(0x14, 0x14).rw("am9519a", FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x15, 0x15).rw("am9519a", FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x16, 0x16).rw("am9519b", FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x17, 0x17).rw("am9519b", FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	// AM_RANGE(0x18, 0x1f) control lines 0 to 7
	map(0xe0, 0xe3).noprw(); //unknown device
}


// read from disk, to memory
WRITE8_MEMBER( dps1_state::portb2_w )
{
	m_dma_dir = 1;
}

// write to disk, from memory
WRITE8_MEMBER( dps1_state::portb4_w )
{
	m_dma_dir = 0;
}

// enable eprom
WRITE8_MEMBER( dps1_state::portb6_w )
{
	membank("bankr0")->set_entry(1); // point at rom
}

// set A16-23
WRITE8_MEMBER( dps1_state::portb8_w )
{
}

// set A8-15
WRITE8_MEMBER( dps1_state::portba_w )
{
	m_dma_adr = (data << 8) | (m_dma_adr & 0xff);
}

// set A0-7
WRITE8_MEMBER( dps1_state::portbc_w )
{
	m_dma_adr = (m_dma_adr & 0xff00) | data;
}

// disable eprom
WRITE8_MEMBER( dps1_state::portbe_w )
{
	membank("bankr0")->set_entry(0); // point at ram
}

// read 8 front-panel switches
READ8_MEMBER( dps1_state::portff_r )
{
	return 0x0e;
}

// write to 8 leds
WRITE8_MEMBER( dps1_state::portff_w )
{
}

// do dma
WRITE_LINE_MEMBER( dps1_state::fdc_drq_w )
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

void dps1_state::machine_reset()
{
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	// set fdc for 8 inch floppies
	m_fdc->set_rate(500000);
	// turn on the motor
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
}

void dps1_state::init_dps1()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x0400]);
	membank("bankw0")->configure_entry(0, &main[0x0400]);
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
	mc2661_device &uart(MC2661(config, "uart", 5.0688_MHz_XTAL)); // Signetics 2651N
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(uart, FUNC(mc2661_device::rx_w));
	rs232.dsr_handler().set(uart, FUNC(mc2661_device::dsr_w));
	rs232.cts_handler().set(uart, FUNC(mc2661_device::cts_w));

	AM9519(config, "am9519a", 0);
	AM9519(config, "am9519b", 0);

	// floppy
	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, false, true);
	//m_fdc->intrq_wr_callback().set(FUNC(dps1_state::fdc_int_w)); // doesn't appear to be used
	m_fdc->drq_wr_callback().set(FUNC(dps1_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", floppies, "floppy0", floppy_image_device::default_floppy_formats).enable_sound(true);
	//FLOPPY_CONNECTOR(config, "fdc:1", floppies, "floppy1", floppy_image_device::default_floppy_formats).enable_sound(true);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("dps1");
}

ROM_START( dps1 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "boot 1280", 0x000, 0x400, CRC(9c2e98fa) SHA1(78e6c9d00aa6e8f6c4d3c65984cfdf4e99434c66) ) // actually on the FDC-2 board
ROM_END

COMP( 1979, dps1, 0, 0, dps1, dps1, dps1_state, init_dps1, "Ithaca InterSystems", "DPS-1", MACHINE_NO_SOUND_HW )
