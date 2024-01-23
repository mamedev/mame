// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson S1C33209/221/222 CMOS 32-bit single chip microcomputer
*/

#include "emu.h"
#include "s1c33209.h"


DEFINE_DEVICE_TYPE(S1C33209, s1c33209_device, "s1c33209", "Epson S1C33209")
DEFINE_DEVICE_TYPE(S1C33221, s1c33221_device, "s1c33221", "Epson S1C33221")
DEFINE_DEVICE_TYPE(S1C33222, s1c33222_device, "s1c33222", "Epson S1C33222")


s1c33209_device::s1c33209_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock,
		address_map_constructor internal_map) :
	c33std_cpu_device_base(mconfig, type, tag, owner, clock, internal_map)
{
}

s1c33209_device::s1c33209_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	s1c33209_device(
			mconfig,
			S1C33209,
			tag,
			owner,
			clock,
			address_map_constructor(FUNC(s1c33209_device::memory_map<0>), this))
{
}

s1c33221_device::s1c33221_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	s1c33209_device(
			mconfig,
			S1C33221,
			tag,
			owner,
			clock,
			address_map_constructor(FUNC(s1c33221_device::memory_map<128 * 1024>), this))
{
}

s1c33222_device::s1c33222_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	s1c33209_device(
			mconfig,
			S1C33222,
			tag,
			owner,
			clock,
			address_map_constructor(FUNC(s1c33222_device::memory_map<64 * 1024>), this))
{
}


void s1c33209_device::device_reset()
{
	c33std_cpu_device_base::device_reset();

	u32 const boot_vector = m_data.read_dword(0x0c0'0000);
	m_pc = boot_vector;
}


template <offs_t RomBytes>
void s1c33209_device::memory_map(address_map &map)
{
	map(0x000'0000, 0x000'1fff).mirror(0x000'2000).ram();
	map(0x003'0000, 0x003'ffff).m(*this, FUNC(s1c33209_device::peripheral_map));
	map(0x004'0000, 0x004'ffff).mirror(0x001'0000).m(*this, FUNC(s1c33209_device::peripheral_map));
	//map(0x006'0000, 0x007'ffff) area 2     reserved for debug mode
	//map(0x008'0000, 0xfff'ffff) area 3     reserved for middleware
	//map(0x010'0000, 0x02f'ffff) area 4-5   external memory
	//map(0x030'0000, 0x037'ffff) area 6     external 8-bit I/O
	//map(0x038'0000, 0x03f'ffff) area 6     external 8-bit I/O
	//map(0x040'0000, 0x0bf'ffff) area 7-9   external memory
	if (RomBytes)
		map(0xc0'0000, 0x0c0'0000 + RomBytes - 1).rom().region(DEVICE_SELF, 0);
	//map(0x0c.'...., 0x0ff'ffff) area 10    external memory
	//map(0x100'0000, 0xfff'ffff) area 11-18 external memory
}

void s1c33209_device::peripheral_map(address_map &map)
{
	//map(0x0140, 0x0140) 8-bit timer 4-5 clock select
	//map(0x0145, 0x0145) 8-bit timer 4-5 clock control
	//map(0x0146, 0x0146) 8-bit timer 0-3 clock select
	//map(0x0147, 0x0147) 16-bit timer 0 clock control
	//map(0x0148, 0x0148) 16-bit timer 1 clock control
	//map(0x0149, 0x0149) 16-bit timer 2 clock control
	//map(0x014a, 0x014a) 16-bit timer 3 clock control
	//map(0x014b, 0x014b) 16-bit timer 4 clock control
	//map(0x014c, 0x014c) 16-bit timer 5 clock control
	//map(0x014d, 0x014d) 8-bit timer 0-1 clock control
	//map(0x014e, 0x014e) 8-bit timer 2-3 clock control
	//map(0x014f, 0x014f) A/D clock control

	//map(0x0151, 0x0151) clock timer run/stop
	//map(0x0152, 0x0152) clock timer interrupt control
	//map(0x0153, 0x0153) clock timer divider
	//map(0x0154, 0x0154) clock timer second
	//map(0x0155, 0x0155) clock timer minute
	//map(0x0156, 0x0156) clock timer hour
	//map(0x0157, 0x0157) clock day (low)
	//map(0x0158, 0x0158) clock day (high)
	//map(0x0159, 0x0159) clock minute comparison
	//map(0x015a, 0x015a) clock hour comparison
	//map(0x015b, 0x015b) clock day comparison

	//map(0x0160, 0x0160) 8-bit timer 0 control
	//map(0x0161, 0x0161) 8-bit timer 0 reload
	//map(0x0162, 0x0162) 8-bit timer 0 counter data
	//map(0x0164, 0x0164) 8-bit timer 1 control
	//map(0x0165, 0x0165) 8-bit timer 1 reload
	//map(0x0166, 0x0166) 8-bit timer 1 counter data
	//map(0x0168, 0x0168) 8-bit timer 2 control
	//map(0x0169, 0x0169) 8-bit timer 2 reload
	//map(0x016a, 0x016a) 8-bit timer 2 counter data
	//map(0x016c, 0x016c) 8-bit timer 3 control
	//map(0x016d, 0x016d) 8-bit timer 3 reload
	//map(0x016e, 0x016e) 8-bit timer 3 counter data
	//map(0x0170, 0x0170) watchdog timer write-protect
	//map(0x0171, 0x0171) watchdog timer enable
	//map(0x0174, 0x0174) 8-bit timer 4 control
	//map(0x0175, 0x0175) 8-bit timer 4 reload
	//map(0x0176, 0x0176) 8-bit timer 4 counter data
	//map(0x0178, 0x0178) 8-bit timer 5 control
	//map(0x0179, 0x0179) 8-bit timer 5 reload
	//map(0x017a, 0x017a) 8-bit timer 5 counter data
	//map(0x0180, 0x0180) power control
	//map(0x0181, 0x0181) prescaler clock select
	//map(0x0190, 0x0190) clock option
	//map(0x019e, 0x019e) power control protect

	//map(0x01e0, 0x01e0) serial interface ch. 0 transmit data
	//map(0x01e1, 0x01e1) serial interface ch. 0 receive data
	//map(0x01e2, 0x01e2) serial interface ch. 0 status
	//map(0x01e3, 0x01e3) serial interface ch. 0 control
	//map(0x01e4, 0x01e4) serial interface ch. 0 IrDA
	//map(0x01e5, 0x01e5) serial interface ch. 1 transmit data
	//map(0x01e6, 0x01e6) serial interface ch. 1 receive data
	//map(0x01e7, 0x01e7) serial interface ch. 1 status
	//map(0x01e8, 0x01e8) serial interface ch. 1 control
	//map(0x01e9, 0x01e9) serial interface ch. 1 IrDA
	//map(0x01f0, 0x01f0) serial interface ch. 2 transmit data
	//map(0x01f1, 0x01f1) serial interface ch. 2 receive data
	//map(0x01f2, 0x01f2) serial interface ch. 2 status
	//map(0x01f3, 0x01f3) serial interface ch. 2 control
	//map(0x01f4, 0x01f4) serial interface ch. 2 IrDA
	//map(0x01f5, 0x01f5) serial interface ch. 3 transmit data
	//map(0x01f6, 0x01f6) serial interface ch. 3 receive data
	//map(0x01f7, 0x01f7) serial interface ch. 3 status
	//map(0x01f8, 0x01f8) serial interface ch. 3 control
	//map(0x01f9, 0x01f9) serial interface ch. 3 IrDA

	//map(0x0240, 0x0240) A/D conversion result (low)
	//map(0x0241, 0x0241) A/D conversion result (high)
	//map(0x0242, 0x0242) A/D trigger
	//map(0x0243, 0x0243) A/D channel
	//map(0x0244, 0x0244) A/D enable
	//map(0x0245, 0x0245) A/D sampling

	//map(0x0260, 0x0260) port 0-1 interrupt priority
	//map(0x0261, 0x0261) port 2-3 interrupt priority
	//map(0x0262, 0x0262) key input interrupt priority
	//map(0x0263, 0x0263) high-speed DMA ch.0-1 interrupt priority
	//map(0x0264, 0x0264) high-speed DMA ch.2-3 interrupt priority
	//map(0x0265, 0x0265) IDMA interrupt priority
	//map(0x0266, 0x0266) 16-bit timer 0-1 interrupt priority
	//map(0x0267, 0x0267) 16-bit timer 2-3 interrupt priority
	//map(0x0268, 0x0268) 16-bit timer 4-5 interrupt priority
	//map(0x0269, 0x0269) 8-bit timer and serial interface ch. 0 interrupt priority
	//map(0x026a, 0x026a) serial interface ch. 1 and A/D interrupt priority
	//map(0x026b, 0x026b) clock timer interrupt priority
	//map(0x026c, 0x026c) port input 4-5 interrupt priority
	//map(0x026d, 0x026d) port input 6-7 interrupt priority
	//map(0x0270, 0x0270) key input and port 0-3 interrupt enable
	//map(0x0271, 0x0271) DMA interrupt enable
	//map(0x0272, 0x0272) 16-bit timer 0-1 interrupt enable
	//map(0x0273, 0x0273) 16-bit timer 2-3 interrupt enable
	//map(0x0274, 0x0274) 16-bit timer 4-5 interrupt enable
	//map(0x0275, 0x0275) 8-bit timer interrupt enable
	//map(0x0276, 0x0276) serial interface interrupt enable
	//map(0x0277, 0x0277) port input 4-7, clock timer and A/D interrupt enable
	//map(0x0280, 0x0280) key input and port 0-3 interrupt factor flag
	//map(0x0281, 0x0281) DMA interrupt factor flag
	//map(0x0282, 0x0282) 16-bit timer 0-1 interrupt factor flag
	//map(0x0283, 0x0283) 16-bit timer 2-3 interrupt factor flag
	//map(0x0284, 0x0284) 16-bit timer 4-5 interrupt factor flag
	//map(0x0285, 0x0285) 8-bit timer interrupt factor flag
	//map(0x0286, 0x0286) serial interface interrupt factor flag
	//map(0x0287, 0x0287) port input 4-7, clock timer and A/D interrupt factor flag
	//map(0x0290, 0x0290) port input 0-3, high-speed DMA ch. 0-1 and 16-bit timer 0 IDMA request
	//map(0x0291, 0x0291) 16-bit timer 1-4 IDMA request
	//map(0x0292, 0x0292) 16-bit timer 5, 8-bit timer and serial interface ch. 0 IDMA request
	//map(0x0293, 0x0293) serial interface ch. 1, A/D and port input 4-7 IDMA request
	//map(0x0294, 0x0294) port input 0-3, high-speed DMA ch. 0-1 and 16-bit timer 0 IDMA enable
	//map(0x0295, 0x0295) 16-bit timer 1-4 IDMA enable
	//map(0x0296, 0x0296) 16-bit timer 5, 8-bit timer and serial interface ch. 0 IDMA enable
	//map(0x0297, 0x0297) serial interface ch. 1, A/D and port input 4-7 IDMA enable
	//map(0x0298, 0x0298) high-speed DMA ch. 0-1 trigger setup
	//map(0x0299, 0x0299) high-speed DMA ch. 2-3 trigger setup
	//map(0x029a, 0x029a) high-speed DMA software trigger
	//map(0x029f, 0x029f) flag set/reset method select

	//map(0x02c0, 0x02c0) K5 function select
	//map(0x02c1, 0x02c1) K5 input port data
	//map(0x02c3, 0x02c3) K6 function select
	//map(0x02c4, 0x02c4) K6 input port data
	//map(0x02c5, 0x02c5) interrupt factor FP function switching
	//map(0x02c6, 0x02c6) port input interrupt select 1
	//map(0x02c7, 0x02c7) port input interrupt select 2
	//map(0x02c8, 0x02c8) port input interrupt input polarity select
	//map(0x02c9, 0x02c9) port input interrupt edge/level select
	//map(0x02ca, 0x02ca) key input interrupt select
	//map(0x02cb, 0x02cb) interrupt factor TM16 function switching
	//map(0x02cc, 0x02cc) key input interrupt (FPK0) input comparison
	//map(0x02cd, 0x02cd) key input interrupt (FPK1) input comparison
	//map(0x02ce, 0x02ce) key input interrupt (FPK0) input mask
	//map(0x02cf, 0x02cf) key input interrupt (FPK1) input mask
	//map(0x02d0, 0x02d0) P0 function select
	//map(0x02d1, 0x02d1) P0 I/O port data
	//map(0x02d2, 0x02d2) P0 I/O control
	//map(0x02d4, 0x02d4) P1 function select
	//map(0x02d5, 0x02d5) P1 I/O port data
	//map(0x02d6, 0x02d6) P1 I/O control
	//map(0x02d7, 0x02d7) port SIO function extension
	//map(0x02d8, 0x02d8) P2 function select
	//map(0x02d9, 0x02d9) P2 I/O port data
	//map(0x02da, 0x02da) P2 I/O control
	//map(0x02db, 0x02db) port SIO function extension
	//map(0x02dc, 0x02dc) P2 function select
	//map(0x02dd, 0x02dd) P2 I/O port data
	//map(0x02de, 0x02de) P2 I/O control
	//map(0x02df, 0x02df) port function extension

	//map(0x8120, 0x8121) area 15-18 setup
	//map(0x8122, 0x8123) area 13-14 setup
	//map(0x8124, 0x8125) area 11-12 setup
	//map(0x8126, 0x8127) area 9-10 setup
	//map(0x8128, 0x8129) area 7-8 setup
	//map(0x812a, 0x812b) area 4-6 setup
	//map(0x812d, 0x812d) TTBR write protect
	//map(0x812e, 0x812e) bus control
	//map(0x8130, 0x8131) DRAM timing setup
	//map(0x8132, 0x8133) access control
	//map(0x8134, 0x8135) TTBR low
	//map(0x8136, 0x8137) TTBR high
	//map(0x8138, 0x8139) G/A read signal control
	//map(0x813a, 0x813a) BCLK select

	//map(0x8180, 0x8181) 16-bit timer 0 comparison A
	//map(0x8182, 0x8183) 16-bit timer 0 comparison B
	//map(0x8184, 0x8185) 16-bit timer 0 counter data
	//map(0x8186, 0x8186) 16-bit timer 0 control
	//map(0x8188, 0x8189) 16-bit timer 1 comparison A
	//map(0x818a, 0x818b) 16-bit timer 1 comparison B
	//map(0x818c, 0x818d) 16-bit timer 1 counter data
	//map(0x818e, 0x818e) 16-bit timer 1 control
	//map(0x8190, 0x8191) 16-bit timer 2 comparison A
	//map(0x8192, 0x8193) 16-bit timer 2 comparison B
	//map(0x8194, 0x8195) 16-bit timer 2 counter data
	//map(0x8196, 0x8196) 16-bit timer 2 control
	//map(0x8198, 0x8199) 16-bit timer 3 comparison A
	//map(0x819a, 0x819b) 16-bit timer 3 comparison B
	//map(0x819c, 0x819d) 16-bit timer 3 counter data
	//map(0x819e, 0x819e) 16-bit timer 3 control
	//map(0x81a0, 0x81a1) 16-bit timer 4 comparison A
	//map(0x81a2, 0x81a3) 16-bit timer 4 comparison B
	//map(0x81a4, 0x81a5) 16-bit timer 4 counter data
	//map(0x81a6, 0x81a6) 16-bit timer 4 control
	//map(0x81a8, 0x81a9) 16-bit timer 5 comparison A
	//map(0x81aa, 0x81ab) 16-bit timer 5 comparison B
	//map(0x81ac, 0x81ad) 16-bit timer 5 counter data
	//map(0x81ae, 0x81ae) 16-bit timer 5 control

	//map(0x8200, 0x8201) IDMA base address low
	//map(0x8202, 0x8203) IDMA base address high
	//map(0x8204, 0x8204) IDMA start
	//map(0x8205, 0x8205) IDMA enable

	//map(0x8220, 0x8221) high-speed DMA ch. 0 transfer counter
	//map(0x8222, 0x8223) high-speed DMA ch. 0 control
	//map(0x8224, 0x8225) high-speed DMA ch. 0 low-order source address set-up
	//map(0x8226, 0x8227) high-speed DMA ch. 0 high-order source address set-up
	//map(0x8228, 0x8229) high-speed DMA ch. 0 low-order destination address set-up
	//map(0x822a, 0x822b) high-speed DMA ch. 0 high-order destination address set-up
	//map(0x822c, 0x822d) high-speed DMA ch. 0 enable
	//map(0x822e, 0x822f) high-speed DMA ch. 0 trigger flag
	//map(0x8230, 0x8231) high-speed DMA ch. 1 transfer counter
	//map(0x8232, 0x8233) high-speed DMA ch. 1 control
	//map(0x8234, 0x8235) high-speed DMA ch. 1 low-order source address set-up
	//map(0x8236, 0x8237) high-speed DMA ch. 1 high-order source address set-up
	//map(0x8238, 0x8239) high-speed DMA ch. 1 low-order destination address set-up
	//map(0x823a, 0x823b) high-speed DMA ch. 1 high-order destination address set-up
	//map(0x823c, 0x823d) high-speed DMA ch. 1 enable
	//map(0x823e, 0x823f) high-speed DMA ch. 1 trigger flag
	//map(0x8240, 0x8241) high-speed DMA ch. 2 transfer counter
	//map(0x8242, 0x8243) high-speed DMA ch. 2 control
	//map(0x8244, 0x8245) high-speed DMA ch. 2 low-order source address set-up
	//map(0x8246, 0x8247) high-speed DMA ch. 2 high-order source address set-up
	//map(0x8248, 0x8249) high-speed DMA ch. 2 low-order destination address set-up
	//map(0x824a, 0x824b) high-speed DMA ch. 2 high-order destination address set-up
	//map(0x824c, 0x824d) high-speed DMA ch. 2 enable
	//map(0x824e, 0x824f) high-speed DMA ch. 2 trigger flag
	//map(0x8250, 0x8251) high-speed DMA ch. 3 transfer counter
	//map(0x8252, 0x8253) high-speed DMA ch. 3 control
	//map(0x8254, 0x8255) high-speed DMA ch. 3 low-order source address set-up
	//map(0x8256, 0x8257) high-speed DMA ch. 3 high-order source address set-up
	//map(0x8258, 0x8259) high-speed DMA ch. 3 low-order destination address set-up
	//map(0x825a, 0x825b) high-speed DMA ch. 3 high-order destination address set-up
	//map(0x825c, 0x825d) high-speed DMA ch. 3 enable
	//map(0x825e, 0x825f) high-speed DMA ch. 3 trigger flag
}
