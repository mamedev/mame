// license: BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Tomasz Slanina, Sarah Walker
/**************************************************************************************************

Acorn RiscPC line of computers

TODO:
- a7000 should use the plain ARM7500 IOMD flavour (ID 0x5b98) rather than the ARM7500FE one;
- rename a7000/p to aa7000/p for consistency with aa310 driver (helps from command line)

TODO (a7000p -bios 2):
- Hangs at boot with nullptr ide1:0 option (strike ESC key several times until Boot menu appears,
  then disable it in Configure machine item);
- In turn the ESC key Cancel looks too slow to catch up (verify);
- CD throws "CD drive not ready or disc not present" when mounted
  (NOTE: needs filesystem changed to CDFS in Configure machine)
- Serial mouse doesn't work even if selected;
- No VIDC10 sound even if configured in games, needs support in IOMD sound DMA;

Notes:
- CTRL + F12 brings a Task window in Risc OS 4+ when in Desktop;
- https://www.riscosopen.org/wiki/documentation/show/CLI%20Basics%20part%201#TOC1
- "Configure SoundSystem 8bit" in CLI to attempt using older VIDC10 sound system (after reboot);

**************************************************************************************************/
#include "emu.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/arm7/arm7.h"
#include "machine/acorn_vidc.h"
#include "machine/arm_iomd.h"
#include "machine/fdc37c665gt.h"
#include "machine/i2cmem.h"
#include "machine/input_merger.h"

#include "formats/acorn_dsk.h"
#include "formats/apd_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/jfd_dsk.h"
#include "formats/st_dsk.h"

#include "imagedev/floppy.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class riscpc_state : public driver_device
{
public:
	riscpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vidc(*this, "vidc")
		, m_iomd(*this, "iomd")
		, m_superio(*this, "superio")
		, m_ide(*this, "ide%u", 1U)
		, m_kbdc(*this, "kbdc")
		, m_screen(*this, "screen")
		, m_i2cmem(*this, "i2cmem")
		, m_mouse(*this, "MOUSE")
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
	required_device<arm_iomd_device> m_iomd;
	required_device<fdc37c665gt_device> m_superio;
	required_device_array<ata_interface_device, 2> m_ide;
	required_device<pc_kbdc_device> m_kbdc;
	required_device<screen_device> m_screen;
	required_device<i2cmem_device> m_i2cmem;
	required_ioport m_mouse;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void a7000_map(address_map &map) ATTR_COLD;
	void riscpc_map(address_map &map) ATTR_COLD;

	bool m_i2cmem_clock = false;
	int iocr_od0_r();
	int iocr_od1_r();
	void iocr_od0_w(int state);
	void iocr_od1_w(int state);

	TIMER_CALLBACK_MEMBER(tc_zero_tick);

	emu_timer *m_tc_zero_timer = nullptr;
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

TIMER_CALLBACK_MEMBER(riscpc_state::tc_zero_tick)
{
	m_superio->fdc_tc_w(0);
}

void riscpc_state::a7000_map(address_map &map)
{
	map(0x00000000, 0x003fffff).mirror(0x00800000).rom().region("user1", 0);
//  map(0x01000000, 0x01ffffff).noprw(); //expansion ROM
	//
//  map(0x02000000, 0x027fffff).mirror(0x00800000).ram(); // VRAM, not installed on A7000 models
//  I/O 03000000 - 033fffff
	// NOTE: 0x1fff >> 2 = 0x7ff, the upper $400 used for LPTx ECP regs
	map(0x03010000, 0x03011fff).rw(m_superio, FUNC(fdc37c665gt_device::read), FUNC(fdc37c665gt_device::write)).umask32(0x000000ff);
//  map(0x03012000, 0x0302afff) //FDC DMA space
	map(0x03012000, 0x03029fff).rw(m_superio, FUNC(fdc37c665gt_device::fdc_dma_r), FUNC(fdc37c665gt_device::fdc_dma_w)).umask32(0x000000ff);
	map(0x0302a000, 0x0302afff).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = m_superio->fdc_dma_r(0);
			if (!machine().side_effects_disabled())
			{
				m_superio->fdc_tc_w(1);
				// TODO: accurate timing, same as below
				m_tc_zero_timer->reset();
				m_tc_zero_timer->adjust(attotime::from_usec(50));
			}
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_superio->fdc_dma_w(0, data);
			m_superio->fdc_tc_w(1);
			m_tc_zero_timer->reset();
			m_tc_zero_timer->adjust(attotime::from_usec(50));
		})
	).umask32(0x000000ff);
//  map(0x0302b000, 0x0302bfff) //Network podule
//  map(0x03040000, 0x0304ffff) //podule space 0,1,2,3
//  map(0x03070000, 0x0307ffff) //podule space 4,5,6,7
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm_iomd_device::map));
//	map(0x03240000, 0x032400ff) a7000p -bios 0 (podule mirror?)
	map(0x03310000, 0x03310003).portr(m_mouse);
//  map(0x033a0004, 0x033a0004) // topbanan, joystick?

	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
//  map(0x08000000, 0x08ffffff).mirror(0x07000000) //EASI space

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
	PORT_START("MOUSE")
	// for debugging we leave video and sound HWs as options, eventually slotify them
	PORT_CONFNAME( 0x01, 0x00, "Monitor Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x01, "TV Screen" )
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	// TODO: unmap for non-quadrature mouse variants
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
	m_tc_zero_timer = timer_alloc(FUNC(riscpc_state::tc_zero_tick), this);
}

void riscpc_state::machine_reset()
{
	m_tc_zero_timer->adjust(attotime::never);
}

// assume same formats as Acorn Archimedes
static void riscpc_floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_HFE_FORMAT);
	//fr.add(FLOPPY_HFE3_FORMAT);
	// Archimedes formats
	fr.add(FLOPPY_ACORN_ADFS_NEW_FORMAT);
	fr.add(FLOPPY_APD_FORMAT);
	fr.add(FLOPPY_JFD_FORMAT);
	// BBC Micro formats
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	// Atari ST formats
	fr.add(FLOPPY_ST_FORMAT);
	fr.add(FLOPPY_MSA_FORMAT);
}

static void riscpc_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}

static void isa_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse",  LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse",     WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse",  MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal",        SERIAL_TERMINAL);
	device.option_add("null_modem",      NULL_MODEM);
	device.option_add("sun_kbd",         SUN_KBD_ADAPTOR);
}

void riscpc_state::base_config(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);

	// auxiliary connector
//  pc_kbdc_device &aux_con(PC_KBDC(config, "aux", ps2_mice, STR_HLE_PS2_MOUSE));
//  aux_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
//  aux_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_data_w));

//  m_kbdc->aux_clk().set(aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
//  m_kbdc->aux_data().set(aux_con, FUNC(pc_kbdc_device::data_write_from_mb));
//  m_kbdc->aux_irq().set(FUNC(riscpc_state::keyboard_interrupt));

	/* video hardware */
	SCREEN(config, m_screen);

	ARM_VIDC20(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(m_iomd, FUNC(arm_iomd_device::sound_drq));

	m_iomd->set_host_cpu_tag(m_maincpu);
	m_iomd->set_vidc_tag(m_vidc);
	m_iomd->iocr_read_od<0>().set(FUNC(riscpc_state::iocr_od0_r));
	m_iomd->iocr_read_od<1>().set(FUNC(riscpc_state::iocr_od1_r));
	m_iomd->iocr_write_od<0>().set(FUNC(riscpc_state::iocr_od0_w));
	m_iomd->iocr_write_od<1>().set(FUNC(riscpc_state::iocr_od1_w));
	m_iomd->irq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_IRQ_LINE);
	m_iomd->fiq_cb().set_inputline(m_maincpu, arm7_cpu_device::ARM7_FIRQ_LINE);
	m_iomd->kclk_cb().set(m_kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_iomd->kdata_cb().set(m_kbdc, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_kbdc, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbdc->out_clock_cb().set(m_iomd, FUNC(arm_iomd_device::kclk_w));
	m_kbdc->out_data_cb().set(m_iomd, FUNC(arm_iomd_device::kdata_w));

	// https://arcwiki.org.uk/index.php/FDC37C665GT
	// sarpc_j233 also uses a 'GT, as per the identifier check it does at startup (65h in CRD)
	// some systems may use a '672 instead (TBD, which ones?)
	FDC37C665GT(config, m_superio, XTAL(24'000'000), upd765_family_device::mode_t::AT);
	m_superio->set_ide<0>(m_ide[0]);
	m_superio->set_ide<1>(m_ide[1]);
	m_superio->fintr().set(m_iomd, FUNC(arm_iomd_device::int4_w));
	m_superio->fdrq().set(m_iomd, FUNC(arm_iomd_device::int9_w));
	subdevice<upd765_family_device>("superio:fdc")->idx_wr_callback().set(m_iomd, FUNC(arm_iomd_device::int1_w));
	m_superio->pintr1().set(m_iomd, FUNC(arm_iomd_device::int2_w));
	m_superio->irq4().set(m_iomd, FUNC(arm_iomd_device::int6_w));
	// TODO: connection with COM2 irq3 (FIRQ?)
	m_superio->txd1().set("serport0", FUNC(rs232_port_device::write_txd));
	m_superio->ndtr1().set("serport0", FUNC(rs232_port_device::write_dtr));
	m_superio->nrts1().set("serport0", FUNC(rs232_port_device::write_rts));
	m_superio->txd2().set("serport1", FUNC(rs232_port_device::write_txd));
	m_superio->ndtr2().set("serport1", FUNC(rs232_port_device::write_dtr));
	m_superio->nrts2().set("serport1", FUNC(rs232_port_device::write_rts));

	INPUT_MERGER_ANY_HIGH(config, "ide_irq").output_handler().set(m_iomd, FUNC(arm_iomd_device::int7_w));

	// cfr. note on top, we need to reserve first option for an HDD connector
	// (even if user don't mount one)
	ATA_INTERFACE(config, m_ide[0]).options(ata_devices, "hdd", nullptr);
	m_ide[0]->default_data(0x0000);
	m_ide[0]->irq_handler().set("ide_irq", FUNC(input_merger_device::in_w<0>));

	ATA_INTERFACE(config, m_ide[1]).options(ata_devices, nullptr, nullptr, false);
	m_ide[1]->default_data(0x0000);
	m_ide[1]->irq_handler().set("ide_irq", FUNC(input_merger_device::in_w<1>));

	FLOPPY_CONNECTOR(config, "superio:fdc:0", riscpc_floppies, "35hd", riscpc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "superio:fdc:1", riscpc_floppies, "35hd", riscpc_floppy_formats).enable_sound(true);

	rs232_port_device &serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("superio", FUNC(fdc37c665gt_device::rxd1_w));
	serport0.dcd_handler().set("superio", FUNC(fdc37c665gt_device::ndcd1_w));
	serport0.dsr_handler().set("superio", FUNC(fdc37c665gt_device::ndsr1_w));
	serport0.ri_handler().set("superio", FUNC(fdc37c665gt_device::nri1_w));
	serport0.cts_handler().set("superio", FUNC(fdc37c665gt_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("superio", FUNC(fdc37c665gt_device::rxd2_w));
	serport1.dcd_handler().set("superio", FUNC(fdc37c665gt_device::ndcd2_w));
	serport1.dsr_handler().set("superio", FUNC(fdc37c665gt_device::ndsr2_w));
	serport1.ri_handler().set("superio", FUNC(fdc37c665gt_device::nri2_w));
	serport1.cts_handler().set("superio", FUNC(fdc37c665gt_device::ncts2_w));

	SOFTWARE_LIST(config, "flop_list").set_compatible("archimedes");
}

void riscpc_state::rpc600(machine_config &config)
{
	constexpr XTAL cpuxtal(60_MHz_XTAL/2);

	ARM610(config, m_maincpu, cpuxtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM_IOMD(config, m_iomd, cpuxtal);
	base_config(config);
}

void riscpc_state::rpc700(machine_config &config)
{
	constexpr XTAL cpuxtal(80_MHz_XTAL/2);
	ARM710A(config, m_maincpu, cpuxtal);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM_IOMD(config, m_iomd, cpuxtal);
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
	// Base xtal comes from the upgrade StrongARM kit, which may or may not be identical to the
	// regular mobo.
	// PLL bump is unverified and may be moved as part of the CPU core actually
	constexpr XTAL cpuxtal(3'686'400);

	SA110(config, m_maincpu, cpuxtal * 44);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	// TODO: bump me up
	ARM_IOMD(config, m_iomd, cpuxtal * 44);
	base_config(config);
}

void riscpc_state::sarpc_j233(machine_config &config)
{
	// TODO: 233 MHz, as above
	constexpr XTAL cpuxtal(3'686'400);

	SA110(config, m_maincpu, cpuxtal * 64);
	m_maincpu->set_addrmap(AS_PROGRAM, &riscpc_state::riscpc_map);

	ARM_IOMD(config, m_iomd, cpuxtal * 64);
	base_config(config);
}

// TODO: BIOS revisions are identical for all computers, may warrant a dummy MACHINE_IS_BIOS_ROOT romset to hold them all instead.

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
	ROM_DEFAULT_BIOS("371")

	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASEFF )
	// Version 3.70
	ROM_SYSTEM_BIOS( 0, "370", "RiscOS 3.70" )
	ROMX_LOAD("1203,191-01.bin", 0x000000, 0x200000, NO_DUMP, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("1203,192-01.bin", 0x000002, 0x200000, NO_DUMP, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	// Version 3.71
	ROM_SYSTEM_BIOS( 1, "371", "RiscOS 3.71" )
	ROMX_LOAD("1203,261-01.bin", 0x000000, 0x200000, CRC(8e3c570a) SHA1(ffccb52fa8e165d3f64545caae1c349c604386e9), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(1))
	ROMX_LOAD("1203,262-01.bin", 0x000002, 0x200000, CRC(cf4615b4) SHA1(c340f29aeda3557ebd34419fcb28559fc9b620f8), ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(1))
ROM_END

#define rom_sarpc_j233 rom_sarpc

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/


COMP( 1994, rpc600,     0,      0,      rpc600,     a7000, riscpc_state, empty_init, "Acorn Computers", "Risc PC 600",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1994, rpc700,     rpc600, 0,      rpc700,     a7000, riscpc_state, empty_init, "Acorn Computers", "Risc PC 700",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1995, a7000,      rpc600, 0,      a7000,      a7000, riscpc_state, empty_init, "Acorn Computers", "Acorn A7000",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, a7000p,     rpc600, 0,      a7000p,     a7000, riscpc_state, empty_init, "Acorn Computers", "Acorn A7000+",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, sarpc,      0,      0,      sarpc,      a7000, riscpc_state, empty_init, "Acorn Computers", "StrongARM Risc PC",      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
COMP( 1997, sarpc_j233, sarpc,  0,      sarpc_j233, a7000, riscpc_state, empty_init, "Acorn Computers", "J233 StrongARM Risc PC", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
