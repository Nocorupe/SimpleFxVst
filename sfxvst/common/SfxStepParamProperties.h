
#pragma once

#include <iomanip>
#include "SfxParamProperties.h"

template<int _Steps>
class SfxStepParamProperties : public SfxParamProperties {
protected :
	void initialize() {
		flags |= kVstParameterUsesFloatStep;
		stepFloat = 1.0f / (float)_Steps;
		smallStepFloat = 0.f;
		largeStepFloat = 1.f;
	}
public :

	SfxStepParamProperties() 
		: SfxParamProperties("", "step") 
	{
		initialize();
	}
	SfxStepParamProperties(std::string aName, std::string aLabel)
		: SfxParamProperties(aName, aLabel)
	{
		initialize();
	}
	~SfxStepParamProperties() = default;

	inline int index() {
		return (int)((mVal * (float)_Steps) + 0.5f);
	}

	inline void setIndex(int aIndex) {
		set(stepFloat * (float)aIndex);
	}

	virtual void set(float aVal) override {
		mVal = stepFloat *(float)((int)((aVal * (float)_Steps) + 0.5f));
	}
	
	virtual void display(char* aText) override {
		std::stringstream ss;
		ss << index();
		vst_strncpy(aText, ss.str().data(), kVstMaxParamStrLen);
	}

};