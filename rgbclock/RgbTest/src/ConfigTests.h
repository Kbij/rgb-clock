/*
 * ConfigTests.h
 *
 *  Created on: Aug 16, 2017
 *      Author: koen
 */

#ifndef SRC_CONFIGTESTS_H_
#define SRC_CONFIGTESTS_H_

#include <string>

std::string CONFIG = R"(
<?xml version="1.0" encoding="UTF-8" ?>
<settings>
        <hw_revision>2</hw_revision>
        <rtc_addr>104</rtc_addr>
        <radio_addr>99</radio_addr>
        <centralio_addr>32</centralio_addr>
	<frequency>92.1</frequency>
        <clockunit name="Unit1">
                <light_addr>65</light_addr>
                <backlight_addr>64</backlight_addr>
                <keyboard_addr>90</keyboard_addr>
                <amplifier_addr>108</amplifier_addr>
                <lcd_addr>33</lcd_addr>
                <lightsensor_addr>41</lightsensor_addr>
        </clockunit>
        <clockunit name="Unit2">
                <light_addr>67</light_addr>
                <backlight_addr>66</backlight_addr>
                <keyboard_addr>91</keyboard_addr>
                <amplifier_addr>109</amplifier_addr>
                <lcd_addr>34</lcd_addr>
                <lightsensor_addr>57</lightsensor_addr>
        </clockunit>
</settings>
)";

#endif /* SRC_CONFIGTESTS_H_ */
