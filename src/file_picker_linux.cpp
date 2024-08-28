#include <cstdlib>
#include <memory>
#include <iostream>
#include <cstring>

#include "file_picker.h"

int fp_get_user_path(char **path_out) {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(nullptr, pclose);
    *path_out = nullptr;

    const size_t bufferSize = 512;
    char buffer[bufferSize];
    std::string result;

    // check for zenity
    if (system("zenity --version > /dev/null 2>&1") == 0) {
        pipe.reset(popen("zenity --file-selection --title=\"Select ROM\" --file-filter=\"GB File|*.gb\" --file-filter=\"ROM File|*.rom\"", "r"));
    }
    // if zenity is not available, check for kdialog
    else if (system("kdialog --version > /dev/null 2>&1") == 0) {
        pipe.reset(popen("kdialog --getopenfilename . --title \"Select ROM\" \"Gameboy ROM files (*.gb *.rom)\"", "r"));
    }
	// neither zenity nor kdialog are available, oh well, we tried
    else {
        return 1;
    }

    if (!pipe) return 1;

    while (fgets(buffer, bufferSize, pipe.get()) != nullptr) {
        result += buffer;
    }

    if (!result.empty() && result.back() == '\n') result.pop_back();

    *path_out = strdup(result.c_str());
    return 0;
}
