
#include "SfxBitCrusher.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new SfxBitCrusher(audioMaster);
}


SfxBitCrusher::SfxBitCrusher(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, NumParams)
	, mAmount("Amount","")
	, mProgramManager(mPPPointers)
{
	setNumInputs(2);		// stereo in
	setNumOutputs(2);		// stereo out
	setUniqueID('FF06');	// identify
	canProcessReplacing();	// supports replacing output
							//	canDoubleReplacing ();	// supports double precision processing
	isSynth(false);

	mPPPointers[0] = &mAmount;

	// Program setup
	mAmount.setIndex(56);
	mProgramManager.addCurrentState("Default");

	mProgramManager.setProgramName("Default");
	resume();
}


SfxBitCrusher::~SfxBitCrusher()
{
	// nothing to do here
}

void SfxBitCrusher::suspend()
{
	// nothing to do
}

void SfxBitCrusher::resume()
{
	// @JP : 新しくエフェクトが起動された
}

void SfxBitCrusher::setProgramName(char* name)
{
	mProgramManager.setProgramName(name);
}

void SfxBitCrusher::setProgram(VstInt32 index)
{
	mProgramManager.setProgram(index);
}


void SfxBitCrusher::getProgramName(char* name)
{
	mProgramManager.getProgramName(name);
}

bool SfxBitCrusher::getProgramNameIndexed(VstInt32 category, VstInt32 index, char * text)
{
	return mProgramManager.getProgramNameIndexed(index, text);
}


void SfxBitCrusher::setParameter(VstInt32 index, float value)
{
	if (index >= numParams) return;
	mPPPointers[index]->set(value);
}


float SfxBitCrusher::getParameter(VstInt32 index)
{
	if (index >= numParams) return 0;
	return mPPPointers[index]->get();
}


void SfxBitCrusher::getParameterName(VstInt32 index, char* name)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(name, mPPPointers[index]->name, kVstMaxNameLen);
}

bool SfxBitCrusher::getParameterProperties(VstInt32 index, VstParameterProperties * properties)
{
	if (index < 0 || index >= numParams) return false;
	memcpy(properties, mPPPointers[index], sizeof(VstParameterProperties));
	return true;
}


void SfxBitCrusher::getParameterDisplay(VstInt32 index, char* text)
{
	if (index < 0 || index >= numParams) return;
	mPPPointers[index]->display(text);
}


void SfxBitCrusher::getParameterLabel(VstInt32 index, char* label)
{
	if (index < 0 || index >= numParams) return;
	vst_strncpy(label, mPPPointers[index]->label, kVstMaxNameLen);
}


bool SfxBitCrusher::getEffectName(char* name)
{
	vst_strncpy(name, "SfxBitCrusher", kVstMaxEffectNameLen);
	return true;
}


bool SfxBitCrusher::getProductString(char* text)
{
	vst_strncpy(text, "SfxBitCrusher", kVstMaxProductStrLen);
	return true;
}


bool SfxBitCrusher::getVendorString(char* text)
{
	vst_strncpy(text, "SimpleFxVst", kVstMaxVendorStrLen);
	return true;
}


VstInt32 SfxBitCrusher::getVendorVersion()
{
	return 1000;
}

VstPlugCategory SfxBitCrusher::getPlugCategory()
{
	return kPlugCategEffect;
}


VstInt32 SfxBitCrusher::canDo(char * text)
{
	if (strcmp(text, PlugCanDos::canDoReceiveVstTimeInfo) == 0) return 1;
	return -1;
}

void SfxBitCrusher::close()
{
	delete this;
}


void SfxBitCrusher::processReplacing(float** aInputs, float** aOutputs, VstInt32 aSampleFrames)
{
	int amount = mAmount.index();
	amount = std::max<int>(amount, 0);
	amount = std::min<int>(amount, 64);

	int fsCrush = (amount / 8) + 1;
	int bitCrush = (amount) % 8;

	mBuff[0].update(aInputs[0], aSampleFrames);
	mBuff[1].update(aInputs[1], aSampleFrames);

	float fsSrc0 = aInputs[0][0];
	float fsSrc1 = aInputs[1][0];

	for (int i = 0; i< aSampleFrames; i++) {

		short sh0 = 0;
		short sh1 = 0;

		// 元を切り替えて周波数クラッシュを実現
		if ((i % fsCrush) == 0) {

			sh0 = (short)(aInputs[0][i] * (float)SHRT_MAX);
			sh1 = (short)(aInputs[1][i] * (float)SHRT_MAX);
			fsSrc0 = aInputs[0][i];
			fsSrc1 = aInputs[1][i];
		}
		else {
			sh0 = (short)(fsSrc0 * (float)SHRT_MAX);
			sh1 = (short)(fsSrc1 * (float)SHRT_MAX);
		}

		sh0 = sh0 & (-1 << bitCrush);
		sh1 = sh1 & (-1 << bitCrush);
	
		aOutputs[0][i] = (float)((float)sh0 / (float)SHRT_MAX);
		aOutputs[1][i] = (float)((float)sh1 / (float)SHRT_MAX);
	}

}
