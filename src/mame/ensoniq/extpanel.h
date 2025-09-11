// license:BSD-3-Clause
// copyright-holders:C. Brunschen
#ifndef MAME_ENSONIQ_EXTPANEL_H
#define MAME_ENSONIQ_EXTPANEL_H

#pragma once

#include "emumem.h"

#include "asio.h"

#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <variant>

class esq_external_panel_device : public device_t
{
public:
  struct Command {
    enum Kind {
      ButtonKind = 1,
      AnalogValueKind = 2,
      SendKind = 3
    };

    enum Kind kind;

    union {
      struct {
        uint8_t button;
        bool pressed;
      };

      struct {
        uint8_t channel;
        uint16_t value;
      };
    };

    static Command Button(uint8_t b, bool p) { Command c { ButtonKind }; c.button = b; c.pressed = p; return c; }
    static Command AnalogValue(uint8_t o, uint16_t v) { Command c { AnalogValueKind }; c.channel = o; c.value = v; return c; }
    static Command Send() { Command c { SendKind }; return c; }
  };

  esq_external_panel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
  virtual ~esq_external_panel_device();

  void set_keyboard(const std::string &keyboard);
  void set_version(const std::string &version);

  bool is_running() { return m_context.operator bool(); }

  void clear_display();
  void set_char(uint8_t row, uint8_t column, uint8_t c, uint8_t attr);
  void set_light(uint8_t light, uint8_t state);
  void button_down(uint8_t button);
  void button_up(uint8_t button);
  void set_button(uint8_t button, bool pressed);
  void set_analog_value(uint8_t channel, int value);
  void set_blink_phase(uint8_t phase);

  bool get_next_command(Command &c) { 
    std::lock_guard lock(m_mutex);
    if (m_commands.empty())
      return false; 

    c = std::move(m_commands.front());
    m_commands.pop_front();
    return true;
  }

  // bulk setters
  void set_display_contents(std::vector<std::pair<uint8_t, uint8_t>> &chars_and_attrs);
  void set_button_states(std::vector<std::pair<uint8_t, bool>> &button_states);
  void set_light_states(std::vector<std::pair<uint8_t, uint8_t>> &light_states);
  void set_analog_values(std::vector<std::pair<uint8_t, uint16_t>> &analog_values);

  static constexpr size_t RX_BUF_SIZE = 64;
  static constexpr uint8_t RX_AWAIT_STX = 0;
  static constexpr uint8_t RX_COLLECT = 1;
  static constexpr uint8_t RX_COLLECT_ESCAPED = 2;

  class context;
  friend class context;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

  void send(const std::string &s);
  void enqueue_command(Command &&command)
  {
    std::lock_guard lock(m_mutex); 
    m_commands.emplace_back(std::move(command));
  }
  void enqueue_button(uint8_t button, bool pressed) { enqueue_command(Command::Button(button, pressed)); }
  void enqueue_analog_value(uint8_t channel, uint16_t value) { enqueue_command(Command::AnalogValue(channel, value)); }
  void enqueue_send() { enqueue_command(Command::Send()); }

  void client_connected() { std::lock_guard lock(m_mutex); m_connected = true; }
  void client_disconnected() { std::lock_guard lock(m_mutex); m_connected = false; }

  template<typename A>
  void handle_received_data(const A& a, size_t length) { for (size_t i = 0; i < length; i++) handle_received_data(a[i]); }
  void handle_received_data(uint8_t c);
  void handle_received_message(const std::string &message);

private:
  void start_network();

  template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"[%s] %s",
				tag(),
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

  std::recursive_mutex m_mutex;

  std::string m_keyboard = "unknown";
  std::string m_version = "0";

  std::list<Command> m_commands;

  uint8_t m_rx_buf[RX_BUF_SIZE];
  size_t m_received = 0;
  bool m_rx_overflow = false;
  std::unique_ptr<context> m_context;
  bool m_connected = false;
};

// device type definition
DECLARE_DEVICE_TYPE(ESQ_EXTERNAL_PANEL_DEVICE, esq_external_panel_device)

#endif // MAME_ENSONIQ_EXTPANEL_H
