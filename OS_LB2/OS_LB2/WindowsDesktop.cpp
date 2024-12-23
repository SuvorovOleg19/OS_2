#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#pragma comment(lib, "Msimg32.lib")


HBITMAP hBitmap = NULL;
TCHAR filePath[MAX_PATH] = _T("");
HWND hButton;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL SelectFile(HWND hWnd) {
    OPENFILENAME ofn = { sizeof(OPENFILENAME), hWnd, NULL, _T("Bitmap Files\0*.bmp\0All Files\0*.*\0"), NULL, 0, 0, filePath, MAX_PATH };
    return GetOpenFileName(&ofn);
}

COLORREF GetTransparentColor(HBITMAP hBitmap) {
    // �������� ���� �������� ������ ������� ��� ����������
    HDC hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hBitmap);

    COLORREF transparentColor = GetPixel(hdc, 0, 0); // ���� �������� ������ �������
    DeleteDC(hdc);
    return transparentColor;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, _T("ResizableBitmapWindow") };
    if (!RegisterClass(&wc)) return MessageBox(NULL, _T("������ ����������� ������!"), _T("������"), MB_OK), 1;

    HWND hWnd = CreateWindow(wc.lpszClassName, _T("������������ ������ 2"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hWnd) return MessageBox(NULL, _T("������ �������� ����!"), _T("������"), MB_OK), 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hBitmap) DeleteObject(hBitmap);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static COLORREF transparentColor = RGB(255, 255, 255); // ���������� ���� �� ���������

    switch (message) {
    case WM_CREATE:
        hButton = CreateWindow(_T("BUTTON"), _T("����� �����"), WS_VISIBLE | WS_CHILD, 10, 10, 120, 30, hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1 && SelectFile(hWnd)) {
            if (hBitmap) DeleteObject(hBitmap);
            hBitmap = (HBITMAP)LoadImage(NULL, filePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if (hBitmap) {
                transparentColor = GetTransparentColor(hBitmap); // ������������� ���������� ����
            }
            else {
                MessageBox(hWnd, _T("�� ������� ��������� �����������!"), _T("������"), MB_OK);
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT clientRect; GetClientRect(hWnd, &clientRect);

        if (hBitmap) {
            HDC hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hBitmap);

            BITMAP bitmap; GetObject(hBitmap, sizeof(BITMAP), &bitmap);

            int newWidth = clientRect.right / 2; // ������ ����������� ����� ���������������
            int newHeight = clientRect.bottom / 2;  // ������ ����������� ����� ���������������
            
            // ���������� ��� ���������� ������ �����
            int x = clientRect.right - newWidth;   // ������ �� ������ ����
            int y = clientRect.bottom - newHeight; // ������ �� ������ ����

            RECT buttonRect;
            GetWindowRect(hButton, &buttonRect);
            ScreenToClient(hWnd, (LPPOINT)&buttonRect.left);
            ScreenToClient(hWnd, (LPPOINT)&buttonRect.right);

            if (y < buttonRect.bottom) y = buttonRect.bottom + 10;

            // ������ ����������� � ������ ����������� �����
            TransparentBlt(hdc, x, y, newWidth, newHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, transparentColor);

            DeleteDC(hdcMem);
        }
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE: {
        int width = LOWORD(lParam), height = HIWORD(lParam);
        SetWindowPos(hButton, NULL, 10, 10, width / 4, height / 10, SWP_NOZORDER);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
