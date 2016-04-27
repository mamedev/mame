// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_dtc.h

    H8 Data Transfer Controller

***************************************************************************/

#ifndef __H8_DTC_H__
#define __H8_DTC_H__

#include "h8_intc.h"

#include <list>

#define MCFG_H8_DTC_ADD( _tag, intc, irq ) \
	MCFG_DEVICE_ADD( _tag, H8_DTC, 0 ) \
	downcast<h8_dtc_device *>(device)->set_info(intc, irq);

struct h8_dtc_state {
	UINT32 base, sra, dar, cr;
	INT32 incs, incd;
	UINT32 count;
	int id;
	int next;
};

class h8_dtc_device : public device_t {
public:
	enum { DTC_CHAINED = 1000 };

	h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_info(const char *intc, int irq);

	DECLARE_READ8_MEMBER(dtcer_r);
	DECLARE_WRITE8_MEMBER(dtcer_w);
	DECLARE_READ8_MEMBER(dtvecr_r);
	DECLARE_WRITE8_MEMBER(dtvecr_w);

	bool trigger_dtc(int vector);
	void count_done(int id);

	inline h8_dtc_state *get_object(int vector) { return states + vector; }
	inline UINT32 get_vector_address(int vector) { return 0x400 | ((vector ? vector : dtvecr & 0x7f) << 1); }
	int get_waiting_vector();
	int get_waiting_writeback();
	void vector_done(int vector);
	void writeback_done(int vector);

protected:
	static const int vector_to_enable[];
	required_device<h8_device> cpu;
	h8_intc_device *intc;
	const char *intc_tag;
	int irq;
	h8_dtc_state states[92];

	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 dtcer[6], dtvecr;
	int cur_active_vector;

	std::list<int> waiting_vector, waiting_writeback;

	void edge(int vector);
	void queue(int vector);
};

extern const device_type H8_DTC;

#endif
