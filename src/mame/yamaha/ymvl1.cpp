// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha VL1 synthesizer

// The VL1-m rackable version exists but we don't have the firmware

// Waiting on tx possible in serial 2, need to implement decent serial
// in the 68301 to go on

// https://www.synthxl.com/offwp/yamaha_vl1_m_service_manual.pdf

// Address decode:
//  2222 1111 1111 1100 0000 0000
//  3210 9876 5432 1098 7654 3210

//  1... .... 000. .... ..0. .... .w led
//  1... .... 011. .... ..0. .... .w pks
//  1... .... 011. .... ..0. .... r. adc
//
//  1... .... 1000 .... .... .... rw vop
//  1... .... 1010 .... .... .... rw glcd
//  1... .... 1011 .... .... .... rw fdc
//
//  0100 0... .... .... .... .... rw raml
//  0100 1... .... .... .... .... rw ramh
//  0101 0... .... .... .... .... rw ssel1
//  0101 1... .... .... .... .... rw ssel2
//
//  1... .... 1000 .... .... .... rw dvop
//  1... .... 1010 .... .... .... rw lcd
//  1... .... 1011 .... .... .... rw dfdc


#include "emu.h"
#include "machine/tmp68301.h"
#include "video/t6963c.h"

namespace {

class vl1_state : public driver_device
{
public:
	vl1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd(*this, "lcd")
	{ }

	void vl1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<tmp68301_device> m_maincpu;
	required_device<lm24014h_device> m_lcd;

	u16 m_led;

	void maincpu_map(address_map &map);

	void led_w(u16 data);
};

void vl1_state::led_w(u16 data)
{
	u8 activated = (((~m_led) & data) >> 8) & 0xf;
	if(activated) {
		if(activated & 1)
			logerror("led.0 %02x\n", data & 0xff);
		if(activated & 2)
			logerror("led.1 %02x\n", data & 0xff);
		if(activated & 4)
			logerror("led.2 %02x\n", data & 0xff);
		if(activated & 8)
			logerror("led.3 %02x\n", data & 0xff);
	}
	m_led = data;
}

void vl1_state::machine_start()
{
	m_led = 0;
}

void vl1_state::maincpu_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x400000, 0x4fffff).ram();
	map(0x800000, 0x800001).w(FUNC(vl1_state::led_w));
	map(0x80a000, 0x80a003).rw(m_lcd, FUNC(lm24014h_device::read), FUNC(lm24014h_device::write)).umask16(0x00ff);
}

void vl1_state::vl1(machine_config &config)
{
	TMP68301(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vl1_state::maincpu_map);

	LM24014H(config, m_lcd);
}

static INPUT_PORTS_START(vl1)
INPUT_PORTS_END

ROM_START(vl1)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE("vl1v2h.ic26", 0, 0x80000, CRC(f4573379) SHA1(55f6423ce436d6472b80d3db5944881c60a42fcf))
	ROM_LOAD16_BYTE("vl1v2l.ic67", 1, 0x80000, CRC(382d24e9) SHA1(4e6637c8e1876c5931f944f8a188d58cdb1eb47f))
ROM_END

} // anonymous namespace

SYST(1993, vl1, 0, 0, vl1, vl1, vl1_state, empty_init, "Yamaha", "VL1", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
