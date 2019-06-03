#include <iostream>
#include "math.h"

#define ZENITH 90.8f
#define DAY_HOURS   24.0f

#pragma pack(1)

typedef union t_c_time
{
    uint32_t dworld;
    struct
    {
        unsigned seconds : 6;
        unsigned minutes : 6;
        unsigned month : 4;
        unsigned hours : 5;
        unsigned date : 5;
        unsigned year : 6;
    };
} t_CTime;


#define JULIAN_DATE_BASE     2440588   // Unix epoch time in Julian calendar (UnixTime = 00:00:00 01.01.1970 => JDN = 2440588)



uint16_t calcDayOfYear(int day, int month, int year)
{
    uint16_t n1 = floorf(275 * month / 9);
    uint16_t n2 = floorf(( month + 9 ) / 12);
    uint16_t n3 = ( 1 + floorf(( year - 4 * floorf(year / 4) + 2 ) / 3) );
    return (n1 - ( n2 * n3 ) + day - 30);
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


/*
* Добавляет секунды к времени
*/
t_CTime addSecToCTime(t_CTime *c_time, const uint32_t sec)
{
    t_CTime result;
    uint32_t ts = rtcToEpoch(c_time);
    ts = ts + sec;
    rtcFromEpoch(ts, &result);
    return result;
}


float calculateSunRiseSet(bool IsSunRise, int day_num, float lat, float lon, int time_offset)
{

    //2. convert the longitude to hour value and calculate an approximate time
    const float alpha = M_PI / 180;
    const float lng_hour = lon / 15;
    float t;
    if ( IsSunRise )
    {
        t = day_num + ( ( 6 - lng_hour ) / DAY_HOURS );   //if setting time is desired:
    }
    else
    {
        t = day_num + ( ( 18 - lng_hour ) / DAY_HOURS );
    }

    //3. calculate the Sun mean anomaly
    const float m = ( 0.9856 * t ) - 3.289;

    //4. calculate the Sun true longitude
    const float l = fmodf(m + ( 1.916 * sinf(alpha * m) ) + ( 0.020 * sinf(2 * alpha * m) ) + 282.634, 360.0);

    //5a. calculate the Sun right ascension
    float ra = fmodf(( 180 / M_PI )*atanf(0.91764 * tanf(alpha * l)), 360.0);

    //5b. right ascension value needs to be in the same quadrant as L
    const float l_quadrant = floorf(l / 90) * 90;
    const float ra_quadrant = floorf(ra / 90) * 90;
    ra = ra + ( l_quadrant - ra_quadrant );

    //5c. right ascension value needs to be converted into hours
    ra = ra / 15;

    //6. calculate the Sun declination
    const float sin_dec = 0.39782 * sinf(alpha * l);
    const float cos_dec = cosf(asinf(sin_dec));

    //7a. calculate the Sun local hour angle
    const float cos_h = ( cosf(alpha * ZENITH) - ( sin_dec * sinf(alpha * lat) ) ) / ( cos_dec * cosf(alpha * lat) );
    printf("cos_h %f\n\r", cos_h);
    if ( cos_h > 1 )
    {
        //printf("the sun never rises %f\n\r", cos_h);
        return 48;
    }

    if ( cos_h < -1 )
    {
        //printf("the sun never sets %f\n\r", cos_h);
        return -48;
    }

    //7b. finish calculating H and convert into hours
    float h;
    if ( IsSunRise )
    {
        h = 360 - ( 180 / M_PI )* acosf(cos_h);   //   if rising time is desired:
    }
    else
    {
        h = ( 180 / M_PI )* acosf(cos_h);
    }

    h = h / 15;

    //8. calculate local mean time of rising/setting
    t = h + ra - ( 0.06571 * t ) - 6.622;

    //9. adjust back to UTC, convert UT value to local time zone of latitude/longitude
    return ( fmodf(t - lng_hour, DAY_HOURS) + time_offset );
}

//#define LATITUDE   (59.9342802f)
//#define LONGITUDE  (30.3350986f)

//#define LATITUDE   (70.5667f)
//#define LONGITUDE  (27.0167f)

//#define LATITUDE (68.9792f)
//#define LONGITUDE (33.0925f)

#define LATITUDE (-68.9792f)
#define LONGITUDE  (33.0925f)

int main(void)
{
    t_CTime day_time, sun_rise, sun_set;
    float rt, st;
    float sunrise_hr, sunrise_min, sunset_hr2, sunset_min2;
    uint32_t total_day, total_night;

    day_time.dworld = 0;
    day_time.month = 1;
    day_time.date = 1;
    day_time.year = 19;
    day_time.minutes = 0;

    sun_rise = sun_set = day_time;
    int num_day = calcDayOfYear(1,1,2019);
    for (int i = num_day; i < 365; i++)
    {

        rt = calculateSunRiseSet(true, i, LATITUDE, LONGITUDE, 3);
        st = calculateSunRiseSet(false, i,LATITUDE, LONGITUDE, 3);

        sunrise_hr = fmodf(DAY_HOURS + rt,DAY_HOURS);
        sunrise_min = modf(fmodf(DAY_HOURS + rt,DAY_HOURS),&sunrise_hr)*60;
        sunset_hr2 = fmodf(DAY_HOURS + st,DAY_HOURS);
        sunset_min2 = modf(fmodf(DAY_HOURS + st,DAY_HOURS),&sunset_hr2)*60;

        sun_rise = day_time;
        sun_set = day_time;

        sun_rise.hours = roundf(sunrise_hr);
        sun_rise.minutes = roundf(sunrise_min);


        sun_set.hours = roundf(sunset_hr2);
        sun_set.minutes = roundf(sunset_min2);

        if (( rt < 48 && rt > -48) && ( st < 48 && st > -48)) {
            uint32_t utr = rtcToEpoch(&sun_rise);
            uint32_t uts = rtcToEpoch(&sun_set);

            if ( uts >= utr) {
                total_day = (uts - utr);
                total_night = 86400 - (uts - utr);
            }
            else
            {
                total_night = (utr - uts);
                total_day = 86400 - (utr - uts);
            }
        }
        else if ( rt == -48 && st == -48)
        {
            total_day = 86400;
            total_night = 0;
        }
        else if ( rt == 48 && st == 48)
        {
            total_day = 0;
            total_night = 86400;
        }

        //Calc night time
        t_CTime night_start, night_end;
        t_CTime day_start, day_end;

        day_start = sun_rise;
        day_end = addSecToCTime(&day_start, total_day);

        night_start = sun_set;
        night_end = addSecToCTime(&night_start, total_night);


        int daytime_hr = total_day / 3600;
        int daytime_min = (total_day % 3600) / 60;

        int night_hr = total_night / 3600;
        int night_min = (total_night % 3600) / 60;



        day_time = addSecToCTime(&day_time, 86400);

        printf("Day: %d\t SunRise: %02.0f:%02.0f \t SunSet: %02.0f:%02.0f \n\r", i, sunrise_hr, sunrise_min, sunset_hr2, sunset_min2);
        printf("day_start   %d.%d.%d %02d:%02d:%02d \n\r", day_start.date, day_start.month, day_start.year, day_start.hours, day_start.minutes, day_start.seconds);
        printf("day_end     %d.%d.%d %02d:%02d:%02d \n\r", day_end.date, day_end.month, day_end.year, day_end.hours, day_end.minutes, day_end.seconds);
        printf("night_start %d.%d.%d %02d:%02d:%02d \n\r", night_start.date, night_start.month, night_start.year, night_start.hours, night_start.minutes, night_start.seconds);
        printf("night_end   %d.%d.%d %02d:%02d:%02d \n\r", night_end.date, night_end.month, night_end.year, night_end.hours, night_end.minutes, night_end.seconds);
        printf("dur_day: %.2d:%.2d \t dur_night: %.2d:%.2d \n\r", daytime_hr, daytime_min, night_hr, night_min );


    }
}

int main1() {
    t_CTime sun_rise, sun_set;
    int num_day = calcDayOfYear(1,1,2019);
    float rt, st, sunrise_hr, sunrise_min, sunset_hr2, sunset_min2;

    for (int i = num_day; i < 366; i++) {

        rt = calculateSunRiseSet(true, i, LATITUDE, LONGITUDE, 3);
        st = calculateSunRiseSet(false, i,LATITUDE, LONGITUDE, 3);

        sunrise_hr = fmodf(DAY_HOURS + rt,DAY_HOURS);
        sunrise_min = modf(fmodf(DAY_HOURS + rt,DAY_HOURS),&sunrise_hr)*60;
        sunset_hr2 = fmodf(DAY_HOURS + st,DAY_HOURS);
        sunset_min2 = modf(fmodf(DAY_HOURS + st,DAY_HOURS),&sunset_hr2)*60;

        sun_rise.date = 1;
        sun_rise.month = 1;
        sun_rise.year = 19;
        sun_rise.hours = sunrise_hr;
        sun_rise.minutes = sunrise_min;
        sun_rise.seconds = 0;

        sun_set.date = 1;
        sun_set.month = 1;
        sun_set.year = 19;
        sun_set.hours = sunset_hr2;
        sun_set.minutes = sunset_min2;
        sun_set.seconds = 0;

        uint32_t utr;
        uint32_t uts;
        uint32_t total_day;
        uint32_t total_night;

        if (( rt < DAY_HOURS && rt > -DAY_HOURS) && ( st < DAY_HOURS && st > -DAY_HOURS)) {
            utr = rtcToEpoch(&sun_rise);
            uts = rtcToEpoch(&sun_set);

            if ( uts >= utr) {
                total_day = uts - utr;
                total_night = 86400 - (uts - utr);
            }
            else
            {
                total_night = utr - uts;
                total_day = 86400 - (utr - uts);
            }
        }
        else if ( rt == -DAY_HOURS && st == -DAY_HOURS)
        {
            total_day = 86400;
            total_night = 0;
        }
        else if ( rt == DAY_HOURS && st == DAY_HOURS)
        {
            total_day = 0;
            total_night = 86400;
        }

            int daytime_hr = total_day / 3600;
            int daytime_min = (total_day % 3600) / 60;

            int night_hr = total_night / 3600;
            int night_min = (total_night % 3600) / 60;



        printf("rt %f \t set %f \n\r", rt, st );
        printf("Day: %d\t SunRise: %02.0f:%02.0f \t SunSet: %02.0f:%02.0f \t daytime: %.2d:%.2d \t nighttime: %.2d:%.2d \n\r", i, sunrise_hr, sunrise_min, sunset_hr2, sunset_min2, daytime_hr, daytime_min, night_hr, night_min );
    }

    return 0;
}