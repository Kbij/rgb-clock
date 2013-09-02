/*
 * Config.h
 *
 *  Created on: Sep 1, 2013
 *      Author: koen
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include <string>
#include <stdint.h>
#include <vector>

namespace App {

struct UnitConfig
{
	std::string mName;
	uint8_t mLight;
	uint8_t mKeyboard;
	uint8_t mAmplifier;
	uint8_t mLightSensor;
	uint8_t mLCD;
};

struct SystemConfig
{
	uint8_t mRtc;
	uint8_t mRadio;
};


class Config {
public:
	Config();
	virtual ~Config();

	bool errorFree();
	const std::vector<UnitConfig>& configuredUnits();
	const SystemConfig& systemConfig();

private:
	bool mErrorFree;
	std::vector<UnitConfig> mConfiguredUnits;
	SystemConfig mSystemConfig;

	void loadXML();

};

} /* namespace App */
#endif /* CONFIG_H_ */
