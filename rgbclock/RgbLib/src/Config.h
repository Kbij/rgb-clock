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
#include <map>

namespace App {

struct UnitConfig
{
	UnitConfig() :
		mName(""),
		mLight(0),
		mKeyboard(0),
		mAmplifier(0),
		mLightSensor(0),
		mBackLight(0),
		mLCD(0){};
	std::string mName;
	uint8_t mLight;
	uint8_t mKeyboard;
	uint8_t mAmplifier;
	uint8_t mLightSensor;
	uint8_t mBackLight;
	uint8_t mLCD;
};

struct SystemConfig
{
	SystemConfig()
	{
		mHardwareRevision = 0;
		mRtc = 0;
		mRadio = 0;
		mCentralIO = 0;
		mFrequency = 0;
	}
	uint8_t mHardwareRevision;
	uint8_t mRtc;
	uint8_t mRadio;
	uint8_t mCentralIO;
	double mFrequency;
};


class Config {
public:
	Config(const std::string& settingsFile);
	virtual ~Config();

	bool errorFree();
	std::map<std::string, UnitConfig> configuredUnits() const;
	SystemConfig systemConfig() const;

private:
	std::string mSettingsFile;
	bool mErrorFree;
	std::map<std::string, UnitConfig> mConfiguredUnits;
	SystemConfig mSystemConfig;

	void loadXML();

};

} /* namespace App */
#endif /* CONFIG_H_ */
