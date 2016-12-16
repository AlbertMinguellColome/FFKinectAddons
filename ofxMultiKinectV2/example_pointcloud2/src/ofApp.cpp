#include "ofMain.h"
#include "ofxMultiKinectV2.h"
#include "ofxPostProcessing.h"
#include "FFShader.h"
#include "ofxGui.h"


#define STRINGIFY(x) #x
#define USE_DIFFUSE_SHADER 0

static string depthFragmentShader =
STRINGIFY(
          varying vec3 normal;
          varying vec3 vertex_to_light_vector;
          
          void main(){
              //this is the fragment shader
              //this is where the pixel level drawing happens
              //gl_FragCoord gives us the x and y of the current pixel its drawing
              
              //we grab the x and y and store them in an int
              //int xVal = int(gl_FragCoord.x);
              //int yVal = int(gl_FragCoord.y);
              
              //we use the mod function to only draw pixels if they are every 2 in x or every 4 in y
              //if( mod(xVal, 2) == 0 && mod(yVal, 4) == 0 ){
              
              
                  // Defining The Material Colors
                  vec4 AmbientColor = vec4(0.0, 0.0, 1.0, 1.0);
                  const vec4 DiffuseColor = vec4(1.0, 0.0, 0.0, 1.0);
                  
                  // Scaling The Input Vector To Length 1
                  vec3 normalized_normal = normalize(normal);
                  vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
                  
                  // Calculating The Diffuse Term And Clamping It To [0;1]
                  float DiffuseTerm = clamp(dot(normalized_normal, normalized_vertex_to_light_vector), 0.0, 1.0);  
                  
                  // Calculating The Final Color  
                  gl_FragColor = AmbientColor + DiffuseColor * DiffuseTerm;  
           
              
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
    
    //GUI
    
    ofxPanel gui;
    ofxIntSlider posX;
    ofxIntSlider posY;
    ofxIntSlider posZ;
    ofxFloatSlider radius;
    ofxToggle togl1;
    ofxToggle togl2;
    ofxToggle togl3;
    ofxToggle togl4;
    ofxToggle togl5;
    ofxToggle togl6;
    ofxToggle togl7;
    ofxToggle togl8;
    ofxToggle togl9;
    ofxToggle togl10;
    ofxToggle togl11;
    ofxToggle togl12;
    
    ofxMultiKinectV2 kinect0;
    
    ofImage text1;
    ofImage text2;
    ofImage text3;
    
    ofEasyCam ecam;
    ofVboMesh mesh;
    ofLight light;
    ofLight light2;
    
    vector< vector<ofVec3f> > points;
    vector< vector<ofColor> > colors;
    vector< vector<int> > indexs;
     ofShader colorShader;
    ofShader fragmentShader;
    ofxPostProcessing post;
    ofShader shader;
    ofMaterial material;
    bool doShader;

    
public:
    
    void setup()
    {
        
#ifdef TARGET_OPENGLES
        shader.load("shaders_gles/noise.vert","shaders_gles/noise.frag");
#else
        if(ofIsGLProgrammableRenderer()){
            shader.load("shaders_gl3/noise.vert", "shaders_gl3/noise.frag");
        }else{
            shader.load("shaders/noise.vert", "shaders/noise.frag");
        }
#endif
        
        doShader = true;
        
        
        //Image setup
        
        text1.load("text1.jpg");
      //  shader.setUniformTexture("texture0", text1.getTexture(), 1 );
        
        //GUI
        
        gui.add(posX.setup("posX", 0, 0, 1000));
        gui.add(posY.setup("posY", 0, 0, 1000));
        gui.add(posZ.setup("posZ", 0, 0, 1000));
        gui.add(radius.setup("radius", 40, 0, 100));
        
        
        
        doShader = true;
        
        ofBackground(0);
        
        ofSetCoordHandedness(OF_RIGHT_HANDED);
        
        ofSetVerticalSync(true);
        ofSetFrameRate(30);
        
        kinect0.open(false, true, 0);
        // Note :
        // Default OpenCL device might not be optimal.
        // e.g. Intel HD Graphics will be chosen instead of GeForce.
        // To avoid it, specify OpenCL device index manually like following.
        // kinect1.open(true, true, 0, 2); // GeForce on MacBookPro Retina
        
        kinect0.start();
        
        mesh.setUsage(GL_DYNAMIC_DRAW);
        mesh.setMode(OF_PRIMITIVE_POINTS);
        

        ecam.setDistance(200);
        
        
        post.init(ofGetWidth(), ofGetHeight());
////        post.createPass<VerticalTiltShifPass>();
////        post.createPass<NoiseWarpPass>();
//        post.createPass<ConvolutionPass>();
////        post.createPass<DofAltPass>();
////        post.createPass<FxaaPass>()->setEnabled(false);
//        post.createPass<BloomPass>()->setEnabled(false);
//        post.createPass<DofPass>()->setEnabled(false);
//        post.createPass<KaleidoscopePass>()->setEnabled(false);
//        post.createPass<NoiseWarpPass>()->setEnabled(false);
//        post.createPass<PixelatePass>()->setEnabled(false);
//        post.createPass<EdgePass>()->setEnabled(false);
////        post.createPass<VerticalTiltShifPass>()->setEnabled(false);
////        post.createPass<GodRaysPass>()->setEnabled(false);
////        post.createPass<FFShader>()->setEnabled(false);
//        
//        
////        post.createPass<LUTPass>()->setEnabled(false);
////        post.createPass<LimbDarkeningPass>()->setEnabled(false);
//        post.createPass<RGBShiftPass>()->setEnabled(false);
////        post.createPass<RimHighlightingPass>()->setEnabled(false);
////        post.createPass<SSAOPass>()->setEnabled(false);
////        post.createPass<ToonPass>()->setEnabled(false);
//        post.createPass<ZoomBlurPass>()->setEnabled(false);
////        post.createPass<BleachBypassPass>()->setEnabled(false);
////        post.createPass<FakeSSSPass>()->setEnabled(false);
//        post.createPass<HorizontalTiltShifPass>()->setEnabled(false);
        
        
//        
//        ofColor ambient ;
//        ambient.r=0.19225;
//        ambient.g=0.19225;
//        ambient.b=0.19225;
//        
//        ofColor difuse;
//        difuse.r=0.50754;
//        difuse.g=0.50754;
//        difuse.b=0.50754;
//        
//        ofColor specular;
//        specular.r=0.508273;
//        specular.g=0.508273;
//        specular.b=0.508273;
//        
//        float shininess=0.4;

        
        
        material.setDiffuseColor(ofColor::blue);
        material.setAmbientColor(ofColor::white);
        material.setSpecularColor(ofColor::white);
        material.setShininess(128);
        
        // Setup light
        light.setPosition(1000 , 1000, 2000);
        light2.setPosition(0,0,0);
        light2.setPointLight();
        
        
        glShadeModel(GL_SMOOTH);
    }
    
    void update() {
        kinect0.update();
        

       
        
        int step = 1;
        int total = 0;
        
        int h = kinect0.getDepthPixelsRef().getHeight();
        int w = kinect0.getDepthPixelsRef().getWidth();

        
        if (kinect0.isFrameNew()) {
            mesh.clear();
            points.clear();
            colors.clear();
            indexs.clear();
            
            {
                for (int j = 0; j < h; j+=step)
                {
                    vector<ofVec3f> temppoints;
                    vector<ofColor> tempcolors;
                    
                    points.push_back(temppoints);
                    colors.push_back(tempcolors);
                    
                    for (int i = 0; i < w; i+=step)
                    {
                        float distance = kinect0.getDistanceAt(i, j);
                        if(distance>30 && distance<400)
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
                            tempPoint2 = ofVec3f(i,j,0);
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
                         //   cout << points[m][n] << endl;
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
                
                }
            calcNormals(mesh);
        }
    
    }
    
    void draw()
    {
        
        gui.draw();
        
     //   ofPushMatrix();

    //    text1.getTexture().bind();
        if( doShader ){
            shader.begin();
            //we want to pass in some varrying values to animate our type / color
            shader.setUniform1f("timeValX", ofGetElapsedTimef() * 0.1 );
            shader.setUniform1f("timeValY", -ofGetElapsedTimef() * 0.18 );
            
            //we also pass in the mouse position
            //we have to transform the coords to what the shader is expecting which is 0,0 in the center and y axis flipped.
            shader.setUniform2f("mouse", mouseX - ofGetWidth()/2, ofGetHeight()/2-mouseY );
            
        }
        
        

        // copy enable part of gl state
    //    glPushAttrib(GL_ENABLE_BIT);
        // setup gl state
        glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_CULL_FACE);
        ofEnableDepthTest();
        light.enable();
        light2.enable();
        
        ofPushMatrix();
        

            ecam.begin();
            //post.begin(ecam);
            ofDrawAxis(1000);
            ofVec3f  vector;
            vector = ofVec3f(0, 0 , 0);
            ecam.lookAt(vector);
            //Point cloud mesh
            ofPushMatrix();
                ofRotateZ(-180);
                ofTranslate(-300,-200,200);
          //      material.begin();
                // colorShader.begin();
                mesh.setMode(OF_PRIMITIVE_TRIANGLES);
                glLineWidth(int(1));
                mesh.drawFaces();
      //          material.end();
            ofPopMatrix();
            ecam.end();
          //  post.end();
        
        if( doShader ){
            shader.end();
        }
        
        //    text1.getTexture().unbind();
            // set gl state back to original
      //      glPopAttrib();

        
        ofPopMatrix();
        
        light.disable();
        light2.disable();
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
        ofDrawBitmapStringHighlight("Device Count : " + ofToString(ofxMultiKinectV2::getDeviceCount()), 10, 40);
        // draw help
        ofSetColor(0, 255, 255);
        ofDrawBitmapString("Number keys toggle effects, mouse rotates scene", 10, 20);
        for (unsigned i = 0; i < post.size(); ++i)
        {
            if (post[i]->getEnabled()) ofSetColor(0, 255, 255);
            else ofSetColor(255, 0, 0);
            ostringstream oss;
            oss << i << ": " << post[i]->getName() << (post[i]->getEnabled()?" (on)":" (off)");
            ofDrawBitmapString(oss.str(), 10, 20 * (i + 2));
        }
        
    
    }
    
    void calcNormals(ofMesh &mesh) {
        for( int i=0; i < mesh.getVertices().size(); i++ ) mesh.addNormal(ofPoint(0,0,0));
        
        for( int i=0; i < mesh.getIndices().size(); i+=3 ){
            const int ia = mesh.getIndices()[i];
            const int ib = mesh.getIndices()[i+1];
            const int ic = mesh.getIndices()[i+2];
            try {
                ofVec3f e1 = mesh.getVertices()[ia] - mesh.getVertices()[ib];
                ofVec3f e2 = mesh.getVertices()[ic] - mesh.getVertices()[ib];
                ofVec3f no = e2.cross( e1 );
                
                mesh.getNormals()[ia] += no;
                mesh.getNormals()[ib] += no;
                mesh.getNormals()[ic] += no;
                
            }
            catch (const error_code& e) {
                // Code that executes when an exception of type
                // networkIOException is thrown in the try block
                // ...
                // Log error message in the exception object
            }
            
            
            // depending on your clockwise / winding order, you might want to reverse the e2 / e1 above if your normals are flipped.
           
        }
    }
    
    void keyPressed(int key)
    {
        unsigned idx = key - '0';
        if (idx < post.size()) post[idx]->setEnabled(!post[idx]->getEnabled());
        
        if( key == 's' ){
            doShader = !doShader;
        }
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
