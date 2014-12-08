#include <iostream>                  // std::cout, std::cin
#include <string>                       // std::string, std::to_string;
#include <time.h>                      // for time stamps
#include <curl/curl.h>                 // cURL to make HTTP requests
#include "Include/API.h"               // API class
#include "Include/picojson.h"        // picojson for usin JSON easily.
#include "Include/memfile.h"        // picojson/curl uses this for temp files

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::stringstream;                  // for concating an int onto a string.
using std::to_string;                       // REQUIRES C++11, make sure to enable with flag -std=c++11

// For picojson
using namespace picojson;

// Main, calls upload function. Uses picojson for JSON serialization / parsing.
int main ()
{
    // Example of using the class iSENSE_Upload
    iSENSE_Upload test;

    // Get user input
    string title;
    string ID;
    string key;
    string letters;
    string num;

    // Get user input.
    cout << "Please set the project ID for this dataset: ";     // Sets project ID
    getline(cin, ID);

    cout << "Please set a contributor key for this project: ";  // Set contributor key
    getline(cin, key);

    cout << "Please enter a title for the dataset: ";       // Gets the title
    getline(cin, title);

    // Add project info / dataset info to the object
    test.set_project_ID(ID);
    test.set_project_title(title);
    test.set_project_label("cURL");
    test.set_contributor_key(key);

    // Let's add a timestamp, but we're lazy so we'll use the function isenseupload provides for us.
    test.generate_timestamp();

    // Let's also push some stuff back to the other vectors.
    /*
	Olsen hall
        42.654761, -71.326674
    */
    test.numbers_pushback("123456789");
    test.text_pushback("THIS IS A STRING OF TEXT");
    test.latitude_pushback("42.654761");
    test.latitude_pushback("42.654761");
    test.latitude_pushback("42.654761");
    test.longitude_pushback("-71.326674");
    test.longitude_pushback("-71.326674");
    test.longitude_pushback("-71.326674");

    // I wonder if we can push back a bunch of numbers?
    for(int i = 0; i < 10; i++)
    {
        // Make sure to use "to_string" to convert an int/double/float/etc to a string!
        test.numbers_pushback(to_string(i));
    }

    // At some point make it possible to add data / fields
    // Try grabbing fields. Hope this works!
    test.GET_PROJ_FIELDS();

    // Try formatting the upload data string without uploading yet.
    test.format_upload_string();

    // Let's try uploading now and see if it works.
    test.POST_JSON_KEY();

    // DEBUG testing
    cout << "\n";
    test.DEBUG();

    // In the future we should tell the user if this upload function was a success. Or if it failed - if it failed then why.
    return 0;
}
