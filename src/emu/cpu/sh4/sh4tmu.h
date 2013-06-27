/* SH3/4 Timer Unit */

TIMER_CALLBACK( sh4_timer_callback );

UINT32 sh4_handle_tcnt0_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcnt1_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcnt2_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcor0_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcor1_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcor2_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcr0_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcr1_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcr2_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tstr_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tocr_addr_r(sh4_state *sh4, UINT32 mem_mask);
UINT32 sh4_handle_tcpr2_addr_r(sh4_state *sh4, UINT32 mem_mask);

void sh4_handle_tstr_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcr0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcr1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcr2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcor0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcor1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcor2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcnt0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcnt1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcnt2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tocr_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
void sh4_handle_tcpr2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);
