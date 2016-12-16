//
//  FFShader.cpp
//  example_pointcloud
//
//  Created by Albert Minguell Colome on 8/9/16.
//
//

#include "FFShader.h"
#include "ofMain.h"

namespace itg
{
    FFShader::FFShader(const ofVec2f& aspect, bool arb, float contrast, float brightness) :
    contrast(contrast), brightness(brightness), RenderPass(aspect, arb, "contrast")
    {
        multiple = 1.0f;
        string fragShaderSrc = STRINGIFY(uniform sampler2D tex0;
                                         uniform float contrast;
                                         uniform float brightness;
                                         uniform float multiple;
                                         
                                         
                                         void main(){
                                             vec4 color = texture2D(tex0,gl_TexCoord[0].st);
                                             
                                             float p = 0.3 *color.g + 0.59*color.r + 0.11*color.b;
                                             p = p * brightness;
                                             vec4 color2 = vec4(p,p,p,1.0);
                                             color *= color2;
                                             color *= vec4(multiple,multiple,multiple,1.0);
                                             color = mix( vec4(1.0,1.0,1.0,1.0),color,contrast);
                                             
                                            // gl_FragColor =  vec4(color.r , color.g, color.b, 1.0);
                                             gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);
                                         }
                                         );
        
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderSrc);
        shader.linkProgram();
        
    }
    
    
    void FFShader::render(ofFbo& readFbo, ofFbo& writeFbo)
    {
        writeFbo.begin();
        
        ofClear(0, 0, 0, 255);
        
        shader.begin();
        
        shader.setUniformTexture("tex0", readFbo, 0);
        shader.setUniform1f("contrast", contrast);
        shader.setUniform1f("brightness", brightness);
        shader.setUniform1f("multiple", multiple);
        
        texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        
        shader.end();
        writeFbo.end();
    }
}