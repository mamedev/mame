// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ampro Little Z80 Board

2013-09-02 Skeleton driver.

Chips: Z80A @4MHz, Z80CTC, Z80DART, WD1770/2, NCR5380. Crystal: 16 MHz

This is a complete CP/M single-board computer that could be mounted
on top of a standard 13cm floppy drive. You needed to supply your own
power supply and serial terminal.

The later versions included a SCSI chip (NCR5380) enabling the use
of a hard drive of up to 88MB.

ToDo:
- (maybe) add scsi interface
- Add printer

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "softlist.h"

class ampro_state : public driver_device
{
public:
	ampro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dart(*this, "dart")
		, m_ctc (*this, "ctc")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	void init_ampro();
	DECLARE_MACHINE_RESET(ampro);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);

	void ampro(machine_config &config);
	void ampro_io(address_map &map);
	void ampro_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<z80dart_device> m_dart;
	required_device<z80ctc_device> m_ctc;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};

/*
d0..d3 Drive select 0-3 (we only emulate 1 drive)
d4     Side select 0=side0
d5     /DDEN
d6     Banking 0=rom
d7     FDC master clock 0=8MHz 1=16MHz (for 20cm disks, not emulated)
*/
WRITE8_MEMBER( ampro_state::port00_w )
{
	membank("bankr0")->set_entry(BIT(data, 6));
	m_fdc->dden_w(BIT(data, 5));
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(BIT(data, 4));
}

READ8_MEMBER( ampro_state::io_r )
{
	if (offset < 0x40)
		return m_ctc->read(space, offset>>4);
	else
		return m_dart->ba_cd_r(space, offset>>2);
}

WRITE8_MEMBER( ampro_state::io_w )
{
	if (offset < 0x40)
		m_ctc->write(space, offset>>4, data);
	else
		m_dart->ba_cd_w(space, offset>>2, data);
}

void ampro_state::ampro_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankr("bankr0").bankw("bankw0");
	map(0x1000, 0xffff).ram();
}

void ampro_state::ampro_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(this, FUNC(ampro_state::port00_w)); // system
	//AM_RANGE(0x01, 0x01) AM_WRITE(port01_w) // printer data
	//AM_RANGE(0x02, 0x03) AM_WRITE(port02_w) // printer strobe
	//AM_RANGE(0x20, 0x27) AM_READWRITE() // scsi chip
	//AM_RANGE(0x28, 0x28) AM_WRITE(port28_w) // scsi control
	//AM_RANGE(0x29, 0x29) AM_READ(port29_r) // ID port
	map(0x40, 0x8f).rw(this, FUNC(ampro_state::io_r), FUNC(ampro_state::io_w));
	map(0xc0, 0xc3).w(m_fdc, FUNC(wd1772_device::write));
	map(0xc4, 0xc7).r(m_fdc, FUNC(wd1772_device::read));
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

static void ampro_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

/* Input ports */
static INPUT_PORTS_START( ampro )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( ampro_state, ampro )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

void ampro_state::init_ampro()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

MACHINE_CONFIG_START(ampro_state::ampro)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",Z80, XTAL(16'000'000) / 4)
	MCFG_DEVICE_PROGRAM_MAP(ampro_mem)
	MCFG_DEVICE_IO_MAP(ampro_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain_intf)
	MCFG_MACHINE_RESET_OVERRIDE(ampro_state, ampro)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL(16'000'000) / 8) // 2MHz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("ctc", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc", z80ctc_device, trg1))

	/* Devices */
	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(16'000'000) / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE("dart", z80dart_device, txca_w))    // Z80DART Ch A, SIO Ch A
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("dart", z80dart_device, rxca_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE("dart", z80dart_device, rxtxcb_w))   // SIO Ch B

	MCFG_DEVICE_ADD("dart", Z80DART, XTAL(16'000'000) / 4)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("dart", z80dart_device, rxa_w))

	MCFG_WD1772_ADD("fdc", XTAL(16'000'000) / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ampro_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_SOFTWARE_LIST_ADD("flop_list", "ampro")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ampro )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "mntr", "Monitor")
	ROMX_LOAD( "mntr", 0x10000, 0x1000, CRC(d59d0909) SHA1(936410f414b1e71445253840eea0045545e4ff0b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "boot", "Boot")
	ROMX_LOAD( "boot", 0x10000, 0x1000, CRC(b3524046) SHA1(5466f7d28c1a04cfbf328095cb35ad1525e91f44), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "scsi", "SCSI Boot")
	ROMX_LOAD( "scsi", 0x10000, 0x1000, CRC(8eb20e5d) SHA1(0ab1ff65cf6d3c1a713a8ac5c1ee4c662ac3da0c), ROM_BIOS(3))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME            FLAGS
COMP( 1980, ampro, 0,      0,      ampro,   ampro, ampro_state, init_ampro, "Ampro", "Little Z80 Board", 0 )
