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
    camera.setup();
}

//--------------------------------------------------------------
void ofApp::update(){

    if(doTakePhoto)
    {
        doTakePhoto = false;
        camera.takePhoto();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
	

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	ofLogVerbose() << "keyPressed: " << key;
	if (key == 'e') 
	{
        doTakePhoto = true;
	}
}
