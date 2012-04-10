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
};

/*----------- defined in machine/cd32.c -----------*/

READ32_DEVICE_HANDLER( amiga_akiko32_r );
WRITE32_DEVICE_HANDLER( amiga_akiko32_w );

DECLARE_LEGACY_DEVICE(AKIKO, akiko);

#endif /* __CUBOCD32_H__ */
