#include <array>
#include <format>
#include <string>
#include <3ds.h>
#include <sys/stat.h>
#include "MainUI.hpp"
#include "../sysmodules/acta.hpp"
#include "../sysmodules/httpc.hpp"
#include "../plgldr.h"
#include "../Backup.hpp"

constexpr Result ResultFPDLocalAccountNotExists = 0xC880C4ED; // FPD::LocalAccountNotExists
const char *NIMBUS_PLUGIN = "/luma/plugins/nimbus.3gx";
const char *NIMBUS_PLUGIN_MAGIC = "NMBS";
constexpr u32 NIMBUS_PLUGIN_VERSION = SYSTEM_VERSION(1, 0, 0);

Result retPNID = 0;
u32 pnidAccountSlot = 0;
AccountId pnid = {};

Result MainUI::unloadAccount(MainStruct *mainStruct) {
    Result rc = 0;

    handleResult(ACTA_UnloadConsoleAccount(), mainStruct, "Unload ACT account");
    if (R_FAILED(rc)) {
        return rc;
    }

    // In order to unload the friends account we need to make it go offline. Enter into exclusive state to disconnect from online
    handleResult(NDMU_EnterExclusiveState(NDM_EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS), mainStruct, "Enter exclusive state");
    if (R_FAILED(rc)) {
        return rc;
    }

    bool online;
    // If the console was connected to online, wait until it disconnects.
    // I tried doing this through notification events but it didn't seem to work
    while (true) {
        handleResult(FRD_IsOnline(&online), mainStruct, "Online check");
        if (R_FAILED(rc)) {
            return rc;
        }

        if (!online) break;
    }

    // Now that the console is offline, unload the account
    handleResult(FRDA_UnloadLocalAccount(), mainStruct, "Unload friends account");

    return rc;
}

Result MainUI::switchAccounts(MainStruct *mainStruct, u8 friend_account_id) {
    Result rc = 0;

    handleResult(FRDA_LoadLocalAccount(friend_account_id), mainStruct, "Switch account");
    if (R_FAILED(rc)) {
        return rc;
    }

    u32 act_account_index = 0;
    handleResult(ACT_GetAccountIndexOfFriendAccountId(&act_account_index, friend_account_id), mainStruct, "Get ACT account ID of friend account ID");
    if (R_FAILED(rc)) {
        return rc;
    }

    if (act_account_index == 0) {
        u32 account_count;
        handleResult(ACT_GetAccountCount(&account_count), mainStruct, "Get account count");
        if (R_FAILED(rc)) {
            return rc;
        }

        handleResult(ACTA_CreateConsoleAccount(), mainStruct, "Create ACT account");
        if (R_FAILED(rc)) {
            return rc;
        }

        act_account_index = account_count + 1;

        handleResult(ACTA_CommitConsoleAccount(act_account_index), mainStruct, "Commit ACT account");
        if (R_FAILED(rc)) {
            return rc;
        }
    }

    handleResult(ACTA_SetDefaultAccount(act_account_index), mainStruct, "Set default account");

    return rc;
}

Result MainUI::createAccount(MainStruct *mainStruct, u8 friend_account_id, NascEnvironment environmentId) {
    Result rc = 0;

    // (Re)Create the friend account
    handleResult(FRDA_CreateLocalAccount(friend_account_id, static_cast<u8>(environmentId), 0, 1), mainStruct, "Create account");
    if (R_FAILED(rc)) {
        return rc;
    }

    // Switch to the friend/act accounts
    handleResult(switchAccounts(mainStruct, friend_account_id), mainStruct, "Switch account");
    if (R_FAILED(rc)) {
        return rc;
    }

    // Reset the act account
    handleResult(ACTA_UnbindServerAccount(friend_account_id, true), mainStruct, "Reset account");

    return rc;
}

Result MainUI::handleAzahar(u8 friend_account_id) {
    s64 emu_id = 0;
    svcGetSystemInfo(&emu_id, 0x20000, 0);

    if (emu_id != 2) {
        // This is not Azahar, nothing to do
        return 0;
    }

    std::array<std::string, 3> patterns = {{
        {"nintendowifi\\.net"},
        {"nintendo\\.net"},
        {"pokemon-gl\\.com"}
    }};
    const std::string replacement = "openpak.org";

    Result res = httpcInit(0x1000);
    if (friend_account_id == 2) {
        // Register OpenPak replacement URLs
        if (R_SUCCEEDED(res)) {
            for (const auto& pattern : patterns) {
                res = HTTPC_Azahar_RegisterURLReplacement(pattern.c_str(), replacement.c_str(), pattern.size(), replacement.size());
                if (R_FAILED(res)) {
                    // If the rule already exists clear it and register again
                    if (res == MAKERESULT(RL_STATUS, RS_INVALIDARG, RM_HTTP, RD_ALREADY_EXISTS)) {
                        res = HTTPC_Azahar_UnregisterURLReplacement(pattern.c_str(), pattern.size());
                        if (R_FAILED(res)) {
                            break;
                        }
                        res = HTTPC_Azahar_RegisterURLReplacement(pattern.c_str(), replacement.c_str(), pattern.size(), replacement.size());
                        if (R_FAILED(res)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    } else {
        // Unregister OpenPak replacement URLs
        for (const auto& pattern : patterns) {
            res = HTTPC_Azahar_UnregisterURLReplacement(pattern.c_str(), pattern.size());
            if (R_FAILED(res)) {
                break;
            } else {
                // Result may have a non-failure info value indicating that the pattern
                // was not registered, clear it.
                res = 0;
            }
        }
    }

    if (res == MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47)) {
        // LLE HTTP was enabled, so custom command didn't exist
        // the HTTP patches will be used instead, so ignore error.
        res = 0;
    }

    httpcExit();
    return res;
}

void MainUI::migrateAccount(MainStruct *mainStruct) {
    Result rc = 0;
    u32 pretendo_account_index = 0;
    // Logs won't override any previous errors
    handleResult(ACT_GetAccountIndexOfFriendAccountId(&pretendo_account_index, 2), mainStruct, "Get account for migration");
    if (pretendo_account_index != 0) {
        bool is_commited = false;
        handleResult(ACT_GetAccountInfo(&is_commited, sizeof(bool), pretendo_account_index, INFO_TYPE_IS_COMMITTED), mainStruct, "Get account commit status");
        if (!is_commited) {
            handleResult(ACTA_CommitConsoleAccount(pretendo_account_index), mainStruct, "Commit account");
        }
    }
}

void MainUI::unlinkPNID(MainStruct *mainStruct) {
    if (R_FAILED(retPNID = ACTA_UnbindServerAccount(pnidAccountSlot, true))) {
        LOG_NIMBUS_ERROR(mainStruct, std::format("ACTA_UnbindServerAccount failed with error code {}!", retPNID).c_str());
	} else {
		LOG_NIMBUS_ERROR(mainStruct, "OpenPak account unlinked.");
	}
}

// Borrowed from https://github.com/azahar-emu/ArticBaseServer/blob/d93be050a4787ed602c603aa14bbaaee066dc1d9/app/sources/main.cpp#L79
void MainUI::launchPlugin(MainStruct *mainStruct) {
    Result rc = 0;
    PluginLoadParameters plgparam = { 0 };
    bool isPlgEnabled = false;

    // Ideally we wouldn't have to set the persistent flag and instead use the builtin
    // functionality, but the Old 3DS does a reboot before opening a Mode3 game,
    // erasing any non-persistent configurations we use since this as a generic launcher.
    //
    // Instead, we have to mark the parameters as persistent and then disable them
    // from the plugin itself
    plgparam.noFlash = false;
    plgparam.pluginMemoryStrategy = PLG_STRATEGY_SWAP;
    plgparam.persistent = 1;
    plgparam.lowTitleId = 0;
    strcpy(plgparam.path, NIMBUS_PLUGIN);

    // Use custom header on config as a way to differentiate between plugin load from launcher
    // and load by the user (by saving the plugin file as default or on a specific game)
    strcpy(reinterpret_cast<char*>(plgparam.config), NIMBUS_PLUGIN_MAGIC);
    plgparam.config[1] = NIMBUS_PLUGIN_VERSION;

    handleResult(plgLdrInit(), mainStruct, "Initialize plg:ldr");
    if (R_FAILED(rc)) {
        return;
    }

    u32 version;
    handleResult(PLGLDR__GetVersion(&version), mainStruct, "Get plg:ldr version");
    if (R_FAILED(rc)) {
        plgLdrExit();
        return;
    }

    if (version < SYSTEM_VERSION(1, 0, 2)) {
        LOG_NIMBUS_ERROR(mainStruct, "Unsupported plugin loader version: update Luma3DS");
        plgLdrExit();
        return;
    }

    // Save the previous plugin loader state
    handleResult(PLGLDR__IsPluginLoaderEnabled(&isPlgEnabled), mainStruct, "Get plugin loader state");
    if (R_FAILED(rc)) {
        plgLdrExit();
        return;
    }
    plgparam.config[2] = isPlgEnabled;

    handleResult(PLGLDR__SetPluginLoaderState(true), mainStruct, "Enable plugin loader");
    if (R_FAILED(rc)) {
        plgLdrExit();
        return;
    }

    handleResult(PLGLDR__SetPluginLoadParameters(&plgparam), mainStruct, "Set plugin load params");
    plgLdrExit();

    return;
}

void MainUI::openPrompt(MainStruct* mainStruct, const std::string& message, PromptStatus promptStatus)
{
    mainStruct->prompt.active = true;
    mainStruct->prompt.message = message;
    mainStruct->prompt.result = PromptResult::None;
    mainStruct->prompt.status = promptStatus;
}

void MainUI::updatePrompt(MainStruct* mainStruct, u32 kDown)
{
    if (!mainStruct->prompt.active)
        return;

    if (kDown & KEY_A) {
        mainStruct->prompt.result = PromptResult::Yes;
    }
    else if (kDown & KEY_B) {
        mainStruct->prompt.result = PromptResult::No;
    }
}

// Prompt hints follow the Switch Patcher footer: A / B with a caption.
static const char *promptHint(PromptStatus s) {
    return s == PromptStatus::Reboot ? "A: Reboot now    B: Later" : "A: Yes    B: Cancel";
}

void MainUI::drawPrompt(MainStruct* mainStruct)
{
    if (!mainStruct->prompt.active) return;

    const float screenW = 320.0f;
    const float screenH = 240.0f;

    C2D_DrawRectSolid(0, 0, 0.1f, screenW, screenH, C2D_Color32(0, 0, 0, 160));

    const float boxW = 280.0f, boxH = 110.0f;
    const float boxX = (screenW - boxW) / 2.0f;
    const float boxY = (screenH - boxH) / 2.0f;

    DrawBox(boxX, boxY, boxW, boxH, 0.2f, COLOR_RAISED, COLOR_WARN, 2);

    C2D_Text msgText;
    C2D_TextBufClear(textBuf);
    C2D_TextFontParse(&msgText, font, textBuf, mainStruct->prompt.message.c_str());
    C2D_TextOptimize(&msgText);
    C2D_DrawText(&msgText, C2D_WithColor | C2D_WordWrap, boxX + 12.0f, boxY + 12.0f, 0.4f,
                 0.55f, 0.55f, COLOR_INK, boxW - 24.0f);

    DrawText(screenW / 2.0f, boxY + boxH - 24.0f, 0.5f, COLOR_MUTED, C2D_AlignCenter, promptHint(mainStruct->prompt.status));
}

// Card rectangles on the bottom screen: Nintendo on the left, OpenPak on the right.
static constexpr float CARD_Y = 44, CARD_W = 128, CARD_H = 104;
static constexpr float CARD_X[2] = {24, 168};

static void drawCard(MainStruct *m, NascEnvironment env, const char *label) {
    int idx = env == NascEnvironment::NASC_ENV_Prod ? 0 : 1;
    bool selected = m->buttonSelected == env;
    bool active = m->currentAccount == env;
    DrawBox(CARD_X[idx], CARD_Y, CARD_W, CARD_H, 0.3f, selected ? COLOR_SELECT : COLOR_RAISED,
            selected ? COLOR_ACCENT : COLOR_LINE, selected ? 3 : 1);
    DrawText(CARD_X[idx] + CARD_W / 2, CARD_Y + 30, 0.8f, selected ? COLOR_STRONG : COLOR_INK, C2D_AlignCenter, label);
    if (active)
        DrawText(CARD_X[idx] + CARD_W / 2, CARD_Y + 66, 0.5f,
                 env == NascEnvironment::NASC_ENV_Test ? COLOR_OK : COLOR_WARN, C2D_AlignCenter, "Active");
}

static void drawPill(bool openpak) {
    const char *label = openpak ? "OPENPAK" : "NINTENDO";
    u32 fg = openpak ? COLOR_OK : COLOR_WARN;
    float w = 92, x = 400 - 12 - w, y = 40;
    DrawBox(x, y, w, 22, 0.3f, openpak ? COLOR_OK_BG : COLOR_WARN_BG, fg, 1);
    DrawText(x + w / 2, y + 3, 0.5f, fg, C2D_AlignCenter, label);
}

bool MainUI::drawUI(MainStruct *mainStruct, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, u32 kDown, u32 kHeld, touchPosition touch)
{
    // First run: the files inside the app become the managed copies. Nothing live changes unless
    // this console is on OpenPak, where a newer set is applied (and the reboot offered).
    if (!mainStruct->updateChecked) {
        mainStruct->updateChecked = true;
        char err[160] = "";
        int changed = Backup::stage(err, sizeof(err));
        bool onOpenPak = mainStruct->currentAccount == NascEnvironment::NASC_ENV_Test;
        if (changed > 0) migrateAccount(mainStruct);
        if (changed < 0 || mainStruct->errorString[0] != 0) {
            if (changed < 0) LOGF_NIMBUS_ERROR(mainStruct, "%s", err);
        } else if (onOpenPak && (changed > 0 || Backup::live() < Backup::managed())) {
            if (Backup::install(err, sizeof(err)))
                openPrompt(mainStruct, "OpenPak setup updated. Reboot to apply?", PromptStatus::Reboot);
            else
                LOGF_NIMBUS_ERROR(mainStruct, "%s", err);
        } else if (changed > 0) {
            mainStruct->status = "OpenPak files updated. They install when you select OpenPak.";
        }
        std::remove(NIMBUS_UPDATE_PATH "/update.txt");
    }

    // Exit to HBL / the HOME Menu depending on whether the app was launched from the CIA or the 3dsx.
    if (kDown & KEY_START) return true;

    updatePrompt(mainStruct, kDown);

    if (mainStruct->prompt.active) {
        if (mainStruct->prompt.result == PromptResult::Yes) {
            PromptStatus st = mainStruct->prompt.status;
            mainStruct->prompt.result = PromptResult::None;
            mainStruct->prompt.active = false;
            if (st == PromptStatus::Reboot) {
                mainStruct->needsReboot = true;
                return true;   // main() reboots the system after shutting the services down
            }
            if (st == PromptStatus::PNIDUnlink) unlinkPNID(mainStruct);
            return false;
        }
        if (mainStruct->prompt.result == PromptResult::No) {
            mainStruct->prompt.result = PromptResult::None;
            mainStruct->prompt.active = false;
            if (mainStruct->prompt.status == PromptStatus::Reboot) mainStruct->status.clear();
            return false;
        }
    }

    bool onOpenPak = mainStruct->currentAccount == NascEnvironment::NASC_ENV_Test;

    // Top screen: the status screen.
    C2D_SceneBegin(top_screen);
    DrawVersionString();
    C2D_DrawSprite(&mainStruct->top);
    DrawText(86, 14, 0.8f, COLOR_STRONG, C2D_AlignLeft, "OpenPak 3DS Patcher");
    DrawText(88, 40, 0.5f, COLOR_MUTED, C2D_AlignLeft, "Network selector");
    drawPill(onOpenPak);
    C2D_DrawRectSolid(12, 76, 0.3f, 376, 1, COLOR_LINE);

    float y = 90;
    if (mainStruct->errorString[0] != 0) {
        DrawText(12, y, 0.5f, COLOR_WARN, C2D_AlignLeft | C2D_WordWrap, std::format("Failed: {}", mainStruct->errorString));
        y += 56;
    } else if (!mainStruct->status.empty()) {
        DrawText(12, y, 0.5f, COLOR_INK, C2D_AlignLeft | C2D_WordWrap, mainStruct->status);
        y += 36;
    } else {
        DrawText(12, y, 0.5f, COLOR_MUTED, C2D_AlignLeft, "Changes apply at boot.");
        y += 22;
    }
    int managed = Backup::managed(), live = Backup::live();
    if (managed > 0)
        DrawText(12, y, 0.45f, COLOR_MUTED, C2D_AlignLeft, std::format("{} of {} OpenPak files installed", live, managed));
    else
        DrawText(12, y, 0.45f, COLOR_MUTED, C2D_AlignLeft, "No OpenPak files staged yet.");
    DrawScamWarning();

    // Bottom screen: Nintendo / OpenPak.
    C2D_SceneBegin(bottom_screen);
    DrawText(160, 14, 0.5f, COLOR_MUTED, C2D_AlignCenter, "Select a network");
    drawCard(mainStruct, NascEnvironment::NASC_ENV_Prod, "Nintendo");
    drawCard(mainStruct, NascEnvironment::NASC_ENV_Test, "OpenPak");
    DrawText(160, 168, 0.42f, COLOR_MUTED, C2D_AlignCenter, "Y: Launch plugin    X: Unlink OpenPak account");
    DrawText(160, 196, 0.5f, COLOR_INK, C2D_AlignCenter, "A: Select    B: Exit");
    drawPrompt(mainStruct);

    // Only allow user interaction when no prompt is open.
    if (mainStruct->prompt.active) return false;

    if (kDown & KEY_B) return true;

    if (kDown & KEY_TOUCH) {
        for (int idx = 0; idx < 2; idx++) {
            if (touch.px >= CARD_X[idx] && touch.px <= CARD_X[idx] + CARD_W && touch.py >= CARD_Y && touch.py <= CARD_Y + CARD_H) {
                mainStruct->buttonSelected = idx == 0 ? NascEnvironment::NASC_ENV_Prod : NascEnvironment::NASC_ENV_Test;
                mainStruct->buttonWasPressed = true;
            }
        }
    }
    else if (kDown & (KEY_LEFT | KEY_RIGHT)) {
        mainStruct->buttonSelected = mainStruct->buttonSelected == NascEnvironment::NASC_ENV_Test ? NascEnvironment::NASC_ENV_Prod : NascEnvironment::NASC_ENV_Test;
    }

    if (kDown & KEY_A) mainStruct->buttonWasPressed = true;

    if (kDown & KEY_X) {
        mainStruct->errorString[0] = 0;
        mainStruct->status.clear();
        retPNID = ACT_GetAccountIndexOfFriendAccountId(&pnidAccountSlot, 2);
        if (R_FAILED(retPNID)) {
            LOGF_NIMBUS_ERROR(mainStruct, "ACT_GetAccountIndexOfFriendAccountId failed with error code %08lx", retPNID);
        } else if (pnidAccountSlot == 0) {
            mainStruct->status = "No OpenPak account is linked on this console.";
        } else if (R_FAILED(retPNID = ACT_GetAccountInfo(pnid, sizeof(pnid), pnidAccountSlot, INFO_TYPE_ACCOUNT_ID))) {
            LOGF_NIMBUS_ERROR(mainStruct, "ACT_GetAccountInfo failed with error code %08lx", retPNID);
        } else if (pnid[0] != '\0') {
            openPrompt(mainStruct, std::format("Unlink the OpenPak account {} from this console? It can be linked again at any time.", pnid), PromptStatus::PNIDUnlink);
        } else {
            mainStruct->status = "No OpenPak account is linked on this console.";
        }
    }

    if (kDown & KEY_Y) {
        mainStruct->errorString[0] = 0;
        mainStruct->status.clear();
        launchPlugin(mainStruct);
        if (mainStruct->errorString[0] == 0) mainStruct->status = "Plugin ready. Launch a game from the HOME Menu.";
        mainStruct->buttonWasPressed = false;
        return false;
    }

    if (mainStruct->buttonWasPressed) {
        mainStruct->buttonWasPressed = false;
        mainStruct->errorString[0] = 0;
        mainStruct->status.clear();

        bool wantOpenPak = mainStruct->buttonSelected == NascEnvironment::NASC_ENV_Test;
        bool again = mainStruct->currentAccount == mainStruct->buttonSelected;
        const char *name = wantOpenPak ? "OpenPak" : "Nintendo";
        char err[160] = "";

        Result rc = 0;
        // Selecting the network already in use re-applies the files rather than refusing.
        if (!again) {
            u8 accountId = (u8)mainStruct->buttonSelected + 1;
            rc = unloadAccount(mainStruct);
            if (R_SUCCEEDED(rc)) {
                rc = switchAccounts(mainStruct, accountId);
                if (rc == ResultFPDLocalAccountNotExists && accountId == 2) {
                    memset(mainStruct->errorString, 0, 256);
                    rc = createAccount(mainStruct, accountId, NascEnvironment::NASC_ENV_Test);
                }
            }
            if (R_SUCCEEDED(rc)) {
                rc = handleAzahar(accountId);
                if (R_FAILED(rc)) LOGF_NIMBUS_ERROR(mainStruct, "Failed to apply Azahar configuration: %08lx", rc);
            }
            if (R_FAILED(rc)) {
                // The friends service is half switched: only a reboot makes it consistent.
                aptSetHomeAllowed(false);
                mainStruct->needsReboot = true;
                openPrompt(mainStruct, "The switch did not finish. Reboot to recover?", PromptStatus::Reboot);
                return false;
            }
            mainStruct->currentAccount = mainStruct->buttonSelected;
        }

        bool ok = wantOpenPak ? Backup::install(err, sizeof(err)) : Backup::remove(err, sizeof(err));
        if (!ok) {
            LOGF_NIMBUS_ERROR(mainStruct, "%s", err);
            return false;
        }
        openPrompt(mainStruct, again ? std::format("{} setup updated. Reboot to apply?", name)
                                     : std::format("{} selected. Reboot to apply?", name), PromptStatus::Reboot);
    }

    return false;
}
