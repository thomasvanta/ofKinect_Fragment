#include "testApp.h"

/*char sz[] = "[Rd9?-2XaUP0QY[hO%9QTYQ`-W`QZhcccYQY[`b";*/

//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	//for(int i=0; i<strlen(sz); i++) sz[i] += 20;
	
	ofEnableSmoothing();
	//ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//ofBackground(0);
	
	// my colors from colorlovers
	//color1 = (33, 20, 38);
	//color2 = (65,84,59);
	//color3 = (155,191,171);
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	

	ofSetFrameRate(60);

    bKinectOpen = true ; 
    
    setup_ofxUI() ;    
    
    mask.setup( "shader/composite_rgb", ofRectangle( 0 , 0 , ofGetScreenWidth() , ofGetScreenHeight() )) ;
    //shader.load( "shader/basicVertex.vert" , "shader/hexagon.frag" ) ;
	shader.load( "shader/noise.vert" , "shader/noise.frag" ) ;
	//shader.load( "shader/noise2.vert" , "shader/noise2.frag" ) ;
	timeSpeed = 2;
    shaderFbo.allocate( ofGetWidth() , ofGetHeight() ) ;
    shaderFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
    shaderFbo.end() ;
    
    maskFbo.allocate( ofGetWidth() , ofGetHeight() ) ;
    maskFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
    maskFbo.end() ;
	
	/*
	// setup fluid stuff
	fluidSolver.setup(100, 100);
    fluidSolver.enableRGB(true).setFadeSpeed(0.125).setDeltaT(0.5).setVisc(0.0005).setColorDiffusion(0);
	fluidDrawer.setup(&fluidSolver);
	
	fluidCellsX			= 150;
	
	drawFluid			= true;
	drawParticles		= false;
	
	windowResized(ofGetWidth(), ofGetHeight());		// force this at start (cos I don't think it is called)
	pMouse = msa::getWindowCenter();
	resizeFluid			= true;
	
	ofEnableAlphaBlending();
	ofSetBackgroundAuto(false);
	 
	
#ifdef USE_GUI 
	gui.addSlider("fluidCellsX", fluidCellsX, 20, 400);
	gui.addButton("resizeFluid", resizeFluid);
    gui.addSlider("colorMult", colorMult, 0, 100);
    gui.addSlider("velocityMult", velocityMult, 0, 100);
	gui.addSlider("fs.viscocity", fluidSolver.viscocity, 0.0, 0.01);
	gui.addSlider("fs.colorDiffusion", fluidSolver.colorDiffusion, 0.0, 0.0003); 
	gui.addSlider("fs.fadeSpeed", fluidSolver.fadeSpeed, 0.0, 0.1); 
	gui.addSlider("fs.solverIterations", fluidSolver.solverIterations, 1, 50); 
	gui.addSlider("fs.deltaT", fluidSolver.deltaT, 0.1, 5);
	gui.addComboBox("fd.drawMode", (int&)fluidDrawer.drawMode, msa::fluid::getDrawModeTitles());
	gui.addToggle("fs.doRGB", fluidSolver.doRGB); 
	gui.addToggle("fs.doVorticityConfinement", fluidSolver.doVorticityConfinement); 
	gui.addToggle("drawFluid", drawFluid); 
	gui.addToggle("drawParticles", drawParticles); 
	gui.addToggle("fs.wrapX", fluidSolver.wrap_x);
	gui.addToggle("fs.wrapY", fluidSolver.wrap_y);
	gui.currentPage().setXMLName("ofxMSAFluidSettings.xml");
    gui.loadFromXML();
	gui.setDefaultKeys(true);
	gui.setAutoSave(false);
    gui.show();
#endif
	 */
	
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
	
	//gui->setUIColors( color3, color2, color1, color2, color1, color1, color1 );
	//gui->setDrawBack(false);
	gui1->setFont("GUI/FreeUniversal-Regular.ttf");
    
    ofAddListener(gui1->newGUIEvent,this,&testApp::guiEvent);
    gui1->loadSettings("GUI/kinectSettings.xml") ;
}

//--------------------------------------------------------------
void testApp::update() {
	
	ofBackground(0);
	
    ofSetWindowTitle( "Kinect + Fragment Shader - FPS:"+ ofToString( ofGetFrameRate() ) ) ; 
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
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
        
		contourFinder.findContours(grayImage, minBlobSize , maxBlobSize , 20, false);
		
		/*for (int i = 0; i < contourFinder.nBlobs; i++){
			//contourFinder.blobs[i].draw(360,540);
			// add vertex
			for (int j = 0; j < contourFinder.blobs[i].nPts; j++) {
				vertices.push_back(new ofVec2f(contourFinder.blobs[i].pts[j].x , contourFinder.blobs[i].pts[j].y));
			}
		}*/
	}
							   
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
	
	/*
	if(resizeFluid) 	{
		fluidSolver.setSize(fluidCellsX, fluidCellsX / msa::getWindowAspectRatio());
		fluidDrawer.setup(&fluidSolver);
		resizeFluid = false;
	}
	
	fluidSolver.update();
	*/
	
}

//--------------------------------------------------------------
void testApp::draw() {
	
	ofSetColor(100);
	
	//ofEllipse(ofGetWidth()/2, ofGetHeight()/2, 400, 400);
    
    shaderFbo.begin() ; //here will be any drawing method
	
	//Diana!!!!!!!!
	//lo que quieras dibujar debe ir en este FBO, as’ que entre las lineas shaderFBO.begin y shaderFBO.end
	//ahora mismo hay dos shaders que se alternan con las teclas 1 y 2
	
        shader.begin() ;
            //shader.setUniform1f("time", ofGetElapsedTimef() );
			//shader.setUniform1f("time", ofGetFrameNum() * 0.001);
			shader.setUniform1f("time", getTime(timeSpeed));
        ofSetColor( 255 ) ;
        ofRect( 0 , 0 , ofGetWidth() , ofGetHeight() ) ;
        shader.end() ;
	/*
	ofClear(0);
	glColor3f(1, 1, 1);
	fluidDrawer.draw(0, 0, ofGetWidth(), ofGetHeight());
	*/
	
    shaderFbo.end() ;
    
	ofSetPolyMode(OF_POLY_WINDING_NONZERO);
    maskFbo.begin() ;
        ofClear( 1 , 1 , 1 , 0 ) ;
		//grayImage.blurGaussian(8);
        grayImage.draw( 0  , 0 , ofGetWidth() , ofGetHeight() ) ;
		
		/*ofBeginShape();
			contourFinder.drawShape(0,0,ofGetWidth(),ofGetHeight());
			//for (int i = 0; i < vertices.size(); i++) {
			//	ofVertex(vertices[i].x, vertices[i].y);
			//}
		ofEndShape();*/

    maskFbo.end() ;
    
    ofSetColor( 100 ) ;
    mask.drawMask( shaderFbo.getTextureReference() , maskFbo.getTextureReference() , 0, 0, 1.0f ) ; 
	
    //to debug to the screen use
    //ofDrawBitmapString( "STRING" , x , y ) ;
	
	//vertices.clear();
    
    
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
		default:
			break;
	}

}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
    
    //ofSaveScreen( ofToDataPath( ofToString( ofGetUnixTime() )  + ".jpg" ) ) ;
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

/*
void testApp::fadeToColor(float r, float g, float b, float speed) {
    glColor4f(r, g, b, speed);
	ofRect(0, 0, ofGetWidth(), ofGetHeight());
}


// add force and dye to fluid, and create particles
void testApp::addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce) {
    float speed = vel.x * vel.x  + vel.y * vel.y * msa::getWindowAspectRatio() * msa::getWindowAspectRatio();    // balance the x and y components of speed with the screen aspect ratio
    if(speed > 0) {
		pos.x = ofClamp(pos.x, 0.0f, 1.0f);
		pos.y = ofClamp(pos.y, 0.0f, 1.0f);
		
        int index = fluidSolver.getIndexForPos(pos);
		
		if(addColor) {
			//			Color drawColor(CM_HSV, (getElapsedFrames() % 360) / 360.0f, 1, 1);
			ofColor drawColor;
			drawColor.setHsb((ofGetFrameNum() % 255), 255, 255);
			
			fluidSolver.addColorAtIndex(index, drawColor * colorMult);
			
			if(drawParticles)
				particleSystem.addParticles(pos * ofVec2f(ofGetWindowSize()), 10);
		}
		
		if(addForce)
			fluidSolver.addForceAtIndex(index, vel * velocityMult);
		
    }
}
*/

//como ves esto est‡ hecho un caos.