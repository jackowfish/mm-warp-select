/*
 * Warp From Map
 *
 * Adds a "warp map" mode to the pause menu's world-map page so you can pick a
 * Song of Soaring destination directly from the map instead of playing the song.
 *
 * The game already contains a complete owl-warp selector (the PAUSE_STATE_OWL_WARP_*
 * states that the Song of Soaring drops you into). It draws the map, a cursor over
 * only the *unlocked* owl statues, the location-name panel, a "Warp to ___? Yes/No"
 * confirmation, and on close it spawns the actor that performs the warp. This mod
 * simply switches the open map page into that native selector on a button press, and
 * lets you toggle back out to the normal map. We don't reimplement any of it.
 */
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

#include "warp_prompt_tex.h"

// Static game symbol not declared in any header; resolved from the reference syms.
extern s16 sInDungeonScene;

// Which C button toggles warp mode. Index matches the mod.toml "toggle_button" enum.
static u16 get_toggle_btn(void) {
    switch (recomp_get_config_u32("toggle_button")) {
        case 1:  return BTN_CUP;
        case 2:  return BTN_CLEFT;
        case 3:  return BTN_CRIGHT;
        default: return BTN_CDOWN;
    }
}

// True when the player is looking at the overworld map page, idle, with at least one
// owl statue unlocked, and soaring isn't restricted in this area.
static s32 can_enter_warp(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    return (pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
           (pauseCtx->pageIndex == PAUSE_MAP) && !sInDungeonScene &&
           (gSaveContext.save.saveInfo.playerData.owlActivationFlags != 0) &&
           (play->interfaceCtx.restrictions.songOfSoaring == 0);
}

// Drop the open map page straight into the native owl-warp selector. We only have to
// reproduce the small amount of state that func_800F4A10's owl-warp branch sets up;
// the loaded map segments and the world-map view registers are already correct because
// the pause menu is open on the map page (both paths share that setup).
static void enter_warp_select(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u16 owlFlags = gSaveContext.save.saveInfo.playerData.owlActivationFlags;
    s32 i;

    // Where to return to if the player backs out (see KaleidoScope owl-warp cleanup).
    pauseCtx->unk_2C8 = PAUSE_MAP;
    pauseCtx->unk_2CA = pauseCtx->cursorPoint[PAUSE_WORLD_MAP];

    // Show only the unlocked owl statues, and park the cursor on one of them.
    for (i = 0; i < (s32)ARRAY_COUNT(pauseCtx->worldMapPoints); i++) {
        pauseCtx->worldMapPoints[i] = false;
    }
    for (i = OWL_WARP_STONE_TOWER; i >= OWL_WARP_GREAT_BAY_COAST; i--) {
        if ((owlFlags >> i) & 1) {
            pauseCtx->worldMapPoints[i] = true;
            pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = i;
        }
    }
    if ((owlFlags >> OWL_WARP_CLOCK_TOWN) & 1) {
        pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = OWL_WARP_CLOCK_TOWN;
    }

    func_8011552C(play, DO_ACTION_WARP); // change the A-button prompt to "Warp"

    R_PAUSE_OWL_WARP_ALPHA = 120;        // dim overlay used behind the warp map
    pauseCtx->infoPanelOffsetY = 0;
    pauseCtx->namedItem = PAUSE_ITEM_NONE; // force the location-name panel to reload
    pauseCtx->cursorSpecialPos = 0;
    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
    pauseCtx->promptChoice = PAUSE_PROMPT_YES;
    pauseCtx->state = PAUSE_STATE_OWL_WARP_SELECT;

    Audio_PlaySfx(NA_SE_SY_DECIDE);
}

// Back out of warp mode to the normal world map, keeping the pause menu open.
static void exit_warp_select(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 i;

    // Restore the normal map: region dots for every visited region.
    for (i = 0; i < (s32)ARRAY_COUNT(pauseCtx->worldMapPoints); i++) {
        pauseCtx->worldMapPoints[i] = false;
    }
    for (i = 0; i < REGION_MAX; i++) {
        if ((gSaveContext.save.saveInfo.regionsVisited >> i) & 1) {
            pauseCtx->worldMapPoints[i] = true;
        }
    }

    func_8011552C(play, DO_ACTION_NONE);
    pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = pauseCtx->unk_2CA;
    pauseCtx->namedItem = PAUSE_ITEM_NONE;
    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
    pauseCtx->state = PAUSE_STATE_MAIN;

    Audio_PlaySfx(NA_SE_SY_CANCEL);
}

// Runs before the pause menu's per-frame update.
RECOMP_HOOK("KaleidoScope_Update") void warp_from_map_update(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    Input* input = CONTROLLER1(&play->state);
    u16 toggle = get_toggle_btn();

    if (can_enter_warp(play) && CHECK_BTN_ALL(input->press.button, toggle)) {
        enter_warp_select(play);
        return;
    }

    // While selecting, the toggle button or B backs out to the normal map. Start still
    // closes the whole menu and A still confirms a destination (both handled natively),
    // so we consume only the buttons we act on here.
    if (pauseCtx->state == PAUSE_STATE_OWL_WARP_SELECT &&
        CHECK_BTN_ANY(input->press.button, toggle | BTN_B)) {
        exit_warp_select(play);
        input->press.button &= ~(toggle | BTN_B);
    }
}

// Draw the "press C-Down for Warp Map" prompt over the world map page.
static void draw_prompt(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 x = (SCREEN_WIDTH - WARP_PROMPT_TEX_WIDTH) / 2;
    s32 y = 202;

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_1CYCLE);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    gDPLoadTextureBlock(POLY_OPA_DISP++, gWarpPromptTex, G_IM_FMT_IA, G_IM_SIZ_8b, WARP_PROMPT_TEX_WIDTH,
                        WARP_PROMPT_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(POLY_OPA_DISP++, x << 2, y << 2, (x + WARP_PROMPT_TEX_WIDTH) << 2, (y + WARP_PROMPT_TEX_HEIGHT) << 2,
                        G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

    gDPPipeSync(POLY_OPA_DISP++);

    CLOSE_DISPS(play->state.gfxCtx);
}

// Runs after the world map page finishes drawing, so our prompt lands on top.
RECOMP_HOOK_RETURN("KaleidoScope_DrawWorldMap") void warp_from_map_draw(PlayState* play) {
    if (recomp_get_config_u32("show_prompt") != 0) { // 0 = Shown
        return;
    }
    if (can_enter_warp(play)) {
        draw_prompt(play);
    }
}
