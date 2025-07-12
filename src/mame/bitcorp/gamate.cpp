// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at 2007, 2014
 Peter Wilhelmsen peter.wilhelmsen@gmail.com
 Morten Shearman Kirkegaard morten+gamate@afdelingp.dk
 Juan Felix Mateos vectrex@hackermesh.org

 A complete hardware description can be found at
 http://blog.kevtris.org/blogfiles/Gamate%20Inside.txt

 ******************************************************************************/

#include "emu.h"
#include "gamate_v.h"
#include "sound/ay8910.h"
#include "bus/gamate/slot.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class gamate_state : public driver_device
{
public:
	gamate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "ay8910")
		, m_cartslot(*this, "cartslot")
		, m_io_joy(*this, "JOY")
		, m_bios(*this, "maincpu")
		, m_ram(*this, "ram")
	{ }

	void gamate(machine_config &config);

	void init_gamate();

private:
	uint8_t card_available_check();
	uint8_t card_available_set();
	void card_reset(uint8_t data);

	uint8_t gamate_nmi_r();
	void sound_w(offs_t offset, uint8_t data);
	uint8_t sound_r(offs_t offset);
	void write_cart(offs_t offset, uint8_t data);
	uint8_t read_cart(offs_t offset);

	TIMER_CALLBACK_MEMBER(gamate_timer);
	TIMER_CALLBACK_MEMBER(gamate_timer2);

	void gamate_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	int m_card_available;

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<gamate_cart_slot_device> m_cartslot;
	required_ioport m_io_joy;
	required_region_ptr<uint8_t> m_bios;
	required_shared_ptr<uint8_t> m_ram;
	emu_timer *timer1;
	emu_timer *timer2;
};

/* todo: what are these really, do they go to the cartridge slot? */
uint8_t gamate_state::card_available_check()
{
	// bits 0 and 1 checked
	return m_card_available ? 3: 1;
}

void gamate_state::card_reset(uint8_t data)
{
	// might reset the card / protection?
}

uint8_t gamate_state::card_available_set()
{
	if (!machine().side_effects_disabled())
		m_card_available = 1;
	return 0;
}

// serial connection
uint8_t gamate_state::gamate_nmi_r()
{
	uint8_t data=0;
	logerror("nmi/4800 read\n");
	return data;
}

uint8_t gamate_state::sound_r(offs_t offset)
{
	m_ay->address_w(offset);
	return m_ay->data_r();
}

void gamate_state::sound_w(offs_t offset, uint8_t data)
{
	m_ay->address_w(offset);
	m_ay->data_w(data);
}

void gamate_state::write_cart(offs_t offset, uint8_t data)
{
	m_cartslot->write_cart(offset, data);
}

uint8_t gamate_state::read_cart(offs_t offset)
{
	return m_cartslot->read_cart(offset);
}

void gamate_state::gamate_mem(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1c00).ram().share("ram");
	map(0x4000, 0x400f).mirror(0x03f0).rw(FUNC(gamate_state::sound_r), FUNC(gamate_state::sound_w));
	map(0x4400, 0x4400).mirror(0x03ff).portr("JOY");
	map(0x4800, 0x4800).mirror(0x03ff).r(FUNC(gamate_state::gamate_nmi_r));
	map(0x5000, 0x5007).mirror(0x03f8).m("video", FUNC(gamate_video_device::regs_map));
	map(0x5800, 0x5800).r(FUNC(gamate_state::card_available_set));
	map(0x5900, 0x5900).w(FUNC(gamate_state::card_reset));
	map(0x5a00, 0x5a00).r(FUNC(gamate_state::card_available_check));
	map(0x6000, 0xdfff).rw(FUNC(gamate_state::read_cart), FUNC(gamate_state::write_cart));

	map(0xe000, 0xefff).mirror(0x1000).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( gamate )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start/Pause")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
INPUT_PORTS_END

void gamate_state::init_gamate()
{
	timer1 = timer_alloc(FUNC(gamate_state::gamate_timer), this);
	timer2 = timer_alloc(FUNC(gamate_state::gamate_timer2), this);
}

void gamate_state::machine_start()
{
	memset(m_ram, 0xff, m_ram.bytes());  /* memory seems to contain 0xff at power up */
	timer2->enable(true);
	timer2->reset(m_maincpu->cycles_to_attotime(1000));

	save_item(NAME(m_card_available));
}

void gamate_state::machine_reset()
{
	m_card_available = 0;
}

TIMER_CALLBACK_MEMBER(gamate_state::gamate_timer)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	timer1->enable(false);
}

TIMER_CALLBACK_MEMBER(gamate_state::gamate_timer2)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	timer1->enable(true);
	timer1->reset(m_maincpu->cycles_to_attotime(10/* cycles short enought to clear irq line early enough*/));
	timer2->enable(true);
	timer2->reset(m_maincpu->cycles_to_attotime(32768/2));
}

void gamate_state::gamate(machine_config &config)
{
	M6502(config, m_maincpu, 4433000/2); // NCR 65CX02
	m_maincpu->set_addrmap(AS_PROGRAM, &gamate_state::gamate_mem);

	GAMATE_VIDEO(config, "video", 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front(); // Stereo headphone output

	AY8910(config, m_ay, 4433000 / 4); // AY compatible, no actual AY chip present
	m_ay->add_route(0, "speaker", 0.5, 0);
	m_ay->add_route(1, "speaker", 0.5, 1);
	m_ay->add_route(2, "speaker", 0.25, 0);
	m_ay->add_route(2, "speaker", 0.25, 1);

	GAMATE_CART_SLOT(config, m_cartslot, gamate_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gamate");
}


/* ROM notes:
gamate_bios_umc.bin is called UMC or NCR ICASC00002
gamate_bios_bit.bin is called BIT ICASC00001
So basically the UMC UA6588F and NCR 81489 CPU's contains the ICASC00002 bios
while the BIT branded CPU contains the ICASC00001 bios.
They're compatible, but for completeness its nice to have both.
Note i have 8 gamate consoles (dated 1990 though 1993) which has the gamate_bios_umc.bin in it
and only 1 dated 1994 which has the gamate_bios_bit.bin in it, so the former seems much more common.
We dumped the BIOS from all our Gamate consoles, and all except one were
identical (SHA1:ea449dc607601f9a68d855ad6ab53800d2e99297):
Gamate_BIOS_9027__9002008__UMC_UA6588F_9027S_606700.bin
Gamate_BIOS_9027__9142222__UMC_UA6588F_9027S_606700.bin
Gamate_BIOS_9027__unknown__UMC_UA6588F_9027S_606690.bin
Gamate_BIOS_9031__9009719__NCR_81489_BIT_WS39323F_ICASC00002_F841400_R9031.bin
Gamate_BIOS_9038__9145157__NCR_81489_BIT_WS39323F_ICASC00002_F842247_N9038.bin
One console, with an unknown serial number, has an updated BIOS
(SHA1:4e9dfbfe916ca485530ef4221593ab68738e2217):
This console appears to have been manufactured in 1994, based on the date markings on the RAM chips,
as well as the PCB.
*/
ROM_START(gamate)
	ROM_REGION(0x1000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "default", "DEFAULT")
	ROMX_LOAD("gamate_bios_umc.bin", 0x0000, 0x1000, CRC(07090415) SHA1(ea449dc607601f9a68d855ad6ab53800d2e99297), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "newer", "NEWER")
	ROMX_LOAD("gamate_bios_bit.bin", 0x0000, 0x1000, CRC(03a5f3a7) SHA1(4e9dfbfe916ca485530ef4221593ab68738e2217), ROM_BIOS(1))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY     FULLNAME  FLAGS
CONS( 1990, gamate, 0,      0,      gamate,  gamate, gamate_state, init_gamate, "Bit Corp", "Gamate", 0 )
