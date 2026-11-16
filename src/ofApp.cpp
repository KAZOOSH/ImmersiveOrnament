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
    ornament.setup(1920*3,1080);

    //init gradient (left color -> white -> right color), colors from settings.json
    gradient.allocate(1920*3,1080);
    fillGradient();

    //init trail buffer: each frame the new ornament is blended on top of the previous result.
    //16-bit float so low opacities fade out fully instead of leaving 8-bit ghosting
    ofFbo::Settings trailSettings;
    trailSettings.width = 1920*3;
    trailSettings.height = 1080;
    trailSettings.internalformat = GL_RGBA16F;
    trail.allocate(trailSettings);
    trail.begin();
    ofClear(0, 0, 0, 255);
    trail.end();
    loadTrailSettings();

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

    receiver.setup(settings["network"]["port"]);
}


//--------------------------------------------------------------
void ofApp::update(){

    //update vidgrabber
    vidGrabber.update();

    //update ornament
    if(vidGrabber.isFrameNew()){
        ornament.update();
        updateVisualFlow(vidGrabber.getPixels());
    }

    //trail opacity driven by motion: little flow -> high opacity (short trail),
    //lots of flow -> low opacity (long trail)
    trailOpacity = ofMap(smoothedVisualFlow, trailFlowLow, trailFlowHigh,
                         trailOpacityStill, trailOpacityMoving, true);

    //blend the current ornament on top of the last frame with trailOpacity
    //(1.0 = no trail, smaller = longer trail)
    trail.begin();
    ofPushStyle();
    ofEnableAlphaBlending();
    ofSetColor(ofFloatColor(1, 1, 1, trailOpacity));
    //ofBackground(0);
    ornament.getTexture().draw(0, 0, trail.getWidth(), trail.getHeight());
    ofPopStyle();
    trail.end();

    while(receiver.hasWaitingMessages()){

		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(m);
		// check for mouse moved message
		if(m.getAddress() == "/1/fader3"){
            tileSize = ofMap(m.getArgAsFloat(0),0,1,30,400);
            //cout << tileSize <<endl;
            //ornament.setTileSize(ofMap(m.getArgAsFloat(0),0,1,30,400));
		}else if(m.getAddress() == "/1/fader2"){
            wallpaperGroup = ofMap(m.getArgAsFloat(0),0,1,9,16.9,true);
            //ornament.setWallpaperGroup(ofMap(m.getArgAsFloat(0),0,1,9,16.9,true));
		}else if(m.getAddress() == "/1/fader1"){
            angle = ofMap(m.getArgAsFloat(0),0,1,0,2*PI);
            //ornament.setAngle(ofMap(m.getArgAsFloat(0),0,1,0,2*PI));
		}
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //draw ornament
    ofBackground(0);
    drawScreen(0);

    //draw debug view to see ornament part in camera picture
    if(isDebug){
        ofPushStyle();
        ornament.drawDebug(400, 400,400,300);
        //draw gui
        ofDrawBitmapStringHighlight("wallpaperGroup: " + ofToString(ornament.getWallpaperGroupAsInt()), 10, 580);
        ofDrawBitmapStringHighlight("cellStructure: " + ofToString(ornament.getCellStructure()), 10, 600);
        ofDrawBitmapStringHighlight("visualFlow: " + ofToString(amtVisualFlow, 2) + " px/frame  (smoothed: " + ofToString(smoothedVisualFlow, 2) + ")", 10, 620);
        ofDrawBitmapStringHighlight("trailOpacity: " + ofToString(trailOpacity, 3), 10, 640);
        panel.draw();
        ofPopStyle();
    }
}

void ofApp::drawWindow2(ofEventArgs &args)
{
    ofBackground(0);
    drawScreen(1);
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

    int sw = settingsScreen["size"][0].get<int>();
    int sh = settingsScreen["size"][1].get<int>();
    ofRectangle dstRect(0, 0, sw, sh);
    ofRectangle srcRect(settingsScreen["texturePosition"][0].get<int>(), settingsScreen["texturePosition"][1].get<int>(), sw, sh);

    ofPushStyle();
    ofSetColor(255);
    // trail alpha is not meaningful (alpha blending also blends alpha), so draw it opaque
    ofDisableAlphaBlending();
    trail.getTexture().drawSubsection(dstRect, srcRect);

    // multiply the same part of the gradient on top: result = trail * gradient
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    gradient.getTexture().drawSubsection(dstRect, srcRect);
    ofPopStyle();

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
    ofSetColor(255);
}

// reads a color from settings.json as [r, g, b] (0-255), falls back if missing/invalid
static ofColor colorFromJson(const ofJson& j, const ofColor& fallback)
{
    if (!j.is_array() || j.size() < 3) return fallback;
    return ofColor(j[0].get<int>(), j[1].get<int>(), j[2].get<int>());
}

void ofApp::fillGradient()
{
    const auto& g = settings["gradient"];
    ofColor left  = colorFromJson(g["left"],  ofColor::red);
    ofColor right = colorFromJson(g["right"], ofColor::blue);

    float w = gradient.getWidth();
    float h = gradient.getHeight();

    // triangle strip: left edge, center, right edge; GPU interpolates the colors
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.addVertex({0, 0, 0});        mesh.addColor(left);
    mesh.addVertex({0, h, 0});        mesh.addColor(left);
    mesh.addVertex({w / 2, 0, 0});    mesh.addColor(ofColor::white);
    mesh.addVertex({w / 2, h, 0});    mesh.addColor(ofColor::white);
    mesh.addVertex({w, 0, 0});        mesh.addColor(right);
    mesh.addVertex({w, h, 0});        mesh.addColor(right);

    gradient.begin();
    ofClear(255);
    mesh.draw();
    gradient.end();
}

// Estimates how much the camera image moves between two frames.
// Works on a box-filtered grayscale image ~160px wide and computes the
// gradient-weighted normal flow:  sum(|It| * |grad I|) / sum(|grad I|^2)
// which is the least-squares flow magnitude along the image gradients.
// Result is in camera pixels per frame (0 = still).
void ofApp::updateVisualFlow(const ofPixels& pix)
{
    const int w = pix.getWidth();
    const int h = pix.getHeight();
    const int ch = pix.getNumChannels();
    if (w == 0 || h == 0 || ch == 0) return;

    const int block = std::max(1, w / 160);
    const int fw = w / block;
    const int fh = h / block;

    // downscale + grayscale
    flowCurr.assign(fw * fh, 0.0f);
    const unsigned char* data = pix.getData();
    const float norm = 1.0f / (block * block * std::min(ch, 3));
    for (int y = 0; y < fh; y++) {
        for (int x = 0; x < fw; x++) {
            float sum = 0;
            for (int by = 0; by < block; by++) {
                const unsigned char* row = data + ((y * block + by) * w + x * block) * ch;
                for (int bx = 0; bx < block; bx++) {
                    const unsigned char* p = row + bx * ch;
                    for (int c = 0; c < std::min(ch, 3); c++) sum += p[c];
                }
            }
            flowCurr[y * fw + x] = sum * norm;
        }
    }

    // first frame or resolution changed: nothing to compare against yet
    if (fw != flowW || fh != flowH || flowPrev.size() != flowCurr.size()) {
        flowW = fw;
        flowH = fh;
        flowPrev = flowCurr;
        amtVisualFlow = 0;
        return;
    }

    double num = 0, den = 0;
    for (int y = 1; y < fh - 1; y++) {
        for (int x = 1; x < fw - 1; x++) {
            int i = y * fw + x;
            // spatial gradient on the average of both frames, temporal difference
            float ix = 0.25f * (flowCurr[i + 1] - flowCurr[i - 1] + flowPrev[i + 1] - flowPrev[i - 1]);
            float iy = 0.25f * (flowCurr[i + fw] - flowCurr[i - fw] + flowPrev[i + fw] - flowPrev[i - fw]);
            float it = flowCurr[i] - flowPrev[i];
            float g2 = ix * ix + iy * iy;
            num += std::abs(it) * std::sqrt(g2);
            den += g2;
        }
    }

    amtVisualFlow = den > 1e-6 ? float(num / den) * block : 0.0f;
    smoothedVisualFlow = ofLerp(smoothedVisualFlow, amtVisualFlow, 0.1);
    std::swap(flowPrev, flowCurr);
}

void ofApp::loadTrailSettings()
{
    ofJson t = (settings.contains("trail") && settings["trail"].is_object()) ? settings["trail"] : ofJson::object();
    trailOpacityStill  = ofClamp(t.value("opacityStill",  1.0f),  0.0f, 1.0f);
    trailOpacityMoving = ofClamp(t.value("opacityMoving", 0.05f), 0.0f, 1.0f);
    trailFlowLow  = t.value("flowLow",  0.3f);
    trailFlowHigh = t.value("flowHigh", 5.0f);
    if (trailFlowHigh <= trailFlowLow) trailFlowHigh = trailFlowLow + 0.001f;
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
        isDebug = true;
        for (size_t i = 0; i < warper.size(); i++)
        {
            if (i == screenId)
            {
                warper[i].enableKeyboardShortcuts();
                warper[i].enableMouseControls();
                warper[i].show();

            }
            else
            {
                warper[i].disableKeyboardShortcuts();
                warper[i].disableMouseControls();
                warper[i].hide();
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
        fillGradient();
        loadTrailSettings();
    }
    if (key == 'f') {
        ofToggleFullscreen();
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    processKeyPressedEvent(key,0);
    //manipulate inputs
    /*if (key == 'm'){
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
    }*/
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
