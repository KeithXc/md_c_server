#ifndef ROUTES_INDEX_H
#define ROUTES_INDEX_H

#include "mongoose.h"

// Serves the index page with a list of markdown files
void serve_index(struct mg_connection *c);

#endif // ROUTES_INDEX_H
