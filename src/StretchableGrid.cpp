#include "StretchableGrid.h"

StretchableGrid::StretchableGrid() {
}
StretchableGrid::~StretchableGrid() {}

//--------------------------------------------------------------
void StretchableGrid::setup(
	ofEasyCam* _mainCam,
	ofParameter<ofVec3f>* _lightPosition,
	ofParameter<ofVec3f>* _lightAmbientColor,
	ofParameter<ofVec3f>* _lightDiffuseColor,
	ofParameter<ofVec3f>* _lightSpecularColor,
	ofTexture* _ndiDepthTexture,
	ofTexture* _ndiImageTexture
){

	/*--------------------------------------------------------------


	 ofGui


	 --------------------------------------------------------------*/
	gui.setup();
	gui.add(g.setup("Gravity", 6.0, 0, 9.8));
	gui.add(w.setup("Wind", 4.0, 0.0, 20.0));
	gui.add(r.setup("Registance", 0.2, 0.0, 2.0));
	gui.add(relaxation.setup("relaxation", 5, 1, 6));
	gui.add(k.setup("Basic Strength", 5000.0, 1.0, 8000.0));
	gui.add(structural_shrink.setup("Structural shrink", 1.0, 0.0, 1.0));
	gui.add(structural_stretch.setup("Structural stretch", 1.0, 0.0, 1.0));
	gui.add(shear_shrink.setup("shear shrink", 1.0, 0.0, 1.0));
	gui.add(shear_stretch.setup("shear shrink", 1.0, 0.0, 1.0));
	gui.add(bending_shrink.setup("Bending shrink", 1.0, 0.0, 1.0));
	gui.add(bending_stretch.setup("Bending stretch", 0.5, 0.0, 1.0));
	gui.add(scale.setup("Scale", 1.0, 0.1, 2.0));

	lightPosition = _lightPosition;
	lightAmbientColor = _lightAmbientColor;
	lightDiffuseColor = _lightDiffuseColor;
	lightSpecularColor = _lightSpecularColor;

	/*--------------------------------------------------------------


	 Camera


	--------------------------------------------------------------*/
	cam = _mainCam;

	/*--------------------------------------------------------------


	 Textures


	--------------------------------------------------------------*/
	ndiDepthTexture = _ndiDepthTexture; // Texture to receive
	ndiImageTexture = _ndiImageTexture; // Texture to receive

	/*--------------------------------------------------------------


	 Particles


	 --------------------------------------------------------------*/
	 // Particles
	//    particles.setMode(OF_PRIMITIVE_POINTS);
	//    particleNum = 1000000;
	particleNum = 10000;
	//    particleNum = 100000;
	//    particleNum = 50000;
	texRes = ceil(sqrt(particleNum));
	std::cout << "texRes: " << texRes << endl;

	particles.setMode(OF_PRIMITIVE_TRIANGLES);
	particles.enableIndices();

	for (int x = 0; x < texRes; x++) {
		for (int y = 0; y < texRes; y++) {
			particles.addVertex(ofVec3f(
				x,
				y,
				0
			));
			particles.addTexCoord(ofVec2f(x, y));
			particles.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
		}
	}

	for (int y = 0; y < texRes - 1; y++) {
		for (int x = 0; x < texRes - 1; x++) {
			//            particles.addIndex(x+y*texRes);               // 0
			//            particles.addIndex((x+1)+y*texRes);           // 1
			//            particles.addIndex(x+(y+1)*texRes);           // 10
			//
			//            particles.addIndex((x+1)+y*texRes);           // 1
			//            particles.addIndex((x+1)+(y+1)*texRes);       // 11
			//            particles.addIndex(x+(y+1)*texRes);           // 10

			int i1 = x + y * texRes;//0
			int i2 = (x + 1) + y * texRes;//1
			int i3 = x + (y + 1)*texRes;//10
			int i4 = (x + 1) + (y + 1)*texRes;//11

			particles.addTriangle(i1, i2, i3);
			particles.addTriangle(i2, i4, i3);
		}
	}

	setNormals(particles);

	/*--------------------------------------------------------------


	 PingPong


	 --------------------------------------------------------------*/
	 // Cloth class
	pingPong.allocate(texRes, texRes, GL_RGBA32F, 1);

	// Set cloth init position
	float * initPos = new float[texRes * texRes * 4];
	for (int x = 0; x < texRes; x++) {
		for (int y = 0; y < texRes; y++) {
			int i = texRes * y + x;
			initPos[i * 4 + 0] = x / (texRes - 1.0) * 2.0 - 1.0; // 0 to 99 = -1 to 1
			initPos[i * 4 + 1] = y / (texRes - 1.0) - 1.0; // 0 to 99 = -1 to 0
			initPos[i * 4 + 2] = y / (texRes - 1.0); // 0 to 99 = 0 to 1
			initPos[i * 4 + 3] = y / (texRes - 1.0) < 0.1 / (texRes - 1.0) ? 0.0 : 1.0; // 0 = 0, others = 1
		}
	}

	pingPong.src->getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);
	pingPong.nowFbo.getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);
	pingPong.oldFbo.getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);

	delete[] initPos;

	/*--------------------------------------------------------------


	 Shader


	 --------------------------------------------------------------*/
	clothUpdateIntegration.load("shaders/clothUpdateIntegration");
	clothUpdateConstraint.load("shaders/clothUpdateConstraint");
	//clothUpdateCollision.load("shaders/clothUpdateCollision");
	render.load("shaders/render");

	clothUpdateDepthCollision.load("shaders/clothUpdateDepthCollision");

	/*--------------------------------------------------------------


	 Image


	 --------------------------------------------------------------*/
	image01.load("images/lion01.jpg");
	image02.load("images/lion02.jpg");
	image03.load("images/lion03.jpg");

	ofFbo::Settings fboSettings;
	fboSettings.width = texRes;
	fboSettings.height = texRes;
	fboSettings.numColorbuffers = 1;
	fboSettings.useDepth = false;
	fboSettings.internalformat = GL_RGBA32F;
	fboSettings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
	fboSettings.wrapModeVertical = GL_CLAMP_TO_EDGE;
	fboSettings.minFilter = GL_NEAREST;
	fboSettings.maxFilter = GL_NEAREST;
	imageTextureFbo.allocate(fboSettings);

	//    for (int x = 0; x < texRes; x++){
	//        for (int y = 0; y < texRes; y++){
	//            imageTextureFbo.addVertex(ofPoint(
	//                                        x - texRes/2,
	//                                        y - texRes/2,
	//                                        0));
	//            imageTextureFbo.addTexCoord(ofVec2f(x, y));
	//            imageTextureFbo.addColor(ofFloatColor(1.0, 1.0, 1.0, 1.0));
	//        }
	//    }

	imageTextureFbo.begin();
	ofClear(0, 0, 0, 0);
	image01.draw(0, 0, texRes, texRes);
	imageTextureFbo.end();
}

//--------------------------------------------------------------
void StretchableGrid::constraint(pingPongBufferSpring& _pingPong, float offset_x, float offset_y, float shrink, float stretch, float coefficient, float _timeStep) {

	_pingPong.dst->begin();
	_pingPong.dst->activateAllDrawBuffers();
	ofClear(0);

	clothUpdateConstraint.begin();
	clothUpdateConstraint.setUniform1f("u_div", texRes);
	//    clothUpdateConstraint.setUniform2f("u_neightbor_offset", offset_x / (texRes + 1), offset_y / (texRes + 1));
	clothUpdateConstraint.setUniform2f("u_neightbor_offset", offset_x / (texRes - 1.0), offset_y / (texRes - 1.0));
	clothUpdateConstraint.setUniform1f("u_neightbor_coefficient", coefficient);
	//    clothUpdateConstraint.setUniform1f("u_rest", scale * 2.0 / texRes * sqrt(offset_x * offset_x + offset_y * offset_y));
	//    clothUpdateConstraint.setUniform1f("u_rest", scale * 2.0 / (texRes + 1) * sqrt(offset_x * offset_x + offset_y * offset_y));
	clothUpdateConstraint.setUniform1f("u_rest", scale * 2.0 / (texRes - 1.0) * sqrt(offset_x * offset_x + offset_y * offset_y));
	clothUpdateConstraint.setUniform1f("u_k", k);
	clothUpdateConstraint.setUniform1f("u_shrink", shrink);
	clothUpdateConstraint.setUniform1f("u_stretch", stretch);
	/* —Í‚©‚ç•ÏˆÊ‚É•ÏŠ·‚·‚éŒvŽZ */
	clothUpdateConstraint.setUniform1f("u_f2dx", _timeStep * _timeStep * 0.5);
	clothUpdateConstraint.setUniformTexture("u_texture0", _pingPong.src->getTexture(0), 0);

	_pingPong.src->draw(0, 0);

	clothUpdateConstraint.end();
	_pingPong.dst->end();

	_pingPong.swap();

	//    ofLog(OF_LOG_NOTICE, "_pingPong address is:" + ofToString(&_pingPong));

	//    ofLog(OF_LOG_NOTICE, "u_neightbor_offset x:"
	//          + ofToString(offset_x / (texRes - 1))
	//          + " y:"
	//          + ofToString(offset_y / (texRes - 1))
	//          );
}

//--------------------------------------------------------------
void StretchableGrid::updateCloth(float _timeStep, float _accTime) {
	//    testCount++;
	//    ofLog(OF_LOG_NOTICE, "testCount is:" + ofToString(testCount));
	//    ofLog(OF_LOG_NOTICE, "_accTime is:" + ofToString(_accTime));

	// Phase 1 - Create and Apply force
	ofVec3f f = *new ofVec3f();
	f.x = 0.0;
	f.y += g;
	f.z += w * (sin(_accTime) * 0.5 + 0.5);

	f *= _timeStep * _timeStep * 0.5;

	float r_ = 1.0 - r * _timeStep;

	//    ofLog(OF_LOG_NOTICE, "g is:" + ofToString(g));
	//    ofLog(OF_LOG_NOTICE, "f is:" + ofToString(f.x) + ", " + ofToString(f.y) + ", " + ofToString(f.z));
	//    ofLog(OF_LOG_NOTICE, "r_ is:" + ofToString(r_));

	pingPong.dst->begin();
	pingPong.dst->activateAllDrawBuffers();
	ofClear(0);

	clothUpdateIntegration.begin();

	clothUpdateIntegration.setUniformTexture("u_tex_now", pingPong.nowFbo.getTexture(0), 0);
	clothUpdateIntegration.setUniformTexture("u_tex_old", pingPong.oldFbo.getTexture(0), 1);
	clothUpdateIntegration.setUniform3f("u_dx", f.x, f.y, f.z);
	clothUpdateIntegration.setUniform1f("u_r", r_);

	pingPong.src->draw(0, 0);

	clothUpdateIntegration.end();
	pingPong.dst->end();

	pingPong.swap();

	// Phase 2 - constraint phase
	for (int ite = 0; ite < relaxation; ite++) {
		constraint(pingPong, 1.0, 0.0, structural_shrink, structural_stretch, 2.0, _timeStep);
		constraint(pingPong, -1.0, 0.0, structural_shrink, structural_stretch, 2.0, _timeStep);
		constraint(pingPong, 0.0, 1.0, structural_shrink, structural_stretch, 2.0, _timeStep);
		constraint(pingPong, 0.0, -1.0, structural_shrink, structural_stretch, 2.0, _timeStep);
		constraint(pingPong, 1.0, 1.0, shear_shrink, shear_stretch, 2.0, _timeStep);
		constraint(pingPong, -1.0, -1.0, shear_shrink, shear_stretch, 2.0, _timeStep);
		constraint(pingPong, 1.0, -1.0, shear_shrink, shear_stretch, 2.0, _timeStep);
		constraint(pingPong, -1.0, 1.0, shear_shrink, shear_stretch, 2.0, _timeStep);
		constraint(pingPong, 2.0, 0.0, bending_shrink, bending_stretch, 4.0, _timeStep);
		constraint(pingPong, -2.0, 0.0, bending_shrink, bending_stretch, 4.0, _timeStep);
		constraint(pingPong, 0.0, 2.0, bending_shrink, bending_stretch, 4.0, _timeStep);
		constraint(pingPong, 0.0, -2.0, bending_shrink, bending_stretch, 4.0, _timeStep);
	}

	// Phase 3 - collision phase
	//   pingPong.dst->begin();
	//   pingPong.dst->activateAllDrawBuffers();
	//   ofClear(0);
	//   
	//   clothUpdateCollision.begin();
	//   
	//   const ofVec3f sphere_pos = *new ofVec3f(0.0, 0.5, 0.0);
	//   const float sphere_radius = 0.5;
	//   
	//   clothUpdateCollision.setUniformTexture("u_texture0", pingPong.src->getTexture(0), 0);
	//   clothUpdateCollision.setUniform3f("u_sphere_pos", sphere_pos.x, sphere_pos.y, sphere_pos.z);
	//   clothUpdateCollision.setUniform1f("u_sphere_radius", sphere_radius);
	//   clothUpdateCollision.setUniform1f("u_div", texRes);

	//   pingPong.src->draw(0, 0);
	//   
	//   clothUpdateCollision.end();
	//   pingPong.dst->end();
	//   
	//pingPong.swap();

	// Phase 4 - collision depth phase
	pingPong.dst->begin();
	pingPong.dst->activateAllDrawBuffers();
	ofClear(0);

	clothUpdateDepthCollision.begin();

	const ofVec3f sphere_pos = *new ofVec3f(0.0, 0.5, 0.0);
	const float sphere_radius = 0.5;

	clothUpdateDepthCollision.setUniformTexture("u_texture0", pingPong.src->getTexture(0), 0);
	clothUpdateDepthCollision.setUniformTexture("u_depth_texture", *ndiDepthTexture, 1);

	clothUpdateDepthCollision.setUniform3f("u_sphere_pos", sphere_pos.x, sphere_pos.y, sphere_pos.z);
	clothUpdateDepthCollision.setUniform1f("u_sphere_radius", sphere_radius);
	clothUpdateDepthCollision.setUniform1f("u_div", texRes);

	pingPong.src->draw(0, 0);

	clothUpdateDepthCollision.end();
	pingPong.dst->end();

	pingPong.swap();


	// Phase 5 - update nowFbo
	pingPong.updateNowOld();
	//    ofLog(OF_LOG_NOTICE, "pingPong address is:" + ofToString(&pingPong));

}

//--------------------------------------------------------------
void StretchableGrid::update() {
	if (fabricFixedPointChangeFlag) {
		setFixedPointType(fabricFixedPointType);
		fabricFixedPointChangeFlag = false;
	}

	float time = ofGetElapsedTimef();

	float ms_delta = time - preTime + surTime;
	ms_delta = MIN(ms_delta, 100);
	while (ms_delta >= timeStep)
	{
		updateCloth(timeStep / 1000.0, accTime / 1000.0);
		accTime += timeStep;
		ms_delta -= timeStep;

		//std::cout << "timeStep: " << timeStep << endl;
		//std::cout << "accTime: " << accTime << endl;
		//std::cout << "ms_delta: " << ms_delta << endl;
	}
	surTime = ms_delta;
}

//--------------------------------------------------------------
void StretchableGrid::draw() {
	render.begin();

	ofMatrix4x4 matrix_for_normal;
	matrix_for_normal.makeIdentityMatrix();

	render.setUniform1f("u_div", texRes);
	render.setUniform1f("u_neightbor_offset", 1.0 / (texRes - 1));
	render.setUniform1f("u_face", 1.0);
	render.setUniformMatrix4f("u_matrix_for_normal", matrix_for_normal);
	render.setUniformTexture("u_posAndAgeTex", pingPong.nowFbo.getTexture(0), 0);
	//render.setUniformTexture("u_imageTex", imageTextureFbo.getTexture(0), 1);
	render.setUniformTexture("u_imageTex", *ndiImageTexture, 1);
	//render.setUniformTexture("u_imageTex", ndiDepthTexture, 1);
	render.setUniform3f("u_directional_light_dir", ofVec3f(
		lightPosition->get().x,
		lightPosition->get().y,
		lightPosition->get().z
	));
	render.setUniform3f("u_ambient_light_color",
		lightAmbientColor->get().x,
		lightAmbientColor->get().y,
		lightAmbientColor->get().z
	);
	render.setUniform3f("u_directional_light_color",
		lightDiffuseColor->get().x,
		lightDiffuseColor->get().y,
		lightDiffuseColor->get().z
	);
	render.setUniform3f("u_specular_light_color",
		lightSpecularColor->get().x,
		lightSpecularColor->get().y,
		lightSpecularColor->get().z
	);

	render.setUniform3f("u_camera_pos", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);

	particles.draw();

	//    particles.drawWireframe();
	//    particles.drawFaces();
	//    particles.draw(OF_MESH_FILL);
	//    particles.enableNormals();
	//    particles.drawFaces();
	render.end();

}

//--------------------------------------------------------------
void StretchableGrid::drawDebug() {
	gui.draw();

	//if (showTex) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);

		ofDrawBitmapStringHighlight("nowFbo",
			5 - 10,
			ofGetHeight() - 5 - texRes * 2 - 10
		);
		pingPong.nowFbo.getTexture(0).draw(
			5,
			ofGetHeight() - 5 - texRes * 2,
			texRes * 2,
			texRes * 2
		);

		ofDrawBitmapStringHighlight("oldFbo",
			texRes * 2 - 10,
			ofGetHeight() - 5 - texRes * 2 - 10
		);
		pingPong.oldFbo.getTexture(0).draw(
			texRes * 2,
			ofGetHeight() - 5 - texRes * 2,
			texRes * 2,
			texRes * 2
		);

		ofDrawBitmapStringHighlight("ndiImageTexture",
			texRes * 4,
			ofGetHeight() - 5 - ndiImageTexture->getHeight() / 2 - 10
		);
		ndiImageTexture->draw(
			texRes * 4,
			ofGetHeight() - 5 - ndiImageTexture->getHeight() / 2,
			ndiImageTexture->getWidth() / 2,
			ndiImageTexture->getHeight() / 2
		);

		ofDrawBitmapStringHighlight("ndiDepthTexture",
			texRes * 4 + ndiImageTexture->getWidth() / 2,
			ofGetHeight() - 5 - ndiDepthTexture->getHeight() / 2 - 10
		);
		ndiDepthTexture->draw(
			texRes * 4 + ndiImageTexture->getWidth() / 2,
			ofGetHeight() - 5 - ndiDepthTexture->getHeight() / 2,
			ndiDepthTexture->getWidth() / 2,
			ndiDepthTexture->getHeight() / 2
		);

		ofPopStyle();
	//}
}

//--------------------------------------------------------------
void StretchableGrid::setFixedPointType(int _type) {
    //ofLog(OF_LOG_NOTICE, "setFabricFixedPos is going");

	float * initPos = new float[texRes * texRes * 4];

	if (_type == 0) {
		std::cout << "_type: " << _type << " fix TOP only" << endl;

		// Cloth, fixed only top position
		for (int x = 0; x < texRes; x++) {
			for (int y = 0; y < texRes; y++) {
				int i = texRes * y + x;
				initPos[i * 4 + 0] = x / (texRes - 1.0) * 2.0 - 1.0; // 0 to 99 = -1 to 1
				initPos[i * 4 + 1] = y / (texRes - 1.0) - 1.0; // 0 to 99 = -1 to 0
				initPos[i * 4 + 2] = y / (texRes - 1.0); // 0 to 99 = 0 to 1
				initPos[i * 4 + 3] = y / (texRes - 1.0) < 0.5 / (texRes - 1.0) ? 0.0 : 1.0; // 0 = 0, others = 1
			}
		}
	}
	else if (_type == 1) {
		std::cout << "_type: " << _type << " fix TOP and BOTTOM" << endl;

		// Fix as RECT
		for (int y = 0; y < texRes; y++) {
			for (int x = 0; x < texRes; x++) {
				int i = texRes * y + x;
				initPos[i * 4 + 0] = x / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 1] = y / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 2] = 0.0;

				float tempY = (float)y / ((float)texRes - 1.0);

				if (
					tempY < 0.5 / ((float)texRes - 1.0) || // Top
					tempY > ((float)texRes - 1.0 - 0.1) / ((float)texRes - 1.0) // Bottom
				){
					//std::cout << "tempW: " << tempW << " x: " << x << " y: " << y << endl;

					initPos[i * 4 + 3] = 0.0;
				}
				else {
					initPos[i * 4 + 3] = 1.0;
				}
			}
		}

	}
	else if (_type == 2) {
		std::cout << "_type: " << _type << " fix LEFT and RIGHT" << endl;

		// Fix as RECT
		for (int y = 0; y < texRes; y++) {
			for (int x = 0; x < texRes; x++) {
				int i = texRes * y + x;
				initPos[i * 4 + 0] = x / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 1] = y / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 2] = 0.0;

				float tempX = (float)x / ((float)texRes - 1.0);

				if (
					tempX < 0.5 / ((float)texRes - 1.0) || // Left
					tempX >((float)texRes - 1.0 - 2.0) / ((float)texRes - 1.0) // Right
					) {
					//std::cout << "tempW: " << tempX << " x: " << x << " y: " << y << endl;

					initPos[i * 4 + 3] = 0.0;
				}
				else {
					initPos[i * 4 + 3] = 1.0;
				}
			}
		}

	}
	else if (_type == 3) {
		std::cout << "_type: " << _type << " fix TOP, BOTTOM, LEFT and RIGHT" << endl;

		// Fix as RECT
		for (int y = 0; y < texRes; y++) {
			for (int x = 0; x < texRes; x++) {
				int i = texRes * y + x;
				initPos[i * 4 + 0] = x / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 1] = y / (texRes - 1.0) * 2.0 - 1.0;
				initPos[i * 4 + 2] = 0.0;

				float tempX = (float)x / ((float)texRes - 1.0);
				float tempY = (float)y / ((float)texRes - 1.0);

				if (
					tempX < 0.5 / ((float)texRes - 1.0) || // Left
					tempX >((float)texRes - 1.0 - 2.0) / ((float)texRes - 1.0) || // Right
					tempY < 0.5 / ((float)texRes - 1.0) || // Top
					tempY >((float)texRes - 1.0 - 0.1) / ((float)texRes - 1.0) // Bottom
					) {
					//std::cout << "tempX: " << tempX << " tempY: " << tempY << " x: " << x << " y: " << y << endl;

					initPos[i * 4 + 3] = 0.0;
				}
				else {
					initPos[i * 4 + 3] = 1.0;
				}
			}
		}

	}

	pingPong.src->getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);
	pingPong.nowFbo.getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);
	pingPong.oldFbo.getTexture(0).loadData(initPos, texRes, texRes, GL_RGBA);

	delete[] initPos;
}

//--------------------------------------------------------------
void StretchableGrid::setNormals(ofMesh &mesh) {

	//The number of the vertices
	int nV = mesh.getNumVertices();

	//The number of the triangles
	int nT = mesh.getNumIndices() / 3;

	//vector<ofPoint> norm( nV ); //Array for the normals
	std::vector<glm::vec3> norm(nV); //Array for the normals

	//Scan all the triangles. For each triangle add its
	//normal to norm's vectors of triangle's vertices
	for (int t = 0; t < nT; t++) {

		//Get indices of the triangle t
		int i1 = mesh.getIndex(3 * t);
		int i2 = mesh.getIndex(3 * t + 1);
		int i3 = mesh.getIndex(3 * t + 2);

		////Get vertices of the triangle
		//const ofPoint &v1 = mesh.getVertex(i1);
		//const ofPoint &v2 = mesh.getVertex(i2);
		//const ofPoint &v3 = mesh.getVertex(i3);

		////Compute the triangle's normal
		//ofPoint dir = ((v2 - v1).crossed(v3 - v1)).normalized();

		//Get vertices of the triangle
		const glm::vec3 &v1 = mesh.getVertex(i1);
		const glm::vec3 &v2 = mesh.getVertex(i2);
		const glm::vec3 &v3 = mesh.getVertex(i3);

		//Compute the triangle's normal
		glm::vec3 dir = glm::normalize(glm::cross(v2 - v1, v3 - v1));

		//Accumulate it to norm array for i1, i2, i3
		norm[i1] += dir;
		norm[i2] += dir;
		norm[i3] += dir;
	}

	//Normalize the normal's length
	for (int i = 0; i < nV; i++) {
		//norm[i].normalize();
		norm[i] = glm::normalize(norm[i]);
	}

	//Set the normals to mesh
	mesh.clearNormals();
	mesh.addNormals(norm);
}