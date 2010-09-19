#include "cpu/adsp2100/adsp2100.h"

class metalmx_state : public driver_device
{
public:
	metalmx_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  gsp(*this, "gsp"),
		  adsp(*this, "adsp"),
		  dsp32c_1(*this, "dsp32c_1"),
		  dsp32c_2(*this, "dsp32c_2") { }

	required_device<m68ec020_device> maincpu;
	required_device<tms34020_device> gsp;
	required_device<adsp2105_device> adsp;
	required_device<dsp32c_device> dsp32c_1;
	required_device<dsp32c_device> dsp32c_2;

	UINT16				*gsp_dram;
	UINT16				*gsp_vram;

	UINT32				*adsp_internal_program_ram;
};
