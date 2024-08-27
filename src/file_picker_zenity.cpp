#include <cstdlib>
#include <memory>
#include <iostream>
#include <cstring>

#include "file_picker.h"

int fp_get_user_path(char **path_out) {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --title=\"Select ROM\" --file-filter=\"GB File|*.gb\" --file-filter=\"ROM File|*.rom\"", "r"), pclose);
    *path_out = nullptr;
    if (!pipe) return 1;

    const size_t bufferSize = 512;
    char buffer[bufferSize];
    std::string result;

    while (fgets(buffer, bufferSize, pipe.get()) != nullptr) {
        result += buffer;
    }

    if (!result.empty() && result.back() == '\n') result.pop_back();

    *path_out = strdup(result.c_str());
    return 0;
}
