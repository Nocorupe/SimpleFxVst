﻿
#include "SfxPeakingEqualizer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <memory>

std::unique_ptr<SfxPeakingEqualizer> gFxUniquePtr;

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	gFxUniquePtr = std::unique_ptr<SfxPeakingEqualizer>(new SfxPeakingEqualizer(audioMaster));
	return gFxUniquePtr.get();
}


SfxPeakingEqualizer::SfxPeakingEqualizer(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 3, NumParams)
	, mFc("Fc", "Hz")
	, mQ("Q", "")
	, mGain("Gain", "")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF06');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mFc;
	mPPPointers[1] = &mQ;
	mPPPointers[2] = &mGain;

	// Program setup
	mFc.setDisplayValue(1000.0f);
	mQ.setDisplayValue(0.7071f);
	mGain.setDisplayValue(12.5f);
	mProgramManager.addCurrentState("Default");

	mFc.setDisplayValue(200.0f);
	mQ.setDisplayValue(0.7071f);
	mGain.setDisplayValue(12.5f);
	mProgramManager.addCurrentState("Fc Low");

	mFc.setDisplayValue(4000.0f);
	mQ.setDisplayValue(0.7071f);
	mGain.setDisplayValue(12.5f);
	mProgramManager.addCurrentState("Fc High");



	mProgramManager.setProgramName("Default");
	resume();
}


SfxPeakingEqualizer::~SfxPeakingEqualizer()
{
	// nothing to do here
}

void SfxPeakingEqualizer::suspend()
{
	// nothing to do
}

void SfxPeakingEqualizer::resume()
{
	// @JP : 新しくエフェクトが起動された
	mSrcBuff[0].init();
	mSrcBuff[1].init();
	mDstBuff[0].init();
	mDstBuff[1].init();

	mIsWarmUp = true;
	mWarmUpCount = 0;
}

void SfxPeakingEqualizer::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxPeakingEqualizer::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxPeakingEqualizer::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxPeakingEqualizer::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxPeakingEqualizer::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxPeakingEqualizer::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxPeakingEqualizer::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxPeakingEqualizer::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxPeakingEqualizer::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxPeakingEqualizer::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxPeakingEqualizer::getEffectName(char* name)
{
	vst_strncpy(name, "SfxPeakingEqualizer", kVstMaxEffectNameLen);
	return true;
}


bool SfxPeakingEqualizer::getProductString(char* text)
{
	vst_strncpy(text, "SfxPeakingEqualizer", kVstMaxProductStrLen);
	return true;
}


bool SfxPeakingEqualizer::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxPeakingEqualizer::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxPeakingEqualizer::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxPeakingEqualizer::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}



void SfxPeakingEqualizer::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	double fc = mFc.getDisplayValue();
	double Q = mQ.getDisplayValue();
	double dBgain = mGain.getDisplayValue();

	double A = std::pow(10.0, (dBgain / 40.0));
	double omega = 2.0 * M_PI * fc / (sampleRate);
	double cos_omega = std::cos(omega);
	double sin_omega = std::sin(omega);
	double alpha = sin_omega / (2.0 * Q);


	mB[0] = 1.0 + alpha*A;
	mB[1] = -2.0 * cos_omega;
	mB[2] = 1.0 - alpha*A;
	mA[0] = 1.0 + alpha / A;
	mA[1] = -2.0 * cos_omega;
	mA[2] = 1.0 - alpha / A;


	mSrcBuff[0].update(aInputs[0], aSampleFrames);
	mSrcBuff[1].update(aInputs[1], aSampleFrames);
	mDstBuff[0].update(aInputs[0], aSampleFrames);
	mDstBuff[1].update(aInputs[1], aSampleFrames);

	if (mIsWarmUp) {

		for (int i = 0; i< aSampleFrames; i++) {
			int count = std::min(mWarmUpCount, WarmUpFrames);
			float w = (float)count / (float)WarmUpFrames;

			mDstBuff[0][i] = (float)(
				(mB[0] / mA[0])*(double)(mSrcBuff[0][i])
				+ (mB[1] / mA[0])*(double)(mSrcBuff[0][i - 1])
				+ (mB[2] / mA[0])*(double)(mSrcBuff[0][i - 2])
				- (mA[1] / mA[0])*(double)(mDstBuff[0][i - 1])
				- (mA[2] / mA[0])*(double)(mDstBuff[0][i - 2]));

			aOutputs[0][i] = (mDstBuff[0][i] * w) + (aInputs[0][i] * (1.0f - w));

			mDstBuff[1][i] = (float)(
				(mB[0] / mA[0])*(double)(mSrcBuff[1][i])
				+ (mB[1] / mA[0])*(double)(mSrcBuff[1][i - 1])
				+ (mB[2] / mA[0])*(double)(mSrcBuff[1][i - 2])
				- (mA[1] / mA[0])*(double)(mDstBuff[1][i - 1])
				- (mA[2] / mA[0])*(double)(mDstBuff[1][i - 2]));

			aOutputs[1][i] = (mDstBuff[1][i] * w) + (aInputs[1][i] * (1.0f - w));

			mWarmUpCount++;
		}

		if (mWarmUpCount >= WarmUpFrames) {
			mIsWarmUp = false;
		}

		return;
	}


	for (int i = 0; i< aSampleFrames; i++) {

		mDstBuff[0][i] = (float)(
			(mB[0] / mA[0])*(double)(mSrcBuff[0][i])
			+ (mB[1] / mA[0])*(double)(mSrcBuff[0][i - 1])
			+ (mB[2] / mA[0])*(double)(mSrcBuff[0][i - 2])
			- (mA[1] / mA[0])*(double)(mDstBuff[0][i - 1])
			- (mA[2] / mA[0])*(double)(mDstBuff[0][i - 2]));

		aOutputs[0][i] = mDstBuff[0][i];

		mDstBuff[1][i] = (float)(
			(mB[0] / mA[0])*(double)(mSrcBuff[1][i])
			+ (mB[1] / mA[0])*(double)(mSrcBuff[1][i - 1])
			+ (mB[2] / mA[0])*(double)(mSrcBuff[1][i - 2])
			- (mA[1] / mA[0])*(double)(mDstBuff[1][i - 1])
			- (mA[2] / mA[0])*(double)(mDstBuff[1][i - 2]));

		aOutputs[1][i] = mDstBuff[1][i];
	}
}
