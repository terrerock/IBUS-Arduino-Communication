#include "IBus.h"

rc_ibus_c rc_ibus(Serial1, STANDARD_IBUS_BAUD_BPS);
uint32_t loop_timer;

void setup()
{
  Serial.begin(9600);      // Serial Monitor port uses UBS-Serial
  rc_ibus.begin();         // RC IBUS uses UART-Serial on pin 0
  loop_timer = micros();
}

void loop()
{ 
  ibus_status_t status = rc_ibus.update_buffer(); 
  uint8_t eng_count = rc_ibus.get_eng_unit_item_count(); // 20 items (see IBus.h)

  for( uint8_t i = 0; i < eng_count; i++ ) {
      Serial.print(rc_ibus.get_eng_unit_item(i));
      Serial.print(" ");
  }
  Serial.println(" ");

  while (micros() - loop_timer < 4000);  // this loop must run at 7 msec or less
    loop_timer = micros();
}
