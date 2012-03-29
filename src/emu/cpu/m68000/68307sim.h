/* 68307 SIM module */

READ16_HANDLER( m68307_internal_sim_r );
WRITE16_HANDLER( m68307_internal_sim_w );

/* used for the CS logic */
#define m68307SIM_BR0 (0x40)
#define m68307SIM_OR0 (0x42)
#define m68307SIM_BR1 (0x44)
#define m68307SIM_OR1 (0x46)
#define m68307SIM_BR2 (0x48)
#define m68307SIM_OR2 (0x4a)
#define m68307SIM_BR3 (0x4c)
#define m68307SIM_OR3 (0x4e)

class m68307_sim
{
	public:

	UINT16 m_br[4];
	UINT16 m_or[4];

	void reset(void);
};