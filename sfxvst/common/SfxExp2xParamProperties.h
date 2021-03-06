﻿#pragma once

#include <iomanip>
#include "SfxStepParamProperties.h"

template<int _Steps>
class SfxExp2xParamProperties : public SfxStepParamProperties<_Steps> {

public:

	SfxExp2xParamProperties()
		: SfxStepParamProperties("", "")
	{
	}
	SfxExp2xParamProperties(std::string aName, std::string aLabel)
		: SfxStepParamProperties(aName, aLabel)
	{
	}
	~SfxExp2xParamProperties() = default;

	virtual void display(char* aText) override {
		std::stringstream ss;
		ss << getDisplayValue();
		vst_strncpy(aText, ss.str().data(), kVstMaxParamStrLen);
	}

	virtual int getDisplayValue() {
		return 0x0001 << index();
	}

	virtual void setDisplayValue(int aVal) {
		setIndex((int)(log2(aVal) + 0.5));
	}
};