#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

struct Pt
{
	vec3	PPos;
	Color	PCol;
	Pt(vec3 pPos, Color pCol) : PPos(pPos), PCol(pCol){}
};

class PointCloudVAO : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;

private:
	void setupGUI();
	void setupScene();
	void setupCloud();

	void updateCloud();

	void drawCloud();
	void drawGUI();

	CinderDSRef	mDS;
	Channel16u	mChanDepth;

	vector<Pt>	mPoints;
	gl::VaoRef		mVao;
	gl::VboRef		mVbo;
	gl::BatchRef	mBatch;
	gl::GlslProgRef	mShader;

	CameraPersp		mSceneView;
	CameraUi		mSceneCtrl;

	params::InterfaceGlRef mGUI;
	int		mParamStep;
	float	mParamDepthMin,
			mParamDepthMax,
			mParamPointSize;
};

void PointCloudVAO::setup()
{
	setupGUI();
	setupScene();
	setupCloud();
}

void PointCloudVAO::update()
{
	updateCloud();
}

void PointCloudVAO::draw()
{
	gl::clear( Color8u( 0, 113, 197 ) ); 
	drawCloud();
	drawGUI();
}

void PointCloudVAO::setupGUI()
{
	mParamStep = 2;
	mParamDepthMin = 100.0f;
	mParamDepthMax = 1000.0f;
	mParamPointSize = 4.0f;

	mGUI = params::InterfaceGl::create("Settings", ivec2(300, 200));
	mGUI->addSeparator();
	mGUI->addParam<int>("paramStep", &mParamStep).optionsStr("label='Spacing'");
	mGUI->addParam<float>("paramDepthMin", &mParamDepthMin).optionsStr("label='Min Depth'");
	mGUI->addParam<float>("paramDepthMax", &mParamDepthMax).optionsStr("label='Max Depth'");
	mGUI->addParam<float>("paramPointSize", &mParamPointSize).optionsStr("label='Point Size'");
}

void PointCloudVAO::setupScene()
{
	getWindow()->setSize(1280, 720);
	setFrameRate(60);

	mDS = CinderDSAPI::create();
	mDS->init();
	mDS->initDepth(FrameSize::DEPTHSD, 60);
	mDS->initRgb(FrameSize::RGBVGA, 60);
	mDS->start();

	vec2 fovs = mDS->getDepthFOVs();
	mSceneView.setPerspective(fovs.y, getWindowAspectRatio(), 1.0f, 6000.0f);
	mSceneView.lookAt(vec3(0), vec3(0, 0, 1000), vec3(0, -1, 0));
	mSceneCtrl = CameraUi(&mSceneView, getWindow());
}


void PointCloudVAO::setupCloud()
{
	mShader = gl::GlslProg::create(loadAsset("shaders/points.vert"), loadAsset("shaders/points.frag"));
	GLint posLoc = mShader->getAttribLocation("vPosition");
	GLint colorLoc = mShader->getAttribLocation("vColor");
	mVao = gl::Vao::create();
	mVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints, GL_DYNAMIC_DRAW);

	gl::ScopedVao vao(mVao);
	gl::ScopedBuffer vbo(mVbo);
	gl::enableVertexAttribArray(posLoc);
	gl::vertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Pt), (const GLvoid*)offsetof(Pt, PPos));
	gl::enableVertexAttribArray(colorLoc);
	gl::vertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Pt), (const GLvoid*)offsetof(Pt, PCol));

	gl::enable(GL_PROGRAM_POINT_SIZE);
}

void PointCloudVAO::updateCloud()
{
	mDS->update();
	auto depth = mDS->getDepthFrame();
	auto iter = depth->getIter();

	mPoints.clear();
	while (iter.line())
	{
		while (iter.pixel())
		{
			if (iter.x() % mParamStep == 0 && iter.y() % mParamStep == 0)
			{
				float x = (float)iter.x();
				float y = (float)iter.y();
				float z = (float)iter.v();
				if (z > mParamDepthMin && z < mParamDepthMax)
				{
					auto world = mDS->getDepthSpacePoint(vec3(x, y, z));
					auto diffuse = mDS->getColorFromDepthSpace(world);
					mPoints.push_back(Pt(world, diffuse));
				}
			}
		}
	}

	mVbo->bufferData(mPoints.size()*sizeof(Pt), mPoints.data(), GL_DYNAMIC_DRAW);
}

void PointCloudVAO::drawCloud()
{
	gl::enableDepthRead();
	gl::setMatrices(mSceneView);

	mShader->uniform("u_PointSize", mParamPointSize);
	gl::ScopedVao vao(mVao);
	gl::ScopedGlslProg glsl(mShader);

	gl::context()->setDefaultShaderVars();
	gl::drawArrays(GL_POINTS, 0, mPoints.size());
	gl::disableDepthRead();
}

void PointCloudVAO::drawGUI()
{
	gl::setMatricesWindow(getWindowSize());
	mGUI->draw();
}

CINDER_APP( PointCloudVAO, RendererGl )
