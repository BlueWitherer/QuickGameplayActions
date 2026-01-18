#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ActionMenu : public CCLayer {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    struct ActionItem {
        bool enabled;
        const char* sprite;
        const char* id;
        std::function<void(CCMenuItem*)> callback;
        float scale = 1.f;
    };

protected:
    ActionMenu();
    virtual ~ActionMenu();

    void onScaleEnd();

    void setOpacity(GLubyte opacity);
    void setScale(float scale) override;

    bool init(PlayLayer* pl);

public:
    static ActionMenu* create(PlayLayer* pl);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
};