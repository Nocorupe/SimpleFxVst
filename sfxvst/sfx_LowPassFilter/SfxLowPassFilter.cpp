
#include "SfxLowPassFilter.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new SfxLowPassFilter(audioMaster);
}


SfxLowPassFilter::SfxLowPassFilter(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
	, mFc("Fc", "Hz")
	, mQ("Q","")
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
	
	// Program setup
	mFc.setDisplayValue(1000.0f);
	mQ.setDisplayValue(0.7071f); // almost 2ms
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	resume();
}


SfxLowPassFilter::~SfxLowPassFilter()
{
	// nothing to do here
}

void SfxLowPassFilter::suspend()
{
	// nothing to do
}

void SfxLowPassFilter::resume()
{
	// @JP : 新しくエフェクトが起動された
	mSrcBuff[0].init();
	mSrcBuff[1].init();
	mDstBuff[0].init();
	mDstBuff[1].init();
}

void SfxLowPassFilter::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxLowPassFilter::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxLowPassFilter::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxLowPassFilter::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxLowPassFilter::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxLowPassFilter::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxLowPassFilter::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxLowPassFilter::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxLowPassFilter::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxLowPassFilter::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxLowPassFilter::getEffectName(char* name)
{
	vst_strncpy(name, "SfxLowPassFilter", kVstMaxEffectNameLen);
	return true;
}


bool SfxLowPassFilter::getProductString(char* text)
{
	vst_strncpy(text, "SfxLowPassFilter", kVstMaxProductStrLen);
	return true;
}


bool SfxLowPassFilter::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxLowPassFilter::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxLowPassFilter::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxLowPassFilter::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}


void SfxLowPassFilter::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	double fc = mFc.getDisplayValue();
	double Q = mQ.getDisplayValue();

	double omega = 2.0 * M_PI * fc / (sampleRate);
	double sin_omega = std::sin(omega);
	double cos_omega = std::cos(omega);
	double alpha = sin_omega / (2.0 * Q);

	mB[0] = (1.0 - cos_omega) / 2.0;
	mB[1] = 1.0 - cos_omega;
	mB[2] = (1.0 - cos_omega) / 2.0;
	mA[0] = 1.0 + alpha;
	mA[1] = -2.0*cos_omega;
	mA[2] = 1.0 - alpha;


	mSrcBuff[0].update(aInputs[0], aSampleFrames);
	mSrcBuff[1].update(aInputs[1], aSampleFrames);
	mDstBuff[0].update(aInputs[0], aSampleFrames);
	mDstBuff[1].update(aInputs[1], aSampleFrames);
	
	for (int i = 0; i< aSampleFrames; i++) {
		
		mDstBuff[0][i] = (float)(
			  (mB[0] / mA[0])*(double)(mSrcBuff[0][i])
			+ (mB[1] / mA[0])*(double)(mSrcBuff[0][i-1])
			+ (mB[2] / mA[0])*(double)(mSrcBuff[0][i-2])
			- (mA[1] / mA[0])*(double)(mDstBuff[0][i-1])
			- (mA[2] / mA[0])*(double)(mDstBuff[0][i-2]));

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
