// analogClock.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <math.h>
#include <time.h>

#include "analogClock.h"

#define IDT_TIMER1  2002
#define MAX_LOADSTRING 100
const double PI=3.1415926535897932384626433832795;
const double DEG2RAD=PI/180;

// Global Variables:
HINSTANCE hInst;								// current instance

typedef struct face
{
    int       xCenter;
    int       yCenter;
    int       radius;
} faceT, *pfaceT;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void getDialFaceRect(HWND, pfaceT*);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND       hWnd;
    DWORD      dwErr;
    int        nRet;
	MSG        msg;
    TCHAR      szTitle[MAX_LOADSTRING];					// The title bar text
    TCHAR      szWindowClass[MAX_LOADSTRING];			// the main window class name

    // instantiate a black brush for background
    HBRUSH  blkBrush = CreateSolidBrush(RGB(0,0,0));

    // Initialize strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ANALOGCLOCK, szWindowClass, MAX_LOADSTRING);
		
    // instantiate and set window class structure
    memset((void*)&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
    wcex.hbrBackground = blkBrush;
	wcex.lpszClassName	= szWindowClass;

	ATOM aRet = RegisterClassEx(&wcex);
    if(0 != aRet)
    {
        hInst = hInstance; // Store instance handle in our global variable
        // TODO : make window full screen
        // get size of the desktop, we want our application to fill the entire screen
        HWND  hDesktop = GetDesktopWindow(); 
        RECT  rcDesktop;

        // TODO: this should be the client area of the desktop, not the fulldesktop.
        GetWindowRect(hDesktop, &rcDesktop);

        hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, rcDesktop.right, rcDesktop.bottom, NULL, NULL, hInstance, NULL);

        if(NULL != hWnd)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);

            // create our one second timer...
            SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC) NULL);

	        while (GetMessage(&msg, NULL, 0, 0))
	        {
                TranslateMessage(&msg);
			    DispatchMessage(&msg);
	        }

            nRet = msg.wParam;
            KillTimer(hWnd, IDT_TIMER1);
        }
        else                       // create windows failed
        {
            dwErr = GetLastError();
            printf("%d\n", dwErr);
            nRet = dwErr;
        }
    }
    else                            // register clas failed
    {
        dwErr = GetLastError();
        printf("%d\n", dwErr);
        nRet = dwErr;
    }

    DeleteObject(blkBrush);

    return (nRet);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int                wmId, wmEvent;
    //DWORD              dwErr;
	PAINTSTRUCT        ps;
    static pfaceT      lpFace = NULL;
    static int         prevMin = -1;
    //static int         prevHour = -1;
    static bool        bRedrawMin = true;
    static bool        bRedrawHour = true;
    static POINT       minHand[HAND_PNTS] = {{0,0}};
    static POINT       hourHand[HAND_PNTS] = {{0,0}};
	HDC                hdc;

	switch (message)
	{
	    case WM_COMMAND:
		    wmId    = LOWORD(wParam);
		    wmEvent = HIWORD(wParam);
		    // Parse the menu selections:
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
		    break;

        case WM_CREATE:
            {
                getDialFaceRect(hWnd, &lpFace);
            }
            break;

	    case WM_PAINT:
		    hdc = BeginPaint(hWnd, &ps);

            if(NULL != lpFace)
            {
                RECT   rcTarget;
                int    xEnd, yEnd;
                int    nHour, nMin, nSec = 10;
                double deg;

                // TODO : get current time....
                struct tm newtime;
                __time64_t long_time;
                TCHAR timebuf[26];
          
  
                // Get time as 64-bit integer, and convert to local time...
                _time64( &long_time ); 
                _localtime64_s( &newtime, &long_time ); 
                // ...extract the components we need...
                nHour = (newtime.tm_hour > 12 ? newtime.tm_hour-12 : newtime.tm_hour);   // keep things on a 12-hour scale
                nMin = newtime.tm_min;
                nSec = newtime.tm_sec;
         
                // draw the clock face...
                SelectObject(ps.hdc, GetStockObject(GRAY_BRUSH)); 
                rcTarget.left = lpFace->xCenter - lpFace->radius;
                rcTarget.right = lpFace->xCenter + lpFace->radius;
                rcTarget.top = lpFace->yCenter - lpFace->radius;
                rcTarget.bottom = lpFace->yCenter + lpFace->radius;
                Ellipse(ps.hdc, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom); 

                // render the digits on the clock face...
                // set font color and size
                HFONT hFont = CreateFont(32,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                                         CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,TEXT("Impact"));
                SelectObject(hdc, hFont);                
                SetTextColor(hdc, RGB(255,0,0));

                for(int ndx = 1; ndx <= 12; ndx++)
                {
                    TCHAR          buf[3];
                    swprintf(buf, L"%2d", ndx);
                    deg = PI - DEG2RAD * ndx * 30;
                    xEnd = (int)(lpFace->xCenter + (lpFace->radius*0.95*sin(deg)));
                    yEnd = (int)(lpFace->yCenter + (lpFace->radius*0.95*cos(deg)));
                    RECT rcText = {xEnd, yEnd, 0, 0};
                    MoveToEx(ps.hdc, xEnd, yEnd, NULL);
                    DrawText(ps.hdc, buf, wcslen(buf), &rcText, DT_CALCRECT);
                    DrawText(ps.hdc, buf, wcslen(buf), &rcText, DT_BOTTOM | DT_NOCLIP);
                }
                DeleteObject(hFont);

                // draw center pin...
                SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
                rcTarget.left = lpFace->xCenter - 3;
                rcTarget.right = lpFace->xCenter + 3;
                rcTarget.top = lpFace->yCenter - 3;
                rcTarget.bottom = lpFace->yCenter + 3;
                Ellipse(ps.hdc, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);

                // draw second hand...
                deg = PI - DEG2RAD* nSec * 6;                       // 6 degrees per second, also account for y-values increasing downward
                MoveToEx(ps.hdc, lpFace->xCenter, lpFace->yCenter, NULL);
                xEnd = (int)(lpFace->xCenter + (lpFace->radius * sin(deg)));
                yEnd = (int)(lpFace->yCenter + (lpFace->radius * cos(deg)));
                LineTo(ps.hdc, xEnd, yEnd);

                // draw minute hand...
                if((prevMin != nMin) || bRedrawMin)                        // minute hand has moved, update coordinates
                {
                    deg = PI - DEG2RAD * nMin * 6;                         // 6 degrees per minute, also account for y-values increasing downward
                    for(int ndx = 0; ndx < HAND_PNTS; ndx++)
                    {
                        // scale the template and copy into working copy
                        minHand[ndx].x = (int)handTemplate[ndx].x;
                        minHand[ndx].y = (int)(lpFace->radius * handTemplate[ndx].y);
                    
                        // rotate the working copy
                        //  | cos(f)    sin(f) | | x |   |  x*cos(f) + y*sin(f)  |
                        //  | -sin(f)   cos(f) | | y |   | -x*sin(f) + y*cos(f) |
                        double x = minHand[ndx].x * cos(deg) + minHand[ndx].y * sin(deg);
                        double y = minHand[ndx].y * cos(deg) - minHand[ndx].x * sin(deg); 
                        minHand[ndx].x = (int)x;
                        minHand[ndx].y = (int)y;
                    
                        // todo: translate the working copy
                        minHand[ndx].x += lpFace->xCenter;
                        minHand[ndx].y += lpFace->yCenter; 
                    }
                    bRedrawMin = false;
                    prevMin = nMin;                      // store current minute so we know when we have to repeat this calc.
                }
                // render the working copy
                Polygon(ps.hdc, minHand, HAND_PNTS);

                // draw hour hand ....
                if((nMin % 15 == 0) || bRedrawHour)                         // only update coordinates on quarter hour intervals, or on resize
                {
                    deg = PI - DEG2RAD * (nHour * 30 + 7.5*(nMin / 15));    // 30 degrees per minute, also account for y-values increasing downward
                    for(int ndx = 0; ndx < HAND_PNTS; ndx++)
                    {
                        // scale the template and copy into working copy, hour hand shorter and fatter
                        hourHand[ndx].x = (int)(1.5 * handTemplate[ndx].x);
                        hourHand[ndx].y = (int)(0.8 * lpFace->radius * handTemplate[ndx].y);
                    
                        // rotate the working copy
                        //  | cos(f)    sin(f) | | x |   |  x*cos(f) + y*sin(f)  |
                        //  | -sin(f)   cos(f) | | y |   | -x*sin(f) + y*cos(f) |
                        double x = hourHand[ndx].x * cos(deg) + hourHand[ndx].y * sin(deg);
                        double y = hourHand[ndx].y * cos(deg) - hourHand[ndx].x * sin(deg); 
                        hourHand[ndx].x = (int)x;
                        hourHand[ndx].y = (int)y;
                    
                        // todo: translate the working copy
                        hourHand[ndx].x += lpFace->xCenter;
                        hourHand[ndx].y += lpFace->yCenter; 
                    }

                    bRedrawHour = false;
                }
                Polygon(ps.hdc, hourHand, HAND_PNTS);
            }
            else
            {
                printf("face is null!!");
            }
		    EndPaint(hWnd, &ps);
		    break;


	    case WM_DESTROY:
		    PostQuitMessage(0);
		    break;

        case WM_SIZE:
        {
            getDialFaceRect(hWnd, &lpFace);
            bRedrawMin = true;
            bRedrawHour = true;
        }
            break;

        case WM_TIMER:
        {
            RECT  rcClient;
            GetClientRect(hWnd, &rcClient);
            InvalidateRect(hWnd, &rcClient, TRUE);
        }
            break;

	    default:
		    return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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



void getDialFaceRect(HWND hWnd, pfaceT* ppface)
{
    if(NULL == *ppface)                         // we haven't instantiated this structure yet
    {
        *ppface = (pfaceT)LocalAlloc(LPTR, sizeof(faceT));
    }

    if(NULL != *ppface)                         // we have a valid structure
    {
        RECT    rcClient;
        
        GetClientRect(hWnd, &rcClient);

        int width = (rcClient.right - rcClient.left)/2;
        int heigth = (rcClient.bottom - rcClient.top)/2; 

        (*ppface)->xCenter = width;
        (*ppface)->yCenter = heigth;

        // radius is the minimum of half-heigth or half-width
        (*ppface)->radius = (width < heigth ? width : heigth);
        (*ppface)->radius = (*ppface)->radius - 30;
    }
}
