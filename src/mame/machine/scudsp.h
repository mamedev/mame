/*SCU DSP stuff*/

void dsp_prg_ctrl_w(address_space &space, UINT32 data);
void dsp_prg_data(UINT32 data);
void dsp_ram_addr_ctrl(UINT32 data);
void dsp_ram_addr_w(UINT32 data);
UINT32 dsp_prg_ctrl_r(address_space &space);
UINT32 dsp_ram_addr_r(void);
void dsp_execute_program(address_space &dmaspace);

