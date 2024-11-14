// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    8530scc.h

    Zilog 8530 SCC (Serial Control Chip) code

*********************************************************************/

#ifndef MAME_MACHINE_8530SCC_H
#define MAME_MACHINE_8530SCC_H

class scc8530_legacy_device : public device_t
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
		IRQ_B_EXT
	};

	scc8530_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto intrq_callback() { return intrq_cb.bind(); }

	uint8_t get_reg_a(int reg);
	uint8_t get_reg_b(int reg);
	void set_reg_a(int reg, uint8_t data);
	void set_reg_b(int reg, uint8_t data);

	void set_status(int status);

	uint8_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint8_t data);

	void write_reg(int offset, uint8_t data);
	uint8_t read_reg(int offset);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(baud_expire);

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
		uint8_t rxData;
		uint8_t txData;

		emu_timer *baudtimer;

		uint8_t reg_val[16];
	};

	int mode;
	int reg;
	int status;
	int IRQV;
	int MasterIRQEnable;
	int lastIRQStat;
	IRQType_t IRQType;

	Chan channel[2];

	devcb_write_line intrq_cb;

	void updatebaudtimer(int ch);
	void updateirqs();
	void initchannel(int ch);
	void resetchannel(int ch);
	void acknowledge();
	uint8_t getareg();
	uint8_t getbreg();
	void putreg(int ch, uint8_t data);
};

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_DEVICE_TYPE(SCC8530, scc8530_legacy_device)

#endif // MAME_MACHINE_8530SCC_H
