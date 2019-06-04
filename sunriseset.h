//
// Created by Pavel on 03.06.2019.
//

#ifndef UNTITLED_SUNRISESET_H
#define UNTITLED_SUNRISESET_H

bool SunRS_CalcValues(t_CTime cur_time, float lat, float lon, int time_offset, int daylight_savings);
t_CTime SunRS_GetDayStart(void);
t_CTime SunRS_GetDayEnd(void);
t_CTime SunRS_GetNightStart(void);
t_CTime SunRS_GetNightEnd(void);

#endif //UNTITLED_SUNRISESET_H
