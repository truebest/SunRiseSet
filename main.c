
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#include "typedef.h"
#include "sunriseset.h"

#define LATITUDE   (53.183968f)
#define LONGITUDE  (43.981667f)

int main() {
    t_CTime current_time, tmp_time;
    current_time.dworld = 0;
    current_time.date = 1;
    current_time.month = 2;
    current_time.year = 21;


    SunRS_CalcValues(current_time, LATITUDE, LONGITUDE, 3, 0);

    tmp_time = SunRS_GetDayStart();
    printf("Day start\t %d.%d.%d %02d:%02d \n\r", tmp_time.date, tmp_time.month, tmp_time.year, tmp_time.hours, tmp_time.minutes);
    tmp_time = SunRS_GetDayEnd();
    printf("Day end  \t %d.%d.%d %02d:%02d \n\r", tmp_time.date, tmp_time.month, tmp_time.year, tmp_time.hours, tmp_time.minutes);
    tmp_time = SunRS_GetNightStart();
    printf("Night start\t %d.%d.%d %02d:%02d \n\r", tmp_time.date, tmp_time.month, tmp_time.year, tmp_time.hours, tmp_time.minutes);
    tmp_time = SunRS_GetNightEnd();
    printf("Night end\t %d.%d.%d %02d:%02d \n\r", tmp_time.date, tmp_time.month, tmp_time.year, tmp_time.hours, tmp_time.minutes);

    return 0;
}