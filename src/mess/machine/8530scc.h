/*********************************************************************

    8530scc.h

    Zilog 8530 SCC (Serial Control Chip) code

*********************************************************************/

#ifndef __8530SCC_H__
#define __8530SCC_H__

#define MCFG_SCC8530_ADD(_tag, _clock, _intrq_cb)	\
	MCFG_DEVICE_ADD(_tag, SCC8530, _clock)	\
	downcast<scc8530_t *>(device)->set_intrq_cb(_intrq_cb);

class scc8530_t : public device_t
{
public:
	enum IRQType_t {
		IRQ_NONE,
		IRQ_A_RX,
		IRQ_A_RX_SPECIAL,
		IRQ_B_RX,
		IRQ_B_RX_SPECIAL,
		IRQ_A_TX,
		IRQ_B_TX,
		IRQ_A_EXT,
		IRQ_B_EXT,
	};

	scc8530_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_intrq_cb(line_cb_t cb);

	UINT8 get_reg_a(int reg);
	UINT8 get_reg_b(int reg);
	void set_reg_a(int reg, UINT8 data);
	void set_reg_b(int reg, UINT8 data);

	void set_status(int status);

	DECLARE_READ8_MEMBER(reg_r);
	DECLARE_WRITE8_MEMBER(reg_w);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	struct Chan {
		bool txIRQEnable;
		bool rxIRQEnable;
		bool extIRQEnable;
		bool baudIRQEnable;
		bool txIRQPending;
		bool rxIRQPending;
		bool extIRQPending;
		bool baudIRQPending;
		bool txEnable;
		bool rxEnable;
		bool txUnderrun;
		bool txUnderrunEnable;
		bool syncHunt;
		bool DCDEnable;
		bool CTSEnable;
		UINT8 rxData;
		UINT8 txData;

		emu_timer *baudtimer;

		UINT8 reg_val[16];
	};

	int mode;
	int reg;
	int status;
	int IRQV;
	int MasterIRQEnable;
	int lastIRQStat;
	IRQType_t IRQType;

	Chan channel[2];

	line_cb_t intrq_cb;

	void updateirqs();
	void initchannel(int ch);
	void resetchannel(int ch);
	void acknowledge();
	UINT8 getareg();
	UINT8 getbreg();
	void putreg(int ch, UINT8 data);
};

/***************************************************************************
    MACROS
***************************************************************************/

extern const device_type SCC8530;

#endif /* __8530SCC_H__ */
