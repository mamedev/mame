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

    ./chdman createhd -chs 306,2,17 -ss 512 -o st406.chd
    ./chdman createhd -chs 306,6,17 -ss 512 -o st412.chd

*/

/*

    TODO:

    - DMA
    - floppy
    - SASI
    - video
    - keyboard

*/

#include "emu.h"
#include "bus/mm2/exp.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i186.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/z80sio.h"
#include "machine/x2212.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "softlist_dev.h"

#define I80186_TAG      "maincpu"

namespace {

class mm2_state : public driver_device
{
public:
	mm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I80186_TAG),
		m_novram(*this, "x2212"),
		m_pic(*this, "pic8259"),
		m_pit(*this, "pit8253"),
		m_mpsc(*this, "i8274"),
		m_speaker(*this, "speaker"),
		m_rs232a(*this, "rs232a"),
		m_rs232b(*this, "rs232b"),
		m_exp(*this, "exp")
	{ }

	void mm2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<x2212_device> m_novram;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<i8274_device> m_mpsc;
	required_device<speaker_sound_device> m_speaker;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<mikromikko2_expansion_bus_device> m_exp;

	void mm2_mem(address_map &map) ATTR_COLD;
	void mm2_io(address_map &map) ATTR_COLD;

	uint8_t status_r(offs_t offset);
	void novram_store(offs_t offset, uint8_t data);
	void novram_recall(offs_t offset, uint8_t data);
	void tcl_w(offs_t offset, uint8_t data) { m_pic->ir0_w(CLEAR_LINE); }
	void diag_w(offs_t offset, uint8_t data) { popmessage("%02x", data); }
	void cls0_w(offs_t offset, uint8_t data) { m_cls0 = BIT(data, 0); }
	void cls1_w(offs_t offset, uint8_t data) { m_cls1 = BIT(data, 0); }
	void dtra_w(offs_t offset, uint8_t data) { m_rs232a->write_dtr(!BIT(data, 0)); }
	void dtrb_w(offs_t offset, uint8_t data) { m_rs232b->write_dtr(!BIT(data, 0)); }
	void tmrout0_w(int state) { if (!m_cls1 && !m_cls0) { m_mpsc->rxca_w(state); m_mpsc->txca_w(state); } };
	void tmrout1_w(int state) { if (!m_cls1 && m_cls0) { m_mpsc->rxca_w(state); m_mpsc->txca_w(state); } };
	void ir0_w(int state) { if (state) { m_pic->ir0_w(ASSERT_LINE); } }

	bool m_cls0;
	bool m_cls1;
};

void mm2_state::novram_store(offs_t offset, uint8_t data)
{
	m_novram->store(1);
	m_novram->store(0);
}

void mm2_state::novram_recall(offs_t offset, uint8_t data)
{
	m_novram->recall(!BIT(data, 0));
}

uint8_t mm2_state::status_r(offs_t offset)
{
	uint8_t data = 0x80;

	data |= !m_rs232a->dsr_r() << 4;

	return data;
}

void mm2_state::mm2_mem(address_map &map)
{
	map(0x00000, 0x1ffff).ram(); // DRAM 128 KB (on SBC186)
	map(0x20000, 0x3ffff).ram(); // DRAM 128 KB (on SBC186)
	map(0x40000, 0x7ffff).ram(); // DRAM 256 KB (on SBC186 or MEME186)
	map(0x80000, 0xbffff).ram(); // DRAM 256 KB (on MEME186)
	map(0xf0000, 0xf01ff).rw(m_novram, FUNC(x2212_device::read), FUNC(x2212_device::write)).umask16(0xff00);
	map(0xf0200, 0xfffff).rom().region(I80186_TAG, 0x200);
}

void mm2_state::mm2_io(address_map &map)
{
	map(0xf800, 0xf803).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xf880, 0xf887).rw(m_mpsc, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0x00ff);
	map(0xf884, 0xf885).r(FUNC(mm2_state::status_r)).umask16(0xff00);
	map(0xf900, 0xf901).w(FUNC(mm2_state::novram_store)).umask16(0x00ff);
	map(0xf930, 0xf937).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
	map(0xf940, 0xf941).w(FUNC(mm2_state::tcl_w)).umask16(0xff00);
	map(0xf950, 0xf951).w(FUNC(mm2_state::diag_w)).umask16(0xff00);
	map(0xf960, 0xf961).w(FUNC(mm2_state::cls0_w)).umask16(0xff00);
	map(0xf962, 0xf963).w(FUNC(mm2_state::cls1_w)).umask16(0xff00);
	//map(0xf965, 0xf965) LOOPBACK LLBA
	//map(0xf967, 0xf967) LOOPBACK LLBB
	//map(0xf969, 0xf969) DATA CODING NRZI
	//map(0xf96b, 0xf96b) SIGNAL LEVELS V24
	//map(0xf96d, 0xf96d) SIGNAL LEVELS X27
	map(0xf96e, 0xf96f).w(FUNC(mm2_state::dtra_w)).umask16(0xff00);
	//map(0xf971, 0xf971) V24 SIGNAL TSTA
	//map(0xf973, 0xf973) V24 SIGNAL SRSA
	map(0xf974, 0xf975).w(FUNC(mm2_state::dtrb_w)).umask16(0xff00);
	//map(0xf97d, 0xf97d) ???
	map(0xf97e, 0xf97f).w(FUNC(mm2_state::novram_recall)).umask16(0xff00);
}

static INPUT_PORTS_START( mm2 )
	// defined in mm2kb.cpp
INPUT_PORTS_END

void mm2_state::machine_start()
{
	// state saving
	save_item(NAME(m_cls0));
	save_item(NAME(m_cls1));
}

void mm2_state::machine_reset()
{
}

void mm2_state::mm2(machine_config &config)
{
	// SBC186
	I80186(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::mm2_mem);
	m_maincpu->set_addrmap(AS_IO, &mm2_state::mm2_io);
	m_maincpu->irmx_irq_cb().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_maincpu->read_slave_ack_callback().set(m_pic, FUNC(pic8259_device::acknowledge));
	m_maincpu->tmrout0_handler().set(FUNC(mm2_state::tmrout0_w));
	m_maincpu->tmrout1_handler().set(FUNC(mm2_state::tmrout1_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(XTAL(16'000'000)/8);
	m_pit->set_clk<1>(XTAL(16'000'000)/8);
	m_pit->set_clk<2>(XTAL(16'000'000)/8);
	m_pit->out_handler<0>().set(FUNC(mm2_state::ir0_w));
	m_pit->out_handler<1>().set(m_mpsc, FUNC(i8274_device::rxtxcb_w));
	m_pit->out_handler<2>().set(m_speaker, FUNC(speaker_sound_device::level_w));

	I8274(config, m_mpsc, XTAL(16'000'000)/4);
	m_mpsc->out_txda_callback().set(m_rs232a, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_rtsa_callback().set(m_rs232a, FUNC(rs232_port_device::write_rts));
	m_mpsc->out_txdb_callback().set(m_rs232b, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_rtsb_callback().set(m_rs232b, FUNC(rs232_port_device::write_rts));
	m_mpsc->out_int_callback().set(m_pic, FUNC(pic8259_device::ir1_w));

	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set(m_mpsc, FUNC(z80dart_device::rxa_w));
	m_rs232a->dcd_handler().set(m_mpsc, FUNC(z80dart_device::dcda_w));
	m_rs232a->cts_handler().set(m_mpsc, FUNC(z80dart_device::ctsa_w));

	RS232_PORT(config, m_rs232b, default_rs232_devices, "terminal");
	m_rs232b->rxd_handler().set(m_mpsc, FUNC(z80dart_device::rxb_w));
	m_rs232b->cts_handler().set(m_mpsc, FUNC(z80dart_device::ctsb_w));

	X2212(config, m_novram);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker, 0).add_route(ALL_OUTPUTS, "mono", 1.00);

	MIKROMIKKO2_EXPANSION_BUS(config, m_exp, 0);
	m_exp->set_memspace(I80186_TAG, AS_PROGRAM);
	m_exp->set_iospace(I80186_TAG, AS_IO);
	m_exp->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_exp->ir2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_exp->ir3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_exp->ir4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_exp->ir5_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_exp->hold1_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_exp->hold2_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_exp->hold3_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_exp->hold4_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_exp->hold5_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	MIKROMIKKO2_EXPANSION_BUS_SLOT(config, "exp1", 0, m_exp, mikromikko2_expansion_bus_cards, "mmc186", false);
	MIKROMIKKO2_EXPANSION_BUS_SLOT(config, "exp2", 0, m_exp, mikromikko2_expansion_bus_cards, "crtc186", false);
	MIKROMIKKO2_EXPANSION_BUS_SLOT(config, "exp3", 0, m_exp, mikromikko2_expansion_bus_cards, nullptr, false);
	MIKROMIKKO2_EXPANSION_BUS_SLOT(config, "exp4", 0, m_exp, mikromikko2_expansion_bus_cards, nullptr, false);
	MIKROMIKKO2_EXPANSION_BUS_SLOT(config, "exp5", 0, m_exp, mikromikko2_expansion_bus_cards, nullptr, false);
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
ROM_END

} // anonymous namespace

COMP( 1983, mm2m35d,  0,     0,      mm2,   mm2,   mm2_state, empty_init, "Nokia Data", "MikroMikko 2 M35D", MACHINE_NOT_WORKING )
