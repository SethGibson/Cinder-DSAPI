#ifndef __DS4__
#define __DS4__

#include "DSAPI.h"
#include "DSAPIUtil.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"

using namespace ci;
namespace DS4
{
	//class DeviceOptions;
	//class Device;

	Channel8u									channel16To8(const ci::Channel16u& channel);

	size_t											getDeviceCount();
	std::map<size_t, std::string>					getDeviceMap();
	std::string										getErrorMessage();

	//ci::Vec2i										mapDepthCoordToColor(const ci::Vec2i& v, uint16_t depth, ICoordinateMapper* mapper);

};
#endif

/*
DSAPI
	virtual bool probeConfiguration() = 0;
	virtual DSPlatform getPlatform() = 0;
	virtual const char * getFirmwareVersionString() = 0;
	virtual const char * getSoftwareVersionString() = 0;
	virtual bool getCameraSerialNumber(uint32_t & number) = 0;
	virtual DSHardware * accessHardware() = 0;
	virtual DSThird * accessThird() = 0;
	virtual DSEmitter * accessEmitter() = 0;
	virtual DSFile * accessFile() = 0;
	virtual int getLRZNumberOfResolutionModes(bool rectified) = 0;
	virtual bool getLRZResolutionMode(bool rectified, int index, int & zWidth, int & zHeight, int & lrzFps, DSPixelFormat & lrPixelFormat) = 0;
	virtual bool setLRZResolutionMode(bool rectified, int zWidth, int zHeight, int lrzFps, DSPixelFormat lrPixelFormat) = 0;
	virtual int getLRZFramerate() = 0;
	virtual DSPixelFormat getLRPixelFormat() = 0;
	virtual bool isLRPixelFormatNative(DSPixelFormat pixelFormat) = 0;
	virtual bool isRectificationEnabled() = 0;
	virtual bool enableZ(bool state) = 0;
	virtual bool isZEnabled() = 0;
	virtual bool enableLeft(bool state) = 0;
	virtual bool isLeftEnabled() = 0;
	virtual bool enableRight(bool state) = 0;
	virtual bool isRightEnabled() = 0;
	virtual void enableLRCrop(bool state) = 0;
	virtual bool isLRCropEnabled() = 0;
	virtual bool startCapture() = 0;
	virtual bool stopCapture() = 0;
	virtual bool grab() = 0;
	virtual uint16_t * getZImage() = 0;
	virtual void * getLImage() = 0;
	virtual void * getRImage() = 0;
	virtual int zWidth() = 0;
	virtual int zHeight() = 0;
	virtual int lrWidth() = 0;
	virtual int lrHeight() = 0;
	virtual int maxLRBits() = 0;
	virtual double getFrameTime() = 0;
	virtual int getFrameNumber() = 0;
	virtual DSStatus getLastErrorStatus() = 0;
	virtual const char * getLastErrorDescription() = 0;
	virtual void setLoggingLevelAndFile(DSLoggingLevel level, const char * logFileName = nullptr) = 0;
	virtual void logCustomMessage(DSLoggingLevel level, const char * message) = 0;
	virtual bool setDisparityShift(uint32_t shift) = 0;
	virtual uint32_t getDisparityShift() = 0;
	virtual bool enableDisparityOutput(bool state) = 0;
	virtual bool isDisparityOutputEnabled() = 0;
	virtual bool setDisparityMultiplier(double multiplier) = 0;
	virtual double getDisparityMultiplier() = 0;
	virtual bool setMinMaxZ(uint16_t minZ, uint16_t maxZ) = 0;
	virtual void getMinMaxZ(uint16_t & minZ, uint16_t & maxZ) = 0;
	virtual uint32_t getZUnits() = 0;
	virtual bool setZUnits(uint32_t units) = 0;
	virtual bool isCalibrationValid() = 0;
	virtual bool getCalibIntrinsicsZ(DSCalibIntrinsicsRectified & intrinsics) = 0;
	virtual bool getCalibIntrinsicsRectLeftRight(DSCalibIntrinsicsRectified & intrinsics) = 0;
	virtual bool getCalibExtrinsicsRectLeftToRectRight(double & baseline) = 0;
	virtual bool getCalibIntrinsicsNonRectLeft(DSCalibIntrinsicsNonRectified & intrinsics) = 0;
	virtual bool getCalibIntrinsicsNonRectRight(DSCalibIntrinsicsNonRectified & intrinsics) = 0;
	virtual bool getCalibExtrinsicsNonRectLeftToNonRectRight(double rotation[9], double translation[3]) = 0;
	virtual double getZToDisparityConstant() = 0;
	virtual bool getCalibZToWorldTransform(double rotation[9], double translation[3]) = 0;
	virtual bool getCalibRectParameters(DSCalibRectParameters & params) = 0;
	virtual void setRecordingFilePath(const char * path) = 0;
	virtual void setImageFileFormat(DSImageFileFormat imageFileFormat) = 0;
	virtual void startRecordingToFile(bool state, int everyNthImage = 1) = 0;
	virtual void stopRecordingToFile() = 0;
	virtual bool isRecordingToFileStarted() = 0;
	virtual int getCapturingEveryNthImage() = 0;
	virtual void takeSnapshot() = 0;

Emitter
	virtual bool enableEmitter(bool enable) = 0;
	virtual bool isEmitterEnabled() = 0;

Hardware
	isHardwareActive() = 0;
	virtual DSChipType getChipVersion() = 0;
	virtual bool setAutoExposure(DSWhichImager whichImager, bool state) = 0;
	virtual bool getAutoExposure(DSWhichImager whichImager, bool & state) = 0;
	virtual bool setImagerGain(float gain, DSWhichImager whichImager) = 0;
	virtual bool getImagerGain(float & gain, DSWhichImager whichImager) = 0;
	virtual bool getImagerMinMaxGain(float & minGain, float & maxGain, DSWhichImager whichImager) = 0;
	virtual bool setImagerExposure(float exposure, DSWhichImager whichImager) = 0;
	virtual bool getImagerExposure(float & exposure, DSWhichImager whichImager) = 0;
	virtual bool getImagerMinMaxExposure(float & minExposure, float & maxExposure, DSWhichImager whichImager) = 0;

Third
	getThirdNumberOfResolutionModes(bool rectified) = 0;
	getThirdResolutionMode(bool rectified, int index, int & thirdWidth, int & thirdHeight, int & thirdFps, DSPixelFormat & thirdPixelFormat) = 0;
		setThirdResolutionMode(bool rectified, int thirdWidth, int thirdHeight, int thirdFps, DSPixelFormat thirdPixelFormat) = 0;
	getThirdFramerate() = 0;
	RGBFrame
		getThirdFrameTime() = 0;
		getThirdFrameNumber() = 0;
	getThirdPixelFormat() = 0;
	isThirdPixelFormatNative(DSPixelFormat pixelFormat) = 0;
	isThirdRectificationEnabled() = 0;
	enableThird(bool state) = 0;
	isThirdEnabled() = 0;
	getThirdImage() = 0;
	thirdWidth() = 0;
	thirdHeight() = 0;
	getCalibIntrinsicsRectThird(DSCalibIntrinsicsRectified & intrinsics) = 0;
	getCalibExtrinsicsZToRectThird(double translation[3]) = 0;
	getCalibIntrinsicsNonRectThird(DSCalibIntrinsicsNonRectified & intrinsics) = 0;
	getCalibExtrinsicsZToNonRectThird(double rotation[9], double translation[3]) = 0;
	getCalibExtrinsicsRectThirdToNonRectThird(double rotation[9]) = 0;
	*/