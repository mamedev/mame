// UpdateAction.cpp

#include "StdAfx.h"

#include "UpdateAction.h"

namespace NUpdateArchive {

const CActionSet kAddActionSet =
{{
  NPairAction::kCopy,
  NPairAction::kCopy,
  NPairAction::kCompress,
  NPairAction::kCompress,
  NPairAction::kCompress,
  NPairAction::kCompress,
  NPairAction::kCompress
}};

const CActionSet kUpdateActionSet =
{{
  NPairAction::kCopy,
  NPairAction::kCopy,
  NPairAction::kCompress,
  NPairAction::kCopy,
  NPairAction::kCompress,
  NPairAction::kCopy,
  NPairAction::kCompress
}};

const CActionSet kFreshActionSet =
{{
  NPairAction::kCopy,
  NPairAction::kCopy,
  NPairAction::kIgnore,
  NPairAction::kCopy,
  NPairAction::kCompress,
  NPairAction::kCopy,
  NPairAction::kCompress
}};

const CActionSet kSynchronizeActionSet =
{{
  NPairAction::kCopy,
  NPairAction::kIgnore,
  NPairAction::kCompress,
  NPairAction::kCopy,
  NPairAction::kCompress,
  NPairAction::kCopy,
  NPairAction::kCompress,
}};

const CActionSet kDeleteActionSet =
{{
  NPairAction::kCopy,
  NPairAction::kIgnore,
  NPairAction::kIgnore,
  NPairAction::kIgnore,
  NPairAction::kIgnore,
  NPairAction::kIgnore,
  NPairAction::kIgnore
}};

}
