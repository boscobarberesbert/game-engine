#pragma once

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

#include <windows.h>
#include <stdio.h>
#include "ImGuiAppLog.h"

#define LOAD_JSON_BOOL(b) { b = config.HasMember(#b) ? config[#b].GetBool() : b; }
#define SAVE_JSON_BOOL(b) { writer.String(#b); writer.Bool(b); }

#define LOAD_JSON_FLOAT(b) { b = config.HasMember(#b) ? config[#b].GetFloat() : b; }
#define SAVE_JSON_FLOAT(b) { writer.String(#b); writer.Double(b); }

#define LOG(format, ...) Log(__FILE__, __LINE__, format, __VA_ARGS__);
#define LOG_BOTH(format, ...) Log(__FILE__, __LINE__, format, __VA_ARGS__), appLog->AddLog(format);

void Log(const char file[], int line, const char* format, ...);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define HAVE_M_PI

typedef unsigned int uint;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

// Configuration -----------
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SCREEN_SIZE 1
#define WIN_FULLSCREEN false
#define WIN_RESIZABLE true
#define WIN_BORDERLESS false
#define WIN_FULLSCREEN_DESKTOP false
#define VSYNC true
#define TITLE "Ko-Fi Engine"
#define ORGANITZATION "CITM UPC 2021-2022"