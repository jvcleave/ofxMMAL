#include "ofMain.h"
#include "ofGLProgrammableRenderer.h"

#include "ofApp.h"
int main()
{
	
	ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp( new ofApp());
}