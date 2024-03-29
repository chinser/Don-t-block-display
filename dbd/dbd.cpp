// dbd.cpp: Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "dbd.h"
#include <shellapi.h>

#define MAX_LOADSTRING 100

#define WM_FLIPPED_TO_TRAY (WM_APP + 1234)
#define ID_FLIPPED_TO_TRAY 1234

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
HICON hTrayIcon;
HANDLE mutex;
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: разместите код здесь.
	//lstrcpyW(szTitle, _T("Don't block display")); // смотреть в dbd.rc


	mutex = CreateMutex(NULL, true, L"MutexOfTheDBD");
	DWORD result;
	result = WaitForSingleObject(mutex, 0);
	if (result != WAIT_OBJECT_0)
	{
		return FALSE;
	}

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DBD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DBD));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DBD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DBD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	
	hTrayIcon = wcex.hIconSm;

    return RegisterClassExW(&wcex);
}


//https://rsdn.org/article/winshell/shellicons.xml
//описание сворачивания в трей
BOOL FlipToTray(HWND hWnd, HICON hIcon, BOOL bMinimize)
{
	// создаем иконку
	NOTIFYICONDATA nid; memset(&nid, 0, sizeof(nid)); nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = ID_FLIPPED_TO_TRAY;
	nid.uCallbackMessage = WM_FLIPPED_TO_TRAY;
	nid.hIcon = hIcon;
	GetWindowText(hWnd, nid.szTip,
		sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	// показываем ее
	BOOL ok = Shell_NotifyIcon(NIM_ADD, &nid);

	// прячем окно
	if (bMinimize)
		ShowWindow(hWnd, SW_MINIMIZE);

	if (ok) // только, если удалось показать иконку
		ShowWindow(hWnd, SW_HIDE);

	return ok;
}

BOOL UnflipFromTray(HWND hWnd, BOOL bRestore)
{
	// идентифицируем иконку
	NOTIFYICONDATA nid; memset(&nid, 0, sizeof(nid)); nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = ID_FLIPPED_TO_TRAY;

	// удаляем ее
	BOOL ok = Shell_NotifyIcon(NIM_DELETE, &nid);

	if (!bRestore) return ok;

	// восстанавливаем окно
	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(hWnd, SW_RESTORE);

	return ok;
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 300, 200, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	FlipToTray(hWnd, hTrayIcon, TRUE);

	return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//не даём заблочиться экрану
	//https://msdn.microsoft.com/en-us/library/windows/desktop/aa373208.aspx

	EXECUTION_STATE _state =
	SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

	switch (message)
	{
	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) FlipToTray(hWnd, hTrayIcon, FALSE);
		}
		break;
	case WM_FLIPPED_TO_TRAY:
		{
		if (wParam == ID_FLIPPED_TO_TRAY && lParam == WM_LBUTTONDBLCLK) UnflipFromTray(hWnd, TRUE);
		}
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		ReleaseMutex(mutex);
		CloseHandle(mutex);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
