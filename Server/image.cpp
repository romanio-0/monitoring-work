#include "image.h"

using namespace Gdiplus;

HBITMAP hBmp;
int bmpWidth;
int bmpHeight;

// taken from the official microsoft documentation
int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
    UINT num = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo *pImageCodecInfo = nullptr;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (Gdiplus::ImageCodecInfo *) (malloc(size));
    if (pImageCodecInfo == nullptr)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;

    int newWidth = LOWORD(lParam);
    int newHeight = HIWORD(lParam);

    int imageWidth;
    int imageHeight;

    static double scale = 1.0;

    switch (message) {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);

            // We draw the image on the window taking into account the scale
            if (hBmp != nullptr) {

                HDC hdcMem = CreateCompatibleDC(hdc);
                SelectObject(hdcMem, hBmp);

                imageWidth = (int)(bmpWidth * scale);
                imageHeight = (int)(bmpHeight * scale);

                // Установим флаг HALFTONE для сглаживания
                SetStretchBltMode(hdc, HALFTONE);

                StretchBlt(hdc, 0, 0, imageWidth, imageHeight,
                           hdcMem, 0, 0, bmpWidth, bmpHeight, SRCCOPY);
                DeleteDC(hdcMem);
            }

            EndPaint(hWnd, &ps);
            break;

        case WM_SIZE:
            // when changing the window size, we recalculate the scale factor

            scale = std::min((double)newWidth / (double)bmpWidth, (double)newHeight / (double)bmpHeight);

            // redraw
            InvalidateRect(hWnd, nullptr, TRUE);
            UpdateWindow(hWnd);
            break;

        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HBITMAP dataToHBmp(screenData *screen){
    return CreateBitmap(screen->width, screen->height, 1,
                        8 * sizeof(RGBQUAD), screen->data);
}

void saveScreen(screenData *screen, const wchar_t *path){
    HBITMAP hBitmap = CreateBitmap(screen->width, screen->height, 1,
                                 8 * sizeof(RGBQUAD), screen->data);
    Bitmap bitmap(hBitmap, nullptr);

    CLSID Clsid;
    GetEncoderClsid(L"image/png", &Clsid);

    bitmap.Save(path, &Clsid, nullptr);
}

void printScreen(screenData* screen) {
    HINSTANCE hInst = nullptr;
    int nCmdShow = SW_SHOW;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);


    // fill out the structure
    WNDCLASSEXA wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc,
                        0, 0, GetModuleHandleA(nullptr),
                         nullptr, nullptr, nullptr, nullptr,
                        "screen", nullptr };
    RegisterClassExA(&wcex);

    HWND hWnd = CreateWindowA("screen", "Screenshot", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              nullptr, nullptr, hInst, nullptr);

    if (!hWnd){
        std::cout << "error create window" << std::endl;
        GdiplusShutdown(gdiplusToken);
        return;
    }

    hBmp = dataToHBmp(screen);
    bmpHeight = screen->height;
    bmpWidth = screen->width;



    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    GdiplusShutdown(gdiplusToken);
}


