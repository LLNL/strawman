//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
// 
// LLNL-CODE-716457
// 
// All rights reserved.
// 
// This file is part of Strawman. 
// 
// For details, see: http://software.llnl.gov/strawman/.
// 
// Please also read strawman/LICENSE
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the disclaimer below.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
///
/// file: strawman_mpi_render_3d.cpp
///
//-----------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <strawman.hpp>
#include <iostream>
#include <math.h>


#include <mpi.h>

#include <conduit_blueprint.hpp>

#include "t_config.hpp"
#include "t_strawman_test_utils.hpp"

using namespace std;
using namespace conduit;
using namespace strawman;

//-----------------------------------------------------------------------------
TEST(strawman_mpi_render_3d, mpi_render_3d_default_pipeline)
{
    //
    // Set Up MPI
    //
    int par_rank;
    int par_size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &par_rank);
    MPI_Comm_size(comm, &par_size);
    
    STRAWMAN_INFO("Rank "
                  << par_rank 
                  << " of " 
                  << par_size
                  << " reporting");
    //
    // Create the data.
    //
    Node data, verify_info;
    create_3d_example_dataset(data,par_rank,par_size);
    
    // There is a bug in conduit blueprint related to rectilinear 
    // reenable this check after updating conduit 
    // EXPECT_TRUE(conduit::blueprint::mesh::verify(data,verify_info));
    conduit::blueprint::mesh::verify(data,verify_info);
    if(par_rank == 0)
    {
        verify_info.print();
    }

    // make sure the _output dir exists
    string output_path = "";
    if(par_rank == 0)
    {
        output_path = prepare_output_dir();
    }
    else
    {
        output_path = output_dir();
    }
    
    string output_file = conduit::utils::join_file_path(output_path,"tout_render_mpi_3d_default_pipeline");

    // remove old images before rendering
    remove_test_image(output_file);
    
    //
    // Create the actions.
    //

    Node actions;
    
    Node &plot = actions.append();
    plot["action"]      = "add_plot";
    plot["field_name"]  = "braid";
    
    Node &opts = plot["render_options"];
    opts["width"]  = 500;
    opts["height"] = 500;
    opts["file_name"] = output_file;
    
    actions.append()["action"] = "draw_plots";
    
    //
    // Run Strawman
    //
    
    Strawman sman;

    Node strawman_opts;
    // we use the mpi handle provided by the fortran interface
    // since it is simply an integer
    strawman_opts["mpi_comm"] = MPI_Comm_c2f(comm);
    sman.Open(strawman_opts);
    sman.Publish(data);
    sman.Execute(actions);
    sman.Close();
    MPI_Barrier(comm);    
    // check that we created an image
    EXPECT_TRUE(check_test_image(output_file));
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int result = 0;

    ::testing::InitGoogleTest(&argc, argv);
    MPI_Init(&argc, &argv);
    result = RUN_ALL_TESTS();
    MPI_Finalize();

    return result;
}
