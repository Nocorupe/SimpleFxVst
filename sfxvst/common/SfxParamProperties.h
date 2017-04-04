
#pragma once

#include "audioeffectx.h"
#include <string>
#include <sstream>


class SfxParamProperties : public VstParameterProperties
{
public :
	char name[kVstMaxNameLen];

public :
	SfxParamProperties()
	{
		name[0] = '\0';
		label[0] = '\0';
	}
	SfxParamProperties(std::string aName, std::string aLabel) 
	{
		vst_strncpy(name, aName.data(), kVstMaxNameLen);
		vst_strncpy(label, aLabel.data(), kVstMaxLabelLen);
	}
	~SfxParamProperties() = default;
	SfxParamProperties(const SfxParamProperties& src) = default;

	virtual void set(float aVal) {
		mVal = aVal;
	}
	virtual float get() {
		return mVal;
	}

	virtual void display(char* aText) {
		std::stringstream ss;
		ss << mVal;
		vst_strncpy(aText, ss.str().data(), kVstMaxNameLen);
	}

protected :
	float mVal;

};



