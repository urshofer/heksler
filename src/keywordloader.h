#ifndef __WORDLOAD
#define __WORDLOAD
#include "json/json.h"
#include <curl/curl.h>


class keywordloader : public ofThread{

private:
    
    string apiurl;
    string sessionid;
    Json::Value  keywords;
    bool loaded;
    
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
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        return buffer;
    }

public:

    
    
    keywordloader(){
    }
    
    void start(string _apiurl, string _sessionid){
        apiurl = _apiurl;
        sessionid = _sessionid;
        loaded = false;
        startThread();
    }
    
    Json::Value getKeywords() {
        Json::Value _kw;
        if (lock()) {
            _kw = keywords;
            unlock();
        }
        return _kw;
    }
    
    void stop(){
        waitForThread();
    }
    

    void threadedFunction(){
        while (isThreadRunning()) {
            Json::Reader getdata;
//            cout << "Trying to load keywords..." << apiurl << "/Load/" << sessionid << "/target" << endl;
            Json::Value _kw;
            if (getdata.parse(curlConnect(apiurl + "/Load/" + sessionid + "/target", ""), _kw )) {
//                cout << "Loaded keywords..." << endl;
                if (lock()) {
                    keywords = _kw;
                    unlock();
                }
                loaded = true;
            }
            ofSleepMillis(5000);
            
        }
    }
    
    
};

#endif












