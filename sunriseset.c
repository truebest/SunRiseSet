/*
 * �������� ������� ������� ������� � ������ ������ � ����������� �� ��������� � �������� �������
 * ����������� � ������� �������� �� ����������������
 * ��� ���������� ������� ������������ ��������� � ������ ������������ ������ float ������� � ����������
 * ����������� ������ ��������� �������� ������ ������� ������ ��� � ����, ����� ��� � ����
 * ����������� ��� ����: ����� ����� 
 */

#include "math.h"
#include <stdint.h>
#include <stdbool.h>
#include "typedef.h"
#include "f_helpers.h"
#include "sunriseset.h"

#define ZENITH		(90.7f)
#define DAY_HOURS	 24.0f
#define ERROR_VAL	 100

#define CALC_SUN_RISE		true
#define CALC_SUN_SET		false

static t_CTime night_end, sun_rise, day_end, sun_set;

/**
 * \brief ������������ ����� ��� � ���� �� ����
 * \param day - ����
 * \param month - �����
 * \param year - ���
 * \return - ����� ���
 */
static uint16_t calcDayOfYear(uint8_t day, uint8_t month, uint16_t year)
{
	const uint16_t n1 = floorf(275 * month / 9);
	const uint16_t n2 = floorf((month + 9) / 12);
	const uint16_t n3 = (1 + floorf((year - 4 * floorf(year / 4) + 2) / 3));
	return (n1 - (n2 * n3) + day - 30);
}

/**
 * \brief ������ �������� ������� � ������ ������. ������� �� ��������� https://edwilliams.org/sunrise_sunset_algorithm.htm
 * \param is_sun_rise - ������ ��� �����
 * \param day_num - ����� ���
 * \param lat - ������
 * \param lon - �������
 * \param time_offset - �������� ������� 
 * \param daylight_savings - ����\����
 * \return ��������� - ����� �������\������
 */
static float calcSunRiseSet(bool is_sun_rise, int day_num, float lat, float lon, int time_offset, int daylight_savings)
{
	const float alpha = M_PI / 180;
	//1. ����������� ������� � ���� �� ������� ��� ����� ��������� �� ��������� 15 �������� � ���
	const float lng_hour = lon / 15;
	//2. � ����������� �� ������� ��� ������, ��� ��������� ����������
	float t = is_sun_rise ? day_num + ((6 - lng_hour) / DAY_HOURS) : day_num + ((18 - lng_hour) / DAY_HOURS);

	//3. ������� �������� ������, ��� ������� ������ ������� ��� ������ ��������� ������ ����� (https://en.wikipedia.org/wiki/Position_of_the_Sun)
	const float m = (0.9856f * t) - 3.289f;

	//4. ������������ �������� ������� ������
	const float l = fmodf(m + (1.916f * sinf(alpha * m)) + (0.020f * sinf(2 * alpha * m)) + 282.634, 360.0f);

	//5a. ������������ ������ ��������� ������ (https://ru.wikipedia.org/wiki/������_�����������)
	float ra = fmodf((180 / M_PI) * atanf(0.91764f * tanf(alpha * l)), 360.0f);

	//5b. �������� ������� ����������� ������ ���� � ��� �� ���������, ��� � L
	const float l_quadrant = floorf(l / 90) * 90;
	const float ra_quadrant = floorf(ra / 90) * 90;
	ra = ra + (l_quadrant - ra_quadrant);

	//5c. ���������� �������� ��������� ������ ���� ���������� � ����
	ra = ra / 15;

	//6. ������������ ��������� ������
	const float sin_dec = 0.39782f * sinf(alpha * l);
	const float cos_dec = cosf(asinf(sin_dec));

	//7a. ������������ ������� ������� ���� ������
	const float cos_h = (cosf(alpha * ZENITH) - (sin_dec * sinf(alpha * lat))) / (cos_dec * cosf(alpha * lat));
	if (cos_h > 1) return ERROR_VAL;
	if (cos_h < -1) return -ERROR_VAL;
	

	//7b. ��������� ������ H � ��������� � ����
	float h = is_sun_rise ? 360 - (180 / M_PI) * acosf(cos_h) : (180 / M_PI) * acosf(cos_h);
	h = h / 15;

	//8. ���������� ������� ������� ����� ������� / ������
	t = h + ra - (0.06571f * t) - 6.622f;

	//9. ��������������� ������� � UTC, ������������� �������� UT � ������� ������� ���� ������ / �������
	return (fmodf(t - lng_hour, DAY_HOURS) + time_offset + daylight_savings);
}


/**
 * \brief 
 * \param cur_time - ������� �����
 * \param lat - ������
 * \param lon - �������
 * \param time_offset - �������� ������� �� UTC
 * \param daylight_savings - �������� ������� ����\����
 * \return 
 */
bool SunRS_CalcValues(t_CTime cur_time, float lat, float lon, int time_offset, int daylight_savings)
{
	float sh;
	uint32_t total_night = 0;
	uint32_t total_day = 0;
	const int day_of_year = calcDayOfYear(cur_time.date, cur_time.month, cur_time.year);
	if ((day_of_year > 0) && (day_of_year < 366))
	{
		const float rt = calcSunRiseSet(CALC_SUN_RISE, day_of_year, lat, lon, time_offset, daylight_savings);
		sh = fmodf(DAY_HOURS + rt, DAY_HOURS);
		float sm = modf(fmodf(DAY_HOURS + rt, DAY_HOURS), &sh) * 60;

		sun_rise = cur_time;
		sun_rise.hours = roundf(sh);
		sun_rise.minutes = roundf(sm);
		sun_rise.seconds = 0;

		const float st = calcSunRiseSet(CALC_SUN_SET, day_of_year, lat, lon, time_offset, daylight_savings);
		sh = fmodf(DAY_HOURS + st, DAY_HOURS);
		sm = modf(fmodf(DAY_HOURS + st, DAY_HOURS), &sh) * 60;

		sun_set = cur_time;
		sun_set.hours = roundf(sh);
		sun_set.minutes = roundf(sm);
		sun_set.seconds = 0;


		if ((rt < ERROR_VAL && rt > -ERROR_VAL) && (st < ERROR_VAL && st > -ERROR_VAL)) {
			const uint32_t utr = rtcToEpoch(&sun_rise);
			const uint32_t uts = rtcToEpoch(&sun_set);

			if (uts >= utr) {
				total_day = (uts - utr);
				total_night = 86400 - (uts - utr);
			}
			else
			{
				total_night = (utr - uts);
				total_day = 86400 - (utr - uts);
			}

		}
		else if (rt == -ERROR_VAL && st == -ERROR_VAL)
		{
			total_day = 86400;
			total_night = 0;
		}
		else if (rt == ERROR_VAL && st == ERROR_VAL)
		{
			total_day = 0;
			total_night = 86400;
		}
		else
		{
			return false;
		}

		day_end = addSecToCTime(&sun_rise, total_day);
		night_end = addSecToCTime(&sun_set, total_night);
		return true;
	}

	return false;
}


t_CTime SunRS_GetDayStart(void)
{
	return sun_rise;
}

t_CTime SunRS_GetDayEnd(void)
{
	return day_end;
}


t_CTime SunRS_GetNightStart(void)
{
	return sun_set;
}

t_CTime SunRS_GetNightEnd(void)
{
	return night_end;
}