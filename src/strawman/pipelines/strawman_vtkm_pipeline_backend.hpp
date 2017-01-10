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
/// file: strawman_vtkm_pipeline_backend.hpp
///
//-----------------------------------------------------------------------------

#ifndef STRAWMAN_VTKM_PIPELINE_BACKEND_HPP
#define STRAWMAN_VTKM_PIPELINE_BACKEND_HPP

#include "strawman_pipeline.hpp"


// thirdparty includes
// VTKm includes

namespace vtkm
{ 
namespace cont
{
class DataSet;
};
};

// conduit includes
#include <conduit.hpp>


//-----------------------------------------------------------------------------
// -- begin strawman:: --
//-----------------------------------------------------------------------------
namespace strawman
{
template<typename DeviceAdatper>
class Renderer;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Templated class that implements our actual VTKm Pipeline
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class DEVICE_ADAPTOR>
class VTKMPipelineBackend: public Pipeline
{
public:

                  VTKMPipelineBackend();
    virtual      ~VTKMPipelineBackend();
    
    void  Initialize(const conduit::Node &options);

    void  Publish(const conduit::Node &data);
    void  Execute(const conduit::Node &actions);
    
    void  Cleanup();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class that Handles Blueprint to VTKm Data Transforms
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class DataAdapter 
{
public:
    // convert blueprint data to an vtkm Data Set
    // assumes "n" conforms to the mesh blueprint
    //
    //  conduit::blueprint::mesh::verify(n,info) == true
    //
    static vtkm::cont::DataSet  *BlueprintToVTKmDataSet(const conduit::Node &n,
                                                        const std::string &field_name);


private:
    // helpers for specific conversion cases
    static vtkm::cont::DataSet  *UniformBlueprintToVTKmDataSet(const std::string &coords_name,
                                                               const conduit::Node &n_coords,
                                                               const std::string &topo_name,
                                                               const conduit::Node &n_topo,
                                                               int &neles,
                                                               int &nverts);


    static vtkm::cont::DataSet  *RectilinearBlueprintToVTKmDataSet(const std::string &coords_name,
                                                                   const conduit::Node &n_coords,
                                                                   const std::string &topo_name,
                                                                   const conduit::Node &n_topo,
                                                                   int &neles,
                                                                   int &nverts);

    static vtkm::cont::DataSet  *StructuredBlueprintToVTKmDataSet(const std::string &coords_name,
                                                                  const conduit::Node &n_coords,
                                                                  const std::string &topo_name,
                                                                  const conduit::Node &n_topo,
                                                                  int &neles,
                                                                  int &nverts);

     static vtkm::cont::DataSet *UnstructuredBlueprintToVTKmDataSet(const std::string &coords_name,
                                                                    const conduit::Node &n_coords,
                                                                    const std::string &topo_name,
                                                                    const conduit::Node &n_topo,
                                                                    int &neles,
                                                                    int &nverts);

    // helper for adding field data
    static void                  AddVariableField(const std::string &field_name,
                                                  const conduit::Node &n_field,
                                                  const std::string &topo_name,
                                                  int neles,
                                                  int nverts,
                                                  vtkm::cont::DataSet *dset);

};

private:
    //forward declarations
    class Plot;
    //class Renderer;

    // Actions
    void            DrawPlots();
    void            RenderPlot(const int plot_id,
                               const conduit::Node &render_options);
    // conduit node that (externally) holds the data from the simulation 
    conduit::Node     m_data; 

    // holds the pipeline's plots
    std::vector<Plot> m_plots;

    Renderer<DEVICE_ADAPTOR> *m_renderer;

    int cuda_device;
    // actions
    void            AddPlot(const conduit::Node &action);
};

//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// -- end strawman:: --
//-----------------------------------------------------------------------------

#endif
//-----------------------------------------------------------------------------
// -- end header ifdef guard
//-----------------------------------------------------------------------------


