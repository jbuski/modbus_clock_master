#ifndef modbus_h
#define modbus_h

#include "ArduinoRS485.h" // ArduinoModbus depends on the ArduinoRS485 library
#include "ArduinoModbus.h"

#define RESET_REGISTER  0b10000000
#define SAVE_REGISTER   0b01000000
#define LOAD_REGISTER   0b00100000

void modbus_init(void);
void write_seconds(uint8_t s);  //Address 0x01
void write_minutes(uint8_t m);  //Address 0x02
void write_hours12(uint8_t h);  //Address 0x03
void write_hours24(uint8_t h);  //Address 0x04
void write_week(uint8_t w);     //Address 0x05
void write_day(uint8_t d);      //Address 0x06

uint8_t send_cal_values(uint16_t val, uint16_t pwm_min,uint16_t pwm_max, uint16_t range_min,uint16_t range_max,uint16_t adr);

#endif