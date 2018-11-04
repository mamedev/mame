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
#include "sound/ay8910.h"
#include "bus/gamate/slot.h"
#include "cpu/m6502/m6502.h"
#include "video/gamate.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class gamate_state : public driver_device
{
public:
	gamate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "ay8910")
		, m_cartslot(*this, "cartslot")
		, m_io_joy(*this, "JOY")
		, m_bios(*this, "bios")
	{ }

	DECLARE_PALETTE_INIT(gamate);

	DECLARE_READ8_MEMBER(card_available_check);
	DECLARE_READ8_MEMBER(card_available_set);
	DECLARE_WRITE8_MEMBER(card_reset);

	DECLARE_READ8_MEMBER(gamate_nmi_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(write_cart);
	DECLARE_READ8_MEMBER(read_cart);

	DECLARE_DRIVER_INIT(gamate);

	uint32_t screen_update_gamate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(gamate_timer);
	TIMER_CALLBACK_MEMBER(gamate_timer2);

	void gamate(machine_config &config);
	void gamate_mem(address_map &map);
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	int m_card_available;

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<gamate_cart_slot_device> m_cartslot;
	required_ioport m_io_joy;
	required_shared_ptr<uint8_t> m_bios;
	emu_timer *timer1;
	emu_timer *timer2;
};

/* todo: what are these really, do they go to the cartridge slot? */
READ8_MEMBER( gamate_state::card_available_check )
{
	// bits 0 and 1 checked
	return m_card_available ? 3: 1;
}

WRITE8_MEMBER( gamate_state::card_reset )
{
	// might reset the card / protection?
}

READ8_MEMBER( gamate_state::card_available_set )
{
	m_card_available = 1;
	return 0;
}

// serial connection
READ8_MEMBER( gamate_state::gamate_nmi_r )
{
	uint8_t data=0;
	logerror("nmi/4800 read\n");
	return data;
}

READ8_MEMBER(gamate_state::sound_r)
{
	m_ay->address_w(space, 0, offset);
	return m_ay->data_r(space, 0);
}

WRITE8_MEMBER(gamate_state::sound_w)
{
	m_ay->address_w(space, 0, offset);
	m_ay->data_w(space, 0, data);
}

WRITE8_MEMBER(gamate_state::write_cart)
{
	m_cartslot->write_cart(space, offset, data);
}

READ8_MEMBER(gamate_state::read_cart)
{
	return m_cartslot->read_cart(space, offset);
}

ADDRESS_MAP_START(gamate_state::gamate_mem)
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0x03f0) AM_READWRITE(sound_r,sound_w)
	AM_RANGE(0x4400, 0x4400) AM_MIRROR(0x03ff) AM_READ_PORT("JOY")
	AM_RANGE(0x4800, 0x4800) AM_MIRROR(0x03ff) AM_READ(gamate_nmi_r)
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x03f8) AM_DEVICE("video", gamate_video_device, regs_map)
	AM_RANGE(0x5800, 0x5800) AM_READ(card_available_set)
	AM_RANGE(0x5900, 0x5900) AM_WRITE(card_reset)
	AM_RANGE(0x5a00, 0x5a00) AM_READ(card_available_check)
	AM_RANGE(0x6000, 0xdfff) AM_READWRITE(read_cart, write_cart)

	AM_RANGE(0xe000, 0xefff) AM_MIRROR(0x1000) AM_ROM AM_SHARE("bios") AM_REGION("maincpu",0)
ADDRESS_MAP_END


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

DRIVER_INIT_MEMBER(gamate_state,gamate)
{
	timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer),this));
	timer2 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gamate_state::gamate_timer2),this));
}

void gamate_state::machine_start()
{
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

MACHINE_CONFIG_START(gamate_state::gamate)
	MCFG_CPU_ADD("maincpu", M6502, 4433000/2) // NCR 65CX02
	MCFG_CPU_PROGRAM_MAP(gamate_mem)

	MCFG_GAMATE_VIDEO_ADD("video")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") // Stereo headphone output
	MCFG_SOUND_ADD("ay8910", AY8910, 4433000 / 4) // AY compatible, no actual AY chip present
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
	MCFG_SOUND_ROUTE(2, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)

	MCFG_GAMATE_CARTRIDGE_ADD("cartslot", gamate_cart, nullptr)

	MCFG_SOFTWARE_LIST_ADD("cart_list","gamate")
MACHINE_CONFIG_END


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
	ROMX_LOAD("gamate_bios_umc.bin", 0x0000, 0x1000, CRC(07090415) SHA1(ea449dc607601f9a68d855ad6ab53800d2e99297), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "newer", "NEWER")
	ROMX_LOAD("gamate_bios_bit.bin", 0x0000, 0x1000, CRC(03a5f3a7) SHA1(4e9dfbfe916ca485530ef4221593ab68738e2217), ROM_BIOS(2) )
ROM_END


//    YEAR  NAME     PARENT  COMPAT    MACHINE  INPUT   CLASS         INIT    COMPANY     FULLNAME  FLAGS
CONS( 1990, gamate,  0,      0,        gamate,  gamate, gamate_state, gamate, "Bit Corp", "Gamate", 0 )
