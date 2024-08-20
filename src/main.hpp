// main.hpp : Include file for standard system include files,
// or project specific include files.

#pragma once

// Fixes to prevent 'std' class members from being overwritten by windows.h
#define WIN32_LEAN_AND_MEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX

// Standard Library
#include <iostream>
#include <array>
#include <vector>

// C Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

// File I/O
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>

// Containers and Data Structures
#include <vector>
#include <array>
#include <tuple>
#include <unordered_set>

// Algorithms and Utilities:
#include <algorithm>
#include <functional>
#include <regex>
#include <cmath>
#include <math.h>
#include <iomanip>
#include <limits>

// String and Character Manipulation
#include <cstring>
#include <sstream>

#include "tinyfd/tinyfiledialogs.h"
#include "io/portini.h"
#include "gui/gui.hpp"
