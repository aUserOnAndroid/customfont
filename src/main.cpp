#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace geode::prelude;
using namespace cocos2d;
using namespace cocos2d::extension;

namespace {
    bool g_ignoreDamage = false;
    bool g_practiceMode = false;
    bool g_autoPlay = false;
    bool g_autoCube = true;
    bool g_autoWave = true;
    bool g_showHitboxes = false;
    bool g_platformerAssist = false;
    bool g_autoHoldingJump = false;
    bool g_autoHoldingRight = false;

    enum class HubTab {
        Player,
        Assist,
        Visual
    };

    constexpr auto kJumpButton = static_cast<PlayerButton>(1);
    constexpr auto kRightButton = static_cast<PlayerButton>(3);

    void showToast(char const* text) {
        if (auto notification = Notification::create(text)) {
            notification->show();
        }
    }

    bool nodeContainsWorldPoint(CCNode* node, CCPoint worldPoint) {
        if (!node || !node->isVisible()) {
            return false;
        }

        auto parent = node->getParent();
        auto localPoint = parent ? parent->convertToNodeSpace(worldPoint) : worldPoint;
        return node->boundingBox().containsPoint(localPoint);
    }

    CCLabelBMFont* makeLabel(char const* text, char const* font, float scale, CCPoint position, CCNode* parent, int zOrder = 1) {
        auto label = CCLabelBMFont::create(text, font);
        label->setScale(scale);
        label->setPosition(position);
        parent->addChild(label, zOrder);
        return label;
    }

    void releaseAutoButtons(PlayerObject* player = nullptr) {
        if (!player) {
            if (auto playLayer = PlayLayer::get()) {
                player = playLayer->m_player1;
            }
        }

        if (!player) {
            g_autoHoldingJump = false;
            g_autoHoldingRight = false;
            return;
        }

        if (g_autoHoldingJump) {
            player->releaseButton(kJumpButton);
            g_autoHoldingJump = false;
        }
        if (g_autoHoldingRight) {
            player->releaseButton(kRightButton);
            g_autoHoldingRight = false;
        }
    }

    void setJumpHeld(PlayerObject* player, bool held) {
        if (!player || g_autoHoldingJump == held) {
            return;
        }

        if (held) {
            player->pushButton(kJumpButton);
        } else {
            player->releaseButton(kJumpButton);
        }
        g_autoHoldingJump = held;
    }

    void setRightHeld(PlayerObject* player, bool held) {
        if (!player || g_autoHoldingRight == held) {
            return;
        }

        if (held) {
            player->pushButton(kRightButton);
        } else {
            player->releaseButton(kRightButton);
        }
        g_autoHoldingRight = held;
    }

    CCRect expandedRect(GameObject* object, float paddingX, float paddingY) {
        auto rect = object->getObjectRect();
        rect.origin.x -= paddingX;
        rect.origin.y -= paddingY;
        rect.size.width += paddingX * 2.f;
        rect.size.height += paddingY * 2.f;
        return rect;
    }

    bool looksLikeGameplayCollision(GameObject* object) {
        if (!object || !object->isVisible() || object->m_isDisabled || object->m_isGroupDisabled || object->m_isTrigger) {
            return false;
        }
        if (object->m_isDecoration || object->m_isDecoration2 || object->m_isPassable || object->m_isNoTouch || object->m_isInvisible) {
            return false;
        }
        return object->m_objectID > 0;
    }

    std::vector<GameObject*> collectNearbyCollisionObjects(PlayLayer* layer, PlayerObject* player, float lookAhead) {
        std::vector<GameObject*> objects;
        if (!layer || !player || !layer->m_objectLayer) {
            return objects;
        }

        auto playerPos = player->getPosition();
        for (auto node : layer->m_objectLayer->getChildrenExt()) {
            auto object = typeinfo_cast<GameObject*>(node);
            if (!looksLikeGameplayCollision(object)) {
                continue;
            }

            auto objectPos = object->getPosition();
            auto dx = objectPos.x - playerPos.x;
            if (dx < -45.f || dx > lookAhead) {
                continue;
            }
            if (std::abs(objectPos.y - playerPos.y) > 180.f) {
                continue;
            }
            objects.push_back(object);
        }
        return objects;
    }

    bool cubeShouldJump(PlayLayer* layer, PlayerObject* player) {
        auto playerPos = player->getPosition();
        auto playerRect = player->getObjectRect();
        playerRect.origin.x -= 4.f;
        playerRect.size.width += 8.f;

        auto lookAhead = std::clamp(static_cast<float>(std::abs(player->getCurrentXVelocity()) * 0.45f), 72.f, 145.f);
        for (auto object : collectNearbyCollisionObjects(layer, player, lookAhead)) {
            auto rect = expandedRect(object, 12.f, 8.f);
            auto isAhead = rect.getMinX() > playerPos.x - 12.f && rect.getMinX() < playerPos.x + lookAhead;
            auto isInLane = rect.getMaxY() > playerRect.getMinY() - 10.f && rect.getMinY() < playerRect.getMaxY() + 28.f;
            if (isAhead && isInLane) {
                return true;
            }
        }
        return false;
    }

    bool waveShouldHold(PlayLayer* layer, PlayerObject* player) {
        auto playerPos = player->getPosition();
        auto lookAhead = std::clamp(static_cast<float>(std::abs(player->getCurrentXVelocity()) * 0.55f), 95.f, 175.f);
        auto targetY = playerPos.y;
        auto foundThreat = false;

        for (auto object : collectNearbyCollisionObjects(layer, player, lookAhead)) {
            auto rect = expandedRect(object, 16.f, 16.f);
            if (rect.getMinX() < playerPos.x - 10.f || rect.getMinX() > playerPos.x + lookAhead) {
                continue;
            }

            foundThreat = true;
            auto centerY = rect.getMidY();
            if (centerY < playerPos.y + 5.f) {
                targetY = std::max(targetY, rect.getMaxY() + 58.f);
            } else {
                targetY = std::min(targetY, rect.getMinY() - 58.f);
            }
        }

        if (!foundThreat) {
            auto winHeight = CCDirector::sharedDirector()->getWinSize().height;
            auto softTop = layer->m_objectLayer->convertToNodeSpace({ 0.f, winHeight - 44.f }).y;
            auto softBottom = layer->m_objectLayer->convertToNodeSpace({ 0.f, 52.f }).y;
            if (playerPos.y < softBottom) {
                return true;
            }
            if (playerPos.y > softTop) {
                return false;
            }
            return player->getYVelocity() < -2.0;
        }

        return targetY > playerPos.y;
    }

    void runAutoPlay(PlayLayer* layer) {
        auto player = layer ? layer->m_player1 : nullptr;
        if (!layer || !player || !layer->isGameplayActive() || !g_autoPlay) {
            releaseAutoButtons(player);
            return;
        }

        auto isCube = player->isInNormalMode();
        auto isWave = !isCube && player->isFlying();

        if (g_platformerAssist) {
            setRightHeld(player, true);
        } else {
            setRightHeld(player, false);
        }

        if (isWave && g_autoWave) {
            setJumpHeld(player, waveShouldHold(layer, player));
            return;
        }

        if (isCube && g_autoCube) {
            auto wantsJump = cubeShouldJump(layer, player);
            auto groundedOrSafe = player->m_lastGroundObject || player->getYVelocity() < -0.5;
            setJumpHeld(player, wantsJump && groundedOrSafe);
            return;
        }

        setJumpHeld(player, false);
    }
}

class ModernMenu : public CCLayer {
protected:
    CCNode* m_floatButton = nullptr;
    CCLayerColor* m_panel = nullptr;
    CCScale9Sprite* m_background = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCLabelBMFont* m_damageLabel = nullptr;
    CCLabelBMFont* m_practiceLabel = nullptr;
    CCLabelBMFont* m_autoPlayLabel = nullptr;
    CCLabelBMFont* m_cubeLabel = nullptr;
    CCLabelBMFont* m_waveLabel = nullptr;
    CCLabelBMFont* m_platformerLabel = nullptr;
    CCLabelBMFont* m_hitboxLabel = nullptr;
    CCLabelBMFont* m_tabTitleLabel = nullptr;
    CCNode* m_playerPage = nullptr;
    CCNode* m_assistPage = nullptr;
    CCNode* m_visualPage = nullptr;
    HubTab m_tab = HubTab::Assist;
    bool m_open = false;
    bool m_dragging = false;
    bool m_movedTouch = false;
    CCPoint m_dragOffset = CCPointZero;
    CCPoint m_touchStart = CCPointZero;

public:
    static ModernMenu* create() {
        auto ret = new ModernMenu();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        this->setID("emir-hub-layer"_spr);

        m_floatButton = ButtonSprite::create("EH", 44, true, "goldFont.fnt", "GJ_button_04.png", 28.f, 0.65f);
        m_floatButton->setPosition({ 62.f, winSize.height * 0.58f });
        m_floatButton->setID("open-emir-hub"_spr);
        this->addChild(m_floatButton, 20);

        this->createPanel(winSize);
        this->switchTab(HubTab::Assist);
        this->refreshStateLabels();
        this->schedule(schedule_selector(ModernMenu::tickStatus), 0.20f);
        return true;
    }

    void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -1, true);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
        auto worldPoint = touch->getLocation();
        m_touchStart = worldPoint;
        m_movedTouch = false;

        if (nodeContainsWorldPoint(m_floatButton, worldPoint)) {
            m_dragging = true;
            m_dragOffset = m_floatButton->getPosition() - this->convertToNodeSpace(worldPoint);
            return true;
        }

        if (m_open && nodeContainsWorldPoint(m_panel, worldPoint)) {
            return true;
        }

        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent*) override {
        if (!m_dragging) {
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto worldPoint = touch->getLocation();
        if (worldPoint.getDistance(m_touchStart) > 6.f) {
            m_movedTouch = true;
        }

        auto nextPosition = this->convertToNodeSpace(worldPoint) + m_dragOffset;
        nextPosition.x = std::clamp(nextPosition.x, 28.f, winSize.width - 28.f);
        nextPosition.y = std::clamp(nextPosition.y, 28.f, winSize.height - 28.f);
        m_floatButton->setPosition(nextPosition);
    }

    void ccTouchEnded(CCTouch*, CCEvent*) override {
        if (m_dragging && !m_movedTouch) {
            this->togglePanel();
        }
        m_dragging = false;
        m_movedTouch = false;
    }

    void ccTouchCancelled(CCTouch*, CCEvent*) override {
        m_dragging = false;
        m_movedTouch = false;
    }

    void keyBackClicked() override {
        if (m_open) {
            this->closePanel();
            return;
        }
        CCLayer::keyBackClicked();
    }

private:
    void createPanel(CCSize winSize) {
        m_panel = CCLayerColor::create({ 0, 0, 0, 0 }, 306.f, 232.f);
        m_panel->setPosition({ winSize.width / 2.f - 153.f, winSize.height / 2.f - 116.f });
        m_panel->setScale(0.86f);
        m_panel->setOpacity(0);
        m_panel->setVisible(false);
        m_panel->setID("emir-hub-panel"_spr);
        this->addChild(m_panel, 15);

        m_background = CCScale9Sprite::create("GJ_square01.png");
        m_background->setContentSize({ 306.f, 232.f });
        m_background->setPosition({ 153.f, 116.f });
        m_background->setColor({ 20, 24, 38 });
        m_background->setOpacity(242);
        m_panel->addChild(m_background);

        auto glow = CCLayerColor::create({ 57, 172, 255, 48 }, 294.f, 34.f);
        glow->setPosition({ 6.f, 190.f });
        m_panel->addChild(glow, 1);

        makeLabel("Emir Hub", "goldFont.fnt", 0.74f, { 153.f, 208.f }, m_panel, 2);
        m_tabTitleLabel = makeLabel("Assist Suite", "bigFont.fnt", 0.32f, { 153.f, 187.f }, m_panel, 2);
        m_tabTitleLabel->setColor({ 165, 210, 255 });

        this->addTabButton("Player", { 64.f, 166.f }, menu_selector(ModernMenu::onPlayerTab), "player-tab"_spr);
        this->addTabButton("Assist", { 153.f, 166.f }, menu_selector(ModernMenu::onAssistTab), "assist-tab"_spr);
        this->addTabButton("Visual", { 242.f, 166.f }, menu_selector(ModernMenu::onVisualTab), "visual-tab"_spr);

        m_playerPage = CCNode::create();
        m_assistPage = CCNode::create();
        m_visualPage = CCNode::create();
        m_panel->addChild(m_playerPage, 3);
        m_panel->addChild(m_assistPage, 3);
        m_panel->addChild(m_visualPage, 3);

        m_damageLabel = this->addActionButton(m_playerPage, "No Death", { 83.f, 116.f }, menu_selector(ModernMenu::onToggleDamage), "damage-toggle"_spr);
        m_practiceLabel = this->addActionButton(m_playerPage, "Practice", { 223.f, 116.f }, menu_selector(ModernMenu::onTogglePractice), "practice-toggle"_spr);
        this->addActionButton(m_playerPage, "Restart", { 83.f, 68.f }, menu_selector(ModernMenu::onRestart), "restart-level"_spr);
        this->addActionButton(m_playerPage, "About", { 223.f, 68.f }, menu_selector(ModernMenu::onAbout), "about-menu"_spr);

        m_autoPlayLabel = this->addActionButton(m_assistPage, "Auto Play", { 83.f, 116.f }, menu_selector(ModernMenu::onToggleAutoPlay), "autoplay-toggle"_spr);
        m_cubeLabel = this->addActionButton(m_assistPage, "Cube AI", { 223.f, 116.f }, menu_selector(ModernMenu::onToggleCube), "cube-toggle"_spr);
        m_waveLabel = this->addActionButton(m_assistPage, "Wave AI", { 83.f, 68.f }, menu_selector(ModernMenu::onToggleWave), "wave-toggle"_spr);
        m_platformerLabel = this->addActionButton(m_assistPage, "Platform", { 223.f, 68.f }, menu_selector(ModernMenu::onTogglePlatformer), "platform-toggle"_spr);

        m_hitboxLabel = this->addActionButton(m_visualPage, "Hitboxes", { 83.f, 116.f }, menu_selector(ModernMenu::onToggleHitboxes), "hitbox-toggle"_spr);
        this->addActionButton(m_visualPage, "Progress", { 223.f, 116.f }, menu_selector(ModernMenu::onProgressbar), "progress-toggle"_spr);
        this->addActionButton(m_visualPage, "Info", { 83.f, 68.f }, menu_selector(ModernMenu::onInfoLabel), "info-toggle"_spr);
        this->addActionButton(m_visualPage, "Debug", { 223.f, 68.f }, menu_selector(ModernMenu::onDebugPulse), "debug-pulse"_spr);

        m_statusLabel = makeLabel("Ready", "chatFont.fnt", 0.52f, { 153.f, 18.f }, m_panel, 2);
        m_statusLabel->setColor({ 170, 240, 170 });
    }

    void addTabButton(char const* text, CCPoint position, SEL_MenuHandler callback, char const* nodeID) {
        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        m_panel->addChild(menu, 4);

        auto sprite = ButtonSprite::create(text, 78, true, "bigFont.fnt", "GJ_button_05.png", 22.f, 0.38f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        button->setPosition(position);
        button->setID(nodeID);
        menu->addChild(button);
    }

    CCLabelBMFont* addActionButton(CCNode* page, char const* text, CCPoint position, SEL_MenuHandler callback, char const* nodeID) {
        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        page->addChild(menu, 3);

        auto sprite = ButtonSprite::create(text, 112, true, "bigFont.fnt", "GJ_button_01.png", 25.f, 0.42f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        button->setPosition(position);
        button->setID(nodeID);
        menu->addChild(button);

        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(0.31f);
        label->setPosition(position + CCPoint { 0.f, -22.f });
        label->setOpacity(225);
        page->addChild(label, 2);
        return label;
    }

    void togglePanel() {
        if (m_open) {
            this->closePanel();
        } else {
            this->openPanel();
        }
    }

    void openPanel() {
        m_open = true;
        m_panel->stopAllActions();
        m_panel->setVisible(true);
        m_panel->runAction(CCEaseBackOut::create(CCScaleTo::create(0.18f, 1.f)));
        m_panel->runAction(CCFadeTo::create(0.12f, 255));
        this->refreshStateLabels();
    }

    void closePanel() {
        m_open = false;
        m_panel->stopAllActions();
        m_panel->runAction(CCSequence::create(
            CCSpawn::create(CCScaleTo::create(0.12f, 0.86f), CCFadeTo::create(0.12f, 0), nullptr),
            CCHide::create(),
            nullptr
        ));
    }

    void switchTab(HubTab tab) {
        m_tab = tab;
        if (m_playerPage) {
            m_playerPage->setVisible(tab == HubTab::Player);
        }
        if (m_assistPage) {
            m_assistPage->setVisible(tab == HubTab::Assist);
        }
        if (m_visualPage) {
            m_visualPage->setVisible(tab == HubTab::Visual);
        }
        if (m_tabTitleLabel) {
            switch (tab) {
                case HubTab::Player:
                    m_tabTitleLabel->setString("Player Controls");
                    break;
                case HubTab::Assist:
                    m_tabTitleLabel->setString("Assist Suite");
                    break;
                case HubTab::Visual:
                    m_tabTitleLabel->setString("Visual Tools");
                    break;
            }
        }
    }

    void updateToggleLabel(CCLabelBMFont* label, char const* name, bool enabled, ccColor3B onColor = { 100, 255, 125 }, ccColor3B offColor = { 255, 135, 135 }) {
        if (!label) {
            return;
        }

        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s: %s", name, enabled ? "ON" : "OFF");
        label->setString(buffer);
        label->setColor(enabled ? onColor : offColor);
    }

    void refreshStateLabels() {
        this->updateToggleLabel(m_damageLabel, "No Death", g_ignoreDamage);
        this->updateToggleLabel(m_practiceLabel, "Practice", g_practiceMode, { 100, 255, 125 }, { 255, 220, 120 });
        this->updateToggleLabel(m_autoPlayLabel, "Auto Play", g_autoPlay, { 100, 255, 125 }, { 255, 135, 135 });
        this->updateToggleLabel(m_cubeLabel, "Cube AI", g_autoCube, { 100, 255, 125 }, { 255, 220, 120 });
        this->updateToggleLabel(m_waveLabel, "Wave AI", g_autoWave, { 100, 255, 125 }, { 255, 220, 120 });
        this->updateToggleLabel(m_platformerLabel, "Platform", g_platformerAssist, { 100, 255, 125 }, { 185, 190, 255 });
        this->updateToggleLabel(m_hitboxLabel, "Hitboxes", g_showHitboxes, { 100, 255, 125 }, { 185, 190, 255 });
    }

    void tickStatus(float) {
        auto playLayer = PlayLayer::get();
        if (!m_statusLabel || !playLayer) {
            return;
        }

        auto percent = playLayer->getCurrentPercent();
        char buffer[128];
        std::snprintf(
            buffer,
            sizeof(buffer),
            "%.2f%% | %s %s %s",
            percent,
            g_autoPlay ? "Auto" : "Manual",
            g_showHitboxes ? "Hitbox" : "Clean",
            g_ignoreDamage ? "Safe" : "Live"
        );
        m_statusLabel->setString(buffer);
        this->refreshStateLabels();
    }

    void applyGameplayOptions() {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleIgnoreDamage(g_ignoreDamage);
            if (g_practiceMode) {
                playLayer->togglePracticeMode(true);
            }
            if (g_showHitboxes) {
                playLayer->updateDebugDrawSettings();
            }
        }
    }

    void onPlayerTab(CCObject*) {
        this->switchTab(HubTab::Player);
    }

    void onAssistTab(CCObject*) {
        this->switchTab(HubTab::Assist);
    }

    void onVisualTab(CCObject*) {
        this->switchTab(HubTab::Visual);
    }

    void onToggleDamage(CCObject*) {
        g_ignoreDamage = !g_ignoreDamage;
        this->applyGameplayOptions();
        this->refreshStateLabels();
        showToast(g_ignoreDamage ? "No Death enabled" : "No Death disabled");
    }

    void onTogglePractice(CCObject*) {
        g_practiceMode = !g_practiceMode;
        if (auto playLayer = PlayLayer::get()) {
            playLayer->togglePracticeMode(g_practiceMode);
        }
        this->refreshStateLabels();
        showToast(g_practiceMode ? "Practice Mode enabled" : "Practice Mode disabled");
    }

    void onToggleAutoPlay(CCObject*) {
        g_autoPlay = !g_autoPlay;
        if (!g_autoPlay) {
            releaseAutoButtons();
        }
        this->refreshStateLabels();
        showToast(g_autoPlay ? "Auto Play enabled" : "Auto Play disabled");
    }

    void onToggleCube(CCObject*) {
        g_autoCube = !g_autoCube;
        this->refreshStateLabels();
        showToast(g_autoCube ? "Cube Auto enabled" : "Cube Auto disabled");
    }

    void onToggleWave(CCObject*) {
        g_autoWave = !g_autoWave;
        this->refreshStateLabels();
        showToast(g_autoWave ? "Wave Auto enabled" : "Wave Auto disabled");
    }

    void onTogglePlatformer(CCObject*) {
        g_platformerAssist = !g_platformerAssist;
        if (!g_platformerAssist) {
            releaseAutoButtons();
        }
        this->refreshStateLabels();
        showToast(g_platformerAssist ? "Platform assist enabled" : "Platform assist disabled");
    }

    void onToggleHitboxes(CCObject*) {
        g_showHitboxes = !g_showHitboxes;
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleDebugDraw();
            playLayer->updateDebugDrawSettings();
        }
        this->refreshStateLabels();
        showToast(g_showHitboxes ? "Hitboxes enabled" : "Hitboxes disabled");
    }

    void onProgressbar(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleProgressbar();
            showToast("Progress bar toggled");
        }
    }

    void onInfoLabel(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->toggleInfoLabel();
            showToast("Info label toggled");
        }
    }

    void onDebugPulse(CCObject*) {
        if (auto playLayer = PlayLayer::get()) {
            playLayer->updateDebugDrawSettings();
            showToast("Debug draw refreshed");
        }
    }

    void onRestart(CCObject*) {
        releaseAutoButtons();
        if (auto playLayer = PlayLayer::get()) {
            playLayer->resetLevelFromStart();
            this->applyGameplayOptions();
            showToast("Level restarted");
        }
    }

    void onAbout(CCObject*) {
        FLAlertLayer::create(
            "Emir Hub",
            "<cg>Emir Hub</c> now has Mega-Hack-style tabs, autoplay assists for <cy>cube</c> and <cy>wave</c>, hitbox/debug tools, and platformer helpers.\n\n"
            "Auto Play is a safe heuristic assist; enable No Death while testing hard levels.",
            "OK"
        )->show();
    }
};

class $modify(EmirHubPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        if (auto menu = ModernMenu::create()) {
            this->addChild(menu, 9999);
        }

        this->toggleIgnoreDamage(g_ignoreDamage);
        if (g_practiceMode) {
            this->togglePracticeMode(true);
        }
        if (g_showHitboxes) {
            this->toggleDebugDraw();
            this->updateDebugDrawSettings();
        }
        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        runAutoPlay(this);
    }

    void resetLevel() {
        releaseAutoButtons(this->m_player1);
        PlayLayer::resetLevel();
        this->toggleIgnoreDamage(g_ignoreDamage);
        if (g_practiceMode) {
            this->togglePracticeMode(true);
        }
        if (g_showHitboxes) {
            this->updateDebugDrawSettings();
        }
    }

    void onQuit() {
        releaseAutoButtons(this->m_player1);
        PlayLayer::onQuit();
    }
};
