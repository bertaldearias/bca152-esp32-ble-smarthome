#ifndef app_sensor_h_
#define app_sensor_h_

#include <stdbool.h>

/* Calibrate: read "light level" in the serial monitor in bright light and in
   the dark, then set this to a value between the two (ADC counts, 0-4095). */
#define LIGHT_THRESHOLD 1000

#define LIGHT_ADC_CHANNEL ADC_CHANNEL_0   /* GPIO1 on ESP32-S3 */

typedef void (*light_changed_f)(bool);

void init_hw(light_changed_f);
bool is_light_low(void);

#endif