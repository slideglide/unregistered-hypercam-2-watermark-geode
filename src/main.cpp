#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(HyperCamWatermarkPLHook, PlayLayer) {
  void setupHasCompleted() { 
    auto sprite = CCSprite::create("unregistered.png"_spr);
    if (sprite) {
      auto winSize = CCDirector::sharedDirector()->getWinSize();
      sprite->setAnchorPoint(CCPoint(0, 1));
      sprite->setScale(0.325f);
      sprite->setPosition(CCPoint(0, winSize.height));
      PlayLayer::setupHasCompleted();
      this->addChild(sprite);
      sprite->setID("hypercam-watermark"_spr);
    }
  }
};
