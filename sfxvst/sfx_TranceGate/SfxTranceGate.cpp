
#include "SfxTranceGate.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <memory>

std::unique_ptr<SfxTranceGate> gFxUniquePtr;

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	gFxUniquePtr = std::unique_ptr<SfxTranceGate>(new SfxTranceGate(audioMaster));
	return gFxUniquePtr.get();
}


SfxTranceGate::SfxTranceGate(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
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
	mSpeed.setDisplayValue(4);
	mLength.set(0.5f);
	mGain.set(1.0f);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	suspend();
	resume();
}


SfxTranceGate::~SfxTranceGate()
{
	// nothing to do here
}

void SfxTranceGate::suspend()
{
	mGateCycle = 0;
	mGateDuty = 0;
	mGateCount = 0;

	for (int i = 0; i< GateWindowSize; i++)
	{
		mGateWindow[i] = (float)(GateWindowSize - i) / (float)GateWindowSize;
	}
}

void SfxTranceGate::resume()
{
	// @JP : 新しくエフェクトが起動された
	mGateCount = 0;
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
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
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
	float speed = (float)mSpeed.getDisplayValue();
	float length = mLength.get();
	float remain = 1.0f - mGain.get();
	float Fs = sampleRate;

	VstTimeInfo* timeInfo = getTimeInfo( kVstTempoValid );
	double BPM = timeInfo->tempo;
	double BPM_base = 4.0;

	mGateCycle = static_cast<unsigned int>(Fs*(((60.0 / BPM)*BPM_base) / speed));
	mGateDuty = static_cast<unsigned int>((double)mGateCycle * length);


	for (int i = 0; i< aSampleFrames; i++) {

		mGateCount++;
		if (mGateCount < 0) continue;
		if (mGateCount > mGateCycle) mGateCount = 0;
		
		float gain = 1.0f;
		if (mGateCount <= mGateDuty)
		{
			if (mGateDuty - mGateCount < GateWindowSize / 2) {
				gain = mGateWindow[GateWindowSize / 2 - (mGateDuty - mGateCount)];
			}
			else {
				gain = 1.0f;
			}
		}
		else
		{
			if (mGateCount - mGateDuty < GateWindowSize / 2) {
				gain = std::max(mGateWindow[GateWindowSize / 2 + (mGateCount - mGateDuty)], remain);
			}
			else {
				gain = remain;
			}
		}

		aOutputs[0][i] = aInputs[0][i] * gain;
		aOutputs[1][i] = aInputs[1][i] * gain;
	}
}
