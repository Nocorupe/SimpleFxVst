﻿
#include "SfxFlanger.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <memory>

std::unique_ptr<SfxFlanger> gFxUniquePtr;

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	gFxUniquePtr = std::unique_ptr<SfxFlanger>(new SfxFlanger(audioMaster));
	return gFxUniquePtr.get();
}


SfxFlanger::SfxFlanger(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
	, mRate("Rate", "Hz")
	, mDepthSpeed("DepthSpeed", "s/me")
	, mFeedback("Feedback", "")
	, mManual("Manual", "")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF01');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mRate;
	mPPPointers[1] = &mDepthSpeed;
	mPPPointers[2] = &mFeedback;
	mPPPointers[3] = &mManual;

	// Program setup
	mRate.set(0.25f);
	mDepthSpeed.setDisplayValue(1024); // almost 2ms
	mFeedback.set(1.0f);
	mManual.set(0.f);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	resume();
}


SfxFlanger::~SfxFlanger()
{
	// nothing to do here
}

void SfxFlanger::suspend()
{
	// nothing to do
}

void SfxFlanger::resume()
{
	// @JP : 新しくエフェクトが起動された
	mCount = 0;
	mPhaseOffset = 2.0f * (float)(M_PI)* mManual.get();
	mBuff[0].init();
	mBuff[1].init();
}

void SfxFlanger::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxFlanger::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxFlanger::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxFlanger::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxFlanger::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxFlanger::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxFlanger::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxFlanger::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxFlanger::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxFlanger::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxFlanger::getEffectName(char* name)
{
	vst_strncpy(name, "SfxFlanger", kVstMaxEffectNameLen);
	return true;
}


bool SfxFlanger::getProductString(char* text)
{
	vst_strncpy(text, "SfxFlanger", kVstMaxProductStrLen);
	return true;
}


bool SfxFlanger::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxFlanger::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxFlanger::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxFlanger::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}



void SfxFlanger::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	const float rate = mRate.get();
	const float feedback = mFeedback.get();
	const float manual = mManual.get();

	VstTimeInfo* timeInfo = getTimeInfo(kVstTempoValid);
	double BPM = (timeInfo->flags & kVstTempoValid) ? timeInfo->tempo : 120.0;
	double BPM_base = 4.0;
	const float depth = (float)((60.0 * BPM_base / BPM) / (double)mDepthSpeed.getDisplayValue());

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
