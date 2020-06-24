/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABInfo
*/

#ifndef DABINFO_H_
#define DABINFO_H_
#include <string>

namespace Hardware
{
struct DABInfo
{
    std::string StationName;
    std::string Info;
    int ReceiveLevel;
};
}
#endif /* !DABINFO_H_ */
