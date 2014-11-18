#include <iostream>
#include <stdio.h>
#include <curl/curl.h>      //CURL to make HTTP requests
#include "../../../../picojson.h"    // May need to change the path for this if not in git repo

using std::cout;
using std::cin;
using std::string;
using std::endl;

int main()
{
    /************************************************************************
        The following actually works! It grabs the project page for project #744
        and displays it to the terminal!

        And the following is off of the cURL webpage. See:
        http://curl.haxx.se/libcurl/c/example.html
    ************************************************************************/

    // This project will try using CURL to make a basic GET request to rSENSE
    // It will then save the JSON it recieves into a picojson object.
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://rsense-dev.cs.uml.edu/api/v1/projects/929");

        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        }
        // Good to try and parse the JSON into a PICOJSON object
        else
        {


        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    cout << endl << endl;       // Add some extra lines on the terminal.

    return 0;
}