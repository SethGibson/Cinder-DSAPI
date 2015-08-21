#ifndef __CI_DSAPI__
#define __CI_DSAPI__
#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include <memory>
#include "DSAPI.h"
#include "DSAPIUtil.h"
#include "cinder/Channel.h"
#include "cinder/CinderGlm.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"

using namespace ci;
using namespace std;

namespace CinderDS
{
	enum FrameSize
	{
		DEPTHSD,	// 480x360
		DEPTHVGA,	// 640x480 (628x468)
		DEPTHQVGA, // 320x240
		RGBVGA,	// 640x480
		RGBHD	// 1920x1080
	};

	enum StereoCam
	{
		DS_LEFT = DSWhichImager::DS_LEFT_IMAGER,	// this may be useful later
		DS_RIGHT = DSWhichImager::DS_RIGHT_IMAGER,
		DS_BOTH = DSWhichImager::DS_BOTH_IMAGERS
	};

	//Index, Serial Number
	typedef pair<int, uint32_t> camera_type;
	vector<camera_type> GetCameraList();

	class CinderDSAPI;
	typedef std::shared_ptr<DSAPI> DSAPIRef;
	typedef std::shared_ptr<CinderDSAPI> CinderDSRef;

	class CinderDSAPI
	{
	protected:
		CinderDSAPI();
	public:
		static CinderDSRef create();
		~CinderDSAPI();

		bool init();
		bool init(uint32_t pSerialNo);

		bool initRgb(const FrameSize &pRes, const int &pFPS);
		bool initDepth(const FrameSize &pRes, const int &pFPS);
		bool initStereo(const FrameSize &pRes, const int &pFPS, const StereoCam &pWhich, const bool &pCrop );
		bool start();
		bool update();
		bool stop();

		const Surface8uRef getRgbFrame();
		const Channel8uRef getLeftFrame();
		const Channel8uRef getRightFrame();
		const Channel16uRef getDepthFrame();

		const vector<ivec2>& mapDepthToColorFrame();
		//const Surface32f& mapDepthToCameraTable();

		// get a 3d point from depth image coords (image x, image y, depth)
		const vec3 getDepthSpacePoint(float pX, float pY, float pZ);
		const vec3 getDepthSpacePoint(int pX, int pY, uint16_t pZ);
		const vec3 getDepthSpacePoint(vec3 pPoint);

		//get a Color object from depth image coords (image x, image y, depth)
		const Color getColorFromDepthImage(float pX, float pY, float pZ);		
		const Color getColorFromDepthImage(int pX, int pY, uint16_t pZ);
		const Color getColorFromDepthImage(vec3 pPoint);

		//get a Color object from camera space coords (camera x, camera y, camera z)
		const Color getColorFromDepthSpace(float pX, float pY, float pZ);		
		const Color getColorFromDepthSpace(vec3 pPoint);

		//get color space UVs from depth image coords
		const vec2 getColorCoordsFromDepthImage(float pX, float pY, float pZ);

		//get color space UVs from depth camera coords
		const vec2 getColorCoordsFromDepthSpace(vec3 pPoint);

		const int getDepthWidth(){ return mLRZWidth; }
		const int getDepthHeight(){ return mLRZHeight; }
		const ivec2 getDepthSize(){ return ivec2(mLRZWidth, mLRZHeight); }
		const vec2 getDepthFOVs();

		const int getRgbWidth(){ return mRgbWidth; }
		const int getRgbHeight(){ return mRgbHeight; }
		const ivec2 getRgbSize(){ return ivec2(mRgbWidth, mRgbHeight); }
		const vec2 getRgbFOVs();

		const DSAPIRef getDSAPI();
		DSThird* getDSThird();
		const DSCalibIntrinsicsRectified getZIntrinsics();
		const DSCalibIntrinsicsRectified getRgbIntrinsics();


		
	private:
		bool	open();
		bool	setupStream(const FrameSize &pRes, ivec2 &pOutSize);

		bool	mHasValidConfig,
				mHasValidCalib,
				mHasRgb,
				mHasDepth,
				mHasLeft,
				mHasRight,
				mIsInit,
				mUpdated;

		int32_t	mLRZWidth,
				mLRZHeight,
				mRgbWidth,
				mRgbHeight;

		DSAPIRef	mDSAPI;
		DSThird		*mDSRGB;
		DSCalibIntrinsicsRectified	mZIntrinsics;
		DSCalibIntrinsicsRectified	mRgbIntrinsics;
		double						mZToRgb[3];

		Surface8uRef		mRgbFrame;
		Channel8uRef		mLeftFrame;
		Channel8uRef		mRightFrame;
		Channel16uRef		mDepthFrame;
		Surface32fRef		mDepthToCameraTable;

		vector<ivec2> mDepthToColor;

	};
};
#endif