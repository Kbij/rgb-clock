/*
 * RDSInfo.h
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#ifndef RDSINFO_H_
#define RDSINFO_H_
#include <string>
#include <stdint.h>

enum class TextType
{
	Unknown,
	TypeA,
	TypeB
};
const char EMPTY_CHAR = '~';

struct RDSInfo {
	uint16_t mProgramId;
	bool mValidRds;
	std::string mStationName;
	std::string mText;
	TextType mTextType;
	uint8_t mReceiveLevel;
	RDSInfo()
	{
		mValidRds = false;
		mProgramId = 0;
		mReceiveLevel = 0;
		clearAll();
		mTextType = TextType::Unknown;
	}
	void clearAll()
	{
		clearStationName();
		clearText();
	}
	void clearStationName()
	{
		mStationName = "";
		mStationName.resize(9,' ');
	}

	void clearText()
	{
		mText = "";
		mText.resize(65,EMPTY_CHAR);
	}

	bool textComplete()
	{
		return mText.find(EMPTY_CHAR) == std::string::npos;
	}
};

#endif /* RDSINFO_H_ */
