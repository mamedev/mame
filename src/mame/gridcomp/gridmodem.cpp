// license:BSD-3-Clause
// copyright-holders:vklachkov

#include "gridmodem.h"

#include "asio.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>

#define LOG_RAW     (1U << 1)
#define LOG_REG     (1U << 2)
#define LOG_CMD     (1U << 3)
#define LOG_ERROR   (1U << 4)
#define LOG_IRQ     (1U << 5)
#define LOG_RX      (1U << 6)
#define LOG_TX      (1U << 7)
#define LOG_SOCKET  (1U << 8)
#define LOG_DIAL    (1U << 9)
#define LOG_RAWCMD  (1U << 10)
#define LOG_RAWREG  (1U << 11)

#define VERBOSE (LOG_REG | LOG_CMD | LOG_DIAL | LOG_ERROR | LOG_SOCKET)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGRAW(...)     LOGMASKED(LOG_RAW,                 __VA_ARGS__)
#define LOGREG(...)     LOGMASKED(LOG_REG,                 __VA_ARGS__)
#define LOGCMD(...)     LOGMASKED(LOG_CMD,                 __VA_ARGS__)
#define LOGERROR(...)   LOGMASKED(LOG_ERROR,               __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,                 __VA_ARGS__)
#define LOGRX(...)      LOGMASKED(LOG_RX,                  __VA_ARGS__)
#define LOGTX(...)      LOGMASKED(LOG_TX,                  __VA_ARGS__)
#define LOGSOCKET(...)  LOGMASKED(LOG_SOCKET,              __VA_ARGS__)
#define LOGRXERR(...)   LOGMASKED(LOG_RX | LOG_ERROR,      __VA_ARGS__)
#define LOGTXERR(...)   LOGMASKED(LOG_TX | LOG_ERROR,      __VA_ARGS__)
#define LOGSOCKERR(...) LOGMASKED(LOG_SOCKET | LOG_ERROR,  __VA_ARGS__)
#define LOGDIAL(...)    LOGMASKED(LOG_DIAL,                __VA_ARGS__)
#define LOGRAWCMD(...)  LOGMASKED(LOG_RAWCMD,              __VA_ARGS__)
#define LOGRAWREG(...)  LOGMASKED(LOG_RAWREG,              __VA_ARGS__)


DEFINE_DEVICE_TYPE(GRID_MODEM, grid_modem_device, "grid_modem", "GRiD Compass Modem")

ALLOW_SAVE_TYPE(grid_modem_device::protocol_state);


class grid_modem_device::network_context
{
	static constexpr size_t TX_QUEUE_LIMIT = RX_QUEUE_SIZE;

public:
	enum class network_event_type : uint8_t
	{
		CONNECTED,
		CONNECT_FAILED,
		DISCONNECTED
	};

	struct network_event
	{
		network_event_type type;
		std::error_code error;
	};

	network_context(std::string host, uint16_t port, std::chrono::nanoseconds write_delay)
		: m_socket(m_ioctx)
		, m_resolver(m_ioctx)
		, m_connect_timeout(m_ioctx)
		, m_write_timer(m_ioctx)
		, m_work_guard(asio::make_work_guard(m_ioctx))
		, m_host(std::move(host))
		, m_port(port)
		, m_write_delay(write_delay)
		, m_next_write_time(std::chrono::steady_clock::now())
	{
	}

	~network_context()
	{
		stop();
	}

	void start()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_connection_state = connection_state::RESOLVING;
		}

		m_thread = std::thread(
				[this] ()
				{
					start_resolve();
					m_ioctx.run();
				});
	}

	void stop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_connection_state = connection_state::STOPPING;
			m_read_state = read_state::IDLE;
			m_write_state = write_state::IDLE;
			m_tx_queue.clear();
		}

		asio::post(
				m_ioctx,
				[this] () { stop_async_operations(); });

		m_work_guard.reset();

		if (m_thread.joinable())
			m_thread.join();

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_connection_state = connection_state::STOPPED;
		}
	}

	bool connected() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_connection_state == connection_state::CONNECTED;
	}

	void set_write_delay(std::chrono::nanoseconds write_delay)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_write_delay = write_delay;
	}

	bool take_event(network_event &event)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_events.empty())
			return false;

		event = m_events.front();
		m_events.pop_front();
		return true;
	}

	size_t take_rx(uint8_t *buffer, size_t capacity)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		const size_t count = std::min(capacity, m_pending_rx.size());
		std::copy_n(m_pending_rx.begin(), count, buffer);
		m_pending_rx.erase(m_pending_rx.begin(), m_pending_rx.begin() + count);
		return count;
	}

	bool has_pending_rx() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return !m_pending_rx.empty();
	}

	void arm_read(size_t capacity)
	{
		if (capacity == 0)
			return;

		const size_t read_size = std::min(capacity, m_read_buffer.size());
		std::unique_lock<std::mutex> lock(m_mutex);
		if ((m_connection_state != connection_state::CONNECTED) ||
				(m_read_state != read_state::IDLE) ||
				!m_pending_rx.empty())
			return;

		m_read_state = read_state::READING;
		lock.unlock();

		asio::post(
				m_ioctx,
				[this, read_size] () { start_socket_read(read_size); });
	}

	bool write_byte(uint8_t data)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_connection_state != connection_state::CONNECTED)
			return false;

		if (m_tx_queue.size() >= TX_QUEUE_LIMIT)
			return false;

		if (m_tx_queue.empty() && (m_write_state == write_state::IDLE))
			m_next_write_time = std::chrono::steady_clock::now();

		m_tx_queue.push_back(data);
		lock.unlock();

		asio::post(m_ioctx, [this] () { start_write(); });
		return true;
	}

private:
	enum class connection_state : uint8_t
	{
		STOPPED,
		RESOLVING,
		CONNECTING,
		CONNECTED,
		STOPPING
	};

	enum class read_state : uint8_t
	{
		IDLE,
		READING
	};

	enum class write_state : uint8_t
	{
		IDLE,
		WRITING,
		DELAYING
	};

	void stop_async_operations()
	{
		std::error_code err;
		m_connect_timeout.cancel(err);
		m_write_timer.cancel(err);
		m_resolver.cancel();
		if (m_socket.is_open())
			m_socket.close(err);
	}

	void start_resolve()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_connection_state != connection_state::RESOLVING)
			return;
		lock.unlock();

		m_resolver.async_resolve(
				m_host,
				std::to_string(m_port),
				[this] (std::error_code err, asio::ip::tcp::resolver::results_type endpoints)
				{ handle_resolve(err, std::move(endpoints)); });
	}

	void handle_resolve(std::error_code err, asio::ip::tcp::resolver::results_type endpoints)
	{
		if (err)
		{
			signal_connect_failed(err);
			return;
		}

		start_connect(endpoints);
	}

	void start_connect(const asio::ip::tcp::resolver::results_type &endpoints)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_connection_state != connection_state::RESOLVING)
			return;

		m_connection_state = connection_state::CONNECTING;
		lock.unlock();

		m_connect_timeout.expires_after(std::chrono::seconds(10));
		m_connect_timeout.async_wait(
				[this] (const std::error_code &err) { handle_connect_timeout(err); });

		asio::async_connect(
				m_socket,
				endpoints,
				[this] (std::error_code err, const asio::ip::tcp::endpoint &endpoint)
				{ handle_connect(err, endpoint); });
	}

	void handle_connect_timeout(const std::error_code &err)
	{
		if (!err && connect_timed_out())
		{
			std::error_code close_err;
			if (m_socket.is_open())
				m_socket.close(close_err);
			signal_connect_failed(asio::error::timed_out);
		}
	}

	void handle_connect(std::error_code err, const asio::ip::tcp::endpoint &)
	{
		std::error_code timer_err;
		m_connect_timeout.cancel(timer_err);

		if (err)
		{
			if (err != asio::error::operation_aborted)
				signal_connect_failed(err);
			return;
		}

		m_socket.set_option(asio::ip::tcp::no_delay(true), err);
		if (err)
		{
			std::error_code close_err;
			if (m_socket.is_open())
				m_socket.close(close_err);
			signal_connect_failed(err);
			return;
		}

		bool close_socket = false;
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_connection_state == connection_state::CONNECTING)
		{
			m_connection_state = connection_state::CONNECTED;
			push_event_locked(network_event_type::CONNECTED, std::error_code());
		}
		else
		{
			close_socket = true;
		}
		lock.unlock();

		if (close_socket)
		{
			std::error_code close_err;
			if (m_socket.is_open())
				m_socket.close(close_err);
		}
	}

	void start_socket_read(size_t read_size)
	{
		if (!m_socket.is_open())
		{
			clear_read_pending();
			signal_disconnected(asio::error::not_connected);
			return;
		}

		m_socket.async_read_some(
				asio::buffer(m_read_buffer.data(), read_size),
				[this] (std::error_code err, std::size_t length)
				{ handle_socket_read(err, length); });
	}

	void handle_socket_read(std::error_code err, std::size_t length)
	{
		if (err || (length == 0))
		{
			clear_read_pending();
			signal_disconnected(err ? err : asio::error::eof);
			return;
		}

		std::lock_guard<std::mutex> lock(m_mutex);
		m_read_state = read_state::IDLE;
		if (m_connection_state == connection_state::CONNECTED)
		{
			m_pending_rx.insert(
					m_pending_rx.end(),
					m_read_buffer.begin(),
					m_read_buffer.begin() + length);
		}
	}

	void start_write()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if ((m_write_state != write_state::IDLE) ||
				m_tx_queue.empty() ||
				(m_connection_state != connection_state::CONNECTED))
			return;

		m_write_byte = m_tx_queue.front();
		m_write_state = write_state::WRITING;
		lock.unlock();

		asio::async_write(
				m_socket,
				asio::buffer(&m_write_byte, sizeof(m_write_byte)),
				[this] (std::error_code err, std::size_t length)
				{ handle_write(err, length); });
	}

	void handle_write(std::error_code err, std::size_t length)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_write_state = write_state::IDLE;

		if (!err && (length == sizeof(m_write_byte)) && !m_tx_queue.empty())
		{
			m_tx_queue.pop_front();
		}
		else
		{
			m_tx_queue.clear();
		}
		lock.unlock();

		if (err || (length != sizeof(m_write_byte)))
			signal_disconnected(err ? err : asio::error::message_size);
		else
			delay_next_write();
	}

	void delay_next_write()
	{
		std::chrono::nanoseconds write_delay;
		std::unique_lock<std::mutex> lock(m_mutex);
		if ((m_write_state != write_state::IDLE) ||
				(m_connection_state != connection_state::CONNECTED))
			return;

		m_write_state = write_state::DELAYING;

		const auto now = std::chrono::steady_clock::now();
		const auto next_write_time = m_next_write_time + m_write_delay;
		m_next_write_time = (next_write_time <= now) ? (now + m_write_delay) : next_write_time;
		write_delay = m_next_write_time - now;
		lock.unlock();

		m_write_timer.expires_after(write_delay);
		m_write_timer.async_wait(
				[this] (const std::error_code &err) { handle_write_delay(err); });
	}

	void handle_write_delay(const std::error_code &err)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_write_state == write_state::DELAYING)
			m_write_state = write_state::IDLE;
		lock.unlock();

		if (!err)
			start_write();
	}

	void clear_read_pending()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_read_state == read_state::READING)
			m_read_state = read_state::IDLE;
	}

	bool connect_timed_out() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_connection_state == connection_state::CONNECTING;
	}

	void signal_connect_failed(std::error_code err)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if ((m_connection_state == connection_state::RESOLVING) ||
				(m_connection_state == connection_state::CONNECTING))
		{
			m_connection_state = connection_state::STOPPED;
			push_event_locked(network_event_type::CONNECT_FAILED, err);
		}
	}

	void signal_disconnected(std::error_code err)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_connection_state != connection_state::CONNECTED)
			return;

		m_connection_state = connection_state::STOPPED;
		m_read_state = read_state::IDLE;
		m_write_state = write_state::IDLE;
		m_tx_queue.clear();
		push_event_locked(network_event_type::DISCONNECTED, err);
		lock.unlock();

		std::error_code close_err;
		if (m_socket.is_open())
			m_socket.close(close_err);
	}

	void push_event_locked(network_event_type type, std::error_code err)
	{
		m_events.push_back(network_event{ type, err });
	}

	mutable std::mutex m_mutex;
	asio::io_context m_ioctx;
	asio::ip::tcp::socket m_socket;
	asio::ip::tcp::resolver m_resolver;
	asio::steady_timer m_connect_timeout;
	asio::steady_timer m_write_timer;
	asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;
	std::thread m_thread;

	const std::string m_host;
	const uint16_t m_port;
	std::chrono::nanoseconds m_write_delay;
	std::chrono::steady_clock::time_point m_next_write_time;

	connection_state m_connection_state = connection_state::STOPPED;
	read_state m_read_state = read_state::IDLE;
	write_state m_write_state = write_state::IDLE;
	std::deque<network_event> m_events;

	std::array<uint8_t, RX_QUEUE_SIZE> m_read_buffer{};
	std::vector<uint8_t> m_pending_rx;
	std::deque<uint8_t> m_tx_queue;
	uint8_t m_write_byte = 0;
};


grid_modem_device::grid_modem_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_int_func(*this)
{
}


grid_modem_device::grid_modem_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: grid_modem_device(mconfig, GRID_MODEM, tag, owner, clock)
{
}


grid_modem_device::~grid_modem_device()
{
}


void grid_modem_device::set_remote_host(const char *host)
{
	m_remote_host = host;
}


void grid_modem_device::set_remote_port(const char *port)
{
	m_remote_port = uint16_t(std::strtoul(port, nullptr, 10));
}


void grid_modem_device::device_start()
{
	m_rx_poll_timer = timer_alloc(FUNC(grid_modem_device::rx_poll_timer), this);
	m_rx_ready_timer = timer_alloc(FUNC(grid_modem_device::rx_ready_timer), this);
	m_tx_ready_timer = timer_alloc(FUNC(grid_modem_device::tx_ready_timer), this);

	save_item(NAME(m_protocol_state));

	save_item(NAME(m_selected_reg));
	save_item(NAME(m_requested_status));
	save_item(NAME(m_return_value));
	save_item(NAME(m_command));
	save_item(NAME(m_bus_status));

	save_item(NAME(m_internal_regs));

	save_item(NAME(m_control_byte));
	save_item(NAME(m_raw_reset_pending));

	save_item(NAME(m_off_hook));
	save_item(NAME(m_carrier));
	save_item(NAME(m_online));
	save_item(NAME(m_rx_ready));
	save_item(NAME(m_tx_ready));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_rx_ready_time));
	save_item(NAME(m_tx_ready_time));

	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_status2));

	save_item(NAME(m_rx_queue));
	save_item(NAME(m_rx_queue_count));
}


void grid_modem_device::device_stop()
{
	stop_socket();
}


void grid_modem_device::device_reset()
{
	reset_hle_state();
}


void grid_modem_device::device_post_load()
{
	if (m_irq_pending)
		m_int_func(ASSERT_LINE);
	else
		m_int_func(CLEAR_LINE);
}


void grid_modem_device::reset_hle_state()
{
	m_protocol_state = protocol_state::IDLE;

	m_selected_reg = 0;
	m_requested_status = 0;
	m_return_value = 0;
	m_command = 0;

	m_bus_status = C4_READY;

	for (uint8_t &value : m_internal_regs)
		value = 0;

	m_control_byte = 0;
	m_raw_reset_pending = false;

	m_off_hook = false;
	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;
	m_baud_rate = 1200;
	reset_serial_timing();

	if (m_rx_poll_timer != nullptr)
		m_rx_poll_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	stop_socket();
	clear_rx_queue();

	clear_irq();
}


uint8_t grid_modem_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0: // DFE0:C0: data/result
		data = m_return_value;
		m_bus_status &= ~C4_DATA_READY;
		break;

	case 1: // DFE0:C2: control / DTMF
		data = m_control_byte;
		break;

	case 2: // DFE0:C4: bus handshake status
		data = m_bus_status;
		break;

	default:
		data = 0xff;
		break;
	}

	LOGRAW("%02x == %02x\n", offset, data);

	return data;
}


void grid_modem_device::write(offs_t offset, uint8_t data)
{
	LOGRAW("%02x <- %02x\n", offset, data);

	switch (offset)
	{
	case 0: // DFE0:C0: data/result
		data_port_w(data);
		break;

	case 1: // DFE0:C2: control / DTMF
		m_control_byte = data;

		if (data == 0x7f)
			LOGDIAL("DTMF/control off\n");
		else
			LOGDIAL("DTMF/control <- %02x\n", data);
		break;

	case 2: // DFE0:C4: bus handshake status
		LOGERROR("ignored write to DFE0:C4: %02x\n", data);
		break;

	case 3: // DFE0:C6: command/protocol control
		command_port_w(data);
		break;

	default:
		LOGERROR("invalid write offset %x <- %02x\n", offset, data);
		break;
	}
}


void grid_modem_device::command_port_w(uint8_t data)
{
	switch (data)
	{
	case 0x00:
		// Seen directly before handshake and after hang-up.
		LOGRAWCMD("direct DFE0:C6=00\n");
		break;

	case 0x01:
		// Seen in a carrier-enable/disable-related path.
		LOGRAWCMD("direct DFE0:C6=01\n");
		break;

	case 0x03:
		if (m_raw_reset_pending)
		{
			LOGCMD("reset sequence complete\n");
			m_raw_reset_pending = false;
		}
		else
		{
			// Direct C6=3 is distinct from command 3 executed via C6=5, C0=3, C6=4.
			LOGRAWCMD("direct DFE0:C6=03\n");
		}
		break;

	case 0x04:
		commit_protocol_transaction();
		break;

	case 0x05:
		begin_protocol_transaction();
		break;

	case 0xc0:
		// Raw reset sequence observed in the DOS driver:
		//
		//   DFE0:C6 = 0xc0
		//   DFE0:C2 = 0x7f
		//   DFE0:C6 = 0x03
		LOGRAWCMD("raw reset arm\n");

		reset_hle_state();
		m_raw_reset_pending = true;
		break;

	default:
		LOGERROR("unhandled direct write %02x to DFE0:C6\n", data);
		break;
	}
}


void grid_modem_device::data_port_w(uint8_t data)
{
	switch (m_protocol_state)
	{
	case protocol_state::IDLE:
		LOGERROR("unexpected write %02x to DFE0:C0 while idle\n", data);
		break;

	case protocol_state::EXPECT_SELECTOR:
		if ((data & 0xc0) == 0xc0)
		{
			m_requested_status = data & 0x03;
			m_protocol_state = protocol_state::READ_STATUS_SELECTED;

			LOGRAWREG("selected status index %u\n", m_requested_status);
		}
		else if ((data & 0x80) != 0)
		{
			m_selected_reg = data & 0x1f;
			m_protocol_state = protocol_state::WRITE_REGISTER_SELECTED;

			LOGRAWREG("selected internal register %u\n", m_selected_reg);
		}
		else
		{
			m_command = data;
			m_protocol_state = protocol_state::EXEC_COMMAND_SELECTED;

			if (m_command == 5)
				LOGIRQ("selected irq command 5\n");
			else
				LOGCMD("selected modem command %u (0x%02x)\n", m_command, m_command);
		}

		m_bus_status = C4_READY;
		break;

	case protocol_state::WRITE_REGISTER_SELECTED:
		LOGERROR("got C0=%02x before C6=04 register-selection commit\n", data);
		break;

	case protocol_state::WRITE_REGISTER_VALUE:
		write_internal_register(m_selected_reg, data);
		m_protocol_state = protocol_state::IDLE;
		m_bus_status = C4_READY;
		break;

	case protocol_state::READ_STATUS_SELECTED:
		LOGERROR("unexpected C0=%02x during read-status transaction\n", data);
		break;

	case protocol_state::EXEC_COMMAND_SELECTED:
		LOGERROR("unexpected C0=%02x during execute-command transaction\n", data);
		break;
	}
}


void grid_modem_device::begin_protocol_transaction()
{
	m_protocol_state = protocol_state::EXPECT_SELECTOR;
	m_bus_status = C4_READY;
}


void grid_modem_device::commit_protocol_transaction()
{
	switch (m_protocol_state)
	{
	case protocol_state::READ_STATUS_SELECTED:
		m_return_value = read_status_index(m_requested_status);
		m_bus_status = C4_READY | C4_DATA_READY;
		m_protocol_state = protocol_state::IDLE;
		LOGRAWREG("read status index %u -> %02x\n", m_requested_status, m_return_value);
		break;

	case protocol_state::WRITE_REGISTER_SELECTED:
		m_protocol_state = protocol_state::WRITE_REGISTER_VALUE;
		m_bus_status = C4_READY;
		break;

	case protocol_state::EXEC_COMMAND_SELECTED:
		execute_modem_command(m_command);
		m_protocol_state = protocol_state::IDLE;
		m_bus_status = C4_READY;
		break;

	case protocol_state::IDLE:
		LOGERROR("unexpected C6=04 while protocol is idle\n");
		break;

	case protocol_state::EXPECT_SELECTOR:
		LOGERROR("unexpected C6=04 before command/selector\n");
		break;

	case protocol_state::WRITE_REGISTER_VALUE:
		LOGERROR("unexpected C6=04 while waiting for register value\n");
		break;
	}
}


uint8_t grid_modem_device::read_status_index(uint8_t index)
{
	switch (index & 0x03)
	{
	case 0:
		process_network_events();
		return dequeue_rx_byte();

	case 1:
		return read_status1();

	case 2:
		return read_status2();

	case 3:
		return STATUS3_MODEM_VERSION;
	}

	return 0x00;
}


uint8_t grid_modem_device::read_status1() const
{
	uint8_t result = STATUS1_OPERATION_COMPLETE;

	if (m_off_hook)
		result |= STATUS1_LINE_READY;

	if (m_carrier)
		result |= STATUS1_CARRIER_DETECTED;

	return result;
}


uint8_t grid_modem_device::read_status2()
{
	process_network_events();

	if (m_irq_pending)
		return m_irq_status2;

	uint8_t result = 0;
	if (m_tx_ready)
		result |= STATUS2_TX_READY;

	if (!rx_queue_empty())
		result |= STATUS2_RX_BYTE_AVAILABLE;

	return result;
}


void grid_modem_device::write_internal_register(uint8_t reg, uint8_t data)
{
	if (reg >= sizeof(m_internal_regs))
	{
		LOGERROR("write %02x to unknown internal register %02x\n",
			data, reg);
		return;
	}

	m_internal_regs[reg] = data;

	switch (reg)
	{
	case 0:
		log_tx_byte(data);
		break;

	case 1:
		handle_reg_1_write(data);
		break;

	case 2:
		log_reg_2_write(data);
		break;

	case 6:
		handle_reg_6_write(data);
		break;

	case 7:
		handle_reg_7_write(data);
		break;

	default:
		LOGREG("internal register %u <- %02x\n", reg, data);
		break;
	}
}


void grid_modem_device::handle_reg_6_write(uint8_t data)
{
	// Driver-observed values:
	//
	// 0x00 before outgoing off-hook
	// 0x10 before handshake
	// 0x80 after carrier detect
	// 0x08 after hang-up
	if (data == 0x80)
		enter_online_data_mode();
	else if ((data == 0x00) || (data == 0x08))
		leave_online_data_mode();

	LOGREG("internal register 6 <- %02x\n", data);
}


void grid_modem_device::handle_reg_1_write(uint8_t data)
{
	const bool is_300_baud = (data & 0x04) != 0;
	const bool high_speed_delay = (data & 0x20) != 0;

	m_baud_rate = is_300_baud ? 300 : 1200;
	if (m_network)
		m_network->set_write_delay(std::chrono::nanoseconds(socket_byte_delay_nsec()));

	LOGREG("register 1 <- %02x: %u baud, high-speed-delay=%u, upper=%02x\n",
		data,
		m_baud_rate,
		high_speed_delay ? 1 : 0,
		data & 0xc0);
}


void grid_modem_device::handle_reg_7_write(uint8_t data)
{
	// Driver-observed values:
	//
	// 0x00 during hang-up
	// 0x04 after carrier detection
	// 0x06 in GRiD-OS before data mode; HLE treats bit 0x02 as
	// transmit-ready interrupt enable.
	//
	// MS-DOS uses 0x04 and polls
	// status 2 bit 0x02 instead of enabling the interrupt.
	if (data == 0x00)
	{
		leave_online_data_mode();
	}
	else if (((data & 0x02) != 0) && m_tx_ready)
	{
		schedule_tx_ready_event(attotime::from_msec(1));
	}

	LOGREG("internal register 7 <- %02x\n", data);
}


void grid_modem_device::enter_online_data_mode()
{
	if (!socket_connected())
	{
		LOGSOCKERR("cannot enter online data mode without socket connection\n");
		m_online = false;
		return;
	}

	m_online = true;
	m_rx_ready = true;
	m_tx_ready = false;
	reset_serial_timing();
	process_network_events();
	if (!rx_queue_empty() && !m_irq_pending)
		raise_rx_event();

	arm_socket_rx();
	schedule_tx_ready_event(attotime::from_msec(1));
}


void grid_modem_device::leave_online_data_mode()
{
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;
	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();
}


void grid_modem_device::execute_modem_command(uint8_t command)
{
	switch (command)
	{
	case 0x00:
		LOGCMD("command 0: idle/hang-up cleanup\n");
		break;

	case 0x01:
		execute_off_hook_command();
		break;

	case 0x02:
		execute_hang_up_command();
		break;

	case 0x03:
	case 0x04:
		execute_handshake_command(command);
		break;

	case 0x05:
		acknowledge_irq();
		LOGIRQ("command 5: IRQ acknowledged\n");
		break;

	case 0x08:
		m_off_hook = true;
		LOGCMD("command 8: line-preparation/off-hook HLE\n");
		break;

	default:
		if ((command >= 0x41) && (command <= 0x4a))
			execute_pulse_dial_command(command);
		else
		{
			LOGERROR("command %02x: unhandled\n", command);
		}
		break;
	}
}


void grid_modem_device::execute_off_hook_command()
{
	// Go off-hook.
	stop_socket();

	m_off_hook = true;
	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	clear_rx_queue();
	clear_irq();

	LOGCMD("command 1: off-hook complete\n");
}


void grid_modem_device::execute_hang_up_command()
{
	// Begin hang-up / return on-hook.
	stop_socket();

	m_off_hook = false;
	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	clear_rx_queue();
	clear_irq();

	LOGCMD("command 2: on-hook/hang-up complete\n");
}


void grid_modem_device::execute_handshake_command(uint8_t command)
{
	// Drivers normally uses:
	//   command 3 -> normal outgoing post-dial handshake
	//   command 4 -> alternate/answer-like handshake path
	//
	// HLE intentionally treats both as socket connection attempts.
	// Carrier is asserted only after the remote socket connects.
	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	if (!m_off_hook)
		LOGCMD("command %u: handshake without explicit off-hook\n", command);
	else
		LOGCMD("command %u: scheduling HLE carrier detect\n", command);

	start_socket_connect();
}


void grid_modem_device::execute_pulse_dial_command(uint8_t command)
{
	const unsigned pulses = command & 0x0f;
	LOGDIAL("pulse dial: %u pulse%s\n", pulses, (pulses == 1) ? "" : "s");
}


void grid_modem_device::raise_line_event()
{
	m_irq_pending = true;
	m_irq_status2 = STATUS2_LINE_EVENT;

	m_int_func(ASSERT_LINE);
}


void grid_modem_device::raise_rx_event()
{
	m_irq_pending = true;
	m_irq_status2 = STATUS2_RX_BYTE_AVAILABLE;

	m_int_func(ASSERT_LINE);
}


void grid_modem_device::raise_tx_event()
{
	m_tx_ready = true;
	m_irq_pending = true;
	m_irq_status2 = STATUS2_TX_READY;

	LOGIRQ("TX ready IRQ raised\n");
	m_int_func(ASSERT_LINE);
}


attotime grid_modem_device::serial_byte_delay() const
{
	return attotime::from_ticks(SERIAL_FRAME_BITS, m_baud_rate);
}


u64 grid_modem_device::socket_byte_delay_nsec() const
{
	return (1'000'000'000ULL * SERIAL_FRAME_BITS + m_baud_rate - 1) / m_baud_rate;
}


attotime grid_modem_device::next_serial_delay(attotime &ready_time) const
{
	const attotime now = machine().time();
	const attotime next_ready_time = ready_time + serial_byte_delay();

	ready_time = (next_ready_time <= now) ? (now + serial_byte_delay()) : next_ready_time;
	return ready_time - now;
}


void grid_modem_device::reset_serial_timing()
{
	const attotime now = machine().time();
	m_rx_ready_time = now;
	m_tx_ready_time = now;
}


void grid_modem_device::schedule_rx_ready_event(attotime delay)
{
	if ((m_rx_ready_timer != nullptr) && m_online)
		m_rx_ready_timer->adjust(delay);
}


void grid_modem_device::schedule_tx_ready_event(attotime delay)
{
	if ((m_tx_ready_timer != nullptr) && m_online)
		m_tx_ready_timer->adjust(delay);
}


void grid_modem_device::acknowledge_irq()
{
	m_irq_pending = false;
	m_irq_status2 = m_tx_ready ? STATUS2_TX_READY : 0;
	m_int_func(CLEAR_LINE);

	process_network_events();

	if (m_online && !rx_queue_empty())
		raise_rx_event();
}


void grid_modem_device::clear_irq()
{
	m_irq_pending = false;
	m_irq_status2 = m_tx_ready ? STATUS2_TX_READY : 0;

	m_int_func(CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(grid_modem_device::rx_poll_timer)
{
	process_network_events();

	if (m_online && !m_irq_pending && !rx_queue_empty())
		raise_rx_event();
}


TIMER_CALLBACK_MEMBER(grid_modem_device::rx_ready_timer)
{
	if (!m_online)
		return;

	m_rx_ready = true;
	process_network_events();

	if (!m_irq_pending && !rx_queue_empty())
		raise_rx_event();
}


TIMER_CALLBACK_MEMBER(grid_modem_device::tx_ready_timer)
{
	if (!m_online)
		return;

	m_tx_ready = true;

	if ((m_internal_regs[7] & 0x02) == 0)
		return;

	if (m_irq_pending)
	{
		m_tx_ready_timer->adjust(attotime::from_msec(1));
		return;
	}

	raise_tx_event();
}


void grid_modem_device::log_tx_byte(uint8_t data)
{
	if (!m_online)
		LOGTXERR("TX while modem is not marked online\n");

	if (!m_tx_ready)
		LOGTXERR("TX while transmitter is not ready\n");

	send_tx_byte(data);
	if (m_online)
	{
		m_tx_ready = false;
		schedule_tx_ready_event(next_serial_delay(m_tx_ready_time));
	}

	if ((data >= 0x20) && (data <= 0x7e))
		LOGTX("TX '%c' / 0x%02x\n", char(data), data);
	else
		LOGTX("TX 0x%02x\n", data);
}


void grid_modem_device::start_socket_connect()
{
	stop_socket();

	LOGSOCKET("socket connecting to %s:%u\n", m_remote_host.c_str(), unsigned(m_remote_port));
	m_network = std::make_unique<network_context>(
			m_remote_host,
			m_remote_port,
			std::chrono::nanoseconds(socket_byte_delay_nsec()));
	m_network->start();
}


void grid_modem_device::stop_socket()
{
	if (m_network)
	{
		LOGSOCKET("socket closed\n");
		m_network.reset();
	}
}


bool grid_modem_device::socket_connected() const
{
	return m_network && m_network->connected();
}


void grid_modem_device::process_network_events()
{
	if (!m_network)
		return;

	network_context::network_event event;
	while (m_network->take_event(event))
	{
		switch (event.type)
		{
		case network_context::network_event_type::CONNECTED:
			handle_socket_connected();
			break;

		case network_context::network_event_type::CONNECT_FAILED:
			handle_socket_connect_failed(event.error.message());
			break;

		case network_context::network_event_type::DISCONNECTED:
			handle_socket_disconnected(event.error.message());
			break;
		}
	}

	if (m_online && m_rx_ready && !rx_queue_full())
	{
		uint8_t rx_byte = 0;
		if (m_network->take_rx(&rx_byte, 1) != 0)
		{
			queue_rx_byte(rx_byte);
			m_rx_ready = false;
			schedule_rx_ready_event(next_serial_delay(m_rx_ready_time));
		}
	}

	arm_socket_rx();
}


void grid_modem_device::handle_socket_connected()
{
	LOGSOCKET("socket connected to %s:%u\n", m_remote_host.c_str(), unsigned(m_remote_port));

	m_carrier = true;
	raise_line_event();
	arm_socket_rx();

	LOGIRQ("carrier detected, IRQ raised\n");
}


void grid_modem_device::handle_socket_connect_failed(const std::string &message)
{
	LOGSOCKERR("socket connect to %s:%u failed: %s\n",
			m_remote_host.c_str(),
			unsigned(m_remote_port),
			message.c_str());

	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	raise_line_event();
}


void grid_modem_device::handle_socket_disconnected(const std::string &message)
{
	LOGSOCKERR("socket disconnected: %s\n", message.c_str());

	m_carrier = false;
	m_online = false;
	m_rx_ready = true;
	m_tx_ready = true;

	if (m_rx_ready_timer != nullptr)
		m_rx_ready_timer->reset();

	if (m_tx_ready_timer != nullptr)
		m_tx_ready_timer->reset();

	raise_line_event();
}


void grid_modem_device::arm_socket_rx()
{
	if (!m_online || !m_rx_ready || !m_network || m_network->has_pending_rx())
		return;

	// The poll timer will retry shortly; do not pull another TCP byte into HLE storage.
	if (rx_queue_full())
		return;

	m_network->arm_read(1);
}


void grid_modem_device::send_tx_byte(uint8_t data)
{
	if (!m_network || !m_network->write_byte(data))
		LOGTXERR("TX while socket is not connected or TX queue is full\n");
}


void grid_modem_device::queue_rx_byte(uint8_t data)
{
	if (rx_queue_full())
	{
		LOGRXERR("RX queue full, dropping 0x%02x\n", data);
		return;
	}

	m_rx_queue[m_rx_queue_count] = data;
	m_rx_queue_count++;

	if ((data >= 0x20) && (data <= 0x7e))
		LOGRX("RX queued '%c' / 0x%02x\n", char(data), data);
	else
		LOGRX("RX queued 0x%02x\n", data);

	if (m_online && !m_irq_pending)
		raise_rx_event();
}


uint8_t grid_modem_device::dequeue_rx_byte()
{
	if (rx_queue_empty())
	{
		process_network_events();
		return 0x00;
	}

	const uint8_t result = m_rx_queue[0];
	if (m_rx_queue_count > 1)
		std::move(&m_rx_queue[1], &m_rx_queue[m_rx_queue_count], &m_rx_queue[0]);
	m_rx_queue_count--;

	if ((result >= 0x20) && (result <= 0x7e))
		LOGRX("RX delivered '%c' / 0x%02x\n", char(result), result);
	else
		LOGRX("RX delivered 0x%02x\n", result);

	process_network_events();
	return result;
}


bool grid_modem_device::rx_queue_empty() const
{
	return m_rx_queue_count == 0;
}


bool grid_modem_device::rx_queue_full() const
{
	return m_rx_queue_count >= RX_QUEUE_SIZE;
}


void grid_modem_device::clear_rx_queue()
{
	m_rx_queue_count = 0;
}


void grid_modem_device::log_reg_2_write(uint8_t data)
{
	const bool use_2_stop_bits = (data & 0x04) != 0;
	const uint8_t data_bits = data & 0x03;
	const uint8_t parity = data & 0x38;

	const char *parity_human = nullptr;

	switch (parity)
	{
	case 0x30:
		parity_human = "Space";
		break;

	case 0x38:
		parity_human = "Mark";
		break;

	case 0x10:
		parity_human = "Even";
		break;

	case 0x18:
		parity_human = "Odd";
		break;

	case 0x00:
		parity_human = "None";
		break;

	default:
		parity_human = "Unknown";
		break;
	}

	LOGREG("register 2 <- %02x: data=%u, stop=%u, parity=%s (%02x)\n",
		data,
		data_bits + 5,
		use_2_stop_bits ? 2 : 1,
		parity_human,
		parity);
}
