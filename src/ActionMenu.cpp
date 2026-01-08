#include "ActionMenu.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ActionMenu::Impl final {
public:
    Ref<PlayLayer> m_playLayer = nullptr;

    Ref<CircleButtonSprite> m_sprite = nullptr;

    float m_scale = static_cast<float>(Mod::get()->getSettingValue<double>("scale"));
    int64_t m_opacity = Mod::get()->getSettingValue<int64_t>("opacity");

    CCSize const m_screenSize = CCDirector::sharedDirector()->getWinSize();
    CCPoint m_dragStartPos = { 0, 0 };

    bool m_isAnimating = false;
    bool m_isDragging = false;
    bool m_isMoving = false;
};

ActionMenu::ActionMenu() {
    m_impl = std::make_unique<Impl>();
};

ActionMenu::~ActionMenu() {};

bool ActionMenu::init(PlayLayer* pl) {
    m_impl->m_playLayer = pl;

    if (!CCLayer::init()) return false;

    // get the saved position
    auto x = Mod::get()->getSavedValue<float>("menu-x", 75.f);
    auto y = Mod::get()->getSavedValue<float>("menu-y", m_impl->m_screenSize.height - 75.f);

    setID("menu"_spr);
    setPosition({ x, y });
    setAnchorPoint({ 0.5, 0.5 });
    setTouchMode(kCCTouchesOneByOne);
    setTouchPriority(-512);
    setZOrder(9999);

    m_impl->m_sprite = CircleButtonSprite::createWithSpriteFrameName("edit_areaModeBtn04_001.png");
    m_impl->m_sprite->setScale(m_impl->m_scale);
    m_impl->m_sprite->setAnchorPoint({ 0.5, 0.5 });

    setContentSize(m_impl->m_sprite->getScaledContentSize());

    m_impl->m_sprite->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });

    setScale(m_impl->m_scale); // set initial scale
    setOpacity(m_impl->m_opacity); // set initial opacity

    addChild(m_impl->m_sprite, 9);

    return true;
};

void ActionMenu::setOpacity(GLubyte opacity) {
    m_impl->m_opacity = opacity;
    if (m_impl->m_sprite) m_impl->m_sprite->setOpacity(isVisible() ? opacity : 0);
};

void ActionMenu::setScale(float scale) {
    m_impl->m_scale = scale;

    if (!m_impl->m_isDragging && !m_impl->m_isAnimating) {
        if (m_impl->m_sprite) {
            m_impl->m_sprite->setScale(scale);
            setContentSize(m_impl->m_sprite->getScaledContentSize());
        };
    };
};

void ActionMenu::onScaleEnd() {
    m_impl->m_isAnimating = false;
};

int64_t ActionMenu::getOpacitySetting() const {
    return m_impl->m_opacity;
};

float ActionMenu::getScaleSetting() const {
    return m_impl->m_scale;
};

bool ActionMenu::ccTouchBegan(CCTouch* touch, CCEvent* ev) {
    if (m_impl->m_sprite) {
        auto const touchLocation = convertToNodeSpace(touch->getLocation());
        auto const box = m_impl->m_sprite->boundingBox();

        if (box.containsPoint(touchLocation)) {
            m_impl->m_isDragging = true;

            m_impl->m_dragStartPos = ccpSub(getPosition(), touch->getLocation());

            m_impl->m_sprite->stopAllActions();
            m_impl->m_isAnimating = true;
            m_impl->m_sprite->runAction(CCSequence::createWithTwoActions(
                CCSpawn::createWithTwoActions(
                    CCEaseExponentialOut::create(CCScaleTo::create(0.25f, m_impl->m_scale * 0.875f)),
                    CCFadeTo::create(0.25f, 255)
                ),
                CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd))
            ));

            return true; // swallow touch
        };
    };

    return false;
};

void ActionMenu::ccTouchMoved(CCTouch* touch, CCEvent* ev) {
    if (m_impl->m_isDragging) {
        CCPoint const touchLocation = touch->getLocation();
        CCPoint const newLocation = ccpAdd(touchLocation, m_impl->m_dragStartPos);

        setPosition(newLocation);

        m_impl->m_isMoving = true;
    };
};

void ActionMenu::ccTouchEnded(CCTouch* touch, CCEvent* ev) {
    // reset state
    m_impl->m_isDragging = false;
    m_impl->m_isMoving = false;

    // store position
    Mod::get()->setSavedValue<float>("menu-x", getPosition().x);
    Mod::get()->setSavedValue<float>("menu-y", getPosition().y);

    m_impl->m_isAnimating = true;

    if (m_impl->m_sprite) {
        // reset scale
        m_impl->m_sprite->stopAllActions();
        m_impl->m_sprite->runAction(CCSequence::create(
            CCSpawn::createWithTwoActions(
                CCFadeTo::create(0.125f, 255),
                CCEaseElasticOut::create(CCScaleTo::create(0.875f, m_impl->m_scale))
            ),
            CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd)),
            CCDelayTime::create(1.f),
            CCFadeTo::create(0.5f, m_impl->m_opacity),
            nullptr
        ));
    };
};

ActionMenu* ActionMenu::create(PlayLayer* pl) {
    auto ret = new ActionMenu();
    if (ret->init(pl)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};