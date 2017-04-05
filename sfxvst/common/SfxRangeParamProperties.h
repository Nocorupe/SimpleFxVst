
#pragma once

#include <iomanip>
#include "SfxParamProperties.h"

template<int MinValue, int MaxValue>
class SfxRangeParamProperties : public SfxParamProperties {

public:

	SfxRangeParamProperties()
		: SfxParamProperties("", "")
	{
	}
	SfxRangeParamProperties(std::string aName, std::string aLabel)
		: SfxParamProperties(aName, aLabel)
	{
	}
	~SfxRangeParamProperties() = default;

	virtual void display(char* aText) override {
		std::stringstream ss;
		ss << getDisplayValue();
		vst_strncpy(aText, ss.str().data(), kVstMaxNameLen);
	}

	virtual int getDisplayValue() {
		return (float)MinValue + mVal * (float)(MaxValue - MinValue);
	}

	virtual void setDisplayValue(float aVal) {
		mVal = (aVal - (float)MinValue) / (float)(MaxValue - MinValue);
	}
};