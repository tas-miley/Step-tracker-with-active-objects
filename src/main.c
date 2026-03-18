#include <zephyr/kernel.h>
#include "ble_ao.h"
#include "imu_ao.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
    LOG_DBG("Initializing from main.");
    // ble_ao_init();
    imu_ao_init();
	return 0;
}
