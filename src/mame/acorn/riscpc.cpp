// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Tomasz Slanina, Sarah Walker
/***************************************************************************

    Acorn RiscPC line of computers

    TODO:
    - IOMD currently hardwired with ARM7500FE flavour for all machines, needs information about
      which uses what;
    - PS/2 keyboard doesn't work properly;
    - Fix pendingUnd fatalerror from ARM7 core;
    - Fix pendingAbtD fatalerror for RiscOS 4.xx;

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/acorn_vidc.h"
#include "machine/arm_iomd.h"
#include "machine/i2cmem.h"
#include "machine/at_keybc.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "debugger.h"


namespace {

class riscpc_state : public driver_device
{
public:
	riscpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vidc(*this, "vidc")
		, m_iomd(*this, "iomd")
		, m_screen(*this, "screen")
		, m_i2cmem(*this, "i2cmem")
		, m_kbdc(*this, "kbdc")
	{ }

	void rpc700(machine_config &config);
	void rpc600(machine_config &config);
	void sarpc(machine_config &config);
	void sarpc_j233(machine_config &config);
	void a7000(machine_config &config);
	void a7000p(machine_config &config);

private:
	void base_config(machine_config &config);

	required_device<cpu_device> m_maincpu;
	required_device<arm_vidc20_device> m_vidc;
	required_device<arm7500fe_iomd_device> m_iomd;
	required_device<screen_device> m_screen;
	required_device<i2cmem_device> m_i2cmem;
	required_device<ps2_keyboard_controller_device> m_kbdc;

	virtual void machine_reset() override;
	virtual void machine_start() override;

	void a7000_map(address_map &map);
	void riscpc_map(address_map &map);

	bool m_i2cmem_clock = false;
	int iocr_od0_r();
	int iocr_od1_r();
	void iocr_od0_w(int state);
	void iocr_od1_w(int state);
};

int riscpc_state::iocr_od1_r()
{
	// TODO: presuming same as Acorn Archimedes, where i2c clock can be readback
	return (m_i2cmem_clock == true) ? 1 : 0;
}

int riscpc_state::iocr_od0_r()
{
	return (m_i2cmem->read_sda() ? 1 : 0); //eeprom read
}

void riscpc_state::iocr_od0_w(int state)
{
	m_i2cmem->write_sda(state == true ? 1 : 0);
}

void riscpc_state::iocr_od1_w(int state)
{
	m_i2cmem_clock = state;
	m_i2cmem->write_scl(state == true ? 1 : 0);
}

void riscpc_state::a7000_map(address_map &map)
{
	map(0x00000000, 0x003fffff).mirror(0x00800000).rom().region("user1", 0);
//  map(0x01000000, 0x01ffffff).noprw(); //expansion ROM
	//
//  map(0x02000000, 0x027fffff).mirror(0x00800000).ram(); // VRAM, not installed on A7000 models
//  I/O 03000000 - 033fffff
//  AM_RANGE(0x03010000, 0x03011fff) //Super IO
//  AM_RANGE(0x03012000, 0x03029fff) //FDC
//  AM_RANGE(0x0302b000, 0x0302bfff) //Network podule
//  AM_RANGE(0x03040000, 0x0304ffff) //podule space 0,1,2,3
//  AM_RANGE(0x03070000, 0x0307ffff) //podule space 4,5,6,7
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map));
	map(0x03310000, 0x03310003).portr("MOUSE");

	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
//  AM_RANGE(0x08000000, 0x08ffffff) AM_MIRROR(0x07000000) //EASI space

	map(0x10000000, 0x13ffffff).ram(); //SIMM 0 bank 0
	map(0x14000000, 0x17ffffff).ram(); //SIMM 0 bank 1
//  map(0x18000000, 0x18ffffff).mirror(0x03000000).ram(); //SIMM 1 bank 0
//  map(0x1c000000, 0x1cffffff).mirror(0x03000000).ram(); //SIMM 1 bank 1
}

void riscpc_state::riscpc_map(address_map &map)
{
	a7000_map(map);
	map(0x02000000, 0x027fffff).mirror(0x00800000).ram(); // VRAM
}


/* Input ports */
static INPUT_PORTS_START( a7000 )
//  PORT_INCLUDE( at_keyboard )

	PORT_START("MOUSE")
	// for debugging we leave video and sound HWs as options, eventually slotify them
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x01, "TV Screen" )
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Mouse Right")   PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Mouse Center")  PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Mouse Left")    PORT_CODE(MOUSECODE_BUTTON1)
	// TODO: understand condition where this occurs
	PORT_CONFNAME( 0x80, 0x00, "CMOS Reset bit" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )
	PORT_CONFNAME( 0x100, 0x000, "Sound HW" )
	PORT_CONFSETTING(    0x000, "16-bit" )
	PORT_CONFSETTING(    0x100, "8-bit" )
	PORT_BIT(0xfffffe00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void riscpc_state::machine_start()
{
	// ...
}

void riscpc_state::machine_reset()
{

}

void riscpc_state::base_config(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);

	// TODO: verify type
	pc_kbdc_device &kbd_con(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_101));
	kbd_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	kbd_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// auxiliary connector
//  pc_kbdc_device &aux_con(PC_KBDC(config, "aux", ps2_mice, STR_HLE_PS2_MOUSE));
//  aux_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
//  aux_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_data_w));

	PS2_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	m_kbdc->hot_res().set(m_iomd, FUNC(arm_iomd_device::keyboard_reset));
	m_kbdc->kbd_clk().set(kbd_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(kbd_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_kbdc->kbd_irq().set(m_iomd, FUNC(arm_iomd_device::keyboard_irq));
//  m_kbdc->aux_clk().set(aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
//  m_kbdc->aux_data().set(aux_con, FUNC(pc_kbdc_device::data_write_from_mb));
//  m_kbdc->aux_irq().set(FUNC(riscpc_state::keyboard_interrupt));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	ARM_VIDC20(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(m_iomd, FUNC(arm_iomd_device::sound_drq));

	m_iomd->set_host_cpu_tag(m_maincpu);
	m_iomd->set_vidc_tag(m_vidc);
	m_iomd->set_kbdc_tag(m_kbdc);
	m_iomd->iocr_read_od<0>().set(FUNC(riscpc_state::iocr_od0_r));
	m_iomd->iocr_read_od<1>().set(FUNC(riscpc_state::iocr_od1_r));
	m_iomd->iocr_write_od<0>().set(FUNC(riscpc_state::iocr_od0_w));
	m_iomd->iocr_write_od<1>().set(FUNC(riscpc_state::iocr_od1_w));
}

void riscpc_state::rpc600(machine_config &config)
{
	constexpr XTAL cpuxtal(60_MHz_XTAL/2);

	ARM7(config, m_maincpu, cpuxtal); // really ARM610
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::rpc700(machine_config &config)
{
	constexpr XTAL cpuxtal(80_MHz_XTAL/2);
	ARM710A(config, m_maincpu, cpuxtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::a7000(machine_config &config)
{
	constexpr XTAL cpuxtal(32'000'000);

	ARM7500(config, m_maincpu, cpuxtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::a7000_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::a7000p(machine_config &config)
{
	constexpr XTAL cpuxtal(48'000'000);

	ARM7500(config, m_maincpu, cpuxtal); // really ARM7500FE
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::a7000_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::sarpc(machine_config &config)
{
	// TODO: ranges from 160 to 233 MHz
	constexpr XTAL cpuxtal(200'000'000);

	SA1110(config, m_maincpu, cpuxtal); // StrongARM
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::sarpc_j233(machine_config &config)
{
	// TODO: 233 MHz, unsupported by xtal module
	constexpr XTAL cpuxtal(200'000'000);

	SA1110(config, m_maincpu, cpuxtal); // StrongARM
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM7500FE_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

ROM_START(rpc600)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.50
	ROM_SYSTEM_BIOS( 0, "350", "RiscOS 3.50" )
	ROMX_LOAD("0277,521-01.bin", 0x000000, 0x100000, CRC(8ba4444e) SHA1(1b31d7a6e924bef0e0056c3a00a3fed95e55b175), ROM_BIOS(0))
	ROMX_LOAD("0277,522-01.bin", 0x100000, 0x100000, CRC(2bc95c9f) SHA1(f8c6e2a1deb4fda48aac2e9fa21b9e01955331cf), ROM_BIOS(0))
ROM_END

ROM_START(rpc700)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.60
	ROM_SYSTEM_BIOS( 0, "360", "RiscOS 3.60" )
	ROMX_LOAD("1203,101-01.bin", 0x000000, 0x200000, CRC(2eeded56) SHA1(7217f942cdac55033b9a8eec4a89faa2dd63cd68), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,102-01.bin", 0x000002, 0x200000, CRC(6db87d21) SHA1(428403ed31682041f1e3d114ea02a688d24b7d94), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
ROM_END

ROM_START(a7000)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.60
	ROM_SYSTEM_BIOS( 0, "360", "RiscOS 3.60" )
	ROMX_LOAD("1203,101-01.bin", 0x000000, 0x200000, CRC(2eeded56) SHA1(7217f942cdac55033b9a8eec4a89faa2dd63cd68), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,102-01.bin", 0x000002, 0x200000, CRC(6db87d21) SHA1(428403ed31682041f1e3d114ea02a688d24b7d94), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
ROM_END

ROM_START(a7000p)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.71
	ROM_SYSTEM_BIOS( 0, "371", "RiscOS 3.71" )
	ROMX_LOAD("1203,261-01.bin", 0x000000, 0x200000, CRC(8e3c570a) SHA1(ffccb52fa8e165d3f64545caae1c349c604386e9), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,262-01.bin", 0x000002, 0x200000, CRC(cf4615b4) SHA1(c340f29aeda3557ebd34419fcb28559fc9b620f8), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	// Version 4.02
	ROM_SYSTEM_BIOS( 1, "402", "RiscOS 4.02" )
	ROMX_LOAD("riscos402_1.bin", 0x000000, 0x200000, CRC(4c32f7e2) SHA1(d290e29a4de7be9eb36cbafbb2dc99b1c4ce7f72), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("riscos402_2.bin", 0x000002, 0x200000, CRC(7292b790) SHA1(67f999c1ccf5419e0a142b7e07f809e13dfed425), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(1))
	// Version 4.39
	ROM_SYSTEM_BIOS( 2, "439", "RiscOS 4.39" )
	ROMX_LOAD("riscos439_1.bin", 0x000000, 0x200000, CRC(dab94cb8) SHA1(a81fb7f1a8117f85e82764675445092d769aa9af), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(2))
	ROMX_LOAD("riscos439_2.bin", 0x000002, 0x200000, CRC(22e6a5d4) SHA1(b73b73c87824045130840a19ce16fa12e388c039), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(2))
ROM_END

ROM_START(sarpc)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.70
	ROM_SYSTEM_BIOS( 0, "370", "RiscOS 3.70" )
	ROMX_LOAD("1203,191-01.bin", 0x000000, 0x200000, NO_DUMP, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,192-01.bin", 0x000002, 0x200000, NO_DUMP, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
ROM_END

ROM_START(sarpc_j233)
	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.71
	ROM_SYSTEM_BIOS( 0, "371", "RiscOS 3.71" )
	ROMX_LOAD("1203,261-01.bin", 0x000000, 0x200000, CRC(8e3c570a) SHA1(ffccb52fa8e165d3f64545caae1c349c604386e9), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,262-01.bin", 0x000002, 0x200000, CRC(cf4615b4) SHA1(c340f29aeda3557ebd34419fcb28559fc9b620f8), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT  CLASS         INIT        COMPANY  FULLNAME                  FLAGS */
COMP( 1994, rpc600,     0,      0,      rpc600,     a7000, riscpc_state, empty_init, "Acorn", "Risc PC 600",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1994, rpc700,     rpc600, 0,      rpc700,     a7000, riscpc_state, empty_init, "Acorn", "Risc PC 700",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1995, a7000,      rpc600, 0,      a7000,      a7000, riscpc_state, empty_init, "Acorn", "Archimedes A7000",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, a7000p,     rpc600, 0,      a7000p,     a7000, riscpc_state, empty_init, "Acorn", "Archimedes A7000+",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, sarpc,      rpc600, 0,      sarpc,      a7000, riscpc_state, empty_init, "Acorn", "StrongARM Risc PC",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, sarpc_j233, rpc600, 0,      sarpc_j233, a7000, riscpc_state, empty_init, "Acorn", "J233 StrongARM Risc PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
