#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <vector>

#include "lua.h"
#include "lualib.h"
#include "Luau/Compiler.h"

#pragma comment(lib, "Comctl32.lib")

static HWND g_editor = nullptr;
static HWND g_output = nullptr;
static HWND g_status = nullptr;
static HWND g_window = nullptr;
static std::wstring g_console;

static std::string WideToUtf8(const std::wstring& input)
{
    if (input.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, input.data(), (int)input.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, input.data(), (int)input.size(), result.data(), size, nullptr, nullptr);
    return result;
}

static std::wstring Utf8ToWide(const std::string& input)
{
    if (input.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, input.data(), (int)input.size(), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, input.data(), (int)input.size(), result.data(), size);
    return result;
}

static void AppendOutput(const std::wstring& text)
{
    g_console += text;
    g_console += L"\r\n";
    if (g_output)
        SetWindowTextW(g_output, g_console.c_str());
}

static int LuauPrint(lua_State* L)
{
    int count = lua_gettop(L);
    std::ostringstream line;
    for (int i = 1; i <= count; ++i)
    {
        if (i > 1) line << "\t";
        const char* value = lua_tostring(L, i);
        line << (value ? value : "<value>");
    }
    AppendOutput(Utf8ToWide(line.str()));
    return 0;
}

static void SetStatus(const wchar_t* value)
{
    if (g_status) SetWindowTextW(g_status, value);
}

static void RunScript()
{
    int length = GetWindowTextLengthW(g_editor);
    std::wstring wideSource(length, L'\0');
    if (length > 0) GetWindowTextW(g_editor, wideSource.data(), length + 1);

    std::string source = WideToUtf8(wideSource);
    AppendOutput(L"> Running script...");
    SetStatus(L"Running");

    lua_State* L = luaL_newstate();
    if (!L)
    {
        AppendOutput(L"ERROR: Could not create Luau state.");
        SetStatus(L"Error");
        return;
    }

    luaL_openlibs(L);
    lua_pushcfunction(L, LuauPrint);
    lua_setglobal(L, "print");

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr, &bytecodeSize);
    if (!bytecode)
    {
        AppendOutput(L"ERROR: Luau compiler returned no bytecode.");
        lua_close(L);
        SetStatus(L"Compile error");
        return;
    }

    int loadResult = luau_load(L, "LuauDesk", bytecode, bytecodeSize, 0);
    free(bytecode);

    if (loadResult != 0)
    {
        const char* message = lua_tostring(L, -1);
        AppendOutput(L"LOAD ERROR: " + Utf8ToWide(message ? message : "Unknown error"));
        lua_close(L);
        SetStatus(L"Load error");
        return;
    }

    int callResult = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (callResult != 0)
    {
        const char* message = lua_tostring(L, -1);
        AppendOutput(L"RUNTIME ERROR: " + Utf8ToWide(message ? message : "Unknown error"));
        lua_close(L);
        SetStatus(L"Runtime error");
        return;
    }

    AppendOutput(L"Finished successfully.");
    lua_close(L);
    SetStatus(L"Ready");
}

static void Layout(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    const int margin = 14;
    const int toolbar = 42;
    const int outputHeight = height / 3;

    MoveWindow(g_editor, margin, toolbar, width - margin * 2, height - toolbar - outputHeight - 48, TRUE);
    MoveWindow(g_output, margin, height - outputHeight - 22, width - margin * 2, outputHeight - 34, TRUE);
    MoveWindow(g_status, margin, height - 22, width - margin * 2, 18, TRUE);

    HWND execute = GetDlgItem(hwnd, 1001);
    HWND clear = GetDlgItem(hwnd, 1002);
    MoveWindow(execute, margin, 8, 112, 28, TRUE);
    MoveWindow(clear, margin + 122, 8, 90, 28, TRUE);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        g_window = hwnd;
        HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        CreateWindowW(L"BUTTON", L"Execute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)1001, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Clear Output", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 0, 0, hwnd, (HMENU)1002, nullptr, nullptr);

        g_editor = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
            L"-- LuauDesk: standalone Luau playground\r\nprint(\"Hello from Luau!\")\r\n\r\nlocal total = 20 + 22\r\nprint(\"Answer:\", total)",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)1003, nullptr, nullptr);

        g_output = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Console output will appear here.",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)1004, nullptr, nullptr);

        g_status = CreateWindowW(L"STATIC", L"Ready",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 0, 0, hwnd, (HMENU)1005, nullptr, nullptr);

        SendMessageW(g_editor, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageW(g_output, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageW(g_status, WM_SETFONT, (WPARAM)font, TRUE);
        Layout(hwnd);
        return 0;
    }
    case WM_SIZE:
        Layout(hwnd);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1001) RunScript();
        if (LOWORD(wParam) == 1002)
        {
            g_console.clear();
            SetWindowTextW(g_output, L"");
            SetStatus(L"Console cleared");
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    INITCOMMONCONTROLSEX controls{ sizeof(controls), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&controls);

    const wchar_t CLASS_NAME[] = L"LuauDeskWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassW(&wc)) return 1;

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"LuauDesk — Standalone Luau Playground",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 980, 720,
        nullptr, nullptr, instance, nullptr);

    if (!hwnd) return 1;
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
