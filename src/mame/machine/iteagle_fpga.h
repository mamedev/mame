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

//MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, _pclass, _subsystem_id)

#define MCFG_ITEAGLE_FPGA_ADD(_tag, _cpu_tag, _irq_num, _serial_irq_num) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_FPGA, 0x55CC33AA, 0xAA, 0xAAAAAA, 0x00) \
	downcast<iteagle_fpga_device *>(device)->set_irq_info(_cpu_tag, _irq_num, _serial_irq_num);

#define MCFG_ITEAGLE_FPGA_INIT(_version, _seq_init) \
	downcast<iteagle_fpga_device *>(device)->set_init_info(_version, _seq_init);

#define MCFG_ITEAGLE_EEPROM_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_EEPROM, 0x80861229, 0x02, 0x020000, 0x00)

#define MCFG_ITEAGLE_EEPROM_INIT(_sw_version, _hw_version) \
	downcast<iteagle_eeprom_device *>(device)->set_info(_sw_version, _hw_version);

// Mimic Cypress CY82C693 Peripheral Controller
#define MCFG_ITEAGLE_PERIPH_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_PERIPH, 0x1080C693, 0x00, 0x060100, 0x00)

// Functional emulation of AMD AM85C30 serial controller
// Two channels, A & B
class iteagle_am85c30
{
public:
	void reset(void);
	void write_control(uint8_t data, int channel);
	uint8_t read_control(int channel);
	void write_data(uint8_t data, int channel);
	uint8_t read_data(int channel);
	void write_rx_str(int channel, std::string resp);
	std::string get_tx_str(int channel) { return m_serial_tx[channel]; };
	void clear_tx_str(int channel) { m_serial_tx[channel].clear(); };
	bool check_interrupt(void) { return (m_rr_regs[0][3] != 0); };
private:
	uint8_t m_rr_regs[2][16];
	uint8_t m_wr_regs[2][16];
	std::string m_serial_tx[2];
	std::string m_serial_rx[2];
};

class iteagle_fpga_device : public pci_device
{
public:
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<nvram_device> m_rtc;
	optional_device<nvram_device> m_e1_nvram;
	required_device<scc85c30_device> m_scc1;
	screen_device *m_screen;

	void set_init_info(int version, int seq_init) {m_version=version; m_seq_init=seq_init;}
	void set_irq_info(const char *tag, const int irq_num, const int serial_num) {
		m_cpu_tag = tag; m_irq_num = irq_num; m_serial_irq_num = serial_num;}

	DECLARE_WRITE_LINE_MEMBER(vblank_update);
	DECLARE_WRITE8_MEMBER(serial_rx_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	emu_timer *     m_timer;
	const char *m_cpu_tag;
	cpu_device *m_cpu;
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

	DECLARE_READ32_MEMBER( fpga_r );
	DECLARE_WRITE32_MEMBER( fpga_w );
	DECLARE_READ32_MEMBER( rtc_r );
	DECLARE_WRITE32_MEMBER( rtc_w );

	DECLARE_READ32_MEMBER(e1_nvram_r);
	DECLARE_WRITE32_MEMBER(e1_nvram_w);
	DECLARE_READ32_MEMBER( e1_ram_r );
	DECLARE_WRITE32_MEMBER( e1_ram_w );

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
	DECLARE_READ32_MEMBER( eeprom_r );
	DECLARE_WRITE32_MEMBER( eeprom_w );

	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

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

	DECLARE_READ32_MEMBER( ctrl_r );
	DECLARE_WRITE32_MEMBER( ctrl_w );

};

DECLARE_DEVICE_TYPE(ITEAGLE_FPGA, iteagle_fpga_device)
DECLARE_DEVICE_TYPE(ITEAGLE_EEPROM, iteagle_eeprom_device)
DECLARE_DEVICE_TYPE(ITEAGLE_PERIPH, iteagle_periph_device)

#endif // MAME_MACHINE_ITEAGLE_FPGA_H
