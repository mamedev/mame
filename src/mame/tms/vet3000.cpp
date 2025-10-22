// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

	VET 3000, "The Video Effects Titler"

	https://www.youtube.com/watch?v=DJXlqe2UXKs
	https://datassette.org/node/106385


	Notes:
        ------------------------------------------------------------
	There's some model and company info in the back of the unit:

		VET 3000
		TMS - Tecnologia em
		Micro Sistemas
		CGC 52.733.847/0001-89
		Made in Brazil

        ------------------------------------------------------------
	There are 2 rear video connectors (In and Out) but the video
	input is currently not supported.

        ------------------------------------------------------------
	There's also a 36-pin (2*18) pcb edge rear connector labeled
	"interface" but the purpose of this is not know yet and so
	it is currently not documented in this driver.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "video/tms9928a.h"
#include "vet3000.lh"

namespace {

class vet3000_state : public driver_device
{
public:
	vet3000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_row(*this, "ROW%u", 1U)
		, m_scan(0xff)
	{ }

	void vet3000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void program_map(address_map &map) ATTR_COLD;
	u8 keyboard_r();
	void keyboard_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_ioport_array<7> m_row;

	u8 m_scan;
};


void vet3000_state::machine_start()
{
	save_item(NAME(m_scan));
};


void vet3000_state::keyboard_w(u8 data)
{
	m_scan = data;
};


u8 vet3000_state::keyboard_r()
{
	u8 value = 0xff;
	for (u8 i=0; i<7; i++)
		if (!BIT(m_scan, i))
			value &= m_row[i]->read();
	return value;
}


void vet3000_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x8000, 0x8001).rw("tms9128", FUNC(tms9128_device::read), FUNC(tms9128_device::write));
	map(0x8002, 0x8002).rw(FUNC(vet3000_state::keyboard_r), FUNC(vet3000_state::keyboard_w));
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( vet3000 )
	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_N)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_M)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EXT/MODE") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_SPACE)

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BORDER BLK") PORT_CODE(KEYCODE_COMMA)

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME(":") PORT_CODE(KEYCODE_COLON) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CURSOR") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("AUTO CENTER") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("COLOR") PORT_CODE(KEYCODE_STOP)

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DOWN/UP") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OBJ") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PAGE") PORT_CODE(KEYCODE_SLASH)

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LEFT/RIGHT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_B)
INPUT_PORTS_END


void vet3000_state::vet3000(machine_config &config)
{
	static constexpr XTAL MAIN_CLOCK = 3.579545_MHz_XTAL;

	/* basic machine hardware */
	MC6809(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vet3000_state::program_map);

	/* video hardware */
	tms9128_device &vdp(TMS9128(config, "tms9128", MAIN_CLOCK)); /* TMS9128NL on the board */
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	config.set_default_layout(layout_vet3000);
}


ROM_START( vet3000 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "vet2.1-tms_vet3000_27128a.bin", 0x0000, 0x4000, CRC(bfdef5fa) SHA1(cd4da3cbda7fa12c9413d052bf69ee758cfe68b3) )
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS                   INIT        COMPANY                         FULLNAME                   FLAGS
SYST( 1988, vet3000, 0,      0,       vet3000, vet3000, vet3000_state, empty_init, "Tecnologia em Micro Sistemas", "VET 3000 (Brazil, v2.1)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
