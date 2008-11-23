/*SCU DSP stuff*/

extern void dsp_prg_ctrl(const address_space *space, UINT32 data);
extern void dsp_prg_data(UINT32 data);
extern void dsp_ram_addr_ctrl(UINT32 data);
extern void dsp_ram_addr_w(UINT32 data);
extern UINT32 dsp_ram_addr_r(void);
extern void dsp_execute_program(const address_space *dmaspace);

