
#include "modbus.h"
#include "AnalogClock.h"

#define DEBUG_LEVEL  1                                              // manages most of the print and println debug, not all but most

#if (DEBUG_LEVEL > 0U)
#define UsrLogln(...)\
		Serial.print("USR modbus.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define UsrLog(...)\
		Serial.print("USR modbus.cpp: ");\
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
		Serial.print("ERR modbus.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define ErrLog(...)\
		Serial.print("ERR modbus.cpp: ");\
		Serial.print(__VA_ARGS__)
#define print_e(...)\
		Serial.print(__VA_ARGS__)
#define print_eln(...)\
		Serial.print(__VA_ARGS__)
#else
#define ErrLogln(...)
#define ErrLog(...)
#define print_e(...)
#define print_eln(...)
#endif

#if (DEBUG_LEVEL > 2U)
#define DbgLogln(...)\
		Serial.print("DBG modbus.cpp: ");\
		Serial.println(__VA_ARGS__)
		
#define DbgLog(...)\
		Serial.print("DBG modbus.cpp: ");\
		Serial.print(__VA_ARGS__)
#else
#define DbgLogln(...)
#define DbgLog(...)
#endif

extern AnalogClock my_clock;

void modbus_init(void)
{
    if (!ModbusRTUClient.begin(115200)) 
    {
        Serial.println("[F] Failed to start Modbus RTU Client!");
        ESP.restart();
    }
    else
    {
        Serial.println("[M] Modbus RTU Client OK!");
    }
}

void write_seconds(uint8_t s) //Address 0x01
{
    DbgLogln("s = ");
    DbgLogln(s,DEC);
    ModbusRTUClient.beginTransmission(1, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(s);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x01 R 0x00 V 0x");
        print_eln(s,HEX);
        ErrLogln(ModbusRTUClient.lastError());
        my_clock.Set_present_seconds_flag(0);
    } 
    else 
    {
        DbgLogln("success A 0x01 R 0x00");
        my_clock.Set_present_seconds_flag(1);
    }

}

void write_minutes(uint8_t m) //Address 0x02
{
    DbgLogln("m = ");
    DbgLogln(m,DEC);
    ModbusRTUClient.beginTransmission(2, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(m);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x02 R 0x00 V 0x");
        print_eln(m,HEX);
        ErrLogln(ModbusRTUClient.lastError());
    } 
    else 
    {
        DbgLogln("success A 0x02 R 0x00");
    }
    delay(100);

}

void write_hours12(uint8_t h) //Address 0x03
{
    DbgLogln("h = ");
    DbgLogln(h,DEC);
    ModbusRTUClient.beginTransmission(3, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(h);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x03 R 0x00 V 0x");
        print_eln(h,HEX);
        ErrLogln(ModbusRTUClient.lastError());
    } 
    else 
    {
        DbgLogln("success A 0x03 R 0x00");
    }
    delay(100);

}

void write_hours24(uint8_t h) //Address 0x04
{
    DbgLogln("h = ");
    DbgLogln(h,DEC);
    ModbusRTUClient.beginTransmission(4, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(h);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x04 R 0x00 V 0x");
        print_eln(h,HEX);
        ErrLogln(ModbusRTUClient.lastError());
    } 
    else 
    {
        DbgLogln("success A 0x04 R 0x00");
    }
    delay(100);

}

void write_week(uint8_t w) //Address 0x05
{
    DbgLogln("w = ");
    DbgLogln(w,DEC);
    ModbusRTUClient.beginTransmission(5, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(w);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x05 R 0x00 V 0x");
        print_eln(w,HEX);
        ErrLogln(ModbusRTUClient.lastError());
    } 
    else 
    {
        DbgLogln("success A 0x05 R 0x00");
    }
    delay(100);

}

void write_day(uint8_t d) //Address 0x06
{
    DbgLogln("d = ");
    DbgLogln(d,DEC);
    ModbusRTUClient.beginTransmission(6, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(d);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed! A 0x06 R 0x00 V 0x");
        print_eln(d,HEX);
        ErrLogln(ModbusRTUClient.lastError());
    } 
    else 
    {
        DbgLogln("success A 0x06 R 0x00");
    }
    delay(100);
}

uint8_t send_cal_values(uint16_t val, uint16_t pwm_min,uint16_t pwm_max, uint16_t range_min,uint16_t range_max,uint16_t adr)
{
    DbgLogln("Adr = ");
    DbgLogln(val,DEC);
    ModbusRTUClient.beginTransmission(adr, HOLDING_REGISTERS, 0x00, 5);
    ModbusRTUClient.write(val);
    ModbusRTUClient.write(pwm_min);
    ModbusRTUClient.write(pwm_max);
    ModbusRTUClient.write(range_min);
    ModbusRTUClient.write(range_max);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed!");
        ErrLogln(ModbusRTUClient.lastError());
        return 0;
    } 
    else 
    {
        DbgLogln("success!");
        return 1;
    }
    delay(100);
}

uint8_t send_reset(uint16_t adr)
{
    DbgLogln("Adr = ");
    DbgLogln(val,DEC);
    ModbusRTUClient.beginTransmission(adr, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(RESET_REGISTER);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed!");
        ErrLogln(ModbusRTUClient.lastError());
        return 0;
    } 
    else 
    {
        DbgLogln("success!");
        return 1;
    }
    delay(100);
}

uint8_t send_save(uint16_t adr)
{
    DbgLogln("Adr = ");
    DbgLogln(val,DEC);
    ModbusRTUClient.beginTransmission(adr, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(SAVE_REGISTER);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed!");
        ErrLogln(ModbusRTUClient.lastError());
        return 0;
    } 
    else 
    {
        DbgLogln("success!");
        return 1;
    }
    delay(100);
}

uint8_t send_load(uint16_t adr)
{
    DbgLogln("Adr = ");
    DbgLogln(val,DEC);
    ModbusRTUClient.beginTransmission(adr, HOLDING_REGISTERS, 0x00, 1);
    ModbusRTUClient.write(LOAD_REGISTER);
    if (!ModbusRTUClient.endTransmission()) 
    {
        ErrLog("failed!");
        ErrLogln(ModbusRTUClient.lastError());
        return 0;
    } 
    else 
    {
        DbgLogln("success!");
        return 1;
    }
    delay(100);
}