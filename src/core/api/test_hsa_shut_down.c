/*
 * =============================================================================
 *   HSA Runtime Conformance Release License
 * =============================================================================
 * The University of Illinois/NCSA
 * Open Source License (NCSA)
 *
 * Copyright (c) 2014, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Developed by:
 *
 *                 AMD Research and AMD HSA Software Development
 *
 *                 Advanced Micro Devices, Inc.
 *
 *                 www.amd.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in
 *    the documentation and/or other materials provided with the distribution.
 *  - Neither the names of <Name of Development Group, Name of Institution>,
 *    nor the names of its contributors may be used to endorse or promote
 *    products derived from this Software without specific prior written
 *    permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

#include <hsa.h>
#include <framework.h>

/**
 *
 * Test Name: hsa_shut_down
 *
 * Purpose: Verify that if API works as expected.
 *
 * Description:
 * 1) After opening one instance of HSA runtime, shut it down.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before opening an instance of HSA runtime, shut it down.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Open and shut down an instance of HSA runtime, then shut it down again.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 */

/**
 *
 * @Brief:
 * Implement description #1.
 * Shutdown is verified with hsa_init() together, actually.
 * Should we delete this test case?
 *
 * @Return
 * int
 *
 */

int test_hsa_shut_down() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The call to hsa_shut_down() failed.\n");

    return 0;
}

/**
 *
 * @Brief
 * Implement description #2.
 *
 * @Return
 * int
 *
 */

int test_hsa_shut_down_not_initialized() {
    hsa_status_t status;

    status = hsa_shut_down();
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "Calling hsa_shut_down() didn't return HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.");

    return 0;
}

/**
 *
 * @Brief
 * Implement description #3.
 *
 * @Return
 * int
 *
 */

int test_hsa_shut_down_after_shut_down() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "Calling hsa_shut_down() didn't return HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.");

    return 0;
}
