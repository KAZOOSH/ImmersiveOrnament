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

    void onTileSizeChanged(int& t);
    void onWallpaperGroupChanged(int& t);
    void onAngleChanged(float& t);
    
    Ornament ornament;
    
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
};

