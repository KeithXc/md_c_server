#ifndef HTTP_HELPERS_H
#define HTTP_HELPERS_H

#include "mongoose.h"
#include <time.h>
#include <stdbool.h>

/**
 * @brief Handles conditional HTTP requests (ETag and If-Modified-Since).
 *
 * This function checks the request headers for "If-None-Match" and "If-Modified-Since".
 * If a match is found with the provided etag or last_modified time, it sends a
 * "304 Not Modified" response and returns true.
 *
 * @param c The mongoose connection.
 * @param hm The HTTP request message.
 * @param etag The ETag of the resource being requested.
 * @param last_modified The last modification time of the resource.
 * @return true if a "304 Not Modified" response was sent, false otherwise.
 */
bool handle_conditional_request(
    struct mg_connection *c,
    struct mg_http_message *hm,
    const char *etag,
    time_t last_modified
);

#endif // HTTP_HELPERS_H
