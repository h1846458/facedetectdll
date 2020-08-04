#pragma once
#include "pch.h"
#include <memory.h> 
#include <stdint.h>
#pragma pack(push)
#pragma pack(1)
#ifdef EXPORT_FACEDETECT_DLL
#define FACEDETECT_API __declspec(dllexport)
#else
#define FACEDETECT_API __declspec(dllimport)
#endif
#define MAXFACE 10
struct faceframe
{
		int x;
		int y;
		int w;
		int h;
		int confidence;
		int angle;
		const char* base64origin;
		const char* base64crop;
		faceframe()
		{
			memset(this, 0, sizeof(faceframe));
		}
};

struct FaceSnap
{
	size_t	facecount;
	faceframe faces[MAXFACE];
	FaceSnap()
	{
		memset(this, 0, sizeof(FaceSnap));
	}
};

typedef void(*MsgCallback)(faceframe msg, char* path);
extern "C"
{
	FACEDETECT_API int facedetect(MsgCallback func,char* path);
}
#pragma pack(pop)