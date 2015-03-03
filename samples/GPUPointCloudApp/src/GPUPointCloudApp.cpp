#ifdef _DEBUG
#pragma comment(lib, "DSAPI32.dbg.lib")
#else
#pragma comment(lib, "DSAPI32.lib")
#endif

#include "cinder/app/AppNative.h"
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

class GPUPointCloudApp : public AppNative
{
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;

	void stopDS();

	struct CloudPoint
	{
		vec3 PPosition;
		vec3 PNormal;
		vec2 PTexCoord;
		CloudPoint(vec3 pPos, vec3 pNorm, vec2 pUV) : PPosition(pPos), PNormal(pNorm), PTexCoord(pUV){}
	};

private:
	void setupMesh();
	void setupDSAPI();

	gl::VboRef mBufferObj;
	geom::BufferLayout mAttribObj;
	gl::VboMeshRef mMeshObj;
	gl::BatchRef mDrawObj;
	gl::GlslProgRef mShaderObj;
	gl::Texture2dRef mTexDepth;
	Surface8u mSurfDepth;

	CameraPersp mCamera;
	MayaCamUI mMayaCam;

	CinderDSRef mCinderDS;
	ivec2 mDepthDims;
};

void GPUPointCloudApp::setup()
{
	getWindow()->setSize(1280, 720);
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(vec3(0, 0, 4), vec3(0), vec3(0, 1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0));
	mMayaCam.setCurrentCam(mCamera);

	setupDSAPI();
	setupMesh();

	getSignalShutdown().connect(std::bind(&GPUPointCloudApp::stopDS, this));
}

void GPUPointCloudApp::setupMesh()
{
	try
	{
		mShaderObj = gl::GlslProg::create(loadAsset("gpucloud_vert.glsl"), loadAsset("gpucloud_frag.glsl"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}

	vector<CloudPoint> cPoints;
	for (int vy = 0; vy < mDepthDims.y; vy++)
	{
		for (int vx = 0; vx < mDepthDims.x; vx++)
		{
			float cy = lmap<float>(vy, 0, mDepthDims.y, 1.0f, -1.0f);
			float cx = lmap<float>(vx, 0, mDepthDims.x, 1.3333f, -1.3333f);
			float cz = 0.0;

			cPoints.push_back(CloudPoint(vec3(cx, cy, cz), vec3(0, 0, 0), vec2(1.0 - vx / (float)mDepthDims.x, 1.0f - vy / (float)mDepthDims.y)));
		}
	}

	//normals
	int id = 1;
	for (int vy = 0; vy < mDepthDims.y; vy++)
	{
		for (int vx = 0; vx < mDepthDims.x; vx++)
		{
			if (id < mDepthDims.x*mDepthDims.y - 1)
			{
				vec3 v1 = cPoints[id - 1].PPosition;
				vec3 v2 = cPoints[id + 1].PPosition;

				vec3 v0 = v1 + v2;
				v0 += cPoints[id].PPosition;

				cPoints[id].PNormal = v0;
				id++;
			}
		}
	}

	mBufferObj = gl::Vbo::create(GL_ARRAY_BUFFER, cPoints, GL_STATIC_DRAW);
	mAttribObj.append(geom::POSITION, 3, sizeof(CloudPoint), offsetof(CloudPoint, PPosition));
	mAttribObj.append(geom::NORMAL, 3, sizeof(CloudPoint), offsetof(CloudPoint, PNormal));
	mAttribObj.append(geom::TEX_COORD_0, 2, sizeof(CloudPoint), offsetof(CloudPoint, PTexCoord));

	mMeshObj = gl::VboMesh::create(cPoints.size(), GL_POINTS, { { mAttribObj, mBufferObj } });
	mDrawObj = gl::Batch::create(mMeshObj, mShaderObj);

	mTexDepth = gl::Texture2d::create(mDepthDims.x, mDepthDims.y);
	mSurfDepth = Surface8u(mDepthDims.x, mDepthDims.y, false, SurfaceChannelOrder::RGB);
}

void GPUPointCloudApp::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();
	if (mCinderDS->init())
		mCinderDS->initDepth(FrameSize::DEPTHSD, 60);
	else
	{
		console() << "Error on DS Init" << endl;
		quit();
	}

	mDepthDims = mCinderDS->getDepthSize();
	mCinderDS->start();
}

void GPUPointCloudApp::mouseDown(MouseEvent event)
{
	mMayaCam.mouseDown(event.getPos());
}

void GPUPointCloudApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void GPUPointCloudApp::update()
{
	if (mCinderDS->update())
	{
		Channel16u cDepth = mCinderDS->getDepthFrame();
		Surface8u::Iter cIter = mSurfDepth.getIter();

		while (cIter.line())
		{
			while (cIter.pixel())
			{
				cIter.r() = 0;
				cIter.g() = 0;
				cIter.b() = 0;

				float cVal = (float)*cDepth.getData(cIter.x(), cIter.y());
				if (cVal > 100 && cVal < 1000)
					cIter.r() = (uint8_t)lmap<float>(cVal, 100, 1000, 255, 0);

			}
		}
		mTexDepth->update(mSurfDepth);
	}
}

void GPUPointCloudApp::draw()
{
	gl::clear(Color(0.25f, 0.1f, 0.15f));
	gl::enableAdditiveBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::setMatrices(mMayaCam.getCamera());

	gl::ScopedTextureBind cTexture(mTexDepth);
	mDrawObj->draw();
}

void GPUPointCloudApp::stopDS()
{
	mCinderDS->stop();
}

CINDER_APP_NATIVE(GPUPointCloudApp, RendererGl)
