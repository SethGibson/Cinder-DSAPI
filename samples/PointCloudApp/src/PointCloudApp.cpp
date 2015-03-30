#ifdef _DEBUG
#pragma comment(lib, "DSAPI32.dbg.lib")
#else
#pragma comment(lib, "DSAPI32.lib")
#endif

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class PointCloudApp : public App
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;

	void exit();

	struct CloudPoint
	{
		vec3 PPosition;
		vec2 PTexCoord;
		CloudPoint(vec3 pPos, vec2 pUV) : PPosition(pPos), PTexCoord(pUV){}
	};

private:
	void setupMesh();
	void setupDSAPI();

	gl::VboRef mBufferObj;
	geom::BufferLayout mAttribObj;
	gl::VboMeshRef mMeshObj;
	gl::BatchRef mDrawObj;
	gl::GlslProgRef mShaderObj;
	gl::Texture2dRef mTexRgb;

	CameraPersp mCamera;
	MayaCamUI mMayaCam;

	CinderDSRef mCinderDS;
	ivec2 mDepthDims, mRgbDims;
	vector<CloudPoint> mPoints;
};

void PointCloudApp::setup()
{
	getWindow()->setSize(1280, 720);
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 2000.0f);
	mCamera.lookAt(vec3(0, 0, 0), vec3(0,0,1), vec3(0, -1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0,0,750.0));
	mMayaCam.setCurrentCam(mCamera);

	setupDSAPI();
	setupMesh();

	getSignalCleanup().connect(std::bind(&PointCloudApp::exit, this));
}

void PointCloudApp::setupMesh()
{
	try
	{
		mShaderObj = gl::GlslProg::create(loadAsset("pointcloud_vert.glsl"), loadAsset("pointcloud_frag.glsl"));
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
}

void PointCloudApp::setupDSAPI()
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

void PointCloudApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown(event.getPos());
}

void PointCloudApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void PointCloudApp::update()
{
	mCinderDS->update();
	mPoints.clear();
	Channel16u cChanDepth = mCinderDS->getDepthFrame();

	mTexRgb->update(mCinderDS->getRgbFrame());
	for (int dy = 0; dy < mDepthDims.y; ++dy)
	{
		for (int dx = 0; dx < mDepthDims.x; ++dx)
		{
			float cDepth = (float)*cChanDepth.getData(dx, dy);
			if (cDepth > 100 && cDepth < 1000)
			{
				vec3 cPos = mCinderDS->getZCameraSpacePoint(vec3(dx, dy, cDepth));
				vec2 cUV = mCinderDS->getColorSpaceCoordsFromZImage(static_cast<float>(dx),
														static_cast<float>(dy),
														cDepth);

				mPoints.push_back(CloudPoint(cPos, cUV));
			}
		}
	}

	mBufferObj->bufferData(mPoints.size()*sizeof(CloudPoint), mPoints.data(), GL_DYNAMIC_DRAW);
	mMeshObj = gl::VboMesh::create(mPoints.size(), GL_POINTS, { { mAttribObj, mBufferObj } });
	mDrawObj->replaceVboMesh(mMeshObj);
}

void PointCloudApp::draw()
{
	gl::clear( Color( 0.25f, 0.1f, 0.15f ) ); 
	gl::enableAdditiveBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::setMatrices(mMayaCam.getCamera());

	gl::ScopedTextureBind cTexture(mTexRgb);
	gl::pointSize(2.0f);
	mDrawObj->draw();
}

void PointCloudApp::exit()
{
	mCinderDS->stop();
}

CINDER_APP( PointCloudApp, RendererGl )
