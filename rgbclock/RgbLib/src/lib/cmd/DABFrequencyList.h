/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** µ
*/

#ifndef DABFREQUENCYLIST_H_
#define DABFREQUENCYLIST_H_
#include <vector>
#include <string>
#include <sstream>

namespace Hardware
{
class FrequencyList
{
public:
    FrequencyList(const std::vector<uint8_t>& data): NUM_FREQS(), mFrequencies()
    {
        parse(data);
    }
    ~FrequencyList() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Number of Frequencies: " << (int) NUM_FREQS << ", List: ";
        int index = 0;
        for(auto freq: mFrequencies)
        {
            ss << index << ":" << (double) freq/1000 << "MHz ";
            ++index;
        }
        return ss.str();
    }

    uint8_t NUM_FREQS;
    std::vector<int> mFrequencies;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() >= 5)
        {
            NUM_FREQS = data[4];
        }
        int index = 8;
        while ((index + 3) <= data.size())
        {
            int frequency = (data[index + 3] << 24) + (data[index + 2] << 16) + (data[index + 1] << 8) + data[index];
            index += 4;
            mFrequencies.push_back(frequency);
        }
    } 
};
}
#endif /* !DABFREQUENCYLIST_H_ */
