#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/dsp32/dsp32.h"

class metalmx_state : public driver_device
{
public:
	metalmx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_gsp(*this, "gsp"),
		  m_adsp(*this, "adsp"),
		  m_dsp32c_1(*this, "dsp32c_1"),
		  m_dsp32c_2(*this, "dsp32c_2") { }

	required_device<m68ec020_device> m_maincpu;
	required_device<tms34020_device> m_gsp;
	required_device<adsp2105_device> m_adsp;
	required_device<dsp32c_device> m_dsp32c_1;
	required_device<dsp32c_device> m_dsp32c_2;

	UINT16				*m_gsp_dram;
	UINT16				*m_gsp_vram;

	UINT32				*m_adsp_internal_program_ram;
};
