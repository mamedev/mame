/***************************************************************************

CuboCD32 definitions

***************************************************************************/

#ifndef __CUBOCD32_H__
#define __CUBOCD32_H__

class cubocd32_state : public amiga_state
{
public:
	cubocd32_state(running_machine &machine, const driver_device_config_base &config)
		: amiga_state(machine, config) { }

	UINT16 potgo_value;
	int cd32_shifter[2];
	void (*input_hack)(running_machine *machine);
	int oldstate[2];
};

/*----------- defined in machine/cubocd32.c -----------*/

READ32_DEVICE_HANDLER( amiga_akiko32_r );
WRITE32_DEVICE_HANDLER( amiga_akiko32_w );

DECLARE_LEGACY_DEVICE(AKIKO, akiko);

#endif /* __CUBOCD32_H__ */
