#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class InstanceCloudApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

	struct MeshInstance
	{
		vec3 IPosition;
		Color IColor;
		float ISize;
		MeshInstance(vec3 pPos, Color pCol, float pSize) :IPosition(pPos), IColor(pCol), ISize(pSize){}
	};

private:
	void setupGUI();
	void setupDSAPI();
	void setupMesh();

	gl::VboRef mInstanceData;
	gl::BatchRef mInstanceDraw;

	vector<MeshInstance> mInstances;
	CinderDSRef mCinderDS;

	CameraPersp mView;
	CameraUi mViewCtrl;

	params::InterfaceGlRef mGUI;
	int		mParamStep;
	float	mParamDepthMin,
			mParamDepthMax,
			mParamPointSize,
			mParamLightY,
			mParamAmbientStr,
			mParamSpecularStr,
			mParamSpecularPow;
};

void InstanceCloudApp::setup()
{
	setupGUI();
	setupDSAPI();
	setupMesh();
}

void InstanceCloudApp::setupGUI()
{
	getWindow()->setSize(1280, 720);

	mParamStep = 2;
	mParamDepthMin = 100.0f;
	mParamDepthMax = 1000.0f;
	mParamPointSize = 4.0f;
	mParamAmbientStr = 0.25f;
	mParamLightY = 1000.0f;
	mParamSpecularStr = 1.0f;
	mParamSpecularPow = 16.0f;

	mGUI = params::InterfaceGl::create("Settings", ivec2(300, 200));
	mGUI->addSeparator();
	mGUI->addParam<int>("paramStepSize", &mParamStep).optionsStr("label='Point Spacing'");
	mGUI->addParam<float>("paramDepthMin", &mParamDepthMin).optionsStr("label='Min Depth'");
	mGUI->addParam<float>("paramDepthMax", &mParamDepthMax).optionsStr("label='Max Depth'");
	mGUI->addParam<float>("paramPointSize", &mParamPointSize).optionsStr("label='Point Size'");
	mGUI->addSeparator();
	mGUI->addParam<float>("paramLightY", &mParamLightY).optionsStr("label='Light Height'");
	mGUI->addParam<float>("paramAmbientStr", &mParamAmbientStr).optionsStr("label='Ambient'");
	mGUI->addParam<float>("paramSpecularPow", &mParamSpecularPow).optionsStr("label='Specular Size'");
	mGUI->addParam<float>("paramSpecularStr", &mParamSpecularStr).optionsStr("label='Specular Strength'");
}

void InstanceCloudApp::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();

	mCinderDS->init();
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->start();
}

void InstanceCloudApp::setupMesh()
{
	auto instanceShader = gl::GlslProg::create(loadAsset("instances.vert"), loadAsset("instances.frag"));
	auto instanceMesh = gl::VboMesh::create(geom::Sphere().radius(1.0).subdivisions(8));

	mInstanceData = gl::Vbo::create(GL_ARRAY_BUFFER, mInstances.size()*sizeof(MeshInstance), mInstances.data(), GL_DYNAMIC_DRAW);

	geom::BufferLayout instanceAttribs;
	instanceAttribs.append(geom::CUSTOM_0, 3, sizeof(MeshInstance), offsetof(MeshInstance, IPosition), 1);
	instanceAttribs.append(geom::CUSTOM_1, 4, sizeof(MeshInstance), offsetof(MeshInstance, IColor), 1);
	instanceAttribs.append(geom::CUSTOM_2, 1, sizeof(MeshInstance), offsetof(MeshInstance, ISize), 1);

	instanceMesh->appendVbo(instanceAttribs, mInstanceData);
	mInstanceDraw = gl::Batch::create(instanceMesh, instanceShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iColor" }, { geom::CUSTOM_2, "iSize" } });
	vec2 cFOV = mCinderDS->getDepthFOVs();

	mView.setPerspective(45.0f, getWindowAspectRatio(), 10.0f, 4000.0f);
	mView.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, -1, 0));
	mView.setPivotDistance(450.0f);
	mViewCtrl = CameraUi(&mView, getWindow());
}

void InstanceCloudApp::update()
{
	mCinderDS->update();
	mInstances.clear();
	
	auto depthChannel = mCinderDS->getDepthFrame();
	auto iter = depthChannel->getIter();

	while (iter.line())
	{
		while (iter.pixel())
		{
			if (iter.x() % mParamStep == 0 && iter.y() % mParamStep == 0)
			{
				float depthValue = (float)iter.v();
				if (depthValue > mParamDepthMin&&depthValue < mParamDepthMax)
				{
					vec3 pos(static_cast<float>(iter.x()), static_cast<float>(iter.y()), depthValue);
					auto world = mCinderDS->getDepthSpacePoint(pos);
					Color rgb = mCinderDS->getColorFromDepthSpace(world);
					mInstances.push_back(MeshInstance(world, rgb, mParamPointSize));
				}
			}
		}
	}

	mInstanceData->bufferData(mInstances.size()*sizeof(MeshInstance), mInstances.data(), GL_DYNAMIC_DRAW);
}

void InstanceCloudApp::draw()
{
	gl::clear(Color(0.1f, 0.25f, 0));
	gl::enableDepthRead();
	gl::setMatrices(mView);
	mInstanceDraw->getGlslProg()->uniform("u_EyeDir", mView.getViewDirection());
	mInstanceDraw->getGlslProg()->uniform("u_LightPos", mParamLightY);
	mInstanceDraw->getGlslProg()->uniform("u_AmbientStrength", mParamAmbientStr);
	mInstanceDraw->getGlslProg()->uniform("u_SpecularStrength", mParamSpecularStr);
	mInstanceDraw->getGlslProg()->uniform("u_SpecularPower", mParamSpecularPow);
	mInstanceDraw->drawInstanced(mInstances.size());

	gl::setMatricesWindow(getWindowSize());
	mGUI->draw();
}

void InstanceCloudApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP(InstanceCloudApp, RendererGl)
