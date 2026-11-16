#pragma once

#include "ofMain.h"
#include "Ornament.h"
#include "ofxGui.h"
#include "ofxQuadWarp.h"
#include "ofxOsc.h"
//#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

public:

    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

        void drawWindow2(ofEventArgs& args);
    void drawWindow3(ofEventArgs& args);
    void drawWindow4(ofEventArgs& args);
    void keyPressedWindow2(ofKeyEventArgs& args);
    void keyPressedWindow3(ofKeyEventArgs& args);
    void keyPressedWindow4(ofKeyEventArgs& args);
    void exit();

    void drawScreen(int screenId);
    void processKeyPressedEvent(int key, int screenId);
    void fillGradient();
    void loadTrailSettings();

    void onTileSizeChanged(int& t);
    void onWallpaperGroupChanged(int& t);
    void onAngleChanged(float& t);

    Ornament ornament;
    ofFbo gradient;
    ofFbo trail;
    float trailOpacity = 1.0f;
    // trailOpacity = map(smoothedVisualFlow, [flowLow, flowHigh] -> [opacityStill, opacityMoving])
    float trailOpacityStill = 1.0f;
    float trailOpacityMoving = 0.05f;
    float trailFlowLow = 0.3f;
    float trailFlowHigh = 5.0f;

    ofxPanel panel;
    ofParameter<int> tileSize;
    ofParameter<int> wallpaperGroup;
    ofParameter<float> angle;

    ofxOscReceiver receiver;

    ofVideoGrabber vidGrabber;
    vector<ofxQuadWarp> warper;
    ofJson settings;

    bool isDebug = false;
    ofPoint points[10];

    // average optical flow of the camera image in camera pixels per frame
    float amtVisualFlow = 0;
    float smoothedVisualFlow = 0; // amtVisualFlow low-pass filtered
    void updateVisualFlow(const ofPixels& pix);
    std::vector<float> flowPrev, flowCurr; // downscaled grayscale frames
    int flowW = 0, flowH = 0;
};
