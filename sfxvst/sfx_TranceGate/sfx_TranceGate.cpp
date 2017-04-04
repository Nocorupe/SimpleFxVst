
#include "sfx_TranceGate.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new SfxTranceGate(audioMaster);
}


SfxTranceGate::SfxTranceGate(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, 3)
	, mSpeed("Speed", "")
	, mLength("Length", "")
	, mGain("Gain", "")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF02');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mSpeed;
	mPPPointers[1] = &mLength;
	mPPPointers[2] = &mGain;

	// Program setup
	mSpeed.set(0.f); // 4
	mLength.set(0.5f);
	mGain.set(1.0f);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	resume();
}


SfxTranceGate::~SfxTranceGate()
{
	// nothing to do here
}

void SfxTranceGate::suspend()
{
	// nothing to do
}

void SfxTranceGate::resume()
{
	// @JP : 新しくエフェクトが起動された
	mCount = 0;
	//mPhaseOffset = 2.0f * (float)(M_PI)* mManual.get();
	mBuff[0].init();
	mBuff[1].init();
}

void SfxTranceGate::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxTranceGate::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxTranceGate::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxTranceGate::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxTranceGate::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxTranceGate::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxTranceGate::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxTranceGate::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxTranceGate::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxTranceGate::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxTranceGate::getEffectName(char* name)
{
	vst_strncpy(name, "SfxTranceGate", kVstMaxEffectNameLen);
	return true;
}


bool SfxTranceGate::getProductString(char* text)
{
	vst_strncpy(text, "SfxTranceGate", kVstMaxProductStrLen);
	return true;
}


bool SfxTranceGate::getVendorString(char* text)
{
	vst_strncpy(text, "Test", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxTranceGate::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxTranceGate::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxTranceGate::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}


void SfxTranceGate::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	const float rate = mRate.get();
	const float depth = mDepth.get();
	const float feedback = mFeedback.get();
	const float manual = mManual.get();

	mBuff[0].update(aInputs[0], aSampleFrames);
	mBuff[1].update(aInputs[1], aSampleFrames);

	for (VstInt32 f = 0; f< aSampleFrames; f++) {
		aOutputs[0][f] = mBuff[0][f];
		aOutputs[1][f] = mBuff[1][f];

		float sin_phase = std::sin((2.0f * (float)(M_PI)* rate * ((float)(mCount) / sampleRate)) + mPhaseOffset);
		float tau = (depth * sampleRate) + ((depth*sampleRate) * sin_phase);
		// accuracy problem
		float t = (float)f - tau;
		int m = static_cast<int>(t);

		if (m >= 0) {
			float delta = t - (float)m;

			aOutputs[0][f] += ((delta * mBuff[0][m + 1]) + (1.0f - delta) * mBuff[0][m]) * feedback;
			aOutputs[1][f] += ((delta * mBuff[1][m + 1]) + (1.0f - delta) * mBuff[1][m]) * feedback;
		}
		else { // = if ( m < 0 ) {
			float delta = (float)m - t;

			aOutputs[0][f] += ((delta * mBuff[0][m - 1]) + (1.0f - delta) * mBuff[0][m]) * feedback;
			aOutputs[1][f] += ((delta * mBuff[1][m - 1]) + (1.0f - delta) * mBuff[1][m]) * feedback;
		}

		mCount++;
	}
}
