#pragma once

/*
 
 BASED off of the original ofxKinect example !
 github.com/ofTheo/ofxKinect
 
 I just added ofxUI and made some stuff a little easier to digest
 
 */

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxSimpleMask.h"
#include "ofxUI.h"

/*
#include "MSAFluid.h"
//#include "MSATimer.h"
#include "ParticleSystem.h"

#define USE_GUI		

#ifdef USE_GUI 
#include "ofxSimpleGuiToo.h"
#endif
*/

// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

class testApp : public ofBaseApp {
public:
	
	void setup();	
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
	
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    
    void setup_ofxUI() ;
	//ofColor color1, color2, color3;
	
	ofxKinect kinect;
	
#ifdef USE_TWO_KINECTS
	ofxKinect kinect2;
#endif
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder;
	
	bool bThreshWithOpenCV;
    
	int nearThreshold;
	int farThreshold;
	
	int angle;
	
    //added for ofxUI
    ofxUICanvas *gui1;
    float guiWidth ; 
	void guiEvent(ofxUIEventArgs &e);
    
    float minBlobSize , maxBlobSize ;
    
    bool bKinectOpen ;
    
    ofShader shader;
    ofxSimpleMask mask ;
    
    ofFbo shaderFbo ;
    ofFbo maskFbo ;
	int timeSpeed;
	float getTime(int);
	
	//vector<ofVec2f*> vertices;
	/*
	void fadeToColor(float r, float g, float b, float speed);
	void addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce);
	
	float                   colorMult;
    float                   velocityMult;	
	int                     fluidCellsX;
	bool                    resizeFluid;
	bool                    drawFluid;
	bool                    drawParticles;
	
	msa::fluid::Solver      fluidSolver;
	msa::fluid::DrawerGl	fluidDrawer;
	
	ParticleSystem          particleSystem;
	
	ofVec2f                 pMouse;

	 */
    
};
