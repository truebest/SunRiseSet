#ifndef _SUNRISESET_H_
#define _SUNRISESET_H_

bool SunRS_CalcValues(t_CTime cur_time, float lat, float lon, int time_offset, int daylight_savings);
t_CTime SunRS_GetDayStart(void);
t_CTime SunRS_GetDayEnd(void);
t_CTime SunRS_GetNightStart(void);
t_CTime SunRS_GetNightEnd(void);

#endif
