#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "pingPongBufferSpring.h"

class StretchableGrid {
public:
	StretchableGrid();
	~StretchableGrid();

	void setup(
		ofEasyCam* _mainCam,
		ofParameter<ofVec3f>* _lightPosition,
		ofParameter<ofVec3f>* _lightAmbientColor,
		ofParameter<ofVec3f>* _lightDiffuseColor,
		ofParameter<ofVec3f>* _lightSpecularColor,
		ofTexture* _ndiDepthTexture,
		ofTexture* _ndiImageTexture
	);
	void update();
	void draw();
	void drawDebug();
	void setFixedPointType(int _type);


private:
	void setNormals(ofMesh &mesh);
	void constraint(pingPongBufferSpring& _pingPong, float offset_x, float offset_y, float shrink, float stretch, float coefficient, float _timeStep);
	void updateCloth(float _timeStep, float _accTime);

	ofEasyCam* cam;
	ofTexture* ndiDepthTexture; // Texture to receive
	ofTexture* ndiImageTexture; // Texture to receive

	// Fix point type
	bool fabricFixedPointChangeFlag = false;
	int fabricFixedPointType = 0;

	ofMesh particles;
	int particleNum, texRes;

	pingPongBufferSpring pingPong;

	ofShader render;
	ofShader clothUpdateIntegration, clothUpdateConstraint, clothUpdateCollision;
	ofShader clothUpdateDepthCollision;

	float timeStep = 16.0;
	float preTime = 0;
	float accTime = 0;
	float surTime = 0;

	int testCount = 0;
	bool showTex = false;

	int imageNum = 0;
	ofImage image01;
	ofImage image02;
	ofImage image03;
	ofFbo imageTextureFbo;

	/*--------------------------------------------------------------


	ofGui


	--------------------------------------------------------------*/
	ofxPanel gui;
	ofxFloatSlider g;
	ofxFloatSlider w;
	ofxFloatSlider r;
	ofxIntSlider relaxation;
	ofxFloatSlider k;
	ofxFloatSlider structural_shrink;
	ofxFloatSlider structural_stretch;
	ofxFloatSlider shear_shrink;
	ofxFloatSlider shear_stretch;
	ofxFloatSlider bending_shrink;
	ofxFloatSlider bending_stretch;
	ofxFloatSlider scale;

	ofParameter<ofVec3f>* lightPosition;
	ofParameter<ofVec3f>* lightAmbientColor;
	ofParameter<ofVec3f>* lightDiffuseColor;
	ofParameter<ofVec3f>* lightSpecularColor;

};