// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair
#ifndef MAME_MACHINE_SCC2698B_H
#define MAME_MACHINE_SCC2698B_H

#include "diserial.h"

// _port should be a letter port index from "a" .. "h"



class scc2698b_device;

class scc2698b_channel : public device_t, public device_serial_interface
{
	friend class scc2698b_device;
public:
	scc2698b_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mpi0_w(int state);
	void mpi1_w(int state);

	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	void write_TXH(int txh);
	int read_RXH();

	void reset_all();
	void reset_tx();
	void reset_rx();

	void set_tx_enable(bool enable);
	void set_rx_enable(bool enable);

	void update_serial_configuration();

	void set_tx_bittime(const attotime &bittime);
	void set_rx_bittime(const attotime &bittime);

	void set_mpp_output(bool output);


	int read_SR();

protected:
	static constexpr unsigned SCC2698B_RX_FIFO_SIZE = 3;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

//  devcb_write_line write_tx, write_mpp1, write_mpp2, write_mpo;

	int moderegister_ptr;

	int tx_transmitting;
	int tx_fifo;
	int tx_bytecount;
	u8 rx_fifo[SCC2698B_RX_FIFO_SIZE];
	int rx_bytecount;

	u8 MR1, MR2, SR, CR, CSR;

	bool tx_enable;
	bool rx_enable;

	void recompute_pin_output(bool force = false);
	int mpp1_value, mpp2_value;
	bool mpp_is_output;

	int channel_port;
	scc2698b_device* parent;
};

class scc2698b_device : public device_t
{
public:
	scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) ATTR_COLD;

	void write(offs_t offset, u8 data);
	uint8_t read(offs_t offset);

	auto intr_A() { return write_intr_A.bind(); }
	auto intr_B() { return write_intr_B.bind(); }
	auto intr_C() { return write_intr_C.bind(); }
	auto intr_D() { return write_intr_D.bind(); }

	required_device_array<scc2698b_channel, 8> m_channel;

	void port_a_rx_w(int state);
	void port_b_rx_w(int state);
	void port_c_rx_w(int state);
	void port_d_rx_w(int state);
	void port_e_rx_w(int state);
	void port_f_rx_w(int state);
	void port_g_rx_w(int state);
	void port_h_rx_w(int state);


	void write_line_tx(int port, int value);
	void write_line_mpp1(int port, int value);
	void write_line_mpp2(int port, int value);
	void write_line_mpo(int port, int value);

	// assume contiguous letters - not EBCDIC-friendly
	template <char Port> auto tx_callback() { return write_tx[Port - 'a'].bind(); }
	template <char Port> auto mpp1_callback() { return write_mpp1[Port - 'a'].bind(); }
	template <char Port> auto mpp2_callback() { return write_mpp2[Port - 'a'].bind(); }
	template <char Port> auto mpo_callback() { return write_mpo[Port - 'a'].bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	devcb_write_line write_intr_A, write_intr_B, write_intr_C, write_intr_D;
	devcb_write_line::array<8> write_tx;
	devcb_write_line::array<8> write_mpp1;
	devcb_write_line::array<8> write_mpp2;
	devcb_write_line::array<8> write_mpo;

	scc2698b_channel* get_channel(int port);

	void reset_port(int port);
	void reset_port_tx(int port);
	void reset_port_rx(int port);

	void write_MR(int port, int value);
	void write_CSR(int port, int value);
	void write_CR(int port, int value);
	void write_THR(int port, int value);

	int read_MR(int port);
	int read_SR(int port);
	int read_RHR(int port);

	void update_block_baudrate(int block);
	void update_port_baudrate(int port);
	attotime generate_baudrate(int block, int tx, int table_index);


private:
	struct Block {
		u8 ACR;
	};

	Block m_blocks[4];

};

DECLARE_DEVICE_TYPE(SCC2698B, scc2698b_device)
DECLARE_DEVICE_TYPE(SCC2698B_CHANNEL, scc2698b_channel)

#endif // MAME_MACHINE_SCC2698B_H
