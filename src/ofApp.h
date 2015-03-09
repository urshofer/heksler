
#pragma once

#include "ofMain.h"
//#include "ofQTKitPlayer.h"
#include "ofxVideoSlicer.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxFontStash.h"
#include "cutfinder.h"
#include "keywordloader.h"

#include <curl/curl.h>
#include "json/json.h"
#include "ofxXmlSettings.h"

#include "Poco/MD5Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"

using Poco::DigestEngine;
using Poco::MD5Engine;
using Poco::DigestOutputStream;
using Poco::StreamCopier;

#include "HttpFormManager.h"

#define __API__ "http://localhost:3000"
#define __USER__ ""
#define __PASS__ ""

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

        static int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
        {
            int result = 0;
            if (buffer != NULL)
            {
                buffer->append(data, size * nmemb);
                result = size * nmemb;
            }
            return result;
        };
    
        std::string curlConnect(string _url, string _post){
            CURL *curl;
            static string buffer;
            buffer.clear();
            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
                
                /* Now specify the POST data */
                if (_post != "") {
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _post.c_str());
                }
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
                curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8" );
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
                curl_easy_perform(curl);
                curl_easy_cleanup(curl);
            }
            return buffer;
        }

   
    struct _keymap
    {
        ofRectangle     bounds;
        ofRectangle     clearbutton;
        std::vector < ofPoint > coords;

    };
    
    typedef std::map<std::string, _keymap > map_vector;
    map_vector 					maps;
    
    
	public:

		void setup();
		void update();
		void draw();
        bool login(string _pass, string _user);
		
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
        void guiSetup();
        void exit();
    
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
        void                onFileProcessed(ofxVideoSlicer::endEvent & ev);

        /* FONT */
    
        ofxFontStash      font, fontlarge, fontmedium;
    
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
        bool        guiVisible;
    
        /* OPENCV */

        void updateColorSize();
        void updateColorPics();
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage    redDiff, redBG,red;
        ofxCvGrayscaleImage    greenDiff, greenBG, green;
        ofxCvGrayscaleImage    blueDiff, blueBG, blue;
        int                    avg;

        /* AUTOMATIC CINEMA STUFF */
        string sessionid, apiurl;
        void         drawKeywords();
        string       json_encoded;
        keywordloader KO;
    
        /* UPLOAD STUFF */
        void newResponse(HttpFormResponse &response);
        HttpFormManager fm;
    
};


