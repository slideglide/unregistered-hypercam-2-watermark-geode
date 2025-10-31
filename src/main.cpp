#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(HyperCamWatermarkPLHook, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        auto sprite = CCSprite::create("unregistered.png"_spr);
        if (!sprite) return;
        auto safeArea = geode::utils::getSafeAreaRect();
        sprite->setAnchorPoint({0, 1});
        sprite->setScale(0.325f);
        sprite->setPosition({
            safeArea.origin.x,
            safeArea.origin.y + safeArea.size.height
        });
        m_uiLayer->addChild(sprite);
        sprite->setID("hypercam-watermark"_spr);
    }
};
