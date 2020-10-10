#ifndef FILE_PICKER_H
#define FILE_PICKER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ask for a file.
 * If a file was chosen, returns 0 and the path in path_out, otherwise 1 and sets path_out to NULL.
 * path_out will be a malloced pointer that must be freed by the caller.
 */
int fp_get_user_path(char **path_out);

#ifdef __cplusplus
}
#endif

#endif
