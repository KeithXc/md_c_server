#include "mongoose.h"
#include "routes.h" // Include our routes header
#include "utils.h"  // Include our new utils header
#include <stdio.h>
#include <string.h> // Required for strncmp
#include <unistd.h> // For readlink

// The main event handler function
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    
    // Construct the full path for the static directory
    char static_dir_path[PATH_MAX];
    snprintf(static_dir_path, sizeof(static_dir_path), "%s/static", g_project_root);
    struct mg_http_serve_opts opts = {.root_dir = static_dir_path};

    // Route the request based on the URL
    if (mg_strcmp(hm->uri, mg_str("/")) == 0) {
      serve_index(c); // Handle the index page
    } else if (strncmp(hm->uri.buf, "/post/", 6) == 0) {
      serve_post(c, hm); // Handle post pages
    } else if (strncmp(hm->uri.buf, "/static/", 8) == 0) {
      mg_http_serve_dir(c, hm, &opts); // Serve files from the 'static' directory
    } else {
      mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
    }
  }
}

int main(void) {
  // Get the project root directory when the server starts
  get_project_root(g_project_root, sizeof(g_project_root));
  
  char exe_path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len != -1) {
      exe_path[len] = '\0';
      printf("Executable path: %s\n", exe_path);
  }
  printf("Project root: %s\n", g_project_root);

  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  printf("Starting server on http://localhost:8000\n");
  mg_http_listen(&mgr, "http://localhost:8000", fn, NULL);
  for (;;) mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);
  return 0;
}
