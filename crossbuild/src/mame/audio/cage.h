/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/

#define CAGE_IRQ_REASON_DATA_READY		(1)
#define CAGE_IRQ_REASON_BUFFER_EMPTY	(2)

MACHINE_DRIVER_EXTERN( cage );
MACHINE_DRIVER_EXTERN( cage_seattle );

void cage_init(int boot_region, offs_t speedup);
void cage_set_irq_handler(void (*irqhandler)(int));
void cage_reset_w(int state);

UINT16 main_from_cage_r(void);
UINT16 cage_control_r(void);
void main_to_cage_w(UINT16 data);
void cage_control_w(UINT16 data);
