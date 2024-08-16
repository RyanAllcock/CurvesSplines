#ifndef HEADER_FILEMANAGER
#define HEADER_FILEMANAGER

#include <fstream> // file accessing
#include <sstream> // structured file extracting

struct FileManager{
	static std::string get(std::string const &fileName){
		std::ifstream file(fileName);
		if(!file) return "";
		std::stringstream data;
		std::string line;
		data << file.rdbuf();
		file.close();
		return data.str();
	}
};

#endif