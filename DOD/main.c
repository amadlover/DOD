#include <Windows.h>
#include <Windowsx.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"
#include "game.h"

#define ID_GAME_TICK 1237

unsigned long long last_tick_count = 0;

LRESULT CALLBACK WindowProc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    AGE_RESULT age_result = AGE_SUCCESS;

    unsigned long long current_tick_count = 0;

    switch (msg)
    {
    case WM_QUIT:
        PostQuitMessage (0);
        break;

    case WM_DESTROY:
        PostQuitMessage (0);
        break;

    case WM_CLOSE:
        PostQuitMessage (0);
        break;

    case WM_KEYDOWN:
        age_result = game_process_key_down (w_param);
        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }
        break;

    case WM_KEYUP:
        age_result = game_process_key_up (w_param);
        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }
        break;

    case WM_TIMER:
        current_tick_count = GetTickCount ();

        age_result = game_update ((size_t)(current_tick_count - last_tick_count));
        last_tick_count = current_tick_count;

        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }

        break;

    case WM_LBUTTONDOWN:
        age_result = game_process_left_mouse_click (GET_X_LPARAM (l_param), GET_Y_LPARAM (l_param));
        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }
        break;

    case WM_RBUTTONDOWN:
        age_result = game_process_right_mouse_click (GET_X_LPARAM (l_param), GET_Y_LPARAM (l_param));
        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }
        break;

    case WM_MOUSEMOVE:
        age_result = game_process_mouse_move (GET_X_LPARAM (l_param), GET_Y_LPARAM (l_param));
        if (age_result != AGE_SUCCESS)
        {
            log_error (age_result);
            PostQuitMessage (age_result);
        }
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
    printf ("Hello Console\n");

    HANDLE con_hnd = GetStdHandle (STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX font = { sizeof (font) };
    GetCurrentConsoleFontEx (con_hnd, FALSE, &font);
    font.dwFontSize.X = 0;
    font.dwFontSize.Y = 14;
    SetCurrentConsoleFontEx (con_hnd, FALSE, &font);

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

    HWND h_wnd = CreateWindow (
        L"DOD", 
        L"DOD",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480, 
        NULL,
        NULL,
        h_instance,
        NULL
    );

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
    }

    last_tick_count = GetTickCount64 ();

    SetTimer (h_wnd, ID_GAME_TICK, 15, NULL);

    MSG msg;
    ZeroMemory (&msg, sizeof (msg));

    while (
        msg.message != WM_QUIT && 
        msg.message != WM_CLOSE && 
        msg.message != WM_DESTROY
    )
    {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
        else
        {
            result = game_submit_present ();
            if (result != AGE_SUCCESS)
            {
                log_error (result);
                goto exit;
            }
        }
    }

    KillTimer (h_wnd, ID_GAME_TICK);

 exit:
    game_shutdown ();

    DestroyWindow (h_wnd);

    _getch ();
    FreeConsole ();

    return EXIT_SUCCESS;
}
