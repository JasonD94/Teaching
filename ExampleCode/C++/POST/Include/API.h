#include <iostream>
#include <string>                // std::string, std::to_string;
#include <ctime>                // Timestamps
#include <vector>
#include <map>
#include <curl/curl.h>          // cURL to make HTTP requests
#include "picojson.h"            // picojson for usin JSON easily.
#include "memfile.h"            // picojson/curl uses this for temp files

// To avoid poluting the namespace, and also to avoid typing std:: everywhere.
using std::map;
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

class iSENSE
{
  public:
    iSENSE();                                          // Default constructor
    iSENSE(string proj_ID, string proj_title,          // Contructor with parameters.
           string label, string contr_key);

    // Similar to the constructor with parameters, but called after the object is created.
    // This way you can change/update the title/project ID/etc.
    void set_project_all(string proj_ID, string proj_title,
                         string label, string contr_key);

    void set_project_ID(string proj_ID);                // This function should set up all the fields
    void set_project_title(string proj_title);          // The user should also set the project title
    void set_project_label(string label);               // This one is optional, by default the label will be "cURL".
    void set_contributor_key(string contr_key);         // User needs to set the contributor key they will be using
    // In the future their should be a username & password function as well.

    // This functions will push data back to the map.
    void push_back(string field_name, string data);

    // Helper functions for uploading data to rSENSE
    void set_URL(string s_URL);             // Set the URL for this object
    void format_upload_string();            // This formats the upload string

    void format_data(vector<string> *vect,      // This formats one FIELD ID : DATA pairs
                              array::iterator it, string field_ID);

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
    object upload_data;              // the upload string, in JSON
    object fields_data;              // the "data" object, with Field ID: [DATA HERE]
    // change the above to be "field name": "data"

    // See if these are needed. At least one or two is.
    value json_data;
    value fields;
    array fields_array;

    /* Data to be uploaded to iSENSE. User must give the pushback function the following:
       1. Field name (as seen on iSENSE)
       2. some data (in string format). For numbers, use to_string.         */
    map<string, vector<string>> map_data;

    // Data needed for processing the upload request
    bool usingDev;                    // Whether the user wants iSENSE or rSENSE

    // consider removing these.
    string upload_URL;                // URL to upload the JSON to
    string get_URL;                   // URL to grab JSON from

    string title;                     // title for the dataset
    string project_ID;                // project ID of the project
    string contributor_label;         // Label for the contributor key. by default this is "cURL"
    string contributor_key;           // contributor key for the project
};


// Default constructor
iSENSE::iSENSE()
{
  // Set these to default values for future references
  upload_URL = "URL";
  get_URL = "URL";
  title = "title";
  project_ID = "empty";
  contributor_key = "key";
  contributor_label = "label";
}


// Constructor with parameters
iSENSE::iSENSE(string proj_ID, string proj_title,     // Contructor with parameters.
                                             string label, string contr_key)
{
  set_project_ID(proj_ID);
  set_project_title(proj_title);
  set_project_label(label);
  set_contributor_key(contr_key);
}

// Similar to the constructor with parameters, but can be called at anytime to
// set up the upload object.
void iSENSE::set_project_all(string proj_ID, string proj_title, string label, string contr_key)
{
  set_project_ID(proj_ID);
  set_project_title(proj_title);
  set_project_label(label);
  set_contributor_key(contr_key);
}


// This function should be called by the user, and should set up all the fields.
void iSENSE::set_project_ID(string proj_ID)
{
  // Set the Project ID, and the upload/get URLs as well.
  project_ID = proj_ID;

  upload_URL = devURL + "/projects/" + project_ID + "/jsonDataUpload";
  get_URL = devURL + "/projects/" + project_ID ;
  GET_PROJ_FIELDS();
}


// The user should also set the project title
void iSENSE::set_project_title(string proj_title)
{
  title = proj_title;
}


// This one is optional, by default the label will be "cURL".
void iSENSE::set_project_label(string label)
{
  contributor_label = label;
}


// As well as the contributor key they will be using
void iSENSE::set_contributor_key(string contr_key)
{
  contributor_key = contr_key;
}


// // Extra function that the user can call to just generate a timestamp
// // and push it back to the timestamp vector.
// void iSENSE::generate_timestamp(void)
// {
//   // this currently isn't working.
//
//   time_t time_stamp;
//   time(&time_stamp);
//   char buffer[sizeof "2011-10-08T07:07:09Z"];
//   strftime(buffer, sizeof buffer, "%FT%TZ", gmtime(&time_stamp));
//
//   string cplusplus_timestamp(buffer);l
//
//   timestamp_pushback(cplusplus_timestamp);
// }


// Add data to the map, which keeps track of the data to be uploaded.
void iSENSE::push_back(string field_name, string data)
{
  // Add the piece of data to the back of the vector with the given field name.
  map_data[field_name].push_back(data);
}


// This is pretty much straight from the GET_curl.cpp file.
void iSENSE::GET_PROJ_FIELDS()
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
void iSENSE::POST_JSON_KEY()
{
    // Check that the project ID is set properly.
    // When the ID is set, the fields are also pulled down as well.
    if(project_ID == "empty")
    {
      cout << "\nError - please set a project ID!\n";
      return;
    }

    // Check that a title and contributor key has been set.
    if(title == "title")
    {
      cout << "\nError - please set a project title!\n";
      return;
    }

    if(contributor_key == "key")
    {
      cout << "\nErrror - please set a contributor key!\n";
      return;
    }

    // If a label wasn't set, automatically set it to "cURL"
    if(contributor_label == "label")
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
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (value(upload_data).serialize()).c_str());

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
void iSENSE::format_upload_string()
{
  // Add the title + the correct formatting
  upload_data["title"] = value(title);
  upload_data["contribution_key"] = value(contributor_key);
  upload_data["contributor_name"] = value(contributor_label);

  // Add each field, with its field ID and an array of all the data in its vector.

  // Grab all the fields using an iterator. Similar to printing them all out below in the debug function.
  array::iterator it;

  // Pointer to one of the vectors in the map
  vector<string> *vect;

  // Check and see if the fields object is empty
  if(fields.is<picojson::null>() == true)
  {
    // Print an error and quit, we can't do anything if the field array wasn't set up correctly.
    cout << "Error - field array wasn't set up. Have you pulled the fields off iSENSE?\n";
    return;
  }

  // We made an iterator above, that will let us run through the fields
  for(it = fields_array.begin(); it != fields_array.end(); it++)
  {
    // Get the current object
    object obj = it->get<object>();

    // Grab the field ID and save it in a string/
    string field_ID = obj["id"].to_str();

    // Grab the field name
    string name = obj["name"].get<string>();

    // Now add all the data in that field's vector (inside the map) to the
    vect = &map_data[name];
    format_data(vect, it, field_ID);
  }

  // Once we've made the field_data object, we can
  // add the field_data object to the upload_data object
  upload_data["data"] = value(fields_data);
}


// This makes the switch above shorter, since I reuse this code for all 5 types of data.
void iSENSE::format_data(vector<string> *vect, array::iterator it, string field_ID)
{
    vector<string>::iterator x;    // For going through the vector
    value::array data;             // Using a picojson::value::array, basically a vector but represents a json array.

    // First we push all the vector data into a json array.
    for(x = vect -> begin(); x < vect -> end(); x++)
    {
      data.push_back(value(*x));
    }

    // Now we push the json array to the upload_data object.
    fields_data[field_ID] = value(data);
}


// Call this function to dump all the data in the given object.
void iSENSE::DEBUG()
{
  cout << "Project Title: \t\t" << title << endl;
  cout << "Project ID: \t\t" << project_ID << endl;
  cout << "Contributor  Key: \t" << contributor_key << endl;
  cout << "Contributor Label: \t" << contributor_label << endl;
  cout << "Upload URL: \t\t" << upload_URL << "\n";
  cout << "GET URL: \t\t" << get_URL << "\n\n";
  cout << "Upload Data: \n" << value(upload_data).serialize() << "\n\n";
  cout << "GET Data: \n" << json_data.serialize() << "\n\n";
  cout << "Field Data: \n" << fields.serialize() << "\n\n";

  // Going to try outputting everything in the vectors here.
  array::iterator it;
  map<string, vector<string>>::iterator y;
  vector<string>::iterator x;

  // We made an iterator above, that will let us run through the data
  for(it = fields_array.begin(); it != fields_array.end(); it++)
  {
    // Output all the fields
    object obj = it->get<object>();
    cout << "\nField ID: " << obj["id"].to_str() << endl;

    /* This part will be important for POSTing. We will want to save the fields and know what type they are.
        If we have a timestamp, number, text, latitude or longitude.
        We can detect this by looking at the "type" value.
    */

    // Grab the  type in number form.
    string name = obj["name"].get<string>();

    y = map_data.find(name);
    x = y->second.begin();

    cout << "Field Name: " << name << endl;
    cout << "Data: \n";

    if(y->second.begin() == y->second.end())
    {
      cout << "Data field is empty.\n";
    }

    for(x = y->second.begin(); x < y->second.end(); x++)
    {
      cout << *x << endl;
    }
  }
}
