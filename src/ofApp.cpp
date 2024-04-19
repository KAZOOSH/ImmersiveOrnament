#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    settings = ofLoadJson("settings.json");
    
    //init video stream
    vidGrabber.listDevices();

    vidGrabber.setDeviceID(1);
	vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(640, 480);
    
    //init ornament
    ornament.setup(1920*3,1080*2);
    
    //assign video input to ornament
    ornament.loadTexture(vidGrabber.getTexture());
    
    //setup gui
    tileSize.set("tile size", 150, 30, 400);
    tileSize.addListener(this, &ofApp::onTileSizeChanged);
    wallpaperGroup.set("group", 12, 9, 16);
    wallpaperGroup.addListener(this, &ofApp::onWallpaperGroupChanged);
    angle.set("angle", 0, 2*PI, 0);
    angle.addListener(this, &ofApp::onAngleChanged);
    panel.setup();
    panel.add(tileSize);
    panel.add(wallpaperGroup);
    panel.add(angle);

    // init warper
    int w = 0;
    int h = 0;
    int i = 0;
    for (auto &s : settings["screens"])
    {
        w = max(w, s["size"][0].get<int>());
        h += s["size"][1].get<int>();

        int ws = s["size"][0].get<int>();
        int hs = s["size"][1].get<int>();

        warper.push_back(ofxQuadWarp());
        warper.back().setSourceRect(ofRectangle(0, 0, ws, hs));      // this is the source rectangle which is the size of the image and located at ( 0, 0 )
        warper.back().setTopLeftCornerPosition(ofPoint(0, 0));       // this is position of the quad warp corners, centering the image on the screen.
        warper.back().setTopRightCornerPosition(ofPoint(ws, 0));     // this is position of the quad warp corners, centering the image on the screen.
        warper.back().setBottomLeftCornerPosition(ofPoint(0, hs));   // this is position of the quad warp corners, centering the image on the screen.
        warper.back().setBottomRightCornerPosition(ofPoint(ws, hs)); // this is position of the quad warp corners, centering the image on the screen.
        warper.back().setup();
        warper.back().disableKeyboardShortcuts();
        warper.back().disableMouseControls();
        warper.back().hide();
        warper.back().load(ofToString(i), "settings.json");
        ++i;
    }    
}


//--------------------------------------------------------------
void ofApp::update(){
    
    //update vidgrabber
    vidGrabber.update();
    
    //update ornament
    if(vidGrabber.isFrameNew()){ 
        ornament.update();
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //draw ornament
    ornament.draw(0,0);
    
    //draw debug view to see ornament part in camera picture
    ornament.drawDebug(0, 300,400,300);
    
    //draw gui
    ofDrawBitmapStringHighlight("wallpaperGroup: " + ofToString(ornament.getWallpaperGroupAsInt()), 10, 580);
    ofDrawBitmapStringHighlight("cellStructure: " + ofToString(ornament.getCellStructure()), 10, 600);
    panel.draw();
    
}

void ofApp::drawWindow2(ofEventArgs &args)
{
    ofBackground(0);
    drawScreen(1);
}

void ofApp::keyPressedWindow2(ofKeyEventArgs &args)
{
    processKeyPressedEvent(args.key, 1);
}

void ofApp::keyPressedWindow3(ofKeyEventArgs &args)
{
    processKeyPressedEvent(args.key, 2);
}

void ofApp::keyPressedWindow4(ofKeyEventArgs &args)
{
    processKeyPressedEvent(args.key, 3);
}

void ofApp::drawWindow3(ofEventArgs &args)
{
    ofBackground(0);
    drawScreen(2);
}

void ofApp::drawWindow4(ofEventArgs &args)
{
    ofBackground(0);
    drawScreen(3);
}

void ofApp::exit()
{
    for (size_t i = 0; i < warper.size(); i++)
    {
        warper[i].save(ofToString(i), "settings.json");
    }
}

void ofApp::drawScreen(int screenId)
{
    auto settingsScreen = settings["screens"][screenId];

    //======================== get our quad warp matrix.

    ofMatrix4x4 mat = warper[screenId].getMatrix();

    //======================== use the matrix to transform our fbo.

    ofPushMatrix();
    ofMultMatrix(mat);

    ofSetColor(255);
    ornament.getTexture().drawSubsection(
        ofRectangle(0, 0, settingsScreen["size"][0].get<int>(), settingsScreen["size"][1].get<int>()),
        ofRectangle(settingsScreen["texturePosition"][0], settingsScreen["texturePosition"][1], settingsScreen["size"][0].get<int>(), settingsScreen["size"][1].get<int>()));
   
    ofPopMatrix();

    //======================== use the matrix to transform points.

    ofSetLineWidth(2);
    ofSetColor(ofColor::cyan);

    for (int i = 0; i < 9; i++)
    {
        int j = i + 1;

        ofVec3f p1 = mat.preMult(ofVec3f(points[i].x, points[i].y, 0));
        ofVec3f p2 = mat.preMult(ofVec3f(points[j].x, points[j].y, 0));

        ofDrawLine(p1.x, p1.y, p2.x, p2.y);
    }

    //======================== draw quad warp ui.

    ofSetColor(ofColor::magenta);
    warper[screenId].drawQuadOutline();

    ofSetColor(ofColor::yellow);
    warper[screenId].drawCorners();

    ofSetColor(ofColor::magenta);
    warper[screenId].drawHighlightedCorner();

    ofSetColor(ofColor::red);
    warper[screenId].drawSelectedCorner();
}

void ofApp::processKeyPressedEvent(int key, int screenId)
{
    if (key == 'h' || key == 'H')
    {
        for (size_t i = 0; i < warper.size(); i++)
        {
            warper[i].hide();
            isDebug = false;
        }
    }
    if (key == 'd' || key == 'D')
    {
        for (size_t i = 0; i < warper.size(); i++)
        {
            if (i == screenId)
            {
                warper[i].enableKeyboardShortcuts();
                warper[i].enableMouseControls();
                warper[i].show();
                isDebug = true;
            }
            else
            {
                warper[i].disableKeyboardShortcuts();
                warper[i].disableMouseControls();
                warper[i].hide();
                isDebug = false;
            }
        }
    }

    if (key == 'l' || key == 'L')
    {
        for (size_t i = 0; i < warper.size(); i++)
        {
            warper[i].load(ofToString(i), "settings.json");
        }
    }

    if (key == 's' || key == 'S')
    {
        for (size_t i = 0; i < warper.size(); i++)
        {
            warper[i].save(ofToString(i), "settings.json");
        }
    }
    if (key == 'r')
    {
        settings = ofLoadJson("settings.json");
    }
    if (key == 'f') {
        ofToggleFullscreen();
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    processKeyPressedEvent(key,0);
    //manipulate inputs
    if (key == 'm'){
        ornament.setCellStructure(ornament.getCellStructure()+1);
    }
    if (key == 'n'){
        ornament.setCellStructure(ornament.getCellStructure()-1);
    }
    if (key == 'c'){
        int wg = ornament.getWallpaperGroupAsInt() + 1;
        ornament.setWallpaperGroup(wg);
    }
    if (key == 'v'){
        ornament.setWallpaperGroup(ornament.getWallpaperGroupAsInt()-1);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

void ofApp::onTileSizeChanged(int& t){
    ornament.setTileSize(t);
}

void ofApp::onWallpaperGroupChanged(int& t){
    ornament.setWallpaperGroup(t);
}

void ofApp::onAngleChanged(float& t){
    ornament.setAngle(t);
}
