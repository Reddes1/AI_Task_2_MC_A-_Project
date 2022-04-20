#pragma once
#include <string>

#include "document.h"

/*
	Helper functions in JSON files.
*/


//Loads and parses JSON file into document object
void ParseNewJSONDocument(rapidjson::Document& doc, std::string& filePath);