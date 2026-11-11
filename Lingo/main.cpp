#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <Windows.h>
#include <filesystem>
#include <string_view>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "ast_printer.h"
#include "interpreter.h"

constexpr std::string_view VERSION = "0.3.0";
namespace fs = std::filesystem;

bool addToPath(const std::string& path);
std::string getdir();


int main(int argc, char* argv[]) {
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		args.push_back(argv[i]);
	}
	if (args.empty()) {
		std::cerr << "Error: Arguments cannot be empty.\n";
		return 1;
	}
	const std::string& cmd = args[0];

	if (cmd == "help") {
		std::string msg[] = {
			"Available commands:",
			"help : Show this help message",
			"version : Show the version of the interpreter",
			"new <directory> <filename> : Create a new lingo file in the specified directory",
			"register : Register this program to PATH, only supported on Windows",
			"run <file path> [--debug]: Run a script from a file",
		};
		for (const auto& line : msg) {
			std::cout << line << '\n';
		}
		return 0;
	}
	else if (cmd == "version") {
		std::cout
			<< "Lingo CLI Interpreter Version "
			<< VERSION << '\n'
			<< "Github: https://github.com/vito247/Lingo\n";
		return 0;
	}
	else if (cmd == "new") {
		if (args.size() < 3) {
			std::cerr
				<< "Error: Not enough arguments for 'new' command.\n"
				<< "Usage: new <directory> <filename>\n";
			return 1;
		}

		fs::path dir(args[1]);
		fs::path filename(args[2]);

		if (filename.extension() != ".lingo") {
			filename += ".lingo";
		}
		if (!fs::exists(dir)) {
			std::cerr
				<< "Error: Directory does not exist: "
				<< dir << '\n';
			return 1;
		}
		if (!fs::is_directory(dir)) {
			std::cerr
				<< "Error: Path is not a directory: "
				<< dir << '\n';

			return 1;
		}
		fs::path filePath = dir / filename;
		if (fs::exists(filePath)) {
			std::cerr
				<< "Error: File already exists: "
				<< filePath << '\n';

			return 1;
		}
		std::ofstream file(filePath);
		if (!file) {
			std::cerr
				<< "Error: Could not create lingo file: "
				<< filePath << '\n';

			return 1;
		}
		file.close();
		std::cout
			<< "Successfully created new lingo file: "
			<< fs::absolute(filePath) << '\n';
		return 0;
	}
	else if (cmd == "register") {
		std::string dir = getdir();
		if (dir.empty() || !addToPath(dir)) {
			std::cerr
				<< "Error: Failed to register program to PATH.\n";
			return 1;
		}
		std::cout
			<< "Successfully registered program to PATH.\n"
			<< "Please restart your terminal to apply the changes.\n";
		return 0;
	}

	else if (cmd == "run") {
		if (args.size() < 2) {
			std::cerr
				<< "Error: Not enough arguments for 'run' command.\n"
				<< "Usage: run <file path> [--debug]\n";
			return 1;
		}

		bool debugMode = false;

		if (args.size() >= 3) {
			if (args[2] == "--debug") {
				debugMode = true;
			}
			else {
				std::cerr
					<< "Error: Unknown option: "
					<< args[2] << '\n';
				return 1;
			}
		}

		try {
			fs::path filePath(args[1]);

			if (filePath.extension() != ".lingo") {
				std::cerr
					<< "Error: Unsupported file extension: "
					<< filePath << '\n';
				return 1;
			}

			std::ifstream file(filePath);

			if (!file.is_open()) {
				std::cerr
					<< "Error: Could not open file: "
					<< filePath << '\n';
				return 1;
			}

			std::string source(
				(std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>()
			);

			if (debugMode) {
				std::cout << "[Lingo Debugger]\n\n";
				std::cout << "[Lexer] Tokenizing...\n";
			}

			Lexer lexer(source);
			auto tokens = lexer.tokenize();

			if (debugMode) {
				std::cout << "[Lexer] Completed.\n";
				std::cout << "[Parser] Parsing...\n";
			}

			Parser parser(tokens);
			auto ast = parser.parse();

			if (debugMode) {
				std::cout << "[Parser] Completed.\n";
				std::cout << "[AST]\n";

				ASTPrinter printer;
				printer.print(ast.get());

				std::cout << "\n[Interpreter] Executing...\n";
			}

			Interpreter interpreter;
			interpreter.execute(ast.get());

			if (debugMode) {
				std::cout << "\n[Debug] Execution completed.\n";
			}

			return 0;
		}
		catch (const std::exception& e) {
			std::cerr
				<< "Error: "
				<< e.what() << '\n';
			return 1;
		}
		catch (...) {
			std::cerr
				<< "Error: An unknown error occurred while running the lingo file.\n";
			return 1;
		}
	}

	std::cerr << "Unknown command: " << cmd << '\n';
	return 1;
}

std::string getdir() {
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