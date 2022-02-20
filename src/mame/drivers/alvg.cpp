// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/******************************************************************************************************
PINBALL
Alvin G pinball machines.

Undumped PAL16L8 (U29) on sound card.

Pinball games:
- Al's Garage Band Goes on a World Tour
- Dual-Pool (unreleased)
- Monte Carlo
- Mystery Castle
- Pistol Poker
- Punchy the Clown
- Slam 'n Jam (unreleased)
- Tropical Isle
Other games:
- A-MAZE-ING Baseball (redemption)
- A G Football (head to head)
- A G Soccer-ball (head to head)
- Dinosaur Eggs (redemption)
- The Death Dealing Adventure of Maxx Badazz Champion Kickboxer (unreleased)
- USA Football (redemption)
- USA Football (head to head)

Status:
- Skeletons

ToDo:
- Add bsmt-based sound card
- Display
- Inputs not working
- Mechanical sounds

****************************************************************************************************/
#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "speaker.h"


namespace {

class alvg_state : public genpin_class
{
public:
	alvg_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_via(*this, "via")
		, m_via0(*this, "via0")
		, m_via1(*this, "via1")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void alvg(machine_config &config);

private:
	void main_map(address_map &map);
	void audio_map(address_map &map);
	void machine_start() override;
	void machine_reset() override;
	void display_w(offs_t, u8);
	void ppi0_pa_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[i] = BIT(data, i); }
	void ppi0_pb_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[8U+i] = BIT(data, i); }
	void ppi0_pc_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[16U+i] = BIT(data, i); }
	void ppi1_pa_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[24U+i] = BIT(data, i); }
	void ppi1_pb_w(u8 data) { m_row = (m_row & 0xff00) | data; }
	void ppi1_pc_w(u8 data) { m_row = (m_row & 0xff) | (data << 8); }
	void ppi2_pa_w(u8 data) { m_lamp_data = (m_lamp_data & 0xff00) | data; }
	void ppi2_pb_w(u8 data) { m_lamp_data = (m_lamp_data & 0xff) | (data << 8); }
	void ppi2_pc_w(u8 data);
	void via_pb_w(u8 data);
	u8 via0_pa_r();
	u8 via0_pb_r() { return m_io_keyboard[12]->read(); }
	void via1_pb_w(u8 data);

	u16 m_row = 0U;
	u16 m_lamp_data = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<via6522_device> m_via;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_ioport_array<13> m_io_keyboard;
	output_finder<128> m_io_outputs;   // 32 solenoids + 96 lamps
};


void alvg_state::main_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2003).mirror(0x3f0).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U12
	map(0x2400, 0x2403).mirror(0x3f0).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U13
	map(0x2800, 0x2803).mirror(0x3f0).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U14
	map(0x2c00, 0x2cff).mirror(0x300).w(FUNC(alvg_state::display_w));
	map(0x3800, 0x380f).mirror(0x3f0).m("via1", FUNC(via6522_device::map)); // U8
	map(0x3c00, 0x3c0f).mirror(0x3f0).m("via0", FUNC(via6522_device::map)); // U7
}

void alvg_state::audio_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x2000, 0x2001).mirror(0xffe).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x3000, 0x37ff).mirror(0x800).ram();
	map(0x4000, 0x4fff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x5000, 0x500f).mirror(0xff0).m("via", FUNC(via6522_device::map));
	map(0x6000, 0x6fff).noprw(); // watchdog
}

static INPUT_PORTS_START( alvg )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP10") // Tilt on Mystery Castle, Punchy, Pistol Pete
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Test+")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Test-")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP56")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP60")
	// From here, these inputs only used by Pistol Pete
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("INP65")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("INP66")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("INP68")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("INP69")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("INP70")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("INP71")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("INP72")

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP73")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP74")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP75")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP76")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP77")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP78")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP79")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP80")

	PORT_START("X10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X11")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X12") // DIAGS
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test")
INPUT_PORTS_END

void alvg_state::display_w(offs_t offset, u8 data)
{
	//printf("%X:%X ",offset,data);
}

void alvg_state::ppi2_pc_w(u8 data)
{
	for (u8 i = 0; i < 12; i++)
		if (BIT(m_lamp_data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[24U+8*i+j] = BIT(data, j);
}

void alvg_state::via_pb_w(u8 data)
{
	m_via1->write_ca1(BIT(data, 1));
}

void alvg_state::via1_pb_w(u8 data)
{
	m_via->write_ca2(BIT(data, 1));
}

u8 alvg_state::via0_pa_r()
{
	u8 data = 0xff;
	for (u8 i = 0; i < 12; i++)
		if (!BIT(m_row, i))
			data &= m_io_keyboard[i]->read();

	return data;
}

void alvg_state::machine_start()
{
	genpin_class::machine_start();

	//m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_row));
	save_item(NAME(m_lamp_data));
}

void alvg_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void alvg_state::alvg(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &alvg_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MOS6522(config, m_via0, XTAL(4'000'000) / 2);  // U7, uses clock2 from maincpu; switch inputs
	m_via0->readpa_handler().set(FUNC(alvg_state::via0_pa_r));
	m_via0->readpb_handler().set(FUNC(alvg_state::via0_pb_r));
	//m_via0->writepa_handler().set(FUNC(alvg_state::via0_pa_w));
	//m_via0->writepb_handler().set(FUNC(alvg_state::via0_pb_w));
	//m_via0->ca2_handler().set_nop();
	m_via0->cb2_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_via0->irq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);

	MOS6522(config, m_via1, XTAL(4'000'000) / 2);  // U8, uses clock2 from maincpu; port A = to sound; port B = serial to display
	//m_via1->readpb_handler().set(FUNC(alvg_state::via1_pb_r));
	m_via1->writepa_handler().set("via", FUNC(via6522_device::write_pa));
	m_via1->writepb_handler().set(FUNC(alvg_state::via1_pb_w));
	//m_via1->ca2_handler().set_nop();
	//m_via1->cb2_handler().set_nop();
	m_via1->irq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);

	I8255A(config, m_ppi0); // U12
	m_ppi0->out_pa_callback().set(FUNC(alvg_state::ppi0_pa_w)); // Solenoids
	m_ppi0->out_pb_callback().set(FUNC(alvg_state::ppi0_pb_w)); // Solenoids
	m_ppi0->out_pc_callback().set(FUNC(alvg_state::ppi0_pc_w)); // Solenoids

	I8255A(config, m_ppi1); // U13
	m_ppi1->out_pa_callback().set(FUNC(alvg_state::ppi1_pa_w)); // Solenoids
	m_ppi1->out_pb_callback().set(FUNC(alvg_state::ppi1_pb_w)); // Switch rows
	m_ppi1->out_pc_callback().set(FUNC(alvg_state::ppi1_pc_w)); // Switch rows

	I8255A(config, m_ppi2); // U14
	m_ppi2->out_pa_callback().set(FUNC(alvg_state::ppi2_pa_w)); // Lamps
	m_ppi2->out_pb_callback().set(FUNC(alvg_state::ppi2_pb_w)); // Lamps
	m_ppi2->out_pc_callback().set(FUNC(alvg_state::ppi2_pc_w)); // Lamps

	// Sound
	MC6809(config, m_audiocpu, XTAL(8'000'000)); // 68B09, 8 MHz crystal, internal divide by 4 to produce E/Q outputs
	m_audiocpu->set_addrmap(AS_PROGRAM, &alvg_state::audio_map);
	MOS6522(config, m_via, XTAL(8'000'000) / 4);  // uses E clock from audiocpu; port A = read sound code; port B = ticket machine
	//m_via->readpa_handler().set(FUNC(alvg_state::via_pa_r));
	//m_via->readpb_handler().set(FUNC(alvg_state::via_pb_r));
	//m_via->writepa_handler().set(FUNC(alvg_state::via_pa_w));
	m_via->writepb_handler().set(FUNC(alvg_state::via_pb_w));
	//m_via->ca2_handler().set_nop();
	//m_via->cb2_handler().set_nop();
	m_via->irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	genpin_audio(config);

	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(8'000'000) / 2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, XTAL(8'000'000) / 8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);
}

/*----------------------------------------------------------------------------
/ A.G. Soccer Ball - A.G. Football has identical ROMs but different playfield
/----------------------------------------------------------------------------*/
ROM_START(agsoccer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("agscpu1r.18u", 0x0000, 0x10000, CRC(37affcf4) SHA1(017d47f54d5b34a4b71c2f5b84ba9bdb1c924299))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v24", 0x0000, 0x10000, CRC(4ba36e8d) SHA1(330dcb1eea8c311df0e57a3b74146601c26d63c0)) // label says 2.4, inside the ROM it says 2.5L though
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

ROM_START(agsoccera)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("agscpu1r.18u", 0x0000, 0x10000, CRC(37affcf4) SHA1(017d47f54d5b34a4b71c2f5b84ba9bdb1c924299))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v21", 0x0000, 0x10000, CRC(aa30bfe4) SHA1(518f7019639a0284461e83ad849bee0be5371580))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

ROM_START(agsoccer07)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ags_cpu_r07u", 0x0000, 0x10000, CRC(009ef717) SHA1(d770ce8fd032f4f1d96b9792509cceebbfaebbd9))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v14", 0x0000, 0x10000, CRC(2544e468) SHA1(d49e2fc91cbb80fdf96f436c614c6f305efafb6f))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Al's Garage Band Goes On A World Tour
/-------------------------------------------------------------------*/
ROM_START(wrldtour)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu27c.512", 0x0000, 0x10000, CRC(c9572fb5) SHA1(47a3e8943ef4207011a33f4a03a6e722c937cc48))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x110000, "gfx3", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
	ROM_RELOAD( 0xc0000, 0x40000)
	ROM_LOAD("dot27c.512", 0x100000, 0x10000, CRC(c8bd48e7) SHA1(e2dc513dd42c05c2018e6d8c0b6f0b2c56e6e059))
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_COPY("gfx3",0x108000,0x0000,0x8000)
ROM_END

ROM_START(wrldtour2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu02b.512", 0x0000, 0x10000, CRC(1658bf40) SHA1(7af9eedab4e7d0cedaf8bfdbc1f27b989a7171cd))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x110000, "gfx3", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
	ROM_RELOAD( 0xc0000, 0x40000)
	ROM_LOAD("dot02b.512", 0x100000, 0x10000, CRC(50e3d59d) SHA1(db6df3482fc485af6bde341750bf8072a296b8da))
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_COPY("gfx3",0x108000,0x0000,0x8000)
ROM_END

ROM_START(wrldtour3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu03.512", 0x0000, 0x10000, CRC(56dee967) SHA1(f7b1f69d96c72b0cf738bdf45701502f7306a4a0))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x110000, "gfx3", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
	ROM_RELOAD( 0xc0000, 0x40000)
	ROM_LOAD("dot03.512", 0x100000, 0x10000, CRC(f8a084bb) SHA1(30eb344ad96b5605693d3a7c703c9ed5c1770ca4))
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_COPY("gfx3",0x108000,0x0000,0x8000)
ROM_END

/*-------------------------------------------------------------------
/ Dinosaur Eggs
/-------------------------------------------------------------------*/
ROM_START(dinoeggs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinoeggs.512", 0x0000, 0x10000, CRC(4712f97f) SHA1(593351dcfd475e685c1e5eb2c1006769d3325c8b))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps071.r02", 0x0000, 0x10000, CRC(288f116c) SHA1(5d03ce66bffe39ec02173525078ff07c5005ef18))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("eps072.r02", 0x000000, 0x40000, CRC(780a4364) SHA1(d8a972debee669f0fe66c7407fbed5ef9cd2ce01))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Mystery Castle
/-------------------------------------------------------------------*/
ROM_START(mystcast)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mcastle.cpu", 0x0000, 0x10000, CRC(936e6799) SHA1(aa29fb5f12f34c695d1556232744f65cd576a2b1))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mcastle.102", 0x0000, 0x10000, CRC(752822d0) SHA1(36461ef03cac5aefa0c03dfdc63c3d294a3b9c09))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_LOAD("mcastle.du4", 0x00000, 0x10000, CRC(686e253a) SHA1(28aff34c120c61e231e2111dc396df515bcbbb89))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("mcastle.du5", 0x00000, 0x40000, CRC(9095c367) SHA1(9d3e9416f662ee2aad891eef059278c530448fcc))
	ROM_RELOAD( 0x40000, 0x40000)
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

ROM_START(mystcasta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_103.bin", 0x0000, 0x10000, CRC(70ab8ece) SHA1(2bf8cd042450968b7500552419a9af5df2589c13))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mcastle.103", 0x0000, 0x10000, CRC(bd4849ac) SHA1(f477ea369539a65c0960be1f1c3b4c5503dd6b75))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_LOAD("u4.bin", 0x00000, 0x10000, CRC(a6969efc) SHA1(82da976cb3d30d6fb1576e4c67febd7235f73f51))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("u5.bin", 0x00000, 0x40000, CRC(e5126980) SHA1(2c6d412c87bf27098dae4351958d84e8f9348423))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("u6.bin", 0x40000, 0x40000, CRC(eb241633) SHA1(8e5db75b32ed2ea74088615bbe1403d4c8feafbd))
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Pistol Poker
/-------------------------------------------------------------------*/
ROM_START(pstlpkr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("p_peteu2.512", 0x0000, 0x10000, CRC(490a1e2d) SHA1(907dd858ed948681e7366a64a0e7537ebe301d6b))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("p_pu102.512", 0x0000, 0x10000, CRC(b8fb806e) SHA1(c2dc19820ea22bbcf5808db2fb4be76a4033d6ea))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("p_parom0.c20", 0x000000, 0x40000, CRC(99986af2) SHA1(52fa7d2979f7f2d6d65ab6d4f7bbfbed16303991))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom1.c20", 0x100000, 0x40000, CRC(ae2af238) SHA1(221d3a0e3fb1daad261d723e873ef0727b88889e))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom2.c20", 0x200000, 0x40000, CRC(f39560a4) SHA1(cdfdf7b44ff4c3f9f4d39fbd8ecbf141d8568088))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom3.c20", 0x300000, 0x40000, CRC(19d5e4de) SHA1(fb59166ebf992e81b92a42898e351d8443adb1c3))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_LOAD("p_peteu4.512", 0x00000, 0x10000, CRC(caa0cabd) SHA1(caff6ca4a9cce4e3d846502696c8838805673261))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("p_peteu5.c20", 0x00000, 0x40000, CRC(1d2cecd8) SHA1(6072a0f744fb9eef728fe7cf5e17d0007edbddd7))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("p_peteu6.c20", 0x40000, 0x40000, CRC(3a56376c) SHA1(69febc17b8416c03a58e651447bbe1e14ff27e50))
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

ROM_START(pstlpkr1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2-ddff.512", 0x0000, 0x10000, CRC(83fa0595) SHA1(d6ebb0e63fd964ccaee3979a7fc13b6adf7b837c))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("p_pu102.512", 0x0000, 0x10000, CRC(b8fb806e) SHA1(c2dc19820ea22bbcf5808db2fb4be76a4033d6ea))
	ROM_REGION(0x400000, "bsmt", 0)
	ROM_LOAD("p_parom0.c20", 0x000000, 0x40000, CRC(99986af2) SHA1(52fa7d2979f7f2d6d65ab6d4f7bbfbed16303991))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom1.c20", 0x100000, 0x40000, CRC(ae2af238) SHA1(221d3a0e3fb1daad261d723e873ef0727b88889e))
	ROM_RELOAD(0x100000 + 0x40000, 0x40000)
	ROM_RELOAD(0x100000 + 0x80000, 0x40000)
	ROM_RELOAD(0x100000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom2.c20", 0x200000, 0x40000, CRC(f39560a4) SHA1(cdfdf7b44ff4c3f9f4d39fbd8ecbf141d8568088))
	ROM_RELOAD(0x200000 + 0x40000, 0x40000)
	ROM_RELOAD(0x200000 + 0x80000, 0x40000)
	ROM_RELOAD(0x200000 + 0xc0000, 0x40000)
	ROM_LOAD("p_parom3.c20", 0x300000, 0x40000, CRC(19d5e4de) SHA1(fb59166ebf992e81b92a42898e351d8443adb1c3))
	ROM_RELOAD(0x300000 + 0x40000, 0x40000)
	ROM_RELOAD(0x300000 + 0x80000, 0x40000)
	ROM_RELOAD(0x300000 + 0xc0000, 0x40000)
	ROM_REGION(0x20000, "dmdcpu", 0)
	ROM_LOAD("p_peteu4.512", 0x00000, 0x10000, CRC(caa0cabd) SHA1(caff6ca4a9cce4e3d846502696c8838805673261))
	ROM_REGION(0x100000, "gfx3", 0)
	ROM_LOAD("p_peteu5.c20", 0x00000, 0x40000, CRC(1d2cecd8) SHA1(6072a0f744fb9eef728fe7cf5e17d0007edbddd7))
	ROM_RELOAD( 0x80000, 0x40000)
	ROM_LOAD("p_peteu6.c20", 0x40000, 0x40000, CRC(3a56376c) SHA1(69febc17b8416c03a58e651447bbe1e14ff27e50))
	ROM_RELOAD( 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Punchy The Clown
/-------------------------------------------------------------------*/
ROM_START(punchy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("epc061.r02", 0x0000, 0x10000, CRC(732fca88) SHA1(dff0aa4b856bafb95b08dae675dd2ad59e1860e1))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps061.r02", 0x0000, 0x10000, CRC(cfde1b9a) SHA1(cbf9e67df6a6762843272493c2caa1413f70fb27))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("eps062.r02", 0x000000, 0x40000, CRC(7462a5cd) SHA1(05141bcc91b1a786444bff7fa8ba2a785dc0d376))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

ROM_START(punchy3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("epc061.r03", 0x0000, 0x10000, CRC(8e91131c) SHA1(1bf1408e4e512b764048f4847cf8e4b7a0bf824d))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps061.r02", 0x0000, 0x10000, CRC(cfde1b9a) SHA1(cbf9e67df6a6762843272493c2caa1413f70fb27))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("eps062.r02", 0x000000, 0x40000, CRC(7462a5cd) SHA1(05141bcc91b1a786444bff7fa8ba2a785dc0d376))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ U.S.A. Football
/-------------------------------------------------------------------*/
ROM_START(usafootb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usa_cpu.bin", 0x0000, 0x10000, CRC(53b00873) SHA1(96812c4722026554a830c62eca64f09d25a0de82))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("usa_snd.bin", 0x0000, 0x10000, CRC(9d509cbc) SHA1(0be629945b5102adf75e88661e0f956e32ca77da))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("usa_vox.bin", 0x000000, 0x40000, CRC(baae0aa3) SHA1(7933bffcf1509ceeea58a4449268c10c9fac554c))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

ROM_START(usafootba)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usa_cpu1.bin", 0x0000, 0x10000, CRC(3b64a6e9) SHA1(65535bc17395416181bafddc61c0fac177eeba2f))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("usa_snd.bin", 0x0000, 0x10000, CRC(9d509cbc) SHA1(0be629945b5102adf75e88661e0f956e32ca77da))
	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("usa_vox.bin", 0x000000, 0x40000, CRC(baae0aa3) SHA1(7933bffcf1509ceeea58a4449268c10c9fac554c))
	ROM_RELOAD(0x000000 + 0x40000, 0x40000)
	ROM_RELOAD(0x000000 + 0x80000, 0x40000)
	ROM_RELOAD(0x000000 + 0xc0000, 0x40000)
ROM_END

} // Anonymous namespace


GAME( 1991, agsoccer,   0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R18u, 2.5L sound)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1991, agsoccera,  agsoccer, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R18u, 2.1 sound)",           MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1991, agsoccer07, agsoccer, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R07u)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1992, wrldtour,   0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour",        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1992, wrldtour2,  wrldtour, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour (R02b)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1992, wrldtour3,  wrldtour, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour (R06a)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, usafootb,   0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "U.S.A. Football",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, usafootba,  usafootb, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "U.S.A. Football (R01u)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, mystcast,   0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Mystery Castle (R02)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, mystcasta,  mystcast, alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Mystery Castle (R03)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, pstlpkr,    0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Pistol Poker (R02)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, pstlpkr1,   pstlpkr,  alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Pistol Poker (R01)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, punchy,     0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Punchy The Clown (R02)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, punchy3,    punchy,   alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Punchy The Clown (R03)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1993, dinoeggs,   0,        alvg, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Dinosaur Eggs",                                MACHINE_IS_SKELETON_MECHANICAL)
