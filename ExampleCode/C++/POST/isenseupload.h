#include <iostream>
#include <string>                     // std::string, std::to_string;
#include <curl/curl.h>              // cURL to make HTTP requests
#include "../../../../picojson.h"    // May need to change the path for this if not in git repo
#include <sstream>              // stringstreams, converting ints to numbers

#include <vector>
using std::vector;

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::stringstream;        // for concating an int onto a string.

/*
    At some point this will hold all the upload related stuff for iSENSE uploading in C++
    Probably functions like:

    - Upload with Contributor key
    - Upload with username / password - don't worry about ATM
    - GET JSON of a project given project ID
    - GET Project Fields given project ID
    - POST JSON dataset.
    - POST JSON appending to a dataset
*/

/*

  {
    "title" : "Title for the dataset (string)"
    "contribution_key" : "key created by project owner (string)",
    "contributor_name" : (string),
    "data" :
    {
        "FIELD_ID" : [1,2,3,4,5],
        "FIELD_ID_2" : ["blue","red",,,"green"]
        ...
    }

  }

    Field types:

    1: timestamp
    2: number
    3: string
    4: lat
    5: long

*/

class iSENSE_Upload
{
    public:
        iSENSE_Upload();                             // Default constructor

        // This function should be called by the user, and should set up all the fields and what not.
        void set_project_ID(int proj_ID);

        // The user should also set the project title
        void set_project_title(string proj_title);

        // This one is optional, by default the label will be "cURL".
        void set_project_label(string label);

        // As well as the contributor key they will be using
        void set_contributor_key(string contr_key);

        //void GET_PROJ_INFO();                    // this will grab info from the project page and display it, ie "api/v1/projects/PROJECT_ID"
        void GET_PROJ_FIELDS();                 // Given a URL has been set, the fields will be pulled and put into the fields vector.

        // Functions for uploading data to rSENSE
        void set_URL(string s_URL);             // Set the URL for this object
        void Format_Upload_String();          // This formats the upload string
        int POST_JSON_KEY();                      // Post using contributor key (will not worry about Username/Password)

        void DEBUG();   // for debugging, dump all the variables in here.

        string devURL = "http://rsense-dev.cs.uml.edu/api/v1";

    private:

        // Data that will be formatted. Will need to add functions for this
        vector <string> fields;                 // this will contain all the field IDs

        string timestamp;
        vector <string> numbers;
        vector <string> text;
        string latitude;
        string longitude;


        string upload_URL;                         // URL to upload the JSON to
        string get_URL;                              // URL to grab JSON from
        string upload_data;                        // the upload string, in JSON
        string contributor_label;                // Label for the contributor key. by default this is "cURL"
        string contributor_key;                  // contributor key for the project
        string title;                                     // title for the dataset
        int project_ID;                                // project ID of the project
};


iSENSE_Upload::iSENSE_Upload()
{
    // Set these to default values for future references
    upload_URL = "URL";
    upload_data = "upload";
    contributor_key = "KEY";
    title = "TITLE";
    project_ID = 0;
}


// This function should be called by the user, and should set up all the fields and what not.
void iSENSE_Upload::set_project_ID(int proj_ID)
{
    project_ID = proj_ID;

    // Convert PROJECT ID from an int to a string using stringstreams.
    stringstream num_to_string;                                // First part converts a number (int in this case) to a string
    num_to_string << project_ID;

    upload_URL = devURL + "/projects/" + num_to_string.str() + "/jsonDataUpload";
    get_URL = devURL + "/projects/" + num_to_string.str() ;
}

// The user should also set the project title
void iSENSE_Upload::set_project_title(string proj_title)
{
    title = proj_title;
}

// This one is optional, by default the label will be "cURL".
void iSENSE_Upload::set_project_label(string label)
{
    contributor_label = label;
}

// As well as the contributor key they will be using
void iSENSE_Upload::set_contributor_key(string contr_key)
{
    contributor_key = contr_key;
}


int iSENSE_Upload::POST_JSON_KEY()
{
    /*
    ERROR CHECKING
    Need to make sure that everything has been setup correctly:

    URL should be set
    upload - ???
    label / key should be set to something
    title should be set to something other than "title"
    project id should be set.
    must have also pulled fields for this project

    Assuming we have that info, we can try and make a POST request to rSENSE

    Will need to:
    1. Make URL to upload to, should include devURL and /project/PROJECT ID HERE/jsonDataUpload
    2. Build the JSON we will upload. We can do this assuming the user has done the above steps.
    3. Make a POST request. If we format stuff right we should be good.

    */

    // Set up the POST URL
    //string post_URL = devURL + "/projects/" + project_ID + "/jsonDataUpload";

    // Format the DATA to be uploaded. Call another function to format this.


    // CURL object and response code.
    CURL *curl;
    CURLcode res;

    // In windows, this will init the winsock stuff
    res = curl_global_init(CURL_GLOBAL_DEFAULT);

    // Set the headers to JSON
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");

    // get a curl handle
    curl = curl_easy_init();
    if(curl)
    {
        // Set the URL that we will be using for our POST.
        curl_easy_setopt(curl, CURLOPT_URL, upload_URL.c_str());

        // POST data. Upload will be the string with all the data.
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, upload_data.c_str());

        // JSON Headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Verbose debug output - turn this on if you are having problems. It will spit out a ton of information.
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        cout << "rSENSE says: \n";

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // For cURL return codes, see the following page:
        // http://curl.haxx.se/libcurl/c/libcurl-errors.html
        cout << "\n\ncURL return code was: " << res << endl;

        // Check for errors
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);                // always cleanup
    }

    curl_global_cleanup();
}

void iSENSE_Upload::DEBUG()
{
    /*
        string upload_URL;                         // URL to upload the JSON to
        string get_URL;                              // URL to grab JSON from
        string upload_data;                        // the upload string, in JSON
        string contributor_label;                // Label for the contributor key. by default this is "cURL"
        string contributor_key;                  // contributor key for the project
        string title;                                     // title for the dataset
        int project_ID;                                // project ID of the project
    */

    cout << "PROJECT TITLE = " << title << endl;
    cout << "PROJECT ID = " << project_ID << endl;
    cout << "Contributor Key = " << contributor_key << endl;
    cout << "Contributor Label = " << contributor_label << endl;
    cout << "Upload URL: " << upload_URL << endl;
    cout << "GET URL: " << get_URL << endl;
    cout << "Upload Data: " << upload_data << endl;
}