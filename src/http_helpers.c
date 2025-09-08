#include "http_helpers.h"
#include <string.h>

bool handle_conditional_request(
    struct mg_connection *c,
    struct mg_http_message *hm,
    const char *etag,
    time_t last_modified // Parameter kept for future use, but currently unused.
) {
    // Check ETag / If-None-Match
    const struct mg_str *etag_hdr = mg_http_get_header(hm, "If-None-Match");
    if (etag_hdr != NULL && etag != NULL) {
        // A simple string comparison is sufficient for our ETag format.
        // A more robust implementation would parse comma-separated values.
        if (mg_strcmp(*etag_hdr, mg_str(etag)) == 0) {
            mg_http_reply(c, 304, "", "");
            return true; // Request handled
        }
    }

    // If-Modified-Since logic is omitted due to lack of a standard date parsing function.
    (void)last_modified; // Suppress unused parameter warning

    return false; // Request not handled, caller should send full response
}
