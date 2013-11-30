/*
 * MainboardControl.h
 *
 *  Created on: Nov 30, 2013
 *      Author: koen
 */

#ifndef MAINBOARDCONTROL_H_
#define MAINBOARDCONTROL_H_
#include "I2C.h"
#include "IOExpander.h"
#include "KeyboardObserverIf.h"


namespace Hardware {
enum class InputSelection
{
	RadioIn,
	Auxin
};

class MainboardControl:  public Hardware::KeyboardObserverIf {
public:
	MainboardControl(I2C &i2c, uint8_t hwrevision, uint8_t address);
	virtual ~MainboardControl();

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state);

	void mute(bool mute);
	void resetTuner();
	void selectInput(InputSelection input);
	void signalWatchdog();

private:
	IOExpander mIO;
	const uint8_t mHwRevision;

};

} /* namespace Hardware */
#endif /* MAINBOARDCONTROL_H_ */
