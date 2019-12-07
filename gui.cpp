#include "gui.hpp"
#include <stdexcept>
#include <typeinfo>
#include <windows.h>

#ifdef UNICODE
#include <locale>
#include <codecvt>
#endif


using namespace std;
using namespace USNLIB;

unordered_map<std::string, gui::atom> gui::wnd_class;

unordered_map<HWND, gui*> gui::record;

const HINSTANCE gui::instance = (HINSTANCE)GetModuleHandle(NULL);

//gui::atom::atom(void) : a(0) {}

gui::atom::atom(const string& name,LPCTSTR icon){
	
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = gui::instance;
#ifdef UNICODE
	wstring_convert< codecvt_utf8_utf16<wchar_t> > converter;
	wstring l_name = converter.from_bytes(name);
	wc.lpszClassName = l_name.c_str();

#else
	wc.lpszClassName = name.c_str();
#endif

	wc.lpfnWndProc = gui::jmp_msg;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = wc.hIcon = LoadIcon(instance, icon);
	a = RegisterClassEx(&wc);
	if (!a)
		throw runtime_error("wndclass register");

}

gui::atom::atom(atom&& obj) : a(obj.a) {
	obj.a = 0;
}

gui::atom::~atom(void){
	if (a)
		UnregisterClass( (LPCTSTR)a , gui::instance);
}
/*
gui::atom& gui::atom::operator=(atom&& obj) {
	if (a)
		throw runtime_error("gui::atom re-assign");
	a = obj.a;
	obj.a = 0;
}
*/
gui::atom::operator ATOM(void) const {
	return a;
}

gui::gui(void) : gui(typeid(gui).name()) {}


gui::gui(const char* k) : hwnd(0), style(WS_OVERLAPPEDWINDOW | WS_VISIBLE), width(CW_USEDEFAULT), height(CW_USEDEFAULT) {
	string name("USNLIB_WND_");
	name.append(k);
	klass = wnd_class.find(string(name));
	if (klass == wnd_class.end()) {
		klass = wnd_class.insert(decltype(wnd_class)::value_type(string(name), atom(name))).first;
	}
	
	
	
}
gui::~gui(void){
	record.erase(hwnd);
}

void gui::show(void){
#ifdef UNICODE
	wstring_convert< codecvt_utf8_utf16<wchar_t> > converter;
	wstring name = converter.from_bytes(tit);

#else
	const string& name = tit;
#endif

	volatile HWND tmp = CreateWindowEx(0, (LPCTSTR)(ATOM)klass->second, name.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, NULL/*hInstance*/, static_cast<void*>(this));
	if (tmp == NULL)
		throw runtime_error("gui CreateWindowEx");
	//{
	//	DWORD errorcode = GetLastError();
	//	stringstream ss;
	//	ss << "gui CreateWindowEx " << hex << errorcode;
	//	throw runtime_error(ss.str().c_str());
	//}
	if (tmp != hwnd)
		throw runtime_error("gui hwnd mismatch");

	record.insert(decltype(record)::value_type(hwnd, this));

	MSG msg;
	while (this->msg_pump(msg)) {
		if (IsDialogMessage(hwnd, &msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//while(true){
	//	BOOL res=GetMessage(&msg,NULL,0,0);
	//	if (res==0 || res==-1)
	//		break;
	//	if (IsDialogMessage(hwnd, &msg))
	//		continue;
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

}

const string& gui::title(void) const {
	return tit;
}

void gui::title(const string& str) {
	tit = str;
	SetWindowText(hwnd, tit.c_str());
}

LRESULT gui::msg_proc(UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

bool gui::msg_pump(MSG& msg) {
	BOOL res = GetMessage(&msg, NULL, 0, 0);
	return res != 0 && res != -1;
}


LRESULT CALLBACK gui::jmp_msg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
	auto it=record.find(hwnd);
	if (it==record.end()){
		if (msg==WM_CREATE && lparam){
			CREATESTRUCT* info = reinterpret_cast<CREATESTRUCT*>(lparam);
			gui* This=static_cast<gui*>(info->lpCreateParams);
			This->hwnd=hwnd;
			return This->msg_proc(msg,wparam,lparam);
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	
	return it->second->msg_proc(msg,wparam,lparam);
	
}






