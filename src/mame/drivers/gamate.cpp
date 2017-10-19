// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at 2007, 2014
 Peter Wilhelmsen peter.wilhelmsen@gmail.com
 Morten Shearman Kirkegaard morten+gamate@afdelingp.dk
 Juan Felix Mateos vectrex@hackermesh.org

 A complete hardware description can be found at
 http://blog.kevtris.org/blogfiles/Gamate%20Inside.txt

 nmi unknown
 bomb blast top status line missing
 ******************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
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
		, m_cart(*this, "cartslot")
		, m_io_joy(*this, "JOY")
		, m_bios(*this, "bios")
		, m_bank(*this, "bank")
		, m_bankmulti(*this, "bankmulti")
	{ }

	DECLARE_PALETTE_INIT(gamate);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_READ8_MEMBER(newer_protection_set);
	DECLARE_WRITE8_MEMBER(protection_reset);
	DECLARE_READ8_MEMBER(gamate_cart_protection_r);
	DECLARE_WRITE8_MEMBER(gamate_cart_protection_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitchmulti_w);
	DECLARE_WRITE8_MEMBER(cart_bankswitch_w);
	DECLARE_READ8_MEMBER(gamate_nmi_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_DRIVER_INIT(gamate);
	uint32_t screen_update_gamate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gamate_interrupt);
	TIMER_CALLBACK_MEMBER(gamate_timer);
	TIMER_CALLBACK_MEMBER(gamate_timer2);

private:
	virtual void machine_start() override;

	struct
	{
		bool set;
		int bit_shifter;
		uint8_t cartridge_byte;
		uint16_t address; // in reality something more like short local cartridge address offset
		bool unprotected;
		bool failed;
	} card_protection;

	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<generic_slot_device> m_cart;
	required_ioport m_io_joy;
	required_shared_ptr<uint8_t> m_bios;
	required_memory_bank m_bank;
	required_memory_bank m_bankmulti;
	emu_timer *timer1;
	emu_timer *timer2;
	uint8_t bank_multi;
	uint8_t *m_cart_ptr;
};

WRITE8_MEMBER( gamate_state::gamate_cart_protection_w )
{
	logerror("%.6f protection write %x %x address:%x data:%x shift:%d\n",machine().time().as_double(), offset, data, card_protection.address, card_protection.cartridge_byte, card_protection.bit_shifter);

	switch (offset)
	{
	case 0:
		card_protection.failed= card_protection.failed || ((card_protection.cartridge_byte&0x80)!=0) != ((data&4)!=0);
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter>=8)
		{
			card_protection.cartridge_byte=m_cart_ptr[card_protection.address++];
			card_protection.bit_shifter=0;
		}
		break;
	}
}

READ8_MEMBER( gamate_state::gamate_cart_protection_r )
{
	uint8_t ret=1;
	if (card_protection.bit_shifter==7 && card_protection.unprotected)
	{
		ret=m_cart_ptr[bank_multi*0x4000];
	}
	else
	{
		card_protection.bit_shifter++;
		if (card_protection.bit_shifter==8)
		{
			card_protection.bit_shifter=0;
			card_protection.cartridge_byte='G';
			card_protection.unprotected=true;
		}
		ret=(card_protection.cartridge_byte&0x80) ? 2 : 0;
		if (card_protection.bit_shifter==7 && !card_protection.failed)
		{ // now protection chip on cartridge activates cartridge chip select on cpu accesses
//          m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, READ8_DELEGATE(gamate_state, gamate_cart_protection_r)); // next time I will try to get this working
		}
		card_protection.cartridge_byte<<=1;
	}
	logerror("%.6f protection read %x %x address:%x data:%x shift:%d\n",machine().time().as_double(), offset, ret, card_protection.address, card_protection.cartridge_byte, card_protection.bit_shifter);
	return ret;
}

READ8_MEMBER( gamate_state::protection_r )
{
	return card_protection.set? 3: 1;
} // bits 0 and 1 checked

WRITE8_MEMBER( gamate_state::protection_reset )
{
// writes 0x20
	card_protection.address=0x6005-0x6001;
	card_protection.bit_shifter=0;
	card_protection.cartridge_byte=m_cart_ptr[card_protection.address++]; //m_cart_rom[card_protection.address++];
	card_protection.failed=false;
	card_protection.unprotected=false;
}

READ8_MEMBER( gamate_state::newer_protection_set )
{
	card_protection.set=true;
	return 0;
}

WRITE8_MEMBER( gamate_state::cart_bankswitchmulti_w )
{
	bank_multi=data;
	m_bankmulti->set_base(m_cart_ptr+0x4000*data+1);
}

WRITE8_MEMBER( gamate_state::cart_bankswitch_w )
{
	m_bank->set_base(m_cart_ptr+0x4000*data);
}

READ8_MEMBER( gamate_state::gamate_nmi_r )
{
	uint8_t data=0;
	popmessage("nmi/4800 read\n");
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

static ADDRESS_MAP_START( gamate_mem, AS_PROGRAM, 8, gamate_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM 
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0x03f0) AM_READWRITE(sound_r,sound_w)
	AM_RANGE(0x4400, 0x4400) AM_MIRROR(0x03ff) AM_READ_PORT("JOY")
	AM_RANGE(0x4800, 0x4800) AM_MIRROR(0x03ff) AM_READ(gamate_nmi_r)
	AM_RANGE(0x5000, 0x5007) AM_MIRROR(0x03f8) AM_DEVICE("video", gamate_video_device, regs_map)
	AM_RANGE(0x5800, 0x5800) AM_READ(newer_protection_set)
	AM_RANGE(0x5900, 0x5900) AM_WRITE(protection_reset)
	AM_RANGE(0x5a00, 0x5a00) AM_READ(protection_r)
	AM_RANGE(0x6001, 0x9fff) AM_READ_BANK("bankmulti")
	AM_RANGE(0xa000, 0xdfff) AM_READ_BANK("bank")
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(gamate_cart_protection_r, gamate_cart_protection_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(cart_bankswitchmulti_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(cart_bankswitch_w)
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
	m_cart_ptr = memregion("maincpu")->base() + 0x6000;
	if (m_cart->exists())
	{
//      m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6000, READ8_DELEGATE(gamate_state, gamate_cart_protection_r));
		m_cart_ptr = m_cart->get_rom_base();
		m_bankmulti->set_base(m_cart->get_rom_base()+1);
		m_bank->set_base(m_cart->get_rom_base()+0x4000); // bankswitched games in reality no offset
	}
//  m_bios[0xdf1]=0xea; m_bios[0xdf2]=0xea; // default bios: $47 protection readback
	card_protection.set=false;
	bank_multi=0;
	card_protection.unprotected=false;
	timer2->enable(true);
	timer2->reset(m_maincpu->cycles_to_attotime(1000));
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

static MACHINE_CONFIG_START( gamate )
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

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "gamate_cart")
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
