// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

P's Attack (c) 2004 Uniana

based off original crystal.cpp by ElSemi

TODO:
- Compact Flash hookup;
- Requires timed based FIFO renderer, loops until both rear and front are equal.
- Enables wavetable IRQ, even if so far no channel enables the submask;
- Unemulated 93C86 EEPROM device;

===================================================================================================

P's Attack (c) 2004 Uniana Co., Ltd

+----------54321---654321--654321---------------------------+
|VOL       TICKET  GUN_1P  GUN_2P                 +---------|
|                                                 |         |
+-+                                               |  256MB  |
  |       CC-DAC                                  | Compact |
+-+                                  EMUL*        |  Flash  |
|                                                 |         |
|5          +---+                                 +---------|
|6          |   |                                           |
|P          | R |   25.1750MHz              +--------------+|
|I          | A |                           |     42Pin*   ||
|N          | M |                           +--------------+|
|           |   |                           +--------------+|
|C          +---+       +------------+      |     SYS      ||
|O                      |            |      +--------------+|
|N          +---+       |            |                      |
|N          |   |       |VRenderZERO+|                      |
|E SERVICE  | R |       | MagicEyes  |  +-------+    62256* |
|C          | A |       |            |  |  RAM  |           |
|T TEST     | M |       |            |  +-------+    62256* |
|O          |   |       +------------+                      |
|R RESET    +---+                                           |
|                                   14.31818MHz             |
+-+                                                         |
  |                                EEPROM                   |
+-+                GAL                                 DSW  |
|                                                           |
|  VGA                           PIC               BAT3.6V* |
+-----------------------------------------------------------+

* denotes unpopulated device

RAM are Samsung K4S641632H-TC75
VGA is a standard PC 15 pin VGA connection
DSW is 2 switch dipswitch (switches 3-8 are unpopulated)
PIC is a Microchip PIC16C711-041/P (silkscreened on the PCB as COSTOM)
SYS is a ST M27C160 EPROM (silkscreened on the PCB as SYSTEM_ROM_32M)
GAL is a GAL16V8B (not dumped)
EMUL is an unpopulated 8 pin connector
EEPROM is a 93C86 16K 5.0v Serial EEPROM (2048x8-bit or 1024x16-bit)
CC-DAC is a TDA1311A Stereo Continuous Calibration DAC


 P's Attack non JAMMA standard 56pin Edge Connector Pinout:

                          56pin Edge Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
       Player 1 Start Lamp   | E | 5 |         Coin Lamp
             +12             | F | 6 |             +12
------------ KEY ------------| G | 7 |------------ KEY -----------
       Player 2 Start Lamp   | H | 8 |        Coin Counter
        L Speaker (-)        | J | 9 |        L Speaker (+)
        R Speaker (-)        | K | 10|        R Speaker (+)
     Video Vertical Sync     | L | 11|
        Video Green          | M | 12|        Video Red
        Video Sync           | N | 13|        Video Blue
        Service Switch       | P | 14|        Video GND
    Video Horizontal Sync    | R | 15|        Test Switch
                             | S | 16|        Coin Switch
       Start Player 2        | T | 17|        Start Player 1
                             | U | 18|
                             | V | 19|
                             | W | 20|
                             | X | 21|
                             | Y | 22|
                             | a | 23|
                             | b | 24|
                             | d | 25|
                             | e | 26|
             GND             | f | 27|             GND
             GND             | g | 28|             GND


TICKET is a 5 pin connector:

  1| LED
  2| GND
  3| OUT
  4| IN
  5| +12v

GUN_xP are 6 pin gun connectors (pins 3-6 match the UNICO sytle guns):

 GUN-1P: Left (Blue) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

 GUN-2P: Right (Pink) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

**************************************************************************************************/

#include "emu.h"
#include "bus/ata/ataintf.h"
#include "cpu/se3208/se3208.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/vrender0.h"

#include "emupal.h"
#include "speaker.h"

namespace {

class psattack_state : public driver_device
{
public:
	psattack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_ata(*this, "ata"),
		m_eeprom(*this, "eeprom")
	{ }


	void psattack(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u32> m_workram;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<ata_interface_device> m_ata;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	void main_map(address_map &map) ATTR_COLD;

	u16 cfcard_data_r();
	u8 cfcard_regs_r(offs_t offset);
	void cfcard_regs_w(offs_t offset, u8 data);
	void output_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};

// TODO: wrong, likely PIC protected too
u8 psattack_state::cfcard_regs_r(offs_t offset)
{
	return m_ata->cs0_r(offset & 7, 0x000000ff);
}

void psattack_state::cfcard_regs_w(offs_t offset, u8 data)
{
	m_ata->cs0_w(offset & 7, 0x000000ff);
}

u16 psattack_state::cfcard_data_r()
{
	// TODO: may not be it (pushes data into stack then never read it other than a comparison check from +0xfc)
	// also not unlocked (keeps returning 0xffff data)
	return m_ata->cs0_r(0, 0x0000ffff);
}

void psattack_state::output_w(offs_t offset, u32 data, u32 mem_mask)
{
	// suppress logging for now
	if (data)
		logerror("%s: output_w: %08x & %08x\n", machine().describe_context(), data, mem_mask);
}

void psattack_state::main_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().nopw();

	//   0x1400c00, 0x1400c01 read cfcard memory (auto increment?)
	//   0x1402204 wants bit 1 on, handshake from PIC?
	//   0x1402800, 0x1402807 read/write regs?
	// cf card interface
	map(0x01400c00, 0x01400c01).r(FUNC(psattack_state::cfcard_data_r));
	map(0x01402800, 0x01402807).rw(FUNC(psattack_state::cfcard_regs_r), FUNC(psattack_state::cfcard_regs_w));

	map(0x01500000, 0x01500003).portr("IN0").w(FUNC(psattack_state::output_w));
	map(0x01500004, 0x01500007).portr("IN1");
	map(0x01500008, 0x0150000b).portr("IN2");
	map(0x0150000e, 0x0150000e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_eeprom->cs_write(BIT(data, 0));
			m_eeprom->clk_write(BIT(data, 1));
			m_eeprom->di_write(BIT(data, 2));
			// TODO: other bits used
		})
	);

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
//  map(0x01802410, 0x01802413) peripheral chip select for cf?

	map(0x02000000, 0x027fffff).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}

static INPUT_PORTS_START( psattack )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: probably coin/start etc.
	PORT_START("IN2")
	PORT_BIT( 0xfffbffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

void psattack_state::machine_start()
{
	// ...
}

void psattack_state::machine_reset()
{
	// ...
}

void psattack_state::psattack(machine_config &config)
{
	// TODO: dynamic via PLL
	SE3208(config, m_maincpu, 14318180 * 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &psattack_state::main_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	// PIC16C711

	// TODO: dynamic via PLL
	VRENDER0_SOC(config, m_vr0soc, 14318180 * 6);
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(XTAL(25'175'000)); // assumed from the only available XTal on PCB

	ATA_INTERFACE(config, m_ata).options(ata_devices, "cf", nullptr, true);
//	m_ata->irq_handler().set([this] (int state) { printf("irq %d\n", state); });
//	m_ata->dmarq_handler().set([this] (int state) { printf("dmarq %d\n", state); });

	EEPROM_93C86_16BIT(config, m_eeprom, 0);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}

ROM_START( psattack )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("5.sys",  0x000000, 0x200000, CRC(f09878e4) SHA1(25b8dbac47d3911615c8874746e420ece13e7181) )

	ROM_REGION( 0x4010, "pic16c711", 0 )
	ROM_LOAD("16c711.pic",  0x0000, 0x137b, CRC(617d8292) SHA1(d32d6054ce9db2e31efaf41015afcc78ed32f6aa) ) // raw dump
	ROM_LOAD("16c711.bin",  0x0000, 0x4010, CRC(b316693f) SHA1(eba1f75043bd415268eedfdb95c475e73c14ff86) ) // converted to binary

	DISK_REGION( "ata:0:cf" )
	DISK_IMAGE( "psattack", 0, SHA1(e99cd0dafc33ec13bf56061f81dc7c0a181594ee) )
ROM_END


} // anonymous namespace


GAME( 2004, psattack, 0,        psattack, psattack,  psattack_state, empty_init, ROT0, "Uniana", "P's Attack", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )

