#pragma once
// license:BSD-3-Clause
// copyright-holders:John Bennett, Sailorsat
/***************************************************************************

	Namco C139 - Serial I/F Controller
	Internal Version

***************************************************************************/
#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class namco_c139_local_device : public device_t
{
public:
	// construction/destruction
	namco_c139_local_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	// I/O operations
	void data_map(address_map& map) ATTR_COLD;
	void regs_map(address_map& map) ATTR_COLD;

	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	//void get_rx_buffer(uint8_t* buffer_ptr, uint16_t* count_ptr);

	void set_rx_ptr(uint8_t* buffer_ptr, uint16_t* count_ptr);

	//TODO - make private with setters/getters
	uint16_t m_rx_bytes;
	uint16_t m_empty;
	uint16_t* m_rx_bytes_ptr = &m_empty;
	uint8_t m_comms_counter;
	uint8_t m_rx_comms_counter;
	uint8_t m_linkid;

	void get_rx_buffer(uint8_t** buffer_ptr, uint16_t** count_ptr);


protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_irq_cb;

private:
	uint16_t m_ram[0x2000];
	uint16_t m_reg[0x0010];

	std::string m_localhost;

	emu_timer* m_tick_timer;



	uint8_t m_buffer[0x200];
	uint8_t m_no_link;
	uint8_t* m_rx_buffer_ptr = &m_no_link; // for internal comms where one board may write to multiple others

	uint16_t m_linktimer;
	uint8_t m_txsize;
	uint8_t m_txblock;
	uint8_t m_reg_f3;
	uint8_t m_autocount = 0;

	TIMER_CALLBACK_MEMBER(tick_timer_callback);

	void comm_tick();
	void read_data(unsigned data_size);
	void read_data_new(void);
	unsigned read_frame(unsigned data_size);
	unsigned find_sync_bit(unsigned tx_offset, unsigned tx_mask);
	void send_data(unsigned data_size);
	void send_data_autotransmit(void);
	void send_frame(unsigned data_size);

	#define REG_0_STATUS 0
	#define REG_1_MODE 1
	#define REG_2_CONTROL 2
	#define REG_3_START 3
	#define REG_4_RXSIZE 4
	#define REG_5_TXSIZE 5
	#define REG_6_RXOFFSET 6
	#define REG_7_TXOFFSET 7
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C139_LOCAL, namco_c139_local_device)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



