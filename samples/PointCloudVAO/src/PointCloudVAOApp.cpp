#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PointCloudVAOApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void PointCloudVAOApp::setup()
{
}

void PointCloudVAOApp::mouseDown( MouseEvent event )
{
}

void PointCloudVAOApp::update()
{
}

void PointCloudVAOApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( PointCloudVAOApp, RendererGl )
