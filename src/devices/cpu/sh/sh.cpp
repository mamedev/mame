
#include "sh.h"

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
void sh_common_execution::ADD(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] += m_sh2_state->r[m];
}
