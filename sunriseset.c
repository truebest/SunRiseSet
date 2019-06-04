//
// Created by Pavel on 03.06.2019.
//

#include "stdbool.h"
#include "stdint.h"
#include <math.h>
#include "typedef.h"
#include "f_helpers.h"
#include "sunriseset.h"



#define ZENITH		(90.8f)
#define DAY_HOURS	 24.0f
#define ERROR_VAL	 100

#define CALC_SUN_RISE		true
#define CALC_SUN_SET		false


static t_CTime night_end, sun_rise, day_end, sun_set;

static const float gtr = M_PI / 180;   //Градусы в радианы: 10 градусов *П/180градусов
static const float rtg = 180 / M_PI;   //Радианы в градусы: радианы умножаешь на 180 градусов/П



/**
 * \brief Рассчитывает номер для в году по дате
 * \param day - день
 * \param month - месяц
 * \param year - год
 * \return - номер дня
 */
static uint16_t calcDayOfYear(uint8_t day, uint8_t month, uint16_t year)
{
    const uint16_t n1 = floorf(275 * month / 9);
    const uint16_t n2 = floorf((month + 9) / 12);
    const uint16_t n3 = (1 + floorf((year - 4 * floorf(year / 4) + 2) / 3));
    return (n1 - (n2 * n3) + day - 30);
}

/**
 * \brief Расчет значений восхода и заката солнца. Основан на алгоритме https://edwilliams.org/sunrise_sunset_algorithm.htm
 * \param is_sun_rise - восход или закат
 * \param day_num - номер дня
 * \param lat - широта
 * \param lon - долгота
 * \param time_offset - смещение времени
 * \param daylight_savings - лето\зима
 * \return результат - время восхода\заката
 */
static float calcSunRiseSet(bool is_sun_rise, int day_num, float lat, float lon, int time_offset, int daylight_savings)
{
    //1. Преобразуем долготу в часы из расчета что земля вращается со скоростью 15 градусов в час
    const float lng_hour = lon / 15;
    //2. В зависимости от восхода или заката, что требуется подсчитать
    float t = is_sun_rise ? day_num + ((6 - lng_hour) / DAY_HOURS) : day_num + ((18 - lng_hour) / DAY_HOURS);

    //3. Средняя аномалия солнца, для расчета удобно считать что солнце вращается вокруг земли (https://en.wikipedia.org/wiki/Position_of_the_Sun)
    const float m = (0.9856f * t) - 3.289f;

    //4. Рассчитываем истинную долготу Солнца
    const float l = fmodf(m + (1.916f * sinf(gtr * m)) + (0.020f * sinf(2 * gtr * m)) + 282.634, 360.0f);

    //5a. Рассчитываем прямое вхождение солнца (https://ru.wikipedia.org/wiki/Прямое_восхождение)
    float ra = fmodf(rtg * atanf(0.91764f * tanf(gtr * l)), 360.0f);

    //5b. Значение прямого восхождения должно быть в том же квадранте, что и L
    const float l_quadrant = floorf(l / 90) * 90;
    const float ra_quadrant = floorf(ra / 90) * 90;
    ra = ra + (l_quadrant - ra_quadrant);

    //5c. Правильное значение вхождения должно быть переведено в часы
    ra = ra / 15;

    //6. Рассчитываем склонение Солнца
    const float sin_dec = 0.39782f * sinf(gtr * l);
    const float cos_dec = cosf(asinf(sin_dec));

    //7a. Рассчитываем местный часовой угол Солнца
    const float cos_h = (cosf(gtr * ZENITH) - (sin_dec * sinf(gtr * lat))) / (cos_dec * cosf(gtr * lat));
    if (cos_h > 1) return ERROR_VAL;
    if (cos_h < -1) return -ERROR_VAL;


    //7b. Закончить расчет H и перевести в часы
    float h = is_sun_rise ? 360 - rtg * acosf(cos_h) : rtg * acosf(cos_h);
    h = h / 15;

    //8. Рассчитать местное среднее время восхода / заката
    t = h + ra - (0.06571f * t) - 6.622f;

    //9. Скорректировать обратно к UTC, преобразовать значение UT в местный часовой пояс широты / долготы
    return (fmodf(t - lng_hour, DAY_HOURS) + time_offset + daylight_savings);
}


/**
 * \brief
 * \param cur_time - текущее время
 * \param lat - широта
 * \param lon - долгота
 * \param time_offset - смещение времени по UTC
 * \param daylight_savings - смещение времени зима\лето
 * \return
 */
bool SunRS_CalcValues(t_CTime cur_time, float lat, float lon, int time_offset, int daylight_savings)
{
    float sh;
    uint32_t total_night;
    uint32_t total_day;
    int day_of_year = calcDayOfYear(cur_time.date, cur_time.month, cur_time.year);
    if ((day_of_year > 0) && (day_of_year < 366))
    {
        const float rt = calcSunRiseSet(CALC_SUN_RISE, day_of_year, lat, lon, time_offset, daylight_savings);
        sh = fmodf(DAY_HOURS + rt, DAY_HOURS);
        float sm = modff(fmodf(DAY_HOURS + rt, DAY_HOURS), &sh) * 60;

        sun_rise = cur_time;
        sun_rise.hours = roundf(sh);
        sun_rise.minutes = roundf(sm);
        sun_rise.seconds = 0;

        float st = calcSunRiseSet(CALC_SUN_SET, day_of_year, lat, lon, time_offset, daylight_savings);
        sh = fmodf(DAY_HOURS + st, DAY_HOURS);
        sm = modff(fmodf(DAY_HOURS + st, DAY_HOURS), &sh) * 60;

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

        day_of_year = calcDayOfYear(night_end.date, night_end.month, night_end.year);
        st = calcSunRiseSet(CALC_SUN_RISE, day_of_year, lat, lon, time_offset, daylight_savings);
        sh = fmodf(DAY_HOURS + st, DAY_HOURS);
        sm = modff(fmodf(DAY_HOURS + st, DAY_HOURS), &sh) * 60;

        night_end.hours = roundf(sh);
        night_end.minutes = roundf(sm);
        night_end.seconds = 0;

        return true;
    }

    return false;
}


/**
 * \brief Возвращает время и дату начала дня (восхода солнца)
 * \return структура времени
 */
t_CTime SunRS_GetDayStart(void)
{
    return sun_rise;
}

/**
 * \brief brief Возвращает время и дату конца дня(заката солнца)
 * \return структура времени
 */
t_CTime SunRS_GetDayEnd(void)
{
    return day_end;
}


/**
 * \brief brief Возвращает время и дату начала ночи(заката солнца)
 * \return структура времени
 */
t_CTime SunRS_GetNightStart(void)
{
    return sun_set;
}

/**
 * \brief brief Возвращает время и дату конца ночи(восхода солнца)
 * \return структура времени
 */
t_CTime SunRS_GetNightEnd(void)
{
    return night_end;
}
