#include "cinder/app/AppNative.h"
#include "cinder/Font.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void BasicApp::setup()
{
}

void BasicApp::mouseDown( MouseEvent event )
{
}

void BasicApp::update()
{
}

void BasicApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( BasicApp, RendererGl )
