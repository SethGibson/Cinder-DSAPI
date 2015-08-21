#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class PointCloudBatch : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

	struct CloudPoint
	{
		vec3 PPosition;
		vec2 PTexCoord;
		CloudPoint(vec3 pPos, vec2 pUV) : PPosition(pPos), PTexCoord(pUV){}
	};

private:
	void setupMesh();
	void setupDSAPI();
	void setupGUI();

	gl::VboRef mBufferObj;
	geom::BufferLayout mAttribObj;
	gl::VboMeshRef mMeshObj;
	gl::BatchRef mDrawObj;
	gl::GlslProgRef mShaderObj;
	gl::Texture2dRef mTexRgb;

	CameraPersp mView;
	CameraUi mViewCtrl;

	CinderDSRef mCinderDS;
	ivec2 mDepthDims, mRgbDims;
	vector<CloudPoint> mPoints;

	params::InterfaceGlRef	mGUI;
	float					mParamMinDepth,
							mParamMaxDepth,
							mParamPointSize;
};

void PointCloudBatch::setup()
{
	setupGUI();

	getWindow()->setSize(1280, 720);
	mView.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 2000.0f);
	mView.lookAt(vec3(0, 0, 0), vec3(0,0,1), vec3(0, -1, 0));
	mView.setPivotDistance(750.0f);
	mViewCtrl = CameraUi(&mView, getWindow());

	setupDSAPI();
	setupMesh();
}

void PointCloudBatch::setupGUI()
{
	mParamMinDepth = 100.0f;
	mParamMaxDepth = 1000.0f;
	mParamPointSize = 4.0f;
	mGUI = params::InterfaceGl::create("Settings", ivec2(300, 100));
	mGUI->addSeparator();
	mGUI->addParam<float>("paramMinDepth", &mParamMinDepth).optionsStr("label='Min Depth'");
	mGUI->addParam<float>("paramMaxDepth", &mParamMaxDepth).optionsStr("label='Max Depth'");
	mGUI->addParam<float>("paramPointSize", &mParamPointSize).optionsStr("label='Point Size'");
}

void PointCloudBatch::setupMesh()
{
	try
	{
		mShaderObj = gl::GlslProg::create(loadAsset("pointcloud.vert"), loadAsset("pointcloud.frag"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}

	mPoints.clear();
	for (int vy = 0; vy < mDepthDims.y; vy++)
	{
		for (int vx = 0; vx < mDepthDims.x; vx++)
		{
			mPoints.push_back(CloudPoint(vec3(0), vec2(0)));
		}
	}

	mBufferObj = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints, GL_DYNAMIC_DRAW);
	mAttribObj.append(geom::POSITION, 3, sizeof(CloudPoint), offsetof(CloudPoint, PPosition));
	mAttribObj.append(geom::TEX_COORD_0, 2, sizeof(CloudPoint), offsetof(CloudPoint, PTexCoord));

	mMeshObj = gl::VboMesh::create(mPoints.size(), GL_POINTS, { { mAttribObj, mBufferObj } });
	mDrawObj = gl::Batch::create(mMeshObj, mShaderObj);

	mTexRgb = gl::Texture2d::create(mRgbDims.x, mRgbDims.y);

	gl::enable(GL_PROGRAM_POINT_SIZE);
}

void PointCloudBatch::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();
	
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHSD, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mDepthDims = mCinderDS->getDepthSize();
	mRgbDims = mCinderDS->getRgbSize();

	mTexRgb = gl::Texture::create(mRgbDims.x, mRgbDims.y);
	mCinderDS->start();
}

void PointCloudBatch::update()
{
	mCinderDS->update();
	mTexRgb->update(*mCinderDS->getRgbFrame());

	mPoints.clear();
	auto depthChannel = mCinderDS->getDepthFrame();
	auto iter = depthChannel->getIter();

	while (iter.line())
	{
		while (iter.pixel())
		{
			float depthValue = (float)*depthChannel->getData(iter.x(), iter.y());
			if (depthValue > mParamMinDepth && depthValue < mParamMaxDepth)
			{
				vec3 pos(static_cast<float>(iter.x()), static_cast<float>(iter.y()), depthValue);
				vec3 world = mCinderDS->getDepthSpacePoint(pos);
				vec2 uv = mCinderDS->getColorCoordsFromDepthSpace(world);

				mPoints.push_back(CloudPoint(world, uv));
			}
		}
	}

	mBufferObj->bufferData(mPoints.size()*sizeof(CloudPoint), mPoints.data(), GL_DYNAMIC_DRAW);
}

void PointCloudBatch::draw()
{
	gl::clear( Color( 0.25f, 0.1f, 0.15f ) ); 
	gl::enableAdditiveBlending();
	gl::enableDepthRead();
	gl::setMatrices(mView);

	gl::ScopedTextureBind cTexture(mTexRgb);
	mDrawObj->getGlslProg()->uniform("u_PointSize", mParamPointSize);
	mDrawObj->draw();

	gl::setMatricesWindow(getWindowSize());
	mGUI->draw();
}

void PointCloudBatch::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP( PointCloudBatch, RendererGl )
