#define FIGHT_MCU  1
#define SHOOT_MCU  2
#define RACING_MCU 3
#define SAMSHO_MCU 4

/*----------- defined in drivers/hng64.c -----------*/

extern int hng64_mcu_type;

void hng64_command3d(running_machine* machine, const UINT16* packet);

