# 1.4.2
- Restored the first working Auto Play core instead of the complex predictive rewrite.
- Kept only small safe additions on top: spike/saw hazard inclusion, disabled-object filtering, and broader ground/flying mode detection.
- Removed stale helper code that referenced undefined label/update variables and could break compilation.

# 1.4.1
- Removed the Android-hostile Auto Play danger sort and kept prediction iteration in object-layer order to avoid template comparator build failures.
- Verified the detailed Auto Play path has no legacy `safeTargetY`/`desiredY` references and keeps local velocity variables scoped where used.

# 1.4.0
- Rebuilt Auto Play around near-future player hitbox prediction instead of simple lane checks.
- Added separate solid and spike avoidance so safe floor landings remain allowed while lethal side/head/spike contacts are avoided.
- Increased lookahead and route targeting for ship, UFO, wave, and swing, plus safer tap timing for ball/spider/UFO/swing.
- Updated Assist labels to focus on detailed Auto Play: Auto Play, Avoid Solid, Avoid Spike, and Release.

# 1.3.0
- Expanded Auto Play into an all-mode assist that detects nearby gameplay collision objects, spikes, and wall-like blockers.
- Reworked Assist toggles into Ground AI for cube/ball/robot/spider and Air AI for ship/UFO/wave/swing.
- Added tap-pulse handling so ball, UFO, spider, and swing modes can react without sticking jump forever.

# 1.2.0
- Expanded the menu into a four-tab all-in-one hub: Player, Assist, Visual, and Utility.
- Added additional Mega-Hack-style controls for hiding player/layers/attempts, BG effects, speed control, checkpoint clearing, and auto-input release.
- Improved hitbox handling to set debug draw to the requested state instead of blindly toggling.
- Updated README/about documentation with the actual feature list and usage notes.

# 1.1.0
- Added tabbed in-game UI with Auto Play, Cube AI, Wave AI, Platform assist, and hitbox/debug controls.

# 1.0.0
- Initial draggable Emir Hub menu with No Death, Practice, Progress, Info, Restart, and About actions.
