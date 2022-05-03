#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofDisableAlphaBlending();// Necessary to pingpong
	//ofSetVerticalSync(false);
	//    ofEnableSmoothing();
    
	ofSetFrameRate(0);
    ofBackgroundGradient( ofColor( 255 ), ofColor( 128 ) );


    /*--------------------------------------------------------------
     
     
     ofGui
     
     
     --------------------------------------------------------------*/
	float guiWidth = int(ofGetWidth() / 8);
	gui.setDefaultWidth(guiWidth);
	gui.setup();
	gui.setPosition(0, 0);
	gui.add(lightPosition.set("Light Pos",
		ofVec3f(-100.0, 50.0, 50.0),
		ofVec3f(-500, -500, -500),
		ofVec3f(500, 500, 500))
	);
	gui.add(lightAmbientColor.set("Light Ambi Col",
		ofVec3f(0.2, 0.2, 0.2),
		ofVec3f(0.0, 0.0, 0.0),
		ofVec3f(1.0, 1.0, 1.0))
	);
	gui.add(lightDiffuseColor.set("Light Diff Col",
		ofVec3f(0.8, 0.8, 0.8),
		ofVec3f(0.0, 0.0, 0.0),
		ofVec3f(1.0, 1.0, 1.0))
	);
	gui.add(lightSpecularColor.set("Light Spec Col",
		ofVec3f(0.5, 0.5, 0.5),
		ofVec3f(0.0, 0.0, 0.0),
		ofVec3f(1.0, 1.0, 1.0))
	);

	gui.add(camNear.setup("Camera Near Clip", 100.0, 1.0, 100.0));
	gui.add(camFar.setup("Camera Far Clip", 3000.0, 100.0, 3000.0));

	// ----------------------------- debug
	//// projection
	//std::cout << "Fov : " << cam.getFov() << endl;
	//std::cout << "AspectRatio : " << cam.getAspectRatio() << endl;
	//std::cout << "NearClip : " << cam.getNearClip() << endl;
	//std::cout << "FarClip : " << cam.getFarClip() << endl;
	//// view
	//std::cout << "Position: " << cam.getPosition() << endl;
	//std::cout << "LookAt : " << cam.getLookAtDir() << endl;
	//std::cout << "Up : " << cam.getUpDir() << endl;

	camNear.addListener(this, &ofApp::changeNearClip);
	camFar.addListener(this, &ofApp::changeFarClip);

    
    /*--------------------------------------------------------------
     
     
     Camera
     
     
     --------------------------------------------------------------*/
	// I dont know why but if setupPerspective comes first, Depthmap works wrong!!
	//    cam.setupPerspective();

	cam.setVFlip(!cam.isVFlipped());
	cam.setNearClip(camNear);
	cam.setFarClip(camFar);
	//    cam.setForceAspectRatio(1.0);

	//This only operates with perspective cameras, and will have no effect with cameras in orthographic mode.
	cam.setFov(60);

	//cam.setupPerspective();
	//cam.setPosition(ofGetWidth()*.5f, ofGetHeight()*.5f, 500);
	//cam.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	/*--------------------------------------------------------------


	 NDI


	 --------------------------------------------------------------*/
	grabber_.setUseTexture(true);
	grabber_.setup();
	grabber_.initGrabber(width, height);
	ndiDepthPixels.allocate(width, height, 1);
	ndiImagePixels.allocate(width, height, 4);
	finder_.watchSources();

	/*--------------------------------------------------------------


	 stretchableGrid


	 --------------------------------------------------------------*/
	stretchableGrid = new StretchableGrid();
	stretchableGrid->setup(
		&cam,
		&lightPosition,
		&lightAmbientColor,
		&lightDiffuseColor,
		&lightSpecularColor,
		&ndiDepthTexture,
		&ndiImageTexture
	);
}

//--------------------------------------------------------------
void ofApp::changeNearClip(float &camNear) {
	flagCameraClipChanged = !flagCameraClipChanged;
}

//--------------------------------------------------------------
void ofApp::changeFarClip(float &camFar) {
	flagCameraClipChanged = !flagCameraClipChanged;
}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle(ofToString(ofGetFrameRate()));

	/*--------------------------------------------------------------


	 NDI


	 --------------------------------------------------------------*/
	grabber_.update();

	if (grabber_.isFrameNew()) {
		//std::cout << "Width : " << grabber_.getWidth() << endl;
		//std::cout << "Height : " << grabber_.getHeight() << endl;

		ndiDepthPixels = grabber_.getPixels().getChannel(3);
		ndiDepthTexture.loadData(ndiDepthPixels);

		ndiImagePixels = grabber_.getPixels();
		ndiImageTexture.loadData(ndiImagePixels);

		//uint8_t ui1, ui2, ui3, ui4;
		//uint32_t ui32 = (uint32_t(ui1) << 24) | (uint32_t(ui2) << 16) | (uint32_t(ui3) << 8) | uint32_t(ui4);
		//// use ui32 as needed...
	}

	/*--------------------------------------------------------------


	 Camera


	 --------------------------------------------------------------*/
	if (flagCameraClipChanged) {
		// Camera Reset
		cam.setNearClip(camNear);
		cam.setFarClip(camFar);
		//        cam.reset();

		// ----------------------------- debug
		// projection
		std::cout << "------------------" << endl;
		std::cout << "Fov : " << cam.getFov() << endl;
		std::cout << "AspectRatio : " << cam.getAspectRatio() << endl;
		std::cout << "NearClip : " << cam.getNearClip() << endl;
		std::cout << "FarClip : " << cam.getFarClip() << endl;
		//    // view
		std::cout << "Position: " << cam.getPosition() << endl;
		std::cout << "LookAt : " << cam.getLookAtDir() << endl;
		std::cout << "Up : " << cam.getUpDir() << endl;

		flagCameraClipChanged = false;
	}

	light.setPosition(-lightPosition.get());

	/*--------------------------------------------------------------


	 StretchableGrid


	 --------------------------------------------------------------*/
	stretchableGrid->update();

}

//--------------------------------------------------------------
void ofApp::draw(){
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ofEnableDepthTest();
	ofPushStyle();

	cam.begin();

	light.enable();
	light.draw();

	stretchableGrid->draw();

	light.disable();

	cam.end();

	ofPopStyle();
	ofDisableDepthTest();


	/*--------------------------------------------------------------


	 debug


	 --------------------------------------------------------------*/
	if (showTex) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		ofSetColor(255);

		gui.draw();
		stretchableGrid->drawDebug();

		if (switchKeyAction == false) {
			using namespace std;
			auto sources = finder_.getSources();
			auto names = accumulate(begin(sources), end(sources), vector<string>(), [](vector<string> result, const ofxNDI::Source &src) {
				result.push_back(ofToString(result.size() + 1, 2, '0') + ". " + src.p_ndi_name + "(" + src.p_url_address + ")");
				return result;
				});
			ofDrawBitmapString("press 1-9 to select available source", 10, 10);
			ofDrawBitmapString(ofJoinString(names, "\n"), 10, 20);
		}

		ofPopStyle();
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	if (key == OF_KEY_SHIFT) {
		switchKeyAction = !switchKeyAction;
	}

	if (switchKeyAction) {
		if (key == '0') {
			stretchableGrid->setFixedPointType(0);
		}
		if (key == '1') {
			stretchableGrid->setFixedPointType(1);
		}
		if (key == '2') {
			stretchableGrid->setFixedPointType(2);
		}
		if (key == '3') {
			stretchableGrid->setFixedPointType(3);
		}
	}
	else {
		if (key >= '1' && key <= '9') {
			int index = key - '1';
			auto sources = finder_.getSources();
			if (sources.size() > index) {
				grabber_.setup(sources[index]);
			}
		}
	}



	/*--------------------*/
	if (key == 'h') {
	}
	else if (key == 'r') {
	}
	else if (key == OF_KEY_UP) {
	}
	else if (key == OF_KEY_DOWN) {
	}



    if(key==' '){
		showTex = !showTex;
	}
    
	if(key=='c'){

        
    }
    
    if(key=='k'){

    }
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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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