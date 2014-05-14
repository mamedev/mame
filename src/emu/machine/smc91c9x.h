// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__

#define ETHER_BUFFER_SIZE   (2048)
#define ETHER_RX_BUFFERS    (4)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class smc91c9x_device : public device_t
{
public:
	smc91c9x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~smc91c9x_device() {}

	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<smc91c9x_device &>(device).m_irq_handler.set_callback(object); }

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	devcb_write_line m_irq_handler;

	/* raw register data and masks */
	UINT16          m_reg[64];
	UINT16          m_regmask[64];

	/* IRQ information */
	UINT8           m_irq_state;

	/* allocate information */
	UINT8           m_alloc_count;

	/* transmit/receive FIFOs */
	UINT8           m_fifo_count;
	UINT8           m_rx[ETHER_BUFFER_SIZE * ETHER_RX_BUFFERS];
	UINT8           m_tx[ETHER_BUFFER_SIZE];

	/* counters */
	UINT32          m_sent;
	UINT32          m_recd;

	void update_ethernet_irq();
	void update_stats();
	void finish_enqueue(int param);
	void process_command(UINT16 data);
};


class smc91c94_device : public smc91c9x_device
{
public:
	smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type SMC91C94;

class smc91c96_device : public smc91c9x_device
{
public:
	smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type SMC91C96;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SMC91C94_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMC91C94, 0)

#define MCFG_SMC91C94_IRQ_CALLBACK(_write) \
	devcb = &smc91c94_device::set_irq_callback(*device, DEVCB_##_write);

#define MCFG_SMC91C96_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMC91C96, 0)

#define MCFG_SMC91C96_IRQ_CALLBACK(_write) \
	devcb = &smc91c96_device::set_irq_callback(*device, DEVCB_##_write);


#endif
