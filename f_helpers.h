//
// Created by Pavel on 03.06.2019.
//

#ifndef UNTITLED_F_HELPERS_H
#define UNTITLED_F_HELPERS_H

void rtcFromEpoch(uint32_t epoch, t_CTime *c_time);
uint32_t rtcToEpoch(t_CTime *c_time);
t_CTime addSecToCTime(t_CTime *c_time, const uint32_t sec);

#endif //UNTITLED_F_HELPERS_H
