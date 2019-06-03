// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DPB-7001 (c) 1981 Quantel

    Skeleton Driver

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "video/mc6845.h"
#include "screen.h"

class dpb7000_state : public driver_device
{
public:
	dpb7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "fd%u", 0U)
		, m_p_int(*this, "p_int")
		, m_brg(*this, "brg")
		, m_rs232(*this, "rs232")
		, m_crtc(*this, "crtc")
		, m_baud_dip(*this, "BAUD")
	{
	}

	void dpb7000(machine_config &config);

private:
	void main_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(bus_error_r);
	DECLARE_WRITE16_MEMBER(bus_error_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr_changed);

	required_device<m68000_base_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<input_merger_device> m_p_int;
	required_device<com8116_device> m_brg;
	required_device<rs232_port_device> m_rs232;
	required_device<sy6545_1_device> m_crtc;
	required_ioport m_baud_dip;
};

void dpb7000_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("monitor", 0);
	map(0xb00000, 0xb7ffff).rw(FUNC(dpb7000_state::bus_error_r), FUNC(dpb7000_state::bus_error_w));
	map(0xb80000, 0xbfffff).ram(); // NB: There's probably more RAM than this.
	map(0xfff801, 0xfff801).rw(m_crtc, FUNC(sy6545_1_device::status_r), FUNC(sy6545_1_device::address_w)).cswidth(16);
	map(0xfff803, 0xfff803).rw(m_crtc, FUNC(sy6545_1_device::register_r), FUNC(sy6545_1_device::register_w)).cswidth(16);
	map(0xfff805, 0xfff805).rw(m_acia[0], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff807, 0xfff807).rw(m_acia[0], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
	map(0xfff811, 0xfff811).rw(m_acia[1], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff813, 0xfff813).rw(m_acia[1], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
	map(0xfff815, 0xfff815).rw(m_acia[2], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff817, 0xfff817).rw(m_acia[2], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
}

static INPUT_PORTS_START( dpb7000 )
	PORT_START("BAUD")
	PORT_DIPNAME( 0x0f, 0x0e, "Baud Rate for Terminal")
	PORT_DIPSETTING(    0x00, "50")
	PORT_DIPSETTING(    0x01, "75")
	PORT_DIPSETTING(    0x02, "110")
	PORT_DIPSETTING(    0x03, "134.5")
	PORT_DIPSETTING(    0x04, "150")
	PORT_DIPSETTING(    0x05, "300")
	PORT_DIPSETTING(    0x06, "600")
	PORT_DIPSETTING(    0x07, "1200")
	PORT_DIPSETTING(    0x08, "1800")
	PORT_DIPSETTING(    0x09, "2000")
	PORT_DIPSETTING(    0x0a, "2400")
	PORT_DIPSETTING(    0x0b, "3600")
	PORT_DIPSETTING(    0x0c, "4800")
	PORT_DIPSETTING(    0x0e, "9600")
	PORT_DIPSETTING(    0x0f, "19200")
INPUT_PORTS_END

void dpb7000_state::machine_start()
{
}

void dpb7000_state::machine_reset()
{
	m_brg->stt_w(m_baud_dip->read());
}

MC6845_UPDATE_ROW(dpb7000_state::crtc_update_row)
{
}

MC6845_ON_UPDATE_ADDR_CHANGED(dpb7000_state::crtc_addr_changed)
{
}

READ16_MEMBER(dpb7000_state::bus_error_r)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

WRITE16_MEMBER(dpb7000_state::bus_error_w)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

void dpb7000_state::dpb7000(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dpb7000_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, m_p_int).output_handler().set_inputline(m_maincpu, 3);

	ACIA6850(config, m_acia[0], 0);
	m_acia[0]->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia[0]->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia[0]->irq_handler().set_inputline(m_maincpu, 6);

	ACIA6850(config, m_acia[1], 0);
	m_acia[1]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[2], 0);
	m_acia[2]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	m_rs232->dcd_handler().set(m_acia[0], FUNC(acia6850_device::write_dcd));
	m_rs232->cts_handler().set(m_acia[0], FUNC(acia6850_device::write_cts));

	COM8116(config, m_brg, 5.0688_MHz_XTAL);   // K1355A/B
	m_brg->ft_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	m_brg->ft_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));
	m_brg->ft_handler().append(m_acia[2], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[2], FUNC(acia6850_device::write_rxc));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479); // Not accurate
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	SY6545_1(config, m_crtc, 22.248_MHz_XTAL / 8);
	m_crtc->set_char_width(9);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen("screen");
	m_crtc->set_update_row_callback(FUNC(dpb7000_state::crtc_update_row), this);
	m_crtc->set_on_update_addr_change_callback(FUNC(dpb7000_state::crtc_addr_changed), this);
}


ROM_START( dpb7000 )
	ROM_REGION16_BE(0x80000, "monitor", 0)
	ROM_LOAD16_BYTE("01616a-nad-4af2.bin", 0x00001, 0x8000, CRC(a42eace6) SHA1(78c629a8afb48a95fc0a86ca762cc5b84bd9929b))
	ROM_LOAD16_BYTE("01616a-ncd-58de.bin", 0x00000, 0x8000, CRC(f70fff2a) SHA1(a6f85d086a0c53d156eeeb157184ebcad4adecb3))
	ROM_LOAD16_BYTE("01616a-mad-f512.bin", 0x10001, 0x8000, CRC(4c3e39f6) SHA1(443095c56481fbcadd4dcec1757d889c8f78805d))
	ROM_LOAD16_BYTE("01616a-mcd-91f2.bin", 0x10000, 0x8000, CRC(4b6b6eb3) SHA1(1bef443d78197d33e44c708ead9604020881f67f))
	ROM_LOAD16_BYTE("01616a-lad-0059.bin", 0x20001, 0x8000, CRC(0daf670d) SHA1(2342a43054ed141de298a1c1a6867949297bb52a))
	ROM_LOAD16_BYTE("01616a-lcd-5639.bin", 0x20000, 0x8000, CRC(c8977d3f) SHA1(4ee9f3a883400b4771e6ae33c6e4edcd5c0b49e7))
	ROM_LOAD16_BYTE("01616a-kad-1d9b.bin", 0x30001, 0x8000, CRC(bda7e309) SHA1(377edf2675a6736fe7ec775894858967b0e9247e))
	ROM_LOAD16_BYTE("01616a-kcd-e51c.bin", 0x30000, 0x8000, CRC(aa05a5cc) SHA1(85dce335a72643f7640524b18cfe480a3c299f23))
	ROM_LOAD16_BYTE("01616a-jad-47a8.bin", 0x40001, 0x8000, CRC(60fff4c9) SHA1(ba60281c0dd8627dffe07e7ea66f4eb688e74001))
	ROM_LOAD16_BYTE("01616a-jcd-7825.bin", 0x40000, 0x8000, CRC(bb258ede) SHA1(ab8042391cd361bcd874b2f9d8fcaf20d4b2ebe7))
	ROM_LOAD16_BYTE("01616a-iad-73a8.bin", 0x50001, 0x8000, CRC(98709fd2) SHA1(bd0f4689600e9fc49dbd8f2f326e18f8d602825e))
	ROM_LOAD16_BYTE("01616a-icd-0562.bin", 0x50000, 0x8000, CRC(bab5274e) SHA1(3e51977da3dfe8fda089b9d2c3199acb4fed3212))
	ROM_LOAD16_BYTE("01616a-had-d6fa.bin", 0x60001, 0x8000, CRC(70d791c5) SHA1(c281e4f27404e58ad5a80d6de1c5583cd9f3fe0e))
	ROM_LOAD16_BYTE("01616a-hcd-9c0e.bin", 0x60000, 0x8000, CRC(938cb614) SHA1(ea7ea8a13e0ab1497691bab53090296ba51d271f))
	ROM_LOAD16_BYTE("01616a-gcd-3ab8.bin", 0x70001, 0x8000, CRC(e9c21438) SHA1(1784ab2de1bb6023565b2e27872a0fcda25e1b1f))
	ROM_LOAD16_BYTE("01616a-gad-397d.bin", 0x70000, 0x8000, CRC(0b95f9ed) SHA1(77126ee6c1f3dcdb8aa669ab74ff112e3f01918a))
ROM_END

COMP( 1981, dpb7000, 0, 0, dpb7000, dpb7000, dpb7000_state, empty_init, "Quantel", "DPB-7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
