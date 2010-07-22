class coolpool_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, coolpool_state(machine)); }

	coolpool_state(running_machine &machine) { }

	UINT16 *vram_base;

	UINT8 cmd_pending;
	UINT16 iop_cmd;
	UINT16 iop_answer;
	int iop_romaddr;

	UINT8 newx[3];
	UINT8 newy[3];
	UINT8 oldx[3];
	UINT8 oldy[3];
	int dx[3];
	int dy[3];

	UINT16 result;
	UINT16 lastresult;
};
