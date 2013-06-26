#ifndef _DP8390_H_
#define _DP8390_H_

#include "emu.h"

struct dp8390_interface
{
	devcb_write_line    irq_cb;
	devcb_write_line    breq_cb;
	devcb_read8         mem_read_cb;
	devcb_write8        mem_write_cb;
};

// device stuff
#define MCFG_DP8390D_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, DP8390D, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_RTL8019A_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, RTL8019A, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

class dp8390_device : public device_t,
						public device_network_interface,
						public dp8390_interface
{
public:
	// construction/destruction
	dp8390_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, float bandwidth, const char *shortname, const char *source);

	DECLARE_WRITE16_MEMBER( dp8390_w );
	DECLARE_READ16_MEMBER( dp8390_r );
	DECLARE_WRITE_LINE_MEMBER( dp8390_cs );
	DECLARE_WRITE_LINE_MEMBER( dp8390_reset );
	void recv_cb(UINT8 *buf, int len);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	int m_type;

	enum {
		TYPE_DP8390D = 0,
		TYPE_RTL8019A
	};

private:
	devcb_resolved_write_line   irq_func;
	devcb_resolved_write_line   breq_func;
	devcb_resolved_read8        mem_read;
	devcb_resolved_write8       mem_write;

	void set_cr(UINT8 newcr);
	void check_dma_complete();
	void do_tx();
	bool mcast_ck(const UINT8 *buf, int len);
	void check_irq() { irq_func((m_regs.imr & m_regs.isr & 0x7f)?ASSERT_LINE:CLEAR_LINE); }
	void recv_overflow();
	void stop();
	void recv(UINT8 *buf, int len);

	int m_reset;
	bool m_cs;
	int m_rdma_active;

	struct {
		UINT8 cr;
		UINT16 clda;
		UINT8 pstart;
		UINT8 pstop;
		UINT8 bnry;
		UINT8 tsr;
		UINT8 tpsr;
		UINT8 ncr;
		UINT8 fifo;
		UINT16 tbcr;
		UINT8 isr;
		UINT16 crda;
		UINT16 rsar;
		UINT16 rbcr;
		UINT8 rsr;
		UINT8 rcr;
		UINT8 cntr0;
		UINT8 tcr;
		UINT8 cntr1;
		UINT8 dcr;
		UINT8 cntr2;
		UINT8 imr;

		UINT8 par[6];
		UINT8 curr;
		UINT8 mar[8];

		UINT8 rnpp;
		UINT8 lnpp;
		UINT16 ac;
	} m_regs;

	struct {
		UINT8 cr9346;
		UINT8 bpage;
		UINT8 config0;
		UINT8 config1;
		UINT8 config2;
		UINT8 config3;
		UINT8 config4;
		UINT8 csnsav;
		UINT8 intr;
	} m_8019regs;
};

class rtl8019a_device : public dp8390_device
{
public:
	rtl8019a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class dp8390d_device : public dp8390_device
{
public:
	dp8390d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// device type definition
extern const device_type DP8390D;
extern const device_type RTL8019A;

#endif
