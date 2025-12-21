#pragma once
#define NOMINMAX
#include "windows.h"
#include <string>


class Window
{
public:
	HWND hwnd = NULL;
	HINSTANCE hinstance = nullptr;
	std::string name = "";
	int width =0;
	int height =0;
	// keybard and mouse
public:
	bool keys[256];
	bool keyJustPressed[256] = { false };        
	int mousex;
	int mousey;
	bool mouseButtons[3];
public:
	void processMessages();

	Window();

	void create(int window_width, int window_height, std::string window_name);

	void updateMouse(int x, int y)
	{
		mousex = x;
		mousey = y;
	}

	// Reset the "just pressed" state
	void resetKeyJustPressed()
	{
		for (int i = 0; i < 256; i++)
			keyJustPressed[i] = false;
	}
	
};

