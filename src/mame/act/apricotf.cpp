// license: BSD-3-Clause
// copyright-holders: Angelo Salese, Curt Coder, AJR
/**************************************************************************************************

ACT Apricot F1 series

TODO:
- Need software dumps to go further;
- Convert workram pointer to ram_device (needs documented range of available configs);

**************************************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "formats/apridisk.h"
#include "imagedev/floppy.h"
#include "apricotkb.h"
#include "machine/74259.h"
#include "machine/buffer.h"
#include "machine/input_merger.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> f1_daisy_device

class f1_daisy_device : public device_t, public z80_daisy_chain_interface
{
public:
	f1_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	IRQ_CALLBACK_MEMBER(inta_cb);

protected:
	virtual void device_start() override;
};

DEFINE_DEVICE_TYPE(F1_DAISY, f1_daisy_device, "f1_daisy", "F1 daisy chain abstraction")

f1_daisy_device::f1_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, F1_DAISY, tag, owner, clock)
	, z80_daisy_chain_interface(mconfig, *this)
{
}

void f1_daisy_device::device_start()
{
}

IRQ_CALLBACK_MEMBER(f1_daisy_device::inta_cb)
{
	device_z80daisy_interface *intf = daisy_get_irq_device();
	if (intf != nullptr)
		return intf->z80daisy_irq_ack();
	else
		return 0xff;
}

// ======================> f1_state

class f1_state : public driver_device
{
public:
	f1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_sio(*this, "sio")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_irqs(*this, "irqs")
		, m_workram(*this, "workram")
		, m_paletteram(*this, "paletteram")
		, m_palette(*this, "palette")
	{ }

	void act_f1(machine_config &config);

private:
	static void floppy_formats(format_registration &fr);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_merger_device> m_irqs;
	required_shared_ptr<u16> m_workram;
	required_shared_ptr<u16> m_paletteram;
	required_device<palette_device> m_palette;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 palette_r(offs_t offset);
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void drive_select_w(int state);
	void hld_w(int state);
	void motor_on_w(int state);
	void video_lines_w(int state);
	void video_columns_w(int state);
	void led0_enable_w(int state);
	void led1_enable_w(int state);

	void m1_w(u8 data);

	int m_width80 = 0;
	int m_lines200 = 0;

	void main_io(address_map &map);
	void main_map(address_map &map);
};


//**************************************************************************
//  VIDEO
//**************************************************************************

u32 f1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int lines = m_lines200 ? 200 : 256;
	// start bases are per line, in a dedicated buffer
	// TODO: is this configurable?
	const u32 vram_table = 0x1e00 >> 1;
	const u32 vram_mask = 0x3ffff >> 1;

	for (int y = 0; y < lines; y++)
	{
		offs_t base_offs = m_workram[(y + vram_table) & vram_mask];

		for (int sx = 0; sx < 80; sx++)
		{
			u16 data = m_workram[(base_offs + sx) & vram_mask];

			if (m_width80)
			{
				for (int x = 0; x < 8; x++)
				{
					int color = (BIT(data, 15) << 1) | BIT(data, 7);

					bitmap.pix(y, (sx * 8) + x) = color;

					data <<= 1;
				}
			}
			else
			{
				for (int x = 0; x < 4; x++)
				{
					int color = (BIT(data, 15) << 3) | (BIT(data, 14) << 2) | (BIT(data, 7) << 1) | BIT(data, 6);

					bitmap.pix(y, (sx * 8) + (x * 2)) = color;
					bitmap.pix(y, (sx * 8) + (x * 2) + 1) = color;

					data <<= 2;
				}
			}
		}
	}

	return 0;
}

u16 f1_state::palette_r(offs_t offset)
{
	return m_paletteram[offset];
}

void f1_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);

	//TODO: we discard offset 0 because all BIOSes sets white there (0xf).
	// Either routed to border or genlock.
	if(ACCESSING_BITS_0_7 && offset)
	{
		u8 i = 0, r = 0, g = 0, b = 0;

		const u16 datax = m_paletteram[offset];
		i = BIT(datax, 0);
		r = (BIT(datax, 1) << 1) | i;
		g = (BIT(datax, 2) << 1) | i;
		b = (BIT(datax, 3) << 1) | i;

		m_palette->set_pen_color(offset, pal2bit(r), pal2bit(g), pal2bit(b));
	}
}


static const gfx_layout charset_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_act_f1 )
	GFXDECODE_ENTRY( "maincpu", 0x0800, charset_8x8, 0, 1 )
GFXDECODE_END

//**************************************************************************
//  I/Os
//**************************************************************************

void f1_state::drive_select_w(int state)
{
	m_fdc->set_floppy(m_floppy[!state]->get_device());
}

void f1_state::hld_w(int state)
{
	// TODO: drive head load
}

void f1_state::motor_on_w(int state)
{
	m_floppy[0]->get_device()->mon_w(!state);
	m_floppy[1]->get_device()->mon_w(!state);
}

void f1_state::video_lines_w(int state)
{
	// video lines (1=200, 0=256)
	m_lines200 = state;
}

void f1_state::video_columns_w(int state)
{
	// video columns (1=80, 0=40)
	m_width80 = state;
}

void f1_state::led0_enable_w(int state)
{
	// TODO: LED 0 enable
}

void f1_state::led1_enable_w(int state)
{
	// TODO: LED 1 enable
}


void f1_state::machine_start()
{
	save_item(NAME(m_width80));
	save_item(NAME(m_lines200));
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( main_map )
//-------------------------------------------------

void f1_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).ram().share("workram");
	map(0xe0000, 0xe001f).rw(FUNC(f1_state::palette_r), FUNC(f1_state::palette_w)).share("paletteram");
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( main_io )
//-------------------------------------------------

void f1_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).w(m_cent_data_out, FUNC(output_latch_device::write));
	map(0x0000, 0x000f).w("syslatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x0010, 0x0017).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0x0020, 0x0027).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)).umask16(0x00ff);
	map(0x0030, 0x0030).w(FUNC(f1_state::m1_w));
	map(0x0040, 0x0047).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write)).umask16(0x00ff);
//  map(0x01e0, 0x01ff) winchester
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( act )
//-------------------------------------------------

static INPUT_PORTS_START( act )
	// defined in act/apricotkb.cpp
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

void f1_state::m1_w(u8 data)
{
	m_ctc->z80daisy_decode(data);
	m_sio->z80daisy_decode(data);
}

//-------------------------------------------------
//  floppy
//-------------------------------------------------

void f1_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();

	fr.add(FLOPPY_APRIDISK_FORMAT);
}

void apricotf_floppies(device_slot_interface &device)
{
	device.option_add("d31v", SONY_OA_D31V);
	device.option_add("d32w", SONY_OA_D32W);
}

static const z80_daisy_config f1_daisy_config[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( act_f1 )
//-------------------------------------------------

void f1_state::act_f1(machine_config &config)
{
	static constexpr auto CLK5 = 14_MHz_XTAL / 3; // nominally 5 MHz, actually 4.66 MHz
	static constexpr auto BAUDCLK = 14_MHz_XTAL / 7 / 13; // documented as 153.8 kHz

	/* basic machine hardware */
	I8086(config, m_maincpu, CLK5); // @ 10D
	m_maincpu->set_addrmap(AS_PROGRAM, &f1_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &f1_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("daisy", FUNC(f1_daisy_device::inta_cb));

	F1_DAISY(config, "daisy").set_daisy_config(f1_daisy_config);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	ls259_device &syslatch(LS259(config, "syslatch")); // 74LS259 @ 16B
	syslatch.q_out_cb<0>().set(FUNC(f1_state::drive_select_w));
	syslatch.q_out_cb<1>().set(FUNC(f1_state::hld_w));
	syslatch.q_out_cb<2>().set(FUNC(f1_state::motor_on_w));
	syslatch.q_out_cb<3>().set(FUNC(f1_state::video_lines_w));
	syslatch.q_out_cb<4>().set(FUNC(f1_state::video_columns_w));
	syslatch.q_out_cb<5>().set(FUNC(f1_state::led0_enable_w));
	syslatch.q_out_cb<6>().set(FUNC(f1_state::led1_enable_w));
	syslatch.q_out_cb<7>().set(m_centronics, FUNC(centronics_device::write_strobe)).invert();

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14_MHz_XTAL, 896, 0, 640, 312, 0, 256);
	//screen.set_raw(14_MHz_XTAL, 896, 0, 640, 260, 0, 200);
	screen.set_screen_update(FUNC(f1_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(m_ctc, FUNC(z80ctc_device::trg3)).invert();

	PALETTE(config, m_palette).set_entries(16);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_act_f1);

	/* Devices */
	APRICOT_KEYBOARD(config, APRICOT_KEYBOARD_TAG, 0);

	Z80SIO(config, m_sio, CLK5 / 2); // Z80-CTC @ 13D
	m_sio->out_txda_callback().set("speaker", FUNC(speaker_sound_device::level_w));
	m_sio->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_sio->out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_sio->out_int_callback().set("irqs", FUNC(input_merger_device::in_w<0>));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.5);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	rs232.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232.dsr_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	Z80CTC(config, m_ctc, CLK5 / 2); // Z80-SIO/2 @ 15D
	m_ctc->intr_callback().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_ctc->set_clk<1>(BAUDCLK);
	m_ctc->set_clk<2>(BAUDCLK);
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxcb_w));
	m_ctc->zc_callback<1>().append(m_sio, FUNC(z80sio_device::txcb_w));
	m_ctc->zc_callback<1>().append("rs232", FUNC(rs232_port_device::write_etc));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::txca_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_sio, FUNC(z80sio_device::ctsa_w)).invert();

	OUTPUT_LATCH(config, m_cent_data_out); // 74LS373 @ 13C
	m_centronics->set_output_latch(*m_cent_data_out);

	// floppy
	WD2797(config, m_fdc, 14_MHz_XTAL / 7); // WD2797 @ 5F
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_TEST);

	FLOPPY_CONNECTOR(config, m_floppy[0], apricotf_floppies, "d32w", f1_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], apricotf_floppies, "d32w", f1_state::floppy_formats);

	// TODO: expansion port (INT lines gated onto CLK0 of CTC)
}



//**************************************************************************
//  ROM definitions
//**************************************************************************

//-------------------------------------------------
//  ROM( f1 )
//-------------------------------------------------

ROM_START( f1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lo_f1_1.6.8f",  0x0000, 0x4000, CRC(be018be2) SHA1(80b97f5b2111daf112c69b3f58d1541a4ba69da0) )    // Labelled F1 - LO Vr. 1.6
	ROM_LOAD16_BYTE( "hi_f1_1.6.10f", 0x0001, 0x4000, CRC(bbba77e2) SHA1(e62bed409eb3198f4848f85fccd171cd0745c7c0) )    // Labelled F1 - HI Vr. 1.6
ROM_END

#define rom_f1e rom_f1
#define rom_f2 rom_f1


//-------------------------------------------------
//  ROM( f10 )
//-------------------------------------------------

ROM_START( f10 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lo_f10_3.1.1.8f",  0x0000, 0x4000, CRC(bfd46ada) SHA1(0a36ef379fa9af7af9744b40c167ce6e12093485) ) // Labelled LO-FRange Vr3.1.1
	ROM_LOAD16_BYTE( "hi_f10_3.1.1.10f", 0x0001, 0x4000, CRC(67ad5b3a) SHA1(a5ececb87476a30167cf2a4eb35c03aeb6766601) ) // Labelled HI-FRange Vr3.1.1
ROM_END


COMP( 1984, f1,   0,      0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F1",  MACHINE_NOT_WORKING )
COMP( 1984, f1e,  f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F1e", MACHINE_NOT_WORKING )
COMP( 1984, f2,   f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F2",  MACHINE_NOT_WORKING )
COMP( 1985, f10,  f1,     0,      act_f1,  act,   f1_state, empty_init, "ACT",   "Apricot F10", MACHINE_NOT_WORKING )
