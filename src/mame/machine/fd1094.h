#define FD1094_STATE_RESET	0x0100
#define FD1094_STATE_IRQ	0x0200
#define FD1094_STATE_RTE	0x0300

int fd1094_set_state(UINT8 *key,int state);
int fd1094_decode(int address,int val,UINT8 *key,int vector_fetch);

#ifdef MAME_DEBUG

typedef struct _fd1094_constraint fd1094_constraint;
struct _fd1094_constraint
{
	offs_t	pc;
	UINT16	state;
	UINT16	value;
	UINT16	mask;
};

#endif
