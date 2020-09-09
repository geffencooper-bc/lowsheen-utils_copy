//==================================================================
// Copyright 2020 Brain Corporation. All rights reserved. Brain
// Corporation proprietary and confidential.
// ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
// WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
// Portions of this Source Code and its related modules/libraries
// may be governed by one or more third party licenses, additional
// information of which can be found at:
// https://info.braincorp.com/open-source-attributions

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//==================================================================

// this file defines options for debug printing

#ifndef DEBUG_H
#define DEBUG_H

// this prints out the file, function, and line
#define DEBUG_VERBOSE 0

// normal printf, used for error messages
#define DEBUG_DEFAULT 0

// define this to see messages sent/received over socketcan (can trace)
#define DEBUG_SOCKET_CAN 0

#if DEBUG_VERBOSE
#define DEBUG_PRINTF(fmt, ...)                                                                                \
    do                                                                                                        \
    {                                                                                                         \
        fprintf(stderr, "FILE: %s FUNCTION: %s() LINE: %d: " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__); \
    } while (0)

#elif DEBUG_DEFAULT
#define DEBUG_PRINTF(...)             \
    do                                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)

#else
#define DEBUG_PRINTF(fmt, ...) \
    do                         \
    {                          \
    } while (0)
#endif

#endif