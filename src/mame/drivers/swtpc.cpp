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

	virtual void machine_start() override;

	void swtpcm(machine_config &config);
	void swtpc(machine_config &config);
	void mem_map(address_map &map);
private:
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

MACHINE_CONFIG_START(swtpc_state::swtpc)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M6800, XTAL(1'843'200) / 2)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_DEVICE_ADD("brg", MC14411, XTAL(1'843'200))
	MCFG_MC14411_F7_CB(WRITELINE("io0", ss50_interface_port_device, f600_1200_w)) // 1200b
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io1", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io2", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io3", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io4", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io5", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io6", ss50_interface_port_device, f600_1200_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io7", ss50_interface_port_device, f600_1200_w))
	MCFG_MC14411_F8_CB(WRITELINE("io0", ss50_interface_port_device, f600_4800_w)) // 600b
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io1", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io2", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io3", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io4", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io5", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io6", ss50_interface_port_device, f600_4800_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io7", ss50_interface_port_device, f600_4800_w))
	MCFG_MC14411_F9_CB(WRITELINE("io0", ss50_interface_port_device, f300_w)) // 300b
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io1", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io2", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io3", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io4", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io5", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io6", ss50_interface_port_device, f300_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io7", ss50_interface_port_device, f300_w))
	MCFG_MC14411_F11_CB(WRITELINE("io0", ss50_interface_port_device, f150_9600_w)) // 150b
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io1", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io2", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io3", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io4", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io5", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io6", ss50_interface_port_device, f150_9600_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io7", ss50_interface_port_device, f150_9600_w))
	MCFG_MC14411_F13_CB(WRITELINE("io0", ss50_interface_port_device, f110_w)) // 110b
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io1", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io2", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io3", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io4", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io5", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io6", ss50_interface_port_device, f110_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("io7", ss50_interface_port_device, f110_w))

	MCFG_SS50_INTERFACE_PORT_ADD("io0", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<0>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<0>))
	MCFG_SS50_INTERFACE_PORT_ADD("io1", default_2rs_devices, "mps")
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<1>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<1>))
	MCFG_SS50_INTERFACE_PORT_ADD("io2", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<2>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<2>))
	MCFG_SS50_INTERFACE_PORT_ADD("io3", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<3>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<3>))
	MCFG_SS50_INTERFACE_PORT_ADD("io4", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<4>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<4>))
	MCFG_SS50_INTERFACE_PORT_ADD("io5", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<5>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<5>))
	MCFG_SS50_INTERFACE_PORT_ADD("io6", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<6>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<6>))
	MCFG_SS50_INTERFACE_PORT_ADD("io7", default_2rs_devices, nullptr)
	MCFG_SS50_INTERFACE_IRQ_CALLBACK(WRITELINE("mainirq", input_merger_device, in_w<7>))
	MCFG_SS50_INTERFACE_FIRQ_CALLBACK(WRITELINE("mainnmi", input_merger_device, in_w<7>))

	MCFG_INPUT_MERGER_ANY_HIGH("mainirq")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))
	MCFG_INPUT_MERGER_ANY_HIGH("mainnmi")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", INPUT_LINE_NMI))

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,28K,32K")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(swtpc_state::swtpcm)
	swtpc(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(XTAL(1'797'100) / 2)

	MCFG_DEVICE_MODIFY("brg")
	MCFG_DEVICE_CLOCK(XTAL(1'797'100))

	MCFG_DEVICE_MODIFY("io1")
	MCFG_SLOT_DEFAULT_OPTION("mpc")
MACHINE_CONFIG_END

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
