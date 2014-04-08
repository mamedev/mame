/***************************************************************************

    h8_intc.h

    H8 interrupt controllers family

****************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __H8_INTC_H__
#define __H8_INTC_H__

#include "h8.h"

#define MCFG_H8_INTC_ADD( _tag )    \
	MCFG_DEVICE_ADD( _tag, H8_INTC, 0 )

#define MCFG_H8H_INTC_ADD( _tag )   \
	MCFG_DEVICE_ADD( _tag, H8H_INTC, 0 )

#define MCFG_H8S_INTC_ADD( _tag )   \
	MCFG_DEVICE_ADD( _tag, H8S_INTC, 0 )


class h8_intc_device : public device_t {
public:
	h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	h8_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	int interrupt_taken(int vector);
	void internal_interrupt(int vector);
	void set_input(int inputnum, int state);
	void set_filter(int icr_filter, int ipr_filter);

	DECLARE_READ8_MEMBER(ier_r);
	DECLARE_WRITE8_MEMBER(ier_w);
	DECLARE_READ8_MEMBER(iscr_r);
	DECLARE_WRITE8_MEMBER(iscr_w);

protected:
	enum { IRQ_LEVEL, IRQ_EDGE, IRQ_DUAL_EDGE };
	enum { MAX_VECTORS = 256 };

	int irq_vector_base;
	int irq_vector_nmi;

	required_device<h8_device> cpu;

	UINT32 pending_irqs[MAX_VECTORS/32];
	int irq_type[8];
	bool nmi_input;
	UINT8 irq_input;
	UINT8 ier;
	UINT8 isr;
	UINT16 iscr;
	int icr_filter, ipr_filter;

	virtual void device_start();
	virtual void device_reset();

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	void update_irq_state();
	void update_irq_types();
	void check_level_irqs(bool force_update = false);
};

class h8h_intc_device : public h8_intc_device {
public:
	h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	h8h_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ8_MEMBER(isr_r);
	DECLARE_WRITE8_MEMBER(isr_w);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_WRITE8_MEMBER(icr_w);
	DECLARE_READ8_MEMBER(icrc_r);
	DECLARE_WRITE8_MEMBER(icrc_w);
	DECLARE_READ8_MEMBER(iscrh_r);
	DECLARE_WRITE8_MEMBER(iscrh_w);
	DECLARE_READ8_MEMBER(iscrl_r);
	DECLARE_WRITE8_MEMBER(iscrl_w);

protected:
	static const int vector_to_slot[];

	UINT32 icr;

	virtual void device_start();
	virtual void device_reset();

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	void update_irq_types();
};

class h8s_intc_device : public h8h_intc_device {
public:
	h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(ipr_r);
	DECLARE_WRITE8_MEMBER(ipr_w);
	DECLARE_READ8_MEMBER(iprk_r);
	DECLARE_WRITE8_MEMBER(iprk_w);

	void set_mode_8(bool mode_8);

private:
	static const int vector_to_slot[];
	UINT8 ipr[11];

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	virtual void device_reset();
};

extern const device_type H8_INTC;
extern const device_type H8H_INTC;
extern const device_type H8S_INTC;

#endif
