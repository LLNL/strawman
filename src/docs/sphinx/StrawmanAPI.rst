.. ############################################################################
.. # Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
.. #
.. # Produced at the Lawrence Livermore National Laboratory
.. #
.. # LLNL-CODE-716457
.. #
.. # All rights reserved.
.. #
.. # This file is part of Conduit.
.. #
.. # For details, see: http://software.llnl.gov/strawman/.
.. #
.. # Please also read strawman/LICENSE
.. #
.. # Redistribution and use in source and binary forms, with or without
.. # modification, are permitted provided that the following conditions are met:
.. #
.. # * Redistributions of source code must retain the above copyright notice,
.. #   this list of conditions and the disclaimer below.
.. #
.. # * Redistributions in binary form must reproduce the above copyright notice,
.. #   this list of conditions and the disclaimer (as noted below) in the
.. #   documentation and/or other materials provided with the distribution.
.. #
.. # * Neither the name of the LLNS/LLNL nor the names of its contributors may
.. #   be used to endorse or promote products derived from this software without
.. #   specific prior written permission.
.. #
.. # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
.. # AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.. # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.. # ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
.. # LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
.. # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.. # DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.. # OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.. # HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
.. # STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
.. # IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
.. # POSSIBILITY OF SUCH DAMAGE.
.. #
.. ############################################################################

Strawman API
============
The top level API for strawman consists of four calls:

  - Open(condiut::Node)
  - Publish(conduit::Node)
  - Execute(conduit::Node)
  - Close()

Open
----
Open provides the initial setup of Strawman from a Conduit Node. 
Options include pipeline type (e.g., VTK-m, Blueprint HDF5, or EAVL) and associated backend if available.
If running in parallel (i.e., MPI), then a MPI comm handle must be supplied.
Strawman will always check the file system for a file called ``strawman_options.json`` that will override compiled in options, and for obvious reasons, a MPI communicator cannot be specified in the file.
Here is a file that would set the pipeline to VTK-m using a TBB backend:


.. code-block:: json

  {
    "pipeline/type"    : "vtkm",
    "pipeline/backend" : "tbb"
  }

A typical integration will include the following code:

.. code-block:: c++

  Strawman strawman;
  conduit::Node strawman_options;
  
  #if USE_MPI
  strawman_options["mpi_comm"] = MPI_Comm_c2f(MPI_COMM_WORLD);
  #endif
  strawman_options["pipeline/type"] = "vtkm";
  strawman_options["pipeline/backend"] = "tbb";

  strawman.Open(strawman_options);

Valid pipelines and backends include:

  - vtkm
    
    - serial
    - cuda
    - tbb
  
  - EAVL
    
    - cpu (will use OpenMP if configured)
    - cuda

  - hdf5
  
Publish
-------
This call publishes data to Strawman through `Conduit Blueprint <http://software.llnl.gov/blueprint_mesh.html>`_ mesh descriptions.
In the Lulesh prox-app, data is already in a form that is compatible with the blueprint conventions and the code to create the Conduit Node is straight-forward:

.. code-block:: c++
      
      // provide state information
      mesh_data["state/time"].set_external(&m_time);
      mesh_data["state/cycle"].set_external(&m_cycle);
      mesh_data["state/domain"] = myRank;

      // coordinate system data
      mesh_data["coordsets/coords/type"] = "explicit";
      mesh_data["coordsets/coords/x"].set_external(m_x);
      mesh_data["coordsets/coords/y"].set_external(m_y);
      mesh_data["coordsets/coords/z"].set_external(m_z);

      // topology data
      mesh_data["topologies/mesh/type"] = "unstructured";
      mesh_data["topologies/mesh/coordset"] = "coords";
      mesh_data["topologies/mesh/elements/shape"] = "hexs";
      mesh_data["topologies/mesh/elements/connectivity"].set_external(m_nodelist);

      // one or more scalar fields      
      mesh_data["fields/p/type"]        = "scalar";
      mesh_data["fields/p/topology"]    = "mesh";
      mesh_data["fields/p/association"] = "element";
      mesh_data["fields/p/values"].set_external(m_p);

If the data does not match the blueprint mesh conventions, then you must transform the data into a compatible format.

You can check if a node confirms to the mesh blueprint using the verify function provided by conduit. 

.. code-block:: c++
    
    #include <conduit_blueprint.hpp>

    Node verify_info;
    if(!conduit::blueprint::mesh::verify(mesh_data,verify_info))
    {
        // verify failed, print error message
        STRAWMAN_INFO("Error: Mesh Blueprint Verify Failed!");
        // show details of what went awry 
        verify_info.print();
    }

Once the Conduit Node has been populated with data conforming to the mesh blueprint, simply publish the data using the Publish call:

.. code-block:: c++

  straman.Publish(mesh_data);

Publish is called each cycle where Strawman is used.

Execute
-------
Execute applies some number of actions to published data.
Each action is described inside of a Conduit Node and passed to the Execute call.
For a full description of supported actions see :ref:`strawman-actions`.

Here is a simple example of adding a plot using the C++ API:

.. code-block:: c++
            
      // In the main simulation loop
      conduit::Node actions;
      conduit::Node &plot = actions.append();
      plot["action"] = "add_plot";
      plot["field_name"] = "p";
      conduit::Node &draw = actions.append();
      draw["action"] = "draw_plots";
      strawman.Publish(mesh_data);
      strawman.Execute(actions);

Close
-----
Close informs Strawman that all actions are complete, and the call performs the appropriate clean-up.

.. code-block:: c++

  strawman.Close();


Error Handling
---------------

  Strawman uses Conduit's error handling machinery. By default when errors occur 
  C++ exceptions are thrown, but you can rewire Conduit's handlers with your own callbacks. For more info
  see the `Conduit Error Handling Tutorial <http://software.llnl.gov/conduit/tutorial_errors.html>`_.





