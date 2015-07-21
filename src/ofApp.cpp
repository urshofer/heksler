
#include "ofApp.h"

void ofApp::guiSetup() {
    bitrateSlider.addListener(this, &ofApp::bitrateChanged);
	widthSlider.addListener(this, &ofApp::widthChanged);
    acThresholdSlider.addListener(this, &ofApp::acThresholdChanged);
    acMinSlider.addListener(this, &ofApp::acMinChanged);
    acMaxSlider.addListener(this, &ofApp::acMaxChanged);
    
    transcodeToggle.addListener(this,&ofApp::buttonPressed);
    proresToggle.addListener(this,&ofApp::buttonPressed);
    autotaggerToggle.addListener(this,&ofApp::autotaggerPressed);
    quietToggle.addListener(this,&ofApp::buttonPressed);
    
	gui.setup("Settings","gui.xml",10,10); // most of the time you don't need a name

    gui.loadFont("Roboto-Gui.ttf", 10);
    font.loadFont("Roboto-Gui.ttf", 15);
    fontlarge.loadFont("Roboto-Gui.ttf", 45);
    fontmedium.loadFont("Roboto-Gui.ttf", 30);
    
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
    gui.setDefaultWidth(348);
    
    gui.setPosition(667,33);
    
    transcodeToggle.setFillColor(ofColor(70,93,109));
    transcodeToggle.setTextColor(ofColor(255));
    transcodeToggle.setBackgroundColor(ofColor(16,22,26));
    
    proresToggle.setFillColor(ofColor(70,93,109));
    proresToggle.setTextColor(ofColor(255));
    proresToggle.setBackgroundColor(ofColor(16,22,26));

    autotaggerToggle.setFillColor(ofColor(70,93,109));
    autotaggerToggle.setTextColor(ofColor(255));
    autotaggerToggle.setBackgroundColor(ofColor(16,22,26));

    quietToggle.setFillColor(ofColor(70,93,109));
    quietToggle.setTextColor(ofColor(255));
    quietToggle.setBackgroundColor(ofColor(16,22,26));
    
    bitrateSlider.setFillColor(ofColor(70,93,109));
    bitrateSlider.setTextColor(ofColor(255));
    bitrateSlider.setBackgroundColor(ofColor(16,22,26));
    
    widthSlider.setFillColor(ofColor(70,93,109));
    widthSlider.setTextColor(ofColor(255));
    widthSlider.setBackgroundColor(ofColor(16,22,26));
    
    acThresholdSlider.setFillColor(ofColor(70,93,109));
    acThresholdSlider.setTextColor(ofColor(255));
    acThresholdSlider.setBackgroundColor(ofColor(16,22,26));
    
    acMinSlider.setFillColor(ofColor(70,93,109));
    acMinSlider.setTextColor(ofColor(255));
    acMinSlider.setBackgroundColor(ofColor(16,22,26));

    acMaxSlider.setFillColor(ofColor(70,93,109));
    acMaxSlider.setTextColor(ofColor(255));
    acMaxSlider.setBackgroundColor(ofColor(16,22,26));
    
    gui.add(bitrateSlider.setup("Output bitrate", 500, 100, 5000));
    gui.add(widthSlider.setup("Output width", 640, 100, 2000));
    
	gui.add(transcodeToggle.setup("Transcode", true));
	gui.add(proresToggle.setup("Prores enabled (H264 standard)", false));
	gui.add(quietToggle.setup("Export Audio", true));
    
    gui.add(acThresholdSlider.setup("Autocut threshold", 120, 0, 255));
    gui.add(acMinSlider.setup("Autocut minimum frames", 10, 10, 250));
    gui.add(acMaxSlider.setup("Autocut maximum frames", 125, 10, 250));
    
	gui.add(autotaggerToggle.setup("Autotagging", true));
    gui.loadFromFile("gui.xml");

}

//--------------------------------------------------------------
void ofApp::setup(){
    
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources/data/");
#endif

    autotagger = true;
    
    /* XML Settings */
	XML.loadFile("serversettings.xml");
	string user	= XML.getValue("USER", __USER__);
	string pass	= XML.getValue("PASS", __PASS__);
	apiurl	= XML.getValue("URL", __API__);
    cs_api = XML.getValue("CSAPI", "");
    autotagger_path = XML.getValue("AT_PATH", "");
    autotagger_file = XML.getValue("AT_FILE", "");
    autotagger_nlp = XML.getValue("AT_NLP", "");
    
    
	ofBackground(19,30,37);
    ofEnableAlphaBlending();
    frameByframe=false;
    shift=false;
    caps = false;
    autocut_threshold = 150;
    autocut_minlength = 20;
    autocut_maxlength = 200;

    in = -1;
    out = -1;
    
    ofSetFrameRate(100);
    guiVisible = false;

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
    
	/* Logging in at the API */
    
    while (!login(user, pass)) {
        cout << "Could not connect to API Server and Log In\n";
        apiurl = ofSystemTextBoxDialog("Connecting to server.\nPlease type the URL:", apiurl);
        user = ofSystemTextBoxDialog("Username:", user);
        pass = ofSystemTextBoxDialog("Password:", pass);
        XML.setValue("USER", user);
        XML.setValue("PASS", pass);
        XML.setValue("URL", apiurl);
        XML.saveFile();
    }

    KO.start(apiurl, sessionid);

    
}

bool ofApp::login(string _pass, string _user) {
    MD5Engine md5;
    md5.update(_pass);
    Json::Reader getdata;
    Json::Value  returnval;
	if (getdata.parse( curlConnect(apiurl + "/Login", "username=" + _user + "&password=" + DigestEngine::digestToHex(md5.digest()) ), returnval )) {
        sessionid = returnval.asString();
        cout << "Session ID: " << sessionid << endl;
        return true;
    }
	else {
        return false;
    }
}



void ofApp::onFileProcessed(ofxVideoSlicer::endEvent & ev) {
    
    cout << "[UPLOAD] starting" << endl;

    
    /* UPLOAD STUFF */
    HttpFormManager fm, cf;
    cf.setHeader("Authorization", cs_api);
    fm.setTimeOut(60);
    
	HttpForm f = HttpForm( apiurl + "/StoreFileAnnotaded/" + sessionid );
	f.addFile("url", ev.file, "movie/mpeg");
    f.addFormField("meta", ev.message);

    cout << "[UPLOAD] autotagger init" << endl;
    long init_time = ofGetElapsedTimeMillis();
    
    if (autotagger) {
        // Loading Tags from CloudSight by analyzing ev.jpg
        //ev.jpg
        HttpForm fc = HttpForm( "http://api.cloudsightapi.com/image_requests" );
        fc.addFile("image_request[image]", ev.jpg, "image/jpeg");
        fc.addFormField("image_request[locale]", "en-EN");
        HttpFormResponse response = cf.submitFormBlocking( fc );
//        printf("form '%s' returned : %s\n", response.url.c_str(), response.responseBody.c_str() );
        
        Json::Reader getdata;
        Json::Value _kw;
        getdata.parse(response.responseBody, _kw );
        // Get JSON Token
        if (_kw["token"].asString() != "") {
            Json::Reader getanalysis;
            Json::Value _analysis;

            // Poll Cloudsight until Response is okay

            do {
                getanalysis.parse(curlConnect(
                    "https://api.cloudsightapi.com/image_responses/" + _kw["token"].asString(),
                    "",
                    "Authorization",
                    cs_api
                    ), _analysis
                );
                if (_analysis["status"].asString() == "not completed") {
                    ofSleepMillis(1000);
                }
            } while (_analysis["status"].asString() == "not completed");

            // Skipped, i.e. offensive
            string nlp_string = "";
            if (_analysis["status"].asString() == "skipped") {
 //               cout << "SKIPPED:" << _analysis["reason"].asString() << endl;
                nlp_string = _analysis["reason"].asString();
            }
            if (_analysis["status"].asString() == "completed") {
   //             cout << "KEYWORDS:" << _analysis["name"].asString() << endl;
                nlp_string = _analysis["name"].asString();
            }
            nlp_string = "{\"dimension\": \"" + autotagger_nlp + "\",\"string\": \"" + nlp_string + "\"}";
   //         cout << nlp_string << endl;
            f.addFormField("nlp", nlp_string);
        }
    }

    cout << "[UPLOAD] autotagger done (" << ofToString(ofGetElapsedTimeMillis() - init_time) << ")" << endl;
    
    // Upload to Server
	HttpFormResponse response = fm.submitFormBlocking( f );
	printf("[UPLOAD] stored: %s\n         Status: %s\n         took:%s", response.url.c_str(), response.ok ? "OK" : "KO", ofToString(ofGetElapsedTimeMillis() - init_time).c_str() );
}



//--------------------------------------------------------------
void ofApp::autotaggerPressed(bool & toggle) {
	/* Autotagger Stuff */
    if (!autotagger) {
        cs_api = ofSystemTextBoxDialog("Api Key for CloudSight\nRecognition API:", cs_api);
        autotagger_path = ofSystemTextBoxDialog("Dimension for /Path/Parts/\nAuto Tagging:", autotagger_path);
        autotagger_file = ofSystemTextBoxDialog("Dimension for _File_Name_\nAuto Tagging:", autotagger_file);
        autotagger_nlp = ofSystemTextBoxDialog("Dimension for\nNatural Language Processing:", autotagger_nlp);
        XML.setValue("CSAPI", cs_api);
        XML.setValue("AT_PATH", autotagger_path);
        XML.setValue("AT_FILE", autotagger_file);
        XML.setValue("AT_NLP", autotagger_nlp);
        XML.saveFile();
    }
    
    autotagger = autotaggerToggle;
    gui.saveToFile(ofToDataPath("gui.xml"));
    
}


//--------------------------------------------------------------
void ofApp::buttonPressed(bool & toggle) {
    ffmpeg.setCodec(proresToggle ? "prores" : "h264");
    ffmpeg.setTranscode(transcodeToggle);
    ffmpeg.setAudio(quietToggle);
    cout << "Transcode: " << transcodeToggle << " Prores: " << proresToggle << endl;
    gui.saveToFile(ofToDataPath("gui.xml"));
}

//--------------------------------------------------------------
void ofApp::acMinChanged(int & min){
    autocut_minlength = min;
    if (min > autocut_maxlength) {
        acMaxChanged(min);
    }
    gui.saveToFile(ofToDataPath("gui.xml"));
}

//--------------------------------------------------------------
void ofApp::acMaxChanged(int & max){
    autocut_maxlength = max;
    gui.saveToFile(ofToDataPath("gui.xml"));
}

//--------------------------------------------------------------
void ofApp::acThresholdChanged(int & thresh){
    autocut_threshold = thresh;
    gui.saveToFile(ofToDataPath("gui.xml"));
}

//--------------------------------------------------------------
void ofApp::bitrateChanged(int & rate){
    ffmpeg.setBitrate(rate);
    gui.saveToFile(ofToDataPath("gui.xml"));
}

//--------------------------------------------------------------
void ofApp::widthChanged(int & width){
    
    // Round width
    int targetH = fingerMovie.getHeight() / fingerMovie.getWidth() * width;
    ffmpeg.setSize(width, targetH);
    gui.saveToFile(ofToDataPath("gui.xml"));
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
    if (fingerMovie.isLoaded()) {
        if (caps) {
            if (!cutter.isThreadRunning() && autocut){
                if (cutter.lastfound != 0) {
                    int _diff = abs(cutter.lastfound - in);
                    fingerMovie.setFrame(cutter.lastfound);
                    
                    // Set In Point: If not set or diff to small or too big
                    
                    if (in == -1 || _diff > autocut_maxlength || _diff < autocut_minlength) {
                        in = cutter.lastfound;
                        in_f = fingerMovie.getPosition()*fingerMovie.getDuration();
                        out = -1;

                    }
                    
                    // Slice Complete: Could send to slicer here.
                    
                    else {
                        out = cutter.lastfound;
                        // HEKSEL
                        if (autocut_maxlength > (out - in)) {
                            cout << "Start FFMPEG Task @" << in << " to " << out << endl;
                            ffmpeg.addTask(currentFile, in_f, (out - in), json_encoded);
                        }
                        // Set out to new in
                        in = out;
                        in_f = fingerMovie.getPosition()*fingerMovie.getDuration();
                    }
                    // ReStart Task
                    cout << "Restart Cutter Task @" << in << endl;
                    cutter.start(fingerMovie.getMoviePath(), 1, autocut_threshold, in);

                }
                else {
                    // Returns true if there's a movie loaded, so autocut will continue
                    autocut = loadMovie();
                }
            }
        }
        else {
            // Regular Movie Update if not done by the autocutter
            if (!cmnd) {
                fingerMovie.update();
                if (fingerMovie.isFrameNew()) {
                    updateColorPics();
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetHexColor(0x000000);
    ofRect(10,60,640,480);
    ofRect(665,60,200,150);
    ofRect(665,60+165,200,150);
    ofRect(665,60+330,200,150);

    ofSetHexColor(0xFFFFFF);
    font.drawString("Drag Video here", 25+(640/2)-(124/2), 58+240);

    if (!caps){
        if (fingerMovie.isLoaded()) {
            fingerMovie.draw(10,60 + ((480 - (640 / fingerMovie.getWidth() * fingerMovie.getHeight()))/2), 640, 640 / fingerMovie.getWidth() * fingerMovie.getHeight());
            ofSetHexColor(0xFF0000);
            float scaledH = 200.0f / (float)redDiff.width * redDiff.height;
            float centerV = (150 - scaledH) / 2;
            redDiff.draw(665,60 + centerV,250,scaledH);
            ofSetHexColor(0x00FF00);
            greenDiff.draw(665,60+165 + centerV,250, scaledH);
            ofSetHexColor(0x0000FF);
            blueDiff.draw(665,60+330 + centerV,250, scaledH);
        }
    }
    else {
        if (cutter.isThreadRunning()) {
            ofSetHexColor(0x333333);
            ofRect(10,60,640,480);
            ofSetHexColor(0xFFFFFF);
            font.drawString("Searching next cut", 25+(640/2)-(124/2), 58+240);
            font.drawString(ofToString(cutter.state()), 25+(640/2)-(124/2), 58+260);
        }
        ofPushStyle();
        ofSetLineWidth(4);
        ofNoFill();
        ofSetHexColor(0xFF0000);
        ofRect(10,60,640,480);
        ofPopStyle();
    }
    
    ofPushStyle();
        ofSetColor(255, 255, 255, 255);
        bg.draw(0,0);

        ofSetHexColor(0x222222);
        int zeile = 77; int zeileoffset = 69;

        font.drawString("Current Frame",888,zeile);
        fontlarge.drawString(ofToString(fingerMovie.getCurrentFrame()),888,zeile+37);
        zeile += zeileoffset;
        font.drawString("Total Frames",888,zeile);
        fontlarge.drawString(ofToString(fingerMovie.getTotalNumFrames()),888,zeile+37);
        zeile += zeileoffset;
        font.drawString("Position",888,zeile);
        fontlarge.drawString(ofToString(fingerMovie.getPosition()*fingerMovie.getDuration(),1),888,zeile+37);
        zeile += zeileoffset;
        font.drawString("Duration",888,zeile);
        fontlarge.drawString(ofToString(fingerMovie.getDuration(),1),888,zeile+37);
        zeile += zeileoffset;
        font.drawString("In",888,zeile);
        font.drawString("Out",888,zeile+zeileoffset);
        fontlarge.drawString(ofToString(in),888,zeile+37);
        fontlarge.drawString(ofToString(out),888,zeile+zeileoffset+37);
        zeile += (zeileoffset * 2);
        font.drawString("Pending" ,888, zeile);
        fontlarge.drawString(ofToString(ffmpeg.processingQueueSize()),888,zeile+37);
    ofPopStyle();

    if (in >= 0) {
        ofPushStyle();
        ofSetColor(255, 255, 255, 255);
        img_in.draw(20, 485 - img_in.getHeight()/5, img_in.getWidth()/5, img_in.getHeight()/5);
        ofSetHexColor(0xffffff);
        ofSetLineWidth(2);
        ofNoFill();
        ofRect(20, 485 - img_in.getHeight()/5, img_in.getWidth()/5, img_in.getHeight()/5);
        ofPopStyle();
    }
    if (out >= 0) {
        ofPushStyle();
        ofSetColor(255, 255, 255, 255);
        img_out.draw(650 - 10 - (img_out.getWidth()/5) , 485 - img_out.getHeight()/5, img_out.getWidth()/5, img_out.getHeight()/5);
        ofSetHexColor(0xffffff);
        ofSetLineWidth(2);
        ofNoFill();
        ofRect(650 - 10 - (img_out.getWidth()/5) , 485 - img_out.getHeight()/5, img_out.getWidth()/5, img_out.getHeight()/5);
        ofPopStyle();
    }

    if (rec) {
        ofPushStyle();
        ofSetColor(0, 0, 0, 128);
        ofRect(158, 485 - img_out.getHeight()/5, 345, img_out.getHeight()/5);
        ofSetColor(0, 255, 0, 255);
        fontlarge.drawString("Press Enter to store",160,485 - img_out.getHeight()/5 + 50);
        ofPopStyle();
    }
    
    

    drawKeywords();
    if (guiVisible) {
        ofSetHexColor(0x10161a);
        ofRect(657,49,367,500);
        gui.draw();
    }
    
}

void ofApp::drawKeywords() {
    ofPushStyle();
    
    float sizeX = 244;
    float sizeY = 194;
    float startX = 13;
    float startY = 561;
    float offsetX = 251;
    float offsetY = 0;
    float dimCount = 0;
    json_encoded = "{";
    if (KO.isThreadRunning()) {
        Json::Value keywords = KO.getKeywords();
        for( Json::ValueIterator itr = keywords.begin() ; itr != keywords.end() ; itr++ ) {
            string dim = itr.key().asString();
            if (dimCount>0) {
                json_encoded += ",";
            }
            json_encoded += "\""+dim+"\": [";
            float x = startX+(dimCount*offsetX);
            float y = startY+(dimCount*offsetY);
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
            ofRectangle clearbutton = font.getStringBoundingBox("Clear", 0, 0);
            ofSetHexColor(0x992222);
            ofRect(x+sizeX-7-clearbutton.width, y+3, clearbutton.width+4, clearbutton.height+4);
            ofSetHexColor(0xFF0000);
            font.drawString("Clear", x+sizeX-5-clearbutton.width, y+15);
            int coordCount = 0;
			if(mactive != maps.end())
			{
                for(std::vector< ofPoint >::const_iterator qactive = mactive->second.coords.begin(); qactive != mactive->second.coords.end(); ++qactive) {
                    ofCircle(x+(sizeX/100*qactive->x)-3, y+(sizeY/100*qactive->y)-3, 6);
                    if (coordCount>0) {
                        json_encoded += ",";
                    }
                    json_encoded += "[\"" + ofToString(qactive->x) + "\",\"" + ofToString(qactive->y) + "\"]";
                    coordCount++;
                }
			}
            else {
                _keymap _k;
                std::vector < _keywords > _kw;
                
                for( Json::ValueIterator element = keywords[dim]["Keywords"].begin() ; element != keywords[dim]["Keywords"].end() ; element++ ) {
                    _keywords __kw;
                    __kw.first = element.key().asString();
                    __kw.second = ofPoint(
                                    ofToFloat(keywords[dim]["Keywords"][__kw.first][0].asString()),
                                    ofToFloat(keywords[dim]["Keywords"][__kw.first][1].asString())
                    );
                    _kw.push_back(__kw);
                }
                
                
                
                _k.bounds = ofRectangle(x, y,sizeX,sizeY);
                _k.clearbutton = font.getStringBoundingBox("Clear", x+sizeX-5-clearbutton.width, y+15);
                _k.clearbutton.width  += 4;
                _k.clearbutton.height += 4;
                _k.clearbutton.x -= 2;
                _k.clearbutton.y -= 2;
                _k.coords.clear();
                _k.keywords = _kw;
                        
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
            case OF_KEY_TAB:
                caps = !caps;
                break;
            case OF_KEY_SHIFT:
                shift = true;
                break;
            case OF_KEY_COMMAND:
                cmnd = true;
            case '1':
                if (in >= 0) fingerMovie.setFrame(in);
                break;
            case '2':
                if (out >= 0) fingerMovie.setFrame(out);
                break;
            case ' ':
                frameByframe=!frameByframe;
                fingerMovie.setPaused(frameByframe);
            break;
            case OF_KEY_LEFT:
                if (shift) {
                    fingerMovie.setFrame(fingerMovie.getCurrentFrame()-10);
                }
                else if (cmnd && !autocut_found) {
                    autocut_found = cutter.updateSync(fingerMovie, -1, autocut_threshold);
                    updateColorPics();
                }
                else if (!cmnd) {
                    fingerMovie.previousFrame();
                }
            break;
            case OF_KEY_RIGHT:
                if (shift) {
                    fingerMovie.setFrame(fingerMovie.getCurrentFrame()+10);
                }
                else if (cmnd && !autocut_found) {
                    autocut_found = cutter.updateSync(fingerMovie, 1, autocut_threshold);
                    updateColorPics();
                }
                else if (!cmnd)  {
                    fingerMovie.nextFrame();
                }
            break;
            case OF_KEY_UP:
                in = fingerMovie.getCurrentFrame();
                img_in.loadData(fingerMovie.getPixels(), fingerMovie.getWidth(), fingerMovie.getHeight(), GL_RGB);
                in_f = fingerMovie.getPosition()*fingerMovie.getDuration();
                rec = out > in;
            break;
            case OF_KEY_DOWN:
                out = fingerMovie.getCurrentFrame();
                img_out.loadData(fingerMovie.getPixels(), fingerMovie.getWidth(), fingerMovie.getHeight(), GL_RGB);
                rec = out > in;
                break;
            break;
            case OF_KEY_RETURN:
                if (caps) {
                    autocut = true;
                    cout << "Start Cutter Task @0" << endl;
                    cutter.start(fingerMovie.getMoviePath(), 1, autocut_threshold, 0);
                }
                else if (rec) {
                    ffmpeg.addTask(currentFile, in_f, (out - in), json_encoded);
                    rec = false;
                    out = in = 0;
                }
                break;
            case ',':
                fingerMovie.setFrame(0);
                break;
            case '.':
                fingerMovie.setFrame(fingerMovie.getTotalNumFrames()-1);
                break;
            break;
        }
    }
    else {
        switch(key) {
            case OF_KEY_RETURN:
                if (caps) {
                    cout << "Stop Cutter Task" << endl;
                    autocut = false;
                    cutter.stop();
                }
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
            autocut_found = false;
            break;
        case OF_KEY_LEFT:
        case OF_KEY_RIGHT:
            autocut_found = false;
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
        //cout << "Check: " << mactive->first << endl;
        
        bool _found = false;
        std::vector< ofPoint >::iterator qactive = mactive->second.coords.begin();
        while (qactive != mactive->second.coords.end()) {

            ofRectangle _test(mactive->second.bounds.x + (mactive->second.bounds.width / 100 * qactive->x) - 6, mactive->second.bounds.y + (mactive->second.bounds.height / 100 * qactive->y) - 6, 12, 12);

            
            
            cout << _test.x << "/" << _test.y << " - " << x << "/" << y << endl;
            
            if(_test.inside(x,y))
            {
                // erase returns the new iterator
              qactive = mactive->second.coords.erase(qactive);
                _found = true;
                cout << "----------------FOUND-----------------" <<  endl;
            }
            else
            {
                ++qactive;
            }

        }
        
        if (mactive->second.bounds.inside(x,y)) {
            
            if (mactive->second.clearbutton.inside(x,y)) {
                mactive->second.coords.clear();
                cout << "Cleared" << endl;
            }
            else if (!_found){
                cout << "Added: " << x << "/" << y << endl;
                mactive->second.coords.push_back(ofPoint(100 / mactive->second.bounds.width * (x-mactive->second.bounds.x),100 / mactive->second.bounds.height * (y-mactive->second.bounds.y)));
            }
        }
    }
    // Toggle GUI
    if (ofRectangle(660,0,364,47).inside(x,y)) {
        guiVisible = !guiVisible;
    }

    
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
//    cout << json_encoded << endl;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    files = dragInfo.files;
    fileCount = 0;
    loadMovie();
}

bool ofApp::loadMovie() {
    if (fingerMovie.isLoaded()) {
        fingerMovie.stop();
        fingerMovie.close();
        img_out.clear();
        img_out.allocate(1,1,GL_RGB);
        img_in.clear();
        img_in.allocate(1,1,GL_RGB);
    }
    if (fileCount < files.size()) {
        currentFile = files[fileCount];
        ofFile file(ofToDataPath(currentFile));
        bool isMovie = (file.getExtension() == "mov" || file.getExtension() == "mp4" || file.getExtension() == "avi" || file.getExtension() == "m4v");

        fingerMovie.loadMovie(currentFile);
        frameByframe=true;
        if (isMovie) {
            updateColorSize();
            updateColorPics();
        }
        in = -1;
        out = -1;
        
        if (autotagger) {
            // ReSet KEywords
            for(map_vector::iterator mactive = maps.begin(); mactive != maps.end(); ++mactive) {
                mactive->second.coords.clear();
            }
            // Set Path Words
            std::vector<string> pathinfo = ofSplitString(currentFile, "/");
            string fName = pathinfo[pathinfo.size()-1];
            pathinfo.pop_back();
            syncKeywords(pathinfo, autotagger_path);
            
            // Set File Name Words
            std::vector<string> fileinfo = ofSplitString(fName, "_");
            fileinfo.pop_back();
            syncKeywords(fileinfo, autotagger_file);
        }
        
        // Movies: Directly Autocut if set
        
        
        if (isMovie && caps && !cutter.isThreadRunning()) {
            cout << "Import VIDEO" << endl;
            autocut = true;
            cutter.start(fingerMovie.getMoviePath(), 1, autocut_threshold, 0);
        }
        
        fileCount++;

        if (!isMovie && caps) {
            cout << "Import AUDIO" << endl;
            drawKeywords();
            directUpload(currentFile, json_encoded);
            loadMovie();
        }
        
        return true;
    }
    return false;
}

void ofApp::directUpload(string file, string json) {
    /* UPLOAD STUFF */
    HttpFormManager fm;
    fm.setTimeOut(60);
	HttpForm f = HttpForm( apiurl + "/StoreFileAnnotaded/" + sessionid );
	f.addFile("url", file, "movie/mpeg");
    f.addFormField("meta", json);
    // Upload to Server
	HttpFormResponse response = fm.submitFormBlocking( f );
	printf("Automatic Cinema Server\nStored : %s\nrStatus : %s\n", response.url.c_str(), response.ok ? "OK" : "KO" );
}



void ofApp::syncKeywords(std::vector<string> words, string dimension) {
    for(std::vector<string>::iterator word = words.begin(); word != words.end(); ++word) {
        cout << "Lookup: " << *word << endl;
        // Check if Word exists in Keywords, then add the corresponding point in the coordinates
        
        for(map_vector::iterator mactive = maps.begin(); mactive != maps.end(); ++mactive) {
            for(std::vector < _keywords >::iterator kactive = mactive->second.keywords.begin(); kactive != mactive->second.keywords.end(); ++kactive) {
                if (kactive->first == *word && dimension == mactive->first) {
                    cout << "Found: " << kactive->first << endl;
                    mactive->second.coords.push_back(kactive->second);
                }
            }
        }
    }
}

void ofApp::exit() {
	std::cout  << "+ Exiting now" << endl;
	if (KO.isThreadRunning()) {
		KO.stopThread();
        KO.waitForThread();
        std::cout  << "- Keyword Loader Thread Stopped" << endl;
    }
	if (cutter.isThreadRunning()) {
		cutter.stopThread();
        cutter.waitForThread();
        std::cout  << "- Autocutter Thread Stopped" << endl;
    }
	std::cout  << "Bye Bye!" << endl;
    std::exit(0);
}

