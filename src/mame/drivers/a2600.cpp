// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol
/***************************************************************************

  Atari VCS 2600 driver

TODO:
- Move the 2 32-in-1 rom dumps into their own driver
- Add 128-in-1 driver

***************************************************************************/

// the new RIOT does not work with the SuperCharger
// for example "mame64 a2600 scharger -cass offifrog" fails to load after playing the tape
#define USE_NEW_RIOT 0


#include "emu.h"

#include "cpu/m6502/m6507.h"
#include "sound/tiaintf.h"
#include "video/tia.h"
#include "bus/vcs/vcs_slot.h"
#include "bus/vcs/rom.h"
#include "bus/vcs/dpc.h"
#include "bus/vcs/harmony_melody.h"
#include "bus/vcs/scharger.h"
#include "bus/vcs/compumat.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "softlist.h"


#if USE_NEW_RIOT
#include "machine/mos6530n.h"
#else
#include "machine/6532riot.h"
#endif


#define CONTROL1_TAG    "joyport1"
#define CONTROL2_TAG    "joyport2"



class a2600_state : public driver_device
{
public:
	a2600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_riot_ram(*this, "riot_ram"),
		m_joy1(*this, CONTROL1_TAG),
		m_joy2(*this, CONTROL2_TAG) ,
		m_cart(*this, "cartslot"),
		m_tia(*this, "tia_video"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_swb(*this, "SWB"),
		m_riot(*this,"riot")
	{ }

	required_shared_ptr<UINT8> m_riot_ram;
	UINT16 m_current_screen_height;

	DECLARE_MACHINE_START(a2600);
	DECLARE_WRITE8_MEMBER(switch_A_w);
	DECLARE_READ8_MEMBER(switch_A_r);
	DECLARE_WRITE8_MEMBER(switch_B_w);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);
	DECLARE_READ8_MEMBER(riot_input_port_8_r);
	DECLARE_READ16_MEMBER(a2600_read_input_port);
	DECLARE_READ8_MEMBER(a2600_get_databus_contents);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback);
	DECLARE_WRITE16_MEMBER(a2600_tia_vsync_callback_pal);
	DECLARE_WRITE8_MEMBER(cart_over_tia_w);
	// investigate how the carts mapped here (Mapper JVP) interact with the RIOT device
	DECLARE_READ8_MEMBER(cart_over_riot_r);
	DECLARE_WRITE8_MEMBER(cart_over_riot_w);
	DECLARE_READ8_MEMBER(cart_over_all_r);
	DECLARE_WRITE8_MEMBER(cart_over_all_w);

protected:
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<vcs_cart_slot_device> m_cart;
	required_device<tia_video_device> m_tia;

	required_device<m6507_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_ioport m_swb;
#if USE_NEW_RIOT
	required_device<mos6532_t> m_riot;
#else
	required_device<riot6532_device> m_riot;
#endif

};



#define MASTER_CLOCK_NTSC   3579545
#define MASTER_CLOCK_PAL    3546894
#define CATEGORY_SELECT     16

static const UINT16 supported_screen_heights[4] = { 262, 312, 328, 342 };


static ADDRESS_MAP_START(a2600_mem, AS_PROGRAM, 8, a2600_state ) // 6507 has 13-bit address space, 0x0000 - 0x1fff
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0f00) AM_DEVREADWRITE("tia_video", tia_video_device, read, write)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0d00) AM_RAM AM_SHARE("riot_ram")
#if USE_NEW_RIOT
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d00) AM_DEVICE("riot", mos6532_t, io_map)
#else
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d00) AM_DEVREADWRITE("riot", riot6532_device, read, write)
#endif
	// AM_RANGE(0x1000, 0x1fff) is cart data and it is configured at reset time, depending on the mounted cart!
ADDRESS_MAP_END


READ8_MEMBER(a2600_state::cart_over_all_r)
{
	if (!space.debugger_access())
		m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;
	UINT8 ret = 0x00;

	if (masked_offset < 0x80)
	{
		ret = m_tia->read(space, masked_offset&0x7f);
	}
	else if (masked_offset < 0x100)
	{
		ret = m_riot_ram[masked_offset & 0x7f];
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		ret = m_tia->read(space, masked_offset&0x7f);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		ret = m_riot->io_r(space, masked_offset);
#else
		ret = m_riot->read(space, masked_offset);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */

	return ret;
}

WRITE8_MEMBER(a2600_state::cart_over_all_w)
{
	m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;

	if (masked_offset < 0x80)
	{
		m_tia->write(space, masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x100)
	{
		m_riot_ram[masked_offset & 0x7f] = data;
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		m_tia->write(space, masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		m_riot->io_w(space, masked_offset, data);
#else
		m_riot->write(space, masked_offset, data);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */
}

WRITE8_MEMBER(a2600_state::switch_A_w)
{
	/* Left controller port */
	m_joy1->joy_w( data >> 4 );

	/* Right controller port */
	m_joy2->joy_w( data & 0x0f );

//  switch( ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
//  {
//  case 0x0a:  /* KidVid voice module */
//      m_cassette->change_state(( data & 0x02 ) ? (cassette_state)CASSETTE_MOTOR_DISABLED : (cassette_state)(CASSETTE_MOTOR_ENABLED | CASSETTE_PLAY), (cassette_state)CASSETTE_MOTOR_DISABLED );
//      break;
//  }
}

READ8_MEMBER(a2600_state::switch_A_r)
{
	UINT8 val = 0;

	/* Left controller port PINs 1-4 ( 4321 ) */
	val |= ( m_joy1->joy_r() & 0x0F ) << 4;

	/* Right controller port PINs 1-4 ( 4321 ) */
	val |= m_joy2->joy_r() & 0x0F;

	return val;
}

WRITE8_MEMBER(a2600_state::switch_B_w)
{
}

WRITE_LINE_MEMBER(a2600_state::irq_callback)
{
}

READ8_MEMBER(a2600_state::riot_input_port_8_r)
{
	return m_swb->read();
}


READ16_MEMBER(a2600_state::a2600_read_input_port)
{
	switch( offset )
	{
	case 0: /* Left controller port PIN 5 */
		return m_joy1->pot_x_r();

	case 1: /* Left controller port PIN 9 */
		return m_joy1->pot_y_r();

	case 2: /* Right controller port PIN 5 */
		return m_joy2->pot_x_r();

	case 3: /* Right controller port PIN 9 */
		return m_joy2->pot_y_r();

	case 4: /* Left controller port PIN 6 */
		return ( m_joy1->joy_r() & 0x20 ) ? 0xff : 0x7f;

	case 5: /* Right controller port PIN 6 */
		return ( m_joy2->joy_r() & 0x20 ) ? 0xff : 0x7f;
	}
	return 0xff;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
READ8_MEMBER(a2600_state::a2600_get_databus_contents)
{
	UINT16  last_address, prev_address;
	UINT8   last_byte, prev_byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	last_address = m_maincpu->pc() + 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = prog_space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = prog_space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{   /* LDA (XX),Y */
		return prog_space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },   /* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },   /* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },   /* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }    /* 342 */
};
#endif

WRITE16_MEMBER(a2600_state::a2600_tia_vsync_callback)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_NTSC ) * 228 * m_current_screen_height );
			}
		}
	}
}

WRITE16_MEMBER(a2600_state::a2600_tia_vsync_callback_pal)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_PAL ) * 228 * m_current_screen_height );
			}
		}
	}
}

// TODO: is this the correct behavior for the real hardware?!?
READ8_MEMBER(a2600_state::cart_over_riot_r)
{
	if (!space.debugger_access())
		m_cart->write_bank(space, offset, 0);
	return m_riot_ram[0x20 + offset];
}

WRITE8_MEMBER(a2600_state::cart_over_riot_w)
{
	m_cart->write_bank(space, offset, 0);
	m_riot_ram[0x20 + offset] = data;

}

WRITE8_MEMBER(a2600_state::cart_over_tia_w)
{
	// Both Cart & TIA see these addresses
	m_cart->write_bank(space, offset, data);
	m_tia->write(space, offset, data);
}

MACHINE_START_MEMBER(a2600_state,a2600)
{
	m_current_screen_height = m_screen->height();
	memset(m_riot_ram, 0x00, 0x80);

	switch (m_cart->get_cart_type())
	{
		case A26_2K:
		case A26_4K:
		case A26_F4:
		case A26_F8:
		case A26_F8SW:
		case A26_FA:
		case A26_E0:
		case A26_E7:
		case A26_CV:
		case A26_DC:
		case A26_FV:
		case A26_8IN1:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_F6:
		case A26_DPC:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(vcs_cart_slot_device::cart_opbase),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_FE:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_ram),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x01fe, 0x01ff, read8_delegate(FUNC(vcs_cart_slot_device::read_bank),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x01fe, 0x01fe, write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_3E:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_ram),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8_delegate(FUNC(a2600_state::cart_over_tia_w), this));
			break;
		case A26_3F:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8_delegate(FUNC(a2600_state::cart_over_tia_w), this));
			break;
		case A26_UA:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x200, 0x27f, read8_delegate(FUNC(vcs_cart_slot_device::read_bank),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_JVP:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			// to verify the actual behavior...
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfa0, 0xfc0, read8_delegate(FUNC(a2600_state::cart_over_riot_r), this), write8_delegate(FUNC(a2600_state::cart_over_riot_w), this));
			break;
		case A26_4IN1:
		case A26_32IN1:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_SS:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_CM:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart));
			break;
		case A26_X07:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0000, 0x0fff, read8_delegate(FUNC(a2600_state::cart_over_all_r), this), write8_delegate(FUNC(a2600_state::cart_over_all_w), this));
			break;
		case A26_HARMONY:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(FUNC(vcs_cart_slot_device::read_rom),(vcs_cart_slot_device*)m_cart), write8_delegate(FUNC(vcs_cart_slot_device::write_bank),(vcs_cart_slot_device*)m_cart));
			break;
	}

	/* Banks may have changed, reset the cpu so it uses the correct reset vector */
	m_maincpu->reset();

	save_item(NAME(m_current_screen_height));
}


#ifdef UNUSED_FUNCTIONS
// try to detect 2600 controller setup. returns 32bits with left/right controller info
unsigned a2600_state::long detect_2600controllers()
{
#define JOYS 0x001
#define PADD 0x002
#define KEYP 0x004
#define LGUN 0x008
#define INDY 0x010
#define BOOS 0x020
#define KVID 0x040
#define CMTE 0x080
#define MLNK 0x100
#define AMSE 0x200
#define CX22 0x400
#define CX80 0x800

	unsigned int left,right;
	int i,j,foundkeypad = 0;
	UINT8 *cart;
	static const unsigned char signatures[][5] =  {
		{ 0x55, 0xa5, 0x3c, 0x29, 0}, // star raiders
		{ 0xf9, 0xff, 0xa5, 0x80, 1}, // sentinel
		{ 0x81, 0x02, 0xe8, 0x86, 1}, // shooting arcade
		{ 0x02, 0xa9, 0xec, 0x8d, 1}, // guntest4 tester
		{ 0x85, 0x2c, 0x85, 0xa7, 2}, // INDY 500
		{ 0xa1, 0x8d, 0x9d, 0x02, 2}, // omega race INDY
		{ 0x65, 0x72, 0x44, 0x43, 2}, // Sprintmaster INDY
		{ 0x89, 0x8a, 0x99, 0xaa, 3}, // omega race
		{ 0x9a, 0x8e, 0x81, 0x02, 4},
		{ 0xdd, 0x8d, 0x80, 0x02, 4},
		{ 0x85, 0x8e, 0x81, 0x02, 4},
		{ 0x8d, 0x81, 0x02, 0xe6, 4},
		{ 0xff, 0x8d, 0x81, 0x02, 4},
		{ 0xa9, 0x03, 0x8d, 0x81, 5},
		{ 0xa9, 0x73, 0x8d, 0x80, 6},
		//                                  { 0x82, 0x02, 0x85, 0x8f, 7}, // Mind Maze (really Mind Link??)
		{ 0xa9, 0x30, 0x8d, 0x80, 7}, // Bionic Breakthrough
		{ 0x02, 0x8e, 0x81, 0x02, 7}, // Telepathy
		{ 0x41, 0x6d, 0x69, 0x67, 9}, // Missile Command Amiga Mouse
		{ 0x43, 0x58, 0x2d, 0x32, 10}, // Missile Command CX22 TrackBall
		{ 0x43, 0x58, 0x2d, 0x38, 11}, // Missile Command CX80 TrackBall
		{ 0x4e, 0xa8, 0xa4, 0xa2, 12}, // Omega Race for Joystick ONLY
		{ 0xa6, 0xef, 0xb5, 0x38, 8} // Warlords.. paddles ONLY
	};
	// start with this.. if anyone finds a game that does NOT work with both controllers enabled
	// it can be fixed here with a new signature (note that the Coleco Gemini has this setup also)
	left = JOYS+PADD; right = JOYS+PADD;
	// default for bad dumps and roms too large to have special controllers
	if ((m_cart_size > 0x4000) || (m_cart_size & 0x7ff)) return (left << 16) + right;

	cart = CART;
	for (i = 0; i < m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
	{
		for (j = 0; j < (sizeof signatures/sizeof signatures[0]); j++)
		{
			if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0] - 1))
			{
				int k = signatures[j][4];
				if (k == 0) return (JOYS << 16) + KEYP;
				if (k == 1) return (LGUN << 16);
				if (k == 2) return (INDY << 16) + INDY;
				if (k == 3) return (BOOS << 16) + BOOS;
				if (k == 5) return (JOYS << 16) + KVID;
				if (k == 6) return (CMTE << 16) + CMTE;
				if (k == 7) return (MLNK << 16) + MLNK;
				if (k == 8) return (PADD << 16) + PADD;
				if (k == 9) return (AMSE << 16) + AMSE;
				if (k == 10) return (CX22 << 16) + CX22;
				if (k == 11) return (CX80 << 16) + CX80;
				if (k == 12) return (JOYS << 16) + JOYS;
				if (k == 4) foundkeypad = 1;
			}
		}
	}
	if (foundkeypad) return (KEYP << 16) + KEYP;
	return (left << 16) + right;
}
#endif


static INPUT_PORTS_START( a2600 )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )
INPUT_PORTS_END


static SLOT_INTERFACE_START(a2600_cart)
	SLOT_INTERFACE_INTERNAL("a26_2k",    A26_ROM_2K)
	SLOT_INTERFACE_INTERNAL("a26_4k",    A26_ROM_4K)
	SLOT_INTERFACE_INTERNAL("a26_f4",    A26_ROM_F4)
	SLOT_INTERFACE_INTERNAL("a26_f6",    A26_ROM_F6)
	SLOT_INTERFACE_INTERNAL("a26_f8",    A26_ROM_F8)
	SLOT_INTERFACE_INTERNAL("a26_f8sw",  A26_ROM_F8_SW)
	SLOT_INTERFACE_INTERNAL("a26_fa",    A26_ROM_FA)
	SLOT_INTERFACE_INTERNAL("a26_fe",    A26_ROM_FE)
	SLOT_INTERFACE_INTERNAL("a26_3e",    A26_ROM_3E)
	SLOT_INTERFACE_INTERNAL("a26_3f",    A26_ROM_3F)
	SLOT_INTERFACE_INTERNAL("a26_e0",    A26_ROM_E0)
	SLOT_INTERFACE_INTERNAL("a26_e7",    A26_ROM_E7)
	SLOT_INTERFACE_INTERNAL("a26_ua",    A26_ROM_UA)
	SLOT_INTERFACE_INTERNAL("a26_cv",    A26_ROM_CV)
	SLOT_INTERFACE_INTERNAL("a26_dc",    A26_ROM_DC)
	SLOT_INTERFACE_INTERNAL("a26_fv",    A26_ROM_FV)
	SLOT_INTERFACE_INTERNAL("a26_jvp",   A26_ROM_JVP)
	SLOT_INTERFACE_INTERNAL("a26_cm",    A26_ROM_COMPUMATE)
	SLOT_INTERFACE_INTERNAL("a26_ss",    A26_ROM_SUPERCHARGER)
	SLOT_INTERFACE_INTERNAL("a26_dpc",   A26_ROM_DPC)
	SLOT_INTERFACE_INTERNAL("a26_4in1",  A26_ROM_4IN1)
	SLOT_INTERFACE_INTERNAL("a26_8in1",  A26_ROM_8IN1)
	SLOT_INTERFACE_INTERNAL("a26_32in1", A26_ROM_32IN1)
	SLOT_INTERFACE_INTERNAL("a26_x07",    A26_ROM_X07)
	SLOT_INTERFACE_INTERNAL("a26_harmony",   A26_ROM_HARMONY)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(a2600_cartslot)
	MCFG_VCS_CARTRIDGE_ADD("cartslot", a2600_cart, nullptr)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a2600")
	MCFG_SOFTWARE_LIST_ADD("cass_list","a2600_cass")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( a2600, a2600_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6507, MASTER_CLOCK_NTSC / 3)
	MCFG_M6502_DISABLE_DIRECT()
	MCFG_CPU_PROGRAM_MAP(a2600_mem)

	MCFG_MACHINE_START_OVERRIDE(a2600_state,a2600)

	/* video hardware */
	MCFG_DEVICE_ADD("tia_video", TIA_NTSC_VIDEO, 0)
	MCFG_TIA_READ_INPUT_PORT_CB(READ16(a2600_state, a2600_read_input_port))
	MCFG_TIA_DATABUS_CONTENTS_CB(READ8(a2600_state, a2600_get_databus_contents))
	MCFG_TIA_VSYNC_CB(WRITE16(a2600_state, a2600_tia_vsync_callback))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK_NTSC, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)
	MCFG_SCREEN_PALETTE("tia_video:palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_TIA_ADD("tia", MASTER_CLOCK_NTSC/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	/* devices */
#if USE_NEW_RIOT
	MCFG_DEVICE_ADD("riot", MOS6532n, MASTER_CLOCK_NTSC / 3)
	MCFG_MOS6530n_IN_PA_CB(READ8(a2600_state, switch_A_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(a2600_state, switch_A_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(a2600_state, riot_input_port_8_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(a2600_state, switch_B_w))
	MCFG_MOS6530n_IRQ_CB(WRITELINE(a2600_state, irq_callback))
#else
	MCFG_DEVICE_ADD("riot", RIOT6532, MASTER_CLOCK_NTSC / 3)
	MCFG_RIOT6532_IN_PA_CB(READ8(a2600_state, switch_A_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(a2600_state, switch_A_w))
	MCFG_RIOT6532_IN_PB_CB(READ8(a2600_state, riot_input_port_8_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(a2600_state, switch_B_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(a2600_state, irq_callback))
#endif

	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, "joy")
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)

	MCFG_FRAGMENT_ADD(a2600_cartslot)
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "NTSC")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( a2600p, a2600_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6507, MASTER_CLOCK_PAL / 3)
	MCFG_CPU_PROGRAM_MAP(a2600_mem)
	MCFG_M6502_DISABLE_DIRECT()

	MCFG_MACHINE_START_OVERRIDE(a2600_state,a2600)

	/* video hardware */
	MCFG_DEVICE_ADD("tia_video", TIA_PAL_VIDEO, 0)
	MCFG_TIA_READ_INPUT_PORT_CB(READ16(a2600_state, a2600_read_input_port))
	MCFG_TIA_DATABUS_CONTENTS_CB(READ8(a2600_state, a2600_get_databus_contents))
	MCFG_TIA_VSYNC_CB(WRITE16(a2600_state, a2600_tia_vsync_callback_pal))


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31 )
	MCFG_SCREEN_UPDATE_DEVICE("tia_video", tia_video_device, screen_update)
	MCFG_SCREEN_PALETTE("tia_video:palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_TIA_ADD("tia", MASTER_CLOCK_PAL/114)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	/* devices */
#if USE_NEW_RIOT
	MCFG_DEVICE_ADD("riot", MOS6532n, MASTER_CLOCK_PAL / 3)
	MCFG_MOS6530n_IN_PA_CB(READ8(a2600_state, switch_A_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(a2600_state, switch_A_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(a2600_state, riot_input_port_8_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(a2600_state, switch_B_w))
	MCFG_MOS6530n_IRQ_CB(WRITELINE(a2600_state, irq_callback))
#else
	MCFG_DEVICE_ADD("riot", RIOT6532, MASTER_CLOCK_PAL / 3)
	MCFG_RIOT6532_IN_PA_CB(READ8(a2600_state, switch_A_r))
	MCFG_RIOT6532_OUT_PA_CB(WRITE8(a2600_state, switch_A_w))
	MCFG_RIOT6532_IN_PB_CB(READ8(a2600_state, riot_input_port_8_r))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(a2600_state, switch_B_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(a2600_state, irq_callback))
#endif

	MCFG_VCS_CONTROL_PORT_ADD(CONTROL1_TAG, vcs_control_port_devices, "joy")
	MCFG_VCS_CONTROL_PORT_ADD(CONTROL2_TAG, vcs_control_port_devices, nullptr)

	MCFG_FRAGMENT_ADD(a2600_cartslot)
	MCFG_SOFTWARE_LIST_FILTER("cart_list", "PAL")
MACHINE_CONFIG_END


ROM_START( a2600 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
ROM_END

#define rom_a2600p rom_a2600

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY     FULLNAME */
CONS( 1977, a2600,  0,      0,      a2600,  a2600, driver_device,   0,      "Atari",    "Atari 2600 (NTSC)" , MACHINE_SUPPORTS_SAVE )
CONS( 1978, a2600p, a2600,  0,      a2600p, a2600, driver_device,   0,      "Atari",    "Atari 2600 (PAL)",   MACHINE_SUPPORTS_SAVE )
