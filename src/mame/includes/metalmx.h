class metalmx_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, metalmx_state(machine)); }

	metalmx_state(running_machine &machine)
		: driver_data_t(machine) { }

	running_device *maincpu;
	running_device *gsp;
	running_device *adsp;
	running_device *dsp32c_1;
	running_device *dsp32c_2;

	UINT16				*gsp_dram;
	UINT16				*gsp_vram;

	UINT32				*adsp_internal_program_ram;
};
