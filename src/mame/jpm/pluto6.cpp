// license:BSD-3-Clause
// copyright-holders: NaokiS

/*
Pluto 6 hardware from Heber Ltd.

Hardware specs:
    Motorola MCF5206e ColdFire at 40Mhz.
    2MB EDO RAM + 256KB Battery backed SRAM.
    512KB Flash BIOS/IPL ROM.
    IDE/ATAPI interface for CompactFlash, HDDs and/or CD-ROM.
    Configurable with up to two Fujistu MB86290A GPUs with 8MB per module.
    Stereo DAC with software based mixer.
    PIC18 System management, RTC and security device.

IO:
    * 32 inputs.
    * 256 lamps with 1-8 dimming levels.
    * 256 LEDs or 32 7-Segment displays with 1-8 dimming levels.
    * 64 open-drain outputs.
    * 6 TTL auxillary outputs.
    * Up to 6 UART ports - 4 used as RS232 ports (One used as BACTA Dataport), 2 used for dual ccTalk channels.
        2x UART on ColdFire, 2x UART in FPGA, optional MC68681 DUART support.
    * RS485/TTL UART port.


Pluto 6 (01-16391-8)
┌─────────────────────────────────────────────────────────┬───────────────┬───────────────────────────────────┐
│┌─┐                         ┌───────────┐                │               │┌────────┐ P9 ________________     │
││⁞│ P26       ┌─┐  ┌─┐      |___________| P4         ┌──┐│ CompactFlash  ││________││......... .........│    │
│└─┘           │││  │││  ┌────┐    U3     U26         │  ││      P6       │    P7      NMOS  NMOS  NMOS ┌──┐  │
│┌──┐          │││  │││  │ U1 │ ┌─────┐ ┌─────┐       │I │├─────┬───────┬─┘            NMOS  NMOS  NMOS │  │P │
││  │P25       │-│  │-│  └────┘ └─────┘ └─────┘       │D ││_____│ PIC18 │              NMOS  NMOS  NMOS │  │1 │
││  │          │││  │││             ┌──────────┐  U2  │E │   P8 │  U 5  │              NMOS  NMOS  NMOS │  │0 │
│└──┘          │││  │││     X3      │          │ ┌─┐  │  │      └───────┘ X1           NMOS  NMOS  NMOS │  │  │
│              │││  │││ ┌──┐  ┌─┐   │ COLDFIRE │ |_|  │  │            X2               NMOS             └──┘  │
│┌──┐          │││  │││ │  │  └─┘   │          │ ┌─┐  └──┘      ┌────────┐                               ┌─┐  │
││  │ P24      │-│  │-│ │  │   U6   └──────────┘ |_|   P5       │||||||||│SW2                            │⁞│P │
││  │          │││  │││ │  │                     U27            ├────────┤              PNP PNP PNP PNP  │⁞│1 │
│└──┘ U41      │││  │││ │  │  ┌──────────┐   ┌───┐              │||||||||│SW1           PNP PNP PNP PNP  │⁞│1 │
│    ┌─┐       │││  │││ │  │  │          │   │   │ U55  ┌─┐     └────────┘              PNP PNP PNP PNP  │⁞│  │
│┌─┐ └─┘       │││  │││ │  │  │  XCS20XL │   └───┘      │⁞│ P27                         PNP PNP PNP PNP  └─┘  │
││⁞│           │││  │││ │  │  │          │              └─┘ _                           TPIC255 TPIC255       │
││⁞│ 74HCT244  │││  │││ │  │  └──────────┘  x4             |O| ┌──┐ U7                  TPIC255 TPIC255 ┌──┐  │
││⁞│           │││  │││ │  │              ┌──────┐         SW3 └──┘           ┌─────┐   TPIC255 TPIC255 │  │  │
│└─┘P23        │││  │││ │  │  U28         │ U56  │                  TPIC255   │     │   TPIC255 TPIC255 │  │P │
│┌───┐         │││  │││ │  │  ┌─┐         │ DUART│                          B1│     │                   │  │1 │
││/-││         └─┘  └─┘ └──┘  └─┘         └──────┘              HCT245        └─────┘       P14         │  │2 │
│││ ││         EXP0 EXP1 P3             U31                     HCT245 HCT245       ┌─────────────────┐ │  │  │
│││ ││  SN75188N SN75188N           ┌──────────────┐                         P16  __│_________________│ │  │  │
│││ ││    SN75188N SN75188N     P19 │______________│  P18              P17   P15 |... ....|     P13     └──┘  │
││\_││P22 ┌───────┐┌──────────┐ ______ ┌────────────────────┐   ┌──────────────┐  ________ ┌──────────────┐   │
│└───┘    │__P21__││___P20____│|... ..|│o o o o o o o  o o o│   │______________│ |... ....|│______________│   │
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────┘

COLDFIRE: MCF5602e CPU @ 40MHz
U1: 29C040 Flash with CPU boot code
U2: 74HCT245
U3: 2MB EDO DRAM
U5: PIC18 Security/System management controller
U6: ??
U7: 74HCT245?
U??: Xilinx XCS20XL Spartan FPGA.
U26: 512KB SRAM
U27: ??
U28: ??
U41: RS485 Transceiver
U55: Heber custom ASIC. Probably a CPLD of sorts
U56: SCC68692 DUART
B1: Varta NiCd 3v SRAM battery, which is likely leaking if not already replaced.

EXP 0+1: Fujitsu Cremson MB86290A/Expansion ports.
    * Some Pluto 6 motherboards are only able to use one GPU as EXP1 has no matching VGA connector.
P3: DIN41612 port which can be used for ROM or RAM. Some JPM games use the ROM board for game code.
P4: ColdFire BDM Debug port - Only populated on development boards.
P5: 40-Pin IDE/ATAPI interface for HDD/CD-ROM drives.
CF: CompactFlash card slot wired as IDE master.
P7: Percentage/Stake input port for respective dongles.
P8: PWDWN/Security port. The manual lists these as being for security monitors, go to the PIC18.
P9: Lamp sinks - Part of the matrixed 256 lamp system.
P10: LEDs - Unsurprisingly, controls LEDs. Can support 256 LEDs OR 32 7-Segment displays.
P11: Lamp sourcees - Part of the matrixed 256 lamp system.
    Manual says this is 48v, but seems to be either 12v or 48v depending on needs.
P12: Reels - Connects to 24 of the open-drain outputs, some of the lamp signals and 6 inputs.
P13 + P14: Expose the remaining TTL inputs and open-drain outputs.
P15: Aux Outputs - 6 TTL outputs. Connects to the VFDs on the development board.
P16: Multiplex Expansion - Can be used to expand the system to 512 lights and 512 LEDs. Insane.
P17: IO Bus Expansion
P18: Main power input for +5v, +/-12v and optionally +48v. Has a power supply ready signal input.
P19: Speaker output connector from audio amp. (4-8ohm 15W)
P20: ccTalk/HI2 dual channel connector.
P21: RS232 A/C/D connector.
P22: BACTA Dataport connector, a.k.a RS232 B.
P23: RS485/TTL connector.
P24: VGA Connector for EXP0.
P25: Either a box header connecting to EXP1 (but not video), or VGA for EXP1.
P26: Labelled as Ethernet on development board, but manual says it was for future expansion.
P27: I2C connector
SW1+2: Software configurable DIP switches.
SW3: Software configurable switch.

* Coldfire Chip select map *
CS0: Boot ROM
CS1: SRAM
CS2: FPGA
CS3: GPU/s
CS4-7: Unused

* Boot process *
On power up, the CPU is held in reset by the glue logic. Meanwhile the PIC18 boots and uses the +5v LED as a heartbeat.
The PIC18 uploads the FPGA bitstream before releasing the reset line on the CPU. The CPU then boots and runs code
from the flash chip. This usually involves running through a security check and then loading either an S19 program from
the CompactFlash, or from the add-on ROM board.

* Security *
TODO: Expand as more is learnt.

The security is handled by at least three parts, the FPGA, the CPLD and the PIC18.
he PIC18 loads the FPGA with a bitstream that is customised by Heber for each manufacturer.
What this entails isn't yet known, but it is known the development board has most/all of this removed.
This means that the PIC18 is essentially a dongle for each game and needs to be dumped. Whilst it contains the bitstream,
its also probably stored in an encrypted for and decrypted within the Heber CPLD/ASIC, as the FPGA bitstream lines connect
to this part, not the PIC18.

GPU/Dual GPU: Commented out due to errors and there is not a currently suitable GPU core for the Pluto 6.

* Game Notes *
pl6_cm:     Hybrid Pluto 6 + PC setup. They are connected over RS232. Pluto 6 is probably handling IO and security here.
            Uses a Intel Celeron (2.1Ghz) PC with a Protech PROX3770 motherboard. Game runs on Windows XP.
pl6_dev:    Heber's official development kit for the Pluto 6. Only ships with a development BIOS and an SDK.
            This "game" requires a bootable (FAT32 with Active partition) CHD, else it does nothing visually.
            The CHD in this driver is an image of the CF card that was supplied by Heber in the SDK for demo purposes.
            Security and FPGA encryption is disabled on this model.

EINT3: DUART INT?

*/

/*
    Game compatibility notes:
        * pl6_demo: Write 0x4e75 at 0x6001063a after CF has loaded to get demo working. Hangs waiting for ColdFire UART to transmit otherwise.
        * pl6vdemo: Doesn't work at all, needs Cremson emulating.
        * pl6ddemo: Doesn't work at all, needs Cremson emulating.
        * pl6_cm:   Windows harddrive is not dumped. PC is connected to the Pluto 6 over RS232.
        * tijkpots: Game is missing CF card. For whatever reason, the blank image doesn't cause it to fail to load, and so reads invalid data
*/
/*
    Todo:
        * Make the GPU slot device... far better than it currently is.
        * Configurable amount of VFDs. Currently set at one, but two is not uncommon and shouldn't always be loaded if not needed
        * Reels
*/

#include "emu.h"

#include "cpu/m68000/mcf5206e.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "bus/rs232/rs232.h"
#include "machine/mc68681.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "machine/pl6_exp.h"
#include "machine/pl6_fpga.h"
#include "machine/pl6_pic.h"
#include "video/serialvfd.h"


#include "speaker.h"


#define LOG_VFD  (1U << 1)
#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGVFDS(...)        LOGMASKED(LOG_VFD,     __VA_ARGS__)

#include "pl6dev.lh"
#include "pl6vdev.lh"

namespace {

class pluto6_state : public driver_device
{
public:
	pluto6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_fpga(*this, "fpga"),
			m_pic(*this, "pic18"),
			m_ata(*this, "ata"),
			m_exp0(*this, "exp0"),
			m_exp1(*this, "exp1"),
			m_duart(*this, "duart"),
			m_i2cmem(*this, "i2cmem"),
			m_inputs(*this, "IN0"),
			m_open_drain(*this, "open_drain%u", 0U),
			m_lamp(*this, "lamp%u", 0U),
			m_led(*this, "led%u", 0U),
			m_aux(*this, "aux%u", 0U),
			m_debug(*this, "DEBUG"),
			m_vfd(*this, "vfd")
	{ }

	void pluto6_dev(machine_config &config);
	[[maybe_unused]] void pluto6_betcom(machine_config &config);
	[[maybe_unused]] void pluto6_jpmrom(machine_config &config);

	// FPGA
	uint32_t input_callback(offs_t offset);
	void lamp_callback(offs_t offset, uint8_t data);
	void led_callback(offs_t offset, uint8_t data);
	void output_callback(offs_t offset, uint8_t data);
	void auxout_callback(offs_t offset, uint8_t data);

	// UART Stuff
	void cfuart_tx1_w(uint8_t state) { m_fpga->cfuart_tx1_w(state); }
	void cfuart_tx2_w(uint8_t state) { m_fpga->cfuart_tx2_w(state); }
	void cfuart_rx_a_w(uint8_t state) { m_maincpu->rx1_w(state); }
	void cfuart_rx_b_w(uint8_t state) { m_maincpu->rx2_w(state); }

	// I2C Stuff
	void pluto_sda(uint8_t state);
	void pluto_scl(uint8_t state);

	uint8_t duart_read(offs_t offset) { return m_duart->read(offset); }
	void duart_write(offs_t offset, uint8_t data){ m_duart->write(offset, data); }
	void duart_rx_a_w(uint8_t state) { m_duart->rx_a_w(state); }
	void duart_rx_b_w(uint8_t state) { m_duart->rx_b_w(state); }
	void duart_tx_a_w(uint8_t state) { m_fpga->duart_tx_a_w(state); }
	void duart_tx_b_w(uint8_t state) { m_fpga->duart_tx_b_w(state); }

	static constexpr feature_type unemulated_features() { return feature::PROTECTION; }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void pluto6(machine_config &config);    // Private to prevent use
	void install_duart(machine_config &config);
	void install_eeprom(machine_config &config);
	//void install_calypso(machine_config &config, u8 slot);


	void duart_irq_handler(int state);
	void gpu1_irq_handler(int state);
	void gpu2_irq_handler(int state);

	void pluto6_map(address_map &map);
	void pluto6v_map(address_map &map);
	void pluto6dv_map(address_map &map);

	void jpm_rom_map(address_map &map);

	// IO
	required_device<mcf5206e_device> m_maincpu;
	required_device<pl6fpga_device> m_fpga;
	required_device<pl6pic_device> m_pic;
	required_device<ata_interface_device> m_ata;

	required_device<pluto6_expansion_slot_device> m_exp0;
	required_device<pluto6_expansion_slot_device> m_exp1;

	bool duart_installed = false;
	optional_device<mc68681_device> m_duart;    // Should be a SCC68692 instead
	uint8_t m_duart_irq_state = 0;

	bool eeprom_installed = false;
	optional_device<i2c_24c04_device> m_i2cmem;

	required_ioport m_inputs;
	output_finder<64> m_open_drain;
	output_finder<256> m_lamp;
	output_finder<256> m_led;
	output_finder<8> m_aux;
	optional_ioport m_debug;

	bool vfd_installed = false;
	optional_device<serial_vfd_device> m_vfd;

	// LED hack
	uint8_t led_array[32] = {0};
	bool led_use_7seg = false;
};


void pluto6_state::pluto6_map(address_map &map)
{
	// ColdFire (Coldfire chip selects in brackets)
	map(0x00000000, 0x0007ffff).rom();                      // Boot ROM (CS0)
	map(0x10000000, 0x1003ffff).ram().share("nvram");       // Battery Backed SRAM (CS1 below 0x12xxxxxx)
	map(0x12000000, 0x1200000f).rw(m_ata, FUNC(ata_interface_device::cs0_swap_r), FUNC(ata_interface_device::cs0_swap_w)); // IDE (CS1)
	map(0x12000010, 0x1200001f).rw(m_ata, FUNC(ata_interface_device::cs1_swap_r), FUNC(ata_interface_device::cs1_swap_w)); // IDE (CS1)

	// FPGA
	map(0x20000000, 0x20003fff).rw(m_fpga, FUNC(pl6fpga_device::dev_r), FUNC(pl6fpga_device::dev_w));   // Entire IO system (CS2)

	// GPU/EXP0/EXP1 Stub - Most bootloaders check this region to see if GPU is installed. (CS3)
	map(0x30000000, 0x31ffffff).noprw();
	map(0x34000000, 0x35ffffff).noprw();

	map(0x50000000, 0x50001fff).ram(); // ColdFire Internal 8Kb SRAM
	map(0x60000000, 0x601fffff).ram(); // 2MB DRAM

	// ColdFire Peripheral Registers
	//map(0xf0000000, 0xf00002ff).rw(m_maincpu, FUNC(mcf5206e_device::dev_r), FUNC(mcf5206e_device::dev_w));
}

void pluto6_state::jpm_rom_map(address_map &map){
	pluto6_map(map);
	map(0x00000000, 0x0008ffff).unmaprw();
	map(0x00000000, 0x001fffff).rom();
}
/*
void pluto6_state::pluto6v_map(address_map &map)
{
    pluto6_map(map);
    map(0x31fcfffc, 0x31fcffff).unmaprw();
    map(0x30000000, 0x307fffff).rw(m_vram1, FUNC(ram_device::read), FUNC(ram_device::write));
    map(0x31fc0000, 0x31ffffff).m(m_gpu1, FUNC(mb86292_device::vregs_map));
}

void pluto6_state::pluto6dv_map(address_map &map)
{
    pluto6_map(map);
    map(0x31fcfffc, 0x31fcffff).unmaprw();
    map(0x30000000, 0x307fffff).rw(m_vram1, FUNC(ram_device::read), FUNC(ram_device::write));
    map(0x31fc0000, 0x31ffffff).m(m_gpu1, FUNC(mb86292_device::vregs_map));

    map(0x35fcfffc, 0x35fcffff).unmaprw();
    map(0x34000000, 0x347fffff).rw(m_vram2, FUNC(ram_device::read), FUNC(ram_device::write));
    map(0x35fc0000, 0x35ffffff).m(m_gpu2, FUNC(mb86292_device::vregs_map));
}

u32 pluto6_state::screen1_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    m_gpu1->screen_update(screen, bitmap, cliprect);
    return 0;
}

u32 pluto6_state::screen2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    m_gpu2->screen_update(screen, bitmap, cliprect);
    return 0;
}
*/

static INPUT_PORTS_START( pluto6 )
	// It's a wacky layout but it's accurate according to testing.
	// Most of this is actually connected to the FPGA, but its in here for ease of reading
	PORT_START( "IN0" )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("IP7")
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("IP6")
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("IP5")
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("IP4")
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("IP3")
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("IP2")
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("IP1")
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("IP0")

	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME("IP15")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(1) PORT_NAME("IP16")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(1) PORT_NAME("IP17")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_PLAYER(1) PORT_NAME("IP18")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_PLAYER(1) PORT_NAME("IP19")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_PLAYER(1) PORT_NAME("IP20")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_PLAYER(1) PORT_NAME("IP21")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_PLAYER(1) PORT_NAME("IP22")

	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("IP23")
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("IP22")
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("IP21")
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("IP20")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("IP19")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("IP18")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("IP17")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("IP16")

	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_PLAYER(2) PORT_NAME("IP31")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_PLAYER(2) PORT_NAME("IP30")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_PLAYER(2) PORT_NAME("IP29")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_PLAYER(2) PORT_NAME("IP28")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_PLAYER(2) PORT_NAME("IP27")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(2) PORT_NAME("IP26")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_NAME("IP25")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_NAME("IP24")

	PORT_START("DEBUG")
	PORT_BIT( 0xffffffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END


void pluto6_state::machine_start()
{
	m_open_drain.resolve();
	m_lamp.resolve();
	m_led.resolve();
	m_aux.resolve();
}

void pluto6_state::machine_reset(){

}

void pluto6_state::duart_irq_handler(int state)
{
	m_duart_irq_state = state;
	//update_interrupts();
}

/*
void pluto6_state::gpu1_irq_handler(int state)
{
    //update_interrupts();
}

void pluto6_state::gpu2_irq_handler(int state)
{
    //update_interrupts();
}
*/

uint32_t pluto6_state::input_callback(offs_t offset){
	return m_inputs->read();
}

void pluto6_state::lamp_callback(offs_t offset, uint8_t data){
	m_lamp[offset] = data;
}

void pluto6_state::led_callback(offs_t offset, uint8_t data){
	if(led_use_7seg){
		// Hack to deal with the fact MAME doesn't have per segment LED displays is layouts
		//  let alone dimming support on them.
		uint8_t digit_num = offset / 8;
		uint8_t bit_num = offset % 8;
		uint8_t disp_num = digit_num / 2 + (digit_num % 2 ? 16 : 0);
		uint8_t mask = ~(1 << bit_num);

		led_array[disp_num] = (led_array[disp_num] & mask) | ((data > 0) << bit_num);
		m_led[disp_num] = led_array[disp_num];
	} else {
		m_led[offset] = data;
	}
}

void pluto6_state::output_callback(offs_t offset, uint8_t data){
	m_open_drain[offset] = data;
}

void pluto6_state::auxout_callback(offs_t offset, uint8_t data){
	LOGVFDS("ao_cb( 0x%02x, 0x%x )\n", offset, data );
	m_aux[offset] = data;
	switch (offset) {
		case 0: m_vfd->write_clock(!(data > 0)); break;
		case 1: m_vfd->write_data(!(data > 0)); break;
		case 2: m_vfd->write_reset(!(data > 0)); break;
		default: break;
	}
}

// I2C BUS
void pluto6_state::pluto_sda(uint8_t state){
	m_maincpu->sda_write(state);
	m_pic->sda_write(state);
	if(eeprom_installed) m_i2cmem->write_sda(state);    // i2cmem seems to not have a write_line call back?
}

void pluto6_state::pluto_scl(uint8_t state){
	m_pic->scl_write(state);
	if(eeprom_installed) m_i2cmem->write_scl(state);
}

void pluto6_state::install_duart(machine_config &config){
	MC68681(config, m_duart, XTAL(3'686'400));  // Actually 3.69MHz on the dev board
	m_duart->irq_cb().set(FUNC(pluto6_state::duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(pluto6_state::duart_tx_a_w));
	m_duart->b_tx_cb().set(FUNC(pluto6_state::duart_tx_b_w));
	m_fpga->duart_rx_a_callback().set(FUNC(pluto6_state::duart_rx_a_w));
	m_fpga->duart_rx_b_callback().set(FUNC(pluto6_state::duart_rx_b_w));
	m_fpga->duart_r_callback().set(FUNC(pluto6_state::duart_read));
	m_fpga->duart_w_callback().set(FUNC(pluto6_state::duart_write));
	duart_installed = true;
}

void pluto6_state::install_eeprom(machine_config &config){
	I2C_24C04(config, m_i2cmem).set_address(0xA0);
	eeprom_installed = true;
}

/*
void pluto6_state::install_calypso(machine_config &config, u8 slot){
    if(slot > 1) return;
    if(slot == 0) m_exp0->option_set("gpu", HEBER_CALYPSO_GPU);
    else m_exp1->option_set("gpu", HEBER_CALYPSO_GPU);
}*/

// Machine defs
void pluto6_state::pluto6(machine_config &config){
	MCF5206E(config, m_maincpu, XTAL(40'000'000));
	m_maincpu->tx1_w_cb().set(FUNC(pluto6_state::cfuart_tx1_w));
	m_maincpu->tx2_w_cb().set(FUNC(pluto6_state::cfuart_tx2_w));
	m_maincpu->sda_w_cb().set(FUNC(pluto6_state::pluto_sda));
	m_maincpu->scl_w_cb().set(FUNC(pluto6_state::pluto_scl));

	PLUTO6_EXPANSION_SLOT(config, m_exp0, 0);
	m_exp0->set_default_option(nullptr);
	PLUTO6_EXPANSION_SLOT(config, m_exp1, 0);
	m_exp1->set_default_option(nullptr);

	SPEAKER(config, "speaker", 2).front();

	HEBER_PLUTO6_FPGA(config, m_fpga);
	m_maincpu->set_addrmap(AS_PROGRAM, &pluto6_state::pluto6_map);
	m_fpga->input_callback().set(FUNC(pluto6_state::input_callback));
	m_fpga->lamp_callback().set(FUNC(pluto6_state::lamp_callback));
	m_fpga->led_callback().set(FUNC(pluto6_state::led_callback));
	m_fpga->output_callback().set(FUNC(pluto6_state::output_callback));
	m_fpga->auxout_callback().set(FUNC(pluto6_state::auxout_callback));
	m_fpga->cfuart_rx_a_callback().set(FUNC(pluto6_state::cfuart_rx_a_w));
	m_fpga->cfuart_rx_b_callback().set(FUNC(pluto6_state::cfuart_rx_b_w));

	HEBER_PLUTO6_PIC(config, m_pic, XTAL(5'000'00));
	m_pic->sda_rx_cb().set(FUNC(pluto6_state::pluto_sda));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	config.set_default_layout(layout_pl6dev);
}

/* Heber Pluto 6 Development Kit */
void pluto6_state::pluto6_dev(machine_config &config){
	pluto6(config);
	m_fpga->set_fpga_type(pl6fpga_device::developer_fpga);
	m_pic->set_address(0xC0);

	m_exp0->option_add("gpu", HEBER_CALYPSO_GPU); // Is optional
	m_exp0->set_default_option(nullptr);

	install_duart(config);
	install_eeprom(config);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);

	SERIAL_VFD(config, m_vfd);
	this->led_use_7seg = true;
}

/* Betcom Pluto 6 */
void pluto6_state::pluto6_betcom(machine_config &config){
	pluto6(config);
	m_fpga->set_fpga_type(pl6fpga_device::betcom_fpga);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);

	SERIAL_VFD(config, m_vfd);
	this->led_use_7seg = true;
}

/* JPM Pluto 6 */
void pluto6_state::pluto6_jpmrom(machine_config &config){
	pluto6(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pluto6_state::jpm_rom_map);
	m_fpga->set_fpga_type(pl6fpga_device::jpm_fpga);
	m_pic->set_address(0xC0);

	ATA_INTERFACE(config, m_ata).options(ata_devices, nullptr, nullptr, true);

	install_eeprom(config);

	SERIAL_VFD(config, m_vfd);
	this->led_use_7seg = true;
}

// ROMS
ROM_START( pl6_kfp )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootrom", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "pl6_kfp", 0, SHA1(a2506e2ff67f2632bcde3281baeaad1d094309b2) )
ROM_END

ROM_START( pl6_lgk )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootrom", 0x00000, 0x80000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "pl6_lgk", 0, SHA1(7d8631a4e336e93c9bfddf9166a22b756d8382fc) )
ROM_END

#if 0
ROM_START( pl6_cm )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "60a8_b1.u1", 0x00000, 0x40000, CRC(1e9bbb77) SHA1(a93b89856f41cfe477a2791ca22c25e8249ab496) )
	ROM_IGNORE(                      0xc0000 )
	ROM_RELOAD(             0x40000, 0x40000 )

	ROM_REGION( 0x100000, "program1", ROMREGION_ERASE00 )
	ROM_LOAD( "acc6_b2.u2", 0x000000, 0x100000, CRC(97494822) SHA1(01b7f2eb9af34e0a2d626ec303979c6b53cb83b9) )
ROM_END

ROM_START( pl6_atw )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "27c801-5e1.u1", 0x000001, 0x100000, CRC(4634a98a) SHA1(81cb44b6f17b49d5652bd5deacbf70aec4ac574b) )
	ROM_LOAD16_BYTE( "27c801-5e2.u2", 0x000000, 0x100000, CRC(0d2601cd) SHA1(eed0d5571d56fa6cb84f579219cd68393a74fbb1) )
	ROM_LOAD16_BYTE( "27c801-snd1.u3", 0x100001, 0x100000, CRC(3937ec11) SHA1(68f53d4fe29791e6b2ae592c9a3118075946172f) )
	ROM_LOAD16_BYTE( "27c801-snd2.u4", 0x100000, 0x100000, CRC(804eabd3) SHA1(1903c3ce1d4b3b2f9e4317da9bf391908afd1724) )
ROM_END

ROM_START( pl6_demo )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "17443iss.u1", 0x00000, 0x80000, CRC(40b4211a) SHA1(4cce50fc67a15b2b1f5d12a9f11fc12c51908773))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "pl6demo_cf", 0, SHA1(a97e0af3b35d02b014d98b48977c4a2584271f20) )
ROM_END
#endif

/*
ROM_START( pl6vdemo )
    ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
    ROM_LOAD( "17443iss.u1", 0x00000, 0x80000, CRC(40b4211a) SHA1(4cce50fc67a15b2b1f5d12a9f11fc12c51908773))

    DISK_REGION( "ata:0:hdd" )
    DISK_IMAGE( "21-17460", 0, SHA1(a3604dea855312f5019a4be9e1e25770203c8039) )
ROM_END

ROM_START( pl6ddemo )
    ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
    ROM_LOAD( "17443iss.u1", 0x00000, 0x80000, CRC(40b4211a) SHA1(4cce50fc67a15b2b1f5d12a9f11fc12c51908773))

    DISK_REGION( "ata:0:hdd" )
    DISK_IMAGE( "21-17460", 0, SHA1(a3604dea855312f5019a4be9e1e25770203c8039) )
ROM_END
*/

#if 0
ROM_START( tijkpots )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "tisland.u1", 0x00000, 0x80000, CRC(E4B67B94) SHA1(d88019190989e92e1538a9abd6e1ef3c3bb89146))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "pl6_tisl", 0, NO_DUMP )
ROM_END
#endif

} // anonymous namespace

GAME( 2014, pl6_kfp, 0, pluto6_dev, pluto6, pluto6_state, empty_init, ROT0, "G Squared", "Kung Fu Pounda", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
//GAME( 2014, pl6_atw, 0, pluto6_jpmrom, pluto6, pluto6_state, empty_init, ROT0, "JPM", "Around The World", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2014, pl6_lgk, 0, pluto6_dev, pluto6, pluto6_state, empty_init, ROT0, "Betcom", "Let's Get Kraken", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
//GAME( 2014, tijkpots, 0, pluto6_betcom, pluto6, pluto6_state, empty_init, ROT0, "Betcom", "Treasure Island Jackpots", MACHINE_MECHANICAL )
//GAME( 2004, pl6_fant, 0, pluto6_dev, pluto6, pluto6_state, empty_init, ROT0, "JPM", "Fabtaztec", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
//GAME( 2004, pl6_cm, 0, pluto6_dev, pluto6, pluto6_state, empty_init, ROT0, "JPM", "Crystal Maze" , MACHINE_NOT_WORKING )
//GAME( 2000, pl6_demo, 0, pluto6_dev, pluto6, pluto6_state, empty_init, ROT0, "Heber", "Pluto 6 Devkit/Evaluation Board", MACHINE_NOT_WORKING )
//GAME( 2000, pl6vdemo, 0, pluto6v_dev, pluto6, pluto6_state, empty_init, ROT0, "Heber", "Pluto 6 Devkit/Evaluation Board Video Demo", MACHINE_NOT_WORKING )
//GAME( 2000, pl6ddemo, 0, pluto6dv_dev, pluto6, pluto6_state, empty_init, ROT0, "Heber", "Pluto 6 Devkit/Evaluation Board Dual Video Demo", MACHINE_NOT_WORKING )
