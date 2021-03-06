#include <iostream>                  // std::cout, std::cin
#include <string>                    // std::string, std::to_string;
#include "include/API.h"             // API class

// To avoid poluting the namespace, and also to avoid typing std:: everywhere.
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::to_string;

// For picojson
using namespace picojson;

/*
 * This code tests appending by dataset ID
 * Not by dataset name.
 */

// Main, calls upload function. Uses picojson for JSON serialization / parsing.
int main ()
{
  // Example of using the iSENSE class
  iSENSE test;
  string title, ID, dataset_ID, key, letters, num, timestamp;

  // This will be a test of the append method.
  title = "this works?";
  ID = "1029";
  key = "key";
  dataset_ID = "7795";

  // Add project info / dataset info to the object
  test.set_project_ID(ID);
  test.set_dataset_ID(dataset_ID);
  test.set_project_title(title);
  test.set_contributor_key(key);

  timestamp = test.generate_timestamp();

  test.push_back("Number", "99");

  // Try grabbing fields. Error checking occurs below.
  test.get_project_fields();

  // Try formatting the upload data string without uploading yet.
  test.format_upload_string(2);

  // Check the fields for errors (manually compare against iSENSE)
  test.debug();

  char ans;

  do{
    cout << "Does the data look alright to you? (enter y/n) -> ";
    cin >> ans;
  }while(ans != 'y' && ans != 'Y' && ans != 'n' && ans != 'N');

  if(ans == 'n' || ans == 'N')
  {
    cout << "\nUser chose not to upload. Quitting instead.\n";
    return 0;
  }

  cout << "\nUploading to rSENSE.\n";
  test.append_key_byID(dataset_ID);

  // In the future we should tell the user if this upload function was a success. Or if it failed then why.
  return 0;
}
