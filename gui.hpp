#pragma once
#include <wtypes.h>
#include <string>
#include <unordered_map>


namespace USNLIB{
	class gui{
		
		class atom{
			ATOM a;
			
		public:
			//atom(void);
			atom(const std::string& ,LPCTSTR icon = IDI_APPLICATION);
			atom(const atom&) = delete;
			atom(atom&&);
			//atom& operator=(atom&&);
			~atom(void);
			operator ATOM(void) const;
			
		};
		
		friend class atom;
		
		static std::unordered_map<std::string,atom> wnd_class;
		
		static std::unordered_map<HWND,gui*> record;
		
		static LRESULT CALLBACK jmp_msg(HWND, UINT, WPARAM, LPARAM);
		
	protected:
		HWND hwnd;
		DWORD style;
		int width, height;
		std::string tit;
		decltype(wnd_class)::const_iterator klass;


		virtual LRESULT msg_proc(UINT, WPARAM, LPARAM);
		virtual bool msg_pump(MSG&);

		gui(const char* klass);
	public:
		static const HINSTANCE instance;
	
		gui(void);
	
		//gui(DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE,int w = CW_USEDEFAULT,int h = CW_USEDEFAULT);
		virtual ~gui(void);

		virtual void show(void);
		virtual const std::string& title(void) const;
		virtual void title(const std::string&);
	};
	
	
	
}