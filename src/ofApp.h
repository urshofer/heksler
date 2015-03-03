#pragma once

#include "ofMain.h"
//#include "ofQTKitPlayer.h"
#include "ofxVideoSlicer.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxFontStash.h"
#include "cutfinder.h"

class ofApp : public ofBaseApp{

    private:
    
        std::string convertCfString( CFStringRef str )
        {
            char buffer[4096];
            Boolean worked = CFStringGetCString( str, buffer, 4095, kCFStringEncodingUTF8 );
            if( worked ) {
                std::string result( buffer );
                return result;
            }
            else
                return std::string();
        }
    
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
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
        int avgPixel(ofPixelsRef px);
    
        /* VIDEO */
    
		ofVideoPlayer 		fingerMovie;
		bool                frameByframe;
        bool                shift, cmnd, autocut;
        bool                rec, caps;
        float               in_f;
        int                 in, out, autocut_direction, autocut_threshold, autocut_startframe, autocut_minlength;
        string              currentFile;
        ofxVideoSlicer      ffmpeg;
        cutfinder           cutter;

        /* FONT */
    
        ofxFontStash      font, fontlarge;
    
        /* GUI */
    
        void bitrateChanged(int & rate);
        void widthChanged(int & width);
        void acThresholdChanged(int & thresh);
        void acMinChanged(int & thresh);
        void buttonPressed(bool & toggle);
        ofxToggle    transcodeToggle;
        ofxToggle    proresToggle;
        ofxIntSlider bitrateSlider;
        ofxIntSlider widthSlider;
        ofxIntSlider acThresholdSlider;
        ofxIntSlider acMinSlider;
        ofxPanel    gui;
        ofImage     bg;
    
        /* OPENCV */

        void updateColorSize();
        void updateColorPics();

        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage    redDiff, redBG,red;
        ofxCvGrayscaleImage    greenDiff, greenBG, green;
        ofxCvGrayscaleImage    blueDiff, blueBG, blue;
    
        int                    avg;

};
