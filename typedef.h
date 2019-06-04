//
// Created by Pavel on 03.06.2019.
//

#ifndef UNTITLED_TYPEDEF_H
#define UNTITLED_TYPEDEF_H

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

#endif //UNTITLED_TYPEDEF_H
