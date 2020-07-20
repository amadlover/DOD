#include <Windows.h>
#include <Windowsx.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"
#include "game.h"

LRESULT CALLBACK WindowProc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
        case WM_COMMAND:
            break;

        case WM_QUIT:
            break;

        case WM_DESTROY:
            PostQuitMessage (0);
            break;

        case WM_CLOSE:
            PostQuitMessage (0);
            break;

        case WM_KEYDOWN:
            break;

        case WM_LBUTTONDOWN:
            game_process_left_mouse_click (GET_X_LPARAM (l_param), GET_Y_LPARAM (l_param));
            break;

        case WM_RBUTTONDOWN:
            game_process_right_mouse_click (GET_X_LPARAM (l_param), GET_Y_LPARAM (l_param));
            break;

        case WM_MOUSEMOVE:
            break;

        default:
            break;
    }

    return DefWindowProc (h_wnd, msg, w_param, l_param);
}


int WINAPI wWinMain (_In_ HINSTANCE h_instance, _In_opt_ HINSTANCE previous_instance, _In_ PWSTR cmd_line, _In_ int cmd_show)
{
    AllocConsole ();
    freopen ("CONOUT$", "w", stdout);
    printf ("%s %s %d Hello Console\n", __FILE__, __FUNCTION__, __LINE__);

    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = h_instance;
    wc.lpszClassName = L"DOD";
    wc.hCursor = LoadCursor (h_instance, IDC_ARROW);

    if (!RegisterClass (&wc))
    {
        return EXIT_FAILURE;
    }

    HWND h_wnd = CreateWindow (L"DOD", 
                               L"DOD",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               640,
                               480, 
                               NULL,
                               NULL,
                               h_instance,
                               NULL);

    if (!h_wnd)
    {
        return EXIT_FAILURE;
    }

    ShowWindow (h_wnd, cmd_show);
    UpdateWindow (h_wnd);

    AGE_RESULT result = game_init (h_instance, h_wnd);
    if (result != AGE_SUCCESS)
    {
        log_error (result);
        goto exit;
        return EXIT_FAILURE;
    }

    MSG msg;
    ZeroMemory (&msg, sizeof (msg));

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
        else
        {
            game_update ();
        }
    }

 exit:
    game_exit ();

    DestroyWindow (h_wnd);

    _getch ();
    FreeConsole ();

    return EXIT_SUCCESS;
}
