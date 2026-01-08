#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ActionMenu : public CCLayer {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    ActionMenu();
    virtual ~ActionMenu();

    void onScaleEnd();

    void onRestart(CCObject*);
    void onPractice(CCObject*);
    void onExit(CCObject*);

    void setOpacity(GLubyte opacity);
    void setScale(float scale);

    bool init(PlayLayer* pl);

public:
    static ActionMenu* create(PlayLayer* pl);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
};