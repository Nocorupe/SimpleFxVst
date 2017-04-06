
#ifndef __sfx_gain__
#define __sfx_gain__


#include "audioeffectx.h"

#include "RingBuffer.h"
#include "SfxLogParamProperties.h"
#include "SfxRangeParamProperties.h"
#include "SfxProgramManager.h"


class SfxPeakingEqualizer : public AudioEffectX
{
public:
	SfxPeakingEqualizer(audioMasterCallback audioMaster);
	~SfxPeakingEqualizer();

	// Processing
	virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) override;

	// OnOff
	virtual void suspend() override;
	virtual void resume() override;

	// Program
	virtual void setProgramName(char* name) override;
	virtual void setProgram(VstInt32 index) override;
	virtual void getProgramName(char* name) override; // get current program name
	virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text) override;

	// Parameters
	virtual void setParameter(VstInt32 index, float value) override;
	virtual float getParameter(VstInt32 index) override;
	virtual void getParameterLabel(VstInt32 index, char* label) override;
	virtual void getParameterDisplay(VstInt32 index, char* text) override;
	virtual void getParameterName(VstInt32 index, char* text) override;

	virtual bool getParameterProperties(VstInt32 index, VstParameterProperties* properties) override;

	virtual bool getEffectName(char* name) override;
	virtual bool getVendorString(char* text) override;
	virtual bool getProductString(char* text) override;
	virtual VstInt32 getVendorVersion() override;
	virtual VstPlugCategory getPlugCategory() override;

	// cando
	virtual VstInt32 canDo(char* text) override;


protected:
	// parameters
	static constexpr int NumParams = 3;
	SfxLogParamProperties<10, 22050> mFc;
	SfxRangeParamProperties<0, 4> mQ;
	SfxRangeParamProperties<0, 20> mGain;


	// status
	std::array<double, 3> mA;
	std::array<double, 3> mB;

	bool mIsWarmUp; // @JP : suspend時にtrue 原音とクロスフェードさせるウォームアップをするか
	static constexpr int WarmUpFrames = 2048;
	int mWarmUpCount;


	RingBuffer<8192> mSrcBuff[2];
	RingBuffer<8192> mDstBuff[2];

	// programs
	SfxProgramManager<NumParams> mProgramManager;

	// for Host
	std::array<SfxParamProperties*, NumParams> mPPPointers;
};

#endif
