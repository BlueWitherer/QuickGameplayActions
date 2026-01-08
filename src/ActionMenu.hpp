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

    bool init(PlayLayer* pl);

public:
    static ActionMenu* create(PlayLayer* pl);

    void setOpacity(GLubyte opacity);
    void setScale(float scale);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

    int64_t getOpacitySetting() const;
    float getScaleSetting() const;
};