#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"

class bfm_sc5_state : public driver_device
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
public:
	DECLARE_DRIVER_INIT(sc5);
	INTERRUPT_GEN_MEMBER(sc5_fake_timer_int);
};
