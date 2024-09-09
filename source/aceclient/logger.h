// SPDX-FileCopyrightText: Copyright (c) 2023-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once

#include <iostream>

#define ACE_CLIENT_LOG_LEVEL_ERROR 1
#define ACE_CLIENT_LOG_LEVEL_INFO 2
#define ACE_CLIENT_LOG_LEVEL_DEBUG 3

#ifndef ACE_CLIENT_LOG_LEVEL
#define ACE_CLIENT_LOG_LEVEL ACE_CLIENT_LOG_LEVEL_DEBUG
#endif

#if ACE_CLIENT_LOG_LEVEL >= ACE_CLIENT_LOG_LEVEL_ERROR
#define LOG_ERROR(x) (std::cerr << "[ACE CLIENT] [ERROR] " << x << std::endl)
#else
#define LOG_INFO(x)
#endif

#if ACE_CLIENT_LOG_LEVEL >= ACE_CLIENT_LOG_LEVEL_INFO
#define LOG_INFO(x) (std::cout << "[ACE CLIENT] [INFO] " << x << std::endl)
#else
#define LOG_INFO(x)
#endif

#if ACE_CLIENT_LOG_LEVEL >= ACE_CLIENT_LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) (std::cout << "[ACE CLIENT] [DEBUG] " << x << std::endl)
#else
#define LOG_DEBUG(x)
#endif
