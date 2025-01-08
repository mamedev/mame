// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/***************************************************************************

Vector Graphic Vector 4. Vector Graphic, Inc was a Californian company that
made a personal computers before the IBM PC disrupted the industry. The
Vector 4 was an office computer with word processing, spreadsheeting,
database, and eventually networked drives.

The ROM boots from the second floppy. "Winchester" boots from the hard drive
(currently unsupported), which is the first device when in use.

On power on the system uses a Z80B, but it also contains an 8088-2 for limited
IBM compatibility. Only one processor is running at a time and reading from a
port can toggle the active processor. They share memory with each other and the
video subsystem. The active processor and the video subsystem alternate access to
the RAM. The BDOS for CP/M makes use of the 8088, so both processors are
necessary for any of the OSes to boot: CP/M, CP/M 86, MS-DOS.

The system had three S-100 slots, with the floppy controller using one. It was a
modified bus since it provided regulated power. The system had 128K RAM and
could be extended to 256K. The keyboard plugged in to a 6P6C jack. There were
female ports for two DB-25 RS-232, a 36-pin micro ribbon (aka, telco,
centronics) parallel, a 50-pin micro ribbon parallel, and a DE-9 RGBI/CGA
monitor.

The 7100-0001 User's Manual provides some overview specifications starting on
page 171 (X A-1) including ports and devices. But it provides few details. The
7200-0001 Technical Information provides thorough descriptions and schematics.

"Executive" is the name of the boot ROM, not to be confused with a model name.
The motherboard is called the Single Board Computer (SBC) and includes all
functionality except for the disk controller.

https://archive.org/details/7200-0001-vector-4-technical-information-sep-82
https://www.bitsavers.org/pdf/vectorGraphic/vector_4/7100-0001_Vector_4_Users_Manual_Feb83.pdf

TODO:
- S-100 interrupts and ready
- Qume (50 pin) parallel port
- WAIT CPU states
- CPM-86 and MS-DOS 2.0 don't boot

****************************************************************************/

#include "emu.h"

#include "sbcvideo.h"
#include "v4_kbd.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/s100/s100.h"
#include "bus/s100/vectordualmode.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/pit8253.h"
#include "sound/sn76496.h"

#include "speaker.h"

namespace {

class vector4_state : public driver_device
{
public:
	vector4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_8088cpu(*this, "8088cpu")
		, m_rambanks(*this, "rambank%u", 0U)
		, m_256k(false)
		, m_ram(*this, RAM_TAG)
		, m_romenbl(*this, "romenbl")
		, m_sbc_video(*this, "video")
		, m_s100(*this, "s100")
		, m_uart0(*this, "uart0")
		, m_centronics(*this, "centprtr")
		, m_ppi_pc(0)
	{ }

	void vector4(machine_config &config);

private:
	void vector4_io(address_map &map) ATTR_COLD;
	void vector4_z80mem(address_map &map) ATTR_COLD;
	void vector4_8088mem(address_map &map) ATTR_COLD;
	void spr_w(uint8_t data);
	uint8_t msc_r();
	void msc_w(uint8_t data) { machine_reset(); }
	void addrmap_w(offs_t offset, uint8_t data);
	void ppi_pb_w(uint8_t data) { m_centronics->write_strobe(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE); }
	void centronics_busy_w(int state) { if (state) m_ppi_pc |= 1; else m_ppi_pc &= ~1; }
	int ppi_pc_r() { return m_ppi_pc; }
	uint8_t s100_r(offs_t offset) { return m_s100->sinp_r(offset+0x20); }
	void s100_w(offs_t offset, uint8_t data) { m_s100->sout_w(offset+0x20, data); }
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_8088cpu;
	required_memory_bank_array<32> m_rambanks;
	bool m_256k;
	required_device<ram_device> m_ram;
	memory_view m_romenbl;
	required_device<vector_sbc_video_device> m_sbc_video;
	required_device<s100_bus_device> m_s100;
	required_device<i8251_device> m_uart0;
	required_device<centronics_device> m_centronics;
	uint8_t m_ppi_pc;
};


void vector4_state::vector4_z80mem(address_map &map)
{
	map.unmap_value_high();
	for (int bank = 0; bank < (1<<5); bank++)
		map(bank << 11, ((bank+1) << 11)-1).bankrw(m_rambanks[bank]);
	// 7200-0001 page 209 (VI A-6) B6
	// ROM mapping only applies to the z80 and does not use address mapping.
	map(0x0000, 0x0fff).view(m_romenbl);
	m_romenbl[0](0x0000, 0x0fff).rom().region("maincpu", 0);
}

void vector4_state::vector4_8088mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3ffff);
}

void vector4_state::vector4_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).mirror(0xff00).rw(m_uart0, FUNC(i8251_device::read), FUNC(i8251_device::write)); // keyboard
	map(0x02, 0x03).mirror(0xff00).w(FUNC(vector4_state::spr_w)); // subsystem port register
	map(0x04, 0x05).mirror(0xff00).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write)); // modem
	map(0x06, 0x07).mirror(0xff00).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write)); // serial printer
	map(0x08, 0x0b).mirror(0xff00).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)); // parallel printer
	map(0x0c, 0x0d).mirror(0xff00).rw(FUNC(vector4_state::msc_r), FUNC(vector4_state::msc_w)); // select Z80/8088-2
	map(0x0e, 0x0e).mirror(0xff00).rw("video:crtc", FUNC(c6545_1_device::status_r), FUNC(c6545_1_device::address_w)); // video controller
	map(0x0f, 0x0f).mirror(0xff00).rw("video:crtc", FUNC(c6545_1_device::register_r), FUNC(c6545_1_device::register_w));
	map(0x10, 0x13).mirror(0xff00).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // baud generator and timer
	map(0x16, 0x17).select(0xff00).w(FUNC(vector4_state::addrmap_w)); // RAM address map
	map(0x18, 0x19).mirror(0xff00).w("sn", FUNC(sn76489_device::write)); // tone generator
	map(0x1c, 0x1f).mirror(0xff00).w(m_sbc_video, FUNC(vector_sbc_video_device::res320_mapping_ram_w)); // resolution 320 mapping RAM
	map(0x20, 0xff).mirror(0xff00).rw(FUNC(vector4_state::s100_r), FUNC(vector4_state::s100_w));
}

static void vector4_s100_devices(device_slot_interface &device)
{
	device.option_add("dualmodedisk", S100_VECTOR_DUALMODE);
}

static INPUT_PORTS_START( vector4 )
INPUT_PORTS_END

void vector4_state::vector4(machine_config &config)
{
	const XTAL _32m(32'640'000);
	const XTAL _2m = _32m/16;

	/* processors */
	// Manual says 5.1 MHz. To do so, schematic shows (A1) it is driven by the
	// 32.64 MHz xtal, 1/16 of the cycles are skipped, and it is divided by 6.
	Z80(config, m_maincpu, 5'100'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vector4_state::vector4_z80mem);
	m_maincpu->set_addrmap(AS_IO, &vector4_state::vector4_io);

	I8088(config, m_8088cpu, 5'100'000);
	m_8088cpu->set_addrmap(AS_PROGRAM, &vector4_state::vector4_8088mem);
	m_8088cpu->set_addrmap(AS_IO, &vector4_state::vector4_io);

	RAM(config, m_ram).set_default_size("128K").set_extra_options("256K");

	/* video hardware */
	SBC_VIDEO(config, m_sbc_video, _32m);
	m_sbc_video->set_buffer(m_ram);
	m_sbc_video->set_chrroml("chargenl");
	m_sbc_video->set_chrromr("chargenr");

	/* i/o */
	XTAL _2mclk(2'000'000); // 7200-0001 page 210 (VI A-7) D13

	S100_BUS(config, m_s100, _2mclk);
	S100_SLOT(config, "s100:1", vector4_s100_devices, "dualmodedisk");
	S100_SLOT(config, "s100:2", vector4_s100_devices, nullptr);
	S100_SLOT(config, "s100:3", vector4_s100_devices, nullptr);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	// 7200-0001 page 210 D7, 208 (VI A-5) A1
	pit.set_clk<0>(_2mclk);
	pit.set_clk<1>(_2mclk);
	pit.set_clk<2>(_2mclk);
	// 7200-0001 page 107 (II 5-16)
	pit.out_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pit.out_handler<0>().append_inputline(m_8088cpu, INPUT_LINE_IRQ0);

	// 7200-0001 page 210 D13, D1
	vector4_keyboard_device &v4kbd(VECTOR4_KEYBOARD(config, "rs232keyboard", 0));
	v4kbd.txd_handler().set(m_uart0, FUNC(i8251_device::write_rxd));
	clock_device &keyboard_clock(CLOCK(config, "keyboard_clock", _2mclk/26/16));
	keyboard_clock.signal_handler().set(m_uart0, FUNC(i8251_device::write_txc));
	keyboard_clock.signal_handler().append(m_uart0, FUNC(i8251_device::write_rxc));
	I8251(config, m_uart0, 0);
	m_uart0->txd_handler().set(v4kbd, FUNC(vector4_keyboard_device::write_rxd));

	// D3
	i8251_device &uart1(I8251(config, "uart1", 0));
	rs232_port_device &rs232com(RS232_PORT(config, "rs232com", default_rs232_devices, nullptr));
	pit.out_handler<1>().set(uart1, FUNC(i8251_device::write_txc));
	pit.out_handler<1>().append(uart1, FUNC(i8251_device::write_rxc));
	uart1.txd_handler().set(rs232com, FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set(rs232com, FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set(rs232com, FUNC(rs232_port_device::write_rts));
	rs232com.rxd_handler().set(uart1, FUNC(i8251_device::write_rxd));
	rs232com.dsr_handler().set(uart1, FUNC(i8251_device::write_dsr));
	rs232com.cts_handler().set(uart1, FUNC(i8251_device::write_cts));

	// D2
	i8251_device &uart2(I8251(config, "uart2", 0));
	rs232_port_device &rs232prtr(RS232_PORT(config, "rs232prtr", default_rs232_devices, nullptr));
	pit.out_handler<2>().set(uart2, FUNC(i8251_device::write_txc));
	pit.out_handler<2>().append(uart2, FUNC(i8251_device::write_rxc));
	uart2.txd_handler().set(rs232prtr, FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set(rs232prtr, FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set(rs232prtr, FUNC(rs232_port_device::write_rts));
	rs232prtr.rxd_handler().set(uart2, FUNC(i8251_device::write_rxd));
	rs232prtr.dsr_handler().set(uart2, FUNC(i8251_device::write_dsr));
	rs232prtr.cts_handler().set(uart2, FUNC(i8251_device::write_cts));

	// 7200-0001 page 110 (II 5-19), 210 D8
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->set_output_latch(cent_data_out);
	m_centronics->busy_handler().set(FUNC(vector4_state::centronics_busy_w));

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(cent_data_out, FUNC(output_latch_device::write));
	ppi.out_pb_callback().set(FUNC(vector4_state::ppi_pb_w));
	ppi.in_pc_callback().set(FUNC(vector4_state::ppi_pc_r));

	SPEAKER(config, "mono").front_center();
	sn76489_device &sn(SN76489(config, "sn", _2m));
	sn.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void vector4_state::machine_start()
{
	m_256k = (m_ram->size() == 256 * 1024);
	m_8088cpu->space(AS_PROGRAM).install_ram(0, m_ram->mask(), m_ram->size() & 0x20000, m_ram->pointer());
	for (int bank = 0; bank < (1<<5); bank++)
		m_rambanks[bank]->configure_entries(0, 1<<7, m_ram->pointer(), 1<<11);

	save_item(NAME(m_ppi_pc));

	// Missing from schematic, but jumper wire present on the board.
	m_uart0->write_cts(0);
}

void vector4_state::machine_reset()
{
	// 7200-0001 page 39 (II 1-10), page 210 D11
	spr_w(0);
	m_8088cpu->suspend(SUSPEND_REASON_HALT, true);
	m_maincpu->resume(SUSPEND_REASON_HALT);
}

/* Subsystem Port Register */
void vector4_state::spr_w(uint8_t data)
{
	// 7200-0001 page 81 (II 3-18)
	if (BIT(data, 0))
		m_romenbl.disable();
	else
		m_romenbl.select(0);
	m_sbc_video->spr_w(data);
}

/* Microprocessor Switching Control */
uint8_t vector4_state::msc_r()
{
	// 7200-0001 page 40 (II 1-11), 208 A3
	if (m_maincpu->suspended(SUSPEND_REASON_HALT)) {
		m_8088cpu->suspend(SUSPEND_REASON_HALT, false);
		m_maincpu->resume(SUSPEND_REASON_HALT);
	} else {
		m_maincpu->suspend(SUSPEND_REASON_HALT, false);
		m_8088cpu->resume(SUSPEND_REASON_HALT);
	}
	return 0xff;
}

/* Address mapping RAM Subsystem */
void vector4_state::addrmap_w(offs_t offset, uint8_t data)
{
	// 7200-0001 page 50 (II 2-7), page 208 B1
	// m_256k is for jumper area B. 7200-0001 page 170 (IV 2-1), page 209 coord A8
	m_rambanks[BIT(offset, 11, 5)]->set_entry(data & (m_256k ? 0x7f : 0x3f));
}

/* ROM definition */
ROM_START( vector4 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v11", "SBC ver 1.1" ) // VECTOR 4 SBC EXECUTIVE 1.1
	ROMX_LOAD( "sbcv11.bin", 0x0000, 0x1000, CRC(56c80656) SHA1(a381bdbb6cdee0ac1dc8b1c1359361a19ba6fe46), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v111", "SBC ver 1.11" ) // VECTOR 4 SBC EXECUTIVE 1.11
	ROMX_LOAD( "u35_sbc_v111_4f11.bin", 0x0000, 0x1000, CRC(fcfd42c6) SHA1(11cd5cf0c3f2d2a30864b8a4b4be51ade03d02ea), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v200", "SBC ver 2.00" ) // VECTOR GRAPHIC V4/40 [F] SBC EXECUTIVE - REVISION 2.00 (AA)
	ROMX_LOAD( "sbcexecf.bin", 0x0000, 0x1000, CRC(738e10b5) SHA1(abce22abe9bac6241b1996d078647fe36b638769), ROM_BIOS(2))

	ROM_REGION( 0x1000, "chargenl", ROMREGION_ERASEFF )
	ROM_LOAD( "cgst60l.bin", 0x0000, 0x1000, CRC(e55a404c) SHA1(4dafd6f588c42081212928d3c4448f10dd461a7a))
	ROM_REGION( 0x1000, "chargenr", ROMREGION_ERASEFF )
	ROM_LOAD( "cgst60r.bin", 0x0000, 0x1000, CRC(201d783c) SHA1(7c2b8988c27b5fa435d31c9b7d4d9ed176c9b8a3))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY            FULLNAME    FLAGS
COMP( 1982, vector4, 0,      0,      vector4, vector4, vector4_state, empty_init,   "Vector Graphic", "Vector 4", MACHINE_NODEVICE_PRINTER | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )
