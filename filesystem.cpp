#include "filesystem.hpp"
#include <stdexcept>
#include <windows.h>

using namespace std;
using namespace USNLIB::filesystem;
//using string = std::string;

template<typename T>
path::TYPE get_type(const T& info) {
	using t = path::TYPE;
	if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return t::DIRECTORY;
	}
	if (info.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) {
		return t::FILE;
	}

	return t::UNKNOWN;
}

template<typename T>
size_t get_size(const T& info) {
	return ((size_t)info.nFileSizeHigh << 32) | info.nFileSizeLow;
}


struct Finder{
	const string parent;
	WIN32_FIND_DATAA file_info;
	HANDLE find_handle;
	
	//Finder(const path* par) : parent(par), find_handle(NULL) {}
	Finder(const string& dir,const std::string& filter) : parent(dir),file_info{0},find_handle(FindFirstFileExA((dir+filter).c_str(),FindExInfoBasic,&file_info,FindExSearchNameMatch,NULL,0)){
		if (find_handle==INVALID_HANDLE_VALUE)
			find_handle=NULL;
	}
	~Finder(void){
		if (find_handle)
			FindClose(find_handle);
	}
	bool step(void){
		if (!FindNextFileA(find_handle,&file_info)){
			FindClose(find_handle);
			find_handle=NULL;
			return false;
		}
		return true;
	}
	operator string(void) const {
		return parent + file_info.cFileName;
	}
	path::TYPE type(void) const {
		return get_type(file_info);
	}
	size_t size(void) const {
		return get_size(file_info);
	}
};


path::iterator::iterator(void) : recursive(false) {}
path::iterator::iterator(const string& p, const string& fil, bool r) : recursive(r),filter(fil) {
	finder.push_back(make_shared<Finder>(p, fil));
	subdir();
}

void path::iterator::subdir(void) {
	if (!recursive)
		return;

	auto f = finder.front();

	if (f->type() == DIRECTORY) {
		if (f->file_info.cFileName[0] == '.') {
			switch (f->file_info.cFileName[1]) {
			case 0:
			case '.':
				return;
			}
		}
		//string str = *f;
		path p(*f, DIRECTORY, 0);
		finder.push_back(make_shared<Finder>(p.pathname(),filter));
	}
}


path path::iterator::operator*(void) const {
	if (finder.empty())
		throw out_of_range("filesystem::path::iterator");

	auto f = finder.front();
	
	return path(*f, f->type(), f->size());
}

path::iterator& path::iterator::operator++(void) {
	if (finder.empty())
		throw out_of_range("filesystem::path::iterator");

	while (!finder.empty()) {
		if (finder.front()->step()) {
			subdir();
			break;
		}
		else {
			finder.pop_front();
		}
	}

	return *this;
}

bool path::iterator::operator!=(const iterator& cmp) const {
	return finder.empty() != cmp.finder.empty();
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
	file_type = get_type(info);
	switch (file_type) {
	case FILE:
		//file_size = ((size_t)info.nFileSizeHigh << 32) | info.nFileSizeLow;
		file_size = get_size(info);
		break;
	case DIRECTORY:
		if (file_path.back() != '\\')
			file_path.push_back('\\');
	case UNKNOWN:
		file_size = 0;
	}
	/*
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
	*/
}

path::operator const string& (void) const{
	return file_path;
}

path::operator bool (void) const{
	switch (file_type) {
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

path::iterator path::begin(bool recursive) const{
	return iterator(pathname(),
		file_type == DIRECTORY ? string("*") : filename(),recursive );
}

path::iterator path::begin(const string& filter,bool recursive) const{
	return iterator(pathname(),filter,recursive);
}

path::iterator path::end(void) const{
	return iterator();
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
