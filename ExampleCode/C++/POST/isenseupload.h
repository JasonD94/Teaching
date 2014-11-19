/*
    Need to make a function that formats the data for upload.
    Then I can test uploading JSON using fields grabbed from a project ID!
*/

#include <iostream>
#include <string>                      // std::string, std::to_string;
#include <curl/curl.h>               // cURL to make HTTP requests
#include "../../../../picojson.h"     // May need to change the path for this if not in git repo
#include <sstream>                 // stringstreams, converting ints to numbers

#include <vector>
using std::vector;

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::stringstream;        // for concating an int onto a string.

// For picojson
using namespace picojson;

// This is from an example program on picojson's github.
// It is used for saving JSON from a server to a temp file.
// See the following URL for an example:
// https://github.com/kazuho/picojson/blob/master/examples/github-issues.cc
typedef struct {
  char* data;   // response data from server
  size_t size;  // response size of data
} MEMFILE;

MEMFILE*
memfopen() {
  MEMFILE* mf = (MEMFILE*) malloc(sizeof(MEMFILE));
  mf->data = NULL;
  mf->size = 0;
  return mf;
}

void
memfclose(MEMFILE* mf) {
  if (mf->data) free(mf->data);
  free(mf);
}

size_t
memfwrite(char* ptr, size_t size, size_t nmemb, void* stream) {
  MEMFILE* mf = (MEMFILE*) stream;
  int block = size * nmemb;
  if (!mf->data)
    mf->data = (char*) malloc(block);
  else
    mf->data = (char*) realloc(mf->data, mf->size + block);
  if (mf->data) {
    memcpy(mf->data + mf->size, ptr, block);
    mf->size += block;
  }
  return block;
}

char*
memfstrdup(MEMFILE* mf) {
  char* buf = (char*)malloc(mf->size + 1);
  memcpy(buf, mf->data, mf->size);
  buf[mf->size] = 0;
  return buf;
}


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
        void set_project_ID(string proj_ID);

        // The user should also set the project title
        void set_project_title(string proj_title);

        // This one is optional, by default the label will be "cURL".
        void set_project_label(string label);

        // As well as the contributor key they will be using
        void set_contributor_key(string contr_key);

        //void GET_PROJ_INFO();                   // this will grab info from the project page and display it, ie "api/v1/projects/PROJECT_ID"
        void GET_PROJ_FIELDS();                 // Given a URL has been set, the fields will be pulled and put into the fields vector.

        // Functions for uploading data to rSENSE
        void set_URL(string s_URL);             // Set the URL for this object
        void Format_Upload_String();          // This formats the upload string
        int POST_JSON_KEY();                      // Post using contributor key (will not worry about Username/Password)

        void DEBUG();   // for debugging, dump all the variables in here.

        string devURL = "http://rsense-dev.cs.uml.edu/api/v1";

    private:

        // Holds the JSON we grab from the given project ID
        value json_data;
        value fields;
        array fields_array;

        // Data for the fields. We'll match them with the given field_array above.
        vector <string> timestamp;
        vector <string> numbers;
        vector <string> text;
        vector <string> latitude;
        vector <string> longitude;


        // Data needed for processing the upload request
        string upload_URL;                         // URL to upload the JSON to
        string get_URL;                              // URL to grab JSON from
        string upload_data;                        // the upload string, in JSON
        string contributor_label;                // Label for the contributor key. by default this is "cURL"
        string contributor_key;                  // contributor key for the project
        string title;                                     // title for the dataset
        string project_ID;                           // project ID of the project
};


iSENSE_Upload::iSENSE_Upload()
{
    // Set these to default values for future references
    upload_URL = "URL";
    get_URL = "URL";
    upload_data = "upload";
    contributor_key = "KEY";
    contributor_label = "LABEL";
    title = "TITLE";
    project_ID = "empty";
}


// This function should be called by the user, and should set up all the fields and what not.
void iSENSE_Upload::set_project_ID(string proj_ID)
{
    project_ID = proj_ID;

    upload_URL = devURL + "/projects/" + project_ID + "/jsonDataUpload";
    get_URL = devURL + "/projects/" + project_ID ;
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

// This is pretty much straight from the GET_curl.cpp file.
void iSENSE_Upload::GET_PROJ_FIELDS()
{
    // Detect errors. We need a valid project ID before we try and perform a GET request.
    if(project_ID == "empty")
    {
        cout << "Error - project ID not set!\n";
        return;
    }

    // This project will try using CURL to make a basic GET request to rSENSE
    // It will then save the JSON it recieves into a picojson object.
    CURL *curl = curl_easy_init();;
    CURLcode res;
    MEMFILE* json_file = memfopen();           // Writing JSON to this file.
    char error[256];                                        // Errors get written here

    // Set the get_URL
    get_URL = "http://rsense-dev.cs.uml.edu/api/v1/projects/" + project_ID;

    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, get_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);    // Write errors to the char array "error"

        // From the picojson example, "github-issues.cc". Used  for writing the JSON to a file.
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memfwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, json_file);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors before trying to use the JSON
        if(res != CURLE_OK)
        {
            // Print out the error
            cout << "Error occured! Error is: ";
            cout << error << endl;

            // Quit so we don't go to the next part.
            return;
        }
         // Good to try and parse the JSON into a PICOJSON object if we get here
        string errors;

        // This will parse the JSON file.
        parse(json_data, json_file->data, json_file->data + json_file->size, &errors);

        // If we have errors, print them out and quit.
        if(errors.empty() != true)
        {
            cout << "Error parsing JSON file!\n";
            cout << "Error was: " << errors;
            return;
        }

        // Save the fields to the field array
        fields = json_data.get("fields");
        fields_array = fields.get<array>();
    }

    // Clean up cURL and close the memfile
    curl_easy_cleanup(curl);
    curl_easy_init();
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
    cout << "Upload URL: " << upload_URL << "\n";
    cout << "GET URL: " << get_URL << "\n\n";
    cout << "Upload Data: " << upload_data << "\n\n";
    cout << "GET Data: " << json_data.serialize() << "\n\n";
    cout << "Field Data: " << fields.serialize() << "\n\n";

    // Print out the field's, along with their type.
    array::iterator it;

    cout << "Printing out all the fields here: \n";

    if(fields.is<picojson::null>() == true)
    {
        return;
    }

    // We made an iterator above, that will let us run through the 3 fields (or how ever many we find) and print them out.
    for(it =fields_array.begin(); it != fields_array.end(); it++)
    {
        // Output all the fields
        object obj = it->get<object>();
        cout << "id: " << obj["id"].to_str();

        /* This part will be important for POSTing. We will want to save the fields and know what type they are.
            If we have a timestamp, number, text, latitude or longitude.
            We can detect this by looking at the "type" value.
        */

        // Grab the  type in number form.
        int type = obj["type"].get<double>();

        // Now we can build a switch statement around this!
        switch(type)
        {
            case 1:
                cout << "\tWe got a timestamp field here!\n";
                break;
            case 2:
                cout << "\tFound a number here!\n";
                break;
            case 3:
                cout << "\tThere's some text over here!\n";
                break;
            case 4:
                cout << "\tLatitude here!\n";
                break;
            case 5:
                cout << "\tLongitude here!\n";
                break;
            default:
                cout << "Error, why'd we get here?\n";
                break;
        }
    }
}