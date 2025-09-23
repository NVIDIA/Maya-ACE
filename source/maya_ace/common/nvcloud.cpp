// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "nvcloud.h"

#include <maya/MGlobal.h>

namespace nvcloud {

bool acquireAgreement() {
    // OPTME: utilize Maya's preference system to store the consent
    bool exists;
    int agreed = MGlobal::optionVarIntValue(NVCF_AGREEMENT_OPTIONVAR, &exists);

    if (exists && agreed) {
        MGlobal::displayInfo("You agreed to the terms of the NVIDIA Cloud Agreement. Proceeding.");
        return true;
    }

    // detect if we are running in batch mode
    if (MGlobal::mayaState() != MGlobal::kInteractive) {
        MGlobal::displayInfo("\n" + NVCF_AGREEMENT + "\n");
        MGlobal::displayError(NVCF_AGREEMENT_INSTRUCTIONS);
        return false;
    }

    // pop up dialog
    MString command = "confirmDialog -title \"Maya-ACE Prominent Disclosure\" "
                    "-message \"" + NVCF_AGREENENT_HTML + "\" "
                    "-button \"Agree\" "
                    "-button \"Disagree\" "
                    "-defaultButton \"Agree\" "
                    "-dismissString \"Disagree\";";
    MString response;
    MGlobal::executeCommand(command, response);

    // Check user response
    if (response == "Agree") {
        MGlobal::displayInfo("Thank you for agreeing to the terms!");
        MGlobal::setOptionVarValue(NVCF_AGREEMENT_OPTIONVAR, 1);
        return true;
    }

    MGlobal::displayError("You must agree to the terms to proceed.");
    return false;
}
} // namespace nvcf
