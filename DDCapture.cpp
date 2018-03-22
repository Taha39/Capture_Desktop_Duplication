// DDCapture.cpp : Defines the entry point for the console application.
//

#include "CaptureDD.h"
#include <Windows.h>

#define	TIME_DIFF(S, E, F)			(((E.QuadPart - S.QuadPart) * 1000)/F.QuadPart)
int VIDEO_FPS;
#pragma comment(lib, "winmm.lib")



struct MonitorDetail
{
	bool bPrimary;
	UINT width;
	UINT height;
	UINT left;
	UINT top;
	MonitorDetail()
	{
		bPrimary = false;
		width = height = left = top = 0;
	}
};
struct MonitorList
{
	UINT8 count;
	MonitorDetail list[2];
	MonitorList() : count(0)
	{
	}
};

BOOL CALLBACK MonitorEnumProcCallback(_In_  HMONITOR hMonitor, _In_  HDC DevC, _In_  LPRECT lprcMonitor, _In_  LPARAM dwData)
{
	MonitorList *pList = (MonitorList*) dwData;
	if (!pList)
		return false;

	if (pList->count >= 2)
		return true;

	MONITORINFO  info;
    info.cbSize = sizeof(MONITORINFO);
    BOOL monitorInfo = GetMonitorInfo(hMonitor, &info);

	UINT8 i = pList->count;
	if(info.dwFlags == 1)
		pList->list[i].bPrimary = true;
	else
		pList->list[i].bPrimary = false;	
	pList->list[i].width = info.rcMonitor.right - info.rcMonitor.left;
	pList->list[i].height = info.rcMonitor.bottom - info.rcMonitor.top;
	pList->list[i].left = info.rcMonitor.left;
	pList->list[i].top = info.rcMonitor.top;

	pList->count++;
    return TRUE;
}

BOOL PopulateMonitorInfo(MonitorList* pList)
{
	HDC m_DevC = GetDC(GetDesktopWindow());
	BOOL bStatus = EnumDisplayMonitors(m_DevC, NULL, MonitorEnumProcCallback, (LPARAM)pList);
	return bStatus;
}

#include <time.h>
#include <string>
using namespace DD;

int _tmain(int argc, _TCHAR* argv[])
{
	MonitorList stMonList;
	PopulateMonitorInfo(&stMonList);
	for (int i = 0; i < stMonList.count; i++)
	{
		printf("Display[%d]: Primary[%d] L,T[%d,%d] WxH[%dx%d]", i, stMonList.list[i].bPrimary, stMonList.list[i].left, stMonList.list[i].top, stMonList.list[i].width, stMonList.list[i].height);
	}

	CaptureDsktpDup* m_scrnCap = new CaptureDsktpDup(stMonList.list[0].width, stMonList.list[0].height, stMonList.list[0].bPrimary);
	int ret = m_scrnCap->init();
	if (!ret)
	{
		exit(0);
	}

	/***************************************/
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	// for more information about date/time format
	
	strftime(buf, sizeof(buf), "%Y%m%d_%X", &tstruct);
	
	std::string str(buf);
	char *temp = (char*)str.c_str();
	char* correctfile = new char[str.length()-1];
	memset(correctfile, 0, str.length()-1);

	{
		int i = 0, j = 0;
		while (temp[i] != '\0')
		{
			if (temp[i] == ':')
			{
				i++;
				continue;				
			}
			correctfile[j] = buf[i];
			j++;
			i++;
		}
	}
	str = correctfile;
	str += ".rgb";
	/**************************************/
	FILE* fp;
	fopen_s(&fp, str.c_str(), "wb");
	int count = 0;
	UINT32 sizeCap = stMonList.list[0].width * stMonList.list[0].height * 4;
	//UINT32 sizeCap = stMonList.list[0].width * stMonList.list[0].height * 3 / 2;
	UINT8* data = new UINT8[sizeCap];
	while(1)
	{
#ifndef ACTUAL_PROJECT
		if (m_scrnCap->capture_frame(data))
#else
		if (m_scrnCap->capture_frame())
#endif
		{
			fwrite(data, 1, sizeCap, fp);
			Sleep(15);
		}
		if(count > 5000)
			break;
		count++;
	}
	fclose(fp);
	return 0;
}

