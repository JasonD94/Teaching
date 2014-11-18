#include <iostream>
#include <stdio.h>
#include <curl/curl.h>      //CURL to make HTTP requests
#include "../../../../picojson.h"    // May need to change the path for this if not in git repo

// Don't cloud up the namespace
using std::cout;
using std::cin;
using std::string;
using std::endl;

// For picojson
using namespace picojson;



// Testing this, must be an easier way
// This is from the picojson example page
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



int main()
{
    // This project will try using CURL to make a basic GET request to rSENSE
    // It will then save the JSON it recieves into a picojson object.
    CURL *curl = curl_easy_init();;
    CURLcode res;
    MEMFILE* json_file = memfopen();           // Writing JSON to this file.
    char error[256];                                // Errors get written here

    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://rsense-dev.cs.uml.edu/api/v1/projects/929");
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
            cout << error << endl;

            // Quit so we don't go to the next part.
            return 0;
        }
         // Good to try and parse the JSON into a PICOJSON object if we get here
        value json_data;
        string errors;

        // This will parse the JSON file.
        parse(json_data, json_file->data, json_file->data + json_file->size, &errors);

        // If we have errors, print them out and quit.
        if(errors.empty() != true)
        {
            cout << "Error parsing JSON file!\n";
            cout << "Error was: " << errors;
            return 0;
        }

        // If we get here, let's try printing out some JSON!
        string output_test = json_data.serialize();
        cout << "\n\n" << output_test << endl;
    }

    // Clean up cURL and close the memfile
    curl_easy_cleanup(curl);
    curl_easy_init();

    cout << endl << endl;       // Add some extra lines on the terminal.

    return 0;
}