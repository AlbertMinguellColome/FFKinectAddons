#include "ofApp.h"

//NOTE: if you are unable to connect to your device on OS X, try unplugging and replugging in the power, while leaving the USB connected.
//ofxKinectV2 will only work if the NUI Sensor shows up in the Superspeed category of the System Profiler in the USB section.

//On OS X if you are not using the example project. Make sure to add OpenCL.framework to the Link Binary With Library Build Phase 
//and change the line in Project.xcconfig to OTHER_LDFLAGS = $(OF_CORE_LIBS) $(OF_CORE_FRAMEWORKS) -framework OpenCL

//--------------------------------------------------------------
void ofApp::setup(){
    
    
#ifdef TARGET_OPENGLES
    shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        shader.load("shadersGL3/shader");
    }else{
        shader.load("shadersGL2/shader");
    }
#endif
    
    
    //Uncomment for verbose info from libfreenect2
    //ofSetLogLevel(OF_LOG_VERBOSE);

    ofBackground(30, 30, 30);

    //see how many devices we have.
    ofxKinectV2 tmp;
    vector <ofxKinectV2::KinectDeviceInfo> deviceList = tmp.getDeviceList();
    
    //allocate for this many devices
    kinects.resize(deviceList.size());
    texDepth.resize(kinects.size());
    texRGB.resize(kinects.size());
        
    panel.setup("", "settings.xml", 10, 100);
    
    //Note you don't have to use ofxKinectV2 as a shared pointer, but if you want to have it in a vector ( ie: for multuple ) it needs to be.
    for(int d = 0; d < kinects.size(); d++){
        kinects[d] = shared_ptr <ofxKinectV2> (new ofxKinectV2());
        kinects[d]->open(deviceList[d].serial);
        panel.add(kinects[d]->params);
    }

    panel.loadFromFile("settings.xml");
    
    
    

}

//--------------------------------------------------------------
void ofApp::update(){

    for(int d = 0; d < kinects.size(); d++){
        kinects[d]->update();
        if( kinects[d]->isFrameNew() ){
            texDepth[d].loadData( kinects[d]->getDepthPixels() );
            ofMesh mesh;
            ofPoint  p1(0,0);
            ofPoint  p2(0,texDepth[d].getWidth());
            ofPoint  p3(texDepth[d].getHeight(),0);
            ofPoint  p4(texDepth[d].getHeight(),texDepth[d].getWidth());
            mesh =texDepth[d].getQuad(p1,p2,p3,p4);
            texRGB[d].loadData( kinects[d]->getRgbPixels() );
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDrawBitmapString("ofxKinectV2: Work in progress addon.\nBased on the excellent work by the OpenKinect libfreenect2 team\n\n-Requires USB 3.0 port ( superspeed )\n-Requires patched libusb. If you have the libusb from ofxKinect ( v1 ) linked to your project it will prevent superspeed on Kinect V2", 10, 14);

    for(int d = 0; d < kinects.size(); d++){
        float dwHD = 1920/4;
        float dhHD = 1080/4;
        
        float shiftY = 100 + ((10 + texDepth[d].getHeight()) * d);
       // shader.begin();
        texDepth[d].draw(200, shiftY);
        texRGB[d].draw(210 + texDepth[d].getWidth(), shiftY, dwHD, dhHD);
       // shader.end();
    }
//    shader.begin();
//    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
//    shader.end();
    
    
  //  panel.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
