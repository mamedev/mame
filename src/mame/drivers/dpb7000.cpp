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
#include "emupal.h"
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
		, m_palette(*this, "palette")
		, m_vdu_ram(*this, "vduram")
		, m_vdu_char_rom(*this, "vduchar")
		, m_baud_dip(*this, "BAUD")
		, m_auto_start(*this, "AUTOSTART")
		, m_config_sw12(*this, "CONFIGSW12")
		, m_config_sw34(*this, "CONFIGSW34")
	{
	}

	void dpb7000(machine_config &config);

private:
	void main_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(bus_error_r);
	DECLARE_WRITE16_MEMBER(bus_error_w);

	DECLARE_WRITE8_MEMBER(csr_w);

	DECLARE_READ16_MEMBER(cpu_ctrlbus_r);
	DECLARE_WRITE16_MEMBER(cpu_ctrlbus_w);

	enum : uint16_t
	{
		SYSCTRL_AUTO_START		= 0x0001,
		SYSCTRL_REQ_B_OUT		= 0x0020,
		SYSCTRL_REQ_A_OUT		= 0x0040,
		SYSCTRL_REQ_A_IN		= 0x0080,
		SYSCTRL_REQ_B_IN		= 0x8000
	};

	DECLARE_READ16_MEMBER(cpu_sysctrl_r);
	DECLARE_WRITE16_MEMBER(cpu_sysctrl_w);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr_changed);

	required_device<m68000_base_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<input_merger_device> m_p_int;
	required_device<com8116_device> m_brg;
	required_device<rs232_port_device> m_rs232;
	required_device<sy6545_1_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_vdu_ram;
	required_memory_region m_vdu_char_rom;
	required_ioport m_baud_dip;
	required_ioport m_auto_start;
	required_ioport m_config_sw12;
	required_ioport m_config_sw34;

	uint8_t m_csr;
	uint16_t m_sys_ctrl;
};

void dpb7000_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("monitor", 0);
	map(0xb00000, 0xb7ffff).rw(FUNC(dpb7000_state::bus_error_r), FUNC(dpb7000_state::bus_error_w));
	map(0xb80000, 0xbfffff).ram();
	map(0xffe000, 0xffefff).ram().share("vduram").umask16(0x00ff);
	map(0xfff801, 0xfff801).rw(m_crtc, FUNC(sy6545_1_device::status_r), FUNC(sy6545_1_device::address_w)).cswidth(16);
	map(0xfff803, 0xfff803).rw(m_crtc, FUNC(sy6545_1_device::register_r), FUNC(sy6545_1_device::register_w)).cswidth(16);
	map(0xfff805, 0xfff805).rw(m_acia[0], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff807, 0xfff807).rw(m_acia[0], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
	map(0xfff809, 0xfff809).w(FUNC(dpb7000_state::csr_w)).cswidth(16);
	map(0xfff80a, 0xfff80b).rw(FUNC(dpb7000_state::cpu_ctrlbus_r), FUNC(dpb7000_state::cpu_ctrlbus_w));
	map(0xfff80c, 0xfff80d).rw(FUNC(dpb7000_state::cpu_sysctrl_r), FUNC(dpb7000_state::cpu_sysctrl_w));
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

	PORT_START("AUTOSTART")
	PORT_DIPNAME( 0x0001, 0x0000, "Auto-Start" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONFIGSW12")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("CONFIGSW34")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void dpb7000_state::machine_start()
{
	save_item(NAME(m_csr));
	save_item(NAME(m_sys_ctrl));
}

void dpb7000_state::machine_reset()
{
	m_brg->stt_w(m_baud_dip->read());
	m_csr = 0;
}

MC6845_UPDATE_ROW(dpb7000_state::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();
	const uint8_t *char_rom = m_vdu_char_rom->base();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = (uint8_t)m_vdu_ram[((ma + column) & 0x7ff)];
		uint16_t addr = code << 4 | (ra & 0x0f);
		uint8_t data = char_rom[addr & 0xfff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(data, 7) && de;

			bitmap.pix32(vbp + y, hbp + x) = pen[color];

			data <<= 1;
		}
	}
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

WRITE8_MEMBER(dpb7000_state::csr_w)
{
	logerror("%s: Card Select write: %02x\n", machine().describe_context(), data & 0x0f);
	m_csr = data & 0x0f;
}

READ16_MEMBER(dpb7000_state::cpu_ctrlbus_r)
{
	uint16_t ret = 0;
	switch (m_csr)
	{
		case 12:
			ret = m_config_sw34->read();
			logerror("%s: CPU read from Control Bus, Config Switches 1/2: %04x\n", machine().describe_context(), ret);
			break;
		case 14:
			ret = m_config_sw12->read();
			logerror("%s: CPU read from Control Bus, Config Switches 3/4: %04x\n", machine().describe_context(), ret);
			break;
		default:
			logerror("%s: CPU read from Control Bus, unknown CSR %d\n", machine().describe_context(), m_csr);
			break;
	}
	return ret;
}

WRITE16_MEMBER(dpb7000_state::cpu_ctrlbus_w)
{
	logerror("%s: CPU to Control Bus write: %04x\n", machine().describe_context(), data);
}

READ16_MEMBER(dpb7000_state::cpu_sysctrl_r)
{
	const uint16_t ctrl = m_sys_ctrl &~ SYSCTRL_AUTO_START;
	const uint16_t auto_start = m_auto_start->read() ? SYSCTRL_AUTO_START : 0;
	const uint16_t ret = ctrl | auto_start;
	logerror("%s: CPU read from System Control: %04x\n", machine().describe_context(), ret);
	return ret;
}

WRITE16_MEMBER(dpb7000_state::cpu_sysctrl_w)
{
	const uint16_t mask = (SYSCTRL_REQ_A_OUT | SYSCTRL_REQ_B_OUT);
	logerror("%s: CPU to Control Bus write: %04x\n", machine().describe_context(), data);
	m_sys_ctrl &= ~mask;
	m_sys_ctrl |= (data & mask);
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

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
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

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// The 6545's clock is driven by the QD output of a 4-bit binary counter, which has its preset wired to 0010.
	// It therefore operates as a divide-by-six counter for the master clock of 22.248MHz.
	SY6545_1(config, m_crtc, 22.248_MHz_XTAL / 6);
	m_crtc->set_char_width(8);
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

	ROM_REGION(0x1000, "vduchar", 0)
	ROM_LOAD( "bw14char.ic1",  0x0000, 0x1000, BAD_DUMP CRC(f9dd68b5) SHA1(50132b759a6d84c22c387c39c0f57535cd380411) )
ROM_END

COMP( 1981, dpb7000, 0, 0, dpb7000, dpb7000, dpb7000_state, empty_init, "Quantel", "DPB-7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
