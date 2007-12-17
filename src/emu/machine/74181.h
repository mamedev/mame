/*
 * 74181
 *
 * 4-Bit Arithmetic Logic Unit
 *
 */


/* constants for setting input lines */
#define TTL74181_INPUT_A0		(0)
#define TTL74181_INPUT_A1		(1)
#define TTL74181_INPUT_A2		(2)
#define TTL74181_INPUT_A3		(3)
#define TTL74181_INPUT_B0		(4)
#define TTL74181_INPUT_B1		(5)
#define TTL74181_INPUT_B2		(6)
#define TTL74181_INPUT_B3		(7)
#define TTL74181_INPUT_S0		(8)
#define TTL74181_INPUT_S1		(9)
#define TTL74181_INPUT_S2		(10)
#define TTL74181_INPUT_S3		(11)
#define TTL74181_INPUT_C		(12)
#define TTL74181_INPUT_M		(13)

/* constants for reads output lines */
#define TTL74181_OUTPUT_F0		(0)
#define TTL74181_OUTPUT_F1		(1)
#define TTL74181_OUTPUT_F2		(2)
#define TTL74181_OUTPUT_F3		(3)
#define TTL74181_OUTPUT_AEQB	(4)
#define TTL74181_OUTPUT_P		(5)
#define TTL74181_OUTPUT_G		(6)
#define TTL74181_OUTPUT_CN4		(7)


void TTL74181_config(int chip, void *interface);
void TTL74181_reset(int chip);

void TTL74181_write(int chip, int startline, int lines, UINT8 data);
UINT8 TTL74181_read(int chip, int startline, int lines);
