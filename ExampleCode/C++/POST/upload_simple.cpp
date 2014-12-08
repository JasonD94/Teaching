#include <iostream>
//#include "Include/API.h"               // API class
#include "Include/picojson.h"        // picojson for usin JSON easily.
#include "Include/memfile.h"        // picojson/curl uses this for temp files

using namespace std;

// For picojson
using namespace picojson;

/*
    Testing picojson and making sure it works correctly.
*/

int main()
{
    object upload_data;                     // the upload string, in JSON

    string title = "title";
    string contributor_key = "123";
    string contributor_label = "cURL";

    // Add the title + the correct formatting
    upload_data["title"] = value(title);
    upload_data["contribution_key"] = value(contributor_key);
    upload_data["contributor_name"] = value(contributor_label);

    string field1 = "1234";
    string field2 = "5678";

    object data;
    data[field1] = value("1");
    data[field2] = value("2");

    upload_data["data"] = value(data);

    cout << upload_data.serialize();
}