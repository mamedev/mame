// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CKZ-80

2010-08-30 Skeleton driver
2010-11-27 Connected to a terminal
2014-01-08 Added devices

Only known info: http://forumcpm.gaby.de/oldboard/showtopic0adb.html?threadid=280

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
#include "machine/terminal.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


namespace {

class ckz80_state : public driver_device
{
public:
	ckz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_terminal(*this, "terminal")
		, m_fdc(*this, "fdc")
	{ }

	void ckz80(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	u8 port80_r();
	u8 port81_r();
	void port40_w(u8 data);
	void kbd_put(u8 data);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	void io_map(address_map &map);
	void mem_map(address_map &map);
	u8 m_term_data = 0U;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_device<generic_terminal_device> m_terminal;
	required_device<upd765a_device> m_fdc;
};


void ckz80_state::port40_w(u8 data)
{
	m_bank1->set_entry(BIT(~data, 1));
}

u8 ckz80_state::port80_r()
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

u8 ckz80_state::port81_r()
{
	return (m_term_data) ? 3 : 1;
}

void ckz80_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xe000, 0xffff).bankr("bank1");
}

void ckz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(FUNC(ckz80_state::port40_w));
	map(0x4c, 0x4f).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw("dart", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0x54, 0x57).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	// 80, 81 - no setup bytes
	map(0x80, 0x80).r(FUNC(ckz80_state::port80_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x81, 0x81).r(FUNC(ckz80_state::port81_r));
	map(0xc0, 0xc1).m(m_fdc, FUNC(upd765a_device::map));
}

/* Input ports */
static INPUT_PORTS_START( ckz80 )
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

WRITE_LINE_MEMBER( ckz80_state::ctc_z0_w )
{
// guess this generates clock for z80dart
}

WRITE_LINE_MEMBER( ckz80_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( ckz80_state::ctc_z2_w )
{
}

void ckz80_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram+0xe000);
	m_bank1->configure_entry(1, m_rom);
	save_item(NAME(m_term_data));
}

void ckz80_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x1fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xe000, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x1fff, m_ram);
				}
			},
			&m_rom_shadow_tap);

	m_bank1->set_entry(1);
}

static void ckz80_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void ckz80_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void ckz80_state::ckz80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ckz80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ckz80_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(ckz80_state::kbd_put));
	UPD765A(config, m_fdc, 8_MHz_XTAL, true, true);
	FLOPPY_CONNECTOR(config, "fdc:0", ckz80_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(ckz80_state::ctc_z0_w));
	ctc.zc_callback<1>().set(FUNC(ckz80_state::ctc_z1_w));
	ctc.zc_callback<2>().set(FUNC(ckz80_state::ctc_z2_w));

	z80dart_device& dart(Z80DART(config, "dart", 16_MHz_XTAL / 4));
	//dart.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	//dart.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	//dart.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	dart.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);

	z80pio_device& pio(Z80PIO(config, "pio", 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


/* ROM definition */
ROM_START( ckz80 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "ckz80.rom", 0x0000, 0x2000, CRC(7081b7c6) SHA1(13f75b14ea73b252bdfa2384e6eead6e720e49e3))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 198?, ckz80, 0,      0,      ckz80,   ckz80, ckz80_state, empty_init, "<unknown>", "CKZ-80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
