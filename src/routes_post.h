#ifndef ROUTES_POST_H
#define ROUTES_POST_H

#include "mongoose.h"

// Serves a single post page
void serve_post(struct mg_connection *c, struct mg_http_message *hm);

#endif // ROUTES_POST_H
