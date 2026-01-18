#include "ActionMenu.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// it's modding time >:3
static auto qga = Mod::get();

class ActionMenu::Impl final {
public:
    Ref<PlayLayer> playLayer = nullptr;

    Ref<CircleButtonSprite> sprite = nullptr;

    CCMenu* menu = nullptr;
    CCScale9Sprite* menuBg = nullptr;

    bool useRestart = qga->getSettingValue<bool>("restart");
    bool usePractice = qga->getSettingValue<bool>("practice");
    bool usePause = qga->getSettingValue<bool>("pause");
    bool useExit = qga->getSettingValue<bool>("exit");

    bool show = qga->getSavedValue<bool>("visible", true);
    bool toggleOnPress = qga->getSettingValue<bool>("toggle-press");

    float scale = static_cast<float>(qga->getSettingValue<double>("scale"));
    int64_t opacity = qga->getSettingValue<int64_t>("opacity");

    CCSize const screenSize = CCDirector::sharedDirector()->getWinSize();
    CCPoint dragStartPos = { 0, 0 };

    bool isAnimating = false;
    bool isDragging = false;
    bool isMoving = false;
};

ActionMenu::ActionMenu() {
    m_impl = std::make_unique<Impl>();
};

ActionMenu::~ActionMenu() {};

bool ActionMenu::init(PlayLayer* pl) {
    m_impl->playLayer = pl;

    if (!CCLayer::init()) return false;

    // get the saved position
    auto x = qga->getSavedValue<float>("menu-x", 75.f);
    auto y = qga->getSavedValue<float>("menu-y", m_impl->screenSize.height - 75.f);

    setID("menu"_spr);
    setPosition({ x, y });
    setAnchorPoint({ 0.5, 0.5 });
    setTouchMode(kCCTouchesOneByOne);
    setTouchPriority(-512);
    setTouchEnabled(true);
    setZOrder(9999);

    m_impl->sprite = CircleButtonSprite::createWithSpriteFrameName("edit_areaModeBtn04_001.png");
    m_impl->sprite->setScale(m_impl->scale * 0.875f);
    m_impl->sprite->setAnchorPoint({ 0.5, 0.5 });

    setContentSize(m_impl->sprite->getScaledContentSize());

    m_impl->sprite->setPosition(getScaledContentSize() / 2.f);

    setScale(m_impl->scale); // set initial scale
    setOpacity(m_impl->opacity); // set initial opacity

    addChild(m_impl->sprite, 9);

    auto layout = RowLayout::create()
        ->setGap(2.5f)
        ->setAutoGrowAxis(0.f);

    m_impl->menu = CCMenu::create();
    m_impl->menu->setID("actions-menu");
    m_impl->menu->setAnchorPoint({ 0, 1 });
    m_impl->menu->setContentSize({ 0.f, 25.f });
    m_impl->menu->setPosition({ (getScaledContentWidth() / 2.f) + 7.5f, (getScaledContentHeight() / 2.f) - 2.5f });
    m_impl->menu->setVisible(m_impl->show);
    m_impl->menu->setLayout(layout);

    auto const btns = std::to_array<ActionItem>({
        {
            m_impl->useRestart,
            "GJ_replayBtn_001.png",
            "restart-btn",
            [this](CCMenuItem*) {
                if (m_impl->playLayer) m_impl->playLayer->resetLevel();
            },
        },
        {
            m_impl->usePractice,
            "GJ_practiceBtn_001.png",
            "toggle-practice-btn",
            [this](CCMenuItem* sender) {
                if (m_impl->playLayer) {
                    m_impl->playLayer->togglePracticeMode(!m_impl->playLayer->m_isPracticeMode);

                    if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
                        auto btnSprite = CCSprite::createWithSpriteFrameName(m_impl->playLayer->m_isPracticeMode ? "GJ_normalBtn_001.png" : "GJ_practiceBtn_001.png");
                        btnSprite->setScale(m_impl->scale);
                        btnSprite->setOpacity(m_impl->opacity);

                        btn->setNormalImage(btnSprite);
                    };
                };
            },
        },
        {
            m_impl->usePause,
            "GJ_pauseBtn_001.png",
            "pause-btn",
            [this](CCMenuItem*) {
                if (m_impl->playLayer) m_impl->playLayer->pauseGame(false);
            },
            1.62f,
        },
        {
            m_impl->useExit,
            "GJ_menuBtn_001.png",
            "exit-btn",
            [this](CCMenuItem*) {
                if (m_impl->playLayer) {
                    m_impl->playLayer->onQuit();

                    // @geode-ignore(unknown-resource)
                    if (auto fmod = FMODAudioEngine::sharedEngine()) fmod->playEffectAsync("quitSound_01.ogg");
                };
            },
        },
        {
            m_impl->useRestart && pl->m_isPlatformer,
            "GJ_replayFullBtn_001.png",
            "full-restart-btn",
            [this](CCMenuItem*) {
                if (m_impl->playLayer) m_impl->playLayer->resetLevelFromStart();
            },
        },
                                                });

    for (auto const& b : btns) {
        if (b.enabled) {
            auto btnSprite = CCSprite::createWithSpriteFrameName(b.sprite);
            btnSprite->setScale(m_impl->scale * b.scale);
            btnSprite->setOpacity(m_impl->opacity);

            auto btn = CCMenuItemExt::createSpriteExtra(
                btnSprite,
                std::move(b.callback)
            );
            btn->setID(b.id);

            m_impl->menu->addChild(btn);
        };
    };

    addChild(m_impl->menu, 1);
    m_impl->menu->updateLayout();

    m_impl->menuBg = CCScale9Sprite::create("square02_001.png");
    m_impl->menuBg->setScale(0.25f);
    m_impl->menuBg->setOpacity(m_impl->opacity / 2);
    m_impl->menuBg->setAnchorPoint(m_impl->menu->getAnchorPoint());
    m_impl->menuBg->setContentSize({ (m_impl->menu->getScaledContentWidth() + 15.f) * 4.f, (m_impl->menu->getScaledContentHeight() * 4.f) + 5.f });
    m_impl->menuBg->setPosition(m_impl->sprite->getPosition());
    m_impl->menuBg->setVisible(m_impl->show);

    addChild(m_impl->menuBg, 0);

    return true;
};

void ActionMenu::setOpacity(GLubyte opacity) {
    m_impl->opacity = opacity;
    if (m_impl->sprite) m_impl->sprite->setOpacity(opacity);
};

void ActionMenu::setScale(float scale) {
    m_impl->scale = scale;

    if (!m_impl->isDragging && !m_impl->isAnimating) {
        if (m_impl->sprite) {
            m_impl->sprite->setScale(scale);
            setContentSize(m_impl->sprite->getScaledContentSize());
        };

        if (m_impl->menu) m_impl->menu->setScale(scale);
        if (m_impl->menuBg) m_impl->menuBg->setScale(scale);
    };
};

void ActionMenu::onScaleEnd() {
    m_impl->isAnimating = false;
};

bool ActionMenu::ccTouchBegan(CCTouch* touch, CCEvent* ev) {
    if (m_impl->sprite) {
        auto const touchLocation = convertToNodeSpace(touch->getLocation());
        auto const box = m_impl->sprite->boundingBox();

        if (box.containsPoint(touchLocation)) {
            m_impl->isDragging = true;

            m_impl->dragStartPos = ccpSub(getPosition(), touch->getLocation());

            m_impl->sprite->stopAllActions();
            m_impl->isAnimating = true;
            m_impl->sprite->runAction(CCSequence::createWithTwoActions(
                CCSpawn::createWithTwoActions(
                    CCEaseExponentialOut::create(CCScaleTo::create(0.25f, m_impl->scale * 0.875f)),
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
    if (m_impl->isDragging) {
        auto const touchLocation = touch->getLocation();
        auto const newLocation = ccpAdd(touchLocation, m_impl->dragStartPos);

        auto clampX = std::max(0.f, std::min(newLocation.x, m_impl->screenSize.width - getScaledContentWidth()));
        auto clampY = std::max(0.f, std::min(newLocation.y, m_impl->screenSize.height - getScaledContentHeight()));

        setPosition(ccp(clampX, clampY));

        m_impl->isMoving = true;
    };
};

void ActionMenu::ccTouchEnded(CCTouch* touch, CCEvent* ev) {
    if (!m_impl->isMoving && m_impl->toggleOnPress) {
        if (m_impl->menu) {
            m_impl->menu->setVisible(!m_impl->show);
            if (m_impl->menuBg) m_impl->menuBg->setVisible(!m_impl->show);

            m_impl->show = !qga->setSavedValue("visible", !m_impl->show);
        };
    };

    // reset state
    m_impl->isDragging = false;
    m_impl->isMoving = false;

    // store position
    qga->setSavedValue<float>("menu-x", getPositionX());
    qga->setSavedValue<float>("menu-y", getPositionY());

    m_impl->isAnimating = true;

    if (m_impl->sprite) {
        // reset scale
        m_impl->sprite->stopAllActions();
        m_impl->sprite->runAction(CCSequence::create(
            CCSpawn::createWithTwoActions(
                CCFadeTo::create(0.125f, 255),
                CCEaseElasticOut::create(CCScaleTo::create(0.875f, m_impl->scale))
            ),
            CCCallFunc::create(this, callfunc_selector(ActionMenu::onScaleEnd)),
            CCDelayTime::create(1.f),
            CCFadeTo::create(0.5f, m_impl->opacity),
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