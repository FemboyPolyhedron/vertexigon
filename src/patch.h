/// patch.h
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discord] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief bps patching
/// mrrp?

#pragma once

/// @brief patch a profile
///
/// @param game     target game
/// @param profile  target profile
/// @param mod      mod name
/// @param ts       timestamp of download
int PATCH_MOD(const char* game, long profile, const char* mod, const char* ts);