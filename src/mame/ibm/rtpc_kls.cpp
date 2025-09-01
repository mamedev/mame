// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM RT PC keyboard, locator and speaker adapter (KLS) device.
 */

#include "emu.h"
#include "rtpc_kls.h"

#include "rtpc_kbd.h"
#include "rtpc_mouse.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

enum kls_p1_mask : u8
{
	KLS_P10 = 0x01, // cmd08 (i)
	KLS_P11 = 0x02, // cmd09 (i)
	KLS_P12 = 0x04, // cmd10 (i)
	KLS_P13 = 0x08, // cmd11 (i)
	KLS_P14 = 0x10, // cmd12 (i)
	KLS_P15 = 0x20, // keylock switch (i)
	KLS_P16 = 0x40, // speaker volume 0 (o)
	KLS_P17 = 0x80, // speaker volume 1 (o)

	KLS_CMD = 0x1f,
	KLS_P1O = 0xc0, // port 1 outputs
};
enum kls_p2_mask : u8
{
	KLS_P20 = 0x01, // iid0 (o)
	KLS_P21 = 0x02, // iid1 (o)
	KLS_P22 = 0x04, // iid2 (o)
	KLS_P23 = 0x08, // i/o channel system reset, active low (o)
	KLS_P24 = 0x10, // ppi -stb (o)
	KLS_P25 = 0x20, // ppi ibf (i)
	KLS_P26 = 0x40, // ppi -ack (o)
	KLS_P27 = 0x80, // speaker frequency (o)

	KLS_IID = 0x07,
	KLS_P2O = 0xdf, // port 2 outputs
};
enum kls_p3_mask : u8
{
	KLS_P30 = 0x01, // uart rx (i)
	KLS_P31 = 0x02, // uart tx (o)
	KLS_P32 = 0x04, // kbd clock/-int0 (i)
	KLS_P33 = 0x08, // ppi -obf/-int1 (i)
	KLS_P34 = 0x10, // 32kHz/t0 (i)
	KLS_P35 = 0x20, // kbd data (i)
	KLS_P36 = 0x40, // kbd data (o)
	KLS_P37 = 0x80, // kbd clock (o)

	KLS_P3O = 0xc2, // port 3 outputs
};

DEFINE_DEVICE_TYPE(RTPC_KLS, rtpc_kls_device, "rtpc_kls", "IBM RT PC Keyboard, Locator, Speaker Adapter")

rtpc_kls_device::rtpc_kls_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RTPC_KLS, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_ppi(*this, "ppi")
	, m_kbdc(*this, "kbdc")
	, m_locc(*this, "locc")
	, m_rcv(*this, { "rcv_loop", "rcv_data" })
	, m_speaker(*this, "kbdc:kbd:speaker")
	, m_keylock(*this, "KEYLOCK")
	, m_atn(*this)
	, m_irq(*this)
{
}

void rtpc_kls_device::map(address_map &map)
{
	map(0x0, 0x1).w(FUNC(rtpc_kls_device::cmd_w)).flags(1);
	map(0x4, 0x7).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void kbd_devices(device_slot_interface &device)
{
	device.option_add("keyboard", RTPC_KBD);
}

void loc_devices(device_slot_interface &device)
{
	device.option_add("mouse", RTPC_MOUSE);
}

void rtpc_kls_device::device_add_mconfig(machine_config &config)
{
	// keyboard, locator, speaker adapter
	// P8051AH 2075
	// 61X6310 8811
	// (c)IBM 1986
	// (c)INTEL '80
	I8051(config, m_mcu, 9.216_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &rtpc_kls_device::mcu_map);
	m_mcu->port_in_cb<0>().set([this]() { return m_mcu_p[0]; });
	m_mcu->port_out_cb<0>().set([this](u8 data) { m_mcu_p[0] = data; });
	m_mcu->port_in_cb<1>().set([this]() { return m_mcu_p[1] | m_keylock->read(); });
	m_mcu->port_out_cb<1>().set(FUNC(rtpc_kls_device::mcu_port1_w));
	m_mcu->port_in_cb<2>().set([this]() { return m_mcu_p[2]; });
	m_mcu->port_out_cb<2>().set(FUNC(rtpc_kls_device::mcu_port2_w));
	m_mcu->port_in_cb<3>().set([this]() { return m_mcu_p[3]; });
	m_mcu->port_out_cb<3>().set(FUNC(rtpc_kls_device::mcu_port3_w));

	CLOCK(config, "t0", 32'768).signal_handler().set(
		[this](int state)
		{
			if (state)
				m_mcu_p[3] |= KLS_P34;
			else
				m_mcu_p[3] &= ~KLS_P34;

			m_mcu->set_input_line(MCS51_T0_LINE, state ? ASSERT_LINE : CLEAR_LINE);
		});

	input_merger_any_high_device &rcv(INPUT_MERGER_ANY_HIGH(config, "rcv"));
	rcv.output_handler().set(
		[this](int state)
		{
			if (state)
			{
				m_mcu_p[3] |= KLS_P30;
				m_ppi_p[1] |= 0x01;
			}
			else
			{
				m_mcu_p[3] &= ~KLS_P30;
				m_ppi_p[1] &= ~0x01;
			}
		});
	INPUT_MERGER_ALL_HIGH(config, m_rcv[0]).output_handler().set(rcv, FUNC(input_merger_any_high_device::in_w<0>));
	INPUT_MERGER_ALL_HIGH(config, m_rcv[1]).output_handler().set(rcv, FUNC(input_merger_any_high_device::in_w<1>));

	// P8255A-5
	// L6430434
	// (C)INTEL '81
	I8255A(config, m_ppi);
	// port A: read/write 8051
	// port B: input
	// port C lower: input
	// port C upper: 8051 handshake
	// port C & 0x20 -> irq
	m_ppi->out_pa_callback().set([this](u8 data) { m_ppi_p[0] = data; });
	m_ppi->in_pa_callback().set([this]() { return m_ppi_p[0]; });

	// TODO: bits 4-1 "non-adapter system board signals"
	m_ppi->in_pb_callback().set([this]() { return m_ppi_p[1]; });

	m_ppi->out_pc_callback().set(FUNC(rtpc_kls_device::ppi_portc_w));
	m_ppi->in_pc_callback().set([this]() { return m_ppi_p[2]; });

	RTPC_KBDC(config, m_kbdc, kbd_devices, "keyboard");
	m_kbdc->out_data_cb().set(
		[this](int state)
		{
			if (state)
				m_mcu_p[3] |= KLS_P35;
			else
				m_mcu_p[3] &= ~KLS_P35;
		});
	m_kbdc->out_clock_cb().set(
		[this](int state)
		{
			if (state)
				m_mcu_p[3] |= KLS_P32;
			else
				m_mcu_p[3] &= ~KLS_P32;

			m_mcu->set_input_line(MCS51_INT0_LINE, state ? CLEAR_LINE : ASSERT_LINE);
		});

	RS232_PORT(config, m_locc, loc_devices, "mouse");
	m_mcu->port_out_cb<3>().append(m_locc, FUNC(rs232_port_device::write_txd)).bit(1);
	m_locc->rxd_handler().set(m_rcv[1], FUNC(input_merger_all_high_device::in_w<1>));
}

void rtpc_kls_device::device_start()
{
	constexpr double const speaker_levels[4] = { 0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0 };

	save_item(NAME(m_cmd));
	save_item(NAME(m_mcu_p));
	save_item(NAME(m_ppi_p));

	m_cmd = 0;

	m_mcu_p[0] = 0xff;
	m_mcu_p[1] = 0xff & ~KLS_P15;
	m_mcu_p[2] = 0xff;
	m_mcu_p[3] = 0xff;

	m_ppi_p[0] = 0;
	m_ppi_p[1] = 0;
	m_ppi_p[2] = 0;

	if (m_speaker)
		m_speaker->set_levels(4, speaker_levels);
}

void rtpc_kls_device::device_reset()
{
}

void rtpc_kls_device::mcu_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("mcu", 0);
	map(0xf800, 0xffff).lr8([this]() { m_atn(1); m_atn(0); return 0; }, "sys_atn");
}

void rtpc_kls_device::mcu_port1_w(u8 data)
{
	LOG("volume %d\n", data >> 6);
	m_mcu_p[1] = (m_mcu_p[1] & ~KLS_P1O) | (data & KLS_P1O);

	// speaker volume wraps to ppi port b.6 and b.5
	m_ppi_p[1] &= ~0x60;
	m_ppi_p[1] |= (data >> 1) & 0x60;

	speaker();
}

void rtpc_kls_device::mcu_port2_w(u8 data)
{
	if ((data ^ m_mcu_p[2]) & KLS_IID)
	{
		static const char *const mcu_code[] =
		{
			"informational", "received byte from keyboard", "received byte from uart", "returning byte requested by system",
			"block transfer ready", "unassigned", "self-test performed", "error condition"
		};
		LOG("kls mcu iid %d: %s\n", data & 7, mcu_code[data & 7]);
	}

	if ((data ^ m_mcu_p[2]) & KLS_P23)
		LOG("kls mcu system reset %d\n", BIT(data, 3));

	if ((data ^ m_mcu_p[2]) & KLS_P24)
	{
		if (!BIT(data, 4))
		{
			LOG("kls mcu data out 0x%02x\n", m_mcu_p[0]);
			m_ppi_p[0] = m_mcu_p[0];
		}
		m_ppi->pc4_w(BIT(data, 4));
	}

	if ((data ^ m_mcu_p[2]) & KLS_P26)
	{

		m_ppi->pc6_w(BIT(data, 6));
		if (!BIT(data, 6))
		{
			LOG("kls mcu data in 0x%02x\n", m_ppi_p[0]);
			m_mcu_p[0] = m_ppi_p[0];
		}
	}

	m_mcu_p[2] = (m_mcu_p[2] & ~KLS_P2O) | (data & KLS_P2O);

	// speaker frequency wraps to ppi port b.7
	m_ppi_p[1] = (m_ppi_p[1] & ~0x80) | (~data & 0x80);

	// iid, ack, stb map to ppi port c
	m_ppi_p[2] = (m_ppi_p[2] & ~0x57) | (data & 0x57);

	speaker();
}

void rtpc_kls_device::mcu_port3_w(u8 data)
{
	// uart txd -> rxd wrap
	m_rcv[0]->in_w<1>(BIT(data, 1));

	m_kbdc->data_write_from_mb(BIT(data, 6));
	m_kbdc->clock_write_from_mb(BIT(data, 7));

	m_mcu_p[3] = (m_mcu_p[3] & ~KLS_P3O) | (data & KLS_P3O);
}

/*
 * bit  i/o  function
 *  0    i   iid0
 *  1    i   iid1
 *  2    i   iid2
 *  3    o   +irq
 *  4    i   -stb
 *  5    o   ibf
 *  6    i   -ack
 *  7    o   -obf
 */
void rtpc_kls_device::ppi_portc_w(u8 data)
{
	LOG("ppi_portc_w 0x%02x\n", data);

	// bit 3 (+irq) -> i/o channel
	if (BIT(m_ppi_p[2] ^ data, 3))
	{
		LOG("kls host irq %d\n", BIT(data, 3));
		m_irq(BIT(data, 3));
	}

	// bit 5 (+ibf) -> mcu p2.5
	if (BIT(m_ppi_p[2] ^ data, 5))
	{
		if (BIT(data, 5))
			m_mcu_p[2] |= KLS_P25;
		else
			m_mcu_p[2] &= ~KLS_P25;
	}

	// bit 7 (-obf) -> mcu p3.3 (-int1)
	if (BIT(m_ppi_p[2] ^ data, 7))
	{
		if (BIT(data, 7))
			m_mcu_p[3] |= KLS_P33;
		else
			m_mcu_p[3] &= ~KLS_P33;

		m_mcu->set_input_line(MCS51_INT1_LINE, !BIT(data, 7));
	}

	m_ppi_p[2] = (m_ppi_p[2] & ~0xa8) | (data & 0xa8);
}

void rtpc_kls_device::mcu_reset_w(int state)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, state);
}

void rtpc_kls_device::rtc_sqw_w(int state)
{
	if (state)
		m_ppi_p[1] |= 0x08;
	else
		m_ppi_p[1] &= ~0x08;
}

void rtpc_kls_device::rtc_irq_w(int state)
{
	if (state)
		m_ppi_p[1] |= 0x10;
	else
		m_ppi_p[1] &= ~0x10;
}

void rtpc_kls_device::cmd_w(u16 data)
{
	LOG("command 0x%02x data 0x%02x (%s)\n", data >> 8, u8(data), machine().describe_context());

	// 00rc cccc dddd dddd
	m_cmd = BIT(data, 8, 6);

	m_rcv[0]->in_w<0>(BIT(m_cmd, 5));
	m_rcv[1]->in_w<0>(!BIT(m_cmd, 5));

	switch (m_cmd & KLS_CMD)
	{
	case 0x0: // extended command
		switch (u8(data))
		{
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			LOG("clr mode bit %d\n", data & 0xf); break;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			LOG("set mode bit %d\n", data & 0xf); break;
		default: LOG("extended command 0x%02x\n", data); break;
		}
		break;
	case 0x1: LOG("write keyboard 0x%02x\n", data); break;
	case 0x2: LOG("write speaker 0x%02x\n", data); break;
	case 0x3: LOG("write uart control 0x%02x\n", data); break;
	case 0x4: LOG("write uart query 0x%02x\n", data); break;
	case 0x5: LOG("set uart rate 0x%02x\n", data); break;
	case 0x6: LOG("init uart 0x%02x\n", data); break;
	case 0x7: LOG("set speaker duration 0x%02x\n", data); break;
	case 0x8: LOG("set speaker freq-hi 0x%02x\n", data); break;
	case 0x9: LOG("set speaker freq-lo 0x%02x\n", data); break;
	case 0xc: LOG("diagnostic write 0x%02x\n", data); break;
	case 0xa: case 0xb: case 0xd: case 0xe: case 0xf:
		LOG("unassigned\n");
		break;
	default:
		LOG("write shared ram addr 0x%x data 0x%02x\n", m_cmd & 0xf, data);
		break;
	}

	m_mcu_p[1] = (m_mcu_p[1] & ~KLS_CMD) | (m_cmd & KLS_CMD);
	m_ppi->write(0, data);
}

void rtpc_kls_device::speaker()
{
	if (m_speaker)
		m_speaker->level_w((m_mcu_p[2] & KLS_P27) ? 0 : m_mcu_p[1] >> 6);
}

ROM_START(rtpc_kls)
	// Version: 073
	// Date: 85 289 (16 Oct 1985)
	// Checksum: 0xc8 0xe8
	ROM_REGION(0x1000, "mcu", 0)
	ROM_LOAD("61x6310_8051.bin", 0x0000, 0x1000, CRC(296c16c1) SHA1(83858109c39d5be37e49f24d1db4e2b15f38843e))
ROM_END

tiny_rom_entry const *rtpc_kls_device::device_rom_region() const
{
	return ROM_NAME(rtpc_kls);
}

INPUT_PORTS_START(rtpc_kls)
	PORT_START("KEYLOCK")
	PORT_CONFNAME(0x20, 0x00, "Key Lock")
	PORT_CONFSETTING(0x00, "Unlock")
	PORT_CONFSETTING(0x20, "Locked")
INPUT_PORTS_END

ioport_constructor rtpc_kls_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rtpc_kls);
}
