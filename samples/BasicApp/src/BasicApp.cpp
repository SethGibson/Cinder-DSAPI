#ifdef _DEBUG
#pragma comment(lib, "DSAPI32.dbg.lib")
#else
#pragma comment(lib, "DSAPI32.lib")
#endif

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class BasicApp : public App
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	void exit();	// So we can shutdown the DS

private:
	CinderDSRef mCinderDS;
	gl::TextureRef mTexRgb;
	gl::TextureRef mTexDepth;

	ivec2 mDepthDims, mRgbDims;

};

void BasicApp::setup()
{
	getWindow()->setSize(1280, 480);
	
	mCinderDS = CinderDSAPI::create();
	
	/* setup our DSAPI instance	
		CinderDSAPI::init() MUST always be called first, this handles low level initialization required
		for any DSAPI access.

		CinderDSAPI::init*() take a FrameSize enum and a float frame rate.  The following are currently implemented:
			initDepth();
			initRgb();
			initStereo(); <- note that initStereo() also takes a StereoCam enum.
		FrameSize options:
			DEPTHQVGA, DEPTHSD, DEPTHVGA, RGBVGA, RGBHD
		StereoCam options:
			DS_LEFT, DS_RIGHT, DS_BOTH
	*/
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);

	// store our frame dimensions
	mRgbDims = mCinderDS->getRgbSize();
	mDepthDims = mCinderDS->getDepthSize();

	//setup initial texture refs
	mTexRgb = gl::Texture::create(mRgbDims.x, mRgbDims.y);
	mTexDepth = gl::Texture::create(mDepthDims.x, mDepthDims.y);

	// setup our shutdown event
	getSignalCleanup().connect(std::bind(&BasicApp::exit, this));

	// Start streaming
	mCinderDS->start();
}

void BasicApp::mouseDown( MouseEvent event )
{
}

void BasicApp::update()
{
	mCinderDS->update();

	// now we just update the texture with the stored Surface8u
	mTexRgb->update(mCinderDS->getRgbFrame());

	// depth is stored as a channel16u so we can just walk that with Channel::Iter
	Channel16u cDepth = mCinderDS->getDepthFrame();
	
	Surface8u cSurfDepth(mDepthDims.x, mDepthDims.y, SurfaceChannelOrder::RGB);
	Surface8u::Iter cIter = cSurfDepth.getIter();

	while (cIter.line())
	{
		while (cIter.pixel())
		{
			// set the pixel to black
			cIter.r() = 0;
			cIter.g() = 0;
			cIter.b() = 0;

			// let's check depth and use it if it's between 100mm and 1 m (1000 mm)
			float cValue = (float)*cDepth.getData(cIter.x(), cIter.y());
			if (cValue > 100 && cValue < 1000)
			{
				int cColor = (int)lmap<float>(cValue, 100, 1000, 255, 0);
				cIter.r() = cColor;
				cIter.g() = cColor;
			}
		}
	}

	// update our depth texture the same as the rgb texture
	mTexDepth->update(cSurfDepth);
}

void BasicApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::draw(mTexDepth, Rectf({ vec2(0), vec2(mRgbDims.x, mRgbDims.y) }));
	gl::draw(mTexRgb, vec2(mRgbDims.x, 0));
}

void BasicApp::exit()
{
	mCinderDS->stop();
}

CINDER_APP( BasicApp, RendererGl )
