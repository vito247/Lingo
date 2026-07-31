#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <Windows.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "interpreter.h"

int command();
bool addToPath(const std::string& path);
std::string getExecutableDirectory();
std::vector<std::string> parseArguments(
    const std::string& input
);

int main() {
    while (true) {
        int result = command();

        if (result == 0) {
            return 0;
        }
    }
}

int command() {
    std::string cmd;

    std::cout << ">> ";
    std::getline(std::cin, cmd);

    std::vector<std::string> args =
        parseArguments(cmd);

    if (args.empty()) {
        std::cout << "Command or Arguments cannot be empty." << std::endl;
        return 1;
    }
    if (args[0] == "exit") {
        return 0;
    }

    if (args[0] == "help") {
        std::string msg[] = {
            "Available commands:",
            "help : Show this help message",
            "register : Register this program to PATH, only supported on Windows",
            "run <file path> : Run a script from a file",
            "exit : Exit the interpreter"
        };

        for (const auto& line : msg) {
            std::cout << line << '\n';
        }

        return 1;
    }
    if (args[0] == "register") {
        std::string executableDir = getExecutableDirectory();
        if (!addToPath(executableDir)) {
            std::cerr << "Error: Failed to register program to PATH.\n";
            return 1;
        }
		std::cout << "Successfully registered program to PATH.\n";
        return 1;
    }
    if (args[0] == "run") {
        if (args.size() < 2) {
            std::cerr
                << "Error: No file path provided.\n";

            return 1;
        }

        std::string path = args[1];
        
        if (path.substr(path.find_last_of(".") + 1) != "lingo") {
            std::cerr
                << "Error: Unsupported file extension: "
                << path
                << '\n';
            return 1;
        }

        std::ifstream file(path);

        if (!file.is_open()) {
            std::cerr
                << "Error: Could not open file: "
                << path
                << '\n';

            return 1;
        }

        std::string source(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        Lexer lexer(source);

        auto tokens =
            lexer.tokenize();

        Parser parser(tokens);

        auto ast =
            parser.parse();

        Interpreter interpreter;

        interpreter.execute(
            ast.get()
        );

        return 1;
    }

    std::cerr
        << "Unknown command: "
        << args[0]
        << '\n';

    return 1;
}
std::vector<std::string> parseArguments(
    const std::string& input
) {
    std::vector<std::string> args;
    std::string current;

    bool inQuotes = false;

    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (std::isspace(
            static_cast<unsigned char>(c)
        ) && !inQuotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }

    if (!current.empty()) {
        args.push_back(current);
    }

    return args;
}
std::string getExecutableDirectory() {
    char buffer[MAX_PATH];

    DWORD length = GetModuleFileNameA(
        nullptr,
        buffer,
        MAX_PATH
    );

    std::string path(
        buffer,
        length
    );

    size_t pos =
        path.find_last_of("\\/");

    if (pos != std::string::npos) {
        path.resize(pos);
    }

    return path;
}
bool addToPath(const std::string& path) {
    HKEY hKey;

    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Environment",
        0,
        KEY_READ | KEY_WRITE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD type = REG_EXPAND_SZ;
    DWORD size = 0;

    result = RegGetValueA(
        hKey,
        nullptr,
        "Path",
        RRF_RT_REG_EXPAND_SZ,
        nullptr,
        nullptr,
        &size
    );

    std::string currentPath;

    if (result == ERROR_SUCCESS && size > 0) {
        currentPath.resize(size);

        RegGetValueA(
            hKey,
            nullptr,
            "Path",
            RRF_RT_REG_EXPAND_SZ,
            nullptr,
            currentPath.data(),
            &size
        );

        if (!currentPath.empty() &&
            currentPath.back() == '\0') {
            currentPath.pop_back();
        }
    }

    if (currentPath.find(path) != std::string::npos) {
        RegCloseKey(hKey);
        return true;
    }

    if (!currentPath.empty()) {
        currentPath += ";";
    }

    currentPath += path;

    result = RegSetValueExA(
        hKey,
        "Path",
        0,
        type,
        reinterpret_cast<const BYTE*>(
            currentPath.c_str()
            ),
        static_cast<DWORD>(
            currentPath.size() + 1
            )
    );

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    SendMessageTimeoutA(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        reinterpret_cast<LPARAM>("Environment"),
        SMTO_ABORTIFHUNG,
        5000,
        nullptr
    );

    return true;
}