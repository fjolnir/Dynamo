#include "glutils.h"
#include <string.h>

bool dynamo_glExtSupported(const char *name)
{
    return strstr((char *)glGetString(GL_EXTENSIONS), name) != NULL;
}