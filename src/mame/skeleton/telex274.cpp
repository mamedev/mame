// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Telex 274 IBM 3274-compatible coaxial controllers.

    The three main boards have the following components identified by PCB silkscreening and/or serial
    numbers:

    COMM IIS

        U1      Am27128A(?)-2DC         ODD (203981-053)
        U2      Am27128A(?)-2DC         EVEN (203982-053)
        U3      NEC D4361C-70           ODD PARITY
        U9      HM62256LP-12            ODD
        U10     HM62256LP-12            EVEN
        U11     NEC D4361C-70           EVEN PARITY
        U13     unknown 64-pin SDIP     DMA
        U45     PAL20L8ANC              ADD DECODE PAL (205264-002)
        U56     SCN68000CAN64           CPU
        U81     10.000000MHz            10 MHZ CLK
        U84     4.0000MHz               4 MHZ CLK
        U98     P8274                   COMM CHIP
        U99     Am9513APC               TIMER

    16 PORT COAX CONTROLLER

        U1      M5M4464P-12             BANK 0 D8:11
        U2      M5M4464P-12             BANK 1 D8:11
        U3      M5M4464P-12             BANK 0 D12:15
        U4      M5M4464P-12             BANK 1 D12:15
        U6      S 74F245N               BUF D8:15
        U7      S 74F245N               BUF D0:7
        U12     M5M4464P-12             BANK 0 D0:3
        U13     M5M4464P-12             BANK 1 D0:3
        U14     M5M4464P-12             BANK 0 D4:7
        U15     M5M4464P-12             BANK 1 D4:7
        U25     KM4164B-12              BANK 0 LOW
        U26     KM4164B-12              BANK 1 LOW
        U27     KM4164B-12              BANK 0 HIGH
        U28     KM4164B-12              BANK 1 HIGH
        U30     SN74LS195AN             WATCHDOG TIMER
        U38     AMPAL16R8PC             (205263-002)
        U41     PAL16R4ACN              (204143-02)
        U44     S 74LS138N              STATE DECODER
        U48     PAL20L8ANC              MEMORY DECODER (205264-002)
        U64/89  PAL16R4ACN              (204140-003)
        U65/90  PAL16R4ACN              (204141-003)
        U66     TC17G022AP-0091         DEV 0-7 COAX
        U67/102 PAL16L8ANC              (204142-001)
        U68     N82S129N                (204570-002)
        U69     SN74LS273N              MISC PORT OUT
        U83     S 74LS541N              MISC PORT IN
        U84/107 S 74LS541N              EXEC FIFO DRIVERS
        U101    TC17G022AP-0091         DEV 8-15 COAX

    EXAM BD  SS-B

        U1/16   TMS 4464-15NL           D12:D15
        U2/17   TMS 4464-15NL           D8:D11
        U3/18   TMS 4464-15NL           D4:D7
        U4/19   TMS 4464-15NL           D0:D3
        U41     AMPAL16R8PC             MEM CONTROL (205261-002)
        U62     PAL20L8ANC              MEM DECODE (205264-002)
        U85     AMPAL16R8PC             MEM CONTROL (205263-002)
        U92     PAL16R8BNC              BUS ARB (205266-002)
        U105    HYB 41256-15            D15
        U106    HYB 41256-15            D14
        U107    HYB 41256-15            D13
        U108    HYB 41256-15            D12
        U109    HYB 41256-15            D11
        U110    HYB 41256-15            D10
        U113    MK48Z02BU-25            (0678A)
        U120    HYB 41256-15            D9
        U121    HYB 41256-15            D8
        U122    HYB 41256-15            D7
        U123    HYB 41256-15            D6
        U124    HYB 41256-15            D5
        U125    HYB 41256-15            D4
        U134    HYB 41256-15            D3
        U135    HYB 41256-15            D2
        U136    HYB 41256-15            D1
        U137    HYB 41256-15            D0
        U138    HYB 41256-15            HI PAR
        U139    HYB 41256-15            LO PAR

    While each of these boards has a CPU, program ROMs are present only on the "COMM" board. Programs for
    the other two boards should be loaded off floppy disk through the WD2797A-PL FDC which resides on the
    front panel. There are also no visible oscillators on this front panel or the "EXAM BD" and only one
    on the "COAX" board (18.8696MHz, likely used to generate data clocks for the protocol gate arrays).

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"
//#include "machine/wd_fdc.h"


namespace {

class telex274_state : public driver_device
{
public:
	telex274_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_coaxcpu(*this, "coaxcpu")
		, m_examcpu(*this, "examcpu")
	{
	}

	void telex274(machine_config &config);

private:
	void main_mem(address_map &map) ATTR_COLD;
	void coax_mem(address_map &map) ATTR_COLD;
	void exam_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_coaxcpu;
	required_device<cpu_device> m_examcpu;
};


void telex274_state::main_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("program", 0);
	map(0x008000, 0x01ffff).ram();
	map(0x080000, 0x09ffff).ram();
	map(0xff811e, 0xff811f).nopr();
	map(0xff8810, 0xff8817).rw("mpsc", FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0x00ff);
	map(0xff8818, 0xff881b).rw("maintimer", FUNC(am9513a_device::read8), FUNC(am9513a_device::write8)).umask16(0x00ff);
}

void telex274_state::coax_mem(address_map &map)
{
}

void telex274_state::exam_mem(address_map &map)
{
}


static INPUT_PORTS_START(telex274)
INPUT_PORTS_END


void telex274_state::telex274(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &telex274_state::main_mem);

	am9513a_device &maintimer(AM9513A(config, "maintimer", 4_MHz_XTAL));
	maintimer.out4_cb().set("mpsc", FUNC(i8274_device::rxca_w)); // ?
	maintimer.out4_cb().append("mpsc", FUNC(i8274_device::txca_w)); // ?
	maintimer.out5_cb().set("mpsc", FUNC(i8274_device::rxtxcb_w)); // ?

	I8274(config, "mpsc", 4'000'000);

	M68000(config, m_coaxcpu, 10'000'000);
	m_coaxcpu->set_addrmap(AS_PROGRAM, &telex274_state::coax_mem);
	m_coaxcpu->set_disable();

	M68000(config, m_examcpu, 10'000'000);
	m_examcpu->set_addrmap(AS_PROGRAM, &telex274_state::exam_mem);
	m_examcpu->set_disable();

	//AM9513A(config, "examtimer", 4'000'000);

	//WD2797(config, "fdc", 1'000'000);
}


ROM_START(telex274)
	ROM_REGION16_BE(0x8000, "program", 0)
	ROM_LOAD16_BYTE("203982-053_even.u2", 0x0000, 0x4000, CRC(56914ba9) SHA1(7a2a9e16491a024b4109c8589a2277d65536e629))
	ROM_LOAD16_BYTE("203981-053_odd.u1", 0x0001, 0x4000, CRC(04a9e2cf) SHA1(58255d6275b76aecd8b81d2ea23ca69167caf232))
ROM_END

} // anonymous namespace


COMP(1986, telex274, 0, 0, telex274, telex274, telex274_state, empty_init, "Telex Computer Products", "Telex 274-61C Sixteen Station Control Unit", MACHINE_IS_SKELETON)
