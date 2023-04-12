#include <Wifi.h>
#include "AnalogClock.h"
#include "cmdArduino.h"
#include "menu.h"
#include "network.h"
#include "modbus.h"

#define DEBUG_LEVEL  1                                               // manages most of the print and println debug, not all but most

#if (DEBUG_LEVEL > 0U)

#define UsrLogln(...)\
		Serial.print("USR menu.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define UsrLog(...)\
		Serial.print("USR menu.cpp: ");\
		Serial.print(__VA_ARGS__)
#define print_k(...)\
		Serial.print(__VA_ARGS__)
		
#define print_kln(...)\
		Serial.println(__VA_ARGS__)
#else
#define UsrLogln(...)
#define UsrLog(...)
#define print_k(...)
#define print_kln(...)
#endif

#if (DEBUG_LEVEL > 1U)
#define ErrLogln(...)\
		Serial.print("ERR menu.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define ErrLog(...)\
		Serial.print("ERR menu.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define ErrLogln(...)
#define ErrLog(...)
#endif

#if (DEBUG_LEVEL > 2U)
#define DbgLogln(...)\
		Serial.print("DBG menu.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define DbgLog(...)\
		Serial.print("DBG menu.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define DbgLogln(...)
#define DbgLog(...)
#endif

extern AnalogClock my_clock;
extern uint8_t start_flag ;
extern unsigned long start_flag_time;

extern Cmd cmd;
extern uint8_t h,s,m;
uint8_t progres_value = 0;

void menu_init(void)
{
  cmd.add("help", print_help);
  cmd.add("load", load);
  cmd.add("save", save);
  cmd.add("stop", stop_clock);
  cmd.add("start", start_clock);
  cmd.add("run", start_clock);
  cmd.add("set", set_clock);
  cmd.add("get", get);
  cmd.add("calh24", calh24);
  cmd.add("calh12", calh12);
  cmd.add("calm", calm);
  cmd.add("cals", cals);
  cmd.add("printcal", print_cal);
  cmd.add("reset", reset_mcu);
  cmd.add("info", info);
  cmd.add("test", test);
  cmd.add("pro", pro);
  cmd.add("task", task_list);
}

void print_help(int arg_cnt, char **args)
{
  Serial.println("help - print help");
  Serial.println("load - load calibration");
  Serial.println("save - save calibration");
  Serial.println("start - start clock");
  Serial.println("run - start clock");
  Serial.println("stop - stop clock");
  Serial.println("set - set clock");
  Serial.println("--> set h m s");
  Serial.println("get - display clock");
  Serial.println("calh24 - calibrate hours");
  Serial.println("--> calh24 min max");
  Serial.println("calh24 - calibrate hours");
  Serial.println("--> calh12 min max");
  Serial.println("calm - calibrate minutes");
  Serial.println("--> calm min max");
  Serial.println("cals - calibrate seconds");
  Serial.println("--> cals min max");
  Serial.println("printcal - print calibration values");
  Serial.println("test - test meters");
  Serial.println("pro - set progress");
  Serial.println("reset - reset MCU");
  Serial.println("info - print info");
}

void test(int arg_cnt, char **args)
{
  reset_timer();
  my_clock.test();
}

void pro(int arg_cnt, char **args)
{
  if (arg_cnt > 1)
  {
    reset_timer();
    progres_value = cmd.conv(args[1],10);
    my_clock.SetProgress(progres_value);
    
  }
  else
  {
  	Serial.println("Not enough arg");  
  }
}

void info(int arg_cnt, char **args)
{
  get_network_info();
  get_clock();
}

void load(int arg_cnt, char **args)
{
  my_clock.Load_cal();
}

void save(int arg_cnt, char **args)
{
  my_clock.Save_cal(); 
}

void stop_clock(int arg_cnt, char **args)
{
  Serial.println("Stopping clock");

  reset_timer();

  my_clock.Set_time(0,0,0);
  my_clock.Convert();
  my_clock.Display();
}

void start_clock(int arg_cnt, char **args)
{
  Serial.println("Starting clock"); 
  start_flag = 1;
}

void set_clock(int arg_cnt, char **args)
{
  if (arg_cnt > 3)
  {
    reset_timer();
    my_clock.Set_time(
          (uint8_t) cmd.conv(args[1],10),
          (uint8_t) cmd.conv(args[2],10),
          (uint8_t) cmd.conv(args[3],10));
    my_clock.Convert();
    my_clock.Display();
    my_clock.DisplayAM_PM();
  }
  else
  {
  	Serial.println("Not enough arg");  
  }
}

void get(int arg_cnt, char **args)
{
   get_clock();
   get_pwm();
}

void reset_timer(void)
{
    start_flag = 0;
    start_flag_time = millis ();
}

void get_clock(void)
{
   Serial.print(my_clock.Get_hours12());
   Serial.print(":");
   Serial.print(my_clock.Get_minutes());
   Serial.print(":");
   Serial.print(my_clock.Get_seconds());
   Serial.print(":");
   Serial.println(my_clock.is_AMorPM());
}

void get_pwm(void)
{
  Serial.print(my_clock.Get_hours12_pwm());
  Serial.print(":");
  Serial.print(my_clock.Get_minutes_pwm());
  Serial.print(":");
  Serial.print(my_clock.Get_seconds_pwm());
  Serial.print(":");
  Serial.println(my_clock.Get_AM_PM_pwm());
}

void calh24(int arg_cnt, char **args)
{
  if (arg_cnt > 2)
  {
    reset_timer();
	  my_clock.Set_Calibration_hours24_min(cmd.conv(args[1],10));
	  my_clock.Set_Calibration_hours24_max(cmd.conv(args[2],10));
    if(!my_clock.Update_hours24_cal_values(progres_value))
    {
      Serial.println("Failed to update");
    }
  }
  else
  {
  	Serial.println("Not enough arg");  
  }
}

void calh12(int arg_cnt, char **args)
{
  if (arg_cnt > 2)
  {
    reset_timer();
	  my_clock.Set_Calibration_hours12_min(cmd.conv(args[1],10));
	  my_clock.Set_Calibration_hours12_max(cmd.conv(args[2],10));
    if(!my_clock.Update_hours12_cal_values(progres_value))
    {
      Serial.println("Failed to update");
    }
  }
  else
  {
  	Serial.println("Not enough arg");  
  }
} 

void calm(int arg_cnt, char **args)
{
  if (arg_cnt > 2)
  {
    reset_timer();
    my_clock.Set_Calibration_minutes_min(cmd.conv(args[1],10));
    my_clock.Set_Calibration_minutes_max(cmd.conv(args[2],10));
    if(!my_clock.Update_minutes_cal_values(progres_value))
    {
      Serial.println("Failed to update");
    }
  }
  else
  {
    Serial.println("Not enough arg");  
  }
} 

void cals(int arg_cnt, char **args)
{
  if (arg_cnt > 2)
  {
    reset_timer();
    my_clock.Set_Calibration_seconds_min(cmd.conv(args[1],10));
    my_clock.Set_Calibration_seconds_max(cmd.conv(args[2],10));
    if(!my_clock.Update_seconds_cal_values(progres_value))
    {
      Serial.println("Failed to update");
    }
  }
  else
  {
  	Serial.println("Not enough arg");  
  }
} 

void print_cal(int arg_cnt, char **args)
{
   Serial.print("hours24_min ");
   Serial.print(my_clock.Get_Calibration_hours24_min());
   Serial.print(" hours24_max ");
   Serial.println(my_clock.Get_Calibration_hours24_max());
   Serial.print("hours12_min ");
   Serial.print(my_clock.Get_Calibration_hours12_min());
   Serial.print(" hours12_max ");
   Serial.println(my_clock.Get_Calibration_hours12_max());
   Serial.print("minutes_min ");
   Serial.print(my_clock.Get_Calibration_minutes_min());
   Serial.print(" minutes_max ");
   Serial.println(my_clock.Get_Calibration_minutes_max());
   Serial.print("seconds_min ");
   Serial.print(my_clock.Get_Calibration_seconds_min());
   Serial.print(" seconds_max ");
   Serial.println(my_clock.Get_Calibration_seconds_max());
}

void task_list(int arg_cnt, char **args)
{
  get_task_list();
}

void reset_mcu(int arg_cnt, char **args)
{
  ESP.restart();
}
