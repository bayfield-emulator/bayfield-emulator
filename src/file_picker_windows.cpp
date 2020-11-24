#include <stdlib.h>
#include <windows.h>
#include <commdlg.h>

#include "file_picker.h"

/*
    This uses the old, XP-style GetOpenFileName picker. 
    In a perfect world, it would be migrated to Common Item Dialog type.
*/
int fp_get_user_path(char **path_out) {
    OPENFILENAMEW params = {0};
    wchar_t *wide_filename = new wchar_t[MAX_PATH];
    wide_filename[0] = '\0';

    params.lStructSize = sizeof(params);
    params.lpstrFile = wide_filename;
    params.nMaxFile = MAX_PATH;
    params.lpstrFilter = L"Gameboy ROM Files\0*.GB;*.ROM\0All Files\0*.*\0\0";
    params.lpstrTitle = L"Select ROM";
    params.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    BOOL ok = GetOpenFileNameW(&params);

    if (!ok) {
        *path_out = NULL;
        delete[] wide_filename;
        return 1;
    }

    char *rets = (char *) calloc(MAX_PATH, 1);
    //                                   The last char will be nonzero if we ran out of space.
    if (wcstombs(rets, params.lpstrFile, MAX_PATH) == (size_t)-1 || rets[MAX_PATH - 1] != 0) {
        *path_out = NULL;
        delete[] wide_filename;
        free(rets);
        return 1;
    }

    *path_out = rets;
    delete[] wide_filename;
    return 0;
}
