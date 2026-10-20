#ifndef app_sensor_h_
#define app_sensor_h_

#include <stdbool.h>

#define LIGHT_DO_PIN 1   /* module D0 pin -> GPIO1 */

/* Level that D0 shows when it is DARK. Most LM393 modules give 1 (HIGH) in
   the dark. If the module's behaviour is reversed, change this to 0. */
#define DARK_LEVEL 1

typedef void (*light_changed_f)(bool);

void init_hw(light_changed_f);
bool is_light_low(void);

#endif