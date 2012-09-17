/***************************************************************************

CuboCD32 definitions

***************************************************************************/

#ifndef __CUBOCD32_H__
#define __CUBOCD32_H__

#include "includes/amiga.h"
#include "machine/microtch.h"

class cd32_state : public amiga_state
{
public:
	cd32_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag),
		  m_microtouch(*this, "microtouch")
		{ }

	required_device<microtouch_device> m_microtouch;

	DECLARE_WRITE8_MEMBER(microtouch_tx);
	UINT16 m_potgo_value;
	int m_cd32_shifter[2];
	void (*m_input_hack)(running_machine &machine);
	int m_oldstate[2];
	DECLARE_CUSTOM_INPUT_MEMBER(cubo_input);
	DECLARE_WRITE32_MEMBER(aga_overlay_w);
	DECLARE_WRITE8_MEMBER(cd32_cia_0_porta_w);
	DECLARE_READ8_MEMBER(cd32_cia_0_portb_r);
	DECLARE_WRITE8_MEMBER(cd32_cia_0_portb_w);
	DECLARE_DRIVER_INIT(cd32);
	DECLARE_DRIVER_INIT(mgprem11);
	DECLARE_DRIVER_INIT(odeontw2);
	DECLARE_DRIVER_INIT(cndypuzl);
	DECLARE_DRIVER_INIT(haremchl);
	DECLARE_DRIVER_INIT(mgnumber);
	DECLARE_DRIVER_INIT(lsrquiz2);
	DECLARE_DRIVER_INIT(lasstixx);
	DECLARE_DRIVER_INIT(lsrquiz);
};

/*----------- defined in machine/cd32.c -----------*/

DECLARE_READ32_DEVICE_HANDLER( amiga_akiko32_r );
DECLARE_WRITE32_DEVICE_HANDLER( amiga_akiko32_w );

class akiko_device : public device_t
{
public:
	akiko_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~akiko_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type AKIKO;


#endif /* __CUBOCD32_H__ */
