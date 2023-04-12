#include "AnalogClock.h"
//#include "Arduino.h"

//#include <ESP8266WiFi.h>
#include "modbus.h"

#define DEBUG_LEVEL  1                                               // manages most of the print and println debug, not all but most

#if (DEBUG_LEVEL > 0U)
#define UsrLogln(...)\
		Serial.print("USR AnalogClock.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define UsrLog(...)\
		Serial.print("USR AnalogClock.cpp: ");\
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
		Serial.print("ERR AnalogClock.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define ErrLog(...)\
		Serial.print("ERR AnalogClock.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define ErrLogln(...)
#define ErrLog(...)
#endif

#if (DEBUG_LEVEL > 2U)
#define DbgLogln(...)\
		Serial.print("DBG AnalogClock.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define DbgLog(...)\
		Serial.print("DBG AnalogClock.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define DbgLogln(...)
#define DbgLog(...)
#endif

#define EEPROM_SIZE 512
#define EEPROM_LOCK_ADR 0
#define EEPROM_LOCK_VAL 0
#define H24_PWM_MAX 7840//255
#define H24_PWM_MIN 0
#define H12_PWM_MAX 7840//255
#define H12_PWM_MIN 0
#define M_PWM_MAX 8192//255
#define M_PWM_MIN 0
#define S_PWM_MAX 8192//255
#define S_PWM_MIN 0



const uint8_t hours2PWM[24]={255,0,29,52,78,104,130,156,175,193,213,234,255,0,29,52,78,104,130,156,175,193,213,234};
const uint8_t AM_PM2PWM[2] ={64,128+64};

#define PWM_RES 8

const uint32_t PWMFreq = 5000; /* 5 KHz */
const uint32_t PWMRes= PWM_RES;
const uint32_t PWMResHEX_s  = 60;
const uint32_t PWMResHEX_m  = 60;
const uint32_t PWMResHEX_12h  = 12;
const uint32_t PWMResHEX_24h  = 24;

AnalogClock::AnalogClock(void)
{
	var.h_pin = 25;
	var.m_pin = 27;
	var.s_pin = 24;
	var.bl_pin = 4;
	var.AM_PM_pin = 2;
}

void AnalogClock::Init(uint32_t  h, uint32_t  m, uint32_t  s, uint32_t bl)
{
	DbgLogln("Init 1");
	print_kln("[A] Init Analog Clock");
	var.h_pin = h;
	var.m_pin = m;
	var.s_pin = s;
	var.bl_pin = bl;
	GPIO_init(1);
	EEPROM_init();
}

void AnalogClock::Init(uint32_t  h, uint32_t  m, uint32_t  s)
{
	DbgLogln("Init 0");
	print_kln("[A] Init Analog Clock");
	var.h_pin = h;
	var.m_pin = m;
	var.s_pin = s;
	GPIO_init(0);
	EEPROM_init();
}

void AnalogClock::Init(uint32_t  h, uint32_t  m, uint32_t  s, uint32_t bl, uint32_t AM_PM )
{
	DbgLogln("Init 2");
	print_kln("[A] Init Analog Clock");
	var.h_pin = h;
	var.m_pin = m;
	var.s_pin = s;
	var.bl_pin = bl;
	var.AM_PM_pin = AM_PM;
	GPIO_init(2);
	EEPROM_init();
}

void AnalogClock::GPIO_init(uint8_t level)
{
	DbgLogln("GPIO_init H M S");
	print_k("[A] Initing GPIO ... ");
	//ledcAttachPin(var.h_pin, 0);
	//ledcAttachPin(var.m_pin, 1);
	//ledcAttachPin(var.s_pin, 2);
	
	//ledcSetup(0, PWMFreq, PWMRes);
	//ledcSetup(1, PWMFreq, PWMRes);
	//ledcSetup(2, PWMFreq, PWMRes);
	
	if (level >= 1)
	{
		DbgLogln("GPIO_init BL");
		//ledcSetup(3, PWMFreq, PWMRes);
		//ledcAttachPin(var.bl_pin, 3);
	}
	if (level >= 2)
	{
		DbgLogln("GPIO_init AM_PM");
		//ledcSetup(4, PWMFreq, PWMRes);
		//ledcAttachPin(var.AM_PM_pin, 4);
	}
	print_kln("Done");	
}

void AnalogClock::Set_time(uint8_t hour, uint8_t minute, uint8_t second)
{
	var.hours24 = hour;
	if (hour >11)
	{
		var.AM_PM = 1;
	}
	else
	{
		var.AM_PM = 0;
	}
	
	if (hour >12)
	{
		hour = hour - 12;
	}
	if (hour >23)
	{
		hour = 23;
	}
	if (minute >59)
	{
		minute = 59;
	}
	if (second>59)
	{
		second = 59;
	}
	var.hours12 = hour;
	var.minutes = minute;
	var.seconds = second;
}

inline void AnalogClock::PWM_h24(uint32_t h)
{
	DbgLog("PWM_24h ");
	DbgLogln(h,DEC);
	//ledcWrite(0,h);
	write_hours24(h);
}

inline void AnalogClock::PWM_h12(uint32_t h)
{
	DbgLog("PWM_12h ");
	DbgLogln(h,DEC);
	//ledcWrite(0,h);
	write_hours12(h);
}

inline void AnalogClock::PWM_m(uint32_t m)
{
	DbgLog("PWM_m ");
	DbgLogln(m,DEC);
	//ledcWrite(1,m);
	write_minutes(m);
}

inline void AnalogClock::PWM_s(uint32_t s)
{
	DbgLog("PWM_s ");
	DbgLogln(s,DEC);
	//ledcWrite(2,s);
	write_seconds(s);
}

void AnalogClock::SetProgress(uint8_t a)
{
	DbgLogln("Set Progress to ");
	DbgLogln(a,DEC);

	if (a>100) a=100;
	//PWM_h(map(a,0,100,0,PWMResHEX));
	//PWM_m(map(a,0,100,0,PWMResHEX));
	//PWM_s(map(a,0,100,0,PWMResHEX));
	PWM_h24(map(a,0,100,0,PWMResHEX_24h));
	PWM_h12(map(a,0,100,0,PWMResHEX_12h));
	PWM_m(map(a,0,100,0,PWMResHEX_m));
	PWM_s(map(a,0,100,0,PWMResHEX_s));
}

void AnalogClock::Display(void)
{	
	DbgLogln("Display");
	//if (var.hours24_pwm !=var.hours24_pwm_last)
	//{
	//	PWM_h24(var.hours24_pwm);
	//}
	//if (var.hours12_pwm !=var.hours12_pwm_last)
	//{
	//	PWM_h12(var.hours12_pwm);
	//}
	if (var.minutes_pwm != var.minutes_pwm_last)
	{
		PWM_m(var.minutes_pwm);
		PWM_h12(var.hours12_pwm);
		PWM_h24(var.hours24_pwm);
	}
	if (var.seconds_pwm != var.seconds_pwm_last)
	{
		PWM_s(var.seconds_pwm);
	}

	var.hours24_pwm_last = var.hours24_pwm;
	var.hours12_pwm_last = var.hours12_pwm;
	var.minutes_pwm_last = var.minutes_pwm;
	var.seconds_pwm_last = var.seconds_pwm;
}	

void AnalogClock::Convert(void)
{
	DbgLogln("Convert");
	//var.hours24_pwm = map(var.hours24,0,24,var.cal_val.hours24_min,var.cal_val.hours24_max);
	var.hours24_pwm = var.hours24;
	DbgLog("PWM hours24_pwm = ");
	DbgLogln(var.hours24_pwm,DEC);
	//var.hours12_pwm = map(var.hours12,0,12,var.cal_val.hours12_min,var.cal_val.hours12_max);
	var.hours12_pwm = var.hours12;
	DbgLog("PWM hours12_pwm = ");
	DbgLogln(var.hours12_pwm,DEC);
	//var.minutes_pwm = map(var.minutes,0,60,var.cal_val.minutes_min,var.cal_val.minutes_max);
	var.minutes_pwm = var.minutes;
	DbgLog("PWM minutes_pwm = ");
	DbgLogln(var.minutes_pwm,DEC);
	//var.seconds_pwm = map(var.seconds,0,60,var.cal_val.seconds_min,var.cal_val.seconds_max);
	var.seconds_pwm = var.seconds;
	DbgLog("PWM seconds_pwm = ");
	DbgLogln(var.seconds_pwm,DEC);
}

uint32_t AnalogClock::Get_hours12()
{
	return (var.hours12);
}

uint32_t AnalogClock::Get_hours24()
{
	return (var.hours24);
}

uint32_t AnalogClock::Get_minutes()
{
	return (var.minutes);
}

uint32_t AnalogClock::Get_seconds()
{
	return (var.seconds);
}

uint32_t AnalogClock::Get_hours12_pwm()
{
	return (var.hours12_pwm);
}

uint32_t AnalogClock::Get_minutes_pwm()
{
	return (var.minutes_pwm);
}

uint32_t AnalogClock::Get_AM_PM_pwm()
{
	return (AM_PM2PWM[is_AMorPM()]);
}

uint32_t AnalogClock::Get_seconds_pwm()
{
	return (var.seconds_pwm);
}

void AnalogClock::DisplayAM_PM()
{	
	//ledcWrite(4,AM_PM2PWM[is_AMorPM()]);
}

void AnalogClock::Test()
{
	//ledcWrite(0,PWMResHEX_s);
	//ledcWrite(1,PWMResHEX_m);
	//ledcWrite(2,PWMResHEX_12h);
	//ledcWrite(2,PWMResHEX_24h);
}

void AnalogClock::Backlight_level(uint8_t bl_level)
{
	var.bl_level = bl_level;
	//ledcWrite(3, bl_level);
}

void AnalogClock::Backlight(uint8_t bl)
{
	var.bl_state = bl;
	//digitalWrite(3, var.bl_state);
}

void AnalogClock::EEPROM_init(void)
{
	EEPROM.begin(EEPROM_SIZE);
	if (EEPROM.read(EEPROM_LOCK_ADR) != EEPROM_LOCK_VAL)
	{	
		print_kln("[A] Init EEPROM");
		Calibration();
		EEPROM.write(EEPROM_LOCK_ADR,EEPROM_LOCK_VAL);
		Save_cal();
	}
	else
	{
		Load_cal();
	}
}

void AnalogClock::EEPROM_erase(void)
{
	print_kln("[A] Erasing EEPROM ... ");
	for (int i=0;i< EEPROM_SIZE;i++)
	{
		EEPROM.write(i,0xFF);
	}
	print_kln("Done");

	print_k("[A] Save EEPROM ... ");
	Save_cal();
	print_kln("Done");
}

void AnalogClock::EEPROM_commit(void)
{
	print_k("[A] Commiting EEPROM ... ");
	EEPROM.commit();
	print_kln("Done");
}

void AnalogClock::Save_cal()
{
	print_k("[A] Save Callibration ... ");

	for (uint8_t i=0;i<sizeof(var.cal_val.raw_cal);i++)
	{
		EEPROM.write(10+i,var.cal_val.raw_cal[i]);
	}
	print_kln("Done");
	#ifdef ENABLE_EEPROM
	UsrLog("Commiting EEPROM... ");
	EEPROM.commit();
	UsrLogln("Done");
	#endif
	
}

void AnalogClock::Load_cal()
{
	print_k("[A] Loading Callibration ... ");
	for (uint8_t i=0;i<sizeof(var.cal_val.raw_cal);i++)
	{
		var.cal_val.raw_cal[i] = EEPROM.read(10+i);
	}
	print_kln("Done");
	print_EEPROM();
	
}

void AnalogClock::print_EEPROM(void)
{
	uint8_t value;
	uint8_t x = 0;
	print_k("------------------------\n\r");
	for (int i = 0;i<sizeof(var.cal_val.raw_cal);i++)
	{
		x++;
		value = EEPROM.read(i);
		if (value <16) print_k("0");
		print_k(value,HEX);
		print_k(" ");
		if (x>7)
		{
			x=0;
			print_k("\n\r");
		}			
	}
	print_k("------------------------\n\r");
}

void AnalogClock::Calibration(void)
{
	print_k("[A] Appling calibration values ... ");
	Set_Calibration_seconds_min(S_PWM_MIN);
	Set_Calibration_seconds_max(S_PWM_MAX);
	Set_Calibration_minutes_min(M_PWM_MIN);
	Set_Calibration_minutes_max(M_PWM_MAX);
	Set_Calibration_hours12_min(H12_PWM_MIN);
	Set_Calibration_hours12_max(H12_PWM_MAX);
	Set_Calibration_hours24_min(H24_PWM_MIN);
	Set_Calibration_hours24_max(H24_PWM_MAX);
	print_kln("Done");
}

void AnalogClock::test(void)
{
	print_k("[A] Testing ... ");
	Set_time(0,0,0);
    Convert();
    Display();
    delay(1000);
    Set_time(24,60,60);
    Convert();
    Display();
    delay(1000);
	Set_time(0,0,0);
    Convert();
    Display();
	delay(1000);
    Set_time(24,60,60);
    Convert();
    Display();
	print_kln("Done");
}


uint8_t AnalogClock::is_AMorPM()
{
	return (var.AM_PM);
}

uint16_t AnalogClock::Get_Calibration_hours24_min()
{
	return (var.cal_val.hours24_min);
}

uint16_t AnalogClock::Get_Calibration_hours24_max()
{
	return (var.cal_val.hours24_max);
}

uint16_t AnalogClock::Get_Calibration_hours12_min()
{
	return (var.cal_val.hours12_min);
}

uint16_t AnalogClock::Get_Calibration_hours12_max()
{
	return (var.cal_val.hours12_max);
}

uint16_t AnalogClock::Get_Calibration_minutes_min()
{
	return (var.cal_val.minutes_min);
}

uint16_t AnalogClock::Get_Calibration_minutes_max()
{
	return (var.cal_val.minutes_max);
}

uint16_t AnalogClock::Get_Calibration_seconds_min()
{
	return (var.cal_val.seconds_min);
}

uint16_t AnalogClock::Get_Calibration_seconds_max()
{
	return (var.cal_val.seconds_max);
}

void AnalogClock::Set_Calibration_hours12_min(uint16_t a)
{
	var.cal_val.hours12_min = a;
}

void AnalogClock::Set_Calibration_hours12_max(uint16_t a)
{
	var.cal_val.hours12_max = a;
}

void AnalogClock::Set_Calibration_hours24_min(uint16_t a)
{
	var.cal_val.hours24_min = a;
}

void AnalogClock::Set_Calibration_hours24_max(uint16_t a)
{
	var.cal_val.hours24_max = a;
}

void AnalogClock::Set_Calibration_minutes_min(uint16_t a)
{
	var.cal_val.minutes_min = a;
}

void AnalogClock::Set_Calibration_minutes_max(uint16_t a)
{
	var.cal_val.minutes_max = a;
}

void AnalogClock::Set_Calibration_seconds_min(uint16_t a)
{
	var.cal_val.seconds_min = a;
}

void AnalogClock::Set_Calibration_seconds_max(uint16_t a)
{
	var.cal_val.seconds_max = a;
}

void AnalogClock::Set_present_seconds_flag(uint8_t a)
{
	var.seconds_present_f = a;
}

void AnalogClock::Set_present_minutes_flag(uint8_t a)
{
	var.minutes_present_f = a;
}

void AnalogClock::Set_present_hours12_flag(uint8_t a)
{
	var.hours12_present_f = a;
}

void AnalogClock::Set_present_hours24_flag(uint8_t a)
{
	var.hours24_present_f = a;
}

void AnalogClock::Set_present_days_flag(uint8_t a)
{
	var.days_present_f = a;
}

void AnalogClock::Set_present_weeks_flag(uint8_t a)
{
	var.weeks_present_f = a;
}

void AnalogClock::Set_present_lun2_flag(uint8_t a)
{
	var.lun2_present_f = a;
}

void AnalogClock::Set_present_lun1_flag(uint8_t a)
{
	var.lun1_present_f = a;
}

uint8_t AnalogClock::Update_seconds_cal_values(uint8_t value)
{
	return send_cal_values(value, var.cal_val.seconds_min, var.cal_val.seconds_max, 0, 60, SEC_ADR);
}

uint8_t AnalogClock::Update_minutes_cal_values(uint8_t value)
{
	return send_cal_values(value, var.cal_val.minutes_min, var.cal_val.minutes_max, 0, 60, MIN_ADR);
}

uint8_t AnalogClock::Update_hours12_cal_values(uint8_t value)
{
	return send_cal_values(value, var.cal_val.hours12_min , var.cal_val.hours12_max, 0, 12, HUR12_ADR);
}

uint8_t AnalogClock::Update_hours24_cal_values(uint8_t value)
{
	return send_cal_values(value, var.cal_val.hours24_min, var.cal_val.hours24_max, 0, 24, HUR24_ADR);
}

void AnalogClock::Send_cal_values(void)
{
	if (var.seconds_present_f == 1)
	{
		if (var.seconds_cal_f == 1)
		{
			if(Update_seconds_cal_values(var.seconds_pwm))
			{
				var.seconds_cal_f = 0;
			}
		}
	}
	else
	{
		var.seconds_cal_f = 1;
	}
	//-------------
	if (var.minutes_present_f == 1)
	{
		if (var.minutes_cal_f == 1)
		{
			if(Update_minutes_cal_values(var.minutes_pwm))
			{
				var.minutes_cal_f = 0;
			}
		}
	}
	else
	{
		var.minutes_cal_f = 1;
	}
		//-------------
	if (var.hours12_present_f == 1)
	{
		if (var.hours12_cal_f == 1)
		{
			if(Update_hours12_cal_values(var.hours12_pwm))
			{
				var.hours12_cal_f = 0;	
			}
		}
	}
	else
	{
		var.hours12_cal_f = 1;
	}
			//-------------
	if (var.hours24_present_f == 1)
	{
		if (var.hours24_cal_f == 1)
		{
			if (Update_hours24_cal_values(var.hours24_pwm))
			{
				var.hours24_cal_f = 0;
			}
		}
	}
	else
	{
		var.hours24_cal_f = 1;
	}
	
}