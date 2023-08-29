// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Zoran ZR36110 mpeg video decoder

#include "emu.h"
#include "zr36110.h"

#define LOG_MICROCODE (1U << 1)
#define LOG_SETUP     (1U << 2)

#define VERBOSE       (LOG_GENERAL|LOG_SETUP)
#include "logmacro.h"

#define LOGMICROCODE(...) LOGMASKED(LOG_MICROCODE, __VA_ARGS__)

DEFINE_DEVICE_TYPE(ZR36110, zr36110_device, "zr36110", "Zoran ZR36110 mpeg decoder")

zr36110_device::zr36110_device(const machine_config &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, ZR36110, tag, owner, clock),
	m_drq_w(*this),
	m_sp_frm_w{*this, *this},
	m_sp_dat_w{*this, *this},
	m_sp_clk_w{*this, *this}
{
}

void zr36110_device::device_start()
{
	save_item(NAME(m_mc1_adr));
	save_item(NAME(m_mc23_adr));
	save_item(NAME(m_setup_adr));
	save_item(NAME(m_state));
	save_item(NAME(m_setup));
	save_item(NAME(m_cmd_phase));
	save_item(NAME(m_cmd));
	save_item(NAME(m_bus_control));
}

void zr36110_device::device_reset()
{
	memset(m_setup, 0, sizeof(m_setup));
	m_setup[0] = 0x10;
	m_mc1_adr = 0;
	m_mc23_adr = 0;
	m_setup_adr = 0;
	m_state = S_IDLE;
	m_cmd_phase = false;
	m_cmd = 0;
	m_bus_control = 0x10;
}

void zr36110_device::mc18_w(u8 data)
{
	m_mc23_adr = 0;
	m_setup_adr = 0;
	LOGMICROCODE("mc1[%03x] = %02x\n", m_mc1_adr, data);
	m_mc1_adr = m_mc1_adr + 1;
}

void zr36110_device::mc1x_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		mc18_w(data >> 8);
	else if(m_bus_control & 0x40) {
		mc18_w(data >> 8);
		mc18_w(data);
	} else {
		mc18_w(data);
		mc18_w(data >> 8);
	}
}

void zr36110_device::mc1_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		mc18_w(data);
	else if(m_bus_control & 0x40) {
		mc18_w(data);
		mc18_w(data >> 8);
	} else {
		mc18_w(data >> 8);
		mc18_w(data);
	}
}

void zr36110_device::mc238_w(u8 data)
{
	m_setup_adr = 0;
	m_mc1_adr = 0;
	LOGMICROCODE("mc23[%03x] = %02x\n", m_mc23_adr, data);
	m_mc23_adr = m_mc23_adr + 1;
	if(m_mc23_adr >= 0x2000 && m_state == S_INIT)
		m_state = S_IDLE;
}

void zr36110_device::mc23_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		mc238_w(data);
	else if(m_bus_control & 0x40) {
		mc238_w(data);
		mc238_w(data >> 8);
	} else {
		mc238_w(data >> 8);
		mc238_w(data);
	}
}

void zr36110_device::mc23x_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		mc238_w(data >> 8);
	else if(m_bus_control & 0x40) {
		mc238_w(data >> 8);
		mc238_w(data);
	} else {
		mc238_w(data);
		mc238_w(data >> 8);
	}
}

double zr36110_device::u6_10_to_f(u16 val)
{
	if(val & 0x8000)
		return (val - 0x10000) / 1024.0;
	else
		return val / 1024.0;
}

double zr36110_device::u5_19_to_f(u32 val)
{
	return val / 524288.0;
}

void zr36110_device::setup_show() const
{
	LOG_OUTPUT_FUNC("Chip setup:\n");
	LOG_OUTPUT_FUNC("  00    - buswidth=%d order=%s stream=%s burst_length=%d\n",
					m_setup[0x00] & 0x80 ? 16 : 8,
					m_setup[0x00] & 0x40 ? "lsb" : "msb",
					m_setup[0x00] & 0x20 ? "dma" : "pio",
					m_setup[0x00] & 0x1f);
	LOG_OUTPUT_FUNC("  01    - busoff=%d\n",
					m_setup[0x01] << 4);
	LOG_OUTPUT_FUNC("  02    - type=%s rate=%s interface=%s half=%s dram=%s start=%s last=%s\n",
					m_setup[0x02] & 0x80 ? m_setup[0x02] & 0x04 ? "serial" : "video" : m_setup[0x02] & 0x04 ? "sectors" : "stream",
					m_setup[0x02] & 0x40 ? "pal" : "ntsc",
					m_setup[0x02] & 0x20 ? "enabled" : "normal",
					m_setup[0x02] & 0x10 ? "as-needed" : "always",
					m_setup[0x02] & 0x08 ? "8M" : "4M",
					m_setup[0x02] & 0x02 ? "sequence" : "any",
					m_setup[0x02] & 0x01 ? "image" : "background");
	LOG_OUTPUT_FUNC("  03    - size=%s hinterpolate=%s color=%s bias=%s fi=%s\n",
					m_setup[0x03] & 0x80 ? "sif-prog" : "ccir-int",
					m_setup[0x03] & 0x40 ? "off" : "on",
					m_setup[0x03] & 0x20 ? (m_setup[0x03] & 0x18) == 0x00 ? "rgb888" : (m_setup[0x03] & 0x18) == 0x08 ? "rgb565" : (m_setup[0x03] & 0x18) == 0x10 ? "rgb555" : "rgb?" : m_setup[0x03] & 0x04 ? "yuv411" : "yuv422",
					m_setup[0x03] & 0x02 ? "on" : "off",
					m_setup[0x03] & 0x01 ? "blank" : "field");
	LOG_OUTPUT_FUNC("  04    - signals=%s hsync=%s,%s vsync=%s,%s field=%s,%s clk=%s\n",
					m_setup[0x04] & 0x80 ? "output" : "input",
					m_setup[0x04] & 0x40 ? "high" : "low",
					m_setup[0x04] & 0x08 ? "blank" : "sync",
					m_setup[0x04] & 0x20 ? "high" : "low",
					m_setup[0x04] & 0x04 ? "blank" : "sync",
					m_setup[0x04] & 0x10 ? "high" : "low",
					m_setup[0x04] & 0x02 ? "II" : "I",
					m_setup[0x04] & 0x01 ? "qclk_v" : "vclk");
	LOG_OUTPUT_FUNC("  05    - sp2frm=%s sp2clk=%s sp2=%s sp1frm=%s sp1clk=%s sp1=%s\n",
					m_setup[0x05] & 0x80 ? m_setup[0x05] & 0x20 ? "window" : "transition" : "pulse",
					m_setup[0x05] & 0x40 ? "output" : "input",
					m_setup[0x05] & 0x10 ? "on" : "off",
					m_setup[0x05] & 0x08 ? m_setup[0x05] & 0x02 ? "window" : "transition" : "pulse",
					m_setup[0x05] & 0x04 ? "output" : "input",
					m_setup[0x05] & 0x01 ? "on" : "off");
	LOG_OUTPUT_FUNC("  06    - serial_audio_mp1=%s fi=%s\n",
					m_setup[0x06] & 0x02 ? "yes" : "no",
					m_setup[0x06] & 0x01 ? "normal" : "resolution");
	LOG_OUTPUT_FUNC("  08-21 - active=%dx%d offset=%d,%d total=%dx%d blank=%d,%d front_blank=%d,%d delay=%d,%d, sync=%d,%d\n",
					(m_setup[0x08] << 8) | m_setup[0x09],
					(m_setup[0x14] << 8) | m_setup[0x15],
					(m_setup[0x0a] << 8) | m_setup[0x0b],
					(m_setup[0x16] << 8) | m_setup[0x17],
					(m_setup[0x0c] << 8) | m_setup[0x0d],
					(m_setup[0x18] << 8) | m_setup[0x19],
					(m_setup[0x0e] << 8) | m_setup[0x0f],
					(m_setup[0x1a] << 8) | m_setup[0x1b],
					(m_setup[0x10] << 8) | m_setup[0x11],
					(m_setup[0x1c] << 8) | m_setup[0x1d],
					(m_setup[0x12] << 8) | m_setup[0x13],
					(m_setup[0x1e] << 8) | m_setup[0x1f],
					m_setup[0x20],
					m_setup[0x21]);
	LOG_OUTPUT_FUNC("  22-29 - crv=%f cbu=%f cgv=%f cgu=%f\n",
					u6_10_to_f((m_setup[0x22] << 8) | m_setup[0x23]),
					u6_10_to_f((m_setup[0x24] << 8) | m_setup[0x25]),
					u6_10_to_f((m_setup[0x26] << 8) | m_setup[0x27]),
					u6_10_to_f((m_setup[0x28] << 8) | m_setup[0x29]));
	LOG_OUTPUT_FUNC("  2a-2c - background=%d,%d,%d\n",
					m_setup[0x2a],
					m_setup[0x2b],
					m_setup[0x2c]);
	LOG_OUTPUT_FUNC("  2d    - high_byte=%02x\n",
					m_setup[0x2d]);
	LOG_OUTPUT_FUNC("  2e-40 - spclk=1/%d,1/%d spbr=%f,%f spdelay=%d,%d vdelay=%d vclk=%d\n",
					(m_setup[0x34] << 8) | m_setup[0x35],
					(m_setup[0x36] << 8) | m_setup[0x37],
					u5_19_to_f((m_setup[0x2e] << 16) | (m_setup[0x2f] << 8) | m_setup[0x30]),
					u5_19_to_f((m_setup[0x31] << 16) | (m_setup[0x32] << 8) | m_setup[0x33]),
					(m_setup[0x38] << 8) | m_setup[0x39],
					(m_setup[0x3a] << 8) | m_setup[0x3b],
					(m_setup[0x3c] << 8) | m_setup[0x3d],
					(m_setup[0x3e] << 16) | (m_setup[0x3f] << 8) | m_setup[0x40]);
	LOG_OUTPUT_FUNC("  42-44 - sp1id=%02x sp2id=%02x vidid=%02x\n",
					m_setup[0x42],
					m_setup[0x43],
					m_setup[0x44]);
}

void zr36110_device::setup8_w(u8 data)
{
	m_mc1_adr = 0;
	m_mc23_adr = 0;
	if(m_setup_adr < 0x80)
		m_setup[m_setup_adr] = data;
	if(m_setup_adr == 0x80 && (VERBOSE & LOG_SETUP))
		setup_show();
	m_setup_adr = m_setup_adr + 1;
}

void zr36110_device::setup_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		setup8_w(data);
	else if(m_bus_control & 0x40) {
		setup8_w(data);
		setup8_w(data >> 8);
	} else {
		setup8_w(data >> 8);
		setup8_w(data);
	}
}

void zr36110_device::setupx_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		setup8_w(data >> 8);
	else if(m_bus_control & 0x40) {
		setup8_w(data >> 8);
		setup8_w(data);
	} else {
		setup8_w(data);
		setup8_w(data >> 8);
	}
}

void zr36110_device::cmd8_w(u8 data)
{
	//  LOG("cmd_w %02x\n", data);

	if(m_cmd_phase) {
		m_cmd |= data;
		switch(m_cmd & 0xff) {
		case 0x00: go(); break;
		case 0x81: case 0x82: case 0x83: case 0x84: end_decoding(m_cmd & 3); break;
		default:
			logerror("Unknown command %02x.%02x\n", m_cmd >> 8, m_cmd & 0xff);
		}
	} else
		m_cmd = data << 8;
	m_cmd_phase = !m_cmd_phase;
}

void zr36110_device::cmd_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		cmd8_w(data);
	else if(m_bus_control & 0x40) {
		cmd8_w(data);
		cmd8_w(data >> 8);
	} else {
		cmd8_w(data >> 8);
		cmd8_w(data);
	}
}

void zr36110_device::cmdx_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		cmd8_w(data >> 8);
	else if(m_bus_control & 0x40) {
		cmd8_w(data >> 8);
		cmd8_w(data);
	} else {
		cmd8_w(data);
		cmd8_w(data >> 8);
	}
}

void zr36110_device::dma8_w(u8 data)
{
	//  logerror("dma %02x\n", data);
}

void zr36110_device::dma_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		dma8_w(data);
	else if(m_bus_control & 0x40) {
		dma8_w(data);
		dma8_w(data >> 8);
	} else {
		dma8_w(data >> 8);
		dma8_w(data);
	}
}

void zr36110_device::dmax_w(u16 data)
{
	if(!(m_bus_control & 0x80))
		dma8_w(data >> 8);
	else if(m_bus_control & 0x40) {
		dma8_w(data >> 8);
		dma8_w(data);
	} else {
		dma8_w(data);
		dma8_w(data >> 8);
	}
}

u8 zr36110_device::stat08_r()
{
	return 0x20 | m_state;
}

u8 zr36110_device::stat18_r()
{
	return 0x0c;
}

u8 zr36110_device::stat28_r()
{
	return 0x01;
}

u16 zr36110_device::stat0_r()
{
	return 0x20 | m_state;
}

u16 zr36110_device::stat1_r()
{
	return 0x0c;
}

u16 zr36110_device::stat2_r()
{
	return 0x01;
}

u16 zr36110_device::stat0x_r()
{
	return (0x20 | m_state) << 8;
}

u16 zr36110_device::stat1x_r()
{
	return 0x0c << 8;
}

u16 zr36110_device::stat2x_r()
{
	return 0x01 << 8;
}

u8 zr36110_device::user8_r()
{
	return 0;
}

u16 zr36110_device::user_r()
{
	return 0;
}

u16 zr36110_device::userx_r()
{
	return 0;
}

void zr36110_device::go()
{
	if(m_state != S_IDLE) {
		logerror("command GO on wrong state, ignoring\n");
		return;
	}
	logerror("command GO\n");
	m_bus_control = m_setup[0];
	m_state = S_NORMAL;
	m_drq_w(1);
}

void zr36110_device::end_decoding(u8 mode)
{
	if(m_state != S_NORMAL) {
		logerror("command END DECODING on wrong state, ignoring\n");
		return;
	}
	logerror("command END DECODING %d\n", mode);
	m_state = S_IDLE;
	m_drq_w(0);
}
