
#include "cocostudio/CocosStudioExport.h"

/*  Custom include block. You should write
    your own include in this block only. Do not
    change the Tag "CUSTOM_INCLUDE_BEGIN" &
    "CUSTOM_INCLUDE_END", The generator use these
    tags to determine the block which need to
    copy at next auto-generation.
    */

// CUSTOM_INCLUDE_BEGIN
#include "ui/CocosGUI.h"
#include "editor-support/cocostudio/WidgetReader/WidgetReader.h"
// CUSTOM_INCLUDE_END

#ifndef _READER_WeCLabel_H_
#define _READER_WeCLabel_H_

using namespace cocostudio;

/*  Custom namespace block. You should write
    your own include in this block only. Do not
    change the Tag "CUSTOM_NAMESPACE_START_BEGIN" &
    "CUSTOM_NAMESPACE_START_END", The generator use these
    tags to determine the block which need to
    copy at next auto-generation.
    */

// CUSTOM_NAMESPACE_START_BEGIN
// CUSTOM_NAMESPACE_START_END

class WeCLabelReader : public WidgetReader
{
    /*  Custom code block. You should write
        your own code in this block only. Do not
        change the Tag "CUSTOM_CODE_BEGIN" &
        "CUSTOM_CODE_END", The generator use these
        tags to determine the block which need to
        copy at next auto-generation.
        */

    // CUSTOM_CODE_BEGIN
    // CUSTOM_CODE_END

    DECLARE_CLASS_NODE_READER_INFO

public:
    WeCLabelReader();
    virtual ~WeCLabelReader();

    static WeCLabelReader* getInstance();

    static void destroyInstance();

    flatbuffers::Offset<flatbuffers::Table> createOptionsWithFlatBuffers(const tinyxml2::XMLElement* objectData, flatbuffers::FlatBufferBuilder* builder);

    void setPropsWithFlatBuffers(cocos2d::Node* node, const flatbuffers::Table* singleNodeOptions);

    cocos2d::Node* createNodeWithFlatBuffers(const flatbuffers::Table* options);
};

/*  Custom namespace end block. You should write
    your own include in this block only. Do not
    change the Tag "CUSTOM_NAMESPACE_END_BEGIN" &
    "CUSTOM_NAMESPACE_END_END", The generator use these
    tags to determine the block which need to
    copy at next auto-generation.
    */

// CUSTOM_NAMESPACE_END_BEGIN
// CUSTOM_NAMESPACE_END_END

#endif  // _READER_WeCLabel_H_

