// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for ADM 36 Video Display Terminal.

    The detachable keyboard has not been dumped. It is controlled serially through the PIO.

************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/er1400.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/tms9927.h"
#include "screen.h"


namespace {

class adm36_state : public driver_device
{
public:
	adm36_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_vtac(*this, "vtac")
		, m_chargen(*this, "chargen")
		, m_vram(*this, "vram%u", 0U)
	{
	}

	void adm36(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void pio_pa_w(u8 data);
	u8 pio_pb_r();
	void pio_pb_w(u8 data);
	void vsyn_w(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<crt5037_device> m_vtac;
	required_region_ptr<u8> m_chargen;
	required_shared_ptr_array<u8, 2> m_vram;

	bool m_vsyn = false;
};


u32 adm36_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void adm36_state::machine_start()
{
	m_vsyn = false;

	save_item(NAME(m_vsyn));
}

void adm36_state::machine_reset()
{
}

void adm36_state::pio_pa_w(u8 data)
{
	m_earom->data_w(BIT(data, 0));
}

u8 adm36_state::pio_pb_r()
{
	return (m_earom->data_r() ? 0 : 1) | (m_vsyn ? 0x20 : 0);
}

void adm36_state::pio_pb_w(u8 data)
{
	m_earom->clock_w(BIT(data, 4));
	m_earom->c3_w(BIT(data, 3));
	m_earom->c2_w(BIT(data, 2));
	m_earom->c1_w(BIT(data, 1));
}

void adm36_state::vsyn_w(int state)
{
	m_vsyn = bool(state);
}

void adm36_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x6000, 0x67ff).ram(); // MK4802J-3 (U76)
	map(0xe000, 0xefff).ram().share("vram0"); // 4x MK4118AN-1 (U66-U69)
	map(0xf000, 0xffff).ram().share("vram1"); // 4x MK4118AN-1 (U81-U84)
}

void adm36_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x0c).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).mirror(0x0c).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x20, 0x23).mirror(0x0c).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x40, 0x4f).rw(m_vtac, FUNC(crt5037_device::read), FUNC(crt5037_device::write));
}


static INPUT_PORTS_START(adm36)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ nullptr }
};

void adm36_state::adm36(machine_config &config)
{
	Z80(config, m_maincpu, 14.728_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &adm36_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &adm36_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80pio_device &pio(Z80PIO(config, "pio", 14.728_MHz_XTAL / 6));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio.out_pa_callback().set(FUNC(adm36_state::pio_pa_w));
	pio.in_pb_callback().set(FUNC(adm36_state::pio_pb_r));
	pio.out_pb_callback().set(FUNC(adm36_state::pio_pb_w));

	ER1400(config, m_earom);

	//F3870(config, "keybcpu", 3.579545_MHz_XTAL);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 14.728_MHz_XTAL / 6));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(14.728_MHz_XTAL / 12);
	ctc.set_clk<1>(14.728_MHz_XTAL / 12);
	ctc.set_clk<2>(14.728_MHz_XTAL / 12);
	ctc.set_clk<3>(14.728_MHz_XTAL / 12);
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<1>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<2>().set("sio", FUNC(z80sio_device::txcb_w));
	ctc.zc_callback<2>().append("sio", FUNC(z80sio_device::rxcb_w));

	z80sio_device &sio(Z80SIO(config, "sio", 14.728_MHz_XTAL / 6)); // MK3887N
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("modem", FUNC(rs232_port_device::write_txd));
	sio.out_rtsa_callback().set("modem", FUNC(rs232_port_device::write_rts));
	sio.out_dtra_callback().set("modem", FUNC(rs232_port_device::write_dtr));
	sio.out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("printer", FUNC(rs232_port_device::write_rts));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.728_MHz_XTAL, 936, 0, 720, 262, 0, 240);
	//screen.set_raw(24.3_MHz_XTAL, 1548, 0, 1188, 262, 0, 240);
	screen.set_screen_update(FUNC(adm36_state::screen_update));

	CRT5037(config, m_vtac, 14.728_MHz_XTAL / 9);
	m_vtac->set_char_width(9);
	m_vtac->set_screen("screen");
	m_vtac->vsyn_callback().set(FUNC(adm36_state::vsyn_w));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr)); // RS-232C, RS-422A or 20mA current loop
	modem.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	modem.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	modem.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
	modem.dsr_handler().set("pio", FUNC(z80pio_device::pb6_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	printer.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	printer.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
}


ROM_START(adm36)
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("u71_136261-012.bin", 0x0000, 0x2000, CRC(f08315c7) SHA1(3943a5fc587e690df81aa9694e6e452673ec5513))
	ROM_LOAD("u72_131671-015.bin", 0x2000, 0x1000, CRC(c397f4e2) SHA1(21513472fe4237bda8448a2ad85496757d4ece12))
	// U73-U75 are empty sockets

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("u56.bin", 0x0000, 0x1000, CRC(70e46897) SHA1(85b4360912fc05243b3b2df29bde5a3def94086b))
ROM_END

} // anonymous namespace


COMP(1981, adm36, 0, 0, adm36, adm36, adm36_state, empty_init, "Lear Siegler", "ADM 36 Video Display Terminal", MACHINE_IS_SKELETON)
