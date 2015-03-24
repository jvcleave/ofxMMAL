#pragma once

#include "ofMain.h"
#include "TerminalListener.h"
#include "VideoCamera.h"


class ofApp : public ofBaseApp, public KeyListener{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);

	
		VideoCamera camera;
        TerminalListener consoleListener;
        void onCharacterReceived(KeyListenerEventData& e);
    	
};

