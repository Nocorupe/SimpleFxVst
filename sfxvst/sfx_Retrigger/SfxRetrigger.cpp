
#include "SfxRetrigger.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new SfxRetrigger(audioMaster);
}


SfxRetrigger::SfxRetrigger(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
	, mSpeed("Speed", "")
	, mLength("Length", "")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF03');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mSpeed;
	mPPPointers[1] = &mLength;

	// Program setup
	mSpeed.setDisplayValue(4);
	mLength.set(0.5f);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	suspend();
	resume();
}


SfxRetrigger::~SfxRetrigger()
{
	// nothing to do here
}

void SfxRetrigger::suspend()
{
	mCycle = 0;
	mDuty = 0;
	mCount = 0;
	mReSrc[0].fill(0.f);
	mReSrc[1].fill(0.f);
	mReSrcCount = 0;
}

void SfxRetrigger::resume()
{
	// @JP : 新しくエフェクトが起動された
	mReSrcCount = 0;
	mCount = 0;
}

void SfxRetrigger::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxRetrigger::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxRetrigger::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxRetrigger::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxRetrigger::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxRetrigger::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxRetrigger::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxRetrigger::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxRetrigger::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxRetrigger::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxRetrigger::getEffectName(char* name)
{
	vst_strncpy(name, "SfxRetrigger", kVstMaxEffectNameLen);
	return true;
}


bool SfxRetrigger::getProductString(char* text)
{
	vst_strncpy(text, "SfxRetrigger", kVstMaxProductStrLen);
	return true;
}


bool SfxRetrigger::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxRetrigger::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxRetrigger::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxRetrigger::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}


void SfxRetrigger::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{

	float speed = (float)mSpeed.getDisplayValue();
	float length = mLength.get();
	float Fs = sampleRate;

	VstTimeInfo* timeInfo = getTimeInfo(kVstTempoValid);
	double BPM = timeInfo->tempo;
	double BPM_base = 4.0;

	mCycle = static_cast<unsigned int>(Fs*(((60.0 / BPM)*BPM_base) / speed));
	mCycle = std::min<unsigned int>(mCycle, MaxCycleSize);
	mDuty = static_cast<int>((double)mCycle * length);

	for ( int i = 0; i< aSampleFrames; i++) {
		
		mCount++;
		if (mCount < 0) continue;
		if (mCount >(int)mCycle) mCount = 0;

		if (mReSrcCount < mCycle) {
			mReSrc[0][mReSrcCount] = aInputs[0][i];
			mReSrc[1][mReSrcCount] = aInputs[1][i];
			mReSrcCount++;
		}


		aOutputs[0][i] = mReSrc[0][mCount];
		aOutputs[1][i] = mReSrc[1][mCount];

		if (mCount > mDuty) {
			aOutputs[0][i] = aInputs[0][i] * 0.0125f;
			aOutputs[1][i] = aInputs[1][i] * 0.0125f;
		}
	}
}
