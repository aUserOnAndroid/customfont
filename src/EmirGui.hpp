#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

namespace EmirGui {
    // Sürüklenebilir Özel Panel Sınıfı
    class DraggablePanel : public CCNode {
    private:
        bool m_isDragging = false;
        CCPoint m_dragOffset;
        CCSize m_size;
        CCScale9Sprite* m_bg;

    public:
        static DraggablePanel* create(float width, float height) {
            auto ret = new DraggablePanel();
            if (ret && ret->init(width, height)) {
                ret->autorelease();
                return ret;
            }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool init(float width, float height) {
            m_size = CCSize{width, height};
            this->setContentSize(m_size);

            // Arka Plan Görseli
            m_bg = CCScale9Sprite::create("GJ_square01.png");
            m_bg->setContentSize(m_size);
            m_bg->setAnchorPoint({0, 0});
            this->addChild(m_bg);

            // Dokunma (Sürükleme) Algılayıcı aktif etme
            auto director = CCDirector::get();
            director->getTouchDispatcher()->addTargetedDelegate(this, 0, true);

            return true;
        }

        // Sürükleme Mantığı
        bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
            auto pos = this->convertTouchToNodeSpace(touch);
            CCRect rect = {0, 0, m_size.width, m_size.height};
            
            if (rect.containsPoint(pos) && this->isVisible()) {
                m_isDragging = true;
                m_dragOffset = this->getPosition() - touch->getLocation();
                return true;
            }
            return false;
        }

        void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
            if (m_isDragging) {
                this->setPosition(touch->getLocation() + m_dragOffset);
            }
        }

        void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
            m_isDragging = false;
        }
    };

    // Toggle (Aç/Kapat) Butonu Oluşturucu
    inline CCMenuItemToggler* createToggle(CCObject* target, SEL_MenuHandler callback) {
        auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        return CCMenuItemToggler::create(offSpr, onSpr, target, callback);
    }
}
