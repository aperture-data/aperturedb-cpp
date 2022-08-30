#pragma once

#include "util/gcc_util.h"  // DISABLE_WARNING

// Cleaning after developers of protobuf 3.9.0:
#ifndef GOOGLE_PROTOBUF_USE_UNALIGNED
#define GOOGLE_PROTOBUF_USE_UNALIGNED 0
#endif

#ifndef GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE
#define GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE 0
#endif

DISABLE_WARNING(effc++)
DISABLE_WARNING(useless-cast)
DISABLE_WARNING(suggest-override)
DISABLE_WARNING(deprecated-declarations)
DISABLE_WARNING(shadow)
DISABLE_WARNING(redundant-decls)
DISABLE_WARNING(deprecated-declarations)
#include "aperturedb/queryMessage.pb.h"
ENABLE_WARNING(deprecated-declarations)
ENABLE_WARNING(redundant-decls)
ENABLE_WARNING(shadow)
ENABLE_WARNING(deprecated-declarations)
ENABLE_WARNING(suggest-override)
ENABLE_WARNING(useless-cast)
ENABLE_WARNING(effc++)
