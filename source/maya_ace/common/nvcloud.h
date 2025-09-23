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

#include <maya/MString.h>

namespace nvcloud {
// Display NVIDIA Cloud Agreement terms, and acquire an acceptance to proceed.

const MString NVCF_AGREEMENT_OPTIONVAR = "NVCF_AGREEMENT_ACCEPTED";

const MString NVCF_AGREENENT_HTML =
"NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM ('Services')"
" allow you to upload audio files to drive an animation. NVIDIA will only use and store the audio files"
" to provide you with the Services. For more information about our data processing practices, see our"
" <a href='https://www.nvidia.com/en-us/about-nvidia/privacy-policy/'>Privacy Policy</a>."
" By clicking 'Agree', you consent to the processing of your data in accordance with the"
" <a href='https://www.nvidia.com/en-us/agreements/cloud-services/nvidia-cloud-agreement/'>NVIDIA Cloud Agreement</a>"
" and"
" <a href='https://developer.download.nvidia.com/licenses/Service_Specific_Terms_for_NVIDIA_Audio2Face_3D_Authoring_Microservice_and_NVIDIA_Audio2Face_3D_Microservice_NIM.pdf'>"
"Service-Specific Terms for NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM</a>"
".";

const MString NVCF_AGREEMENT =
"NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM ('Services')"
" allow you to upload audio files to drive an animation. NVIDIA will only use and store the audio files"
" to provide you with the Services. For more information about our data processing practices, see our"
" 'Privacy Policy'<https://www.nvidia.com/en-us/about-nvidia/privacy-policy/>."
" By proceeding, you consent to the processing of your data in accordance with the"
" 'NVIDIA Cloud Agreement'<https://www.nvidia.com/en-us/agreements/cloud-services/nvidia-cloud-agreement>"
" and"
" 'Service-Specific Terms for NVIDIA Audio2Face-3D Authoring Microservice and NVIDIA Audio2Face-3D Microservice NIM'"
"<https://developer.download.nvidia.com/licenses/Service_Specific_Terms_for_NVIDIA_Audio2Face_3D_Authoring_Microservice_and_NVIDIA_Audio2Face_3D_Microservice_NIM.pdf>"
".";

const MString NVCF_AGREEMENT_INSTRUCTIONS =
"To agree and proceed, Please set an optionVar " + NVCF_AGREEMENT_OPTIONVAR + " as 1.\n"
" MEL Script: optionVar -iv \"" + NVCF_AGREEMENT_OPTIONVAR + "\" 1;\n"
" Python: maya.cmds.optionVar(iv=['" + NVCF_AGREEMENT_OPTIONVAR + "', 1])\n";

bool acquireAgreement();
}
