//
// Created by Pavel on 03.06.2019.
//

#include <stdint.h>
#include "typedef.h"
#include "f_helpers.h"


#define JULIAN_DATE_BASE     2440588   // Unix epoch time in Julian calendar (UnixTime = 00:00:00 01.01.1970 => JDN = 2440588)

// Convert epoch time to Date/Time structures
void rtcFromEpoch(uint32_t epoch, t_CTime *c_time)
{
    // These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

    const uint64_t jd = ( ( epoch + 43200 ) / ( 86400 >> 1 ) ) + ( 2440587 << 1 ) + 1;
    const uint64_t jdn = jd >> 1;

    uint32_t tm = epoch;
    uint32_t t1 = tm / 60;
    const int16_t sec = tm - ( t1 * 60 );
    tm = t1; t1 = tm / 60;
    const int16_t min = tm - ( t1 * 60 );
    tm = t1; t1 = tm / 24;
    const int16_t hour = tm - ( t1 * 24 );

    //int16_t dow = jdn % 7;
    const uint32_t a = jdn + 32044;
    const uint32_t b = ( ( 4 * a ) + 3 ) / 146097;
    const uint32_t c = a - ( ( 146097 * b ) / 4 );
    const uint32_t d = ( ( 4 * c ) + 3 ) / 1461;
    const uint32_t e = c - ( ( 1461 * d ) / 4 );
    const uint32_t m = ( ( 5 * e ) + 2 ) / 153;
    const int16_t day = e - ( ( ( 153 * m ) + 2 ) / 5 ) + 1;
    const int16_t month = m + 3 - ( 12 * ( m / 10 ) );
    const int16_t year = ( 100 * b ) + d - 4800 + ( m / 10 );

    c_time->year = year - 2000;
    c_time->month = month;
    c_time->date = day;
    c_time->hours = hour;
    c_time->minutes = min;
    c_time->seconds = sec;
}



// Convert Date/Time structures to epoch time
uint32_t rtcToEpoch(t_CTime *c_time)
{
    // These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

    // Calculate some coefficients
    const uint8_t a = ( 14 - c_time->month ) / 12;
    const uint16_t y = ( c_time->year + 2000 ) + 4800 - a;   // years since 1 March, 4801 BC
    const uint8_t m = c_time->month + ( 12 * a ) - 3;   // since 1 March, 4801 BC

    // Gregorian calendar date compute
    uint32_t jdn = c_time->date;
    jdn += ( 153 * m + 2 ) / 5;
    jdn += 365 * y;
    jdn += y / 4;
    jdn += -y / 100;
    jdn += y / 400;
    jdn = jdn - 32045;
    jdn = jdn - JULIAN_DATE_BASE;     // Calculate from base date
    jdn *= 86400;                      // Days to seconds
    jdn += c_time->hours * 3600;      // ... and today seconds
    jdn += c_time->minutes * 60;
    jdn += c_time->seconds;

    return jdn;
}


/*
* Добавляет секунды к структуре C_TIme
*/
t_CTime addSecToCTime(t_CTime *c_time, const uint32_t sec)
{
    t_CTime result;
    uint32_t ts = rtcToEpoch(c_time);
    ts = ts + sec;
    rtcFromEpoch(ts, &result);
    return result;
}
