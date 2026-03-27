#pragma once

#include "procUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>


// thread function for sending ping messages and fixfinger 
void procPingAndFixFinger(void*);