/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup freestyle
 * \brief Class to render a WingedEdge data structure
 * from a polyhedral data structure organized in nodes of a scene graph.
 */

#include "WEdge.h"

#include "../scene_graph/IndexedFaceSet.h"
#include "../scene_graph/NodeTransform.h"
#include "../scene_graph/SceneVisitor.h"

#include "../system/FreestyleConfig.h"
#include "../system/RenderMonitor.h"

namespace Freestyle {

class WingedEdgeBuilder : public SceneVisitor {
 public:
  inline WingedEdgeBuilder() : SceneVisitor()
  {
    _current_wshape = nullptr;
    _current_frs_material = nullptr;
    _current_matrix = nullptr;
    _winged_edge = new WingedEdge;  // Not deleted by the destructor
    _pRenderMonitor = nullptr;
  }

  virtual ~WingedEdgeBuilder()
  {
    for (vector<Matrix44r *>::iterator it = _matrices_stack.begin(); it != _matrices_stack.end();
         ++it)
    {
      delete *it;
    }
    _matrices_stack.clear();
  }

  VISIT_DECL(IndexedFaceSet);
  VISIT_DECL(NodeShape);
  VISIT_DECL(NodeTransform);

  virtual void visitNodeTransformAfter(NodeTransform &);

  //
  // Accessors
  //
  /////////////////////////////////////////////////////////////////////////////

  inline WingedEdge *getWingedEdge()
  {
    return _winged_edge;
  }

  inline WShape *getCurrentWShape()
  {
    return _current_wshape;
  }

  inline FrsMaterial *getCurrentFrsMaterial()
  {
    return _current_frs_material;
  }

  inline Matrix44r *getCurrentMatrix()
  {
    return _current_matrix;
  }

  //
  // Modifiers
  //
  /////////////////////////////////////////////////////////////////////////////

  inline void setCurrentWShape(WShape *wshape)
  {
    _current_wshape = wshape;
  }

  inline void setCurrentFrsMaterial(FrsMaterial *mat)
  {
    _current_frs_material = mat;
  }

#if 0
  inline void setCurrentMatrix(Matrix44r *matrix)
  {
    _current_matrix = matrix;
  }
#endif

  inline void setRenderMonitor(RenderMonitor *iRenderMonitor)
  {
    _pRenderMonitor = iRenderMonitor;
  }

 protected:
  virtual bool buildWShape(WShape &shape, IndexedFaceSet &ifs);
  virtual void buildWVertices(WShape &shape, const float *vertices, uint vsize);

  RenderMonitor *_pRenderMonitor;

 private:
  void buildTriangleStrip(const float *vertices,
                          const float *normals,
                          vector<FrsMaterial> &iMaterials,
                          const float *texCoords,
                          const IndexedFaceSet::FaceEdgeMark *iFaceEdgeMarks,
                          const uint *vindices,
                          const uint *nindices,
                          const uint *mindices,
                          const uint *tindices,
                          const uint nvertices);

  void buildTriangleFan(const float *vertices,
                        const float *normals,
                        vector<FrsMaterial> &iMaterials,
                        const float *texCoords,
                        const IndexedFaceSet::FaceEdgeMark *iFaceEdgeMarks,
                        const uint *vindices,
                        const uint *nindices,
                        const uint *mindices,
                        const uint *tindices,
                        const uint nvertices);

  void buildTriangles(const float *vertices,
                      const float *normals,
                      vector<FrsMaterial> &iMaterials,
                      const float *texCoords,
                      const IndexedFaceSet::FaceEdgeMark *iFaceEdgeMarks,
                      const uint *vindices,
                      const uint *nindices,
                      const uint *mindices,
                      const uint *tindices,
                      const uint nvertices);

  void transformVertices(const float *vertices,
                         uint vsize,
                         const Matrix44r &transform,
                         float *res);

  void transformNormals(const float *normals, uint nsize, const Matrix44r &transform, float *res);

  WShape *_current_wshape;
  FrsMaterial *_current_frs_material;
  WingedEdge *_winged_edge;
  Matrix44r *_current_matrix;
  vector<Matrix44r *> _matrices_stack;

  MEM_CXX_CLASS_ALLOC_FUNCS("Freestyle:WingedEdgeBuilder")
};

} /* namespace Freestyle */
