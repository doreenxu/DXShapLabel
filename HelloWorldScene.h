#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

#include "cocos/ui/UIVideoPlayer.h"
#include "extensions/GUI/CCScrollView/CCTableView.h"

class HelloWorld :
    public cocos2d::Layer//,
    // public cocos2d::extension::TableViewDataSource
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    // CREATE_FUNC(HelloWorld);

    static HelloWorld* create()
    {
        HelloWorld *pRet = new(std::nothrow) HelloWorld();
        if (pRet && pRet->init())
        {
            pRet->autorelease();
            return pRet;
        }
        else
        {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }

    virtual void onEnter() override;
    virtual void onExit() override;
};

#endif // __HELLOWORLD_SCENE_H__
