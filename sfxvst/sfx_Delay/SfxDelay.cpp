
#include "SfxDelay.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <memory>

std::unique_ptr<SfxDelay> gFxUniquePtr;

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	gFxUniquePtr = std::unique_ptr<SfxDelay>(new SfxDelay(audioMaster));
	return gFxUniquePtr.get();
}


SfxDelay::SfxDelay(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
	, mDepthSpeed("DepthSpeed", "s/me")
	, mDry("Dry", "")
	, mWet("Wet", "")
	, mFeedback("Feedback", "")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF04');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mDepthSpeed;
	mPPPointers[1] = &mDry;
	mPPPointers[2] = &mWet;
	mPPPointers[3] = &mFeedback;
	
	// Program setup
	mDepthSpeed.setDisplayValue(16);
	mDry.set(1.0);
	mWet.set(0.75f);
	mFeedback.set(0.50f);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	suspend();
	resume();
}


SfxDelay::~SfxDelay()
{
	// nothing to do here
}

void SfxDelay::suspend()
{
	// nothing to do
}

void SfxDelay::resume()
{
	// @JP : 新しくエフェクトが起動された
	mBuff[0].init();
	mBuff[1].init();
}

void SfxDelay::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxDelay::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxDelay::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxDelay::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxDelay::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxDelay::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxDelay::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxDelay::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxDelay::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxDelay::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxDelay::getEffectName(char* name)
{
	vst_strncpy(name, "SfxDelay", kVstMaxEffectNameLen);
	return true;
}


bool SfxDelay::getProductString(char* text)
{
	vst_strncpy(text, "SfxDelay", kVstMaxProductStrLen);
	return true;
}


bool SfxDelay::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxDelay::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxDelay::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxDelay::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}


void SfxDelay::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	if (aSampleFrames >= 8196) throw std::exception("SampleFrames > BufferSize");
	

	VstTimeInfo* timeInfo = getTimeInfo(kVstTempoValid);
	double BPM = (timeInfo->flags & kVstTempoValid) ? timeInfo->tempo : 120.0;
	double BPM_base = 4.0;
	const float delaySec = (float)((60.0 * BPM_base / BPM) / (double)mDepthSpeed.getDisplayValue());
	int delaySamples = (int)(sampleRate * delaySec);
	
	const float dry = mDry.get();
	const float wet = mWet.get();
	const float feedback = mFeedback.get();

	for (VstInt32 f = 0; f < aSampleFrames; f++) {
		if ((f - delaySamples) < 0) {
			mBlockBuff[0][f] = (aInputs[0][f]) + (mBuff[0][f - delaySamples] * feedback);
			mBlockBuff[1][f] = (aInputs[1][f]) + (mBuff[1][f - delaySamples] * feedback);
			
			aOutputs[0][f] = (aInputs[0][f] * dry) + (mBuff[0][f - delaySamples] * wet);
			aOutputs[1][f] = (aInputs[1][f] * dry) + (mBuff[1][f - delaySamples] * wet);
		}
		else {
			mBlockBuff[0][f] = (aInputs[0][f]) + (mBlockBuff[0][f - delaySamples] * feedback);
			mBlockBuff[1][f] = (aInputs[1][f]) + (mBlockBuff[1][f - delaySamples] * feedback);

			aOutputs[0][f] = (aInputs[0][f] * dry) + (mBlockBuff[0][f - delaySamples] * wet);
			aOutputs[1][f] = (aInputs[1][f] * dry) + (mBlockBuff[1][f - delaySamples] * wet);
		}

	}

	mBuff[0].update(mBlockBuff[0].data(), aSampleFrames);
	mBuff[1].update(mBlockBuff[1].data(), aSampleFrames);

}
