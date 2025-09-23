// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "timeslider.h"
#include "nodes/a2f_player.h"
#include <maya/MGlobal.h>
#include <maya/MDGMessage.h>
#include <assert.h>

namespace TimeSliderProgress {

const int kSliderHeight = 2;

static std::vector<CustomDrawID> g_DrawContexts;
static CustomDrawID g_LastUpdatedContext = kInvalidContext;

#if ENABLE_TIME_SLIDER == 1

CustomDrawID registerCustomDraw() {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return kInvalidContext;
    }

    auto& mgr = MTimeSliderCustomDrawManager::instance();
    auto ctx = mgr.registerCustomDrawOutside(MTimeSliderCustomDrawManager::MDrawLocation::kBelow, 
        "A2FAnimationPlayerBackgroundEvaluationProgress", 
        "Audio2Face Animation Player Background Evaluation Progress", 
        0, 
        kSliderHeight);
    g_DrawContexts.push_back(ctx);
    return ctx;
}

void deregisterCustomDraw(CustomDrawID ctx) {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return;
    }
    if (kInvalidContext == ctx) {
        return;
    }
    auto& mgr = MTimeSliderCustomDrawManager::instance();
    g_DrawContexts.erase(std::remove(g_DrawContexts.begin(), g_DrawContexts.end(), ctx), g_DrawContexts.end());
    mgr.deregisterCustomDraw(ctx);

    // check last updated context
    if (ctx == g_LastUpdatedContext) {
        if (0 < g_DrawContexts.size()) {
            g_LastUpdatedContext = g_DrawContexts.back();
        } else {
            g_LastUpdatedContext = kInvalidContext;
        }
    }

    show(g_LastUpdatedContext);
}

void clean() {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return;
    }

    auto& mgr = MTimeSliderCustomDrawManager::instance();
    for (auto ctx: g_DrawContexts) {
        mgr.deregisterCustomDraw(ctx);
    }
    g_DrawContexts.clear();
    g_LastUpdatedContext = kInvalidContext;
    mgr.requestTimeSliderRedraw();
}

void hide() {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return;
    }
    auto& mgr = MTimeSliderCustomDrawManager::instance();
    for (auto ctx: g_DrawContexts) {
        mgr.setDrawVisible(ctx, false);
        //mgr.setBackgroundColor(ctx, kSilderBGColor);
        //mgr.setDrawHeight(ctx, kSliderHeight);
    }
    mgr.requestTimeSliderRedraw();
}

void show(CustomDrawID ctx) {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return;
    }
    if (kInvalidContext == ctx) {
        return;
    }
    // taking time as input so it's not depedent on the frame rate
    auto& mgr = MTimeSliderCustomDrawManager::instance();
    for (auto ctx: g_DrawContexts) {
        mgr.setDrawVisible(ctx, false);
    }
    mgr.setDrawVisible(ctx, true);
    mgr.requestTimeSliderRedraw();
    g_LastUpdatedContext = ctx;
}

void draw(CustomDrawID ctx, double seconds, bool isPending, bool done) {
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        // Skip drawing if no GUI
        return;
    }
    if (kInvalidContext == ctx) {
        return;
    }    
    // taking time as input so it's not depedent on the frame rate
    auto& mgr = MTimeSliderCustomDrawManager::instance();

    MTimeSliderDrawPrimitive prim(
        MTimeSliderDrawPrimitive::kFilledRect, 
        MTime(0, MTime::kSeconds), 
        MTime(seconds, MTime::kSeconds),
        isPending ? MColor(0.5f, 0.6f, 0.7f) : (done ? MColor(0.0f, 0.8f, 0.6f) : MColor(1.0f, 0.85f, 0.3f)),
        /* bottom=*/0.0,
        /* height=*/kSliderHeight,
        /* priority=*/0);
    MTimeSliderDrawPrimitives prims;
    prims.append(prim);
    mgr.setDrawPrimitives(ctx, std::move(prims));
    show(ctx);
}

#else

CustomDrawID registerCustomDraw() {
    return kInvalidContext;
}

void deregisterCustomDraw(CustomDrawID ctx) {
    return;
}

void clean() {
    return;
}


void hide() {
    return;
}

void show(CustomDrawID ctx) {
    return;
}

void draw(CustomDrawID ctx, double seconds, bool isPending, bool done) {
    return;
}

#endif

} // namespace TimeSliderProgress
