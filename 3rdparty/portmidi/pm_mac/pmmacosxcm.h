/* system-specific definitions */

PmError pm_macosxcm_init(void);
void pm_macosxcm_term(void);

PmDeviceID find_default_device(const char *path, int input, PmDeviceID id);
