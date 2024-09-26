// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Epic 14E video display terminal.

    This green-screen terminal emulates the TeleVideo 925. It was later
    acquired by ADDS and rereleased as the Viewpoint/925+.

***************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/input_merger.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "sound/spkrdev.h"
#include "video/scn2674.h"
#include "screen.h"
#include "speaker.h"


namespace {

class epic14e_state : public driver_device
{
public:
	epic14e_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via")
		, m_speaker(*this, "speaker")
		, m_charram(*this, "charram")
		, m_attrram(*this, "attrram")
		, m_chargen(*this, "chargen")
		, m_dsw(*this, "DSW%u", 1U)
		, m_pa_select(0)
		, m_invert_screen(false)
	{
	}

	void epic14e(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 pa_r();
	void pa_w(u8 data);
	void pb_w(u8 data);
	void ca2_w(int state);

	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, u8 data);

	void cpu_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_attrram;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<3> m_dsw;

	u8 m_pa_select;
	bool m_invert_screen;
};


void epic14e_state::machine_start()
{
	save_item(NAME(m_pa_select));
	save_item(NAME(m_invert_screen));
}

SCN2672_DRAW_CHARACTER_MEMBER(epic14e_state::draw_character)
{
	const u8 chardata = m_chargen[charcode << 4 | linecount];
	u16 dots = ((chardata & 0x7f) << 2) | (BIT(chardata, 7) ? 3 : 0);
	if (m_invert_screen)
		dots = ~dots;

	for (int i = 0; i < 9; i++)
	{
		bitmap.pix(y, x++) = BIT(dots, 8) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
}


u8 epic14e_state::pa_r()
{
	u8 state = 0x8f; // TODO: PA7 = shift data from keyboard?
	for (int n = 0; n < 3; n++)
		if (!BIT(m_dsw[n]->read(), m_pa_select))
			state |= 0x10 << n;
	return state;
}

void epic14e_state::pa_w(u8 data)
{
	m_pa_select = data & 0x0f;
}

void epic14e_state::pb_w(u8 data)
{
	m_speaker->level_w(BIT(data, 7));
}

void epic14e_state::ca2_w(int state)
{
	m_invert_screen = !state;
}


u8 epic14e_state::vram_r(offs_t offset)
{
	return (BIT(offset, 0) ? m_charram : m_attrram)[offset >> 1];
}

void epic14e_state::vram_w(offs_t offset, u8 data)
{
	(BIT(offset, 0) ? m_charram : m_attrram)[offset >> 1] = data;
}

void epic14e_state::cpu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x3fff).rw(FUNC(epic14e_state::vram_r), FUNC(epic14e_state::vram_w));
	map(0x6000, 0x6007).rw("pvtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x8000, 0x8003).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x9000, 0x9003).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xa010, 0xa01f).m(m_via, FUNC(via6522_device::map));
	map(0xe000, 0xffff).rom().region("program", 0);
}

void epic14e_state::char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
}

void epic14e_state::attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("attrram");
}


static INPUT_PORTS_START(epic14e)
	PORT_START("DSW1")
	PORT_DIPNAME(0x0200, 0x0200, "Modem Stop Bits") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x0200, "1")
	PORT_DIPSETTING(0x0000, "2")
	PORT_DIPNAME(0x0100, 0x0100, "Modem Data Bits") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x0000, "7")
	PORT_DIPSETTING(0x0100, "8")
	PORT_DIPNAME(0x0080, 0x0080, "Modem Protocol") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x0080, "X-ON/X-OFF")
	PORT_DIPSETTING(0x0000, "CTS")
	PORT_DIPNAME(0x0078, 0x0008, "Modem Baud Rate") PORT_DIPLOCATION("SW1:7,6,5,4")
	PORT_DIPSETTING(0x0070, "50")
	PORT_DIPSETTING(0x0068, "75")
	PORT_DIPSETTING(0x0060, "110")
	PORT_DIPSETTING(0x0058, "135")
	PORT_DIPSETTING(0x0050, "150")
	PORT_DIPSETTING(0x0048, "300")
	PORT_DIPSETTING(0x0040, "600")
	PORT_DIPSETTING(0x0038, "1200")
	PORT_DIPSETTING(0x0030, "1800")
	PORT_DIPSETTING(0x0028, "2400")
	PORT_DIPSETTING(0x0020, "3600")
	PORT_DIPSETTING(0x0018, "4800")
	PORT_DIPSETTING(0x0010, "7200")
	PORT_DIPSETTING(0x0008, "9600")
	PORT_DIPSETTING(0x0000, "19200")
	PORT_DIPNAME(0x0007, 0x0007, "Modem Parity") PORT_DIPLOCATION("SW1:10,9,8")
	PORT_DIPSETTING(0x0007, "Disable")
	PORT_DIPSETTING(0x0006, "Odd")
	PORT_DIPSETTING(0x0004, "Even")
	PORT_DIPSETTING(0x0002, "Mark")
	PORT_DIPSETTING(0x0000, "Space")
	PORT_BIT(0xfc00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW2")
	PORT_DIPNAME(0x0200, 0x0200, "Auxiliary Stop Bits") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x0200, "1")
	PORT_DIPSETTING(0x0000, "2")
	PORT_DIPNAME(0x0100, 0x0100, "Auxiliary Data Bits") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x0000, "7")
	PORT_DIPSETTING(0x0100, "8")
	PORT_DIPNAME(0x0080, 0x0080, "Auxiliary Protocol") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x0080, "X-ON/X-OFF")
	PORT_DIPSETTING(0x0000, "DTR")
	PORT_DIPNAME(0x0078, 0x008, "Auxiliary Baud Rate") PORT_DIPLOCATION("SW2:7,6,5,4")
	PORT_DIPSETTING(0x0070, "50")
	PORT_DIPSETTING(0x0068, "75")
	PORT_DIPSETTING(0x0060, "110")
	PORT_DIPSETTING(0x0058, "135")
	PORT_DIPSETTING(0x0050, "150")
	PORT_DIPSETTING(0x0048, "300")
	PORT_DIPSETTING(0x0040, "600")
	PORT_DIPSETTING(0x0038, "1200")
	PORT_DIPSETTING(0x0030, "1800")
	PORT_DIPSETTING(0x0028, "2400")
	PORT_DIPSETTING(0x0020, "3600")
	PORT_DIPSETTING(0x0018, "4800")
	PORT_DIPSETTING(0x0010, "7200")
	PORT_DIPSETTING(0x0008, "9600")
	PORT_DIPSETTING(0x0000, "19200")
	PORT_DIPNAME(0x0007, 0x0007, "Auxiliary Parity") PORT_DIPLOCATION("SW2:10,9,8")
	PORT_DIPSETTING(0x0007, "Disable")
	PORT_DIPSETTING(0x0006, "Odd")
	PORT_DIPSETTING(0x0004, "Even")
	PORT_DIPSETTING(0x0002, "Mark")
	PORT_DIPSETTING(0x0000, "Space")
	PORT_BIT(0xfc00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW3")
	PORT_DIPNAME(0x0300, 0x0300, "Communications Mode") PORT_DIPLOCATION("SW3:2,1")
	PORT_DIPSETTING(0x0300, "Full Duplex")
	PORT_DIPSETTING(0x0200, "Half Duplex")
	PORT_DIPSETTING(0x0100, "Block")
	PORT_DIPSETTING(0x0000, "Local")
	PORT_DIPNAME(0x0080, 0x0080, "Screen Mode") PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(0x0080, "Green on Black")
	PORT_DIPSETTING(0x0000, "Black on Green")
	PORT_DIPNAME(0x0040, 0x0040, "Key Click") PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(0x0040, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0020, 0x0020, "Screen Refresh") PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(0x0000, "50 Hz")
	PORT_DIPSETTING(0x0020, "60 Hz")
	PORT_DIPNAME(0x0010, 0x0010, "Terminal Emulation") PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(0x0010, "Epic 14E")
	PORT_DIPSETTING(0x0000, "Other")
	PORT_DIPNAME(0x0008, 0x0008, "Page/Line Attributes") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(0x0008, "Page")
	PORT_DIPSETTING(0x0000, "Line")
	PORT_DIPNAME(0x0004, 0x0004, "Edit Keys") PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(0x0004, "Transmitted")
	PORT_DIPSETTING(0x0000, "Local")
	PORT_DIPNAME(0x0002, 0x0002, "Return Key") PORT_DIPLOCATION("SW3:9")
	PORT_DIPSETTING(0x0002, "CR")
	PORT_DIPSETTING(0x0000, "CR/LF")
	PORT_DIPNAME(0x0001, 0x0001, "CRT Saver") PORT_DIPLOCATION("SW3:10")
	PORT_DIPSETTING(0x0001, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_BIT(0xfc00, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void epic14e_state::epic14e(machine_config &config)
{
	M6502(config, m_maincpu, 17.01_MHz_XTAL / 9); // SY6502A (1.89 MHz confirmed)
	m_maincpu->set_addrmap(AS_PROGRAM, &epic14e_state::cpu_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	MOS6522(config, m_via, 17.01_MHz_XTAL / 9); // SY6522A
	m_via->readpa_handler().set(FUNC(epic14e_state::pa_r));
	m_via->writepa_handler().set(FUNC(epic14e_state::pa_w));
	m_via->writepb_handler().set(FUNC(epic14e_state::pb_w));
	m_via->ca2_handler().set(FUNC(epic14e_state::ca2_w));
	m_via->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);

	I8748(config, "keybmcu", 4608000).set_disable();

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(17.01_MHz_XTAL, 900, 0, 720, 315, 0, 300);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	scn2672_device &pvtc(SCN2672(config, "pvtc", 17.01_MHz_XTAL / 9));
	pvtc.intr_callback().set_inputline(m_maincpu, m6502_device::NMI_LINE);
	pvtc.set_character_width(9);
	pvtc.set_display_callback(FUNC(epic14e_state::draw_character));
	pvtc.set_addrmap(0, &epic14e_state::char_map);
	pvtc.set_addrmap(1, &epic14e_state::attr_map);
	pvtc.set_screen("screen");
	// TODO: Serial keyboard clocked at 60 Hz frame rate

	mos6551_device &acia1(MOS6551(config, "acia1", 17.01_MHz_XTAL / 9)); // SY6551A
	acia1.set_xtal(1.8432_MHz_XTAL);
	acia1.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	mos6551_device &acia2(MOS6551(config, "acia2", 17.01_MHz_XTAL / 9)); // SY6551A
	acia2.set_xtal(1.8432_MHz_XTAL); // each ACIA has its own XTAL
	acia2.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	//RS232_PORT(config, "modem", default_rs232_devices, nullptr);
	//RS232_PORT(config, "aux", default_rs232_devices, nullptr);
}


ROM_START(epic14e)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("u6.bin",  0x0000, 0x1000, CRC(014b5da0) SHA1(190c2d48c6928d143458ba094f785d40ac29f2c0))
	ROM_LOAD("u13.bin", 0x1000, 0x1000, CRC(2b406a88) SHA1(e619cc020ab5eabad99967b27cb969ceb191f5ee))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("u24.bin", 0x0000, 0x1000, CRC(da409f03) SHA1(5a90a6b865dad20dc3f455448670b4f5baa55028))

	ROM_REGION(0x0400, "keybmcu", 0)
	ROM_LOAD("246.bin", 0x0000, 0x0400, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(1982, epic14e, 0, 0, epic14e, epic14e, epic14e_state, empty_init, "Epic Computer Products", "Epic 14E (v1.0)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
