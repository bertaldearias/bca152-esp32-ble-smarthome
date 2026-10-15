#ifndef app_sensor_h_
#define app_sensor_h_

#include <stdbool.h>

#define LIGHT_THRESHOLD 1000   /* ADC counts (0-4095); tune on your hardware */
#define LIGHT_ADC_CHANNEL ADC_CHANNEL_0   /* GPIO1 on ESP32-S3 */

typedef void (*light_changed_f)(bool);

void init_hw(light_changed_f);
bool is_light_low(void);

#endif