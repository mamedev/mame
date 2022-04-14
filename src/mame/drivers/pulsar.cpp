// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Pulsar Little Big Board (6000 series)

2013-12-29 Skeleton driver.

Chips: Z80A @4MHz, Z80DART, FD1797-02, 8255A-5, AY-5-8116, MSM5832.
Crystals: 4 MHz, 5.0688 MHz, 32768.

This is a complete CP/M single-board computer. You needed to supply your own
power supply and serial terminal.

The terminal must be set for 9600 baud, 7 bits, even parity, 1 stop bit.


ToDo:
- Fix floppy. It needs to WAIT the cpu whenever port 0xD3 is read, wait
  for either DRQ or INTRQ to assert, then release the cpu and then do the
  actual port read. Our Z80 cannot do that.
- Fix FDC so MAME doesn't crash when a certain disk is inserted.


Monitor Commands:
B - Boot from disk
D - Dump memory
F - Fill memory
G - Go
I - In port
L - Load bootstrap from drive A to 0x80
M - Modify memory
O - Out port
P - choose which rs232 channel for the console
T - Test memory
V - Move memory
X - Test off-board memory banks

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/com8116.h"
#include "machine/i8255.h"
#include "machine/msm5832.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"


namespace {

class pulsar_state : public driver_device
{
public:
	pulsar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_rtc(*this, "rtc")
	{ }

	void pulsar(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;

	void ppi_pa_w(u8 data);
	void ppi_pb_w(u8 data);
	void ppi_pc_w(u8 data);
	u8 ppi_pc_r();

	void io_map(address_map &map);
	void mem_map(address_map &map);

	floppy_image_device *m_floppy;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<msm5832_device> m_rtc;
};

void pulsar_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xf800, 0xffff).bankr("bank1");
}

void pulsar_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xc0, 0xc3).mirror(0x0c).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0xd0, 0xd3).mirror(0x0c).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xe0, 0xe3).mirror(0x0c).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf0, 0xf0).mirror(0x0f).w("brg", FUNC(com8116_device::stt_str_w));
}


/*
d0..d3 Drive select 0-3 (we only emulate 1 drive)
d4     Side select 0=side0
d5     /DDEN
d6     /DSK_WAITEN (don't know what this is, not emulated)
d7     XMEMEX line (for external memory, not emulated)
*/
void pulsar_state::ppi_pa_w(u8 data)
{
	m_floppy = nullptr;
	if (BIT(data, 0))
		m_floppy = m_floppy0->get_device();
	else
	if (BIT(data, 1))
		m_floppy = m_floppy1->get_device();
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(BIT(data, 5));
	if (m_floppy)
		m_floppy->mon_w(0);
}

/*
d0..d3 RTC address
d4     RTC read line
d5     RTC write line
d6     RTC hold line
d7     Allow 64k of ram
*/
void pulsar_state::ppi_pb_w(u8 data)
{
	m_rtc->address_w(data & 0x0f);
	m_rtc->read_w(BIT(data, 4));
	m_rtc->write_w(BIT(data, 5));
	m_rtc->hold_w(BIT(data, 6));
	m_bank1->set_entry(BIT(data, 7));
}

// d0..d3 Data lines to rtc
void pulsar_state::ppi_pc_w(u8 data)
{
	m_rtc->data_w(data & 15);
}

// d7     /2 SIDES
u8 pulsar_state::ppi_pc_r()
{
	uint8_t data = 0;
	if (m_floppy)
		data = m_floppy->twosid_r() << 7;
	return m_rtc->data_r() | data;
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dart" },
	{ nullptr }
};

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static void pulsar_floppies(device_slot_interface &device)
{
	device.option_add("flop", FLOPPY_8_DSDD);
}

/* Input ports */
static INPUT_PORTS_START( pulsar )
INPUT_PORTS_END

void pulsar_state::machine_reset()
{

	m_bank1->set_entry(1);
	m_rtc->cs_w(1); // always enabled

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall ram over the rom shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void pulsar_state::machine_start()
{
	// register for savestates
	m_bank1->configure_entry(0, m_ram+0xf800);
	m_bank1->configure_entry(1, m_rom);
}

void pulsar_state::pulsar(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pulsar_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pulsar_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);


	/* Devices */
	i8255_device &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(pulsar_state::ppi_pa_w));
	ppi.out_pb_callback().set(FUNC(pulsar_state::ppi_pb_w));
	ppi.in_pc_callback().set(FUNC(pulsar_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(pulsar_state::ppi_pc_w));

	MSM5832(config, "rtc", 32.768_kHz_XTAL);

	z80dart_device& dart(Z80DART(config, "dart", 4_MHz_XTAL));
	dart.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	dart.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	dart.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("dart", FUNC(z80dart_device::rxa_w));
	rs232.cts_handler().set("dart", FUNC(z80dart_device::ctsa_w));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	com8116_device &brg(COM8116(config, "brg", 5.0688_MHz_XTAL));
	// Schematic has the labels for FT and FR the wrong way around, but the pin numbers are correct.
	brg.fr_handler().set("dart", FUNC(z80dart_device::txca_w));
	brg.fr_handler().append("dart", FUNC(z80dart_device::rxca_w));
	brg.ft_handler().set("dart", FUNC(z80dart_device::txcb_w));
	brg.ft_handler().append("dart", FUNC(z80dart_device::rxcb_w));

	FD1797(config, m_fdc, 4_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "fdc:0", pulsar_floppies, "flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pulsar_floppies, "flop", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

/* ROM definition */
ROM_START( pulsarlb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "mon7", "MP7A")
	ROMX_LOAD( "mp7a.u2",   0x0000, 0x0800, CRC(726b8a19) SHA1(43b2af84d5622c1f67584c501b730acf002a6113), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mon6", "LBOOT6") // Blank screen until floppy boots
	ROMX_LOAD( "lboot6.u2", 0x0000, 0x0800, CRC(3bca9096) SHA1(ff99288e51a9e832785ce8e3ab5a9452b1064231), ROM_BIOS(1))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY   FULLNAME                          FLAGS
COMP( 1981, pulsarlb, 0,      0,      pulsar,  pulsar, pulsar_state, empty_init, "Pulsar", "Little Big Board (6000 series)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

