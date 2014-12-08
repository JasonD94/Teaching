#include <iostream>
//#include "Include/API.h"               // API class
#include "Include/picojson.h"        // picojson for usin JSON easily.
#include "Include/memfile.h"        // picojson/curl uses this for temp files
#include "vector"

using namespace std;

// For picojson
using namespace picojson;

/*
    Testing picojson and making sure it works correctly.
*/

int main()
{
    // Hmm, this works!
    // This suggests a simple way of uploading to any dataset on iSENSE!

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

    cout << endl << value(upload_data).serialize() << "\n\n";

    // Going to test picojson array right now.
    object arg_data;

    arg_data[0] = value(1);
    arg_data[1] = value(2);
    arg_data[2] = value(3);

    upload_data["arg_data"] = value(arg_data);

    cout << value(upload_data).serialize() << "\n\n";

}