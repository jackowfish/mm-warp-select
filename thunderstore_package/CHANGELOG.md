# Changelog

## 1.0.1
- Fix a crash on load. The mod now hooks the resident pause-menu wrappers
  (KaleidoScopeCall_Update / KaleidoScopeCall_Draw) instead of the relocatable
  kaleido overlay, and no longer reads overlay data that isn't mapped at load time.

## 1.0.0
- Initial release: switch the pause-menu map page into a warp selector (C-Down) to pick a Song of
  Soaring destination from the map. Only unlocked owl statues are selectable. Configurable button
  and on-screen prompt.
