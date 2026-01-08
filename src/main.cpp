#include "ActionMenu.hpp"

#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(ActionsPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        auto menu = ActionMenu::create(this);
        m_uiLayer->addChild(menu, 999);
    };
};