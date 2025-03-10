/* SPDX-FileCopyrightText: 2024 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 */

#pragma once

#include "render_graph/vk_command_buffer_wrapper.hh"
#include "render_graph/vk_render_graph_links.hh"
#include "render_graph/vk_resource_state_tracker.hh"
#include "vk_common.hh"
#include "vk_pipeline_data.hh"

namespace blender::gpu::render_graph {

/**
 * Type of nodes of the render graph.
 */
enum class VKNodeType {
  UNUSED,
  BEGIN_QUERY,
  BEGIN_RENDERING,
  BLIT_IMAGE,
  CLEAR_ATTACHMENTS,
  CLEAR_COLOR_IMAGE,
  CLEAR_DEPTH_STENCIL_IMAGE,
  COPY_BUFFER,
  COPY_IMAGE,
  COPY_IMAGE_TO_BUFFER,
  COPY_BUFFER_TO_IMAGE,
  DISPATCH,
  DISPATCH_INDIRECT,
  DRAW,
  DRAW_INDEXED,
  DRAW_INDEXED_INDIRECT,
  DRAW_INDIRECT,
  END_QUERY,
  END_RENDERING,
  FILL_BUFFER,
  RESET_QUERY_POOL,
  SYNCHRONIZATION,
  UPDATE_BUFFER,
  UPDATE_MIPMAPS,
};

BLI_INLINE std::ostream &operator<<(std::ostream &os, const VKNodeType node_type)
{
  switch (node_type) {
    case VKNodeType::UNUSED:
      os << "UNUSED";
      break;
    case VKNodeType::BEGIN_QUERY:
      os << "BEGIN_QUERY";
      break;
    case VKNodeType::BEGIN_RENDERING:
      os << "BEGIN_RENDERING";
      break;
    case VKNodeType::END_QUERY:
      os << "END_QUERY";
      break;
    case VKNodeType::END_RENDERING:
      os << "END_RENDERING";
      break;
    case VKNodeType::CLEAR_ATTACHMENTS:
      os << "CLEAR_ATTACHMENTS";
      break;
    case VKNodeType::CLEAR_COLOR_IMAGE:
      os << "CLEAR_COLOR_IMAGE";
      break;
    case VKNodeType::CLEAR_DEPTH_STENCIL_IMAGE:
      os << "CLEAR_DEPTH_STENCIL_IMAGE";
      break;
    case VKNodeType::FILL_BUFFER:
      os << "FILL_BUFFER";
      break;
    case VKNodeType::COPY_BUFFER:
      os << "COPY_BUFFER";
      break;
    case VKNodeType::COPY_IMAGE:
      os << "COPY_IMAGE";
      break;
    case VKNodeType::COPY_IMAGE_TO_BUFFER:
      os << "COPY_IMAGE_TO_BUFFER";
      break;
    case VKNodeType::COPY_BUFFER_TO_IMAGE:
      os << "COPY_BUFFER_TO_IMAGE";
      break;
    case VKNodeType::BLIT_IMAGE:
      os << "BLIT_IMAGE";
      break;
    case VKNodeType::DISPATCH:
      os << "DISPATCH";
      break;
    case VKNodeType::DISPATCH_INDIRECT:
      os << "DISPATCH_INDIRECT";
      break;
    case VKNodeType::DRAW:
      os << "DRAW";
      break;
    case VKNodeType::DRAW_INDEXED:
      os << "DRAW_INDEXED";
      break;
    case VKNodeType::DRAW_INDEXED_INDIRECT:
      os << "DRAW_INDEXED_INDIRECT";
      break;
    case VKNodeType::DRAW_INDIRECT:
      os << "DRAW_INDIRECT";
      break;
    case VKNodeType::RESET_QUERY_POOL:
      os << "RESET_QUERY_POOL";
      break;
    case VKNodeType::SYNCHRONIZATION:
      os << "SYNCHRONIZATION";
      break;
    case VKNodeType::UPDATE_BUFFER:
      os << "UPDATE_BUFFER";
      break;
    case VKNodeType::UPDATE_MIPMAPS:
      os << "UPDATE_MIPMAPS";
      break;
  }
  return os;
}

BLI_INLINE bool node_type_is_within_rendering(VKNodeType node_type)
{
  return ELEM(node_type,
              VKNodeType::CLEAR_ATTACHMENTS,
              VKNodeType::DRAW,
              VKNodeType::DRAW_INDEXED,
              VKNodeType::DRAW_INDEXED_INDIRECT,
              VKNodeType::DRAW_INDIRECT);
}

BLI_INLINE bool node_type_is_rendering(VKNodeType node_type)
{
  return ELEM(node_type, VKNodeType::BEGIN_RENDERING, VKNodeType::END_RENDERING) ||
         node_type_is_within_rendering(node_type);
}

/**
 * Info class for a node type.
 *
 * Nodes can be created using `NodeCreateInfo`. When a node is created the `VKNodeInfo.node_type`
 * and `VKNodeInfo.set_node_data` are used to fill a VKRenderGraphNode instance. The
 * VKRenderGraphNode is stored sequentially in the render graph. When the node is created the
 * dependencies are extracted by calling `VKNodeInfo.build_links`.
 *
 * Eventually when a node is recorded to a command buffer `VKNodeInfo.build_commands` is invoked.
 */
template<VKNodeType NodeType,
         typename NodeCreateInfo,
         typename NodeData,
         VkPipelineStageFlags PipelineStage,
         VKResourceType ResourceUsages>
class VKNodeInfo : public NonCopyable {

 public:
  using CreateInfo = NodeCreateInfo;
  using Data = NodeData;

  /**
   * Node type of this class.
   *
   * The node type used to link VKRenderGraphNode instance to a VKNodeInfo.
   */
  static constexpr VKNodeType node_type = NodeType;

  /**
   * Which pipeline stage does this command belongs to. The pipeline stage is used when generating
   * pipeline barriers.
   */
  static constexpr VkPipelineStageFlags pipeline_stage = PipelineStage;

  /**
   * Which resource types are relevant. Some code can be skipped when a node can only depend on
   * resources of a single type.
   */
  static constexpr VKResourceType resource_usages = ResourceUsages;

  /**
   * Update the node data with the data inside create_info.
   *
   * Has been implemented as a template to ensure all node specific data
   * (`Data`/`CreateInfo`) types can be included in the same header file as the logic. The
   * actual node data (`VKRenderGraphNode` includes all header files.)
   *
   * This function must be implemented by all node classes. But due to cyclic inclusion of header
   * files it is implemented as a template function.
   */
  template<typename Node, typename Storage>
  static void set_node_data(Node &node, Storage &storage, const CreateInfo &create_info);

  /**
   * Extract read/write resource dependencies from `create_info` and add them to `node_links`.
   */
  virtual void build_links(VKResourceStateTracker &resources,
                           VKRenderGraphNodeLinks &node_links,
                           const CreateInfo &create_info) = 0;

  /**
   * Build the commands and add them to the command_buffer.
   *
   * The command buffer is passed as an interface as this is replaced by a logger when running test
   * cases. The test cases will validate the log to find out if the correct commands where added.
   */
  virtual void build_commands(VKCommandBufferInterface &command_buffer,
                              Data &data,
                              VKBoundPipelines &r_bound_pipelines) = 0;
};
}  // namespace blender::gpu::render_graph
