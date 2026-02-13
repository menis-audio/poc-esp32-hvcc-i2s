#pragma once

// Set the hvcc patch name used with -n <name> when generating.
// Provide both identifier and header path for inclusion.
// Example: hvcc ... -n mySynth
#define HV_PATCH_NAME mySynth
// Generated header file under main/hvcc/c will be named Heavy_<name>.h
#define HV_HEADER_PATH "Heavy_mySynth.h"
