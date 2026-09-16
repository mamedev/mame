// license:BSD-3-Clause
// copyright-holders:smf

#include "emu.h"
#include "jvs.h"

#define LOG_PACKETS (1U << 1)
#define LOG_SIGNALS (1U << 2)
#define LOG_IGNORED (1U << 3)
#define LOG_CONFLICT (1U << 4)

//#define VERBOSE (LOG_PACKETS /*| LOG_SIGNALS | LOG_IGNORED | LOG_CONFLICT*/)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGPACKETS(...)  LOGMASKED(LOG_PACKETS, __VA_ARGS__)
#define LOGSIGNALS(...)  LOGMASKED(LOG_SIGNALS, __VA_ARGS__)
#define LOGIGNORED(...)  LOGMASKED(LOG_IGNORED, __VA_ARGS__)
#define LOGCONFLICT(...)  LOGMASKED(LOG_CONFLICT, __VA_ARGS__)

DEFINE_DEVICE_TYPE(JVS_PORT, jvs_port_device, "jvs_port", "JVS IO Port")

#define FIX_CONFLICT (1)

jvs_port_device::jvs_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JVS_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_jvs_interface>(mconfig, *this),
	m_device(nullptr),
	m_debug_timer(nullptr),
	m_rxd_cb(*this),
	m_sense_cb(*this),
	m_rts(0),
	m_rxd(0),
	m_txd(1)
{
}

jvs_port_device::~jvs_port_device()
{
}

void jvs_port_device::device_config_complete()
{
	m_device = get_card_device();
}

void jvs_port_device::device_start()
{
	device_jvs_interface *intf;
	if (!owner()->interface(intf))
	{
		if (VERBOSE & LOG_PACKETS)
			m_debug_timer = timer_alloc(FUNC(jvs_port_device::debug_timer_callback), this);

		m_debug_bit = -1;
		m_debug_index = 0;
		m_debug_rxd = 1;
		std::fill_n(m_debug_buffer, std::size(m_debug_buffer), 0);

		update_rxd();

		save_item(NAME(m_debug_bit));
		save_item(NAME(m_debug_index));
		save_item(NAME(m_debug_rxd));
		save_item(NAME(m_debug_buffer));
	}

	save_item(NAME(m_rts));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
}

void jvs_port_device::rts(int state)
{
	if (m_rts != state)
	{
		LOGSIGNALS("%s rts %d\n", machine().describe_context(), state);

		if (machine().scheduler().currently_executing())
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(jvs_port_device::update_rts), this), state);
		else
			update_rts(state);
	}
}

void jvs_port_device::update_rts(s32 state)
{
	debug_flush();
	debug_stop();

	m_rts = state;

	if (m_device)
	{
		if (m_rts && !m_rxd && !(FIX_CONFLICT && m_txd))
			m_rxd_cb(1);

		update_rxd();
	}
}

void jvs_port_device::txd(int state)
{
	if (m_txd != state)
	{
		LOGSIGNALS("%s txd %d\n", machine().describe_context(), state);
		m_txd = state;

		update_rxd();
	}
}

void jvs_port_device::update_rxd()
{
	int high = 0;
	int low = 0;

	if (m_rts)
	{
		high += (m_txd != 0);
		low += (m_txd == 0);
	}
	else if (!m_txd)
		LOGIGNORED("ignored txd\n");

	for (device_jvs_interface *intf = m_device; intf; intf = intf->m_jvs ? intf->m_jvs->m_device : nullptr)
	{
		if (intf->m_rts)
		{
			high += (intf->m_txd != 0);
			low += (intf->m_txd == 0);
		}
		else if (!intf->m_txd)
			LOGIGNORED("ignored txd %s\n", intf->device().tag());
	}

	if (high + low > 1 && ((VERBOSE & LOG_CONFLICT) || FIX_CONFLICT))
	{
		debug_flush('@');

		std::string conflict;
		if (VERBOSE & LOG_CONFLICT)
			conflict = util::string_format("conflict detected between %d devices (%d high, %d low)", high + low, high, low);

		if (m_rts)
		{
			if (VERBOSE & LOG_CONFLICT)
				conflict += util::string_format(",%s (%d)", tag(), m_txd);

			if (FIX_CONFLICT && low && m_txd)
				m_rts = 0;
		}

		for (device_jvs_interface *intf = m_device; intf; intf = intf->m_jvs ? intf->m_jvs->m_device : nullptr)
			if (intf->m_rts)
			{
				if (VERBOSE & LOG_CONFLICT)
					conflict += util::string_format(",%s (%d)", intf->device().tag(), intf->m_txd);

				if (FIX_CONFLICT && low && intf->m_txd)
					intf->m_rts = 0;
			}

		LOGCONFLICT("%s\n", conflict);

		if (FIX_CONFLICT && low)
			high = 0;
	}

	int rxd = low && !high ? 0 : 1;
	if (m_rxd != rxd)
	{
		m_rxd = rxd;

		for (device_jvs_interface *intf = m_device; intf; intf = intf->m_jvs ? intf->m_jvs->m_device : nullptr)
			if (!intf->m_rts)
				intf->rxd(m_rxd);

		if (!m_rts)
			m_rxd_cb(m_rxd);

		debug_rxd(m_rxd);
	}

	if (high + low == 0)
	{
		debug_flush();
		debug_stop();
	}
}

void jvs_port_device::debug_rxd(int state)
{
	if (!state && m_debug_rxd && m_debug_timer && m_debug_timer->expire().is_never())
		m_debug_timer->adjust(attotime::from_hz(115200 * 2), 0, attotime::from_hz(115200));

	m_debug_rxd = state;
}

void jvs_port_device::debug_flush(wchar_t ch)
{
	if (m_debug_index)
	{
		if (VERBOSE & LOG_PACKETS)
		{
			std::string debug;
			debug += m_rts ? ">" : "<";
			for (int i = 0; i < m_debug_index; i++)
				debug += util::string_format(" %02x", m_debug_buffer[i]);
			if (ch)
				debug += ch;
			debug += "\n";

			LOGPACKETS("%s", debug);
		}

		std::fill_n(m_debug_buffer, m_debug_index, 0);
		m_debug_index = 0;

		debug_stop();
	}
}

void jvs_port_device::debug_stop()
{
	if (m_debug_timer)
		m_debug_timer->adjust(attotime::never);
	m_debug_bit = -1;
}

TIMER_CALLBACK_MEMBER(jvs_port_device::debug_timer_callback)
{
	if (m_debug_bit < 0)
	{
		if (m_debug_rxd)
			debug_stop();
		else
			m_debug_bit = 0;
	}
	else if (m_debug_bit == 8)
	{
		m_debug_index++;

		if (!m_debug_rxd)
			debug_flush('*');
		else if (m_debug_index >= std::size(m_debug_buffer))
			debug_flush('!');

		debug_stop();
	}
	else
	{
		if (m_debug_rxd)
			m_debug_buffer[m_debug_index] |= 1 << m_debug_bit;

		m_debug_bit++;
	}
}

device_jvs_interface::device_jvs_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "jvs"),
	m_jvs(*this, "jvs"),
	m_jvs_sense(jvs_port_device::sense::None),
	m_default_inputs(true),
	m_output_cb(*this),
	m_analog_output_cb(*this),
	m_system(*this, "SYSTEM"),
	m_system_cb(*this, 0),
	m_player(*this, "PLAYER%u", 1U),
	m_player_cb(*this, 0),
	m_coin(*this, "COIN%u", 1U),
	m_coin_cb(*this, 0),
	m_analog_input(*this, "ANALOG_INPUT%u", 1U),
	m_analog_input_cb(*this, 0),
	m_rotary_input(*this, "ROTARY_INPUT%u", 1U),
	m_rotary_input_cb(*this, 0),
	m_screen_position_enable(*this, "SCREEN_POSITION_INPUT_ENABLE%u", 1U),
	m_screen_position_enable_cb(*this, 0xffff),
	m_screen_position_x(*this, "SCREEN_POSITION_INPUT_X%u", 1U),
	m_screen_position_x_cb(*this, 0),
	m_screen_position_y(*this, "SCREEN_POSITION_INPUT_Y%u", 1U),
	m_screen_position_y_cb(*this, 0),
	m_rts(0),
	m_sense(jvs_port_device::sense::None),
	m_txd(1)
{
	m_port = dynamic_cast<jvs_port_device *>(device.owner());

	m_root = m_port;
	if (m_root)
	{
		while (m_root->owner() && dynamic_cast<jvs_port_device *>(m_root->owner()->owner()))
			m_root = dynamic_cast<jvs_port_device *>(m_root->owner()->owner());
	}
}

void device_jvs_interface::interface_post_start()
{
	sense(jvs_port_device::sense::Uninitialized);

	device().save_item(NAME(m_jvs_sense));
	device().save_item(NAME(m_rts));
	device().save_item(NAME(m_sense));
	device().save_item(NAME(m_txd));
}

device_jvs_interface::~device_jvs_interface()
{
}

void device_jvs_interface::rts(int state)
{
	if (m_rts != state)
	{
		if (device().machine().scheduler().currently_executing())
			device().machine().scheduler().synchronize(timer_expired_delegate(FUNC(device_jvs_interface::update_rts), this), state);
		else
			update_rts(state);
	}
}

void device_jvs_interface::update_rts(int32_t state)
{
	m_rts = state;

	if (m_root)
	{
		if (m_rts && !m_root->m_rxd && !(FIX_CONFLICT && m_txd))
			rxd(1);

		m_root->update_rxd();
	}
}

void device_jvs_interface::txd(int state)
{
	if (m_txd != state)
	{
		m_txd = state;

		if (m_root)
			m_root->update_rxd();
	}
}

void device_jvs_interface::sense(uint8_t state)
{
	if (m_sense != state)
	{
		if (device().machine().scheduler().currently_executing())
			device().machine().scheduler().synchronize(timer_expired_delegate(FUNC(device_jvs_interface::update_sense), this), state);
		else
			update_sense(state);
	}
}

void device_jvs_interface::update_sense(int32_t state)
{
	m_sense = state;

	if (m_port)
	{
#if VERBOSE & LOG_SIGNALS
		device().
		LOG_OUTPUT_FUNC("%s sense %s %d\n", device().tag(),
			m_sense == jvs_port_device::sense::Initialized ? "Initialized" :
			m_sense == jvs_port_device::sense::Uninitialized ? "Uninitialized" :
			m_sense == jvs_port_device::sense::None ? "None" : "", state);
#endif

		device_jvs_interface *intf;
		if (m_port->owner()->interface(intf))
			intf->jvs_sense(m_sense);
		else
			m_port->m_sense_cb(m_sense);
	}
}

uint8_t device_jvs_interface::system_r(uint8_t mem_mask)
{
	return (m_default_inputs ? m_system.read_safe(0) : m_system_cb(mem_mask)) & mem_mask;
}

uint32_t device_jvs_interface::player_r(offs_t offset, uint32_t mem_mask)
{
	return (m_default_inputs ? m_player[offset].read_safe(0) : m_player_cb[offset](0, mem_mask)) & mem_mask;
}

uint8_t device_jvs_interface::coin_r(offs_t offset, uint8_t mem_mask)
{
	return (m_default_inputs ? m_coin[offset].read_safe(0) : m_coin_cb[offset](0, mem_mask)) & mem_mask;
}

uint16_t device_jvs_interface::analog_input_r(offs_t offset, uint16_t mem_mask)
{
	return (m_default_inputs ? m_analog_input[offset].read_safe(0) : m_analog_input_cb[offset](0, mem_mask)) & mem_mask;
}

uint16_t device_jvs_interface::rotary_input_r(offs_t offset)
{
	return m_default_inputs ? m_rotary_input[offset].read_safe(0) : m_rotary_input_cb[offset]();
}

uint16_t device_jvs_interface::screen_position_enable_r(offs_t offset)
{
	return m_default_inputs ? m_screen_position_enable[offset].read_safe(0xffff) : m_screen_position_enable_cb[offset]();
}

uint16_t device_jvs_interface::screen_position_x_r(offs_t offset)
{
	return m_default_inputs ? m_screen_position_x[offset].read_safe(0) : m_screen_position_x_cb[offset]();
}

uint16_t device_jvs_interface::screen_position_y_r(offs_t offset)
{
	return m_default_inputs ? m_screen_position_y[offset].read_safe(0) : m_screen_position_y_cb[offset]();
}

void device_jvs_interface::add_jvs_port(machine_config &config)
{
	JVS_PORT(config, m_jvs, jvs_port_devices, nullptr);
}

void device_jvs_interface::jvs_sense(uint8_t state)
{
	m_jvs_sense = state;
}

#include "cyberlead.h"
#include "namcoio.h"

void jvs_port_devices(device_slot_interface &device)
{
	device.option_add("namco_amc", NAMCO_AMC);
	device.option_add("namco_asca1", NAMCO_ASCA1);
	device.option_add("namco_asca3", NAMCO_ASCA3);
	device.option_add("namco_asca3a", NAMCO_ASCA3A);
	device.option_add("namco_asca5", NAMCO_ASCA5);
	device.option_add("namco_csz1", NAMCO_CSZ1);
	device.option_add("namco_cyberlead", NAMCO_CYBERLEAD);
	device.option_add("namco_cyberleada", NAMCO_CYBERLEADA);
	device.option_add("namco_emio102", NAMCO_EMIO102);
	device.option_add("namco_empri101", NAMCO_EMPRI101);
	device.option_add("namco_fca10", NAMCO_FCA10);
	device.option_add("namco_fca11", NAMCO_FCA11);
	device.option_add("namco_fcb", NAMCO_FCB);
	device.option_add("namco_tssio", NAMCO_TSSIO);
	device.option_add("namco_xmiu1", NAMCO_XMIU1);
}
