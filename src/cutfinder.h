#ifndef __CUTFIND
#define __CUTFIND


class cutfinder : public ofThread{

private:
    
    bool autocut;
    int autocut_direction, autocut_startframe, autocut_minlength, autocut_threshold;
    ofVideoPlayer fingerMovie;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage    redDiff, redBG,red;
    ofxCvGrayscaleImage    greenDiff, greenBG, green;
    ofxCvGrayscaleImage    blueDiff, blueBG, blue;
    int                    avg;

public:

    cutfinder(){
    }
    
    void start(ofVideoPlayer &movie, int _autocut_direction, int _autocut_minlength, int _autocut_threshold){
        autocut = true;
        fingerMovie = movie;
        if (!fingerMovie.isLoaded()) {
            return;
        }
        autocut_direction = _autocut_direction;
        autocut_minlength = _autocut_minlength;
        autocut_threshold = _autocut_threshold;
        
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
        
        
        autocut_startframe = fingerMovie.getCurrentFrame();
        startThread();
    }
    
    void updateColorSize() {
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
    
    void updateColorPics() {
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
    
    
    bool updateSync(ofVideoPlayer &movie, int _autocut_direction, int _autocut_minlength, int _autocut_threshold, int _autocut_startframe) {
        fingerMovie = movie;
        autocut_startframe = _autocut_startframe;
        autocut_direction  = _autocut_direction;
        autocut_minlength  = _autocut_minlength;
        autocut_threshold  = _autocut_threshold;
        updateColorSize();
        return update();
    }
    
    bool update(){
        
        //do we have a new frame?

        fingerMovie.setFrame(fingerMovie.getCurrentFrame()+autocut_direction);
        fingerMovie.update();
        
        updateColorPics();
        
        avg = (avgPixel(redDiff.getPixelsRef()) +  avgPixel(greenDiff.getPixelsRef()) + avgPixel(blueDiff.getPixelsRef())) / 3;

        
        if (avg > autocut_threshold && fingerMovie.getCurrentFrame() - autocut_startframe > autocut_minlength) {
            return true;
        }
        
        if (isThreadRunning() && autocut && fingerMovie.getCurrentFrame()<fingerMovie.getTotalNumFrames()-1) {
            update();
        }
        else {
            return false;
        }
            
    }
    
    void stop(){
    }
    

    void threadedFunction(){
        fingerMovie.setUseTexture(false);
        updateColorSize();
        update();
        fingerMovie.setUseTexture(true);
    }
    
    
};

#endif












