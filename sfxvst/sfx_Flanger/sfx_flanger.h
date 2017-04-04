
#ifndef __sfx_gain__
#define __sfx_gain__


#include "audioeffectx.h"

#include "RingBuffer.h"
#include "SfxParamProperties.h"
#include "SfxProgramManager.h"


class SfxFlanger : public AudioEffectX
{
public:
	SfxFlanger(audioMasterCallback audioMaster);
	~SfxFlanger();

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
	SfxParamProperties mRate;
	SfxParamProperties mDepth;
	SfxParamProperties mFeedback;
	SfxParamProperties mManual;


	// status
	VstInt32 mCount;
	float mPhaseOffset;

	RingBuffer<8192> mBuff[2];

	// programs
	SfxProgramManager<4> mProgramManager;

	// for Host
	std::array<SfxParamProperties*, 4> mPPPointers;
};

#endif
