/*****************************************************************************
 *
 * includes/amstr_pc.h
 *
 ****************************************************************************/

#ifndef AMSTR_PC_H_
#define AMSTR_PC_H_

#include "includes/pc.h"
#include "machine/pc_lpt.h"

class amstrad_pc_state : public pc_state
{
public:
	amstrad_pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc_state(mconfig, type, tag),
		m_lpt1(*this, "lpt_1"),
		m_lpt2(*this, "lpt_2")
			{ m_mouse.x =0; m_mouse.y=0;}

	required_device<pc_lpt_device> m_lpt1;
	required_device<pc_lpt_device> m_lpt2;

	DECLARE_READ8_MEMBER( pc1640_port60_r );
	DECLARE_WRITE8_MEMBER( pc1640_port60_w );

	DECLARE_READ8_MEMBER( pc1640_mouse_x_r );
	DECLARE_READ8_MEMBER( pc1640_mouse_y_r );
	DECLARE_WRITE8_MEMBER( pc1640_mouse_x_w );
	DECLARE_WRITE8_MEMBER( pc1640_mouse_y_w );

	DECLARE_READ8_MEMBER( pc200_port378_r );
	DECLARE_READ8_MEMBER( pc200_port278_r );
	DECLARE_READ8_MEMBER( pc1640_port378_r );
	DECLARE_READ8_MEMBER( pc1640_port3d0_r );
	DECLARE_READ8_MEMBER( pc1640_port4278_r );
	DECLARE_READ8_MEMBER( pc1640_port278_r );

	DECLARE_DRIVER_INIT(pc1512);
	DECLARE_DRIVER_INIT(pc1640);
	DECLARE_DRIVER_INIT(ppc512);
	DECLARE_DRIVER_INIT(pc200);

private:
	struct {
		UINT8 x,y; //byte clipping needed
	} m_mouse;

	// 64 system status register?
	UINT8 m_port60;
	UINT8 m_port61;
	UINT8 m_port62;
	UINT8 m_port65;

	int m_dipstate;
};

INPUT_PORTS_EXTERN( amstrad_keyboard );


#endif /* AMSTR_PC_H_ */
