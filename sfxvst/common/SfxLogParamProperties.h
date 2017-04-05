
#pragma once

#include <iomanip>
#include "SfxParamProperties.h"

template<int MinValue,int MaxValue>
class SfxLogParamProperties : public SfxParamProperties {

public:

	SfxLogParamProperties()
		: SfxParamProperties("", "")
	{
	}
	SfxLogParamProperties(std::string aName, std::string aLabel)
		: SfxParamProperties(aName, aLabel)
	{
	}
	~SfxLogParamProperties() = default;

	virtual void display(char* aText) override {
		std::stringstream ss;
		ss << getDisplayValue();
		vst_strncpy(aText, ss.str().data(), kVstMaxNameLen);
	}

	virtual int getDisplayValue() {
		return (float)MinValue + ((std::powf(10.0f, mVal) - 1.0f) / 9.0f) * (float)(MaxValue-MinValue);
	}

	virtual void setDisplayValue(float aVal) {
		mVal = log10f(( ((aVal-(float)MinValue) / (float)(MaxValue-MinValue)) * 9.0f) + 1.0f);
	}
};