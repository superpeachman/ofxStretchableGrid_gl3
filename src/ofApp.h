#pragma once

#include "ofMain.h"
#include "StretchableGrid.h"
#include "ofxGui.h"
#include "ofxNDIVideoGrabber.h"
#include "ofxNDIFinder.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

	void changeNearClip(float &camNear);
	void changeFarClip(float &camFar);
	bool flagCameraClipChanged = false;

	bool switchKeyAction = false;

	ofLight light;

	/*--------------------------------------------------------------


	StretchableGrid


	--------------------------------------------------------------*/
	StretchableGrid* stretchableGrid;
    ofEasyCam cam;
    bool showTex = false;

	/*--------------------------------------------------------------


	ofGui


	--------------------------------------------------------------*/
	ofxPanel gui;
	ofParameter<ofVec3f> lightPosition;
	ofParameter<ofVec3f> lightAmbientColor;
	ofParameter<ofVec3f> lightDiffuseColor;
	ofParameter<ofVec3f> lightSpecularColor;
	ofxFloatSlider camNear;
	ofxFloatSlider camFar;


	/*--------------------*/

	
	//NDI


	/*--------------------*/
	ofxNDIFinder finder_;
	ofxNDIVideoGrabber grabber_;

	int width = 576;
	int height = 432;

	ofShader clothUpdateDepthCollision;

	// ofTexture
	ofPixels ndiDepthPixels;
	ofTexture ndiDepthTexture; // Texture to receive

	ofPixels ndiImagePixels;
	ofTexture ndiImageTexture; // Texture to receive

	ofImage ndiImage; // Image to receive
	ofTexture depthTex; ///< the depth texture
};
