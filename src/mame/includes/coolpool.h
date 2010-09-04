class coolpool_state : public driver_device
{
public:
	coolpool_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	UINT16 *vram_base;
	required_shared_ptr<UINT16> m_nvram;

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

	running_device *maincpu;
	running_device *dsp;
};
