#include "filesystem.hpp"
#include <stdexcept>
#include <windows.h>

using namespace std;
using namespace USNLIB::filesystem;
//using string = std::string;

struct Finder{
	const path* parent;
	WIN32_FIND_DATAA file_info;
	HANDLE find_handle;
	
	Finder(const path* par) : parent(par), find_handle(NULL) {}
	Finder(const path* par,const std::string& str) : parent(par),file_info{0},find_handle(FindFirstFileExA(((const string&)*par+str).c_str(),FindExInfoBasic,&file_info,FindExSearchNameMatch,NULL,0)){
		if (find_handle==INVALID_HANDLE_VALUE)
			find_handle=NULL;
	}
	~Finder(void){
		if (find_handle)
			FindClose(find_handle);
	}
	void step(void){
		if (!FindNextFileA(find_handle,&file_info)){
			FindClose(find_handle);
			find_handle=NULL;
		}
	}

};


path::iterator::iterator(const path* p) : finder(make_shared<Finder>(p)) {}
path::iterator::iterator(const path* p,const string& str) : finder(make_shared<Finder>(p,str)) {}


path path::iterator::operator*(void) const {
	if (!finder->find_handle)
		throw out_of_range("filesystem::path::iterator");
	const WIN32_FIND_DATAA& info = finder->file_info;
	
	return path(finder->parent->pathname()+info.cFileName, info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? DIRECTORY : FILE, ((size_t)info.nFileSizeHigh << 32) | info.nFileSizeLow);
}

path::iterator& path::iterator::operator++(void) {
	if (!finder->find_handle)
		throw out_of_range("filesystem::path::iterator");

	finder->step();

	return *this;
}

bool path::iterator::operator!=(const iterator& cmp) const {
	return finder->find_handle != cmp.finder->find_handle;
}



path::path(const string& str,TYPE t,size_t s) : file_path(str),file_type(t),file_size(s){
	if (file_type == DIRECTORY && file_path.back() != '\\')
		file_path.push_back('\\');
}

path::path(void) : file_type(UNKNOWN),file_size(0) {}

path::path(const string& str) : file_path(str) {



	WIN32_FILE_ATTRIBUTE_DATA info;
	if (!GetFileAttributesExA(file_path.c_str(), GetFileExInfoStandard, &info))
		throw runtime_error("filesystem::path::refresh");
	if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		file_type = DIRECTORY;
		file_size = 0;
		if (file_path.back() != '\\')
			file_path.push_back('\\');
		return;
	}
	if (info.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL )) {
		file_type = FILE;
		file_size = ((size_t)info.nFileSizeHigh << 32) | info.nFileSizeLow;
		return;
	}

	file_type = UNKNOWN;
	file_size = 0;

}

path::operator const string& (void) const{
	return file_path;
}

path::operator bool (void) const{
	switch(file_type){
	case UNKNOWN:
		return false;
	case FILE:
	case DIRECTORY:
		return true;
	}
}

path::TYPE path::type(void) const{	
	return file_type;
}

size_t path::size(void) const{
	return file_size;
}

path::iterator path::begin(void) const{
	return iterator(this,string(file_type==DIRECTORY?"*":""));
}

path::iterator path::begin(const string& filter) const{
	return iterator(this,filter);
}

path::iterator path::end(void) const{
	return iterator(this);
}

string path::pathname(void) const {
	auto pos = file_path.find_last_of('\\');
	if (pos == file_path.npos)
		return string();
	return file_path.substr(0, pos+1);
}

string path::filename(void) const {
	auto pos = file_path.find_last_of('\\');
	if (pos == file_path.npos)
		return file_path;
	if (pos + 1 == file_path.size())
		return string();
	return file_path.substr(pos + 1, file_path.npos);
}

string path::extension(void) const {
	auto pos = file_path.find_last_of(".\\");
	if (pos == file_path.npos)
		return string();

	if (file_path.at(pos) == '.')
		return file_path.substr(pos + 1, file_path.npos);

	return string();
}
