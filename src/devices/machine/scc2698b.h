// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair


#ifndef MAME_MACHINE_SCC2698B_H
#define MAME_MACHINE_SCC2698B_H

class scc2698b_device;

class scc2698b_channel : public device_t, public device_serial_interface
{
	friend class scc2698b_device;
public:
	scc2698b_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_tx_cb(device_t &device, Object &&cb) { return downcast<scc2698b_channel &>(device).write_tx.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_mpp1_cb(device_t &device, Object &&cb) { return downcast<scc2698b_channel &>(device).write_mpp1.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_mpp2_cb(device_t &device, Object &&cb) { return downcast<scc2698b_channel &>(device).write_mpp2.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_mpo_cb(device_t &device, Object &&cb) { return downcast<scc2698b_channel &>(device).write_mpo.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(mpi0_w);
	DECLARE_WRITE_LINE_MEMBER(mpi1_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line write_tx, write_mpp1, write_mpp2, write_mpo;

	int moderegister_ptr;

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


private:
	struct Block {

	};

	Block m_blocks[4];

};

DECLARE_DEVICE_TYPE(SCC2698B, scc2698b_device)
DECLARE_DEVICE_TYPE(SCC2698B_CHANNEL, scc2698b_channel)

#endif // MAME_MACHINE_SCC2698B_H
