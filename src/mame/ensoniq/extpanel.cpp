// license:BSD-3-Clause
// copyright-holders:Vas Crabb
// copyright-holders:C. Brunschen
//
// esq_external_panel_device::context is heavily based
// on cps2_comm_device::context by Vas Crabb.

#include "emu.h"
#include "extpanel.h"
#include "emuopts.h"

#include "asio.h"

#include <memory>
#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

class esq_external_panel_device::context {
public:
  context(
      esq_external_panel_device &device,
      std::optional<asio::ip::tcp::endpoint> const &endpoint) :
    m_device(device),
    m_acceptor(m_ioctx),
    m_socket(m_ioctx),
    m_endpoint(endpoint),
    m_stopping(false)
  {
  }

  std::error_code start()
  {
    std::error_code err;

    if (m_endpoint)
    {
      m_acceptor.open(m_endpoint->protocol(), err);
      if (!err)
      {
        asio::ip::tcp::acceptor::reuse_address option(true);
        m_acceptor.set_option(option);
      }
      if (!err)
        m_acceptor.bind(*m_endpoint, err);
      if (!err)
        m_acceptor.listen(1, err);
      if (err)
        return err;
    }

    m_thread = std::thread(
        [this] ()
        {
          start_accept();
          m_ioctx.run();
          LOG("Network thread completed\n");
        });

    return err;
  }

  void stop()
  {
    m_ioctx.post(
        [this] ()
        {
          m_stopping = true;
          std::error_code err;
          if (m_acceptor.is_open())
            m_acceptor.close(err);
          if (m_socket.is_open())
            m_socket.close(err);
        });
    m_thread.join();
  }

  void send_with_crlf(const std::string &s)
  {
    m_ioctx.post(
        [this, s] ()
        {
          if (m_socket.is_open())
          {
            bool const sending = !m_send_buf.empty();
            m_send_buf.append(s.data(), s.length());
            m_send_buf.append("\r\n", 2);
            if (!sending)
            {
              start_send();
            }
          }
        });
  }

private:
  template <typename Format, typename... Params>
  void logerror(Format &&fmt, Params &&... args) const
  {
    util::stream_format(
        std::cerr,
        "[%s] %s",
        m_device.tag(),
        util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
  }

  void start_accept()
  {
    LOG("Accepting external panel connection on %s\n", *m_endpoint);
    m_acceptor.async_accept(
        [this] (std::error_code const &err, asio::ip::tcp::socket sock)
        {
          if (err)
          {
            LOG("Error accepting external panel connection: %s\n", err.message());
            if (!m_stopping)
              start_accept();
          }
          else
          {
            LOG("Accepted external panel connection from %s\n", sock.remote_endpoint());
            m_socket = std::move(sock);
            m_device.client_connected();
            start_receive();
          }
        });
  }

  void start_receive()
  {
    m_socket.async_read_some(
        asio::buffer(m_receive_buf),
        [this] (std::error_code const &err, std::size_t length)
        {
          if (err || !length)
          {
            if (err)
              LOG("Error receiving from external connection: %s\n", err.message());
            else
              LOG("External panel connection lost\n");
            m_send_buf.clear();
            m_device.client_disconnected();
            if (!m_stopping)
            {
              std::error_code e;
              m_socket.close(e);
              start_accept();
            }
          }
          else
          {
            m_device.handle_received_data(m_receive_buf, length);
            start_receive();
          }
        });
  }

  template<size_t size>
  class buffer
  {
  public:
    std::size_t append(const char *src, std::size_t len)
    {
      std::size_t used = 0U;
      if (!m_full)
      {
        if (m_wp >= m_rp)
        {
          used = std::min<std::size_t>(m_buf.size() - m_wp, len);
          std::copy_n(&src[0], used, &m_buf[m_wp]);
          m_wp = (m_wp + used) % m_buf.size();
        }
        std::size_t const block = std::min<std::size_t>(len - used, m_rp - m_wp);
        if (block)
        {
          std::copy_n(&src[used], block, &m_buf[m_wp]);
          used += block;
          m_wp += block;
        }
        m_full = m_wp == m_rp;
      }
      return used;
    }

    std::size_t consume(std::size_t len)
    {
      std::size_t const delta = std::min<std::size_t>(
          m_full ? m_buf.size() : ((m_wp + m_buf.size() - m_rp) % m_buf.size()),
          len);
      m_rp = (m_rp + delta) % m_buf.size();
      if (delta)
        m_full = false;
      return delta;
    }

    auto content() const
    {
      return asio::buffer(
          &m_buf[m_rp],
          ((m_full || (m_wp < m_rp)) ? m_buf.size() : m_wp) - m_rp);
    }

    void clear() { m_wp = m_rp = 0U; }

    bool empty() const { return !m_full && (m_wp == m_rp); }

  private:
    unsigned m_wp = 0U, m_rp = 0U;
    bool m_full = false;
    std::array<u8, size> m_buf;
  };

  void start_send()
  {
    m_socket.async_write_some(
        m_send_buf.content(),
        [this] (std::error_code const &err, std::size_t length)
        {
          m_send_buf.consume(length);
          if (err)
          {
            LOG("Error sending to external panel connection: %s\n", err.message().c_str());
            m_device.client_disconnected();
            m_send_buf.clear();
            m_socket.close();
          }
          else if (!m_send_buf.empty())
          {
            start_send();
          }
        });
  }

  esq_external_panel_device &m_device;
  std::thread m_thread;
  asio::io_context m_ioctx;
  asio::ip::tcp::acceptor m_acceptor;
  asio::ip::tcp::socket m_socket;
  std::optional<asio::ip::tcp::endpoint> const m_endpoint;
  bool m_stopping;
  buffer<4096> m_send_buf;
  std::array<char, 128> m_receive_buf;
};

#define MT_ANALOG 'A'
#define MT_BUTTON 'B'
#define MT_CONTROL 'C'
#define MT_DISPLAY 'D'
#define MT_INFO 'I'

DEFINE_DEVICE_TYPE(ESQ_EXTERNAL_PANEL, esq_external_panel_device, "esq_external_panel", "Ensoniq External_Panel")

esq_external_panel_device::esq_external_panel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
  device_t(mconfig, ESQ_EXTERNAL_PANEL, tag, owner, clock)
{
}

esq_external_panel_device::~esq_external_panel_device()
{
}

void esq_external_panel_device::device_start()
{
}

void esq_external_panel_device::device_add_mconfig(machine_config &config)
{
}

/**
 * This must only be called with the mutex locked, and after checking that m_connected == true.
 */
void esq_external_panel_device::send(const std::string &s)
{
  m_context->send_with_crlf(s);
}

void esq_external_panel_device::clear_display()
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send("DX");
}

void esq_external_panel_device::set_keyboard(const std::string &keyboard)
{
  std::lock_guard lock(m_mutex);
  m_keyboard = keyboard;
}

void esq_external_panel_device::set_version(const std::string &version)
{
  std::lock_guard lock(m_mutex);
  m_version = version;
}


void esq_external_panel_device::set_char(uint8_t row, uint8_t column, uint8_t c, uint8_t attr)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("DC %d %d %02x %1x", row, column, c, attr));
}

void esq_external_panel_device::set_light(uint8_t light, uint8_t state) 
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("L %d %d", light, state));
}

void esq_external_panel_device::button_down(uint8_t button)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("BD %d", button));
}

void esq_external_panel_device::button_up(uint8_t button)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("BU %d", button));
}

void esq_external_panel_device::set_button(uint8_t button, bool pressed)
{ 
  if (pressed)
  {
    button_down(button);
  }
  else
  {
    button_up(button);
  }
}

void esq_external_panel_device::set_analog_value(uint8_t channel, int value)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("A %d %d", channel, value));
}

void esq_external_panel_device::set_blink_phase(uint8_t phase)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;
  send(util::string_format("P %d", phase));
}

void esq_external_panel_device::set_display_contents(std::vector<std::pair<uint8_t, uint8_t>> &chars_and_attrs)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;

  std::ostringstream os;
  os << "DC 0 0";
  for (auto ca : chars_and_attrs)
  {
    util::stream_format(os, " %02x %1x", ca.first, ca.second);
  }

  send(os.str());
}

void esq_external_panel_device::set_button_states(std::vector<std::pair<uint8_t, bool>> &button_states)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;

  std::ostringstream ups;
  ups << "BU";
  std::ostringstream downs;
  downs << "BD";
  for (auto bs : button_states)
  {
    util::stream_format(bs.second ? downs : ups, " %d", bs.first);
  }
  send(ups.str());
  send(downs.str());
}

void esq_external_panel_device::set_light_states(std::vector<std::pair<uint8_t, uint8_t>> &light_states)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;

  std::ostringstream os;
  os << "L";
  for (auto ls : light_states)
  {
    util::stream_format(os, " %d %d", ls.first, ls.second);
  }
  send(os.str());
}

void esq_external_panel_device::set_analog_values(std::vector<std::pair<uint8_t, uint16_t>> &analog_values)
{
  std::lock_guard lock(m_mutex);
  if (!m_connected) return;

  std::ostringstream os;
  os << "A";
  for (auto av : analog_values)
  {
    util::stream_format(os, " %d %d", av.first, av.second);
  }
  send(os.str());
}

void esq_external_panel_device::handle_received_data(uint8_t c) {
  // LOG("Received %02x, overflow=%d\r\n", c, m_rx_overflow);
  if (m_rx_overflow) 
  {
    if (c == '\n')
    {
      // LOG("Found end of overflowing message, restarting.\r\n");
      m_received = 0;
      m_rx_overflow = false;
    }
    else
    {
      // LOG("In buffer overflow, ignoring.\r\n");
    }
  }
  else
  {
    switch(c)
    {
      case '\r': // ignore
        // LOG("ignoring CR\r\n");
        break;

      case '\n':
        // LOG("LF, have a message of %d bytes\r\n", m_received);
        // We have a complete message!
        handle_received_message(std::string(m_rx_buf, m_rx_buf + m_received));
        // Now wait for the start of the next message.
        m_received = 0;
        break;
      
      default:
        if (m_received >= RX_BUF_SIZE)
        {
          // LOG("Already have %d of %d characters - buffer overflow!\r\n", m_received, RX_BUF_SIZE);
          m_rx_overflow = true;
        }
        else
        {
          m_rx_buf[m_received++] = c;
          // LOG("accumulated character %02x, now have %d (of max %d)\r\n", c, m_received, RX_BUF_SIZE);
        }
    }
  }
}

void esq_external_panel_device::handle_received_message(const std::string &message)
{
  if (message.empty()) return;

  std::istringstream is(message);
  char mt;
  is >> mt;

  LOG("Handing Message '%s' of type %c\r\n", message.c_str(), mt);

if (mt == MT_CONTROL)
  {
    // For now, simply send all of the current state.
    enqueue_send();
  }
  else if (mt == MT_INFO)
  {
    send(util::string_format("I %s,%s", m_keyboard, m_version));
  }
  else if (mt == MT_BUTTON)
  {
    char ud;
    is >> ud;
    int button;
    is >> button;
    bool pressed = ud == 'D';
    enqueue_button(button, pressed);
  }
  else if (mt == MT_ANALOG)
  {
    int channel, value;
    is >> channel;
    is >> value;
    uint16_t analog_value = (value << 6);
    enqueue_analog_value(channel, analog_value);
  }
  else
  {
    // LOG("Ignoring unknown message '%s'\r\n", message);
  }
}

void esq_external_panel_device::device_reset()
{
  if (!m_context)
  {
    start_network();
  }
}

void esq_external_panel_device::device_stop()
{
  if (m_context)
  {
    m_context->stop();
    m_context.reset();
  }
}

void esq_external_panel_device::start_network()
{
  auto const &opts = mconfig().options();
  std::error_code err;
  std::istringstream parsestr;
  parsestr.imbue(std::locale::classic());

  std::optional<asio::ip::tcp::endpoint> endpoint;
  if (*opts.comm_localhost())
  {
    LOG("trying to start network on '%s'", opts.comm_localhost());
    asio::ip::address const bindaddr = asio::ip::make_address(opts.comm_localhost(), err);
    if (err)
    {
      osd_printf_error("[%s] invalid local IP address %s, disabling communication.\n", tag(), opts.comm_localhost());
      return;
    }

    parsestr.str(opts.comm_localport());
    parsestr.seekg(0, std::ios_base::beg);
    asio::ip::port_type bindport;
    parsestr >> bindport;
    if (!parsestr || !bindport)
    {
      osd_printf_error("[%s] invalid local TCP port %s, disabling communication.\n", tag(), opts.comm_localport());
      return;
    }

    endpoint.emplace(bindaddr, bindport);
  }

  if (!endpoint)
  {
    osd_printf_error("[%s] no TCP addresses configured, disabling communication.\n", tag());
    return;
  }

  auto ctx = std::make_unique<context>(*this, endpoint);
  err = ctx->start();
  if (err)
  {
    osd_printf_error("[%s] error opening/binding sockets, disabling communication (%s).\n", tag(), err.message());
    m_context.reset();
    return;
  }

  m_context = std::move(ctx);
}
