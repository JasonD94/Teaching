#include <iostream>                   	// std::cout, std::cin
#include <string>                   	// std::string, std::to_string;
#include <curl/curl.h>                 	// cURL to make HTTP requests
#include <time.h>                       // for time stamps
#include "isenseupload.h"            	// using isenseupload.h
#include "../../../../picojson.h"       // May need to change the path for this if not in git repo

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::stringstream;                // for concating an int onto a string.
using std::to_string;                   // REQUIRES C++11, make sure to enable with flag -std=c++11

// For picojson
using namespace picojson;

// Main, calls upload function. Uses ***something*** for JSON serialization / parsing.
int main ()
{
    // Example of using the class iSENSE_Upload
    iSENSE_Upload test;

    // Add project info / dataset info to the object
    test.set_project_ID("929");
    test.set_project_title("TEST");
    test.set_project_label("cURL");
    test.set_contributor_key("123");

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

    // DEBUG testing
    test.DEBUG();

    // In the future we should tell the user if this upload function was a success. Or if it failed - if it failed then why.
    return 0;
}
