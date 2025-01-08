// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha VL1 synthesizer

// The VL1-m rackable version exists but we don't have the firmware

// https://www.synthxl.com/offwp/yamaha_vl1_m_service_manual.pdf

// Address decode:
//  2222 1111 1111 1100 0000 0000
//  3210 9876 5432 1098 7654 3210

//  1... .... 000. .... ..0. .... .w led
//  1... .... 010. .... ..0. .... r. pks
//  1... .... 011. .... ..0. .... r. adc
//
//  1... .... 1000 .... .... .... rw vop (dspv #4)
//  1... .... 1010 .... .... .... rw lcd
//  1... .... 1011 .... .... .... rw fdc
//
//  0100 0... .... .... .... .... rw raml
//  0100 1... .... .... .... .... rw ramh
//  0101 0... .... .... .... .... rw ssel1 (top gate array)
//  0101 1... .... .... .... .... rw ssel2 (bottom gate array)

//  Uses 4 dsp-v which each have two inputs and two outputs.  Their
//  stream inputs/outputs are connected thus:
//    #1.2 -> #3.2
//    #2.1 -> #3.1
//    #3.1 -> #4.1
//    #3.2 -> #4.2
//    #4.1 -> dac

// Uses 3 tmp68301.  The main one controls dspv #4.  The first subcpu
// controls dspvs #2 and #3, the second #1, Each subgroup is tucked
// behind a gate array, selected by ssel1/2.

// There's also an undumped 63b01 dubbed "pks" which scans the
// switches and has one 8-bit port and an irq line that goes to the
// main tmp.


#include "emu.h"
#include "cpu/m68000/tmp68301.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "sound/dspv.h"
#include "video/t6963c.h"

namespace {

class vl1_state : public driver_device
{
public:
	vl1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu1(*this, "subcpu1"),
		m_subcpu2(*this, "subcpu2"),
		m_dspv1(*this, "dspv1"),
		m_dspv2(*this, "dspv2"),
		m_dspv3(*this, "dspv3"),
		m_dspv4(*this, "dspv4"),
		m_lcd(*this, "lcd"),
		m_fdc(*this, "fdc"),
		m_mem_s1(*this, "subcpu1_ram"),
		m_mem_s2(*this, "subcpu2_ram")
	{ }

	void vl1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<tmp68301_device> m_maincpu, m_subcpu1, m_subcpu2;
	required_device<dspv_device> m_dspv1, m_dspv2, m_dspv3, m_dspv4;
	required_device<lm24014h_device> m_lcd;
	required_device<hd63266f_device> m_fdc;

	required_shared_ptr<u16> m_mem_s1, m_mem_s2;
	u16 m_led, m_main_ctrl, m_sub1_ctrl, m_sub2_ctrl;

	void maincpu_map(address_map &map) ATTR_COLD;
	void subcpu1_map(address_map &map) ATTR_COLD;
	void subcpu2_map(address_map &map) ATTR_COLD;

	u16 main_r();
	void main_w(u16 data);
	u16 sub1_r();
	void sub1_w(u16 data);
	u16 sub2_r();
	void sub2_w(u16 data);

	void led_w(u16 data);

	static void hd_floppy(device_slot_interface &device);
};

void vl1_state::led_w(u16 data)
{
	u8 activated = (((~m_led) & data) >> 8) & 0xf;
	if(activated && 0) {
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

// Parallel port:
//   o 0-3: adsel 1-4 | breath controller adc & muxer
//   o 4  : adst      |
//   i 5  : eoc       |
//   o 7  : inh       |
//   o 8-9: bshalt 1-2
//   i a  : fdc irq
//   i b  : fdc hd out (hd floppy detection)
//   i d-e: re 1-2
//   o f  : reclr

u16 vl1_state::main_r()
{
	if(0)
		logerror("main_r\n");
	return m_main_ctrl;
}

void vl1_state::main_w(u16 data)
{
	if(0)
		logerror("main_w adsel=%x adst=%d inh=%d bshalt=%d reclr=%d\n",
				 BIT(data, 0, 4),
				 BIT(data, 4),
				 BIT(data, 7),
				 BIT(data, 8, 2),
				 BIT(data, 15));

	m_main_ctrl = data;
	m_subcpu1->set_input_line(INPUT_LINE_HALT, !BIT(data, 8));
	m_subcpu2->set_input_line(INPUT_LINE_HALT, !BIT(data, 9));
}

u16 vl1_state::sub1_r()
{
	return m_sub1_ctrl;
}

void vl1_state::sub1_w(u16 data)
{
	if(1)
		logerror("sub1_w dsp = %d %d\n",
				 BIT(data, 0),
				 BIT(data, 1));

	m_sub1_ctrl = data;
	m_dspv2->set_input_line(INPUT_LINE_RESET, !BIT(data, 0));
	m_dspv3->set_input_line(INPUT_LINE_RESET, !BIT(data, 1));
}

u16 vl1_state::sub2_r()
{
	return m_sub2_ctrl;
}

void vl1_state::sub2_w(u16 data)
{
	if(1)
		logerror("sub2_w dsp = %d\n",
				 BIT(data, 0));

	m_sub2_ctrl = data;
	m_dspv1->set_input_line(INPUT_LINE_RESET, !BIT(data, 0));
}

void vl1_state::machine_start()
{
	save_item(NAME(m_main_ctrl));
	save_item(NAME(m_sub1_ctrl));
	save_item(NAME(m_sub2_ctrl));
	save_item(NAME(m_led));
}

void vl1_state::machine_reset()
{
	m_led = 0;
	m_main_ctrl = 0;
	m_sub1_ctrl = 0;
	m_sub2_ctrl = 0;

	m_subcpu1->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_subcpu2->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_dspv1->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dspv2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dspv3->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dspv4->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void vl1_state::maincpu_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x400000, 0x4fffff).ram();
	map(0x500000, 0x53ffff).ram().share(m_mem_s1);
	map(0x580000, 0x5bffff).ram().share(m_mem_s2);
	map(0x800000, 0x800001).w(FUNC(vl1_state::led_w));
	map(0x804000, 0x804000).lr8(NAME([]() -> u8 { return 0; })); // pks
	map(0x806000, 0x806000).lr8(NAME([]() -> u8 { return 0; })); // adc
	map(0x808000, 0x80807f).m(m_dspv4, FUNC(dspv_device::map));
	map(0x80a000, 0x80a003).rw(m_lcd, FUNC(lm24014h_device::read), FUNC(lm24014h_device::write)).umask16(0x00ff);
	map(0x80b000, 0x80b003).m(m_fdc, FUNC(hd63266f_device::map)).umask16(0x00ff);
}

void vl1_state::subcpu1_map(address_map &map)
{
	map(0x000000, 0x03ffff).ram().share(m_mem_s1);
	map(0x040000, 0x04007f).m(m_dspv2, FUNC(dspv_device::map));
	map(0x050000, 0x05007f).m(m_dspv3, FUNC(dspv_device::map));
}

void vl1_state::subcpu2_map(address_map &map)
{
	map(0x000000, 0x03ffff).ram().share(m_mem_s2);
	map(0x040000, 0x04007f).m(m_dspv1, FUNC(dspv_device::map));
}


void vl1_state::hd_floppy(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void vl1_state::vl1(machine_config &config)
{
	TMP68301(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vl1_state::maincpu_map);
	m_maincpu->parallel_r_cb().set(FUNC(vl1_state::main_r));
	m_maincpu->parallel_w_cb().set(FUNC(vl1_state::main_w));
	m_maincpu->tx2_handler().set(m_subcpu1, FUNC(tmp68301_device::rx2_w));
	m_maincpu->tx2_handler().append(m_subcpu2, FUNC(tmp68301_device::rx2_w));

	TMP68301(config, m_subcpu1, 16_MHz_XTAL);
	m_subcpu1->set_addrmap(AS_PROGRAM, &vl1_state::subcpu1_map);
	m_subcpu1->parallel_r_cb().set(FUNC(vl1_state::sub1_r));
	m_subcpu1->parallel_w_cb().set(FUNC(vl1_state::sub1_w));

	TMP68301(config, m_subcpu2, 16_MHz_XTAL);
	m_subcpu2->set_addrmap(AS_PROGRAM, &vl1_state::subcpu2_map);
	m_subcpu2->parallel_r_cb().set(FUNC(vl1_state::sub2_r));
	m_subcpu2->parallel_w_cb().set(FUNC(vl1_state::sub2_w));

	DSPV(config, m_dspv1, 24.576_MHz_XTAL);
	DSPV(config, m_dspv2, 24.576_MHz_XTAL);
	DSPV(config, m_dspv3, 24.576_MHz_XTAL);
	DSPV(config, m_dspv4, 24.576_MHz_XTAL);

	HD63266F(config, m_fdc, 16_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	FLOPPY_CONNECTOR(config, "fdc:0", hd_floppy, "35hd", floppy_image_device::default_pc_floppy_formats);

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
