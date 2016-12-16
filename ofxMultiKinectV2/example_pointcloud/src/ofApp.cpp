#include "ofMain.h"
#include "ofxMultiKinectV2.h"
#include "GpuRegistration.h"
#include "ofxDelaunay.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"

#define MAX_KINECT_DISTANCE 150
#define MIN_KINECT_DISTANCE 40


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

static string colorFragmentShader =
STRINGIFY(
          uniform sampler2DRect tex;
   //       uniform float time;
  //        uniform mat4 modelViewProjectionMatrix;
 //         in vec4 position;

          void main()
          {
              vec4 col;
              col.r= 0.9;
              col.g=0.6;
              col.b=0.8;
              col.a=1.0;
              gl_FragColor = col;
              
           //   float displacementHeight = 400;
//              float displacementY = sin(time + (position.x / 100.0)) * displacementHeight;
//              
//              vec4 modifiedPosition = modelViewProjectionMatrix * position;
//              modifiedPosition.y += displacementY;
//              gl_Position = modifiedPosition;
          }
          );


//========================================================================
class ofApp : public ofBaseApp{
    
    ofxCvGrayscaleImage blob;
    // gui
    bool showGui;
    ofxPanel gui;
    ofxSlider<int> colorAlpha;
    ofxSlider<float> noiseAmount;
    ofxToggle useRealColors;
    ofxSlider<int> pointSkip;
    ofMesh convertedMesh;
    ofMesh wireframeMesh;
    

    ofImage image;
    ofImage imageRGB;
    ofImage imageIR;
    ofImage imageDepth;
    
    ofxMultiKinectV2 kinect0;
    
    ofEasyCam ecam;
    ofShader depthShader;
    ofShader irShader;
    ofShader colorShader;
    
    ofTexture colorTex0;
    ofTexture depthTex0;
    ofTexture irTex0;
    GpuRegistration gr;
    ofxDelaunay triangulation;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    ofxCvContourFinder contourFinder;
    ofParameter<int> nearThreshold;
    ofParameter<int> farThreshold;
    ofParameter<int> step;
    ofParameter<bool> stepOffset;
    
    ofParameter<bool> maskPointcloud;
    ofParameter<bool> pointcloud;
    ofParameter<bool> makeTriangles;
    
    bool bThreshWithOpenCV;
    bool bDrawPointCloud;
    

    ofVboMesh vbo;
    bool process_occlusion;
    
    ofMesh mesh;	// ofVboMesh
    
//    vector< vector<ofVec3f> > points;
//    vector< vector<ofColor> > colors;
//    vector< vector<int> > indexs;
    
    vector< vector<ofVec3f> > points;
    vector< vector<ofColor> > colors;
    vector< vector<int> > indexs;
    
public:
    
    void setup()
    {
        nearThreshold = 255;
        farThreshold = 0;
        bThreshWithOpenCV = true;
        
        ofSetFrameRate(30);
        //process_occlusion = true;
       // gr.setup(kinect0.getProtonect(), 2);
        
        depthShader.setupShaderFromSource(GL_FRAGMENT_SHADER, depthFragmentShader);
        depthShader.linkProgram();
        
        irShader.setupShaderFromSource(GL_FRAGMENT_SHADER, irFragmentShader);
        irShader.linkProgram();
        
        colorShader.setupShaderFromSource(GL_FRAGMENT_SHADER, colorFragmentShader);
        colorShader.linkProgram();
        //------
       // ofSetVerticalSync(true);
        //ofSetFrameRate(60);
        
       // kinect0.open(false, true, 0);
        // Note :
        // Default OpenCL device might not be optimal.
        // e.g. Intel HD Graphics will be chosen instead of GeForce.
        // To avoid it, specify OpenCL device index manually like following.
         kinect0.open(true, true, 0, 0); // GeForce on MacBookPro Retina
        
        kinect0.start();
        
      //  mesh.setUsage(GL_DYNAMIC_DRAW);
       // mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
        
        //ecam.setAutoDistance(false);
        //ecam.setDistance(200);
        
        vbo.clear();
        vbo.setMode(OF_PRIMITIVE_POINTS);
        
//        //Gl config
//        glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
//        glPointSize(4); // make the points bigger
//        glShadeModel(GL_FLAT);
//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_POINT_SMOOTH);
        
        
      //  kinect0.setRegistration(true);
        
       // blob.allocate(640,480,OF_IMAGE_GRAYSCALE);
        
        
        // Gui
        gui.setup();
        gui.setPosition(10,10);
        gui.add(noiseAmount.setup("Noise Amount", 0.0, 0.0,20.0));
        gui.add(pointSkip.setup("Point Skip", 1, 1,20));
        gui.add(useRealColors.setup("Real Colors", false));
        gui.add(colorAlpha.setup("Color Alpha", 255,0,255));
        gui.loadFromFile("settings.xml");
        showGui = true;
        
    }
    
    void update() {
    
        
        kinect0.update();
        
        mesh.clear();
        points.clear();
        colors.clear();
        indexs.clear();
        
        int step = 5;
        int total = 0;
        
        int h = kinect0.getDepthPixelsRef().getHeight();
        int w = kinect0.getDepthPixelsRef().getWidth();
        
        for (int j = 0; j < h; j+=step)
        {
            vector<ofVec3f> temppoints;
            vector<ofColor> tempcolors;
    
            points.push_back(temppoints);
            colors.push_back(tempcolors);
            
            for (int i = 0; i < w; i+=step)
            {
                float distance = kinect0.getDistanceAt(i, j);
                if(distance>50 && distance<1000)
                {
                    ofVec3f tempPoint;
                    ofColor tempColor;
                    
                    tempPoint = ofVec3f(i, j, distance*-2.0 );
                  //  tempColor = ofColor(kinect.getColorAt(i,j));
                    
                    points[j/step].push_back(tempPoint);
                    colors[j/step].push_back(tempColor);
                    
                    total++;
                }else{
                    ofVec3f tempPoint2;
                    ofColor tempColor2;
                    tempPoint2 = ofVec3f(i,j,0);	//範囲外には深度0
                    tempColor2 = ofColor(0);
                    points[j/step].push_back(tempPoint2);
                    colors[j/step].push_back(tempColor2);
                }
            }
        }
        
        int ind = 0;
        for (int m = 0; m < h; m+=step)
        {
            vector<int> tempindexs;
            indexs.push_back(tempindexs);
            
            for (int n = 0; n < w; n+=step)
            {
                if(points[m/step][n/step].z != 0){
                              cout << points[m][n] << endl;
                    mesh.addColor(colors[m/step][n/step]);
                    mesh.addVertex(points[m/step][n/step]);
                    
                    indexs[m/step].push_back(ind);
                    ind++;
                }else{
                    indexs[m/step].push_back(-1);
                }
            }
        }
        
        int W = int(w/step);
        for (int b = 0; b < h-step; b+=step){
            for (int a = 0; a < w-1; a+=step)
            {
                if( (indexs[int(b/step)][int(a/step)]!=-1 && indexs[int(b/step)][int(a/step+1)]!=-1) && (indexs[int(b/step+1)][int(a/step+1)]!=-1 && indexs[int(b/step+1)][int(a/step)]!=-1) ){
                    
                    mesh.addTriangle(indexs[int(b/step)][int(a/step)],indexs[int(b/step)][int(a/step+1)],indexs[int(b/step+1)][int(a/step+1)]);
                    mesh.addTriangle(indexs[int(b/step)][int(a/step)],indexs[int(b/step+1)][int(a/step+1)],indexs[int(b/step+1)][int(a/step)]);
                }
            }
        }

        
        imageRGB.setFromPixels(kinect0.getColorPixelsRef());
        imageIR.setFromPixels(kinect0.getIrPixelsRef());
        imageDepth.setFromPixels(kinect0.getDepthPixelsRef());
        
        
/*        if (kinect0.isFrameNew()) {
            
            
            int h = kinect0.getDepthPixelsRef().getHeight();
int w = kinect0.getDepthPixelsRef().getWidth();
            
            mesh.clear();
            vbo.clear();
            triangulation.reset();
            {
                
                unsigned char* pix = new unsigned char[640*480];
                
                unsigned char* gpix = new unsigned char[640*480];
                
                for(int x=0;x<640;x+=1) {
                    for(int y=0;y<480;y+=1) {
                        float distance = kinect0.getDistanceAt(x, y);
                        
                        int pIndex = x + y * 640;
                        pix[pIndex] = 0;
                        
                        if(distance > MIN_KINECT_DISTANCE && distance < MAX_KINECT_DISTANCE) {
                            pix[pIndex] = 255;
                        }
                        
                    }
                }
                
                blob.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
                
                int numPoints = 0;
                
                for(int x=0;x<640;x+=pointSkip*2) {
                    for(int y=0;y<480;y+=pointSkip*2) {
                        int pIndex = x + 640 * y;
                        
                        if(blob.getPixels()[pIndex]> 0) {
                            ofVec3f wc = kinect0.getWorldCoordinateAt(x, y);
                            
                            wc.x = x - 320.0;
                            wc.y = y - 240.0;
                            
                            if(abs(wc.z) > 100 && abs(wc.z ) < 2000) {
                                
                                wc.z = -wc.z;
                                
                                wc.x += ofSignedNoise(wc.x,wc.z)*noiseAmount;
                                wc.y += ofSignedNoise(wc.y,wc.z)*noiseAmount;
                                
                                wc.x = ofClamp(wc.x, -320,320);
                                wc.y = ofClamp(wc.y, -240,240);
                                
                                triangulation.addPoint(wc);
                            }
                            numPoints++;
                        }
                        
                    }
                }
                
                if(numPoints >0)
                    triangulation.triangulate();
                
                for(int i=0;i<triangulation.triangleMesh.getNumVertices();i++) {
                    triangulation.triangleMesh.addColor(ofColor(0,0,0));
                }
                
                for(int i=0;i<triangulation.triangleMesh.getNumIndices()/3;i+=1) {
                    ofVec3f v = triangulation.triangleMesh.getVertex(triangulation.triangleMesh.getIndex(i*3));
                    
                    v.x = ofClamp(v.x, -319,319);
                    v.y = ofClamp(v.y, -239, 239);
                    
                   // ofColor c = kinect0.getColorAt(v.x+320.0, v.y+240.0);
                    ofColor c = kinect0.getColorPixelsRef().getColor(v.x+320.0, v.y+240.0);
                    if(!useRealColors)
                        c = ofColor(255,0,0);
                    
                    c.a = colorAlpha;
                    
                    triangulation.triangleMesh.setColor(triangulation.triangleMesh.getIndex(i*3),c);
                    triangulation.triangleMesh.setColor(triangulation.triangleMesh.getIndex(i*3+1),c);
                    triangulation.triangleMesh.setColor(triangulation.triangleMesh.getIndex(i*3+2),c);
                }
                
                
                convertedMesh.clear();
                wireframeMesh.clear();
                wireframeMesh.setMode(OF_PRIMITIVE_TRIANGLES);
                for(int i=0;i<triangulation.triangleMesh.getNumIndices()/3;i+=1) {
                    
                    int indx1 = triangulation.triangleMesh.getIndex(i*3);
                    ofVec3f p1 = triangulation.triangleMesh.getVertex(indx1);
                    int indx2 = triangulation.triangleMesh.getIndex(i*3+1);
                    ofVec3f p2 = triangulation.triangleMesh.getVertex(indx2);
                    int indx3 = triangulation.triangleMesh.getIndex(i*3+2);
                    ofVec3f p3 = triangulation.triangleMesh.getVertex(indx3);
                    
                    ofVec3f triangleCenter = (p1+p2+p3)/3.0;
                    triangleCenter.x += 320;
                    triangleCenter.y += 240;
                    
                    triangleCenter.x = floor(ofClamp(triangleCenter.x, 0,640));
                    triangleCenter.y = floor(ofClamp(triangleCenter.y, 0, 480));
                    
                    int pixIndex = triangleCenter.x + triangleCenter.y * 640;
                    if(pix[pixIndex] > 0) {
                        
                        convertedMesh.addVertex(p1);
                        convertedMesh.addColor(triangulation.triangleMesh.getColor(indx1));
                        
                        convertedMesh.addVertex(p2);
                        convertedMesh.addColor(triangulation.triangleMesh.getColor(indx2));
                        
                        convertedMesh.addVertex(p3);
                        convertedMesh.addColor(triangulation.triangleMesh.getColor(indx3));
                        
                        wireframeMesh.addVertex(p1);
                        wireframeMesh.addVertex(p2);
                        wireframeMesh.addVertex(p3);
                    }
                }
                
                contourFinder.findContours(blob, 10, (w*h)/2, 20, false);
                
                delete pix;
                delete gpix;

        //--------
//                int step = 8;
//                int h = kinect0.getDepthPixelsRef().getHeight();
//                int w = kinect0.getDepthPixelsRef().getWidth();
//                // load grayscale depth image from the kinect source
//                //grayImage.setFromPixels(kinect0.getDepthPixelsRef(), w, h);
//                ofPixels pixels=kinect0.getDepthPixelsRef();
//                grayImage.setFromPixels(pixels,w,h);
//                image.setFromPixels(kinect0.getDepthPixelsRef());
//                if(bThreshWithOpenCV) {
//                    grayThreshNear = grayImage;
//                    grayThreshFar = grayImage;
//                    grayThreshNear.threshold(nearThreshold, true);
//                    grayThreshFar.threshold(farThreshold);
//                    cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
//                } else {
//                    
//                    // or we do it ourselves - show people how they can work with the pixels
//                    ofPixels pix = grayImage.getPixels();
//                    
//                    int numPixels = grayImage.getWidth() * grayImage.getHeight();
//                    for(int i = 0; i < numPixels; i++) {
//                        if(pix[i] < nearThreshold && pix[i] > farThreshold) {
//                            pix[i] = 255;
//                        } else {
//                            pix[i] = 0;
//                        }
//                    }
//                }
//                
//                // update the cv images
//                grayImage.flagImageChanged();
//                
//                // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
//                // also, find holes is set to true so we will get interior contours as well....
//                contourFinder.findContours(grayImage, 10, (w*h)/2, 20, false);
//                
//                for(int y = 0; y < h; y += step) {
//                    for(int x = 0; x < w; x += step) {
//                        float dist = kinect0.getDistanceAt(x, y);
//                        if(dist > MIN_KINECT_DISTANCE && dist < MAX_KINECT_DISTANCE) {
//                            ofVec3f pt = kinect0.getWorldCoordinateAt(x, y, dist);
//                            
//                            ofColor c;
//                            float h = ofMap(dist, 50, 200, 0, 255, true);
//                            c.setHsb(h, 255, 255);
//                            c = kinect0.getColorPixelsRef().getColor(x,y);
//                            //mesh.addColor(c);
//                            //mesh.addVertex(pt);
//                            vbo.addVertex(pt);
//                            triangulation.addPoint(ofPoint(pt.x,pt.y,pt.y));
//                        }
//                    }
//                }
//
//            }
//            triangulation.triangulate();
//            colorTex0.loadData(kinect0.getColorPixelsRef());
//            depthTex0.loadData(kinect0.getDepthPixelsRef());
//            irTex0.loadData(kinect0.getIrPixelsRef());
//            
//            depthTex0.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
//         //   gr.update(depthTex0, colorTex0, process_occlusion);
            }
        }*/
    }
    
    void draw()
    {
        
        
        // OpenGLの設定。ここ忘れるとz深度が正しい順番にならない
        glEnable(GL_DEPTH_TEST);
        
        int step = 4;
        for (int j = 0; j < kinect0.getDepthPixelsRef().getHeight(); j+=step) {
            for (int i = 0; i < kinect0.getDepthPixelsRef().getWidth(); i+=step)
            {
                // (x,y)のz位置取得
                float distance = kinect0.getDistanceAt(i, j);
                

                if(distance>MIN_KINECT_DISTANCE && distance<MAX_KINECT_DISTANCE)
                {
                    ofSetColor(kinect0.getColorPixelsRef().getColor(i, j));
                    
                    ofPushMatrix();
                    ofTranslate(i, j, distance*-2.0);
                    ofDrawCircle(0, 0, 2);
                    ofPopMatrix();
                }
            }
        }
        
        imageRGB.draw(0, 0,512,424);
        imageDepth.draw(512,0);
        imageIR.draw(0,512);
        
        
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        glLineWidth(int(4));
        mesh.draw();
        
//        ofBackground(219, 214, 217);
//        //glEnable(GL_DEPTH_TEST);
//        
//        ofPushMatrix();
//        
//        ecam.begin();
//        ecam.setScale(1,-1,1);
//        
//        ofSetColor(255,255,255);
//        ofTranslate(0, -80,1100);
//        ofFill();
//        
//       // postFx.begin();
//        
//        glPushAttrib(GL_ALL_ATTRIB_BITS);
//        glShadeModel(GL_FLAT);
//        glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
//        convertedMesh.drawFaces();
//        glShadeModel(GL_SMOOTH);
//        glPopAttrib();
//        
//        if(useRealColors) {
//            ofSetColor(30,30,30, 255);
//        } else
//            ofSetColor(124,136,128,255);
//        
//        ofPushMatrix();
//        ofTranslate(0, 0,0.5);
//        wireframeMesh.drawWireframe();
//        ofPopMatrix();
//        ecam.end();
//        ofPopMatrix();
//        
//      //  postFx.end();
//        
//        if(showGui) {
//            
//            ofPushStyle();
//            ofSetColor(255,255,255,255);
//            gui.draw();
//            ofPopStyle();
//        }
//        ofSetColor(255, 255, 255);
//        blob.draw(0, 0);
        
//        ofClear(0);
//        
//        if (vbo.getVertices().size()) {
//            ofPushStyle();
//            glPointSize(2);
//            ecam.begin();
//            ofDrawAxis(100);
//            ofPushMatrix();
//            glEnable(GL_DEPTH_TEST);
//            ofEnableBlendMode(OF_BLENDMODE_ADD);
//            ofEnablePointSprites();
//            ofTranslate(0, 0, -100);
//            colorShader.begin();
//            colorShader.setUniformTexture("tex", colorTex0, 0);
//           // colorShader.setUniform1f("time", ofGetElapsedTimef());
//         //   colorTex0.draw(0, 0, kinect0.getDepthPixelsRef().getWidth(),kinect0.getDepthPixelsRef().getHeight());
//            triangulation.draw();
//            //mesh.draw();
//            
//            //vbo.drawFaces();
//            colorShader.end();
//            ofDisablePointSprites();
//            glDisable(GL_DEPTH_TEST);
//            ofDisableBlendMode();
//            ofPopMatrix();
//            ecam.end();
//            ofPopStyle();
//            
//            grayImage.draw(10, 320, 400, 300);
//           // image.draw(10, 320, 400, 300);
//        }
//        
//        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
//        ofDrawBitmapStringHighlight("Device Count : " + ofToString(ofxMultiKinectV2::getDeviceCount()), 10, 40);
    }
};

//#include "ofAppGLFWWindow.h"
//========================================================================
int main( ){
    ofSetupOpenGL(1920,1080,OF_WINDOW);            // <-------- setup the GL context
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    
}
