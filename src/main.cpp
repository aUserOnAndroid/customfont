#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "EmirGui.hpp"

using namespace geode::prelude;

// Küresel Ayar Değişkeni (Hitbox açık mı kapalı mı?)
bool g_showHitboxesEnabled = false;

// Sürüklenen Panel için Global Referans
EmirGui::DraggablePanel* g_myPanel = nullptr;

// --- 1. OYUN İÇİ HITBOX HOOK SİSTEMİ ---
class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        // Eğer toggle açıksa oyun başladığında hitboxları zorla göster
        if (g_showHitboxesEnabled) {
            this->m_showHitboxes = true;
        }

        return true;
    }
};

// --- 2. ANA MENÜ VE GUI PANEL SİSTEMİ ---
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto winSize = CCDirector::get()->getWinSize();
        auto menu = this->getChildByID("bottom-menu");

        // Menüdeki Mod Açma Butonu (Mavi Dişli İkonu)
        auto openBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(MyMenuLayer::togglePanelVisibility)
        );
        openBtn->setID("emir-gui-toggle"_spr);
        menu->addChild(openBtn);
        menu->updateLayout();

        // Sürüklenebilir Paneli Oluştur (Eğer önceden oluşturulmadıysa)
        if (!g_myPanel) {
            g_myPanel = EmirGui::DraggablePanel::create(220, 120);
            g_myPanel->setPosition({winSize.width / 2 - 110, winSize.height / 2 - 60});
            g_myPanel->setVisible(false); // Başlangıçta gizli
            
            // Panelin içine butonlar için bir menü ekleyelim
            auto panelMenu = CCMenu::create();
            panelMenu->setPosition({0, 0});
            g_myPanel->addChild(panelMenu);

            // Panel Başlığı
            auto title = CCLabelBMFont::create("Emir Mod Menu", "bigFont.fnt");
            title->setScale(0.4f);
            title->setPosition({110, 95});
            g_myPanel->addChild(title);

            // Hitbox Yazısı
            auto label = CCLabelBMFont::create("Show Hitboxes", "goldFont.fnt");
            label->setScale(0.5f);
            label->setPosition({90, 50});
            g_myPanel->addChild(label);

            // Hitbox Toggle Butonu
            auto hitboxToggle = EmirGui::createToggle(this, menu_selector(MyMenuLayer::onHitboxToggle));
            hitboxToggle->setPosition({170, 50});
            hitboxToggle->setScale(0.7f);
            hitboxToggle->toggle(g_showHitboxesEnabled); // Mevcut durumu yükle
            panelMenu->addChild(hitboxToggle);

            // Paneli ekrana bas (Sahneye ekle)
            this->addChild(g_myPanel, 100);
        } else {
            // Eğer sahne yenilenirse paneli yeni sahneye tekrar bağla
            g_myPanel->removeFromParent();
            this->addChild(g_myPanel, 100);
        }

        return true;
    }

    // Ana butona basınca paneli açar / kapatır
    void togglePanelVisibility(CCObject*) {
        if (g_myPanel) {
            g_myPanel->setVisible(!g_myPanel->isVisible());
        }
    }

    // Hitbox Toggle'ına basıldığında çalışacak kod
    void onHitboxToggle(CCObject* sender) {
        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        // Toggler durumunu tersine çeviriyoruz (isToggled() yeni durumu verir)
        g_showHitboxesEnabled = !toggle->isToggled(); 
        
        // Bilgilendirme logu
        log::info("Hitbox Durumu Değişti: {}", g_showHitboxesEnabled);
    }
};
