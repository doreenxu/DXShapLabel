#include "WeCLabelReader.h"

#include "WeCLabel.h"
#include "tinyxml2/tinyxml2.h"
#include "flatbuffers/flatbuffers.h"
#include "cocostudio/CSParseBinary_generated.h"
#include "WeCLabelOptions_generated.h"

USING_NS_CC;
using namespace flatbuffers;

// force using namespace ui, sea config of lua binding.
using namespace ui;

static WeCLabelReader* instanceWeCLabelReader = nullptr;

IMPLEMENT_CLASS_NODE_READER_INFO(WeCLabelReader)

WeCLabelReader::WeCLabelReader()
{
}

WeCLabelReader::~WeCLabelReader()
{
}

WeCLabelReader* WeCLabelReader::getInstance()
{
    if (!instanceWeCLabelReader)
    {
        instanceWeCLabelReader = new (std::nothrow) WeCLabelReader();
    }
    return instanceWeCLabelReader;
}

void WeCLabelReader::destroyInstance()
{
    CC_SAFE_DELETE(instanceWeCLabelReader);
}

Offset<Table> WeCLabelReader::createOptionsWithFlatBuffers(const tinyxml2::XMLElement *objectData, flatbuffers::FlatBufferBuilder *builder)
{
    auto temp = WidgetReader::getInstance()->createOptionsWithFlatBuffers(objectData, builder);
    auto nodeOptions = *(Offset<WidgetOptions>*)(&temp);

    std::string textFontName;
    std::string textFontSize;
    Color4B textFontColor;
    std::string textHorAlign;
    std::string textVerAlign;
    std::string textOverflow;
    std::string textSpacingX;
    std::string textSpacingY;
    std::string textMaxLines;
    std::string textLevel;
    std::string textBbcode;
    std::string textEffect;
    Color4B textEffectColor;
    std::string textBorderSize;
    std::string textShadowOffsetX;
    std::string textShadowOffsetY;
    std::string textText;

    const tinyxml2::XMLAttribute* attribute = objectData->FirstAttribute();
    while (attribute)
    {
        std::string attriname = attribute->Name();
        std::string value = attribute->Value();

        if (attriname == "FontName") textFontName = value;
        if (attriname == "FontSize") textFontSize = value;
        if (attriname == "HorAlign") textHorAlign = value;
        if (attriname == "VerAlign") textVerAlign = value;
        if (attriname == "Overflow") textOverflow = value;
        if (attriname == "SpacingX") textSpacingX = value;
        if (attriname == "SpacingY") textSpacingY = value;
        if (attriname == "MaxLines") textMaxLines = value;
        if (attriname == "Level") textLevel = value;
        if (attriname == "Bbcode") textBbcode = value;
        if (attriname == "Effect") textEffect = value;
        if (attriname == "BorderSize") textBorderSize = value;
        if (attriname == "ShadowOffsetX") textShadowOffsetX = value;
        if (attriname == "ShadowOffsetY") textShadowOffsetY = value;
        if (attriname == "Text") textText = value;

        attribute = attribute->Next();
    }

    auto child = objectData->FirstChildElement();
    while (child)
    {
        std::string childname = child->Name();

        if (childname == "FontColor")
        {
            auto att = child->FirstAttribute();
            while (att)
            {
                std::string name = att->Name();
                std::string value = att->Value();
                if (name == "R") textFontColor.r = (GLubyte)atoi(value.c_str());
                else if (name == "G") textFontColor.g = (GLubyte)atoi(value.c_str());
                else if (name == "B") textFontColor.b = (GLubyte)atoi(value.c_str());
                else if (name == "A") textFontColor.a = (GLubyte)atoi(value.c_str());
                att = att->Next();
            }
        }
        if (childname == "EffectColor")
        {
            auto att = child->FirstAttribute();
            while (att)
            {
                std::string name = att->Name();
                std::string value = att->Value();
                if (name == "R") textEffectColor.r = (GLubyte)atoi(value.c_str());
                else if (name == "G") textEffectColor.g = (GLubyte)atoi(value.c_str());
                else if (name == "B") textEffectColor.b = (GLubyte)atoi(value.c_str());
                else if (name == "A") textEffectColor.a = (GLubyte)atoi(value.c_str());
                att = att->Next();
            }
        }

        child = child->NextSiblingElement();
    }

    std::string tmpFontName = (textFontName.c_str());
    int tmpFontSize = std::atoi(textFontSize.c_str());
    flatbuffers::Color tmpFontColor(textFontColor.a, textFontColor.r, textFontColor.g, textFontColor.b);
    int tmpHorAlign = std::atoi(textHorAlign.c_str());
    int tmpVerAlign = std::atoi(textVerAlign.c_str());
    int tmpOverflow = std::atoi(textOverflow.c_str());
    float tmpSpacingX = (float)std::atof(textSpacingX.c_str());
    float tmpSpacingY = (float)std::atof(textSpacingY.c_str());
    int tmpMaxLines = std::atoi(textMaxLines.c_str());
    int tmpLevel = std::atoi(textLevel.c_str());
    bool tmpBbcode = textBbcode == "True" ? true : false;
    int tmpEffect = std::atoi(textEffect.c_str());
    flatbuffers::Color tmpEffectColor(textEffectColor.a, textEffectColor.r, textEffectColor.g, textEffectColor.b);
    int tmpBorderSize = std::atoi(textBorderSize.c_str());
    int tmpShadowOffsetX = std::atoi(textShadowOffsetX.c_str());
    int tmpShadowOffsetY = std::atoi(textShadowOffsetY.c_str());
    std::string tmpText = (textText.c_str());

    auto options = flatbuffers::CreateWeCLabelOptions(*builder,
        nodeOptions,
        builder->CreateString(tmpFontName),
        tmpFontSize,
        &tmpFontColor,
        tmpHorAlign,
        tmpVerAlign,
        tmpOverflow,
        tmpSpacingX,
        tmpSpacingY,
        tmpMaxLines,
        tmpLevel,
        tmpBbcode,
        tmpEffect,
        &tmpEffectColor,
        tmpBorderSize,
        tmpShadowOffsetX,
        tmpShadowOffsetY,
        builder->CreateString(tmpText)
        );

    return *(Offset<Table>*)(&options);
}

void WeCLabelReader::setPropsWithFlatBuffers(cocos2d::Node *node, const flatbuffers::Table* singleNodeOptions)
{
    auto options = (flatbuffers::WeCLabelOptions*)singleNodeOptions;

    WeCLabel* real = dynamic_cast<WeCLabel*>(node);
    real->setFontName(options->fontName()->c_str());
    real->setFontSize(options->fontSize());
    auto tmpFontColor = options->fontColor();
    real->setFontColor(Color4B(tmpFontColor->r(), tmpFontColor->g(), tmpFontColor->b(), tmpFontColor->a()));
#ifdef CC_STUDIO_ENABLED_VIEW
    real->setHorAlign(options->horAlign());
    real->setVerAlign(options->verAlign());
    real->setOverflow(options->overflow());
    real->setEffect(options->effect());
#else
    real->setHorAlign(static_cast<WeCLabel::HorAlign>(options->horAlign()));
    real->setVerAlign(static_cast<WeCLabel::VerAlign>(options->verAlign()));
    real->setOverflow(static_cast<WeCLabel::Overflow>(options->overflow()));
    real->setEffect(static_cast<WeCLabel::Effect>(options->effect()));
#endif
    real->setSpacingX(options->spacingX());
    real->setSpacingY(options->spacingY());
    real->setMaxLines(options->maxLines());
    real->setLevel(options->level());
    real->setBbcode(options->bbcode());
    auto tmpEffectColor = options->effectColor();
    real->setEffectColor(Color4B(tmpEffectColor->r(), tmpEffectColor->g(), tmpEffectColor->b(), tmpEffectColor->a()));
    real->setBorderSize(options->borderSize());
    real->setShadowOffsetX(options->shadowOffsetX());
    real->setShadowOffsetY(options->shadowOffsetY());
    real->setText(options->text()->c_str());

}

Node* WeCLabelReader::createNodeWithFlatBuffers(const flatbuffers::Table *nodeOptions)
{
    WeCLabel* node = WeCLabel::create();

    auto options = (flatbuffers::WeCLabelOptions*)nodeOptions;

    // super
    auto superReader = WidgetReader::getInstance();
    superReader->setPropsWithFlatBuffers(node, (Table*)options->nodeOptions());

    // self
    setPropsWithFlatBuffers(node, (Table*)options);

    return node;
}
