// license:BSD-3-Clause
// copyright-holders:Joseph Khilfeh

/*

Linksys PAP2 two-port analog telephone adapter

ESS Visba 3 ES3890F @ 27MHz (CPU/MCU)
Realtek RTL8019AS (NIC)
ICSI IC41LV16100S-50KG (2MB RAM)
SST39VF080-70-4C-EI (1MB flash ROM)
2x Silicon Labs Si3210-KT (line interfaces)

Four of the five LEDs are controlled by ES3890F GPIOs
Power LEDs are connected to AUX1[4] and AUX1[5] (pin 112 and 113)
Phone 1 LED is connected to AUX1[6] (pin 114)
Phone 2 LED is connected to AUX1[7] (pin 115)
Ethernet LED is connected to RTL8019AS somehow

RTL8019AS pin 65 is tied to VDD to disable PnP
RTL8019AS pin 96 is tied to ground to share the flash ROM's 8-bit data bus
RTL8019AS registers are selected when ES3890F pin 19 (/CS1) is low
It looks like the RTL8019AS EEPROM pins are connected to ES3890F AUX2[0-3]?


There are several ATAs based on essentially the same hardware:

Sipura SPA-2000
Sipura SPA-1000
Sipura SPA-3000 (ES3890F, RTL8019AS, 2MB RAM, 1MB flash, unknown line interfaces)
Sipura SPA-1001 (ES3890F, RTL8019AS, 2MB RAM, 1MB flash, 1x Si3210)
Sipura SPA-2100 (Sipura SIP316F @ 27MHz, 2x RTL8019AS, 8MB RAM, 2MB flash, unknown line interfaces)
Sipura SPA-2002
Linksys PAP2T (ES3890F, RTL8019AS, 2MB RAM, 1MB flash, 2x Si3215)
Linksys SPA2102?

There are also ATAs with similar names but probably very different hardware:

Linksys PAP2 V2
Cisco SPA112
Cisco SPA122

*/

#include "emu.h"

#include "cpu/mipsx/mipsx.h"


namespace {

class pap2_state : public driver_device

{
public:
	pap2_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void pap2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<mipsx_cpu_device> m_maincpu;

	void mem(address_map &map) ATTR_COLD;
};


void pap2_state::machine_reset()
{
	m_maincpu->set_state_int(STATE_GENPC, 0x1cffff80); // might actually be 0x7fffff80 with a ROM mirror
}

void pap2_state::mem(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0x1c000000, 0x1c0fffff).mirror(0x00f00000).rom().region("maincpu", 0);
	// ES3890F registers at 0x20000000-0x2000ffff, ES6008 datasheet could be helpful
}

void pap2_state::pap2(machine_config &config)
{
	MIPSX(config, m_maincpu, 27_MHz_XTAL * 3); // guessing 3x from ES3880 datasheet
	m_maincpu->set_addrmap(AS_PROGRAM, &pap2_state::mem);
}

INPUT_PORTS_START(pap2)
INPUT_PORTS_END

ROM_START(pap2)
	ROM_REGION32_BE(0x100000, "maincpu", 0 )
	ROM_LOAD("linksys-pap2-2.0.12-ls.u51", 0x000000, 0x100000, BAD_DUMP CRC(4d0f1e5d) SHA1(73b163b00a3709a14f7419283c8515dd91009598) )
	// Original ROM label is unknown, if it had one. S/N FH900DC35989, MAC 00:12:17:FB:70:DC
ROM_END

}

SYST( 200?, pap2, 0, 0, pap2, pap2, pap2_state, empty_init, "Linksys (Cisco)", "PAP2", MACHINE_IS_SKELETON )
