#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(ActionsPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        // just a concept for now
        log::info("{} hooked setupHasCompleted!", GEODE_MOD_ID);
    };
};