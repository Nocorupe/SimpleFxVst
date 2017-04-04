#pragma once

#include <tuple>
#include <vector>
#include "SfxParamProperties.h"

template<int NumParam>
class SfxProgramManager {
private :
	std::array<SfxParamProperties*, NumParam>& mRefPPPointers;
	std::vector<std::pair<std::string, std::array<float, NumParam>>> mPrograms;
	int mCurrentProgramIndex;
public :
	SfxProgramManager(std::array<SfxParamProperties*, NumParam>& aRefPPPointers)
		: mRefPPPointers(aRefPPPointers)
		, mCurrentProgramIndex(-1)
	{}
	~SfxProgramManager() = default;

	void addCurrentState(std::string aProgramName) {
		std::array<float, NumParam> params;
		for (int i = 0; i < NumParam; i++) {
			params[i] = mRefPPPointers[i]->get();
		}
		mPrograms.push_back(std::make_pair(aProgramName, params));
	}

	void setProgram(int aIndex) {
		if (aIndex < 0 || aIndex >= (int)mPrograms.size()) return;
		for (int i = 0; i < NumParam; i++) {
			mRefPPPointers[i]->set( std::get<1>(mPrograms[aIndex])[i] );
		}
		mCurrentProgramIndex = aIndex;
	}

	void setProgramName(char* aName) {
		std::string name(aName);
		for (std::size_t i = 0; i < mPrograms.size(); i++) {
			if (name == std::get<0>(mPrograms[i])) {
				setProgram((int)i);
				return;
			}
		}
	}

	void getProgramName(char* aDst) {
		if (mCurrentProgramIndex < 0) return;
		vst_strncpy(aDst, std::get<0>(mPrograms[mCurrentProgramIndex]).data(), kVstMaxProgNameLen);
	}

	bool getProgramNameIndexed(VstInt32 aIndex, char* aDst) {
		if (aIndex < 0 || aIndex >= (int)mPrograms.size()) return false;
		vst_strncpy(aDst, std::get<0>(mPrograms[aIndex]).data(), kVstMaxProgNameLen);
		return true;
	}
};
