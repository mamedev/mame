// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_timer16.h

    H8 16 bits timer


***************************************************************************/

#ifndef __H8_TIMER16_H__
#define __H8_TIMER16_H__

#include "h8.h"
#include "h8_intc.h"

#define MCFG_H8_TIMER16_ADD( _tag, _count, _tstr )  \
	MCFG_DEVICE_ADD( _tag, H8_TIMER16, 0 )          \
	downcast<h8_timer16_device *>(device)->set_info(_count, _tstr);

#define MCFG_H8_TIMER16_CHANNEL_ADD( _tag, tgr_count, tbr_count, intc, irq_base ) \
	MCFG_DEVICE_ADD( _tag, H8_TIMER16_CHANNEL, 0 )  \
	downcast<h8_timer16_channel_device *>(device)->set_info(tgr_count, tbr_count, intc, irq_base);

#define MCFG_H8H_TIMER16_CHANNEL_ADD( _tag, tgr_count, tbr_count, intc, irq_base ) \
	MCFG_DEVICE_ADD( _tag, H8H_TIMER16_CHANNEL, 0 ) \
	downcast<h8h_timer16_channel_device *>(device)->set_info(tgr_count, tbr_count, intc, irq_base);

#define MCFG_H8S_TIMER16_CHANNEL_ADD( _tag, tgr_count, tier_mask, intc, irq_base, t0, t1, t2, t3, t4, t5, t6, t7 ) \
	MCFG_DEVICE_ADD( _tag, H8S_TIMER16_CHANNEL, 0 ) \
	downcast<h8s_timer16_channel_device *>(device)->set_info(tgr_count, tier_mask, intc, irq_base, t0, t1, t2, t3, t4, t5, t6, t7);

#define MCFG_H8S_TIMER16_CHANNEL_SET_CHAIN( _tag )  \
	downcast<h8s_timer16_channel_device *>(device)->set_chain(_tag);

class h8_timer16_channel_device : public device_t {
public:
	enum {
		CHAIN,
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		DIV_1,
		DIV_2,
		DIV_4,
		DIV_8,
		DIV_16,
		DIV_32,
		DIV_64,
		DIV_128,
		DIV_256,
		DIV_512,
		DIV_1024,
		DIV_2048,
		DIV_4096
	};

	enum {
		TGR_CLEAR_NONE = -1,
		TGR_CLEAR_EXT  = -2
	};

	enum {
		IRQ_A = 0x01,
		IRQ_B = 0x02,
		IRQ_C = 0x04,
		IRQ_D = 0x08,
		IRQ_V = 0x10,
		IRQ_U = 0x20,
		IRQ_TRIG = 0x40
	};


	h8_timer16_channel_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	h8_timer16_channel_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	void set_info(int tgr_count, int tbr_count, std::string intc, int irq_base);

	DECLARE_READ8_MEMBER(tcr_r);
	DECLARE_WRITE8_MEMBER(tcr_w);
	DECLARE_READ8_MEMBER(tmdr_r);
	DECLARE_WRITE8_MEMBER(tmdr_w);
	DECLARE_READ8_MEMBER(tior_r);
	DECLARE_WRITE8_MEMBER(tior_w);
	DECLARE_READ8_MEMBER(tier_r);
	DECLARE_WRITE8_MEMBER(tier_w);
	DECLARE_READ8_MEMBER(tsr_r);
	DECLARE_WRITE8_MEMBER(tsr_w);
	DECLARE_READ16_MEMBER(tcnt_r);
	DECLARE_WRITE16_MEMBER(tcnt_w);
	DECLARE_READ16_MEMBER(tgr_r);
	DECLARE_WRITE16_MEMBER(tgr_w);
	DECLARE_READ16_MEMBER(tbr_r);
	DECLARE_WRITE16_MEMBER(tbr_w);

	UINT64 internal_update(UINT64 current_time);
	void set_ier(UINT8 value);
	void set_enable(bool enable);
	void tisr_w(int offset, UINT8 data);
	UINT8 tisr_r(int offset) const;

protected:
	required_device<h8_device> cpu;
	h8_timer16_channel_device *chained_timer;
	h8_intc_device *intc;
	std::string chain_tag;
	std::string intc_tag;
	int interrupt[6];
	UINT8 tier_mask;

	int tgr_count, tbr_count;
	int tgr_clearing;
	UINT8 tcr, tier, ier, isr;
	int clock_type, clock_divider;
	UINT16 tcnt, tgr[6];
	UINT64 last_clock_update, event_time;
	UINT32 phase, counter_cycle;
	bool counter_incrementing;
	bool channel_active;

	virtual void device_start() override;
	virtual void device_reset() override;

	void update_counter(UINT64 cur_time = 0);
	void recalc_event(UINT64 cur_time = 0);
	virtual void tcr_update();
	virtual void tier_update();
	virtual void isr_update(UINT8 value);
	virtual UINT8 isr_to_sr() const;
};

class h8h_timer16_channel_device : public h8_timer16_channel_device {
public:
	h8h_timer16_channel_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~h8h_timer16_channel_device();

	void set_info(int tgr_count, int tbr_count, std::string intc, int irq_base);

protected:
	virtual void tcr_update() override;
	virtual void tier_update() override;
	virtual void isr_update(UINT8 value) override;
	virtual UINT8 isr_to_sr() const override;
};

class h8s_timer16_channel_device : public h8_timer16_channel_device {
public:
	h8s_timer16_channel_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~h8s_timer16_channel_device();

	void set_info(int tgr_count, UINT8 _tier_mask, std::string intc, int irq_base,
					int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7);
	void set_chain(std::string chain_tag);

protected:
	int count_types[8];

	virtual void tcr_update() override;
	virtual void tier_update() override;
	virtual void isr_update(UINT8 value) override;
	virtual UINT8 isr_to_sr() const override;
};

class h8_timer16_device : public device_t {
public:
	h8_timer16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void set_info(int timer_count, UINT8 default_tstr);

	DECLARE_READ8_MEMBER(tstr_r);
	DECLARE_WRITE8_MEMBER(tstr_w);
	DECLARE_READ8_MEMBER(tsyr_r);
	DECLARE_WRITE8_MEMBER(tsyr_w);
	DECLARE_READ8_MEMBER(tmdr_r);
	DECLARE_WRITE8_MEMBER(tmdr_w);
	DECLARE_READ8_MEMBER(tfcr_r);
	DECLARE_WRITE8_MEMBER(tfcr_w);
	DECLARE_READ8_MEMBER(toer_r);
	DECLARE_WRITE8_MEMBER(toer_w);
	DECLARE_READ8_MEMBER(tocr_r);
	DECLARE_WRITE8_MEMBER(tocr_w);
	DECLARE_READ8_MEMBER(tisr_r);
	DECLARE_WRITE8_MEMBER(tisr_w);
	DECLARE_READ8_MEMBER(tisrc_r);
	DECLARE_WRITE8_MEMBER(tisrc_w);
	DECLARE_WRITE8_MEMBER(tolr_w);

protected:
	required_device<h8_device> cpu;
	h8_timer16_channel_device *timer_channel[6];
	int timer_count;
	UINT8 default_tstr;
	UINT8 tstr;

	virtual void device_start() override;
	virtual void device_reset() override;
};

extern const device_type H8_TIMER16;
extern const device_type H8_TIMER16_CHANNEL;
extern const device_type H8H_TIMER16_CHANNEL;
extern const device_type H8S_TIMER16_CHANNEL;

#endif
