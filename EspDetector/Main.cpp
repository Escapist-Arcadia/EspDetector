#include<Windows.h>
#include<ctime>
#include<CommCtrl.h>
#pragma comment(lib,"Comctl32.lib")
#include<Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#include<Psapi.h>
#pragma comment(lib,"Psapi.lib")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include"EspArray.hpp"
#include"EspString.hpp"
#include"EspConvert.hpp"
#include"resource.h"
const char pszClassName[] = "EspDetector";
const char pszWindowName[] = "应用使用时间记录工具 Escapist.桃花源";
const char pszClassName_Pw[] = "EspDetectorPw";
const char pszWindowName_Pw[] = "请输入密码";
const char pszWindowName_Pw2[] = "请再输入一次";

HFONT hFont = ::CreateFontA(12, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE, "宋体");
HFONT hPwFont = ::CreateFontA(24, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE, "微软雅黑");


HWND hMainWnd;
HWND hPwWnd;
HINSTANCE hInst;
EspString pszResult;
HCURSOR hCurProc;
HCURSOR hCurNormal;
HICON hIcoNormal;
HICON hIcoProc;
bool bProcPath = false;
bool bProcName = false;
bool bProcTask = false;
bool bTerminatePath = false;
bool bTerminateName = false;
bool bTaskName = false;
EspArray<unsigned long> arrTerminate;
unsigned long dwPointThreadID;
char pszPointThreadName[256];
char pszPointWndName[256];
EspArray<EspString> arrTerminatePath;
EspArray<EspString> arrTerminateName;
EspArray<EspString> arrTaskName;

int nTerminateTime = 0;

void DelayExA(long long DelayTime)
{
	LARGE_INTEGER CurrTicks, TicksCount;
	QueryPerformanceFrequency(&TicksCount);
	QueryPerformanceCounter(&CurrTicks);
	TicksCount.QuadPart = TicksCount.QuadPart * DelayTime / 1000;
	TicksCount.QuadPart = TicksCount.QuadPart + CurrTicks.QuadPart;
	while (CurrTicks.QuadPart < TicksCount.QuadPart)
		QueryPerformanceCounter(&CurrTicks);
}

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc_Password(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPreInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	hInst = hInstance;
	WNDCLASSEXA WindowClass, WindowClass_Password;
	WindowClass.cbSize = sizeof(WNDCLASSEXA);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = hInstance;
	WindowClass.hIcon = ::LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON_MAIN));
	WindowClass.hCursor = ::LoadCursorA(hInstance, MAKEINTRESOURCEA(32512));
	WindowClass.hbrBackground = ::CreateSolidBrush(RGB(255, 255, 255));
	WindowClass.lpszMenuName = NULL;
	WindowClass.hIconSm = ::LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON_MAIN));

	::memcpy(&WindowClass_Password, &WindowClass, sizeof(WNDCLASSEXA));

	WindowClass.lpfnWndProc = WndProc;
	WindowClass.lpszClassName = pszClassName;
	if (!::RegisterClassExA(&WindowClass))
		return 0;

	WindowClass_Password.lpfnWndProc = WndProc_Password;
	WindowClass_Password.lpszClassName = pszClassName_Pw;
	if (!::RegisterClassExA(&WindowClass_Password))
		return 0;

	hMainWnd = ::CreateWindowExA(WS_EX_LEFT, pszClassName, pszWindowName,
		WS_OVERLAPPED | WS_CAPTION,
		0, 0, 1200, 800, NULL, NULL, hInstance, NULL);
	if (hMainWnd == NULL)
		return 0;

	::ShowWindow(hMainWnd, SW_SHOWDEFAULT);
	::UpdateWindow(hMainWnd);

	MSG Message;
	while (::GetMessageA(&Message, NULL, 0, 0))
	{
		::TranslateMessage(&Message);
		::DispatchMessageA(&Message);
	}

	::ShowWindow(hMainWnd, SW_SHOWDEFAULT);
	HANDLE hFile;
	if ((hFile = ::CreateFileA("C:\\Users\\Escap\\Desktop\\Result.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return 0;
	if (!::WriteFile(hFile, pszResult.GetBuffer(), pszResult.GetLength(), NULL, NULL))
		return 0;
	::CloseHandle(hFile);

	return 0;
}

HWND Button_Start, Button_Stop, Button_Delete, Button_Change, Button_StartEx, Label_Interval, Edit_Interval, Button_Hide, Button_Exit;
HWND Label_Time;
HWND ListView_DataList;

HWND ListBox_TerminatePathList;
HWND Button_StartTerminatePath;
HWND Image_TerminatePath_Cursor;
HWND Edit_TerminatePath;
HWND Button_TerminatePath_Add, Button_TerminatePath_Delete;

HWND ListBox_TerminateNameList;
HWND Button_StartTerminateName;
HWND Image_TerminateName_Cursor;
HWND Edit_TerminateName;
HWND Button_TerminateName_Add, Button_TerminateName_Delete;

HWND ListBox_TaskNameList;
HWND Button_StartTaskName;
HWND Image_TaskName_Cursor;
HWND Edit_TaskName;
HWND Button_TaskName_Add, Button_TaskName_Delete;

HWND Label_LaunchTime;
HWND Label_TerminateTime;
HWND Label_TaskTime;
HWND Label_TaskRatio;

HWND Edit_Password;

HANDLE hThread_Main, hThread_Time;

unsigned int nIndex;

LVITEMA Item;

unsigned int nStartTime;
unsigned int nCurrentTime;
unsigned int nEntireTime;
unsigned int nEffectTime;

char pszEntireTime[10];
char pszTerminateTime[5];
char pszEffectTime[10];
char pszEffectRatio[30];

char pszPassword[20];
/*-1: Breaking
0: Unlocked
1: Reinput
100: Locked*/
int nLockStatus = 0;

DWORD __stdcall ThreadProc_Main(LPVOID lpParameter)
{
	HWND hCurr, hPrev = NULL;
	unsigned long long nWndStartTime = ::GetTickCount64(), nWndEndTime;
	char pszWndName[256] = { 0 };
	char pszPrevWndName[256] = { 0 };
	char pszThreadName[256] = { 0 };
	unsigned long dwThreadID = 0;

	char pszIndex[10];
	char pszHWND[10];
	char pszDuration[20];

	SYSTEMTIME sStartTime;
	SYSTEMTIME sEndTime;
	::GetLocalTime(&sStartTime);

	char pszStartTime[13], pszEndTime[13];

	short nStat = 0;
	bool bTask = false;

	Item.mask = LVIF_TEXT;
	while (true)
	{
		hCurr = ::GetAncestor(::GetForegroundWindow(), GA_ROOT);
		if (hCurr != NULL)
		{
			if (hCurr != hPrev)
			{
				nWndEndTime = ::GetTickCount64();
				::GetLocalTime(&sEndTime);
				::GetWindowTextA(hCurr, pszWndName, 256);
				Item.iItem = ::SendMessageA(ListView_DataList, LVM_GETITEMCOUNT, 0, 0);
				Item.iSubItem = 0;
				::wsprintfA(pszIndex, "%d", Item.iItem + 1);
				Item.pszText = pszIndex;
				//Item.pszText = EspConvert::ToString((unsigned long)hCurr).GetBuffer();
				::SendMessageA(ListView_DataList, LVM_INSERTITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 1;
				Item.pszText = (char*)"窗口变化";
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 2;
				Item.pszText = pszPrevWndName;
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 3;
				::GetWindowThreadProcessId(hPrev, &dwThreadID);
				::K32GetModuleFileNameExA((HMODULE)::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwThreadID), NULL, pszThreadName, 256);
				Item.pszText = pszThreadName;
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 4;
				::sprintf_s(pszStartTime, "%.2d:%.2d:%.2d", sStartTime.wHour, sStartTime.wMinute, sStartTime.wSecond);
				Item.pszText = pszStartTime;
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 5;
				::sprintf_s(pszEndTime, "%.2d:%.2d:%.2d", sEndTime.wHour, sEndTime.wMinute, sEndTime.wSecond);
				Item.pszText = pszEndTime;
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 6;
				::sprintf_s(pszDuration, "%.2f秒", (double)(nWndEndTime - nWndStartTime) / 1000);
				Item.pszText = pszDuration;
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 7;
				if (bTerminateName)
					switch (nStat)
					{
					case 0:Item.pszText = (char*)"正常"; break;
					case 1:Item.pszText = (char*)"拦截成功"; break;
					case -1:Item.pszText = (char*)"拦截失败"; break;
					}
				else
					Item.pszText = (char*)"N/A";
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				Item.iSubItem = 8;
				if (bTaskName)
					if (bTask)
					{
						Item.pszText = (char*)"是";
						nEffectTime += (nWndEndTime - nWndStartTime) / 1000;
						::sprintf_s(pszEffectTime, "%.2d:%.2d:%.2d", nEffectTime / 3600, (nEffectTime / 60) % 60, nEffectTime % 60);
						::SetWindowTextA(Label_TaskTime, pszEffectTime);
						::sprintf_s(pszEffectRatio, "%.2f%%", (nEntireTime ? (((double)nEffectTime / nEntireTime) * 100) : 0.0));
						::SetWindowTextA(Label_TaskRatio, pszEffectRatio);
					}
					else
						Item.pszText = (char*)"否";
				else
					Item.pszText = (char*)"N/A";
				::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
				::SendMessageA(ListView_DataList, LVM_ENSUREVISIBLE, SendMessageA(ListView_DataList, LVM_GETITEMCOUNT, 0, 0) - 1, false);

				if (bTerminateName)
				{
					nStat = 0;
					for (unsigned int T = 0; T < arrTerminateName.GetCount(); ++T)
						if (::strstr(pszWndName, arrTerminateName.GetElementAt(T).GetAnsiStr()))
						{
							::GetWindowThreadProcessId(hCurr, &dwThreadID);
							if (::TerminateProcess(::OpenProcess(PROCESS_TERMINATE, FALSE, dwThreadID), 0))
							{
								nStat = 1;
								::sprintf_s(pszTerminateTime, "%d", ++nTerminateTime);
								::SetWindowTextA(Label_TerminateTime, pszTerminateTime);
								break;
							}
							else
							{
								nStat = -1;
								break;
							}
						}
				}

				if (bTaskName)
				{
					bTask = false;
					for (unsigned int T = 0; T < arrTaskName.GetCount(); ++T)
						if (::strstr(pszWndName, arrTaskName.GetElementAt(T).GetAnsiStr()))
						{
							bTask = true;
							break;
						}
				}

				nWndStartTime = nWndEndTime;
				sStartTime = sEndTime;
				hPrev = hCurr;
				::strcpy_s(pszPrevWndName, pszWndName);

			}
			else
			{
				::GetWindowTextA(hCurr, pszWndName, 256);
				if (::strcmp(pszPrevWndName, pszWndName) && pszWndName != NULL)
				{
					nWndEndTime = ::GetTickCount64();
					::GetLocalTime(&sEndTime);
					::GetWindowTextA(hCurr, pszWndName, 256);
					Item.iItem = ::SendMessageA(ListView_DataList, LVM_GETITEMCOUNT, 0, 0);
					Item.iSubItem = 0;
					::wsprintfA(pszIndex, "%d", Item.iItem + 1);
					Item.pszText = pszIndex;
					//Item.pszText = EspConvert::ToString((unsigned long)hCurr).GetBuffer();
					::SendMessageA(ListView_DataList, LVM_INSERTITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 1;
					Item.pszText = (char*)"标题变化";
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 2;
					Item.pszText = pszPrevWndName;
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 3;
					::GetWindowThreadProcessId(hPrev, &dwThreadID);
					::K32GetModuleFileNameExA((HMODULE)::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwThreadID), NULL, pszThreadName, 256);
					Item.pszText = pszThreadName;
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 4;
					::sprintf_s(pszStartTime, "%.2d:%.2d:%.2d", sStartTime.wHour, sStartTime.wMinute, sStartTime.wSecond);
					Item.pszText = pszStartTime;
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 5;
					::sprintf_s(pszEndTime, "%.2d:%.2d:%.2d", sEndTime.wHour, sEndTime.wMinute, sEndTime.wSecond);
					Item.pszText = pszEndTime;
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 6;
					::sprintf_s(pszDuration, "%.2f秒", (double)(nWndEndTime - nWndStartTime) / 1000);
					Item.pszText = pszDuration;
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 7;
					if (bTerminateName)
						switch (nStat)
						{
						case 0:Item.pszText = (char*)"正常"; break;
						case 1:Item.pszText = (char*)"拦截成功"; break;
						case -1:Item.pszText = (char*)"拦截失败"; break;
						}
					else
						Item.pszText = (char*)"N/A";
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					Item.iSubItem = 8;
					if (bTaskName)
						if (bTask)
						{
							Item.pszText = (char*)"是";
							nEffectTime += (nWndEndTime - nWndStartTime) / 1000;
							::sprintf_s(pszEffectTime, "%.2d:%.2d:%.2d", nEffectTime / 3600, (nEffectTime / 60) % 60, nEffectTime % 60);
							::SetWindowTextA(Label_TaskTime, pszEffectTime);
							::sprintf_s(pszEffectRatio, "%.2f%%", (nEntireTime ? (double)((double)(nEffectTime / nEntireTime) * 100) : 0.0));
							::SetWindowTextA(Label_TaskRatio, pszEffectRatio);
						}
						else
							Item.pszText = (char*)"否";
					else
						Item.pszText = (char*)"N/A";
					::SendMessageA(ListView_DataList, LVM_SETITEMA, 0, (LPARAM)&Item);
					::SendMessageA(ListView_DataList, LVM_ENSUREVISIBLE, SendMessageA(ListView_DataList, LVM_GETITEMCOUNT, 0, 0) - 1, false);

					if (bTerminateName)
					{
						nStat = 0;
						for (unsigned int T = 0; T < arrTerminateName.GetCount(); ++T)
						{
							if (::strstr(pszWndName, arrTerminateName.GetElementAt(T).GetAnsiStr()))
							{
								::GetWindowThreadProcessId(hCurr, &dwThreadID);
								if (::TerminateProcess(::OpenProcess(PROCESS_TERMINATE, FALSE, dwThreadID), 0))
								{
									nStat = 1;
									::sprintf_s(pszTerminateTime, "%d", ++nTerminateTime);
									::SetWindowTextA(Label_TerminateTime, pszTerminateTime);
									break;
								}
								else
								{
									nStat = -1;
									break;
								}
							}
						}
					}
					if (bTaskName)
					{
						bTask = false;
						for (unsigned int T = 0; T < arrTaskName.GetCount(); ++T)
							if (::strstr(pszWndName, arrTaskName.GetElementAt(T).GetAnsiStr()))
							{
								bTask = true;
								break;
							}
					}

					nWndStartTime = nWndEndTime;
					sStartTime = sEndTime;
					::strcpy_s(pszPrevWndName, pszWndName);
				}
			}
		}
	}

	return 0;
}

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		RECT Rect_Pos = { 0 };
		GetWindowRect(hWnd, &Rect_Pos);
		SetWindowPos(hWnd, HWND_TOP, ((GetSystemMetrics(SM_CXSCREEN) - Rect_Pos.right) / 2), ((GetSystemMetrics(SM_CYSCREEN) - Rect_Pos.bottom) / 2), Rect_Pos.right, Rect_Pos.bottom, SWP_SHOWWINDOW);
		hCurProc = ::LoadCursorA(hInst, MAKEINTRESOURCEA(IDC_CURSOR_PROCESSING));
		hIcoNormal = ::LoadIconA(hInst, MAKEINTRESOURCEA(IDI_ICON_NORMAL));
		hIcoProc = ::LoadIconA(hInst, MAKEINTRESOURCEA(IDI_ICON_PROCESSING));


		::SendMessageA((Button_Start = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "开始", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, 8, 80, 24, hWnd, (HMENU)1001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_Stop = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "停止", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 96, 8, 80, 24, hWnd, (HMENU)1002, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_Delete = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "清空", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 184, 8, 80, 24, hWnd, (HMENU)1003, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_Change = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "更换模式", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 272, 8, 80, 24, hWnd, (HMENU)1004, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_StartEx = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "定时开始", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 360, 8, 80, 24, hWnd, (HMENU)1005, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_Interval = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "执行间隔", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 448, 8, 48, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Edit_Interval = ::CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_NUMBER, 503, 8, 32, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_Hide = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "隐藏屏蔽窗口", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 544, 8, 80, 24, hWnd, (HMENU)1006, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		::SendMessageA((::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "剩余时间", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 8, 36, 64, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_Time = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 80, 36, 738, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		::SendMessageA((ListView_DataList = ::CreateWindowExA(WS_EX_LEFT, WC_LISTVIEWA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, 8, 64, 1168, 400, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA(ListView_DataList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
		{
			LVCOLUMNA Column;
			Column.mask = LVCF_WIDTH | LVCF_TEXT;
			Column.cx = 36;
			Column.pszText = (char*)"序号";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 0, (LPARAM)&Column);
			Column.cx = 60;
			Column.pszText = (char*)"变换类型";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 1, (LPARAM)&Column);
			Column.cx = 380;
			Column.pszText = (char*)"窗口标题";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 2, (LPARAM)&Column);
			Column.cx = 380;
			Column.pszText = (char*)"进程名称";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 3, (LPARAM)&Column);
			Column.cx = 60;
			Column.pszText = (char*)"开始时间";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 4, (LPARAM)&Column);
			Column.cx = 60;
			Column.pszText = (char*)"结束时间";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 5, (LPARAM)&Column);
			Column.cx = 60;
			Column.pszText = (char*)"持续时间";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 6, (LPARAM)&Column);
			Column.cx = 60;
			Column.pszText = (char*)"拦截状态";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 7, (LPARAM)&Column);
			Column.cx = 36;
			Column.pszText = (char*)"任务";
			::SendMessageA(ListView_DataList, LVM_INSERTCOLUMNA, 8, (LPARAM)&Column);
		}//Column

		::SendMessageA((ListBox_TerminateNameList = ::CreateWindowExA(WS_EX_LEFT, WC_LISTBOXA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_HASSTRINGS | LBS_EXTENDEDSEL, 8, 468, 584, 120, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_StartTerminateName = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "开始检测", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, 582, 80, 24, hWnd, (HMENU)40001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		Image_TerminateName_Cursor = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY, 92, 578, 32, 32, hWnd, (HMENU)19999, hInst, NULL);
		::SendMessageA(Image_TerminateName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
		::SendMessageA((Edit_TerminateName = ::CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 128, 582, 392, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TerminateName_Add = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "添加", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 524, 582, 32, 24, hWnd, (HMENU)50001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TerminateName_Delete = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "删除", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 560, 582, 32, 24, hWnd, (HMENU)50002, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		::SendMessageA((ListBox_TerminatePathList = ::CreateWindowExA(WS_EX_LEFT, WC_LISTBOXA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_HASSTRINGS | LBS_EXTENDEDSEL, 8, 610, 584, 120, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_StartTerminatePath = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "开始检测", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, 724, 80, 24, hWnd, (HMENU)20001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		Image_TerminatePath_Cursor = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY, 92, 720, 32, 32, hWnd, (HMENU)9999, hInst, NULL);
		::SendMessageA(Image_TerminatePath_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
		::SendMessageA((Edit_TerminatePath = ::CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 128, 724, 392, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TerminatePath_Add = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "添加", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 524, 724, 32, 24, hWnd, (HMENU)30001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TerminatePath_Delete = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "删除", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 560, 724, 32, 24, hWnd, (HMENU)30002, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		::SendMessageA((ListBox_TaskNameList = ::CreateWindowExA(WS_EX_LEFT, WC_LISTBOXA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_HASSTRINGS | LBS_EXTENDEDSEL, 596, 468, 584, 120, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_StartTaskName = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "开始检测", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 596, 582, 80, 24, hWnd, (HMENU)60001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		Image_TaskName_Cursor = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY, 680, 578, 32, 32, hWnd, (HMENU)29999, hInst, NULL);
		::SendMessageA(Image_TaskName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
		::SendMessageA((Edit_TaskName = ::CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 716, 582, 392, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TaskName_Add = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "添加", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1112, 582, 32, 24, hWnd, (HMENU)7001, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Button_TaskName_Delete = ::CreateWindowExA(WS_EX_LEFT, WC_BUTTONA, "删除", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1148, 582, 32, 24, hWnd, (HMENU)7002, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		::SendMessageA((::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "已执行时间", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 596, 610, 84, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_LaunchTime = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "00:00:00", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 684, 610, 496, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "已拦截次数", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 596, 638, 84, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_TerminateTime = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "0", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 684, 638, 496, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "有效时间", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 596, 666, 84, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_TaskTime = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "00:00:00", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 684, 666, 496, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "有效时间占比", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 596, 694, 84, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);
		::SendMessageA((Label_TaskRatio = ::CreateWindowExA(WS_EX_LEFT, WC_STATICA, "0.00%", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE, 684, 694, 496, 24, hWnd, NULL, hInst, NULL)), WM_SETFONT, (WPARAM)hFont, NULL);

		hThread_Main = ::CreateThread(NULL, NULL, ThreadProc_Main, NULL, CREATE_SUSPENDED, NULL);

		break;
	}

	case WM_DESTROY:
	{
		::CloseHandle(hThread_Main);
		::KillTimer(hWnd, 12345);
		::PostQuitMessage(0);
		break;
	}

	case WM_COMMAND:
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case 1001:
			{
				if (nLockStatus == 0)
				{
					hPwWnd = ::CreateWindowExA(WS_EX_TOPMOST, pszClassName_Pw, pszWindowName_Pw, WS_OVERLAPPED | WS_CAPTION, 0, 0, 300, 69, NULL, NULL, hInst, 0);
					if (hPwWnd == NULL)
						return 0;
					::ShowWindow(hPwWnd, SW_SHOWDEFAULT);
					::UpdateWindow(hPwWnd);
					::SetForegroundWindow(hPwWnd);
				}
				else
					::MessageBoxExA(NULL, "已经开始了", NULL, MB_OK, 0);
				break;
			}
			case 1002:
			{
				if (nLockStatus == 100)
				{
					hPwWnd = ::CreateWindowExA(WS_EX_TOPMOST, pszClassName_Pw, pszWindowName_Pw, WS_OVERLAPPED | WS_CAPTION, 0, 0, 300, 69, NULL, NULL, hInst, 0);
					if (hPwWnd == NULL)
						return 0;
					::ShowWindow(hPwWnd, SW_SHOWDEFAULT);
					::UpdateWindow(hPwWnd);
					::SetForegroundWindow(hPwWnd);
				}
				else
					::MessageBoxExA(NULL, "还没有开始", NULL, MB_OK, 0);
				break;
			}
			case 20001:
			{
				if (bTerminatePath)
				{
					bTerminatePath = false;
					::SetWindowTextA(Button_StartTerminatePath, "开始检测");
				}
				else
				{
					bTerminatePath = true;
					::SetWindowTextA(Button_StartTerminatePath, "停止检测");
				}
				break;
			}
			case 30001:
			{
				if (dwPointThreadID)
				{
					for (unsigned int T = 0; T < arrTerminate.GetCount(); ++T)
						if (arrTerminate.GetElementAt(T) == dwPointThreadID)
							return 0;
					::SendMessageA(ListBox_TerminatePathList, LB_ADDSTRING, 0, (long)pszPointThreadName);
					arrTerminate.AddElement(dwPointThreadID);
					arrTerminatePath.AddElement(pszPointThreadName);
				}
				break;
			}
			case 40001:
			{
				if (bTerminateName)
				{
					bTerminateName = false;
					::SetWindowTextA(Button_StartTerminateName, "开始检测");
				}
				else
				{
					bTerminateName = true;
					::SetWindowTextA(Button_StartTerminateName, "停止检测");
				}
				break;
			}
			case 50001:
			{
				if (::GetWindowTextLengthA(Edit_TerminateName))
				{
					::GetWindowTextA(Edit_TerminateName, pszPointWndName, 256);
					for (unsigned int T = 0; T < arrTerminateName.GetCount(); ++T)
						if (!::strcmp(arrTerminateName.GetElementAt(T), pszPointWndName))
							return 0;
					::SendMessageA(ListBox_TerminateNameList, LB_ADDSTRING, 0, (long)pszPointWndName);
					arrTerminateName.AddElement(pszPointWndName);
					::SetWindowTextA(Edit_TerminateName, NULL);
				}
				break;
			}
			case 50002:
			{
				nIndex = ::SendMessageA(ListBox_TerminateNameList, LB_GETCURSEL, 0, 0);
				if (nIndex != LB_ERR)
				{
					arrTerminateName.DeleteElement(nIndex);
					::SendMessageA(ListBox_TerminateNameList, LB_DELETESTRING, nIndex, 0);
				}
				break;
			}
			case 60001:
			{
				if (bTaskName)
				{
					bTaskName = false;
					::SetWindowTextA(Button_StartTaskName, "开始检测");
				}
				else
				{
					bTaskName = true;
					::SetWindowTextA(Button_StartTaskName, "停止检测");
				}
				break;
			}
			case 7001:
			{
				if (::GetWindowTextLengthA(Edit_TaskName))
				{
					::GetWindowTextA(Edit_TaskName, pszPointWndName, 256);
					for (unsigned int T = 0; T < arrTaskName.GetCount(); ++T)
						if (!::strcmp(arrTaskName.GetElementAt(T), pszPointWndName))
							return 0;
					::SendMessageA(ListBox_TaskNameList, LB_ADDSTRING, 0, (long)pszPointWndName);
					arrTaskName.AddElement(pszPointWndName);
					::SetWindowTextA(Edit_TaskName, NULL);
				}
				break;
			}
			case 7002:
			{
				nIndex = ::SendMessageA(ListBox_TaskNameList, LB_GETCURSEL, 0, 0);
				if (nIndex != LB_ERR)
				{
					arrTaskName.DeleteElement(nIndex);
					::SendMessageA(ListBox_TaskNameList, LB_DELETESTRING, nIndex, 0);
				}
				break;
			}
			}

		}

		if (HIWORD(wParam) == STN_CLICKED)
		{
			if (LOWORD(wParam) == 9999)
			{
				if (bProcPath == false)
				{
					bProcPath = true;
					::SendMessageA(Image_TerminatePath_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoProc);
					hCurNormal = ::SetCursor(hCurProc);
					::SetCapture(hWnd);
				}
			}
			if (LOWORD(wParam) == 19999)
			{
				if (bProcName == false)
				{
					bProcName = true;
					::SendMessageA(Image_TerminateName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoProc);
					hCurNormal = ::SetCursor(hCurProc);
					::SetCapture(hWnd);
				}
			}
			if (LOWORD(wParam) == 29999)
			{
				if (bProcTask == false)
				{
					bProcTask = true;
					::SendMessageA(Image_TaskName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoProc);
					hCurNormal = ::SetCursor(hCurProc);
					::SetCapture(hWnd);
				}
			}
		}

		break;

	}

	case WM_TIMER:
	{
		if (wParam == 12345)
		{
			::sprintf_s(pszEntireTime, "%.2d:%.2d:%.2d", nEntireTime / 3600, (nEntireTime / 60) % 60, nEntireTime % 60);
			::SetWindowTextA(Label_LaunchTime, pszEntireTime);


			++nEntireTime;
		}
		break;
	}

	case WM_LBUTTONUP:
	{
		if (bProcPath == true)
		{
			bProcPath = false;
			::SendMessageA(Image_TerminatePath_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
			::SetCursor(hCurNormal);
			::ReleaseCapture();

			POINT sCurrPoint;
			::GetCursorPos(&sCurrPoint);
			::GetWindowThreadProcessId(::WindowFromPoint(sCurrPoint), &dwPointThreadID);
			//arrTerminate.AddElement(dwPointThreadID);
			::K32GetModuleFileNameExA((HMODULE)::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwPointThreadID), NULL, pszPointThreadName, 256);
			::SetWindowTextA(Edit_TerminatePath, pszPointThreadName);
		}
		if (bProcName == true)
		{
			bProcName = false;
			::SendMessageA(Image_TerminateName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
			::SetCursor(hCurNormal);
			::ReleaseCapture();

			POINT sCurrPoint;
			::GetCursorPos(&sCurrPoint);
			::GetWindowTextA(::GetAncestor(::WindowFromPoint(sCurrPoint), GA_ROOT), pszPointWndName, 256);
			::SetWindowTextA(Edit_TerminateName, pszPointWndName);
		}
		if (bProcTask == true)
		{
			bProcTask = false;
			::SendMessageA(Image_TaskName_Cursor, STM_SETIMAGE, IMAGE_ICON, (long)hIcoNormal);
			::SetCursor(hCurNormal);
			::ReleaseCapture();

			POINT sCurrPoint;
			::GetCursorPos(&sCurrPoint);
			::GetWindowTextA(::GetAncestor(::WindowFromPoint(sCurrPoint), GA_ROOT), pszPointWndName, 256);
			::SetWindowTextA(Edit_TaskName, pszPointWndName);
		}

		break;
	}

	default:
		return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

WNDPROC lpfnOldProc;
long __stdcall EditProc_Password(HWND WindowHandle, unsigned int Message, unsigned int wParam, long lParam)
{
	if (Message == WM_KEYDOWN)
		if (wParam == VK_RETURN)
		{
			switch (nLockStatus)
			{
			case 0:
			{
				::GetWindowTextA(WindowHandle, pszPassword, 20);
				::DestroyWindow(hPwWnd);
				hPwWnd = ::CreateWindowExA(WS_EX_TOPMOST, pszClassName_Pw, pszWindowName_Pw2, WS_OVERLAPPED | WS_CAPTION, 0, 0, 300, 69, NULL, NULL, hInst, 0);
				if (hPwWnd == NULL)
					return 0;
				::ShowWindow(hPwWnd, SW_SHOWDEFAULT);
				::UpdateWindow(hPwWnd);
				::SetForegroundWindow(hPwWnd);
				nLockStatus = 1;
				break;
			}
			case 1:
			{
				char pszCheckPassword[20];
				::GetWindowTextA(WindowHandle, pszCheckPassword, 20);
				if (::strcmp(pszPassword, pszCheckPassword))
				{
					::DestroyWindow(hPwWnd);
					::MessageBoxExA(NULL, "两次密码输入不一致！", "错误", MB_OK, 0);
					::memset(pszPassword, 0, 20);
					nLockStatus = 0;
				}
				else
				{
					::DestroyWindow(hPwWnd);
					::ResumeThread(hThread_Main);
					nLockStatus = 100;
					::SetTimer(hMainWnd, 12345, 1000, NULL);
				}
				break;
			}
			case 100:
			{
				char pszUserInput[20];
				::GetWindowTextA(WindowHandle, pszUserInput, 20);
				if (::strcmp(pszPassword, pszUserInput))
				{
					::DestroyWindow(hPwWnd);
					::MessageBoxExA(NULL, "密码错误！", "错误", MB_OK, 0);
				}
				else
				{
					::DestroyWindow(hPwWnd);
					::SuspendThread(hThread_Main);
					nLockStatus = 0;
					::KillTimer(hMainWnd, 12345);
				}
				break;
			}
			}
		}
	return CallWindowProcA(lpfnOldProc, WindowHandle, Message, wParam, lParam);
}
long __stdcall WndProc_Password(HWND WindowHandle, unsigned int Message, unsigned int wParam, long lParam)
{
	switch (Message)
	{
	case WM_CREATE:
	{
		RECT Rect;
		GetWindowRect(WindowHandle, &Rect);
		SetWindowPos(WindowHandle, HWND_TOP, ((GetSystemMetrics(SM_CXSCREEN) - Rect.right) / 2), ((GetSystemMetrics(SM_CYSCREEN) - Rect.bottom) / 2), Rect.right, Rect.bottom, SWP_SHOWWINDOW);

		::SendMessageA((Edit_Password = ::CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD, 0, 0, 284, 30, WindowHandle, NULL, hInst, 0)), WM_SETFONT, (WPARAM)hPwFont, 0);

		lpfnOldProc = (WNDPROC)::SetWindowLongA(Edit_Password, GWL_WNDPROC, (LONG)EditProc_Password);
		::SetFocus(Edit_Password);

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT PaintStruct;
		HDC DCHandle = BeginPaint(WindowHandle, &PaintStruct);
		//添加绘图代码
		EndPaint(WindowHandle, &PaintStruct);
		break;
	}
	default:
		return DefWindowProcA(WindowHandle, Message, wParam, lParam);
	}
	return 0;
}
