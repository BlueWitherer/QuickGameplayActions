#include "ActionMenu.hpp"

#include <Geode/modify/PlayLayer.hpp>

class $modify(ActionsPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        if (auto menu = ActionMenu::create(this)) m_uiLayer->addChild(menu, 99);
    };
};