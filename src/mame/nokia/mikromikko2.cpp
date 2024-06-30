// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Models:

    M13A: Floppy drive 5.25", 320 KB + memory 128 KB
    M13B: Floppy drive 5.25", 320 KB + memory 256 KB
    M13C: Floppy drive 5.25", 320 KB + memory 512 KB
    M13D: Floppy drive 5.25", 320 KB + memory 768 KB
    M14A: 2 floppy drives 5.25", 320 KB + memory 128 KB
    M14B: 2 floppy drives 5.25", 320 KB + memory 256 KB
    M14C: 2 floppy drives 5.25", 320 KB + memory 512 KB
    M14D: 2 floppy drives 5.25", 320 KB + memory 768 KB
    M15A: Floppy drive 5.25", 640 KB + memory 128 KB
    M15B: Floppy drive 5.25", 640 KB + memory 256 KB
    M15C: Floppy drive 5.25", 640 KB + memory 512 KB
    M15D: Floppy drive 5.25", 640 KB + memory 768 KB
    M16A: 2 floppy drives 5.25", 640 KB + memory 128 KB
    M16B: 2 floppy drives 5.25", 640 KB + memory 256 KB
    M16C: 2 floppy drives 5.25", 640 KB + memory 512 KB
    M16D: 2 floppy drives 5.25", 640 KB + memory 768 KB
    M25B: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 5 MB + memory 256 KB
    M25C: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 5 MB + memory 512 KB
    M25D: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 5 MB + memory 768 KB
    M35B: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 15 MB + memory 256 KB
    M35C: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 15 MB + memory 512 KB
    M35D: Floppy drive 5.25", 640 KB + hard disk 5.25" Winchester, 15 MB + memory 768 KB

    ./chdman createhd -chs 306,2,32 -ss 256 -o st406.chd

*/

#include "emu.h"
#include "mikromikko2.h"
#include "softlist_dev.h"

void mm2_state::novram_store(offs_t offset, uint8_t data)
{
	m_novram->store(1);
	m_novram->store(0);
}

void mm2_state::novram_recall(offs_t offset, uint8_t data)
{
	m_novram->recall(BIT(data, 0));
}

uint8_t mm2_state::videoram_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint16_t data = program.read_word(0xd0000 | (offset << 1));

	// character
	m_drb0->write(data & 0xff);

	// attributes
	m_drb1->write(data >> 8);

	return data & 0xff;
}

void mm2_state::mm2_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram(); // DRAM 128 KB (on SBC186)
	map(0x20000, 0x3ffff).ram(); // DRAM 128 KB (on SBC186)
	map(0x40000, 0x7ffff).ram(); // DRAM 256 KB (on SBC186 or MEME186)
	map(0x80000, 0xbffff).ram(); // DRAM 256 KB (on MEME186)
	map(0xd0000, 0xd1fff).ram(); // video RAM
	map(0xd8000, 0xd9fff).rom().region("chargen", 0);
	map(0xf0000, 0xf01ff).rw(m_novram, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0xff00);
	map(0xf0200, 0xfffff).rom().region(I80186_TAG, 0x200);
}

void mm2_state::mm2_io_map(address_map &map)
{
	// SBC16
	map(0xf800, 0xf803).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xf880, 0xf887).rw(m_mpsc, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0xff00);
	//map(0xf885, 0xf885) STATUS INPUT PORT
	map(0xf900, 0xf901).w(FUNC(mm2_state::novram_store)).umask16(0x00ff);
	map(0xf97e, 0xf97f).w(FUNC(mm2_state::novram_recall)).umask16(0xff00);
	map(0xf930, 0xf937).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	//map(0xf941, 0xf941) TIMER INTERRUPT CLEAR LATCH
	//map(0xf951, 0xf951) DIAGNOSTIC DISPLAY
	//map(0xf961, 0xf961) CLOCK SELECT CLS0
	//map(0xf963, 0xf963) CLOCK SELECT CLS1
	//map(0xf965, 0xf965) LOOPBACK LLBA
	//map(0xf967, 0xf967) LOOPBACK LLBB
	//map(0xf969, 0xf969) DATA CODING NRZI
	//map(0xf96b, 0xf96b) SIGNAL LEVELS V24
	//map(0xf96d, 0xf96d) SIGNAL LEVELS X27
	//map(0xf96f, 0xf96f) V24 SIGNAL DTRA
	//map(0xf971, 0xf971) V24 SIGNAL TSTA
	//map(0xf973, 0xf973) V24 SIGNAL SRSA
	//map(0xf975, 0xf975) V24 SIGNAL DTRB

	// CRTC186
	map(0xf980, 0xf9ff).rw(m_vpac, FUNC(crt9007_device::read), FUNC(crt9007_device::write)).umask16(0x00ff);
	map(0xf980, 0xf981).rw(m_sio, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w)).umask16(0xff00);
	map(0xf982, 0xf983).rw(m_sio, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w)).umask16(0xff00);
	map(0xf9c0, 0xf9c1).w(FUNC(mm2_state::cpl_w));
	map(0xf9c2, 0xf9c3).w(FUNC(mm2_state::blc_w));
	map(0xf9c4, 0xf9c5).w(FUNC(mm2_state::mode_w));
	map(0xf9c6, 0xf9c7).w(FUNC(mm2_state::modeg_w));
	map(0xf9ca, 0xf9cb).w(FUNC(mm2_state::c70_50_w));
	map(0xf9cc, 0xf9cd).w(FUNC(mm2_state::crb_w));
	map(0xf9ce, 0xf9cf).w(FUNC(mm2_state::cru_w));

	// MMC186
	map(0xfa00, 0xfa1f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	// map(0xfa20, 0xfa20) SASI COMMAND/STATUS
	// map(0xfa22, 0xfa22) SASI DATA
	map(0xfa40, 0xfa41).r(m_fdc, FUNC(upd765a_device::msr_r)).umask16(0x00ff);
	map(0xfa42, 0xfa43).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w)).umask16(0x00ff);
	// map(0xfa60, 0xfa60) CONTROL
	// map(0xfa70, 0xfa70) DMA HIGH ADDRESS BITS
}

void mm2_state::vpac_mem(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(mm2_state::videoram_r));
}

static INPUT_PORTS_START( mm2 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 15,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{  0*16,  1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
		8*16,  9*16, 10*16, 11*16, 12*16, 13*16, 14*16 },
	16*16
};

static const gfx_layout gfxlayout =
{
	8, 1,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 15*16 },
	16*16
};

static GFXDECODE_START( gfx_mm2 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0, gfxlayout, 0, 1 )
GFXDECODE_END

uint32_t mm2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_blc)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	uint16_t *vram = (uint16_t *)m_maincpu->space(AS_PROGRAM).get_read_ptr(0xd1000);
	uint16_t *crom = (uint16_t *)m_maincpu->space(AS_PROGRAM).get_read_ptr(0xd8000);

	for (int sy = 0; sy < 24; sy++)
	{
		for (int y = 0; y < 12; y++)
		{
			for (int sx = 0; sx < 80; sx++)
			{
				uint16_t data = vram[(sy * 80) + sx];
				offs_t char_addr = ((data & 0x1ff) << 4) | y;
				uint16_t char_data = crom[char_addr];

				for (int bit = 0; bit < 8; bit++)
				{
					bool pixel = BIT(char_data, 0) ^ !m_cpl;
					char_data >>= 1;

					bitmap.pix((sy * 12) + y, (sx * 8) + bit) = pixel;
				}
			}
		}
	}

	return 0;
}

void mm2_state::machine_start()
{
}

void mm2_state::machine_reset()
{
	m_cpl = 0;
	m_blc = 0;
	m_mode = 0;
	m_modeg = 0;
	m_c70_50 = 0;
	m_cru = 0;
	m_crb = 0;
}

void mm2_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

static void mm2_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void mm2_state::mm2(machine_config &config)
{
	// SBC186
	I80186(config, m_maincpu, 16_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::mm2_map);
	m_maincpu->set_addrmap(AS_IO, &mm2_state::mm2_io_map);

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(16_MHz_XTAL/8);
	m_pit->set_clk<1>(16_MHz_XTAL/8);
	m_pit->set_clk<2>(16_MHz_XTAL/8);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	//m_pit->out_handler<1>().set()); MPSC ch B line clock
	//m_pit->out_handler<2>().set()); Beep control

	I8274(config, m_mpsc, 16_MHz_XTAL/4);
	m_mpsc->out_int_callback().set(m_pic, FUNC(pic8259_device::ir1_w));

	X2212(config, m_novram);

	// CRTC186
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(71);
	screen.set_screen_update(FUNC(mm2_state::screen_update));
	screen.set_size(640, 420);
	screen.set_visarea(0, 640-1, 0, 420-1);
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mm2);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CRT9007(config, m_vpac, 35.4525_MHz_XTAL/8);
	m_vpac->set_addrmap(0, &mm2_state::vpac_mem);
	m_vpac->set_character_width(10);
	m_vpac->int_callback().set(FUNC(mm2_state::vpac_int_w));
	m_vpac->set_screen(SCREEN_TAG);

	CRT9212(config, m_drb0, 0);

	CRT9212(config, m_drb1, 0);

	I8251(config, m_sio, 16_MHz_XTAL/4);
	m_sio->rxrdy_handler().set(FUNC(mm2_state::sio_rxrdy_w));
	m_sio->txrdy_handler().set(FUNC(mm2_state::sio_txrdy_w));

	// MMC186
	AM9517A(config, m_dmac, 16_MHz_XTAL/4);

	UPD765A(config, m_fdc, 16_MHz_XTAL/2, true, true);
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir4_w));

	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", mm2_floppies, "525qd", mm2_state::floppy_formats).enable_sound(true);

	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("mm2_flop");
}

ROM_START( mm2m35d )
	ROM_REGION16_LE( 0x10000, I80186_TAG, 0 )
	ROM_DEFAULT_BIOS("c")
	ROM_SYSTEM_BIOS(0, "a", "A")
	ROMX_LOAD( "9488a.ic38", 0x0000, 0x4000, CRC(ae831b67) SHA1(d922f02dfac783d0c86ca9a09bc2ad345ee1e71a), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "9490a.ic52", 0x0001, 0x4000, CRC(3ca470d1) SHA1(4cc300544e4a81939c2eb87e22c3ea367a7ec62c), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "9489a.ic41", 0x8000, 0x4000, CRC(a0f19bf5) SHA1(6af91b2f798ddfa9430546e23f00bbeb5ead5a29), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "9491a.ic58", 0x8001, 0x4000, CRC(cf7f3e6d) SHA1(5bf24661f5535d40d1b6ef7f2599f424f6eb2a11), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "c", "C")
	ROMX_LOAD( "9488c.ic38", 0x0000, 0x4000, CRC(cbd151f0) SHA1(16470d4c2cee7a515640894d7ff1b3662516082a), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "9490c.ic52", 0x0001, 0x4000, CRC(bfde706e) SHA1(8a154aa00d480684b00aa7c30be6d6a78dd9ddaa), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "9489c.ic41", 0x8000, 0x4000, CRC(b5086aac) SHA1(f8d7a936baa701dcc30949fe1241be2ab9b80201), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "9491c.ic58", 0x8001, 0x4000, CRC(32047735) SHA1(408f03bc2d89257488e4b3336500681bb168cdec), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_REGION16_LE( 0x4000, "chargen", 0 )
	ROMX_LOAD( "9067e.ic40", 0x0000, 0x2000, CRC(fa719d92) SHA1(af6cc03a8171b9c95e8548c5e0268816344d7367), ROM_SKIP(1) )

	ROM_REGION( 0x2000, "attr", 0 )
	ROM_LOAD( "9026a.ic26", 0x0000, 0x2000, CRC(fe1da600) SHA1(3a5512b08d8f7bb5a0ff3f50bcf33de649a0489d) )

	ROM_REGION( 0x100, "timing", 0 )
	ROM_LOAD( "739025b.ic8", 0x000, 0x100, CRC(c538b10a) SHA1(9810732a52ee6b8313d27462b27acc7e4d5badeb) )

	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "keyboard", 0x000, 0x400, NO_DUMP )
ROM_END

COMP( 1983, mm2m35d,  0,     0,      mm2,   mm2,   mm2_state, empty_init, "Nokia Data", "MikroMikko 2 M35D", MACHINE_NOT_WORKING )
