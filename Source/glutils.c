#include "glutils.h"
#include <string.h>

static char *_Extensions = NULL;

bool dynamo_glExtSupported(const char *name)
{
    if(!_Extensions)
        _Extensions = (char *)glGetString(GL_EXTENSIONS);
    return strstr(_Extensions, name) != NULL;
}