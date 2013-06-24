#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableSmoothing();
	ofSetFrameRate(60);
	ofBackground(0);
	ofSetVerticalSync(false);
	ofEnableAlphaBlending();
	ofDisableArbTex(); //necessary to pass textures as uniforms
	
	img.loadImage("test.jpg");
	
	//#---------------Kinect stuff---------------	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	
#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

    bKinectOpen = true ; 
	
    //#----------------UI
    setup_ofxUI() ;    
    
	//#--------------Shaders-Fbos-Etc.--------------------------
		//---defaults---
		timeSpeed = 3;
		blur = 10;
		thres = 130;
		//---loading and initializing---
    mask.setup( "shader/composite_rgb", ofRectangle( 0 , 0 , ofGetScreenWidth() , ofGetScreenHeight() )) ;
	shader.load("", "shaders/noiseBlur.glsl");
    shaderFbo.allocate( ofGetWidth() , ofGetHeight() ) ;
    shaderFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
    shaderFbo.end() ;
    
    maskFbo.allocate( ofGetWidth() , ofGetHeight() ) ;
    maskFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
    maskFbo.end() ;
	}

void testApp::setup_ofxUI()
{
    //Setup ofxUI
    float dim = 24;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    guiWidth = 210-xInit;
    
    gui1 = new ofxUIScrollableCanvas(0, 0, guiWidth+xInit, ofGetHeight());
	gui1->addWidgetDown(new ofxUILabel("KINECT PARAMS", OFX_UI_FONT_MEDIUM ));
    gui1->addWidgetDown(new ofxUIRangeSlider( guiWidth-xInit, dim, 0.0, 255.0, farThreshold, nearThreshold, "DEPTH RANGE"));
    gui1->addWidgetDown(new ofxUIRangeSlider(guiWidth-xInit, dim, 0.0, ((kinect.width * kinect.height ) / 2 ), minBlobSize , maxBlobSize, "BLOB SIZE"));
    gui1->addWidgetDown(new ofxUIToggle("THRESHOLD OPENCV" , bThreshWithOpenCV , dim , dim ) ); 
    gui1->addSlider( "MOTOR ANGLE", -30.0f  , 30.0f  , angle, guiWidth-xInit, dim ) ;
    gui1->addWidgetDown(new ofxUIToggle("OPEN KINECT" , bKinectOpen , dim , dim ) ) ;
	gui1->addSlider( "BLUR", 0  , 20 , blur, guiWidth-xInit, dim ) ;
	gui1->addSlider( "THRESHOLD", 0  , 150  , thres, guiWidth-xInit, dim ) ;
	
	//gui->setDrawBack(false);
	//gui1->setFont("GUI/FreeUniversal-Regular.ttf");
    
    ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
    gui1->loadSettings("GUI/kinectSettings.xml") ;
	gui1->setVisible(false);
}

//--------------------------------------------------------------
void testApp::update() {
	
	ofBackground(0);
	
    //ofSetWindowTitle( "Kinect + Fragment Shader - FPS:"+ ofToString( ofGetFrameRate() ) ) ; 
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayImage.mirror(false, true);
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 100;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
        // findContours( ofxCvGrayscaleImage&  input, int minArea, int maxArea, int nConsidered, bool bFindHoles, bool bUseApproximation ) ;
        
		//grayImage.blurGaussian(30);
		//grayImage.threshold(15, false);
		contourFinder.findContours(grayImage, minBlobSize , maxBlobSize , 20, true);
		
	}
							   
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
	
}

//--------------------------------------------------------------
void testApp::draw() {
    
    shaderFbo.begin() ; //here will be any drawing method
	
        shader.begin() ;
			if (timeSpeed == 3) {
				shader.setUniformTexture("iChannel0", img, 1);
			}
			shader.setUniform1f("iGlobalTime", getTime(timeSpeed));
			shader.setUniform2f("iResolution", ofGetWidth(), ofGetHeight());
			ofSetColor( 255 ) ;
			ofRect( 0 , 0 , ofGetWidth() , ofGetHeight() ) ;
        shader.end() ;

    shaderFbo.end() ;
    
	maskFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
		grayImage.blur(blur);
		grayImage.threshold(thres, false);
        grayImage.draw( 0  , 0 , ofGetWidth() , ofGetHeight() ) ;
		//contourFinder.drawShape(0,0,ofGetWidth(),ofGetHeight());
		//ofSetColor(255);
		//ofRect( 0 , 0 , ofGetWidth()/2 , ofGetHeight()/2 ) ;

    maskFbo.end() ;
    
    ofSetColor( 255 ) ;
    mask.drawMask( shaderFbo.getTextureReference() , maskFbo.getTextureReference() , 0, 0, 1.0f ) ; 
    
    
}
//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	
	if(name == "DEPTH RANGE")
	{
		ofxUIRangeSlider *slider = (ofxUIRangeSlider *) e.widget;
        farThreshold = slider->getScaledValueLow() ;
        nearThreshold = slider->getScaledValueHigh() ; 
	}
    
    if(name == "BLOB SIZE")
	{
		ofxUIRangeSlider *slider = (ofxUIRangeSlider *) e.widget;
        minBlobSize = slider->getScaledValueLow() ;
        maxBlobSize = slider->getScaledValueHigh() ;
	}
    
    if(name == "THRESHOLD OPENCV" )
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        bThreshWithOpenCV = toggle->getValue() ;
	}
    
    if(name == "MOTOR ANGLE" )
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget;
        angle = slider->getScaledValue() ;
        kinect.setCameraTiltAngle(angle);
	}
    
    if(name == "OPEN KINECT" )
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        bKinectOpen = toggle->getValue() ;
        if ( bKinectOpen == true )
            kinect.open() ;
        else
            kinect.close() ;
	}
	
	if(name == "BLUR" )
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget;
        blur = slider->getScaledValue() ;
	}
	
	if(name == "THRESHOLD" )
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget;
        thres = slider->getScaledValue() ;
	}
    
    gui1->saveSettings("GUI/kinectSettings.xml") ; 
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.close();
	delete gui1;
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//----------------------------------
float testApp::getTime(int t){
	t = timeSpeed;
	switch (t) {
		case 1:
			return ofGetElapsedTimef();
			break;
		case 2:
			return (ofGetFrameNum() * 0.001);
			break;
		case 3:
			return ofGetElapsedTimef();
			break;
		default:
			break;
	}

}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
    
	switch (key) {
	
		case 'D':
		case 'd':
			gui1->toggleVisible();
			break;
		case 'f':
			ofToggleFullscreen();
			break;
		case '1':
			shader.load( "shader/basicVertex.vert" , "shader/hexagon.frag" ) ;
			timeSpeed = 1;
			break;
		case '2':
			shader.load( "shader/noise.vert" , "shader/noise.frag" ) ;
			timeSpeed = 2;
			break;
		case '3':
			shader.load( "" , "shader/noiseBlur.glsl" ) ;
			timeSpeed = 3;
			break;

			
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}