#pragma once

#include "ofMain.h"
#include "TerminalListener.h"
#include "StillCamera.h"


class ofApp : public ofBaseApp, public KeyListener{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);

	
		StillCamera camera;
        TerminalListener consoleListener;
        void onCharacterReceived(KeyListenerEventData& e);
    
        bool doTakePhoto;
	
};

