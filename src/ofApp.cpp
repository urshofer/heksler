#include "ofApp.h"

void ofApp::guiSetup() {
    bitrateSlider.addListener(this, &ofApp::bitrateChanged);
	widthSlider.addListener(this, &ofApp::widthChanged);
    acThresholdSlider.addListener(this, &ofApp::acThresholdChanged);
    acMinSlider.addListener(this, &ofApp::acMinChanged);
    
    transcodeToggle.addListener(this,&ofApp::buttonPressed);
    proresToggle.addListener(this,&ofApp::buttonPressed);
    
	gui.setup(); // most of the time you don't need a name
    gui.loadFont("Roboto-Gui.ttf", 10);
    font.loadFont("Roboto-Gui.ttf", 15);
    fontlarge.loadFont("Roboto-Gui.ttf", 60);
    
    bg.loadImage("gui.png");
    
    gui.setDefaultBorderColor(ofColor(70,93,109));
    gui.setDefaultFillColor(ofColor(70,93,109));
    gui.setFillColor(ofColor(70,93,109));
    
    gui.setDefaultHeaderBackgroundColor(ofColor(16,22,26));
    gui.setDefaultBackgroundColor(ofColor(16,22,26));
    gui.setHeaderBackgroundColor(ofColor(16,22,26));
    
    gui.setDefaultHeight(24);
    gui.setDefaultTextColor(255);
    gui.setDefaultTextPadding(20);
    gui.setDefaultWidth(250);
    
    gui.setPosition(695, 560);
    
    transcodeToggle.setFillColor(ofColor(70,93,109));
    transcodeToggle.setTextColor(ofColor(70,93,109));
    transcodeToggle.setBackgroundColor(ofColor(16,22,26));
    
    proresToggle.setFillColor(ofColor(70,93,109));
    proresToggle.setTextColor(ofColor(70,93,109));
    proresToggle.setBackgroundColor(ofColor(16,22,26));
    
    bitrateSlider.setFillColor(ofColor(70,93,109,50));
    bitrateSlider.setTextColor(ofColor(70,93,109));
    bitrateSlider.setBackgroundColor(ofColor(16,22,26));
    
    widthSlider.setFillColor(ofColor(70,93,109,50));
    widthSlider.setTextColor(ofColor(70,93,109));
    widthSlider.setBackgroundColor(ofColor(16,22,26));
    
    acThresholdSlider.setFillColor(ofColor(70,93,109,50));
    acThresholdSlider.setTextColor(ofColor(70,93,109));
    acThresholdSlider.setBackgroundColor(ofColor(16,22,26));
    
    acMinSlider.setFillColor(ofColor(70,93,109,50));
    acMinSlider.setTextColor(ofColor(70,93,109));
    acMinSlider.setBackgroundColor(ofColor(16,22,26));
    
    gui.add(bitrateSlider.setup("Output bitrate", 500, 100, 5000));
    gui.add(widthSlider.setup("Output width", 640, 100, 2000));
    
	gui.add(transcodeToggle.setup("Transcode", true));
	gui.add(proresToggle.setup("H264/MP4 encoding", false));
    
    gui.add(acThresholdSlider.setup("Autocut threshold", 120, 0, 255));
    gui.add(acMinSlider.setup("Autocut minimum frames", 5, 0, 255));
}

//--------------------------------------------------------------
void ofApp::setup(){
    
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources/data/");
#endif

    
    
	ofBackground(60);
    ofEnableAlphaBlending();
    frameByframe=false;
    shift=false;
    caps = false;
    autocut_threshold = 120;
    autocut_minlength = 10;
    ofSetFrameRate(200);

    guiSetup();
    
	// Uncomment this to show movies with alpha channels
	fingerMovie.setPixelFormat(OF_PIXELS_RGB);
	fingerMovie.setLoopState(OF_LOOP_NORMAL);

    ffmpeg.start();
    ofAddListener(ffmpeg.onFileProcessed,this, &ofApp::onFileProcessed);
    
    colorImg.allocate(320,240);
    red.allocate(320,240);
    redBG.allocate(320,240);
    redDiff.allocate(320,240);
    
    green.allocate(320,240);
    greenBG.allocate(320,240);
    greenDiff.allocate(320,240);
    
    blue.allocate(320,240);
    blueBG.allocate(320,240);
    blueDiff.allocate(320,240);
    
    
	/* Load Configuration */
    
    
    /* XML Settings */
    ofxXmlSettings XML;
    string user, pass;
	XML.loadFile("serversettings.xml");
	user	= XML.getValue("USER", __USER__);
	pass	= XML.getValue("PASS", __PASS__);
	apiurl	= XML.getValue("URL", __API__);
    
	/* Logging in at the API */
    
    MD5Engine md5;
    md5.update(pass);
    Json::Reader getdata;
    Json::Value  returnval;
	if (getdata.parse( curlConnect(apiurl + "/Login", "username=" + user + "&password=" + DigestEngine::digestToHex(md5.digest()) ), returnval )) {
        sessionid = returnval.asString();
        cout << "Session ID: " << sessionid << endl;
    }
	else {
        cout << "Could not connect to API Server and Log In\n"; ofExit();
    }

    keywordsloaded = loadKeywords();
    ofAddListener(fm.formResponseEvent, this, &ofApp::newResponse);
    
}

void ofApp::newResponse(HttpFormResponse &response){
	printf("form '%s' returned : %s\n", response.url.c_str(), response.ok ? "OK" : "KO" );
}


void ofApp::onFileProcessed(ofxVideoSlicer::endEvent & ev) {
    cout << ev.file << " processed with the message " << ev.message;
	HttpForm f = HttpForm( apiurl + "/StoreFileAnnotaded/" + sessionid );
	//form field name, file name, mime type
	f.addFile("file", ev.file, "movie/mpeg");
    f.addFormField("meta", ev.message);
	fm.submitForm( f, false );
}

bool ofApp::loadKeywords() {
    Json::Reader getdata;
	if (getdata.parse( curlConnect(apiurl + "/Load/" + sessionid + "/target", ""), keywords )) {
        cout << "Loaded keywords..." << endl;
        return true;
	}
    return false;
}


//--------------------------------------------------------------
void ofApp::buttonPressed(bool & toggle) {
    ffmpeg.setCodec(proresToggle ? "prores" : "h264");
    ffmpeg.setTranscode(transcodeToggle);
    
	transcodeToggle.setName(transcodeToggle?"Transcode":"Keep Original");
	proresToggle.setName(proresToggle?"Prores/MOV encoding":"H264/MP4 encoding");
    
    cout << "Transcode: " << transcodeToggle << " Prores: " << proresToggle << endl;
}

//--------------------------------------------------------------
void ofApp::acMinChanged(int & min){
    autocut_minlength = min;
}


//--------------------------------------------------------------
void ofApp::acThresholdChanged(int & thresh){
    autocut_threshold = thresh;
}

//--------------------------------------------------------------
void ofApp::bitrateChanged(int & rate){
    ffmpeg.setBitrate(rate);
}

//--------------------------------------------------------------
void ofApp::widthChanged(int & width){
    
    // Round width
    int targetH = fingerMovie.getHeight() / fingerMovie.getWidth() * width;
    ffmpeg.setSize(width, targetH);
}

int ofApp::avgPixel(ofPixelsRef & px) {
    int i = 0;
    int tot = 0;
    while( i < px.size()) {
        tot += px[i];
        i+=5;
    }
    tot /= (px.size()/5);
    return tot;
}

void ofApp::updateColorSize() {
    if (colorImg.getWidth() != fingerMovie.getWidth() || colorImg.getHeight() != fingerMovie.getHeight()) {
        colorImg.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        red.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        redBG.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        redDiff.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        
        green.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        greenBG.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        greenDiff.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        
        blue.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        blueBG.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
        blueDiff.allocate(fingerMovie.getWidth(),fingerMovie.getHeight());
    }
}

//

void ofApp::updateColorPics() {
    colorImg.setFromPixels(fingerMovie.getPixelsRef());
    colorImg.convertToGrayscalePlanarImages(red, green, blue);
    
    redDiff.absDiff(redBG,red);
    redDiff.threshold(30);
    
    greenDiff.absDiff(greenBG, green);
    greenDiff.threshold(30);
    
    blueDiff.absDiff(blueBG,blue);
    blueDiff.threshold(30);
    
    redBG = red;
    greenBG = green;
    blueBG = blue;
}


//--------------------------------------------------------------
void ofApp::update(){

    if (caps) {
        if (!cutter.isThreadRunning()){
            if (autocut) {
                autocut = false;
                fingerMovie.setFrame(fingerMovie.getCurrentFrame()-1);
                fingerMovie.setFrame(fingerMovie.getCurrentFrame()+1);
            }
            fingerMovie.update();
            if (fingerMovie.isFrameNew()) {
                updateColorPics();
            }
        }
    }
    else {
        if (autocut) {
            autocut = !cutter.updateSync(fingerMovie, autocut_direction, autocut_minlength, autocut_threshold, autocut_startframe);
        }
        else {
            fingerMovie.update();
        }
        if (fingerMovie.isFrameNew()) {
            updateColorPics();
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetHexColor(0x000000);
    ofRect(40,60,640,480);
    ofRect(695,60,250,150);
    ofRect(695,60+165,250,150);
    ofRect(695,60+330,250,150);

    ofSetHexColor(0xFFFFFF);
    font.drawString("Drag Video here", 55+(640/2)-(124/2), 58+240);

    if (!cutter.isThreadRunning()){
        if (fingerMovie.isLoaded()) {
            fingerMovie.draw(40,60 + ((480 - (640 / fingerMovie.getWidth() * fingerMovie.getHeight()))/2), 640, 640 / fingerMovie.getWidth() * fingerMovie.getHeight());
            ofSetHexColor(0xFF0000);
            float scaledH = 250.0f / (float)redDiff.width * redDiff.height;
            float centerV = (150 - scaledH) / 2;
            redDiff.draw(695,60 + centerV,250,scaledH);
            ofSetHexColor(0x00FF00);
            greenDiff.draw(695,60+165 + centerV,250, scaledH);
            ofSetHexColor(0x0000FF);
            blueDiff.draw(695,60+330 + centerV,250, scaledH);
        }
    }
    else {
        ofSetHexColor(0x333333);
        ofRect(40,60,640,480);
        ofSetHexColor(0xFFFFFF);
        font.drawString("Searching next cut", 55+(640/2)-(124/2), 58+240);
    }

    if (caps) {
        ofPushStyle();
        ofSetLineWidth(3);
        ofNoFill();
        ofSetHexColor(0xFF0000);
        ofRect(40,60,640,480);
        ofPopStyle();
    }
    
    ofSetColor(255, 255, 255, 255);
    bg.draw(0,0);

    ofSetHexColor(0x465d6d);    
    font.drawString("Settings",695,574);
    
    font.drawString("Current Frame",40,610);
    fontlarge.drawString(ofToString(fingerMovie.getCurrentFrame()),40,660);

    font.drawString("Total Frames",213+40,610);
    fontlarge.drawString(ofToString(fingerMovie.getTotalNumFrames()),213+40,660);

    font.drawString("Position",40,680);
    fontlarge.drawString(ofToString(fingerMovie.getPosition()*fingerMovie.getDuration(),2),40,730);
    
    font.drawString("Duration",213+40,680);
    fontlarge.drawString(ofToString(fingerMovie.getDuration(),2),213+40,730);

    ofSetHexColor(0xFF0000);
    font.drawString("In",(2*213)+40,610);
    font.drawString("Out",(2*213)+40,680);
    if (rec) {
        ofCircle(60, 80, 10);
        fontlarge.drawString(ofToString(in),(2*213)+40,660);
        fontlarge.drawString(ofToString(out),(2*213)+40,730);
        out = fingerMovie.getCurrentFrame();
    }
    ofSetHexColor(0x00FF00);
    font.drawString("Pending Clips: " + ofToString(ffmpeg.processingQueueSize()),695, 30);
    

    drawKeywords();
	gui.draw();
    
}

void ofApp::drawKeywords() {
    ofPushStyle();
    
    float sizeX = 295;
    float sizeY = 175;
    float startX = 985;
    float startY = 17;
    float offsetX = 0;
    float offsetY = 192;
    float dimCount = 0;
    json_encoded = "{";
    if (keywordsloaded && keywords.isObject()) {
        for( Json::ValueIterator itr = keywords.begin() ; itr != keywords.end() ; itr++ ) {
            string dim = itr.key().asString();
            if (dimCount>0) {
                json_encoded += ",";
            }
            json_encoded += "\""+dim+"\": [";
            float x = startX+(dimCount*offsetX);
            float y = startY+(dimCount*offsetY);
            ofSetHexColor(0x000000);
            ofRect(x, y,sizeX,sizeY);
            ofSetHexColor(0xFFFFFF);
            font.drawString(dim, x+5, y+15);
            ofSetHexColor(0x999999);
            
            // Draw Keywords
            for( Json::ValueIterator element = keywords[dim]["Keywords"].begin() ; element != keywords[dim]["Keywords"].end() ; element++ ) {
                string key = element.key().asString();
                float k_x = ofToFloat(keywords[dim]["Keywords"][key][0].asString());
                float k_y = ofToFloat(keywords[dim]["Keywords"][key][1].asString());
                ofRectangle s = font.getStringBoundingBox(key, 0, 0);
                font.drawString(key, x+(sizeX/100*k_x)-(s.width/2), y+(sizeY/100*k_y)-(s.height/2));
            }
            // Draw Coord Points
            map_vector::const_iterator mactive = maps.find(dim);
            ofSetHexColor(0xFF0000);
            ofRectangle clearbutton = font.getStringBoundingBox("Clear", 0, 0);
            font.drawString("Clear", x+sizeX-5-clearbutton.width, y+15);
            int coordCount = 0;
			if(mactive != maps.end())
			{
                for(std::vector< ofPoint >::const_iterator qactive = mactive->second.coords.begin(); qactive != mactive->second.coords.end(); ++qactive) {
                    ofCircle(x+(sizeX/100*qactive->x)-1, y+(sizeY/100*qactive->y)-1, 2);
                    if (coordCount>0) {
                        json_encoded += ",";
                    }
                    json_encoded += "[\"" + ofToString(qactive->x) + "\",\"" + ofToString(qactive->y) + "\"]";
                    coordCount++;
                }
			}
            else {
                _keymap _k;
                _k.bounds = ofRectangle(x, y,sizeX,sizeY);
                _k.clearbutton = font.getStringBoundingBox("Clear", x+sizeX-5-clearbutton.width, y+15);
                _k.coords.clear();
                maps[dim] = _k;
                cout << "Added " << dim << endl;
            }

            json_encoded += "]";
            dimCount++;
        }
    }
    json_encoded += "}";
    
    ofPopStyle();
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    if (!cutter.isThreadRunning()) {
        switch(key){
            case -1:
                caps = true;
                break;
            case OF_KEY_SHIFT:
                shift = true;
                break;
            case ' ':
                frameByframe=!frameByframe;
                fingerMovie.setPaused(frameByframe);
                if (!cutter.isThreadRunning()) {
                    autocut = false;
                }
            break;
            case OF_KEY_LEFT:
                if (shift) {
                    fingerMovie.setFrame(fingerMovie.getCurrentFrame()-10);
                }
                else if (cmnd) {
                    autocut = true;
                    autocut_direction = -1;
                    autocut_startframe = fingerMovie.getCurrentFrame();
                    if (caps) {
                        cutter.start(fingerMovie, autocut_direction, autocut_minlength, autocut_threshold);
                    }
                }
                else {
                    fingerMovie.previousFrame();
                }
            break;
            case OF_KEY_RIGHT:
                if (shift) {
                    fingerMovie.setFrame(fingerMovie.getCurrentFrame()+10);
                }
                else if (cmnd) {
                    autocut = true;
                    autocut_direction = 1;
                    autocut_startframe = fingerMovie.getCurrentFrame();
                    if (caps) {
                        cutter.start(fingerMovie, autocut_direction, autocut_minlength, autocut_threshold);
                    }
                }
                else {
                    fingerMovie.nextFrame();
                }
            break;
            case OF_KEY_UP:
                rec=!rec;
                if (rec) {
                    in = fingerMovie.getCurrentFrame();
                    in_f = fingerMovie.getPosition()*fingerMovie.getDuration();
                    out = in;
                }
            break;
            case OF_KEY_DOWN:
                if (fingerMovie.getCurrentFrame()>in && rec) {
                    out = fingerMovie.getCurrentFrame();
                    ffmpeg.addTask(currentFile, in_f, (out - in + 2), json_encoded);
                }
                rec = false;
                break;
            break;
            case ',':
                fingerMovie.setFrame(0);
                if (!cutter.isThreadRunning()) {
                    autocut = false;
                }
                break;
            case '.':
                fingerMovie.setFrame(fingerMovie.getTotalNumFrames()-1);
                if (!cutter.isThreadRunning()) {
                    autocut = false;
                }
                break;
            case OF_KEY_COMMAND:
                cmnd = true;
            break;
        }
    }
   }

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch(key){
        case OF_KEY_SHIFT:
            shift = false;
            break;
        case OF_KEY_COMMAND:
            cmnd = false;
            if (!cutter.isThreadRunning()) {
                autocut = false;
            }
            break;
        case -1:
            caps = false;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    // Add marks if on keyword area
    for(map_vector::iterator mactive = maps.begin(); mactive != maps.end(); ++mactive) {
        cout << "Check: " << mactive->first << endl;
        if (mactive->second.bounds.inside(x,y)) {
            
            if (mactive->second.clearbutton.inside(x,y)) {
                mactive->second.coords.clear();
                cout << "Cleared" << endl;
            }
            else {
                cout << "Added: " << x << "/" << y << endl;
                mactive->second.coords.push_back(ofPoint(100 / mactive->second.bounds.width * (x-mactive->second.bounds.x),100 / mactive->second.bounds.height * (y-mactive->second.bounds.y)));
            }
        }
    }
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    cout << json_encoded << endl;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    if (fingerMovie.isLoaded()) {
        fingerMovie.stop();
        fingerMovie.close();
    }
    currentFile = dragInfo.files[0];
	fingerMovie.loadMovie(currentFile);
    frameByframe=true;
    updateColorSize();
    updateColorPics();

}
