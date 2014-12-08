#include <iostream>
#include <string>                // std::string, std::to_string;
#include <ctime>                // Timestamps
#include <vector>
#include <map>
#include <curl/curl.h>          // cURL to make HTTP requests
#include "picojson.h"            // picojson for usin JSON easily.
#include "memfile.h"            // picojson/curl uses this for temp files

// To avoid poluting the namespace, and also to avoid typing std:: everywhere.
using std::vector;
using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::to_string;

// For picojson
using namespace picojson;

const string devURL = "http://rsense-dev.cs.uml.edu/api/v1";
const string liveURL = "http://isenseproject.org/api/v1";

class iSENSE_Upload
{
    public:
        iSENSE_Upload();                                                // Default constructor
        iSENSE_Upload(string proj_ID, string proj_title,     // Contructor with parameters.
                                string label, string contr_key);

        // Similar to the constructor with parameters.
        void set_project_all(string proj_ID, string proj_title,
                                      string label, string contr_key);

        void set_project_ID(string proj_ID);                      // This function should set up all the fields
        void set_project_title(string proj_title);                // The user should also set the project title
        void set_project_label(string label);                    // This one is optional, by default the label will be "cURL".
        void set_contributor_key(string contr_key);         // User needs to set the contributor key they will be using

        // These functions will push data back to the vectors.
        void timestamp_pushback(string new_timestamp);
        void generate_timestamp(void);
        void numbers_pushback(string new_numbers);
        void text_pushback(string new_text);
        void latitude_pushback(string new_latitude);
        void longitude_pushback(string new_longitude);

        // Helper functions for uploading data to rSENSE
        void set_URL(string s_URL);             // Set the URL for this object
        void format_upload_string();            // This formats the upload string

        void format_data(vector<string> *vect,      // This formats one FIELD ID : DATA pairs
                                  array::iterator it);

        // iSENSE API functions
        void GET_PROJ_FIELDS();                  // Given a URL has been set, the fields will be pulled and put into the fields vector.
        void POST_JSON_KEY();                    // Post using contributor key

        /*  Future functions to be implemented at a later date.
        void POST_JSON_USER();                  // Post using a username / password
        void POST_APPEND_KEY();
        void POST_APPEND_USER();
        void POST_EDIT_KEY();
        void POST_EDIT_USER();
        void POST_FIELDS();
        void POST_PROJECTS();
        // is it possible to post media objects? or even bother?

        void GET_USER();                            // Check's if a username / password is valid
        void GET_PROJECTS(string search_term);
        void GET_DATASET_ID();
        void GET_FIELDS_ID();
        */

        // for debugging, dump all the variables in here.
        void DEBUG();

    private:

        // Holds the JSON we grab from the given project ID
        object upload_data;                     // the upload string, in JSON
        object fields_data;                       // the "data" object, with Field ID: [DATA HERE]

        // See if these are needed. At least one or two is.
        value json_data;
        value fields;
        array fields_array;

        // Data for the fields. We'll match them with the given field_array above.
        vector <string> timestamp;
        vector <string> numbers;              // These two should become vector of vectors of strings.
        vector <string> text;                     // That way multiple number / text fields won't cause issues when pulling fields/uploading.
                                                            // They should also probably be maps so that
        vector <string> latitude;
        vector <string> longitude;

        // Data needed for processing the upload request
        bool usingDev;                            // Whether the user wants iSENSE or rSENSE
        string upload_URL;                      // URL to upload the JSON to
        string get_URL;                           // URL to grab JSON from
        string contributor_label;              // Label for the contributor key. by default this is "cURL"
        string contributor_key;                // contributor key for the project
        string title;                                 // title for the dataset
        string project_ID;                        // project ID of the project
};


// Default constructor
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


// Constructor with parameters
iSENSE_Upload::iSENSE_Upload(string proj_ID, string proj_title,     // Contructor with parameters.
                                                string label, string contr_key)
{
    set_project_ID(proj_ID);
    set_project_title(proj_title);
    set_project_label(label);
    set_contributor_key(contr_key);
}

// Similar to the constructor with parameters, but can be called at anytime to
// set up the upload object.
void set_project_all(string proj_ID, string proj_title, string label, string contr_key)
{
    set_project_ID(proj_ID);
    set_project_title(proj_title);
    set_project_label(label);
    set_contributor_key(contr_key);
}


// This function should be called by the user, and should set up all the fields.
void iSENSE_Upload::set_project_ID(string proj_ID)
{
    // Set the Project ID, and the upload/get URLs as well.
    project_ID = proj_ID;

    upload_URL = devURL + "/projects/" + project_ID + "/jsonDataUpload";
    get_URL = devURL + "/projects/" + project_ID ;
    GET_PROJ_FIELDS();
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


// Extra function that the user can call to just generate a timestamp
// and push it back to the timestamp vector.
void iSENSE_Upload::generate_timestamp(void)
{
    time_t time_stamp;
    time(&time_stamp);
    char buffer[sizeof "2011-10-08T07:07:09Z"];
    strftime(buffer, sizeof buffer, "%FT%TZ", gmtime(&time_stamp));

    string cplusplus_timestamp(buffer);

    timestamp_pushback(cplusplus_timestamp);
}


// Given a timestamp in time_t format, push it back to the timestamp vector
void iSENSE_Upload::timestamp_pushback(string new_timestamp)
{
    // Push back to the timestamp vector
    timestamp.push_back(new_timestamp);
}


// Given a number, this will push it back to the vector of numbers
void iSENSE_Upload::numbers_pushback(string new_numbers)
{
    numbers.push_back(new_numbers);
}


// Given a string of text, this will push the string back to the vector of text strings
void iSENSE_Upload::text_pushback(string new_text)
{
    text.push_back(new_text);
}


// Given a latitude in string form, this function pushes a latitude back to the vector of latitude data
void iSENSE_Upload::latitude_pushback(string new_latitude)
{
    latitude.push_back(new_latitude);
}


// Given a longitude in string form, this function pushes a longitude back to the vector of longitude data
void iSENSE_Upload::longitude_pushback(string new_longitude)
{
    longitude.push_back(new_longitude);
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


// Call this function to POST data to rSENSE
void iSENSE_Upload::POST_JSON_KEY()
{
    // Check that the project ID is set properly.
    // When the ID is set, the fields are also pulled down as well.
    if(project_ID == "empty")
    {
        cout << "\nError - please set a project ID!\n";
        return;
    }

    // Check that a title and contributor key has been set.
    if(title == "TITLE")
    {
        cout << "\nError - please set a project title!\n";
        return;
    }

    if(contributor_key == "KEY")
    {
        cout << "\nErrror - please set a contributor key!\n";
        return;
    }

    // If a label wasn't set, automatically set it to "cURL"
    if(contributor_label == "LABEL")
    {
        contributor_label = "cURL";
    }

    // Format the data to be uploaded. Call another function to format this.
    format_upload_string();

    // Once we get the data formatted, we can try to POST to rSENSE

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
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        cout << "\nrSENSE says: \n";

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


// This function is called by the JSON uplosad function
// It formats the upload string
void iSENSE_Upload::format_upload_string()
{
    // Add the title + the correct formatting
    upload_data["title"] = value(title);
    upload_data["contribution_key"] = value(contributor_key);
    upload_data["contributor_name"] = value(contributor_label);

    // Add each field, with its field ID and an array of all the data in its vector.
    // Will need to check each field ID's type and then add all of the strings in that vector.

    // Grab all the fields using an iterator. Similar to printing them all out below in the debug function.
    array::iterator it;

    // Pointer to one of the 5 vectors, for using a function vs writing the same thing 5 times.
    vector<string> *vect;

    // Check and see if the fields object is empty
    if(fields.is<picojson::null>() == true)
    {
        // Print an error and quit, we can't do anything if the field array wasn't set up correctly.
        cout << "Error - field array wasn't set up. Have you pulled the fields off iSENSE?\n";
        return;
    }

    // We made an iterator above, that will let us run through the 3 fields
    for(it =fields_array.begin(); it != fields_array.end(); it++)
    {
        // Get the current object
        object obj = it->get<object>();

        // Grab the field ID and save it in a string/
        string field_ID = obj["id"].to_str();

        /*
            This part will be important for POSTing. We will want to save the fields and know what type they are.
            If we have a timestamp, number, text, latitude or longitude.
            We can detect this by looking at the "type" value.
        */

        // Grab the  type in number form.
        // DOUBLE CHECK WHY WE GET A DOUBLE INTO AN INTEGER...
        int type = obj["type"].get<double>();

        // Now we can build a switch statement around this!
        switch(type)
        {
            case 1:
                // Found a timestamp field
                vect = timestamp;
                format_data(vect, it);

                break;

            case 2:
                // We found a number, so run through that vector
                vect = numbers;
                format_data(vect, it);

                break;

            case 3:
                // Found a text field
                vect = text;
                format_data(vect, it);
                break;

            case 4:
                // Latitude here
                vect = latitude;
                format_data(vect, it);

                break;

            case 5:
                // Longitude here
                vect = longitude;
                format_data(vect, it);
                break;

            default:
                cout << "Error, why'd we get here?\n";

                break;
        }
    }

    // Once we've made the field_data object, we can
    // add the field_data object to the upload_data object
    upload_data["data"] = value(fields_data);
}


// This makes the switch above shorter, since I reuse this code for all 5 types of data.
void iSENSE_Upload::format_data(vector<string> *vect, array::iterator it)
{
    vector<string>::iterator x;    // For going through the vector
    array data;

    for(x = vect.begin(); x < vect.end(); x++)
    {
        data.pushback(x);
    }


}


// Call this function to dump all the data in the given object.
void iSENSE_Upload::DEBUG()
{
    cout << "PROJECT TITLE = " << title << endl;
    cout << "PROJECT ID = " << project_ID << endl;
    cout << "Contributor Key = " << contributor_key << endl;
    cout << "Contributor Label = " << contributor_label << endl;
    cout << "Upload URL: " << upload_URL << "\n";
    cout << "GET URL: " << get_URL << "\n\n";
    cout << "Upload Data: " << upload_data << "\n\n";
    cout << "GET Data: " << json_data.serialize() << "\n\n";
    cout << "Field Data: " << fields.serialize() << "\n\n";

    // Going to try outputting everything in the vectors here.
    vector<string>::iterator x;

    cout << "Timestamps in this project: \n";

    // Try printing everything out.
    for(x = timestamp.begin(); x < timestamp.end(); x++)
    {
        cout << *x << endl;
    }

    // If we find nothing, let the user know.
    if(timestamp.begin() == timestamp.end())
    {
        cout << "No timestampsfound.\n";
    }

    cout << "\nNumbers in this project: \n";

    // Try printing everything out.
    for(x = numbers.begin(); x < numbers.end(); x++)
    {
        cout << *x << endl;
    }

    // If we find nothing, let the user know.
    if(numbers.begin() == numbers.end())
    {
        cout << "No numbers found.\n";
    }

    cout << "\nText here in this project: \n";

    // Try printing everything out.
    for(x = text.begin(); x < text.end(); x++)
    {
        cout << *x << endl;
    }

    // If we find nothing, let the user know.
    if(text.begin() == text.end())
    {
        cout << "No text found.\n";
    }

    cout << "\nLatitude data in this project: \n";

    // Try printing everything out.
    for(x = latitude.begin(); x < latitude.end(); x++)
    {
        cout << *x << endl;
    }

    // If we find nothing, let the user know.
    if(latitude.begin() == latitude.end())
    {
        cout << "No latitude data found.\n";
    }

    cout << "\nLongitude data in this project: \n";

    // Try printing everything out.
    for(x = longitude.begin(); x < longitude.end(); x++)
    {
        cout << *x << endl;
    }

    // If we find nothing, let the user know.
    if(longitude.begin() == longitude.end())
    {
        cout << "No longitude data found.\n";
    }

    // Print out the field's, along with their type.
    array::iterator it;

    cout << "\nPrinting out all the fields here, using the array fields_array: \n";

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
