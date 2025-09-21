
#include "web.h"
#include "utils.h"
#include <emscripten.h> // Required for EM_JS macros
#include <emscripten/console.h>
#include <emscripten/em_js.h>
#include <emscripten/em_macros.h>
#include <emscripten/em_types.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
// Toggle console visibility
EM_JS(void, toggle_console, (void), {
  var output = document.getElementById("output");
  output.hidden = !output.hidden;
});

// Set canvas size to full window and return area
EM_JS(int, canvas_set_size, (void), {
  var canvas = document.getElementById("canvas");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  return window.innerWidth * window.innerHeight;
});

EM_JS(void, close_window, (void), { window.close(); });
void close_window_wrapper() { close_window(); }
// Print a float value
EM_JS(void, print_float, (float val), { Module.print(val); });

// Print a UTF-8 string
EM_JS(void, print, (const char *str), { Module.print(UTF8ToString(str)); });

void call_canvas() { canvas_set_size(); }

void print_console(const char *str) { print(str); };

em_str_callback_func onPreloadSuccess(const char *filename) {
  TRACELOG(LOG_INFO, "File loaded:", filename);
}

em_str_callback_func onPreloadError(const char *fileName) {
  TRACELOG(LOG_WARNING, "File failed to load:", fileName);
}
#include <stdlib.h> // For free()

EM_JS(char *, list_all_files, (const char *dir), {
  const currentDir = UTF8ToString(dir);
  let output = 'Files in ' + currentDir + ':\n';

  function traverse(folder) {
    const names = FS.readdir(folder);
    for (let i = 0; i < names.length; i++) {
      const name = names[i];
      if (name == '.' || name == '..')
        continue; // Fixed: removed spaces in ===

      const path = folder + '/' + name;
      try {
        const node = FS.lookupPath(path).node;
        if (FS.isFile(node.mode)) {
          output += '[FILE] ' + path + '\n';
        } else if (FS.isDir(node.mode)) {
          output += '[DIR]  ' + path + '\n';
          traverse(path); // Recursive call for subdirectories
        }
      }
      catch(e) { output += '[ERROR] ' + path + ': ' + e.message + '\n'; }
    }
  }

  try { traverse(currentDir); }
  catch(e) { output = 'Error reading directory: ' + e.message; }

  // Return the string allocated on the Emscripten heap
  return stringToNewUTF8(output);
});

void list_files() {
  char *fileList = list_all_files("/");
  if (fileList) {
    print_console(fileList);
    free(fileList);
  } else {
    print_console("Failed to list files\n");
  }
}
