class metalmx_state : public driver_device
{
public:
	metalmx_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	running_device *maincpu;
	running_device *gsp;
	running_device *adsp;
	running_device *dsp32c_1;
	running_device *dsp32c_2;

	UINT16				*gsp_dram;
	UINT16				*gsp_vram;

	UINT32				*adsp_internal_program_ram;
};
