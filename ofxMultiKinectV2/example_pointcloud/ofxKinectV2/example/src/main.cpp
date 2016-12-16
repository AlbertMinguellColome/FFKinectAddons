#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofSetupOpenGL(1400, 1000, OF_WINDOW);			// <-------- setup the GL context
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 2);
    settings.width = 1920;
    settings.height = 1080;
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
