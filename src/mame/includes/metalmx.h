typedef struct _metalmx_state metalmx_state;
struct _metalmx_state
{
	const device_config *maincpu;
	const device_config *gsp;
	const device_config *adsp;
	const device_config *dsp32c_1;
	const device_config *dsp32c_2;

	UINT16				*gsp_dram;
	UINT16				*gsp_vram;

	UINT32				*adsp_internal_program_ram;
};
