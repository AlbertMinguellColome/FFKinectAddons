//
//  FFShader.h
//  example_pointcloud
//
//  Created by Albert Minguell Colome on 8/9/16.
//
//

#pragma once

#include "RenderPass.h"
#include "ofShader.h"

namespace itg
{
    class FFShader : public RenderPass
    {
    public:
        static const int MAX_KERNEL_SIZE = 25;
        
        typedef shared_ptr<FFShader> Ptr;
        
        FFShader(const ofVec2f& aspect, bool arb, float contrast = 1.0f, float brightness = 1.0f);
        
        void render(ofFbo& readFbo, ofFbo& writeFbo);
        
        float getContrast() { return contrast; }
        void setContrast(float val) { contrast = val; }
        
        float getBrightness() { return brightness; }
        void setBrightness(float val) { brightness = val; }
        
        float getMultiple() { return multiple; }
        void setMultiple(float val) { multiple = val; }
    private:
        
        ofShader shader;
        
        float contrast;
        float brightness;
        float multiple;
    };
}