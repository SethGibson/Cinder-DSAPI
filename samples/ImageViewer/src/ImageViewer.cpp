#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class ImageViewer : public App
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	CinderDSRef mCinderDS;
	gl::Texture2dRef mTexRgb;
	gl::Texture2dRef mTexDepth;

	ivec2 mDepthDims, mRgbDims;

};

void ImageViewer::setup()
{
	getWindow()->setSize(1280, 480);
	
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);

	mRgbDims = mCinderDS->getRgbSize();
	mDepthDims = mCinderDS->getDepthSize();

	mTexRgb = gl::Texture::create(mRgbDims.x, mRgbDims.y);
	mTexDepth = gl::Texture::create(mDepthDims.x, mDepthDims.y);

	mCinderDS->start();
}

void ImageViewer::mouseDown( MouseEvent event )
{
}

void ImageViewer::update()
{
	mCinderDS->update();
	mTexRgb->update(*mCinderDS->getRgbFrame());
	auto depthChannel = mCinderDS->getDepthFrame();
	auto depthSurface = Surface8u::create(mDepthDims.x, mDepthDims.y, SurfaceChannelOrder::RGB);
	auto iter = depthSurface->getIter();

	while (iter.line())
	{
		while (iter.pixel())
		{
			iter.r() = 0;
			iter.g() = 0;
			iter.b() = 0;

			// Not optimal mapping, TODO: color from histogram
			float depthValue = (float)*depthChannel->getData(iter.x(), iter.y()); 
			if (depthValue > 100 && depthValue < 1000)
			{
				int colorValue = (int)lmap<float>(depthValue, 100, 1000, 255, 0);
				iter.r() = colorValue;
				iter.g() = colorValue;
			}
		}
	}

	mTexDepth->update(*depthSurface);
}

void ImageViewer::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::draw(mTexDepth, Rectf({ vec2(0), vec2(mRgbDims.x, mRgbDims.y) }));
	gl::draw(mTexRgb, vec2(mRgbDims.x, 0));
}

void ImageViewer::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP( ImageViewer, RendererGl )
