#ifndef appled_attn_h_
#define appled_attn_h_

#include <stdbool.h>

#ifndef ATTN_LED_PIN
#define ATTN_LED_PIN 19   /* book default; override with -DATTN_LED_PIN=<pin> */
#endif

void appled_init(void);
void appled_set(bool);

#endif