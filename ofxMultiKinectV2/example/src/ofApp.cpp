#include "ofMain.h"
#include "ofxMultiKinectV2.h"

#define STRINGIFY(x) #x

static string depthFragmentShader =
STRINGIFY(
    uniform sampler2DRect tex;
    void main()
    {
      vec4 col = texture2DRect(tex, gl_TexCoord[0].xy);
        float value = col.r;
        float low1 = 500.0;
        float high1 = 5000.0;
        float low2 = 1.0;
        float high2 = 0.0;
        float d = clamp(low2 + (value - low1) * (high2 - low2) / (high1 - low1), 0.0, 1.0);
        if (d == 1.0) {
            d = 0.0;
        }
      gl_FragColor = vec4(vec3(d), 1.0);
    }
);

static string irFragmentShader =
STRINGIFY(
    uniform sampler2DRect tex;
    void main()
    {
        vec4 col = texture2DRect(tex, gl_TexCoord[0].xy);
        float value = col.r / 65535.0;
        gl_FragColor = vec4(vec3(value), 1.0);
    }
);

//========================================================================
class ofApp : public ofBaseApp{
    ofShader depthShader;
    ofShader irShader;
    
    ofxMultiKinectV2 kinect0;
    ofxMultiKinectV2 kinect1;
    ofTexture colorTex0;
    ofTexture colorTex1;
    ofTexture depthTex0;
    ofTexture depthTex1;
    ofTexture irTex0;
    ofTexture irTex1;
public:
    void setup()
    {
        ofSetVerticalSync(true);
        ofSetFrameRate(60);
        
        depthShader.setupShaderFromSource(GL_FRAGMENT_SHADER, depthFragmentShader);
        depthShader.linkProgram();
        
        irShader.setupShaderFromSource(GL_FRAGMENT_SHADER, irFragmentShader);
        irShader.linkProgram();
        
        kinect0.open(true, true, 0);
        kinect1.open(true, true, 1);
        // Note :
        // Default OpenCL device might not be optimal.
        // e.g. Intel HD Graphics will be chosen instead of GeForce.
        // To avoid it, specify OpenCL device index manually like following.
        // kinect1.open(true, true, 0, 2); // GeForce on MacBookPro Retina
        
        kinect0.start();
        kinect1.start();
    }
    
    void update() {
        kinect0.update();
        if (kinect0.isFrameNew()) {
            colorTex0.loadData(kinect0.getColorPixelsRef());
            depthTex0.loadData(kinect0.getDepthPixelsRef());
            irTex0.loadData(kinect0.getIrPixelsRef());
        }
        kinect1.update();
        if (kinect1.isFrameNew()) {
            colorTex1.loadData(kinect1.getColorPixelsRef());
            depthTex1.loadData(kinect1.getDepthPixelsRef());
            irTex1.loadData(kinect1.getIrPixelsRef());
        }
    }
    
    void draw()
    {
        ofClear(0);

        if (colorTex0.isAllocated()) {
            colorTex0.draw(0, 0, 640, 360);
        }
        if (depthTex0.isAllocated()) {
            depthShader.begin();
            depthTex0.draw(640, 0, 512, 424);
            depthShader.end();
            irShader.begin();
            irTex0.draw(1152, 0, 512, 424);
            irShader.end();
        }
        if (colorTex1.isAllocated()) {
            colorTex1.draw(0, 424, 640, 360);
        }
        if (depthTex1.isAllocated()) {
            depthShader.begin();
            depthTex1.draw(640, 424, 512, 424);
            depthShader.end();
            irShader.begin();
            irTex1.draw(1152, 424, 512, 424);
            irShader.end();
        }
        
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
        ofDrawBitmapStringHighlight("Device Count : " + ofToString(ofxMultiKinectV2::getDeviceCount()), 10, 40);
    }
    
    void keyPressed(int key)
    {
        if (key == ' ') {
            kinect0.setEnableFlipBuffer(!kinect0.isEnableFlipBuffer());
        }
    }
};

//========================================================================
int main( ){
    ofSetupOpenGL(1680,800,OF_WINDOW);            // <-------- setup the GL context
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    
}
