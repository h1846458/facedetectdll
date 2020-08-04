#include "pch.h"
#include <stdio.h>
#include "opencv.hpp"
#include "highgui\highgui.hpp"
#include "facedetectcnn.h"
#include <map>
#include <vector>
#include <iostream>
#include "facedetectapi.h"
#include <sstream>
#define CV_IMWRITE_JPEG_QUALITY 95
#define EXPORT_FACEDETECT_DLL
using std::stringstream;
#define DETECT_BUFFER_SIZE 0x20000
using namespace cv;
static std::string base64Decode(const char* Data, int DataByte) 
{
	//解码表
	const char DecodeTable[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		62, // '+'
		0, 0, 0,
		63, // '/'
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
		0, 0, 0, 0, 0, 0,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
	};
	std::string strDecode;
	int nValue;
	int i = 0;
	while (i < DataByte) {
		if (*Data != '\r' && *Data != '\n') {
			nValue = DecodeTable[*Data++] << 18;
			nValue += DecodeTable[*Data++] << 12;
			strDecode += (nValue & 0x00FF0000) >> 16;
			if (*Data != '=') {
				nValue += DecodeTable[*Data++] << 6;
				strDecode += (nValue & 0x0000FF00) >> 8;
				if (*Data != '=') {
					nValue += DecodeTable[*Data++];
					strDecode += nValue & 0x000000FF;
				}
			}
			i += 4;
		}
		else {
			Data++;
			i++;
		}
	}
	return strDecode;
}

static std::string base64Encode(const unsigned char* Data, int DataByte)
{
	//编码表
	const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	//返回值
	std::string strEncode;
	unsigned char Tmp[4] = { 0 };
	int LineLength = 0;
	for (int i = 0; i < (int)(DataByte / 3); i++) {
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		Tmp[3] = *Data++;
		strEncode += EncodeTable[Tmp[1] >> 2];
		strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
		strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
		strEncode += EncodeTable[Tmp[3] & 0x3F];
		if (LineLength += 4, LineLength == 76) { strEncode += "\r\n"; LineLength = 0; }
	}
	//对剩余数据进行编码
	int Mod = DataByte % 3;
	if (Mod == 1) {
		Tmp[1] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
		strEncode += "==";
	}
	else if (Mod == 2) {
		Tmp[1] = *Data++;
		Tmp[2] = *Data++;
		strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
		strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
		strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
		strEncode += "=";
	}


	return strEncode;
}

//imgType 包括png bmp jpg jpeg等opencv能够进行编码解码的文件
static std::string Mat2Base64(const cv::Mat& img, std::string imgType)
{
	//Mat转base64
	std::string img_data;
	std::vector<uchar> vecImg;
	std::vector<int> vecCompression_params;
	vecCompression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	vecCompression_params.push_back(90);
	imgType = "." + imgType;
	cv::imencode(imgType, img, vecImg, vecCompression_params);
	img_data = base64Encode(vecImg.data(), vecImg.size());
	return img_data;
}

/*
static cv::Mat Base2Mat(std::string& base64_data) {
	cv::Mat img;
	std::string s_mat;
	s_mat = base64Decode(base64_data.data(), base64_data.size());
	std::vector<char> base64_img(s_mat.begin(), s_mat.end());
	img = cv::imdecode(base64_img, cv::CV_LOAD_IMAGE_COLOR);
	return img;
}*/

int facedetect(MsgCallback func, char* path)
{
	cv::Mat image = cv::imread(path);
	int width = image.size().width;
	int height = image.size().height;
	if(image.empty())
	{
		fprintf(stderr, "Can not load the image file %s.\n", path);
		return -1;
	}
	
	int * pResults = NULL; 
    unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
    if(!pBuffer)
    {
        fprintf(stderr, "Can not alloc buffer.\n");
        return -1;
    }
	pResults = facedetect_cnn(pBuffer, (unsigned char*)(image.ptr(0)), image.cols, image.rows, (int)image.step);
	cv::Mat result_cnn = image.clone();
	string tmpstr = path;
	size_t sl = strlen(path);
	string pform = tmpstr.substr(sl - 3, 3);
	for(int i = 0; i < (pResults ? *pResults : 0); i++)
	{
        short * p = ((short*)(pResults+1)) + 142*i;
        int x = p[0] - p[2]*2/5;
		int y = p[1] - p[3] * 3/5;
		int w = p[2] + p[2]*3/5;
		int h = p[3] + p[3];
		float confidence = p[4];
		float angle = p[5];
		if (x > 0 && y > 0 && x + w < width && y + h < height && w * h > 100 * 200)
		{
			faceframe faces;
			//stringstream fmt;
			//fmt << "{'face_rect': [" << x << ", " << y << ", " << w << ", " << h << "], " << "'confidence': " << confidence << ", 'angle': " << angle << "}";
			faces.x = x;
			faces.y = y;
			faces.w = w;
			faces.h = h;
			faces.confidence = confidence;
			faces.angle = angle;
			Rect croprect = Rect(x, y, w, y);
			Mat cropimg = image(croprect);
			string img_base64_origin = Mat2Base64(image, pform);
			string	img_base64_crop = Mat2Base64(cropimg, pform);
			faces.base64origin = img_base64_origin.c_str();
			faces.base64crop = img_base64_crop.c_str();
			//fprintf(stdout, img_base64_origin.c_str());
			(*func)(faces, path);                  
			//fmt.str().c_str();
		}
		//rectangle(result_cnn, Rect(x, y, w, h), Scalar(0, 255, 0), 2);
	}
    //cout << facemap["face0"][0];
	//imshow("result_cnn", result_cnn);
	//waitKey();
    //release the buffer
    free(pBuffer);
	return 0;
}
