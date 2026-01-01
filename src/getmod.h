/// getmod.h
///
/// (c) femboypolyhedron, 2025
/// 
/// Authors
/// - femboypolyhedron
///   [GitHub] @FemboyPolyhedron
///   [Discord] @elephant_lover
///   [Youtube] @FemboyPolyhedron
///
/// @brief fetch and install mods
/// the only file with cpp compatibility

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int GET_MOD(const char* src, char* out_mod, char* out_ts);
int INSTALL_MOD(const char* game, long profile, const char* mod, const char* ts);

#ifdef __cplusplus
}
#endif
