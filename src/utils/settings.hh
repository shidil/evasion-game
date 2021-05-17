#ifndef SETTINGS_H
#define SETTINGS_H
#include <raylib.h>

#include <cmath>
#include <string>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

namespace evs {
inline float read_high_score() {
  double score_encoded = 0;
#if defined(PLATFORM_WEB)
  score_encoded = EM_ASM_DOUBLE(
      { return localStorage.getItem('ac152700-b6e1-11eb-8529-0242ac130003') || 0; });
#else
return 0;
#endif
  return (float)score_encoded / 0.123678459;
}

inline void set_high_score(float score) {
  double score_encoded = score * 0.123678459;
#if defined(PLATFORM_WEB)
  EM_ASM({ localStorage.setItem('ac152700-b6e1-11eb-8529-0242ac130003', $0); },
         score_encoded);
#else
#endif
}
}  // namespace evs

#endif  // SETTINGS_H
