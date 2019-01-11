// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Regnecentralen Piccolo RC702

2016-09-10 Skeleton driver

Undumped prom at IC55 type 74S287
Keyboard has 8048 and 2758, both undumped.

ToDo:
- Everything

Issues:
- Floppy disc error.


****************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/7474.h"
#include "machine/am9517a.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "video/i8275.h"

#include "screen.h"
#include "speaker.h"



class rc702_state : public driver_device
{
public:
	rc702_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_ctc1(*this, "ctc1")
		, m_pio(*this, "pio")
		, m_dma(*this, "dma")
		, m_beep(*this, "beeper")
		, m_7474(*this, "7474")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{
	}

	void init_rc702();
	DECLARE_MACHINE_RESET(rc702);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_WRITE8_MEMBER(port1c_w);
	DECLARE_WRITE_LINE_MEMBER(crtc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_WRITE_LINE_MEMBER(q_w);
	DECLARE_WRITE_LINE_MEMBER(qbar_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	void kbd_put(u8 data);

	void rc702(machine_config &config);
	void rc702_io(address_map &map);
	void rc702_mem(address_map &map);
private:
	bool m_q_state;
	bool m_qbar_state;
	bool m_drq_state;
	uint16_t m_beepcnt;
	uint8_t m_dack;
	bool m_tc;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80pio_device> m_pio;
	required_device<am9517a_device> m_dma;
	required_device<beep_device> m_beep;
	required_device<ttl7474_device> m_7474;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
};


void rc702_state::rc702_mem(address_map &map)
{
	map(0x0000, 0x07ff).bankr("bankr0").bankw("bankw0");
	map(0x0800, 0xffff).ram();
}

void rc702_state::rc702_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x01).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x04, 0x05).m(m_fdc, FUNC(upd765a_device::map));
	map(0x08, 0x0b).rw("sio1", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w)); // boot sequence doesn't program this
	map(0x0c, 0x0f).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x17).portr("DSW").w(this, FUNC(rc702_state::port14_w)); // motors
	map(0x18, 0x1b).w(this, FUNC(rc702_state::port18_w)); // memory banking
	map(0x1c, 0x1f).w(this, FUNC(rc702_state::port1c_w)); // sound
	map(0xf0, 0xff).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
}

/* Input ports */
static INPUT_PORTS_START( rc702 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Minifloppy") // also need to switch frequencies to fdc
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END

MACHINE_RESET_MEMBER( rc702_state, rc702 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_beepcnt = 0xffff;
	m_dack = 0;
	m_tc = 0;
	m_7474->preset_w(1);
	m_fdc->set_ready_line_connected(1); // always ready for minifloppy; controlled by fdc for 20cm
	m_fdc->set_unscaled_clock(4000000); // 4MHz for minifloppy; 8MHz for 20cm
	m_maincpu->reset();
}

WRITE_LINE_MEMBER( rc702_state::q_w )
{
	m_q_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);
}

WRITE_LINE_MEMBER( rc702_state::qbar_w )
{
	m_qbar_state = state;

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

WRITE_LINE_MEMBER( rc702_state::crtc_drq_w )
{
	m_drq_state = state;

	if (m_q_state && m_drq_state)
		m_dma->dreq3_w(1);
	else
		m_dma->dreq3_w(0);

	if (m_qbar_state && m_drq_state)
		m_dma->dreq2_w(1);
	else
		m_dma->dreq2_w(0);
}

WRITE_LINE_MEMBER( rc702_state::tc_w )
{
	m_tc = state;
	if ((m_dack == 1) && m_tc)
	{
		m_dack = 0;
		m_fdc->tc_w(1);
	}
	else
		m_fdc->tc_w(0);
}

WRITE_LINE_MEMBER( rc702_state::dack1_w )
{
	m_dack = 1;
	if ((m_dack == 1) && m_tc)
	{
		m_dack = 0;
		m_fdc->tc_w(1);
	}
	else
		m_fdc->tc_w(0);
}

WRITE8_MEMBER( rc702_state::port14_w )
{
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	floppy->mon_w(!BIT(data, 0));
}

WRITE8_MEMBER( rc702_state::port18_w )
{
	membank("bankr0")->set_entry(1); // replace roms with ram
}

WRITE8_MEMBER( rc702_state::port1c_w )
{
		m_beep->set_state(1);
		m_beepcnt = 0x3000;
}

// monitor is orange even when powered off
static const rgb_t our_palette[3] = {
	rgb_t(0xc0, 0x60, 0x00), // off
	rgb_t(0xff, 0xb4, 0x00), // on
};

void rc702_state::init_rc702()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
	m_palette->set_pen_colors(0, our_palette, ARRAY_LENGTH(our_palette));
}

I8275_DRAW_CHARACTER_MEMBER( rc702_state::display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t gfx = 0;

	if (!vsp)
		gfx = m_p_chargen[(linecount & 15) | (charcode << 4)];

	if (lten)
		gfx = 0xff;

	if (rvv)
		gfx ^= 0xff;

	// Highlight not used
	bitmap.pix32(y, x++) = palette[BIT(gfx, 1) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 2) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 3) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 4) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 5) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 6) ? 1 : 0];
	bitmap.pix32(y, x++) = palette[BIT(gfx, 7) ? 1 : 0];
}

// Baud rate generator. All inputs are 0.614MHz.
WRITE_LINE_MEMBER( rc702_state::clock_w )
{
	m_ctc1->trg0(state);
	m_ctc1->trg1(state);
	if (m_beepcnt == 0)
		m_beep->set_state(0);
	if (m_beepcnt < 0xfe00)
		m_beepcnt--;
}

WRITE_LINE_MEMBER( rc702_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->hack_w(state); // tell dma that bus has been granted
}

READ8_MEMBER( rc702_state::memory_read_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER( rc702_state::memory_write_byte )
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "ctc1" },
	{ "sio1" },
	{ "pio" },
	{ nullptr }
};

void rc702_state::kbd_put(u8 data)
{
	m_pio->pa_w(machine().dummy_space(), 0, data);
	m_pio->strobe_a(0);
	m_pio->strobe_a(1);
}

static void floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

MACHINE_CONFIG_START(rc702_state::rc702)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(8'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(rc702_mem)
	MCFG_DEVICE_IO_MAP(rc702_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain_intf)

	MCFG_MACHINE_RESET_OVERRIDE(rc702_state, rc702)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, 614000)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, rc702_state, clock_w))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL(8'000'000) / 2)
	MCFG_Z80CTC_ZC0_CB(WRITELINE("sio1", z80dart_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio1", z80dart_device, rxca_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE("sio1", z80dart_device, rxtxcb_w))
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("sio1", Z80DART, XTAL(8'000'000) / 2)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL(8'000'000) / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
//  MCFG_Z80PIO_OUT_PB_CB(WRITE8(*this, rc702_state, portxx_w)) // parallel port

	MCFG_DEVICE_ADD("dma", AM9517A, XTAL(8'000'000) / 2)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(*this, rc702_state, busreq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(*this, rc702_state, tc_w)) // inverted
	MCFG_I8237_IN_MEMR_CB(READ8(*this, rc702_state, memory_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(*this, rc702_state, memory_write_byte))
	MCFG_I8237_IN_IOR_1_CB(READ8("fdc", upd765a_device, mdma_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8("fdc", upd765a_device, mdma_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8("crtc", i8275_device, dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8("crtc", i8275_device, dack_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, rc702_state, dack1_w)) // inverted

	MCFG_UPD765A_ADD("fdc", false, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE("ctc1", z80ctc_device, trg3))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE("dma", am9517a_device, dreq1_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* Keyboard */
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(rc702_state, kbd_put))

	MCFG_DEVICE_ADD("7474", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(*this, rc702_state, q_w))
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(*this, rc702_state, qbar_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(272*2, 200+4*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 272*2-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275_device, screen_update)

	MCFG_DEVICE_ADD("crtc", I8275, 11640000/7)
	MCFG_I8275_CHARACTER_WIDTH(7)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(rc702_state, display_pixels)
	MCFG_I8275_IRQ_CALLBACK(WRITELINE("7474", ttl7474_device, clear_w)) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("ctc1", z80ctc_device, trg2))
	MCFG_I8275_DRQ_CALLBACK(WRITELINE(*this, rc702_state, crtc_drq_w))
	MCFG_PALETTE_ADD("palette", 2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("beeper", BEEP, 1000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( rc702 )
	ROM_REGION( 0x10800, "maincpu", 0 )
		ROM_SYSTEM_BIOS(0, "rc700", "RC700")
		ROMX_LOAD( "rob358.rom", 0x10000, 0x0800,  CRC(254aa89e) SHA1(5fb1eb8df1b853b931e670a2ff8d062c1bd8d6bc), ROM_BIOS(1))
		ROM_SYSTEM_BIOS(1, "rc702", "RC702")
		ROMX_LOAD( "roa375.ic66", 0x10000, 0x0800, CRC(034cf9ea) SHA1(306af9fc779e3d4f51645ba04f8a99b11b5e6084), ROM_BIOS(2))
		ROM_SYSTEM_BIOS(2, "rc703", "RC703")
		ROMX_LOAD( "rob357.rom", 0x10000, 0x0800,  CRC(dcf84a48) SHA1(7190d3a898bcbfa212178a4d36afc32bbbc166ef), ROM_BIOS(3))

	ROM_REGION( 0x1000, "chargen", 0 )
		ROM_LOAD( "roa296.rom", 0x0000, 0x0800, CRC(7d7e4548) SHA1(efb8b1ece5f9eeca948202a6396865f26134ff2f) ) // char
		ROM_LOAD( "roa327.rom", 0x0800, 0x0800, CRC(bed7ddb0) SHA1(201ae9e4ac3812577244b9c9044fadd04fb2b82f) ) // semi_gfx
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME         FLAGS
COMP( 1979, rc702, 0,      0,      rc702,   rc702, rc702_state, init_rc702, "Regnecentralen", "RC702 Piccolo", MACHINE_NOT_WORKING )
