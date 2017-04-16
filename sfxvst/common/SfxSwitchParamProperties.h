
#pragma once

#include "SfxParamProperties.h"

class SfxSwitchParamProperties : public SfxParamProperties {
protected :
	void initialize() {
		flags |= kVstParameterIsSwitch;
	}
public :
	SfxSwitchParamProperties() 
		: SfxParamProperties() 
	{
		initialize();
	}
	SfxSwitchParamProperties(std::string aName, std::string aLabel)
		: SfxParamProperties(aName,aLabel)
	{
		initialize();
	}

	~SfxSwitchParamProperties() = default;

	virtual void set(float aVal) override {
		mVal = (aVal < 0.5f) ? 0.0f : 1.0f;
	}

	virtual void display(char* aText) override {
		if (mVal < 0.5f) {
			vst_strncpy(aText, "off", kVstMaxParamStrLen);
		}
		else {
			vst_strncpy(aText, "on", kVstMaxParamStrLen);
		}
	}
};
