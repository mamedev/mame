// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CZK-80

2010-08-30 Skeleton driver
2010-11-27 Connected to a terminal
2014-01-08 Added devices

Only known info: http://forum.z80.de/showtopic.php?threadid=280

On main board there are Z80A CPU, Z80A PIO, Z80A DART and Z80A CTC
   there is 8K ROM and XTAL 16MHz
   Two undumped proms, AM27S20DC and D3631-1.
FDC board contains Z80A DMA and NEC 765A (XTAL on it is 8MHZ)
Mega board contains 74LS612 and memory chips (32x 41256)

Status:
  It prints 2 lines of text, then waits for a floppy.

ToDo:
- Everything... no diagram or manuals, so EVERYTHING below is guesswork.
- Need software

I/O ports: These ranges are what is guessed
  40 : rom switching
  4c-4F : PIO
  50-53 : DART
  54-57 : CTC
  80-81 : Parallel port (no programming bytes are sent, so it isn't a device)
  C0-C1 : FDC
  It is not known what address is used by:
  - the DMA
  - the Motor-on signal(s)
  as there are no unknown writes.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/terminal.h"


class czk80_state : public driver_device
{
public:
	czk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_fdc(*this, "fdc")
	{ }

	void czk80(machine_config &config);
	void init_czk80();

protected:
	virtual void machine_reset() override;

private:
	TIMER_CALLBACK_MEMBER(czk80_reset);
	DECLARE_READ8_MEMBER(port80_r);
	DECLARE_READ8_MEMBER(port81_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	void kbd_put(u8 data);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	void czk80_io(address_map &map);
	void czk80_mem(address_map &map);
	uint8_t m_term_data;
	required_device<z80_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<upd765a_device> m_fdc;
};


WRITE8_MEMBER( czk80_state::port40_w )
{
	membank("bankr1")->set_entry(BIT(data, 1));
}

READ8_MEMBER( czk80_state::port80_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( czk80_state::port81_r )
{
	return (m_term_data) ? 3 : 1;
}

void czk80_state::czk80_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bankr0").bankw("bankw0");
	map(0x2000, 0xdfff).ram();
	map(0xe000, 0xffff).bankr("bankr1").bankw("bankw1");
}

void czk80_state::czk80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(FUNC(czk80_state::port40_w));
	map(0x4c, 0x4f).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw("dart", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0x54, 0x57).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	// 80, 81 - no setup bytes
	map(0x80, 0x80).r(FUNC(czk80_state::port80_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x81, 0x81).r(FUNC(czk80_state::port81_r));
	map(0xc0, 0xc1).m(m_fdc, FUNC(upd765a_device::map));
}

/* Input ports */
static INPUT_PORTS_START( czk80 )
INPUT_PORTS_END

/* Z80 Daisy Chain */

static const z80_daisy_config daisy_chain[] =
{
	{ "pio" },
	{ "dart" },
	{ "ctc" },
	{ nullptr }
};

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( czk80_state::ctc_z0_w )
{
// guess this generates clock for z80dart
}

WRITE_LINE_MEMBER( czk80_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( czk80_state::ctc_z2_w )
{
}

/* after the first 4 bytes have been read from ROM, switch the ram back in */
TIMER_CALLBACK_MEMBER( czk80_state::czk80_reset)
{
	membank("bankr0")->set_entry(1);
}

void czk80_state::machine_reset()
{
	machine().scheduler().timer_set(attotime::from_usec(3), timer_expired_delegate(FUNC(czk80_state::czk80_reset),this));
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	membank("bankr1")->set_entry(0); // point at rom
	membank("bankw1")->set_entry(0); // always write to ram
}

void czk80_state::init_czk80()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);

	membank("bankr1")->configure_entry(1, &main[0xe000]);
	membank("bankr1")->configure_entry(0, &main[0x10000]);
	membank("bankw1")->configure_entry(0, &main[0xe000]);
}

static void czk80_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void czk80_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void czk80_state::czk80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &czk80_state::czk80_mem);
	m_maincpu->set_addrmap(AS_IO, &czk80_state::czk80_io);
	m_maincpu->set_daisy_config(daisy_chain);

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(czk80_state::kbd_put));
	UPD765A(config, m_fdc, 8_MHz_XTAL, true, true);
	FLOPPY_CONNECTOR(config, "fdc:0", czk80_floppies, "525dd", floppy_image_device::default_floppy_formats);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(czk80_state::ctc_z0_w));
	ctc.zc_callback<1>().set(FUNC(czk80_state::ctc_z1_w));
	ctc.zc_callback<2>().set(FUNC(czk80_state::ctc_z2_w));

	z80dart_device& dart(Z80DART(config, "dart", 16_MHz_XTAL / 4));
	//dart.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	//dart.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	//dart.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	dart.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	z80pio_device& pio(Z80PIO(config, "pio", 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


/* ROM definition */
ROM_START( czk80 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "czk80.rom", 0x10000, 0x2000, CRC(7081b7c6) SHA1(13f75b14ea73b252bdfa2384e6eead6e720e49e3))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 198?, czk80, 0,      0,      czk80,   czk80, czk80_state, init_czk80, "<unknown>", "CZK-80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
