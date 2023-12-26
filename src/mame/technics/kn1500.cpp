// license:GPL2+
// copyright-holders:Felipe Sanches
/******************************************************************************

	Technics SX-KN1500 music keyboard driver

******************************************************************************/

#include "emu.h"
#include "cpu/tlcs900/tmp95c061.h"
#include "video/hd44780.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class kn1500_state : public driver_device
{
public:
	kn1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_lcdc(*this, "hd44780")
	{ }

	void kn1500(machine_config &config);

private:
	required_device<tmp95c061_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<hd44780_device> m_lcdc;

	HD44780_PIXEL_UPDATE(kn1500_pixel_update);
	void kn1500_palette(palette_device &palette) const;
	void kn1500_tlcs900_porta_w(offs_t offset, uint8_t data);
	void kn1500_tlcs900_portb_w(offs_t offset, uint8_t data);
	void kn1500_tlcs900_port5_w(offs_t offset, uint8_t data);
	void kn1500_tlcs900_port6_w(offs_t offset, uint8_t data);
	void kn1500_tlcs900_port7_w(offs_t offset, uint8_t data);
	void kn1500_mem(address_map &map);
	uint16_t dsp_lcd_r(offs_t offset);
	void dsp_lcd_w(offs_t offset, uint16_t data);
	uint16_t tone_generator_r(offs_t offset);
	void tone_generator_w(offs_t offset, uint16_t data);

	bool m_mute; // Mute Control
	bool m_nreset; // Active-low Reset
	bool m_dspcs; // DSP Chip Select
	bool m_lcdcs; // Liquid Crystal Chip Select
	bool m_dspcd; // DSP command/data
	bool m_dspwr; // DSP Write
	bool m_dsprd; // DSP Read
	bool m_dsp_lcd_bit7; // called DSP/LCDR on the schematics
	uint8_t m_dsp_lcd_byte; // latch for data sent to DSP or Liquid Crystal
};

void kn1500_state::kn1500_mem(address_map &map)
{
	map(0x000000, 0x77ffff).ram(); //~CS3: MSAR3=00 MAMR3=0f
//	map(0x000080, 0x07ffff).ram(); //~CS3: MSAR3=00 MAMR3=0f
	//map(0x780000, 0x780003).rw(FUNC(kn1500_state::dsp_lcd_r), FUNC(kn1500_state::dsp_lcd_w)); // ~CS0 & A19=1 & A18-16=000: MSAR0=78 MAMR0=3f
	map(0x780004, 0x78ffff).noprw();
	map(0x790000, 0x79ffff).rw(FUNC(kn1500_state::tone_generator_r), FUNC(kn1500_state::tone_generator_w)); //~CS0 & A19=1 & A18-16=001: MSAR0=78 MAMR0=3f
	//map(0x7a0000, 0x7affff).w(m_fdc, FUNC(upd765a_device::dack_w)); //~CS0 & A19=1 & A18-16=010: MSAR0=78 MAMR0=3f
	map(0x7a0000, 0x7affff).noprw(); // ????
	//map(0x7b0000, 0x7bffff).m(m_fdc, FUNC(upd765a_device::map)); //~CS0 & A19=1 & A18-16=011: MSAR0=78 MAMR0=3f
	map(0x7b0000, 0x7bffff).noprw(); // ????
	map(0x7c0000, 0xbfffff).noprw(); // ????
	map(0xc00000, 0xdfffff).rom().region("rhythm", 0).nopw(); //~RHYMCS = ~CS1: MSAR1=c0 MAMR1=7f
	map(0xe00000, 0xffffff).rom().region("prog", 0).nopw();   //~PROGCS = ~CS2: MSAR2=e0 MAMR2=3f

//[:maincpu] ':maincpu' (FA0484): unmapped program memory write to DFF83A = 005A & 00FF
//[:maincpu] ':maincpu' (FA048F): unmapped program memory write to DFF83A = 0000 & 00FF
//[:maincpu] ':maincpu' (FA0494): unmapped program memory write to DFF83A = A500 & FF00
//[:maincpu] ':maincpu' (FA049F): unmapped program memory write to DFF83A = 3200 & FF00
//[:maincpu] ':maincpu' (FA0484): unmapped program memory write to DFF83C = 005A & 00FF
//[:maincpu] ':maincpu' (FA048F): unmapped program memory write to DFF83C = 0000 & 00FF


}

static void kn1500_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static INPUT_PORTS_START(kn1500)
	//PORT_START("...")
	//PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_?) PORT_NAME("...")
	//PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


void kn1500_state::kn1500_tlcs900_porta_w(offs_t offset, uint8_t data)
{
	//int DSPRDY = BIT(data, 1);
	//int ? = BIT(data, 2);
	//int FDMOT = BIT(data, 3);
}


void kn1500_state::kn1500_tlcs900_portb_w(offs_t offset, uint8_t data)
{
	//int ? = BIT(data, 0);
	//int FDINT = BIT(data, 1);
	//int FDRST = BIT(data, 2);
	//int FDTC = BIT(data, 3);
	//int ? = BIT(data, 4);
	//int FDDRQ = BIT(data, 5);
	//int ? = BIT(data, 6);
	//int ? = BIT(data, 7);
}

void kn1500_state::kn1500_tlcs900_port5_w(offs_t offset, uint8_t data)
{
	logerror("port5_w: %02X\n", data);
	m_mute = BIT(data, 3);
}

void kn1500_state::kn1500_tlcs900_port6_w(offs_t offset, uint8_t data)
{
	logerror("port6_w: %02X\n", data);
	m_nreset = BIT(data, 5);
}

void kn1500_state::kn1500_tlcs900_port7_w(offs_t offset, uint8_t data)
{
	uint8_t value;
	logerror("port7_w (LCD/DSP): %02X\n", data);

	m_dspcs = BIT(data, 0);
	m_lcdcs = BIT(data, 1);
	m_dspcd = BIT(data, 2);
	m_dspwr = BIT(data, 3);
	m_dsprd = BIT(data, 4);
	m_dsp_lcd_bit7 = BIT(data, 5);
	//m_dsprst = BIT(data, 6);
	//m_dsprst2 = BIT(data, 7);
	value = m_dsp_lcd_bit7 | m_dsp_lcd_byte;

	// DSP:
	if (BIT(data, 0) && !m_dspcs) {
		logerror("DSP(value=%02X): //Implement-me!\n", value);
		// TODO: Implement-me!
	}

	// LCD:
	if (BIT(data, 1) && !m_lcdcs && !m_dspwr) {
		// Note: the DSPWR signal is tied to the R/~W pin of the LCD controller
		//       and DSPRD sets the RS pin (register select: HIGH=characters / LOW=commands).
		if (m_dsprd){
			// Send chars
			logerror("m_lcdc->data_w(%02X);\n", value);
			m_lcdc->data_w(value);
		} else {
			// Send commands
			logerror("m_lcdc->control_w(%02X);\n", value);
			m_lcdc->control_w(value);
		}
	}

}

uint16_t kn1500_state::dsp_lcd_r(offs_t offset){
	// DSP:
	if (!m_dspcs) {
		// TODO: Implement-me!
		return 0;
	}

	return 0;
}

void kn1500_state::dsp_lcd_w(offs_t offset, uint16_t data){
	m_dsp_lcd_byte = (data >> 8) & 0x7f;
}

uint16_t kn1500_state::tone_generator_r(offs_t offset){
	// TODO: Implement-me!
	return 0;
}

void kn1500_state::tone_generator_w(offs_t offset, uint16_t data){
	// TODO: Implement-me!
}


void kn1500_state::kn1500_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 188));
	palette.set_pen_color(1, rgb_t(92, 83, 128));
}

static const gfx_layout prot_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_kn1500 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, prot_charlayout, 0, 1 )
GFXDECODE_END

HD44780_PIXEL_UPDATE(kn1500_state::kn1500_pixel_update)
{
	if ( pos < 16 && line==0 )
	{
		bitmap.pix(y, pos*6 + x) = state;
	}

	if ( pos >= 64 && pos < 80 && line==0 )
	{
		bitmap.pix(y+9,(pos-64)*6 + x) = state;
	}
}

void kn1500_state::kn1500(machine_config &config)
{
	TMP95C061(config, m_maincpu, 24_MHz_XTAL); // TMP95C061AF
	m_maincpu->set_am8_16(0);
	m_maincpu->set_addrmap(AS_PROGRAM, &kn1500_state::kn1500_mem);
	m_maincpu->porta_write().set(FUNC(kn1500_state::kn1500_tlcs900_porta_w));
	m_maincpu->portb_write().set(FUNC(kn1500_state::kn1500_tlcs900_portb_w));
	m_maincpu->port5_write().set(FUNC(kn1500_state::kn1500_tlcs900_port5_w));
	m_maincpu->port6_write().set(FUNC(kn1500_state::kn1500_tlcs900_port6_w));
	m_maincpu->port7_write().set(FUNC(kn1500_state::kn1500_tlcs900_port7_w));

	UPD765A(config, m_fdc, 24'000'000, true, true); // actual controller is UPD72070GF3BE at IC4
	m_fdc->drq_wr_callback().set([this](int state){ m_maincpu->set_input_line(TLCS900_INT7, state); });
	m_fdc->intrq_wr_callback().set([this](int state){ m_maincpu->set_input_line(TLCS900_INT5, state); });
	FLOPPY_CONNECTOR(config, "fdc:0", kn1500_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 9*2);
	screen.set_visarea(0, 6*16-1, 0, 9*2-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(kn1500_state::kn1500_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_kn1500);

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(kn1500_state::kn1500_pixel_update));


	/* sound hardware */
	//SPEAKER(config, "lspeaker").front_left();
	//SPEAKER(config, "rspeaker").front_right();
}


ROM_START(kn1500)
	ROM_REGION16_LE(0x200000, "prog" , 0)
	ROM_LOAD("technics_qsigt3c16079_5y68-j079_japan_9649eai.ic15", 0x00000, 0x200000, CRC(0f78da9a) SHA1(53d5c43d833fb005a7bd377583252b84b646253d))

	ROM_REGION16_LE(0x200000, "rhythm" , 0)
//	ROM_LOAD("technics_qsigt3c16068_japan.ic17", 0x00000, 0x200000, NO_DUMP)
	ROM_LOAD("technics_qsigt3c16079_5y68-j079_japan_9649eai.ic15.rest", 0x00000, 0x200000, CRC(ce60897a) SHA1(9b54f693f693488132b93e8bfed1927d7e741ae1))
ROM_END

} // anonymous namespace

//   YEAR  NAME   PARENT  COMPAT  MACHINE INPUT   STATE         INIT        COMPANY      FULLNAME                 FLAGS
CONS(199?, kn1500,    0,       0, kn1500, kn1500, kn1500_state, empty_init, "Technics", "SX-KN1500 PCM Keyboard", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
