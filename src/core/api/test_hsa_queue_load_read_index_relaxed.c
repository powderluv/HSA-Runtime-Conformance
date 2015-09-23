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

/**
 *
 * Test Name: hsa_queue_load_read_index_relaxed
 *
 * Purpose: Verify that API of hsa_queue_load_read_index_relaxed() works as expected
 *
 * Description:
 *
 * 1) Load the read index from a valid, normal queue.
 *    a) Check if the read_index is 0 once the queue is created.
 *    b) Launch a few kernels, check if the read_index is updated
 *       correctly.
 *    c) Repeat b) several times, and check the read_index is correct
 *       when the it is greater than a set multiple of the queue_size.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include "test_helper_func.h"

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_queue_load_read_index_relaxed() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Don't create a queue for test on an agent that does not
            // support DISPATCH.
            continue;
        }

        // Create a queue
        hsa_queue_t* queue;
        const uint32_t queue_size = 4;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Verify the read_index has been initialized to 0
        uint64_t read_index = hsa_queue_load_read_index_relaxed(queue);
        ASSERT(0 == read_index);

        int jj;
        uint64_t total_packets_launched = 0;
        for (jj = 1; jj < queue_size; ++jj) {
            // Launch a few kernels
            launch_no_op_kernels(agent_list.agents + ii, queue, jj);

            // Verify the read_index has been updated correctly
            read_index = hsa_queue_load_read_index_relaxed(queue);
            ASSERT_MSG(total_packets_launched + jj == read_index, "The read index was not updated correctly.\n");
            total_packets_launched = read_index;
        }

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}