#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <Shlwapi.h>
#include <iterator>
#include <cwchar>
#include <streambuf>
#pragma comment(lib, "Shlwapi.lib")

HWND hMainWindow;
HWND hTextEditButton;
HWND hFontEditButton;
HWND hDeveloperInfo;
HWND hWebsiteLink;

const wchar_t MAIN_CLASS_NAME[] = L"SkyTextToolWindowClass";
const wchar_t TEXT_EDITOR_CLASS_NAME[] = L"TextEditorWindowClass";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TextEditorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateMainWindowControls(HINSTANCE hInstance);
void HandleTextEdit();
void HandleFontEdit();
std::wstring GetGameBaseDirectory();
void ShowMessage(const std::wstring& title, const std::wstring& message, UINT type);

std::wstring UTF8ToWide(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return L"";
    }
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WideToUTF8(const std::wstring& wide_str) {
    if (wide_str.empty()) {
        return "";
    }
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), (int)wide_str.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), (int)wide_str.size(), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

struct TextEditorData {
    std::vector<std::string> originalRawLines;
    std::vector<std::wstring> extractedValues;
    std::vector<int> valueLineIndices;
    std::vector<std::pair<size_t, size_t>> valueQuotePositions;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClass(&wc)) {
        ShowMessage(L"Error", L"Failed to register main window class! Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
        return 0;
    }

    WNDCLASS textEditorWc = {};
    textEditorWc.lpfnWndProc = TextEditorWindowProc;
    textEditorWc.hInstance = hInstance;
    textEditorWc.lpszClassName = TEXT_EDITOR_CLASS_NAME;
    textEditorWc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    textEditorWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClass(&textEditorWc)) {
        ShowMessage(L"Error", L"Failed to register Text Editor window class! Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
        return 0;
    }

    hMainWindow = CreateWindowEx(
        0,
        MAIN_CLASS_NAME,
        L"Sky Children of the Light Text Tool",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hMainWindow == nullptr) {
        ShowMessage(L"Error", L"Failed to create main window! Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
        return 0;
    }

    CreateMainWindowControls(hInstance);

    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClass(MAIN_CLASS_NAME, hInstance);
    UnregisterClass(TEXT_EDITOR_CLASS_NAME, hInstance);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case 1001:
            HandleTextEdit();
            break;
        case 1002:
            HandleFontEdit();
            break;
        case 1003:
            ShellExecute(nullptr, L"open", L"https://gamesinkurdish.com", nullptr, nullptr, SW_SHOWNORMAL);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
                   break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CreateMainWindowControls(HINSTANCE hInstance) {
    hTextEditButton = CreateWindow(
        L"BUTTON",
        L"Text Edit",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        50, 50, 120, 40,
        hMainWindow,
        (HMENU)1001,
        hInstance,
        nullptr
    );

    hFontEditButton = CreateWindow(
        L"BUTTON",
        L"Font Edit",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        220, 50, 120, 40,
        hMainWindow,
        (HMENU)1002,
        hInstance,
        nullptr
    );

    hDeveloperInfo = CreateWindow(
        L"STATIC",
        L"Developer: Ameer Xoshnaw",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        10, 150, 360, 20,
        hMainWindow,
        nullptr,
        hInstance,
        nullptr
    );

    hWebsiteLink = CreateWindow(
        L"STATIC",
        L"Website: gamesinkurdish.com",
        WS_VISIBLE | WS_CHILD | SS_CENTER | SS_NOTIFY,
        10, 170, 360, 20,
        hMainWindow,
        (HMENU)1003,
        hInstance,
        nullptr
    );

    HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Segoe UI");
    SendMessage(hTextEditButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hFontEditButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hDeveloperInfo, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hWebsiteLink, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void HandleTextEdit() {
    HWND hTextEditorWindow = CreateWindowEx(
        0, TEXT_EDITOR_CLASS_NAME, L"Text Editor",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        hMainWindow, nullptr, GetModuleHandle(nullptr), nullptr
    );

    if (hTextEditorWindow == nullptr) {
        ShowMessage(L"Error", L"Failed to create text editor window! Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
    }
}

LRESULT CALLBACK TextEditorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hEditorFont = nullptr;
    static HWND hEditControl = nullptr;

    switch (uMsg) {
    case WM_CREATE: {
        TextEditorData* pData = new TextEditorData();
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pData);

        CreateWindow(L"BUTTON", L"Export", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hwnd, (HMENU)2001, GetModuleHandle(nullptr), nullptr);
        CreateWindow(L"BUTTON", L"Import", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            100, 10, 80, 30, hwnd, (HMENU)2002, GetModuleHandle(nullptr), nullptr);
        CreateWindow(L"BUTTON", L"Paste Edited Text", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            190, 10, 140, 30, hwnd, (HMENU)2004, GetModuleHandle(nullptr), nullptr);
        CreateWindow(L"BUTTON", L"Import From File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            340, 10, 140, 30, hwnd, (HMENU)2005, GetModuleHandle(nullptr), nullptr);

        hEditControl = CreateWindow(
            L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
            10, 50, 760, 500,
            hwnd, (HMENU)2003, GetModuleHandle(nullptr), nullptr
        );

        hEditorFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, L"Consolas");
        SendMessage(hEditControl, WM_SETFONT, (WPARAM)hEditorFont, TRUE);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        TextEditorData* pData = (TextEditorData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (wmId) {
        case 2001:
        {
            if (!pData) break;

            std::wstring gamePath = GetGameBaseDirectory();
            std::wstring localizableStringsPathW = gamePath + L"\\data\\Strings\\Base.lproj\\Localizable.strings";

            std::ifstream inputFile(localizableStringsPathW);
            if (!inputFile.is_open()) {
                ShowMessage(L"Error", L"Could not open Localizable.strings for export. Check game path and permissions.", MB_ICONERROR);
                break;
            }

            pData->originalRawLines.clear();
            pData->extractedValues.clear();
            pData->valueLineIndices.clear();
            pData->valueQuotePositions.clear();

            std::string line;
            int currentLineIndex = 0;
            while (std::getline(inputFile, line)) {
                pData->originalRawLines.push_back(line);

                size_t eqPos = line.find("=");
                if (eqPos != std::string::npos) {
                    size_t firstQuote = line.find("\"", eqPos);
                    if (firstQuote != std::string::npos) {
                        size_t secondQuote = line.find("\"", firstQuote + 1);
                        if (secondQuote != std::string::npos) {
                            std::string extracted_utf8 = line.substr(firstQuote + 1, secondQuote - (firstQuote + 1));
                            pData->extractedValues.push_back(UTF8ToWide(extracted_utf8));
                            pData->valueLineIndices.push_back(currentLineIndex);
                            pData->valueQuotePositions.push_back({ firstQuote + 1, secondQuote });
                        }
                    }
                }
                currentLineIndex++;
            }
            inputFile.close();

            std::wstring combinedTextForEditor;
            for (const auto& val : pData->extractedValues) {
                combinedTextForEditor += val + L"\r\n";
            }

            SetWindowText(hEditControl, combinedTextForEditor.c_str());

            ShowMessage(L"Export", L"Strings extracted and displayed. You can now edit the text.", MB_OK | MB_ICONINFORMATION);
        }
        break;
        case 2002:
        {
            if (!pData || pData->extractedValues.empty()) {
                ShowMessage(L"Error", L"No data to import. Please export first.", MB_ICONERROR);
                break;
            }

            int textLen = GetWindowTextLength(hEditControl);
            std::vector<wchar_t> buffer(textLen + 1);
            GetWindowText(hEditControl, buffer.data(), textLen + 1);
            std::wstring editedDisplayContent(buffer.data());

            std::wistringstream ss(editedDisplayContent);
            std::wstring editedLineW;
            std::vector<std::wstring> newValues;
            while (std::getline(ss, editedLineW)) {
                if (!editedLineW.empty() && editedLineW.back() == L'\r') {
                    editedLineW.pop_back();
                }
                newValues.push_back(editedLineW);
            }

            if (newValues.size() != pData->extractedValues.size()) {
                ShowMessage(L"Error", L"The number of editable lines in the text editor does not match the original extracted values. Import aborted to prevent file corruption.", MB_ICONERROR);
                break;
            }

            std::wstring gamePath = GetGameBaseDirectory();
            std::wstring localizableStringsPathW = gamePath + L"\\data\\Strings\\Base.lproj\\Localizable.strings";

            std::ofstream outputFile(localizableStringsPathW);
            if (!outputFile.is_open()) {
                ShowMessage(L"Error", L"Could not open Localizable.strings for import (write access). Check game path and permissions.", MB_ICONERROR);
                break;
            }

            std::vector<std::string> linesToWrite = pData->originalRawLines;

            for (size_t i = 0; i < pData->valueLineIndices.size(); ++i) {
                int originalIdx = pData->valueLineIndices[i];
                if (originalIdx < linesToWrite.size() && i < newValues.size()) {
                    std::string newValueUTF8 = WideToUTF8(newValues[i]);

                    size_t value_start_pos = pData->valueQuotePositions[i].first;
                    size_t value_end_pos = pData->valueQuotePositions[i].second;

                    linesToWrite[originalIdx].replace(value_start_pos, value_end_pos - value_start_pos, newValueUTF8);
                }
            }

            for (const auto& line : linesToWrite) {
                outputFile << line << std::endl;
            }
            outputFile.close();

            ShowMessage(L"Import", L"Strings imported successfully!", MB_OK | MB_ICONINFORMATION);
        }
        break;
        case 2004:
        {
            if (!OpenClipboard(hwnd)) {
                ShowMessage(L"Error", L"Could not open clipboard.", MB_ICONERROR);
                break;
            }

            HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
            if (hClipboardData == nullptr) {
                CloseClipboard();
                ShowMessage(L"Error", L"No text found in clipboard.", MB_ICONINFORMATION);
                break;
            }

            wchar_t* clipboardText = static_cast<wchar_t*>(GlobalLock(hClipboardData));
            if (clipboardText == nullptr) {
                CloseClipboard();
                ShowMessage(L"Error", L"Could not lock clipboard data.", MB_ICONERROR);
                break;
            }

            std::wstring pastedContent(clipboardText);
            GlobalUnlock(hClipboardData);
            CloseClipboard();


            SetWindowText(hEditControl, pastedContent.c_str());
            ShowMessage(L"Paste", L"Text pasted from clipboard to editor.", MB_OK | MB_ICONINFORMATION);
        }
        break;
        case 2005:
        {
            OPENFILENAME ofn;
            wchar_t szFile[MAX_PATH] = L"";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileName(&ofn) == TRUE) {
                std::ifstream inFile(ofn.lpstrFile);
                if (!inFile.is_open()) {
                    ShowMessage(L"Error", L"Could not open selected file for reading.", MB_ICONERROR);
                    break;
                }

                std::string fileContentUTF8;
                std::string fileLineUTF8;
                while (std::getline(inFile, fileLineUTF8)) {
                    fileContentUTF8 += fileLineUTF8 + "\r\n";
                }
                if (!fileContentUTF8.empty() && fileContentUTF8.back() == '\n') {
                    fileContentUTF8.pop_back();
                    if (!fileContentUTF8.empty() && fileContentUTF8.back() == '\r') {
                        fileContentUTF8.pop_back();
                    }
                }
                inFile.close();

                std::wstring fileContentW = UTF8ToWide(fileContentUTF8);

                SetWindowText(hEditControl, fileContentW.c_str());
                ShowMessage(L"Import From File", L"Content loaded into editor from:\n" + std::wstring(ofn.lpstrFile), MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
        }
    }
                   break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkColor(hdc, RGB(50, 50, 50));
        return (LRESULT)CreateSolidBrush(RGB(50, 50, 50));
    }
    case WM_DESTROY: {
        TextEditorData* pData = (TextEditorData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (pData) {
            delete pData;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        }
        if (hEditorFont) {
            DeleteObject(hEditorFont);
            hEditorFont = nullptr;
        }
        DestroyWindow(hwnd);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void HandleFontEdit() {
    OPENFILENAME ofn;
    wchar_t szFile[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"TrueType Fonts (*.ttf)\0*.ttf\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::wstring selectedFontPath = ofn.lpstrFile;
        ShowMessage(L"Font Selected", L"Selected font: " + selectedFontPath, MB_OK | MB_ICONINFORMATION);

        std::wstring gameBaseDir = GetGameBaseDirectory();
        if (gameBaseDir.empty()) {
            ShowMessage(L"Error", L"Could not determine game base directory.", MB_ICONERROR);
            return;
        }

        std::wstring converterExePath = gameBaseDir + L"\\ameer_ttf_to_slug.exe";
        std::wstring tempSlugPath = gameBaseDir + L"\\aaa.slug";
        std::wstring fontBinDir = gameBaseDir + L"\\data\\assets\\initial\\Data\\Fonts\\Bin\\";

        if (!PathFileExists(converterExePath.c_str())) {
            ShowMessage(L"Error", L"Font converter (ameer_ttf_to_slug.exe) not found at: " + converterExePath, MB_ICONERROR);
            return;
        }

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        std::wstring cmdLine = L"\"" + converterExePath + L"\" \"" + selectedFontPath + L"\" -o \"" + tempSlugPath + L"\"";
        std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
        cmdLineBuffer.push_back(L'\0');

        if (!CreateProcess(
            nullptr,
            cmdLineBuffer.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            gameBaseDir.c_str(),
            &si,
            &pi
        )) {
            ShowMessage(L"Error", L"Failed to run font converter via CreateProcess! Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
            return;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (!PathFileExists(tempSlugPath.c_str())) {
            ShowMessage(L"Error", L"Font conversion failed. Output file 'aaa.slug' not found. Check converter output or path.", MB_ICONERROR);
            return;
        }

        std::vector<std::wstring> targetSlugs = {
            L"Arabic-Bold.slug", L"Arabic.slug", L"CJK-Ext-A.slug",
            L"CJK-Ext-B-I.slug", L"CJK-Remaining.slug", L"CJK-Uni-1.slug",
            L"CJK-Uni-2.slug", L"CJK-Uni-3.slug", L"CJK-Uni-4.slug",
            L"Japanese-Common.slug", L"Latin-Bold.slug", L"Latin.slug",
            L"Special.slug", L"Thai-Bold.slug", L"Thai.slug"
        };

        bool allReplaced = true;
        for (const auto& slugFile : targetSlugs) {
            std::wstring destinationPath = fontBinDir + slugFile;

            if (PathFileExists(destinationPath.c_str())) {
                if (_wremove(destinationPath.c_str()) != 0) {
                    ShowMessage(L"Error", L"Failed to remove existing file: " + destinationPath + L". Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
                    allReplaced = false;
                    break;
                }
            }

            if (!CopyFile(tempSlugPath.c_str(), destinationPath.c_str(), FALSE)) {
                ShowMessage(L"Error", L"Failed to copy font to: " + destinationPath + L". Error: " + std::to_wstring(GetLastError()), MB_ICONERROR);
                allReplaced = false;
                break;
            }
        }

        _wremove(tempSlugPath.c_str());

        if (allReplaced) {
            ShowMessage(L"Font Edit", L"Fonts replaced successfully!", MB_OK | MB_ICONINFORMATION);
        }
        else {
            ShowMessage(L"Font Edit", L"Font replacement completed with errors. Check previous messages.", MB_ICONWARNING);
        }
    }
}

std::wstring GetGameBaseDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    std::wstring exePath(buffer);
    size_t lastSlash = exePath.rfind(L"\\");
    if (lastSlash != std::wstring::npos) {
        return exePath.substr(0, lastSlash);
    }
    return L"";
}

void ShowMessage(const std::wstring& title, const std::wstring& message, UINT type) {
    MessageBox(hMainWindow, message.c_str(), title.c_str(), type);
}