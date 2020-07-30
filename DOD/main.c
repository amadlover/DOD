#include <Windows.h>
#include <Windowsx.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include <SDL.h>
//#include <SDL_thread.h>

#include "error.h"
#include "log.h"
#include "game.h"

#define ID_GAME_TICK 1237

//SDL_Thread* game_input_thread = NULL;
//SDL_Thread* game_update_thread = NULL;
//SDL_Thread* game_submit_present_thread = NULL;

LRESULT CALLBACK WindowProc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    AGE_RESULT age_result = AGE_SUCCESS;

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

        case WM_TIMER:
            printf ("WM_TIMER\n");
            age_result = game_update ();
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
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
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

    AGE_RESULT age_result = game_init (h_instance, h_wnd);
    if (age_result != AGE_SUCCESS)
    {
        log_error (age_result);
        goto exit;
    }

    SetTimer (h_wnd, ID_GAME_TICK, 8, (TIMERPROC)game_update);

    MSG msg;
    ZeroMemory (&msg, sizeof (msg));

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_TIMER) 
            {
                msg.hwnd = h_wnd;
            }
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
        else
        {
            age_result = game_submit_present ();
            if (age_result != AGE_SUCCESS)
            {
                log_error (age_result);
                goto exit;
            }
        }
    }

 exit:
    game_exit ();

    DestroyWindow (h_wnd);

    _getch ();
    FreeConsole ();

 //   SDL_WaitThread (game_update_thread, &age_result);
 //   SDL_WaitThread (game_submit_present_thread, &age_result);
    
    return EXIT_SUCCESS;
}