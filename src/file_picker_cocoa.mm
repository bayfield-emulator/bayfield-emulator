#import <string.h>
#import <Cocoa/Cocoa.h>

#import "file_picker.h"

/* Reminder: ARC is not enabled so if you are changing this file,
 * anything you alloc/init/copy needs to be released explicitly */

int fp_get_user_path(char **path_out) {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.title = @"Select ROM";
    panel.canChooseFiles = YES;
    panel.canChooseDirectories = NO;
    panel.allowsMultipleSelection = NO;

    NSModalResponse response = [panel runModal];
    if (response == NSModalResponseOK) {
        char *path = strdup(panel.URL.path.UTF8String);
        *path_out = path;
        return 0;
    } else {
        *path_out = NULL;
        return 1;
    }
}
