#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "EmirGui.hpp"

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        
        // EmirGui ile buton oluşturma
        auto btn = EmirGui::createCircleBtn(
            "GJ_likeBtn_001.png", 
            this, 
            menu_selector(MyMenuLayer::onEmirClick)
        );

        btn->setID("emir-buton"_spr);
        EmirGui::addToMenu(dynamic_cast<CCMenu*>(menu), btn);

        return true;
    }

    void onEmirClick(CCObject* sender) {
        // Framework'teki Popup sınıfını tetikleme
        EmirGui::Popup::create("EmirGui Paneli")->show();
    }
};
