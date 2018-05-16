// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair


#ifndef MAME_MACHINE_SCC2698B_H
#define MAME_MACHINE_SCC2698B_H

class scc2698b_device;

#define SCC2698B_RX_FIFO_SIZE	3

// _port should be a letter port index from "a" .. "h"
#define MCFG_SCC2698B_TX_CALLBACK(_port, _cb) \
	devcb = &(downcast<scc2698b_device &>(*device).m_channel_##_port)->set_tx_cb(DEVCB_##_cb);

#define MCFG_SCC2698B_MPP1_CALLBACK(_port, _cb) \
	devcb = &(downcast<scc2698b_device &>(*device).m_channel_##_port)->set_mpp1_cb(DEVCB_##_cb);

#define MCFG_SCC2698B_MPP2_CALLBACK(_port, _cb) \
	devcb = &(downcast<scc2698b_device &>(*device).m_channel_##_port)->set_mpp2_cb(DEVCB_##_cb);

#define MCFG_SCC2698B_MPO_CALLBACK(_port, _cb) \
	devcb = &(downcast<scc2698b_device &>(*device).m_channel_##_port)->set_mpo_cb(DEVCB_##_cb);



class scc2698b_channel : public device_t, public device_serial_interface
{
	friend class scc2698b_device;
public:
	scc2698b_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_tx_cb(Object &&cb) { return write_tx.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_mpp1_cb(Object &&cb) { return write_mpp1.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_mpp2_cb(Object &&cb) { return write_mpp2.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_mpo_cb(Object &&cb) { return write_mpo.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(mpi0_w);
	DECLARE_WRITE_LINE_MEMBER(mpi1_w);

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
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line write_tx, write_mpp1, write_mpp2, write_mpo;

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

};

class scc2698b_device : public device_t
{
public:
	scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void write_reg(int offset, uint8_t data);
	uint8_t read_reg(int offset);

	template <class Object> static devcb_base &set_intr_A(device_t &device, Object &&cb) { return downcast<scc2698b_device &>(device).write_intr_A.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_intr_B(device_t &device, Object &&cb) { return downcast<scc2698b_device &>(device).write_intr_B.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_intr_C(device_t &device, Object &&cb) { return downcast<scc2698b_device &>(device).write_intr_C.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_intr_D(device_t &device, Object &&cb) { return downcast<scc2698b_device &>(device).write_intr_D.set_callback(std::forward<Object>(cb)); }


	required_device<scc2698b_channel> m_channel_a;
	required_device<scc2698b_channel> m_channel_b;
	required_device<scc2698b_channel> m_channel_c;
	required_device<scc2698b_channel> m_channel_d;
	required_device<scc2698b_channel> m_channel_e;
	required_device<scc2698b_channel> m_channel_f;
	required_device<scc2698b_channel> m_channel_g;
	required_device<scc2698b_channel> m_channel_h;

	DECLARE_WRITE_LINE_MEMBER(port_a_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_b_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_c_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_d_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_e_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_f_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_g_rx_w);
	DECLARE_WRITE_LINE_MEMBER(port_h_rx_w);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

	devcb_write_line write_intr_A, write_intr_B, write_intr_C, write_intr_D;

	void log_register_access(int offset, int value, const char* direction, const char* reg_name);

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
