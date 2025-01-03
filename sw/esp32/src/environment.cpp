
#include "environment.h"

// placing this destructor in the .cpp file is needed to prevent the
// linking error 'undefined reference to vtable for IEnvironment'.
// see https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable
IEnvironment::~IEnvironment() {}
