#ifndef __CUTFIND
#define __CUTFIND


class cutfinder : public ofThread{

private:
    
    bool autocut;
    int autocut_direction, autocut_startframe, autocut_threshold;
    ofVideoPlayer movie;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage    redDiff, redBG,red;
    ofxCvGrayscaleImage    greenDiff, greenBG, green;
    ofxCvGrayscaleImage    blueDiff, blueBG, blue;
    int                    avg;
    ofQTKitPlayer*         player;
    string                 file;

public:
    
    int lastfound;

    cutfinder(){
        movie.setUseTexture(false);
        player = new ofQTKitPlayer();
        ofPtr <ofBaseVideoPlayer> ptr(player);
        movie.setPlayer(ptr);
    }
    
    void start(string _file, int _autocut_direction, int _autocut_threshold, int _autocut_startframe){
        autocut = true;
        if (file != _file) {
            if (movie.isLoaded()) {
                player->close();
                cout << "unload old movie..." << endl;
            }
            player->loadMovie(_file, OF_QTKIT_DECODE_PIXELS_ONLY);
            file = _file;
            cout << "laod new movie movie..." << endl;
        }
        
        
        while (!movie.isLoaded()) {
            ofSleepMillis(100);
            cout << "Wait for Clip being loaded..." << endl;
        }
        autocut_direction = _autocut_direction;
        autocut_threshold = _autocut_threshold;
        autocut_startframe = _autocut_startframe;
        movie.setFrame(autocut_startframe);
        
        colorImg.setUseTexture(false);
        redDiff.setUseTexture(false);
        redBG.setUseTexture(false);
        red.setUseTexture(false);
        greenDiff.setUseTexture(false);
        greenBG.setUseTexture(false);
        green.setUseTexture(false);
        blueDiff.setUseTexture(false);
        blueBG.setUseTexture(false);
        blue.setUseTexture(false);
        startThread();
    }
    
    void updateColorSize(ofVideoPlayer &fingerMovie) {
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
    
    int avgPixel(ofPixelsRef & px) {
        int i = 0;
        int tot = 0;
        while( i < px.size()) {
            tot += px[i];
            i+=5;
        }
        tot /= (px.size()/5);
        return tot;
    }
    
    //
    
    void updateColorPics(ofVideoPlayer &fingerMovie) {
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
    
    
    bool updateSync(ofVideoPlayer &fingerMovie, int _autocut_direction, int _autocut_threshold) {
        autocut_direction  = _autocut_direction;
        autocut_threshold  = _autocut_threshold;
        updateColorSize(fingerMovie);
        return update(fingerMovie);
    }
    
    int state() {
        ofScopedLock lock(mutex);
        return lastfound;
    }
    
    bool update(ofVideoPlayer &fingerMovie){
        
        //do we have a new frame?
        int _f = fingerMovie.getCurrentFrame()+autocut_direction;
        fingerMovie.setFrame(_f);
        fingerMovie.update();
        
        updateColorPics(fingerMovie);
        
        avg = (avgPixel(redDiff.getPixelsRef()) +  avgPixel(greenDiff.getPixelsRef()) + avgPixel(blueDiff.getPixelsRef())) / 3;

        
        if (isThreadRunning() && autocut) {
            lastfound = _f;
            if (avg > autocut_threshold) {
                cout << "FOUND" << lastfound << endl;
                return true;
            }
            if (_f < fingerMovie.getTotalNumFrames()-1) {
                update(fingerMovie);
            }
            else {
                cout << "NO CUT FOUND. REACHED END OF MOVIE" << endl;                
                lastfound = 0;
                return false;
            }
        }
        else {
            if (avg > autocut_threshold) {
                lastfound = _f;
                return true;
            }
            else {
                return false;
            }
        }
    }
    
    void stop(){
        autocut = false;
    }
    

    void threadedFunction(){
        updateColorSize(movie);
        update(movie);
        autocut = false;
    }
    
    
};

#endif












