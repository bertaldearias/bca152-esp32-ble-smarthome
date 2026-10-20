
#ifndef app_sw_h_
#define app_sw_h_

#include <stdbool.h>

#define SWITCH_LED_PIN 4

void init_hw(void);
void switch_set(bool on);
bool switch_get(void);

#endif
