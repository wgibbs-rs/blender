/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup spoutliner
 */

#include "BLT_translation.hh"

#include "BKE_library.hh"

#include "DNA_ID.h"

#include "../outliner_intern.hh"

#include "tree_element_id_library.hh"

namespace blender::ed::outliner {

TreeElementIDLibrary::TreeElementIDLibrary(TreeElement &legacy_te, Library &library)
    : TreeElementID(legacy_te, library.id)
{
  legacy_te.name = library.filepath;
}

StringRefNull TreeElementIDLibrary::get_warning() const
{
  Library &library = reinterpret_cast<Library &>(id_);

  if (library.runtime->tag & LIBRARY_TAG_RESYNC_REQUIRED) {
    return RPT_(
        "Contains linked library overrides that need to be resynced, updating the library is "
        "recommended");
  }

  if (library.id.tag & ID_TAG_MISSING) {
    return RPT_("Missing library");
  }

  return {};
}

}  // namespace blender::ed::outliner
