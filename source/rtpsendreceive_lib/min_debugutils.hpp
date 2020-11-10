#pragma once

#ifdef DEBUG
#define DEBUG_LOG(STRING) cout << #STRING << std::endl;
else
#define DEBUG_LOG(STRING)
#endif