#include "HelloWorldScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "tinyxml2/tinyxml2.h"
#include "WeCLabel.h"
// #include "cocos/ui/UIVideoPlayer.h"

#include "WeCLabel.h"
#include "WeCChartFontManager.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

cocos2d::ui::WeCLabel *lbl = nullptr;

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init())
    {
        return false;
    }

    auto scene = CSLoader::getInstance()->createNode("Layer.csb");
    this->addChild(scene);

    lbl = dynamic_cast<cocos2d::ui::WeCLabel*>(scene->getChildByName("lbl"));

    return true;
}

void HelloWorld::onEnter()
{
    cocos2d::Layer::onEnter();
}

void HelloWorld::onExit()
{
    cocos2d::Layer::onExit();
}
