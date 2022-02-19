// license:BSD-3-Clause
// copyright-holders:Ted Green
//*************************************
// iteagle fpga device
//*************************************
#ifndef MAME_MACHINE_ITEAGLE_FPGA_H
#define MAME_MACHINE_ITEAGLE_FPGA_H

#include "machine/pci.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"
#include "screen.h"

// Functional emulation of AMD AM85C30 serial controller
// Two channels, A & B
class iteagle_am85c30
{
public:
	void reset();
	void write_control(uint8_t data, int channel);
	uint8_t read_control(int channel);
	void write_data(uint8_t data, int channel);
	uint8_t read_data(int channel);
	void write_rx_str(int channel, std::string resp);
	std::string get_tx_str(int channel) { return m_serial_tx[channel]; }
	void clear_tx_str(int channel) { m_serial_tx[channel].clear(); }
	bool check_interrupt() { return (m_rr_regs[0][3] != 0); }
private:
	uint8_t m_rr_regs[2][16];
	uint8_t m_wr_regs[2][16];
	std::string m_serial_tx[2];
	std::string m_serial_rx[2];
};

class iteagle_fpga_device : public pci_device
{
public:
	template <typename T, typename U>
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag, U &&cpu_tag, int irq_num, int serial_num)
		: iteagle_fpga_device(mconfig, tag, owner, clock)
	{
		set_screen_tag(std::forward<T>(screen_tag));
		set_irq_info(std::forward<U>(cpu_tag), irq_num, serial_num);
	}
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_init_info(int version, int seq_init) {m_version=version; m_seq_init=seq_init;}
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_irq_info(T &&tag, const int irq_num, int serial_num)
	{ m_cpu.set_tag(std::forward<T>(tag)); m_irq_num = irq_num; m_serial_irq_num = serial_num; }

	DECLARE_WRITE_LINE_MEMBER(vblank_update);
	void serial_rx_w(uint8_t data);

	enum { IO_SYSTEM, IO_IN1, IO_SW5, IO_NUM };
	template <unsigned N> auto in_callback() { return m_in_cb[N].bind(); }
	auto trackx_callback() { return m_trackx_cb.bind(); }
	auto tracky_callback() { return m_tracky_cb.bind(); }
	auto gunx_callback() { return m_gunx_cb.bind(); }
	auto guny_callback() { return m_guny_cb.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<nvram_device> m_rtc;
	optional_device<nvram_device> m_e1_nvram;
	required_device<scc85c30_device> m_scc1;
	required_device<screen_device> m_screen;
	required_device<device_execute_interface> m_cpu;
	devcb_read16::array<3> m_in_cb;
	devcb_read8 m_trackx_cb;
	devcb_read8 m_tracky_cb;
	devcb_read16 m_gunx_cb;
	devcb_read16 m_guny_cb;

	emu_timer *     m_timer;
	int m_irq_num;
	int m_serial_irq_num;

	uint32_t m_fpga_regs[0x20 / 4];
	uint32_t m_rtc_regs[0x800 / 4];
	uint32_t m_e1_nv_data[0x40 / 4];
	uint32_t m_e1_ram[0x10000 / 4];
	uint32_t m_prev_reg;

	uint32_t m_version;
	uint32_t m_seq_init;
	uint32_t m_seq;
	uint32_t m_seq_rem1, m_seq_rem2;

	int m_vblank_state;
	int m_gun_x, m_gun_y;

	iteagle_am85c30 m_serial0_1;
	iteagle_am85c30 m_serial2_3;

	void update_sequence(uint32_t data);
	void update_sequence_eg1(uint32_t data);

	void rtc_map(address_map &map);
	void fpga_map(address_map &map);
	void ram_map(address_map &map);

	uint32_t fpga_r(offs_t offset, uint32_t mem_mask = ~0);
	void fpga_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t e1_nvram_r(offs_t offset, uint32_t mem_mask = ~0);
	void e1_nvram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t e1_ram_r(offs_t offset, uint32_t mem_mask = ~0);
	void e1_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER(serial_interrupt);
};

class iteagle_eeprom_device : public pci_device {
public:
	iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// optional information overrides
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	void set_info(int sw_version, int hw_version) {m_sw_version=sw_version; m_hw_version=hw_version;}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	address_space *m_memory_space;
	uint16_t m_sw_version;
	uint8_t m_hw_version;

	std::array<uint16_t, 0x40> m_iteagle_default_eeprom;

	void eeprom_map(address_map &map);
	uint32_t eeprom_r(offs_t offset, uint32_t mem_mask = ~0);
	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

// Mimic Cypress CY82C693 Peripheral Controller
class iteagle_periph_device : public pci_device {
public:
	iteagle_periph_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	optional_device<nvram_device> m_rtc;

	uint32_t m_ctrl_regs[0xd0/4];
	uint8_t m_rtc_regs[0x100];

	void ctrl_map(address_map &map);

	uint32_t ctrl_r(offs_t offset, uint32_t mem_mask = ~0);
	void ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

};

DECLARE_DEVICE_TYPE(ITEAGLE_FPGA, iteagle_fpga_device)
DECLARE_DEVICE_TYPE(ITEAGLE_EEPROM, iteagle_eeprom_device)
DECLARE_DEVICE_TYPE(ITEAGLE_PERIPH, iteagle_periph_device)

#endif // MAME_MACHINE_ITEAGLE_FPGA_H
