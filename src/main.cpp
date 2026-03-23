#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

struct Settings {
    bool enabled      = true;
    bool inEverywhere = true;
    bool inGameplay   = true;
    bool inEditor     = true;

    static Settings load() {
        auto mod = Mod::get();
        return {
            mod->getSettingValue<bool>("enabled"),
            mod->getSettingValue<bool>("in-everywhere"),
            mod->getSettingValue<bool>("in-gameplay"),
            mod->getSettingValue<bool>("in-editor"),
        };
    }
} static s_settings;


static void showWatermark(CCNode* parent) {
    if (!parent || parent->getChildByID("hypercam-watermark"_spr)) return;

    auto sprite = CCSprite::create("unregistered.png"_spr);
    if (!sprite) return;

    auto safe = geode::utils::getSafeAreaRect();
    sprite->setAnchorPoint({0, 1});
    sprite->setScale(0.325f);
    sprite->setPosition({ safe.origin.x, safe.origin.y + safe.size.height });
    sprite->setID("hypercam-watermark"_spr);
    parent->addChild(sprite, 9999);
}

static void hideWatermark(CCNode* parent) {
    if (!parent) return;
    if (auto existing = parent->getChildByID("hypercam-watermark"_spr)) existing->removeFromParent();
}

static void syncWatermarks(PlayLayer* pl = nullptr, EditorUI* editor = nullptr, bool useGlobal = true) {
    if (useGlobal) {
        if (!pl) pl = PlayLayer::get();
        if (!editor && LevelEditorLayer::get()) editor = LevelEditorLayer::get()->m_editorUI;
    }

    auto& cfg = s_settings;
    auto* overlay = geode::OverlayManager::get();
    CCNode* uiLayer = pl ? pl->m_uiLayer : nullptr;

    bool wantOverlay  = cfg.enabled && cfg.inEverywhere;
    bool wantPlayLayer = cfg.enabled && !cfg.inEverywhere && cfg.inGameplay && pl;
    bool wantEditor   = cfg.enabled && !cfg.inEverywhere && cfg.inEditor && editor;

    wantOverlay  ? showWatermark(overlay)  : hideWatermark(overlay);
    wantPlayLayer ? showWatermark(uiLayer) : hideWatermark(uiLayer);
    wantEditor   ? showWatermark(editor)   : hideWatermark(editor);
}

$on_mod(Loaded) {
    s_settings = Settings::load();

    geode::listenForSettingChanges<bool>("enabled",      [](bool v) { s_settings.enabled      = v; syncWatermarks(); });
    geode::listenForSettingChanges<bool>("in-everywhere",[](bool v) { s_settings.inEverywhere = v; syncWatermarks(); });
    geode::listenForSettingChanges<bool>("in-gameplay",  [](bool v) { s_settings.inGameplay   = v; syncWatermarks(); });
    geode::listenForSettingChanges<bool>("in-editor",    [](bool v) { s_settings.inEditor     = v; syncWatermarks(); });

    syncWatermarks();
}

class $modify(HyperCamWatermarkPLHook, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        syncWatermarks(this, nullptr);
    }

    void onQuit() {
        syncWatermarks(nullptr, nullptr, false);
        PlayLayer::onQuit();
    }
};

class $modify(HyperCamWatermarkLELHook, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;
        geode::queueInMainThread([this]() {
            syncWatermarks(nullptr, m_editorUI);
        });
        return true;
    }

    void onExit() {
        syncWatermarks(nullptr, nullptr, false);
        LevelEditorLayer::onExit();
    }
};