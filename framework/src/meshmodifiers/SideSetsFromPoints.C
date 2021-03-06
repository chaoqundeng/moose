//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromPoints.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMesh.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

registerMooseObject("MooseApp", SideSetsFromPoints);

template <>
InputParameters
validParams<SideSetsFromPoints>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params.addClassDescription("Adds a new sideset starting at the specified point containing all "
                             "connected element faces with the same normal.");
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");
  params.addRequiredParam<std::vector<Point>>(
      "points", "A list of points from which to start painting sidesets");
  return params;
}

SideSetsFromPoints::SideSetsFromPoints(const InputParameters & parameters)
  : AddSideSetsBase(parameters),
    _boundary_names(getParam<std::vector<BoundaryName>>("new_boundary")),
    _points(getParam<std::vector<Point>>("points"))
{
  if (_points.size() != _boundary_names.size())
    mooseError("point list and boundary list are not the same length");
}

void
SideSetsFromPoints::modify()
{
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SideSetsFromPoints::modify()!");

  // We can't call this in the constructor, it appears that _mesh_ptr is always NULL there.
  _mesh_ptr->errorIfDistributedMesh("SideSetsFromPoints");

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  setup();

  _visited.clear();

  std::unique_ptr<PointLocatorBase> pl = PointLocatorBase::build(TREE, *_mesh_ptr);

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    const Elem * elem = (*pl)(_points[i]);

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor_ptr(side))
        continue;

      // See if this point is on this side
      std::unique_ptr<const Elem> elem_side = elem->side_ptr(side);

      if (elem_side->contains_point(_points[i]))
      {
        // This is the side that we want to paint our sideset with
        // First get the normal
        _fe_face->reinit(elem, side);
        const std::vector<Point> & normals = _fe_face->get_normals();

        flood(elem, normals[0], boundary_ids[i]);
      }
    }
  }

  finalize();

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    _mesh_ptr->getMesh().boundary_info->sideset_name(boundary_ids[i]) = _boundary_names[i];
}
