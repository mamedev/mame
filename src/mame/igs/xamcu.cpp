// license:BSD-3-Clause
// copyright-holders:XingXing, Vas Crabb

#include "emu.h"
#include "xamcu.h"

#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(IGS_XA_ICS_SOUND, igs_xa_mcu_ics_sound_device, "igs_xa_ics_sound", "IGS MX10EXAQC-based ICS2115 sound system")
DEFINE_DEVICE_TYPE(IGS_XA_SUBCPU,    igs_xa_mcu_subcpu_device,    "igs_xa_subcpu",    "IGS MX10EXAQC sub-CPU")


igs_xa_mcu_device_base::igs_xa_mcu_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_irq_cb(*this),
	m_command(0),
	m_num_params(0),
	m_port_dat{ 0, 0, 0, 0 },
	m_port0_latch(0),
	m_port2_latch(0),
	m_irq(0)
{
}

igs_xa_mcu_device_base::~igs_xa_mcu_device_base()
{
}


void igs_xa_mcu_device_base::device_add_mconfig(machine_config &config)
{
	MX10EXA(config, m_mcu, DERIVED_CLOCK(1, 1)); // MX10EXAQC (Philips 80C51XA)
	m_mcu->port_in_cb<0>().set(FUNC(igs_xa_mcu_device_base::mcu_p0_r));
	m_mcu->port_in_cb<1>().set(FUNC(igs_xa_mcu_device_base::mcu_p1_r));
	m_mcu->port_in_cb<2>().set(FUNC(igs_xa_mcu_device_base::mcu_p2_r));
	m_mcu->port_in_cb<3>().set(FUNC(igs_xa_mcu_device_base::mcu_p3_r));
	m_mcu->port_out_cb<0>().set(FUNC(igs_xa_mcu_device_base::mcu_p0_w));
	m_mcu->port_out_cb<1>().set(FUNC(igs_xa_mcu_device_base::mcu_p1_w));
	m_mcu->port_out_cb<2>().set(FUNC(igs_xa_mcu_device_base::mcu_p2_w));
	m_mcu->port_out_cb<3>().set(FUNC(igs_xa_mcu_device_base::mcu_p3_w));
}

void igs_xa_mcu_device_base::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_num_params));
	save_item(NAME(m_port_dat));
	save_item(NAME(m_port0_latch));
	save_item(NAME(m_port2_latch));
	save_item(NAME(m_irq));
}

void igs_xa_mcu_device_base::device_reset()
{
	m_command = 0;
	m_num_params = 0;
}


inline void igs_xa_mcu_device_base::set_irq(int state)
{
	if (bool(m_irq) != bool(state))
	{
		m_irq = state;
		m_irq_cb(state);
	}
}


u8 igs_xa_mcu_device_base::mcu_p0_r()
{
	LOG("%s: COMMAND READ LOWER mcu_p0_r() returning %02x with port3 as %02x\n", machine().describe_context(), m_port0_latch, m_port_dat[3]);
	return m_port0_latch;
}

u8 igs_xa_mcu_device_base::mcu_p1_r()
{
	LOG("%s: mcu_p1_r()\n", machine().describe_context());
	return m_port_dat[1]; // superkds XA will end up failing returning port1 dat for now, but not attempt to play any sounds otherwise?
}

u8 igs_xa_mcu_device_base::mcu_p2_r()
{
	LOG("%s: COMMAND READ mcu_p2_r() returning %02x with port3 as %02x\n", machine().describe_context(), m_port2_latch, m_port_dat[3]);
	return m_port2_latch;
}

u8 igs_xa_mcu_device_base::mcu_p3_r()
{
	LOG("%s: mcu_p3_r()\n", machine().describe_context());
	return m_port_dat[3];
}


void igs_xa_mcu_device_base::mcu_p0_w(u8 data)
{
	LOG("%s: mcu_p0_w() %02x with port 3 as %02x and port 1 as %02x\n", machine().describe_context(), data, m_port_dat[3], m_port_dat[1]);
	m_port_dat[0] = data;
}

void igs_xa_mcu_device_base::mcu_p1_w(u8 data)
{
	LOG("%s: mcu_p1_w() %02x\n", machine().describe_context(), data);
	m_port_dat[1] = data;
}

void igs_xa_mcu_device_base::mcu_p2_w(u8 data)
{
	LOG("%s: mcu_p2_w() %02x with port 3 as %02x\n", machine().describe_context(), data, m_port_dat[3]);
	m_port_dat[2] = data;
}

void igs_xa_mcu_device_base::mcu_p3_w(u8 data)
{
	LOG("%s: mcu_p3_w() %02x - do latches oldport3 %02x newport3 %02x\n", machine().describe_context(), data, m_port_dat[3], data);
	m_port_dat[3] = data;
}


TIMER_CALLBACK_MEMBER(igs_xa_mcu_device_base::do_cmd_w)
{
	m_command = u16(u32(param));

	m_num_params--;
	if (m_num_params <= 0)
	{
		LOG("command is %02x size %02x\n", m_command >> 8, m_command & 0x00ff);
		m_num_params = m_command & 0x00ff;
	}
	else
	{
		LOG("param %04x\n", m_command);
	}
	m_mcu->pulse_input_line(XA_EXT_IRQ0, m_mcu->minimum_quantum_time());
}



igs_xa_mcu_ics_sound_device::igs_xa_mcu_ics_sound_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	igs_xa_mcu_device_base(mconfig, IGS_XA_ICS_SOUND, tag, owner, clock),
	m_ics(*this, "ics"),
	m_response{ 0, 0 }
{
}

igs_xa_mcu_ics_sound_device::~igs_xa_mcu_ics_sound_device()
{
}


void igs_xa_mcu_ics_sound_device::cmd_w(u16 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(igs_xa_mcu_ics_sound_device::do_cmd_w), this), s32(u32(data)));
}


void igs_xa_mcu_ics_sound_device::device_add_mconfig(machine_config &config)
{
	igs_xa_mcu_device_base::device_add_mconfig(config);

	m_mcu->port_out_cb<1>().set(FUNC(igs_xa_mcu_ics_sound_device::mcu_p1_w));
	m_mcu->port_out_cb<3>().set(FUNC(igs_xa_mcu_ics_sound_device::mcu_p3_w));

	SPEAKER(config, "mono").front_center();

	ICS2115(config, m_ics, 33.8688_MHz_XTAL); // TODO: Correct?
	m_ics->irq().set_inputline(m_mcu, XA_EXT_IRQ2);
	m_ics->add_route(ALL_OUTPUTS, "mono", 5.0);
}

void igs_xa_mcu_ics_sound_device::device_start()
{
	igs_xa_mcu_device_base::device_start();

	save_item(NAME(m_response));
}


void igs_xa_mcu_ics_sound_device::mcu_p1_w(u8 data)
{
	igs_xa_mcu_device_base::mcu_p1_w(data);

	set_irq(BIT(data, 3));
}

void igs_xa_mcu_ics_sound_device::mcu_p3_w(u8 data)
{
	u8 const falling = m_port_dat[3] & ~data;
	igs_xa_mcu_device_base::mcu_p3_w(data);

	// high->low transition on bit 0x80 must read into latches!
	if (BIT(falling, 7))
	{
		if (!BIT(data, 4))
		{
			m_port0_latch = m_ics->read(m_port_dat[1] & 7);
			LOG("read from ics [%d] = [%02x]\n", m_port_dat[1] & 7, m_port0_latch);
		}
		else if (!BIT(data, 5))
		{
			LOG("read command [%d] = [%04x]\n", m_port_dat[1] & 7, m_command);
			m_port2_latch = m_command >> 8;
			m_port0_latch = m_command & 0x00ff;
		}
	}

	if (BIT(falling, 6))
	{
		if (!BIT(data, 4))
		{
			LOG("write to ics [%d] = [%02x]\n", m_port_dat[1] & 7, m_port_dat[0]);
			m_ics->write(m_port_dat[1] & 7, m_port_dat[0]);
		}
		else if (!BIT(data, 5))
		{
			u16 const dat = (u16(m_port_dat[2]) << 8) | m_port_dat[0];
			LOG("write response [%d] = [%04x]\n", m_port_dat[1] & 7, dat);
			switch (m_port_dat[1] & 7)
			{
			case 1:
				m_response[1] = dat;
				break;
			case 2:
				m_response[0] = dat;
				break;
			}
		}
	}
}



igs_xa_mcu_subcpu_device::igs_xa_mcu_subcpu_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	igs_xa_mcu_device_base(mconfig, IGS_XA_SUBCPU, tag, owner, clock),
	m_response(0)
{
}

igs_xa_mcu_subcpu_device::~igs_xa_mcu_subcpu_device()
{
}


void igs_xa_mcu_subcpu_device::cmd_w(offs_t offset, u16 data)
{
	if (!BIT(offset, 0))
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(igs_xa_mcu_subcpu_device::do_cmd_w), this), s32(u32(data)));
	}
	else
	{
		LOG("%s: lower IRQ %08x\n", machine().describe_context(), data);
		set_irq(0);
	}
}


void igs_xa_mcu_subcpu_device::device_add_mconfig(machine_config &config)
{
	igs_xa_mcu_device_base::device_add_mconfig(config);

	m_mcu->port_out_cb<3>().set(FUNC(igs_xa_mcu_subcpu_device::mcu_p3_w));
}

void igs_xa_mcu_subcpu_device::device_start()
{
	igs_xa_mcu_device_base::device_start();

	save_item(NAME(m_response));
}


void igs_xa_mcu_subcpu_device::mcu_p3_w(u8 data)
{
	u8 const rising = ~m_port_dat[3] & data;
	u8 const falling = m_port_dat[3] & ~data;
	igs_xa_mcu_device_base::mcu_p3_w(data);

	if (BIT(rising, 5))
		set_irq(1);

	// high->low transition on bit 0x80 must read into latches!
	if (BIT(falling, 7))
	{
		LOG("read command [%d] = [%04x]\n", m_port_dat[1] & 7, m_command);
		m_port2_latch = m_command >> 8;
		m_port0_latch = m_command & 0x00ff;
	}

	if (BIT(falling, 6))
	{
		u16 const dat = (u16(m_port_dat[2]) << 8) | m_port_dat[0];
		LOG("write response [%d] = [%04x]\n", m_port_dat[1] & 7, dat);
		m_response = dat;
	}
}
