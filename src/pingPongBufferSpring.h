#ifndef GPUParticles_pingPongBufferSpring_h
#define GPUParticles_pingPongBufferSpring_h

struct pingPongBufferSpring {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA, int _numColorBuffers = 1){
        // Allocate
        ofFbo::Settings fboSettings;
        fboSettings.width  = _width;
        fboSettings.height = _height;
        fboSettings.numColorbuffers = _numColorBuffers;
        fboSettings.useDepth = false;
        fboSettings.internalformat = _internalformat;    // Gotta store the data as floats, they won't be clamped to 0..1
        fboSettings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
        fboSettings.wrapModeVertical = GL_CLAMP_TO_EDGE;
        fboSettings.minFilter = GL_NEAREST; // No interpolation, that would mess up data reads later!
        fboSettings.maxFilter = GL_NEAREST;
        for(int i = 0; i < 2; i++){
            FBOs[i].allocate(fboSettings);
        }
        
        oldFbo.allocate(fboSettings);
        nowFbo.allocate(fboSettings);
        
        // Clean
        clear();
        
        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;

        updateNewOldFbo.load("shaders/updateNowOldFbo");

        updateNowOld();
    }
    
    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }
    
    void updateNowOld(){        
        oldFbo.begin();
        oldFbo.activateAllDrawBuffers();
        ofClear(0);
        
        updateNewOldFbo.begin();
        updateNewOldFbo.setUniformTexture("texture0", nowFbo.getTexture(0), 0);

        nowFbo.draw(0, 0);
        
        updateNewOldFbo.end();
        oldFbo.end();
        
        
        nowFbo.begin();
        nowFbo.activateAllDrawBuffers();
        ofClear(0);
        
        updateNewOldFbo.begin();
        updateNewOldFbo.setUniformTexture("texture0", src->getTexture(0), 0);
        
        src->draw(0, 0);
        
        updateNewOldFbo.end();
        nowFbo.end();

    }
    
    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,255);
            FBOs[i].end();
        }
        
        nowFbo.begin();
        ofClear(0,255);
        nowFbo.end();
        
        oldFbo.begin();
        ofClear(0,255);
        oldFbo.end();
    }
    
    ofFbo& operator[]( int n ){ return FBOs[n];}
    
    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong
    
//    ofFbo   *now;       // Source       ->  Ping
//    ofFbo   *old;       // Destination  ->  Pong
    ofFbo   nowFbo;       // Source       ->  Ping
    ofFbo   oldFbo;       // Destination  ->  Pong
private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO´s
    int     flag;       // Integer for making a quick swap
    ofShader updateNewOldFbo;
};

#endif
