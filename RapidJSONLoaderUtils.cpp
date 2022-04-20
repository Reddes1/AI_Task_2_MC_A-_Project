#include "RapidJSONLoaderUtils.h"

#include <fstream>
#include <sstream>

void ParseNewJSONDocument(rapidjson::Document& doc, std::string& filePath)
{
	std::stringstream jsonDocBuffer;
	std::string inputLine;
	std::ifstream inputStream(filePath);

	while (std::getline(inputStream, inputLine))
		jsonDocBuffer << inputLine << "\n";

	inputStream.close();
	doc.Parse(jsonDocBuffer.str().c_str());
	assert(doc.IsObject());
}
