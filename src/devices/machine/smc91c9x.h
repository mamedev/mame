// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles, Jean-François DEL NERO

**************************************************************************/

#ifndef MAME_MACHINE_SMC91C9X_H
#define MAME_MACHINE_SMC91C9X_H

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class smc91c9x_device : public device_t,public device_network_interface
{
public:
	template <class Object> static devcb_base &set_irq_callback(device_t &device, Object &&cb) { return downcast<smc91c9x_device &>(device).m_irq_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	virtual void recv_cb(uint8_t *data, int length) override;

protected:
	smc91c9x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr unsigned ETHER_BUFFER_SIZE   = 2048;
	static constexpr unsigned ETHER_RX_BUFFERS    = 16;
	static constexpr unsigned ETHER_TX_BUFFERS    = 16;
	static constexpr unsigned ETHERNET_ADDR_SIZE = 6;

	// internal state
	devcb_write_line m_irq_handler;

	// link unconnected
	bool m_link_unconnected;

	/* raw register data and masks */
	uint16_t          m_reg[64];
	uint16_t          m_regmask[64];

	/* IRQ information */
	uint8_t           m_irq_state;

	/* allocate information */
	uint8_t           m_alloc_count;

	/* transmit/receive FIFOs */
	uint32_t          rx_fifo_out;
	uint32_t          rx_fifo_in;
	uint8_t           m_rx[ETHER_BUFFER_SIZE * ETHER_RX_BUFFERS];

	uint32_t          tx_fifo_out;
	uint32_t          tx_fifo_in;
	uint8_t           m_tx[ETHER_BUFFER_SIZE * ETHER_TX_BUFFERS];

	/* counters */
	uint32_t          m_sent;
	uint32_t          m_recd;

	int ethernet_packet_is_for_me(const uint8_t mac_address[]);
	int is_broadcast(uint8_t mac_address[]);

	void update_ethernet_irq();
	void update_stats();

	void process_command(uint16_t data);
	void clear_tx_fifo();
	void clear_rx_fifo();

	int  send_frame();

};


class smc91c94_device : public smc91c9x_device
{
public:
	smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class smc91c96_device : public smc91c9x_device
{
public:
	smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(SMC91C94, smc91c94_device)
DECLARE_DEVICE_TYPE(SMC91C96, smc91c96_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SMC91C94_ADD(tag) \
		MCFG_DEVICE_ADD((tag), SMC91C94, 0)

#define MCFG_SMC91C94_IRQ_CALLBACK(write) \
		devcb = &smc91c94_device::set_irq_callback(*device, DEVCB_##write);

#define MCFG_SMC91C96_ADD(tag) \
	MCFG_DEVICE_ADD((tag), SMC91C96, 0)

#define MCFG_SMC91C96_IRQ_CALLBACK(write) \
	devcb = &smc91c96_device::set_irq_callback(*device, DEVCB_##write);

#endif // MAME_MACHINE_SMC91C9X_H
