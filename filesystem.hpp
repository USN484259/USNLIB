#pragma once
#include "types.h"
#include <string>
#include <deque>
#include <memory>

struct Finder;

namespace USNLIB{
	namespace filesystem{	
	
		class path{
		public:
			class iterator {		//input_iterator_tag

				friend class path;
				const bool recursive;
				const std::string filter;
				std::deque<std::shared_ptr<Finder> > finder;

				void subdir(void);

				iterator(void);
				iterator(const std::string&,const std::string&,bool = false);

			public:
				path iterator::operator*(void) const;
				iterator& iterator::operator++(void);
				bool iterator::operator!=(const iterator&) const;

			};
			enum TYPE { UNKNOWN , FILE , DIRECTORY };
			
		private:
		
			std::string file_path;
			TYPE file_type;
			size_t file_size;
			
			friend class iterator;
			path(const std::string&,TYPE,size_t);
			//void refresh(void) const;
			//static std::string&& resolve(const std::string&);

		public:
			path(void);
			path(const std::string&);
			operator const std::string& (void) const;
			operator bool (void) const;
			
			TYPE type(void) const;
			size_t size(void) const;
			
			iterator begin(bool = false) const;
			iterator begin(const std::string&,bool = false) const;
			iterator end(void) const;
			
			std::string pathname(void) const;
			std::string filename(void) const;
			std::string extension(void) const;
			
			path operator+(const std::string&) const;
		};
		
	}
}