#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

static bool s_inEverywhere = true;
static bool s_inGameplay   = true;
static bool s_inEditor     = true;

static void setWatermark(CCNode* parent, bool show) {
    if (!parent) return;

    auto existing = parent->getChildByID("hypercam-watermark"_spr);

    if (show) {
        if (existing) return;

        auto sprite = CCSprite::create("unregistered.png"_spr);
        if (!sprite) return;

        auto safe = geode::utils::getSafeAreaRect();
        sprite->setAnchorPoint({0, 1});
        sprite->setScale(0.325f);
        sprite->setPosition({ safe.origin.x, safe.origin.y + safe.size.height });

        sprite->setID("hypercam-watermark"_spr);
        parent->addChild(sprite, 9999);
    } else {
        if (existing) {
            existing->removeFromParent();
        }
    }
}

static void syncWatermarks(PlayLayer* pl = nullptr, EditorUI* editor = nullptr, bool useGlobal = true) {
    if (useGlobal) {
        if (!pl) pl = GameManager::sharedState()->getPlayLayer();
        if (!editor && LevelEditorLayer::get()) editor = LevelEditorLayer::get()->m_editorUI;
    }

    bool anyVisible = false;
    if (s_inEverywhere) {
        setWatermark(geode::OverlayManager::get(), true);
        anyVisible = true;
        
        if (pl) setWatermark(pl->m_uiLayer, false);
        if (editor) setWatermark(editor, false);
    } else {
        setWatermark(geode::OverlayManager::get(), false);
        
        if (s_inGameplay && pl) {
            setWatermark(pl->m_uiLayer, true);
            anyVisible = true;
        } else if (pl) {
            setWatermark(pl->m_uiLayer, false);
        }

        if (s_inEditor && editor) {
            setWatermark(editor, true);
            anyVisible = true;
        } else if (editor) {
            setWatermark(editor, false);
        }
    }
}

static ListenerHandle* s_everywhereL = nullptr;
static ListenerHandle* s_gameplayL   = nullptr;
static ListenerHandle* s_editorL     = nullptr;

$on_mod(Loaded) {
    auto mod = Mod::get();
    
    s_inEverywhere = mod->getSettingValue<bool>("in-everywhere");
    s_inGameplay   = mod->getSettingValue<bool>("in-gameplay");
    s_inEditor     = mod->getSettingValue<bool>("in-editor");
    
    s_everywhereL = geode::listenForSettingChanges<bool>(
        "in-everywhere",
        [](bool v) { s_inEverywhere = v; log::info("in-everywhere = {}", v); syncWatermarks(); }
    );
    
    s_gameplayL = geode::listenForSettingChanges<bool>(
        "in-gameplay",
        [](bool v) { s_inGameplay = v; log::info("in-gameplay = {}", v); syncWatermarks(); }
    );
    
    s_editorL = geode::listenForSettingChanges<bool>(
        "in-editor",
        [](bool v) { s_inEditor = v; log::info("in-editor = {}", v); syncWatermarks(); }
    );
    
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