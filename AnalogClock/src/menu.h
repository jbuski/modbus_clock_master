#ifndef menu_h
#define menu_h
#include <EEPROM.h>
#include "Arduino.h"

void menu_init(void);
void print_help(int arg_cnt, char **args);
void test(int arg_cnt, char **args);
void pro(int arg_cnt, char **args);
void info(int arg_cnt, char **args);
void load(int arg_cnt, char **args);
void save(int arg_cnt, char **args);
void stop_clock(int arg_cnt, char **args);
void set_clock(int arg_cnt, char **args);
void start_clock(int arg_cnt, char **args);
void get(int arg_cnt, char **argsid);
void get_clock(void);
void get_pwm(void);
void get_clock(int arg_cnt, char **args);
void calh12(int arg_cnt, char **args);
void calh24(int arg_cnt, char **args);
void calm(int arg_cnt, char **args);
void cals(int arg_cnt, char **args);
void print_cal(int arg_cnt, char **args);
void task_list(int arg_cnt, char **args);
void reset_mcu(int arg_cnt, char **args);
void reset_timer(void);

#endif
