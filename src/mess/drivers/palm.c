/****************************************************************************

    drivers/palm.c
    Palm (MC68328) emulation

    Driver by MooglyGuy

    Additional bug fixing by R. Belmont

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/mc68328.h"
#include "sound/dac.h"
#include "debugger.h"
#include "machine/ram.h"
#include "rendlay.h"

class palm_state : public driver_device
{
public:
	palm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_lsi(*this, MC68328_TAG),
	m_dac(*this, "dac"),
	m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mc68328_device> m_lsi;
	required_device<dac_device> m_dac;
	required_device<ram_device> m_ram;
	// mc68328 needs modernising
	//DECLARE_WRITE8_MEMBER(palm_dac_transition);
	//DECLARE_WRITE8_MEMBER(palm_port_f_out);
	//DECLARE_READ8_MEMBER(palm_port_c_in);
	//DECLARE_READ8_MEMBER(palm_port_f_in);
	//DECLARE_WRITE16_MEMBER(palm_spim_out);
	//DECLARE_READ16_MEMBER(palm_spim_in);
	UINT8 m_port_f_latch;
	UINT16 m_spim_data;
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_INPUT_CHANGED_MEMBER(pen_check);
	DECLARE_INPUT_CHANGED_MEMBER(button_check);
	DECLARE_WRITE8_MEMBER(palm_port_f_out);
	DECLARE_READ8_MEMBER(palm_port_c_in);
	DECLARE_READ8_MEMBER(palm_port_f_in);
	DECLARE_WRITE16_MEMBER(palm_spim_out);
	DECLARE_READ16_MEMBER(palm_spim_in);
	DECLARE_WRITE8_MEMBER(palm_dac_transition);
};

static offs_t palm_dasm_override(device_t &device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);


/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

INPUT_CHANGED_MEMBER(palm_state::pen_check)
{
	UINT8 button = machine().root_device().ioport("PENB")->read();

	if(button)
		mc68328_set_penirq_line(m_lsi, 1);
	else
		mc68328_set_penirq_line(m_lsi, 0);
}

INPUT_CHANGED_MEMBER(palm_state::button_check)
{
	UINT8 button_state = machine().root_device().ioport("PORTD")->read();

	mc68328_set_port_d_lines(m_lsi, button_state, (int)(FPTR)param);
}

WRITE8_MEMBER(palm_state::palm_port_f_out)
{
	m_port_f_latch = data;
}

READ8_MEMBER(palm_state::palm_port_c_in)
{
	return 0x10;
}

READ8_MEMBER(palm_state::palm_port_f_in)
{
	return m_port_f_latch;
}

WRITE16_MEMBER(palm_state::palm_spim_out)
{
	m_spim_data = data;
}

READ16_MEMBER(palm_state::palm_spim_in)
{
	return m_spim_data;
}

static void palm_spim_exchange( device_t *device )
{
	palm_state *state = device->machine().driver_data<palm_state>();
	UINT8 x = device->machine().root_device().ioport("PENX")->read();
	UINT8 y = state->ioport("PENY")->read();

	switch( state->m_port_f_latch & 0x0f )
	{
		case 0x06:
			state->m_spim_data = (0xff - x) * 2;
			break;

		case 0x09:
			state->m_spim_data = (0xff - y) * 2;
			break;
	}
}

void palm_state::machine_start()
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	space.install_read_bank (0x000000, machine().device<ram_device>(RAM_TAG)->size() - 1, machine().device<ram_device>(RAM_TAG)->size() - 1, 0, "bank1");
	space.install_write_bank(0x000000, machine().device<ram_device>(RAM_TAG)->size() - 1, machine().device<ram_device>(RAM_TAG)->size() - 1, 0, "bank1");
	membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());

	save_item(NAME(m_port_f_latch));
	save_item(NAME(m_spim_data));

	if (m_maincpu->debug())
		m_maincpu->debug()->set_dasm_override(palm_dasm_override);
}

void palm_state::machine_reset()
{
    // Copy boot ROM
	UINT8* bios = machine().root_device().memregion("bios")->base();
	memset(machine().device<ram_device>(RAM_TAG)->pointer(), 0, machine().device<ram_device>(RAM_TAG)->size());
	memcpy(machine().device<ram_device>(RAM_TAG)->pointer(), bios, 0x20000);

	m_maincpu->reset();
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START(palm_map, AS_PROGRAM, 16, palm_state)
	AM_RANGE(0xc00000, 0xe07fff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfff000, 0xffffff) AM_DEVREADWRITE_LEGACY(MC68328_TAG, mc68328_r, mc68328_w)
ADDRESS_MAP_END


/***************************************************************************
    AUDIO HARDWARE
***************************************************************************/

WRITE8_MEMBER(palm_state::palm_dac_transition)
{
	m_dac->write_unsigned8(0x7f * data );
}


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/
static MC68328_INTERFACE(palm_dragonball_iface)
{
	"maincpu",

	DEVCB_NULL,                   // Port A Output
	DEVCB_NULL,                   // Port B Output
	DEVCB_NULL,                   // Port C Output
	DEVCB_NULL,                   // Port D Output
	DEVCB_NULL,                   // Port E Output
	DEVCB_DRIVER_MEMBER(palm_state,palm_port_f_out),// Port F Output
	DEVCB_NULL,                   // Port G Output
	DEVCB_NULL,                   // Port J Output
	DEVCB_NULL,                   // Port K Output
	DEVCB_NULL,                   // Port M Output

	DEVCB_NULL,                   // Port A Input
	DEVCB_NULL,                   // Port B Input
	DEVCB_DRIVER_MEMBER(palm_state,palm_port_c_in),// Port C Input
	DEVCB_NULL,                   // Port D Input
	DEVCB_NULL,                   // Port E Input
	DEVCB_DRIVER_MEMBER(palm_state,palm_port_f_in),// Port F Input
	DEVCB_NULL,                   // Port G Input
	DEVCB_NULL,                   // Port J Input
	DEVCB_NULL,                   // Port K Input
	DEVCB_NULL,                   // Port M Input

	DEVCB_DRIVER_MEMBER(palm_state,palm_dac_transition),

	DEVCB_DRIVER_MEMBER16(palm_state,palm_spim_out),
	DEVCB_DRIVER_MEMBER16(palm_state,palm_spim_in),
	palm_spim_exchange
};


static MACHINE_CONFIG_START( palm, palm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", M68000, 32768*506 )		/* 16.580608 MHz */
	MCFG_CPU_PROGRAM_MAP( palm_map)
	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME( ATTOSECONDS_IN_USEC(1260) )
	MCFG_QUANTUM_TIME( attotime::from_hz(60) )


	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES( VIDEO_UPDATE_BEFORE_VBLANK )
	MCFG_SCREEN_SIZE( 160, 220 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 159, 0, 219 )
	MCFG_VIDEO_START( mc68328 )
	MCFG_SCREEN_UPDATE_STATIC( mc68328 )
	MCFG_PALETTE_LENGTH( 2 )
	MCFG_PALETTE_INIT( mc68328 )
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_MC68328_ADD( palm_dragonball_iface ) // lsi device
MACHINE_CONFIG_END

static INPUT_PORTS_START( palm )
	PORT_START( "PENX" )
	PORT_BIT( 0xff, 0x50, IPT_LIGHTGUN_X ) PORT_NAME("Pen X") PORT_MINMAX(0, 0xa0) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START( "PENY" )
	PORT_BIT( 0xff, 0x50, IPT_LIGHTGUN_Y ) PORT_NAME("Pen Y") PORT_MINMAX(0, 0xa0) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, pen_check, NULL)

	PORT_START( "PORTD" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Power") PORT_CODE(KEYCODE_D)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Up") PORT_CODE(KEYCODE_Y)	  PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Down") PORT_CODE(KEYCODE_H)	PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Button 1") PORT_CODE(KEYCODE_F)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Button 2") PORT_CODE(KEYCODE_G)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Button 3") PORT_CODE(KEYCODE_J)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)5)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Button 4") PORT_CODE(KEYCODE_K)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, (void*)6)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

#define PALM_68328_BIOS \
	ROM_REGION16_BE( 0x208000, "bios", 0 )  \
	ROM_SYSTEM_BIOS( 0, "1.0e", "Palm OS 1.0 (English)" )   \
	ROMX_LOAD( "palmos10-en.rom", 0x000000, 0x080000, CRC(82030062) SHA1(00d85c6a0588133cc4651555e9605a61fc1901fc), ROM_GROUPWORD | ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 1, "2.0eper", "Palm OS 2.0 Personal (English)" ) \
	ROMX_LOAD( "palmos20-en-pers.rom", 0x000000, 0x100000, CRC(40ea8baa) SHA1(8e26e213de42da1317c375fb1f394bb945b9d178), ROM_GROUPWORD | ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 2, "2.0epro", "Palm OS 2.0 Professional (English)" ) \
	ROMX_LOAD( "palmos20-en-pro.rom", 0x000000, 0x100000, CRC(baa5b36a) SHA1(535bd9548365d300f85f514f318460443a021476), ROM_GROUPWORD | ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 3, "2.0eprod", "Palm OS 2.0 Professional (English, Debug)" ) \
	ROMX_LOAD( "palmis20-en-pro-dbg.rom", 0x000000, 0x100000, CRC(0d1d3a3b) SHA1(f18a80baa306d4d46b490589ee9a2a5091f6081c), ROM_GROUPWORD | ROM_BIOS(4) ) \
	ROM_SYSTEM_BIOS( 4, "3.0e", "Palm OS 3.0 (English)" ) \
	ROMX_LOAD( "palmos30-en.rom", 0x008000, 0x200000, CRC(6f461f3d) SHA1(7fbf592b4dc8c222be510f6cfda21d48ebe22413), ROM_GROUPWORD | ROM_BIOS(5) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 5, "3.0ed", "Palm OS 3.0 (English, Debug)" ) \
	ROMX_LOAD( "palmos30-en-dbg.rom", 0x008000, 0x200000, CRC(4deda226) SHA1(1c67d6fee2b6a4acd51cda6ef3490305730357ad), ROM_GROUPWORD | ROM_BIOS(6) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 6, "3.0g", "Palm OS 3.0 (German)" ) \
	ROMX_LOAD( "palmos30-de.rom", 0x008000, 0x200000, CRC(b991d6c3) SHA1(73e7539517b0d931e9fa99d6f6914ad46fb857b4), ROM_GROUPWORD | ROM_BIOS(7) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 7, "3.0f", "Palm OS 3.0 (French)" ) \
	ROMX_LOAD( "palmos30-fr.rom", 0x008000, 0x200000, CRC(a2a9ff6c) SHA1(7cb119f896017e76e4680510bee96207d9d28e44), ROM_GROUPWORD | ROM_BIOS(8) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 8, "3.0s", "Palm OS 3.0 (Spanish)" ) \
	ROMX_LOAD( "palmos30-sp.rom", 0x008000, 0x200000, CRC(63a595be) SHA1(f6e03a2fedf0cbe6228613f50f8e8717e797877d), ROM_GROUPWORD | ROM_BIOS(9) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 9, "3.3e", "Palm OS 3.3 (English)" ) \
	ROMX_LOAD( "palmos33-en-iii.rom", 0x008000, 0x200000, CRC(1eae0253) SHA1(e4626f1d33eca8368284d906b2152dcd28b71bbd), ROM_GROUPWORD | ROM_BIOS(10) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 10, "3.3f", "Palm OS 3.3 (French)" ) \
	ROMX_LOAD( "palmos33-fr-iii.rom", 0x008000, 0x200000, CRC(d7894f5f) SHA1(c7c90df814d4f97958194e0bc28c595e967a4529), ROM_GROUPWORD | ROM_BIOS(11) ) \
	ROM_RELOAD(0x000000, 0x004000)	\
	ROM_SYSTEM_BIOS( 11, "3.3g", "Palm OS 3.3 (German)" ) \
	ROMX_LOAD( "palmos33-de-iii.rom", 0x008000, 0x200000, CRC(a5a99c45) SHA1(209b0154942dab80b56d5e6e68fa20b9eb75f5fe), ROM_GROUPWORD | ROM_BIOS(12) ) \
	ROM_RELOAD(0x000000, 0x004000)

ROM_START( pilot1k )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "1.0e" )
ROM_END

ROM_START( pilot5k )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "1.0e" )
ROM_END

ROM_START( palmpers )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "2.0eper" )
ROM_END

ROM_START( palmpro )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "2.0epro" )
ROM_END

ROM_START( palmiii )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "3.0e" )
ROM_END

ROM_START( palmv )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.1e", "Palm OS 3.1 (English)" )
	ROMX_LOAD( "palmv31-en.rom", 0x008000, 0x200000, CRC(4656b2ae) SHA1(ec66a93441fbccfd8e0c946baa5d79c478c83e85), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "3.1g", "Palm OS 3.1 (German)" )
	ROMX_LOAD( "palmv31-de.rom", 0x008000, 0x200000, CRC(a9631dcf) SHA1(63b44d4d3fc2f2196c96d3b9b95da526df0fac77), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "3.1f", "Palm OS 3.1 (French)" )
	ROMX_LOAD( "palmv31-fr.rom", 0x008000, 0x200000, CRC(0d933a1c) SHA1(d0454f1159705d0886f8a68e1b8a5e96d2ca48f6), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 3, "3.1s", "Palm OS 3.1 (Spanish)" )
	ROMX_LOAD( "palmv31-sp.rom", 0x008000, 0x200000, CRC(cc46ca1f) SHA1(93bc78ca84d34916d7e122b745adec1068230fcd), ROM_GROUPWORD | ROM_BIOS(4) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 4, "3.1j", "Palm OS 3.1 (Japanese)" )
	ROMX_LOAD( "palmv31-jp.rom", 0x008000, 0x200000, CRC(c786db12) SHA1(4975ff2af76892370c5d4d7d6fa87a84480e79d6), ROM_GROUPWORD | ROM_BIOS(5) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 5, "3.1e2", "Palm OS 3.1 (English) v2" )
	ROMX_LOAD( "palmv31-en-2.rom", 0x008000, 0x200000, CRC(caced2bd) SHA1(95970080601f72a77a4c338203ed8809fab17abf), ROM_GROUPWORD | ROM_BIOS(6) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.1e2" )
ROM_END

ROM_START( palmvx )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.3e", "Palm OS 3.3 (English)" )
	ROMX_LOAD( "palmvx33-en.rom", 0x000000, 0x200000, CRC(3fc0cc6d) SHA1(6873d5fa99ac372f9587c769940c9b3ac1745a0a), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmvx40-en.rom", 0x000000, 0x200000, CRC(488e4638) SHA1(10a10fc8617743ebd5df19c1e99ca040ac1da4f5), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmvx41-en.rom", 0x000000, 0x200000, CRC(e59f4dff) SHA1(5e3000db318eeb8cd1f4d9729d0c9ebca560fa4a), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_DEFAULT_BIOS( "4.1e" )
ROM_END

ROM_START( palmiiic )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.5eb", "Palm OS 3.5 (English) beta" )
	ROMX_LOAD( "palmiiic350-en-beta.rom", 0x008000, 0x200000, CRC(d58521a4) SHA1(508742ea1e078737666abd4283cf5e6985401c9e), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "3.5c", "Palm OS 3.5 (Chinese)" )
	ROMX_LOAD( "palmiiic350-ch.rom", 0x008000, 0x200000, CRC(a9779f3a) SHA1(1541102cd5234665233072afe8f0e052134a5334), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmiiic40-en.rom", 0x008000, 0x200000, CRC(6b2a5ad2) SHA1(54321dcaedcc80de57a819cfd599d8d1b2e26eeb), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.0e" )
ROM_END

ROM_START( palmm100 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.51e", "Palm OS 3.5.1 (English)" )
	ROMX_LOAD( "palmm100-351-en.rom", 0x008000, 0x200000, CRC(ae8dda60) SHA1(c46248d6f05cb2f4337985610cedfbdc12ac47cf), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.51e" )
ROM_END

ROM_START( palmm130 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmm130-40-en.rom", 0x008000, 0x400000, CRC(58046b7e) SHA1(986057010d62d5881fba4dede2aba0d4d5008b16), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.0e" )
ROM_END

ROM_START( palmm505 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmos40-en-m505.rom", 0x008000, 0x400000, CRC(822a4679) SHA1(a4f5e9f7edb1926647ea07969200c5c5e1521bdf), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmos41-en-m505.rom", 0x008000, 0x400000, CRC(d248202a) SHA1(65e1bd08b244c589b4cd10fe573e0376aba90e5f), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.1e" )
ROM_END

ROM_START( palmm515 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmos41-en-m515.rom", 0x008000, 0x400000, CRC(6e143436) SHA1(a0767ea26cc493a3f687525d173903fef89f1acb), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.0e" )
ROM_END

ROM_START( visor )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.52e", "Palm OS 3.5.2 (English)" )
	ROMX_LOAD( "visor-352-en.rom", 0x008000, 0x200000, CRC(c9e55271) SHA1(749e9142f4480114c5e0d7f21ea354df7273ac5b), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.52e" )
ROM_END

ROM_START( spt1500 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.1pim", "Version 4.1 (pim)" )
	ROMX_LOAD( "spt1500v41-pim.rom",	  0x008000, 0x200000, CRC(29e50eaf) SHA1(3e920887bdf74f8f83935977b02f22d5217723eb), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "4.1pimn", "Version 4.1 (pimnoft)" )
	ROMX_LOAD( "spt1500v41-pimnoft.rom",  0x008000, 0x200000, CRC(4b44f284) SHA1(4412e946444706628b94d2303b02580817e1d370), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "4.1nopimn", "Version 4.1 (nopimnoft)" )
	ROMX_LOAD( "spt1500v41-nopimnoft.rom",0x008000, 0x200000, CRC(4ba19190) SHA1(d713c1390b82eb4e5fbb39aa10433757c5c49e02), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

ROM_START( spt1700 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "1.03pim", "Version 1.03 (pim)" )
	ROMX_LOAD( "spt1700v103-pim.rom",	 0x008000, 0x200000, CRC(9df4ee50) SHA1(243a19796f15219cbd73e116f7dfb236b3d238cd), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

ROM_START( spt1740 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "1.03pim", "Version 1.03 (pim)" )
	ROMX_LOAD( "spt1740v103-pim.rom",	 0x008000, 0x200000, CRC(c29f341c) SHA1(b56d7f8a0c15b1105972e24ed52c846b5e27b195), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "1.03pimn", "Version 1.03 (pimnoft)" )
	ROMX_LOAD( "spt1740v103-pimnoft.rom", 0x008000, 0x200000, CRC(b2d49d5c) SHA1(c133dc021b6797cdb93b666c5b315b00b5bb0917), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "1.03nopim", "Version 1.03 (nopim)" )
	ROMX_LOAD( "spt1740v103-nopim.rom",   0x008000, 0x200000, CRC(8ea7e652) SHA1(2a4b5d6a426e627b3cb82c47109cfe2497eba29a), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

static MACHINE_CONFIG_DERIVED( pilot1k, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("512K,1M,2M,4M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pilot5k, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,4M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( palmpro, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( palmiii, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( palmv, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( palmvx, palm )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")
MACHINE_CONFIG_END

/*    YEAR  NAME      PARENT    COMPAT   MACHINE      INPUT     INIT     COMPANY   FULLNAME */
COMP( 1996, pilot1k,  0,        0,       pilot1k,     palm, driver_device,     0,     "U.S. Robotics", "Pilot 1000", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
COMP( 1996, pilot5k,  pilot1k,  0,       pilot5k,     palm, driver_device,     0,     "U.S. Robotics", "Pilot 5000", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
COMP( 1997, palmpers, pilot1k,  0,       pilot5k,     palm, driver_device,     0,     "U.S. Robotics", "Palm Pilot Personal", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
COMP( 1997, palmpro,  pilot1k,  0,       palmpro,     palm, driver_device,     0,     "U.S. Robotics", "Palm Pilot Pro", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
COMP( 1998, palmiii,  pilot1k,  0,       palmiii,     palm, driver_device,     0,     "3Com", "Palm III", GAME_SUPPORTS_SAVE | GAME_NO_SOUND )
COMP( 1998, palmiiic, pilot1k,  0,       palmiii,     palm, driver_device,     0,     "Palm Inc", "Palm IIIc", GAME_NOT_WORKING )
COMP( 2000, palmm100, pilot1k,  0,       palmiii,     palm, driver_device,     0,     "Palm Inc", "Palm m100", GAME_NOT_WORKING )
COMP( 2000, palmm130, pilot1k,  0,       palmiii,     palm, driver_device,     0,     "Palm Inc", "Palm m130", GAME_NOT_WORKING )
COMP( 2001, palmm505, pilot1k,  0,       palmiii,     palm, driver_device,     0,     "Palm Inc", "Palm m505", GAME_NOT_WORKING )
COMP( 2001, palmm515, pilot1k,  0,       palmiii,     palm, driver_device,     0,     "Palm Inc", "Palm m515", GAME_NOT_WORKING )
COMP( 1999, palmv,    pilot1k,  0,       palmv,       palm, driver_device,     0,     "3Com", "Palm V", GAME_NOT_WORKING )
COMP( 1999, palmvx,   pilot1k,  0,       palmvx,      palm, driver_device,     0,     "Palm Inc", "Palm Vx", GAME_NOT_WORKING )
COMP( 2001, visor,    pilot1k,  0,       palmvx,      palm, driver_device,     0,     "Handspring", "Visor Edge", GAME_NOT_WORKING )
COMP( 19??, spt1500,  pilot1k,  0,       palmvx,      palm, driver_device,     0,     "Symbol", "SPT 1500", GAME_NOT_WORKING )
COMP( 19??, spt1700,  pilot1k,  0,       palmvx,      palm, driver_device,     0,     "Symbol", "SPT 1700", GAME_NOT_WORKING )
COMP( 19??, spt1740,  pilot1k,  0,       palmvx,      palm, driver_device,     0,     "Symbol", "SPT 1740", GAME_NOT_WORKING )

#include "palm_dbg.c"
