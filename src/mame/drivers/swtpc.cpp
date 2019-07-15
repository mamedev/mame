// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SWTPC 6800 Computer System

        10/12/2009 Skeleton driver.

        http://www.swtpc.com/mholley/swtpc_6800.htm

    MIKBUG is made for a PIA (parallel) interface.
    SWTBUG is made for a ACIA (serial) interface at the same address.
    MIKBUG will actually read the bits as they arrive and assemble a byte.
    Its delay loops are based on an underclocked XTAL.

    Note: All commands must be in uppercase. See the SWTBUG manual.

    ToDo:
        - Add more SS-50 interface slot options
        - Emulate MP-A2 revision of CPU board, with four 2716 ROM sockets
          and allowance for extra RAM boards at A000-BFFF and C000-DFFF


Commands:
B Breakpoint
C Clear screen
D Disk boot
E End of tape
F Find a byte
G Goto
J Jump
L Ascii Load
M Memory change (enter to quit, - to display next byte)
O Optional Port
P Ascii Punch
R Register dump
Z Goto Prom (0xC000)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/input_merger.h"
#include "machine/mc14411.h"
#include "machine/ram.h"
#include "bus/ss50/interface.h"
#include "bus/ss50/mps.h"

class swtpc_state : public driver_device
{
public:
	swtpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_brg(*this, "brg")
	{ }

	void swtpcm(machine_config &config);
	void swtpc(machine_config &config);

private:
	virtual void machine_start() override;

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<mc14411_device> m_brg;
};

void swtpc_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x8003).mirror(0x1fc0).rw("io0", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x8004, 0x8007).mirror(0x1fc0).rw("io1", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x8008, 0x800b).mirror(0x1fc0).rw("io2", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x800c, 0x800f).mirror(0x1fc0).rw("io3", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x8010, 0x8013).mirror(0x1fc0).rw("io4", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x8014, 0x8017).mirror(0x1fc0).rw("io5", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x8018, 0x801b).mirror(0x1fc0).rw("io6", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0x801c, 0x801f).mirror(0x1fc0).rw("io7", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xa000, 0xa07f).ram(); // MCM6810
	map(0xe000, 0xe3ff).mirror(0x1c00).rom().region("mcm6830", 0);
}

/* Input ports */
static INPUT_PORTS_START( swtpc )
INPUT_PORTS_END


void swtpc_state::machine_start()
{
	m_brg->rsa_w(0);
	m_brg->rsb_w(1);

	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
}

void swtpc_state::swtpc(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(1'843'200) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &swtpc_state::mem_map);

	MC14411(config, m_brg, XTAL(1'843'200));
	m_brg->out_f<7>().set("io0", FUNC(ss50_interface_port_device::f600_1200_w)); // 1200b
	m_brg->out_f<7>().append("io1", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io2", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io3", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io4", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io5", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io6", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io7", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<8>().set("io0", FUNC(ss50_interface_port_device::f600_4800_w)); // 600b
	m_brg->out_f<8>().append("io1", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io2", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io3", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io4", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io5", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io6", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<8>().append("io7", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<9>().set("io0", FUNC(ss50_interface_port_device::f300_w)); // 300b
	m_brg->out_f<9>().append("io1", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io2", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io3", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io4", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io5", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io6", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io7", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<11>().set("io0", FUNC(ss50_interface_port_device::f150_9600_w)); // 150b
	m_brg->out_f<11>().append("io1", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io2", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io3", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io4", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io5", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io6", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<11>().append("io7", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<13>().set("io0", FUNC(ss50_interface_port_device::f110_w)); // 110b
	m_brg->out_f<13>().append("io1", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io2", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io3", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io4", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io5", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io6", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io7", FUNC(ss50_interface_port_device::f110_w));

	ss50_interface_port_device &io0(SS50_INTERFACE(config, "io0", ss50_default_2rs_devices, nullptr));
	io0.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	io0.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<0>));
	ss50_interface_port_device &io1(SS50_INTERFACE(config, "io1", ss50_default_2rs_devices, "mps"));
	io1.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	io1.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<1>));
	ss50_interface_port_device &io2(SS50_INTERFACE(config, "io2", ss50_default_2rs_devices, nullptr));
	io2.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<2>));
	io2.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<2>));
	ss50_interface_port_device &io3(SS50_INTERFACE(config, "io3", ss50_default_2rs_devices, nullptr));
	io3.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<3>));
	io3.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<3>));
	ss50_interface_port_device &io4(SS50_INTERFACE(config, "io4", ss50_default_2rs_devices, nullptr));
	io4.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<4>));
	io4.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<4>));
	ss50_interface_port_device &io5(SS50_INTERFACE(config, "io5", ss50_default_2rs_devices, nullptr));
	io5.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<5>));
	io5.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<5>));
	ss50_interface_port_device &io6(SS50_INTERFACE(config, "io6", ss50_default_2rs_devices, nullptr));
	io6.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<6>));
	io6.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<6>));
	ss50_interface_port_device &io7(SS50_INTERFACE(config, "io7", ss50_default_2rs_devices, nullptr));
	io7.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<7>));
	io7.firq_cb().set("mainnmi", FUNC(input_merger_device::in_w<7>));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, "mainnmi").output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	RAM(config, RAM_TAG).set_default_size("2K").set_extra_options("4K,8K,12K,16K,20K,24K,28K,32K");
}

void swtpc_state::swtpcm(machine_config &config)
{
	swtpc(config);
	m_maincpu->set_clock(XTAL(1'797'100) / 2);

	m_brg->set_clock(XTAL(1'797'100));

	subdevice<ss50_interface_port_device>("io1")->set_default_option("mpc");
}

/* ROM definition */
ROM_START( swtpc )
	ROM_REGION( 0x0400, "mcm6830", 0 )
	ROM_LOAD("swtbug.bin", 0x0000, 0x0400, CRC(f9130ef4) SHA1(089b2d2a56ce9526c3e78ce5d49ce368b9eabc0c))
ROM_END

ROM_START( swtpcm )
	ROM_REGION( 0x0400, "mcm6830", 0 )
	ROM_LOAD("mikbug.bin", 0x0000, 0x0400, CRC(e7f4d9d0) SHA1(5ad585218f9c9c70f38b3c74e3ed5dfe0357621c))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                                     FULLNAME      FLAGS
COMP( 1977, swtpc,  0,      0,      swtpc,   swtpc, swtpc_state, empty_init, "Southwest Technical Products Corporation", "SWTPC 6800 Computer System (with SWTBUG)", MACHINE_NO_SOUND_HW )
COMP( 1975, swtpcm, swtpc,  0,      swtpcm,  swtpc, swtpc_state, empty_init, "Southwest Technical Products Corporation", "SWTPC 6800 Computer System (with MIKBUG)", MACHINE_NO_SOUND_HW )
