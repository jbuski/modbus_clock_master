#ifndef AnalogClock_h
#define AnalogClock_h
#include <EEPROM.h>
#include "Arduino.h"

#define SEC_ADR 0x01
#define MIN_ADR 0x02
#define HUR12_ADR 0x03
#define HUR24_ADR 0x04

struct Variable{
	//PWM GPIOS
	uint32_t h_pin;
	uint32_t m_pin;
	uint32_t s_pin;
	uint32_t bl_pin;
	uint32_t AM_PM_pin;
	//backlight
	uint8_t bl_level;
	uint8_t bl_state:1;
	//Time
	uint8_t hours24;
	uint8_t hours12;
	uint8_t minutes;
	uint8_t seconds;
	//AM_PM 
	uint8_t AM_PM;
	//pwm values
	uint16_t hours12_pwm;
	uint16_t hours24_pwm;
	uint16_t minutes_pwm;
	uint16_t seconds_pwm;
	uint16_t AM_PM_pwm;
	uint16_t hours12_pwm_last;
	uint16_t hours24_pwm_last;
	uint16_t minutes_pwm_last;
	uint16_t seconds_pwm_last;
	
	uint8_t lun2_present_f:1;
	uint8_t lun1_present_f:1;
	uint8_t days_present_f:1;
	uint8_t weeks_present_f:1;
	uint8_t hours12_present_f:1;
	uint8_t hours24_present_f:1;
	uint8_t minutes_present_f:1;
	uint8_t seconds_present_f:1;

	uint8_t seconds_cal_f:1;
	uint8_t minutes_cal_f:1;
	uint8_t hours12_cal_f:1;
	uint8_t hours24_cal_f:1;
	uint8_t weeks_cal_f:1;
	uint8_t days_cal_f:1;
	uint8_t lun1_cal_f:1;
	uint8_t lun2_cal_f:1;

	//cal values stored in EEPROM
	union cal_val_d
		{
			struct
			{
				uint16_t lun2_min;
				uint16_t lun2_max;				
				uint16_t lun1_min;
				uint16_t lun1_max;
				uint16_t weeks_min;
				uint16_t weeks_max;
				uint16_t days_min;
				uint16_t days_max;
				uint16_t hours24_min;
				uint16_t hours24_max;
				uint16_t hours12_min;
				uint16_t hours12_max;
				uint16_t minutes_min;
				uint16_t minutes_max;
				uint16_t seconds_min;
				uint16_t seconds_max;
			};
			uint8_t raw_cal[32];
		}cal_val;
};

class AnalogClock {
	public: 
		AnalogClock(void);
		void Init(uint32_t  h, uint32_t  m, uint32_t  s);
		void Init(uint32_t  h, uint32_t  m, uint32_t  s, uint32_t bl);
		void Init(uint32_t  h, uint32_t  m, uint32_t  s, uint32_t bl ,uint32_t AM_PM);
		void Init(void);
		void Calibration(void);
		void Set_time(uint8_t hour, uint8_t minute, uint8_t second);
		void Display(void);
		void Test(void);
		void Backlight(uint8_t bl);
		void Backlight_level(uint8_t bl_level);
		void Load_cal(void);
		void Save_cal(void);
		uint8_t is_AMorPM(void);
		void DisplayAM_PM(void);
		void Convert(void);
		uint32_t Get_hours24();
		uint32_t Get_hours12();
		uint32_t Get_minutes();
		uint32_t Get_seconds();
		uint32_t Get_hours24_pwm(void);
		uint32_t Get_hours12_pwm(void);
		uint32_t Get_minutes_pwm(void);
		uint32_t Get_seconds_pwm(void);
		uint32_t Get_AM_PM_pwm();
		uint16_t Get_Calibration_hours24_min(void);
		uint16_t Get_Calibration_hours24_max(void);
		uint16_t Get_Calibration_hours12_min(void);
		uint16_t Get_Calibration_hours12_max(void);
		uint16_t Get_Calibration_minutes_min(void);
		uint16_t Get_Calibration_minutes_max(void);
		uint16_t Get_Calibration_seconds_min(void);
		uint16_t Get_Calibration_seconds_max(void);
		void Set_Calibration_hours24_min(uint16_t a);
		void Set_Calibration_hours24_max(uint16_t a);
		void Set_Calibration_hours12_min(uint16_t a);
		void Set_Calibration_hours12_max(uint16_t a);
		void Set_Calibration_minutes_min(uint16_t a);
		void Set_Calibration_minutes_max(uint16_t a);
		void Set_Calibration_seconds_min(uint16_t a);
		void Set_Calibration_seconds_max(uint16_t a);
		void SetProgress(uint8_t a);
		void EEPROM_erase(void);
		void EEPROM_commit(void);
		void print_EEPROM(void);
		void test(void);

		void Set_present_seconds_flag(uint8_t a);
		void Set_present_minutes_flag(uint8_t a);
		void Set_present_hours12_flag(uint8_t a);
		void Set_present_hours24_flag(uint8_t a);
		void Set_present_days_flag(uint8_t a);
		void Set_present_weeks_flag(uint8_t a);
		void Set_present_lun2_flag(uint8_t a);
		void Set_present_lun1_flag(uint8_t a);
		void Send_cal_values(void);
		uint8_t Update_seconds_cal_values(uint8_t value);
		uint8_t Update_minutes_cal_values(uint8_t value);
		uint8_t Update_hours12_cal_values(uint8_t value);
		uint8_t Update_hours24_cal_values(uint8_t value);
	private:
		Variable var;
		void EEPROM_init(void);
		void GPIO_init(uint8_t level);
		inline void PWM_lun2(uint32_t h);
		inline void PWM_lun1(uint32_t h);
		inline void PWM_weeks(uint32_t h);
		inline void PWM_days(uint32_t d);
		inline void PWM_h24(uint32_t h);
		inline void PWM_h12(uint32_t h);
		inline void PWM_m(uint32_t m);
		inline void PWM_s(uint32_t s);
};

#endif
