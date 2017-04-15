
#pragma once

#include "audioeffectx.h"
#include <string>
#include <sstream>


class SfxParamProperties : public VstParameterProperties
{
public :
	// @JP: 
	// Vst2.4 document では kVstParamStrLen が used for effGetParamName　
	// だが 多くのVSTプラグイン同様、より多くの文字数で扱う
	char name[kVstMaxNameLen];

public :
	SfxParamProperties()
	{
		name[0] = '\0';
		label[0] = '\0';
	}
	SfxParamProperties(std::string aName, std::string aLabel) 
	{
		vst_strncpy(name, aName.data(), kVstMaxParamStrLen);
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
		vst_strncpy(aText, ss.str().data(), kVstMaxParamStrLen);
	}

protected :
	float mVal;

};



