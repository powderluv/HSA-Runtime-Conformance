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
 * Test Name: aql_private_memory
 * Scope: Conformance
 *
 * Purpose: Verifies that a kernel that uses private memory can be dispatched properly.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) For an agent create a queue.
 * 3) Query the agent's memory regions and ensure that a region exists that is a
 * HSA_SEGMENT_PRIVATE segment type.
 * 3) Load and initialize the private_memory kernel.
 * 4) Create a completion signal for dispatch.
 * 5) Enqueue a dispatch packet, specifying the correct private_segment_size in the
 * AQL packet obtained from the hsa_ext_code_descriptor_t structure.
 * 6) Wait for the kernel to finish executing and verify that it executed correctly.
 *
 * Expected Results: The kernel should execute correctly.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

int test_aql_private_memory() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("private_memory.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Find a memory region in the global segment
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 == global_region.handle) {
            // Skip the agent if global fine grained memory isn't available
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the executable
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

        status = finalize_executable(agent_list.agents[ii],
                                     1,
                                     &module,
                                     HSA_MACHINE_MODEL_LARGE,
                                     HSA_PROFILE_FULL,
                                     HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO,
                                     HSA_CODE_OBJECT_TYPE_PROGRAM,
                                     0,
                                     control_directives,
                                     &code_object,
                                     &executable);

        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        char* symbol_names[1];
        symbol_names[0] = "&__private_memory_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the data block to be used by the kernel
        // The size of the data block must be able to fit into one workgroup
        uint32_t block_size = 128;
        uint32_t* data_in;
        uint32_t* data_out;
        status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_in);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_out);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_memory_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Initialize the data
        memset(data_out, 0, block_size * sizeof(uint32_t));
        int kk;
        for (kk = 0; kk < block_size; ++kk) {
            data_in[kk] = kk;
        }
        int mult = 2;

        launch_memory_kernel(queue, data_in, data_out, block_size,
                symbol_record.private_segment_size,
                0,
                symbol_record.kernel_object,
                kernarg_buffer);

        // Verify the kernel was executed correctly
        for (kk = 0; kk < block_size; ++kk) {
            if (data_in[kk] != data_out[kk]) {
                ASSERT(0);
            }
        }

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the data buffers
        status = hsa_memory_free(data_in);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(data_out);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);
		
        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy program
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
