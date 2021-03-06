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

/*
 * Test Name: memory_allocate_max_size
 *
 * Purpose: Verify that hsa_memory_allocate can't allocate more than MAX_ALLOC_SIZE memory once.
 *
 * Test Description:
 *
 * 1. Iterate over all of the agents in the system.
 *
 * 2. For each agent, iterate over all of the memory regions.
 *
 * 3. Get the attribute of HSA_REGION_INFO_ALLOC_MAX_SIZE.
 *
 * 4. Call hsa_memory_allocate with a size greater than HSA_REGION_ALLOC_MAX_SIZE.
 *
 * 5. Check that the API return the correct error code.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_allocate_max_size() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    hsa_device_type_t agent_type;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        hsa_agent_t agent = agent_list.agents[ii];

        // Get the list of regions
        struct region_list_s region_list;
        get_region_list(agent, &region_list);

        int jj;
        for (jj = 0; jj < region_list.num_regions; ++jj) {
            // Determine if allocation requests are allowed
            bool allowed;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED, &allowed);

            if (!allowed) {
                continue;
            }

            size_t max_size;
            status = hsa_region_get_info(region_list.regions[jj], HSA_REGION_INFO_ALLOC_MAX_SIZE, &max_size);

            char *buf;
            status = hsa_memory_allocate(region_list.regions[jj], (max_size + 16), (void**) &buf);
            ASSERT(status == HSA_STATUS_ERROR_INVALID_ALLOCATION);
        }

        free_region_list(&region_list);
    }

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
