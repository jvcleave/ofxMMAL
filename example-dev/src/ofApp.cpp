#include "ofApp.h"

void ofApp::onCharacterReceived(KeyListenerEventData& e)
{
	keyPressed((int)e.character);
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetLogLevel("ofThread", OF_LOG_SILENT);
    
    doTakePhoto = false;
	consoleListener.setup(this);
    photo.allocate(camera.width, camera.height, OF_IMAGE_COLOR);
    hasTakenPhoto = false;
    camera.setup();
}

//--------------------------------------------------------------
void ofApp::update(){

    if(doTakePhoto)
    {
        doTakePhoto = false;
        hasTakenPhoto = photo.loadImage(camera.takePhoto());
        
        if(hasTakenPhoto)
        {
            photo.update();
        }else{
            ofLogError(__func__) << "LOAD PHOTO FAIL";
        }
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if(hasTakenPhoto)
    {
        photo.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	ofLogVerbose() << "keyPressed: " << key;
	if (key == 'e') 
	{
        doTakePhoto = true;
	}
}
