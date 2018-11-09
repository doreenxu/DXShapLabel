#include "WeCChartFontManager.h"

#include <array>

#if CC_TARGET_PLATFORM != CC_PLATFORM_WIN32 && CC_TARGET_PLATFORM != CC_PLATFORM_WINRT && CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID
#include <iconv.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
#endif

#include "tinyxml2/tinyxml2.h"
#include "2d/CCFontFreeType.h"
#include "xxhash/xxhash.h"
#include "base/CCDirector.h"
#include "platform/CCFileUtils.h"
#include "base/CCEventDispatcher.h"
#include "renderer/CCGLProgram.h"
#include "WeCLabel.h"
#include "2d/CCCamera.h"

NS_CC_BEGIN

#if CC_STUDIO_ENABLED_VIEW
const char *WeCLabel::sVertexShaderSource =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec4 a_color;\n"
"#ifdef GL_ES\n"
"varying lowp vec4 v_fragmentColor;\n"
"varying mediump vec2 v_texCoord;\n"
"\n#else\n"
"varying vec4 v_fragmentColor;\n"
"varying vec2 v_texCoord;\n"
"\n#endif\n"
"void main()\n"
"{\n"
"   gl_Position = CC_PMatrix * a_position;\n"
"   v_fragmentColor = a_color;\n"
"   v_texCoord = a_texCoord;\n"
"}\n";

const char *WeCLabel::sFragmentShaderSource =
"#ifdef GL_ES\n"
"precision lowp float;\n"
"#endif\n"
"varying vec4 v_fragmentColor;"
"varying vec2 v_texCoord;"
"void main()\n"
"{\n"
"   gl_FragColor = vec4(v_fragmentColor.rgb, v_fragmentColor.a * texture2D(CC_Texture0, v_texCoord).a);\n"
"}";

const char *WeCLabel::sShaderName = "ShaderWeCLabel";
#endif

WeCCharFontManager::WeCFontConfig::WeCFontConfig()
    :fontName(""), fontSize(0), outline(0) // Fix the crash.
{
}

WeCCharFontManager::WeCFontConfig::~WeCFontConfig()
{
}

WeCSystemFontInfo::WeCSystemFontInfo()
{
}

WeCSystemFontInfo::~WeCSystemFontInfo()
{
}

#ifdef CC_STUDIO_ENABLED_VIEW
FT_Library WeCFontFreeType::sFTlibrary = nullptr;
bool WeCFontFreeType::sFTLibraryInit = false;
#endif

WeCFontFileInfoMap WeCFontFreeType::sFontFileInfos = WeCFontFileInfoMap();

WeCFontFreeType::WeCFontFreeType()
    :_iconv(nullptr), mIsValid(false)
{
}

WeCFontFreeType::~WeCFontFreeType(void)
{
}

int WeCFontFreeType::getPrintedLineHeight(void) const
{
    return 0;
}

bool WeCFontFreeType::_getFontFace(const std::string &name, FT_Face &face, FT_Encoding &encode)
{
    // Check if the font face existed or not.
    auto found = sFontFileInfos.find(name);
    if (found != sFontFileInfos.end())
    {
        face = found->second.face;
        encode = found->second.encode;
        return true;
    }

    // Get the full path of the font file.
    auto fileutils = FileUtils::getInstance();

    std::string fullPath = fileutils->fullPathForFilename(name);

    CCLOG("WeCFontFreeType::_getFontFace: %s, %s", name.c_str(), fullPath.c_str());

    Data data;
    FT_Error error = 0;

    unsigned char* dataPtr = nullptr;
    unsigned int dataSize = 0;

    if (fullPath.find("assets/") != std::string::npos)
    {
        // If start with assets/, use assets manager to load the data, and use FT_New_Memory_Face
        // to create the font face.
        data = FileUtils::getInstance()->getDataFromFile(fullPath);

        if (data.isNull())
        {
            CCLOG("create font failed %s, data is null", fullPath.c_str());
            return false;
        }
        
        dataSize = data.getSize();
        dataPtr = (unsigned char*)malloc(sizeof(unsigned char*) * data.getSize());
        memcpy(dataPtr, data.getBytes(), sizeof(unsigned char) * dataSize);

        CCLOG("start create font %s, data pointer %p, %d", fullPath.c_str(), dataPtr, dataSize);

        error = FT_New_Memory_Face(_getFTLibrary(), dataPtr, dataSize, 0, &face);
    }
    else
    {
        // Load the face from the path.
        error = FT_New_Face(_getFTLibrary(), fullPath.c_str(), 0, &face);
    }

    if (error != 0)
    {
        if (face != nullptr)
        {
            FT_Done_Face(face);
            face = nullptr;
        }

        return false;
    }

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
    {
        int foundIndex = -1;
        for (int charmapIndex = 0; charmapIndex < face->num_charmaps; charmapIndex++)
        {
            if (face->charmaps[charmapIndex]->encoding != FT_ENCODING_NONE)
            {
                foundIndex = charmapIndex;
                break;
            }
        }

        if (foundIndex == -1)
            return false;

        encode = face->charmaps[foundIndex]->encoding;
        if (FT_Select_Charmap(face, encode))
        {
            if (face != nullptr)
            {
                FT_Done_Face(face);
                face = nullptr;
            }

            return false;
        }
    }

    // Clear font supported.
    /*
#ifndef CC_STUDIO_ENABLED_VIEW
    int printFontSize = fontSize;
    if (ignoreScale == false)
    {
        float scale = WeCCharFontManager::sFontScale;
        printFontSize = std::ceil(fontSize * scale);
    }

#else
    int printFontSize = fontSize;
#endif

    if (FT_Set_Pixel_Sizes(face, printFontSize, printFontSize))
    {
        if (face != nullptr)
        {
            FT_Done_Face(face);
            face = nullptr;
        }

        return false;
    }

    lineHeight = static_cast<int>(face->size->metrics.height >> 6);
    */

    WeCFontFileInfo info;
    info.face = face;
    info.encode = encode;
    info.data = dataPtr;
    info.dataSize = dataSize;
    info.reference = 1;
    sFontFileInfos.emplace(name, info);

    return true;
}

bool WeCFontFreeType::_doneFontface(const std::string &name)
{
    auto found = sFontFileInfos.find(name);

    if (found == sFontFileInfos.end())
        return false;

    found->second.reference -= 1;

    if (found->second.reference <= 0)
    {
        // Destroy the face.
        if (found->second.face != nullptr)
            FT_Done_Face(found->second.face);

        // Destroy the data if is not null.
        if (found->second.data != nullptr)
        {
            free(found->second.data);
            found->second.data = nullptr;
            found->second.dataSize = 0;
        }

        // remove the font from the map.
        sFontFileInfos.erase(found);
    }

    return true;
}

unsigned short WeCFontFreeType::_convertUnicode2GB2312(unsigned short ch)
{
    if (ch < 256)
        return ch;

    const size_t strLen = 1;
    const size_t gb2312StrSize = 2;
    char gb2312Text[gb2312StrSize];
    memset(gb2312Text, 0, gb2312StrSize);

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT
    WideCharToMultiByte(936, NULL, (LPCWCH)(&ch), strLen, (LPSTR)gb2312Text, gb2312StrSize, NULL, NULL);
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    conversionEncodingJNI((char*)(&ch), gb2312StrSize, "UTF-16LE", gb2312Text, "GB2312");
#else
    if (_iconv == nullptr)
    {
        _iconv = iconv_open("gb2312", "utf-16le");
    }

    if (_iconv == (iconv_t)-1)
    {
        CCLOG("conversion from utf16 to gb2312 not available");
    }
    else
    {
        char* pin = (char*)(&ch);
        char* pout = gb2312Text;
        size_t inLen = strLen * 2;
        size_t outLen = gb2312StrSize;

        iconv(_iconv, (char**)&pin, &inLen, &pout, &outLen);
    }
#endif

    unsigned short gb2312Code = 0;
    unsigned char* dst = (unsigned char*)&gb2312Code;
    dst[0] = gb2312Text[1];
    dst[1] = gb2312Text[0];

    return gb2312Code;
}

unsigned char* WeCFontFreeType::_getBitmap(unsigned short code, FT_Face face, FT_Stroker stroker, FT_Encoding encoding,
    float outline, int &outWidth, int &outHeight, Rect &outRect, int &xadvance, int &ascender)
{
    bool invalidChar = true;
    unsigned char* re = nullptr;

    unsigned short targetCode = code;

    switch (encoding)
    {
    case FT_ENCODING_UNICODE:
        targetCode = code;
        break;
    case FT_ENCODING_GB2312:
        targetCode = _convertUnicode2GB2312(code);
        break;
    default:
        break;
    }

    do
    {
        if (nullptr == face)
            break;

        if (outline <= 0)
        {
            char glyphName[1024];

            FT_Get_Glyph_Name(face, 76, glyphName, 1024);

            FT_Load_Glyph(face, 76, FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT);

            if (FT_Load_Char(face, targetCode, FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT))
            // if (FT_Load_Char(face, targetCode, FT_LOAD_RENDER))
                break;  // Break the do while.

            auto& metrics = face->glyph->metrics;
            outRect.origin.x = metrics.horiBearingX >> 6;
            outRect.origin.y = -(metrics.horiBearingY >> 6);
            outRect.size.width = (metrics.width >> 6);
            outRect.size.height = (metrics.height >> 6);
            xadvance = (static_cast<int>(face->glyph->metrics.horiAdvance >> 6));
            ascender = static_cast<int>(face->size->metrics.ascender >> 6);

            outWidth = face->glyph->bitmap.width;
            outHeight = face->glyph->bitmap.rows;
            re = face->glyph->bitmap.buffer;
        }
        else
        {
            auto library = cocos2d::FontFreeType::getFTLibrary();

            if (library == nullptr)
                break;

            if (stroker == nullptr)
                break;

            if (FT_Load_Char(face, targetCode, FT_LOAD_NO_BITMAP))
                break;

            if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
                break;

            FT_Glyph glyph;

            if (FT_Get_Glyph(face->glyph, &glyph))
            {
                if (glyph)
                    FT_Done_Glyph(glyph);
                glyph = nullptr;
                break;
            }

            FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

            if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
            {
                if (glyph)
                    FT_Done_Glyph(glyph);
                glyph = nullptr;
                break;
            }

            FT_BBox bbox;

            FT_Outline *outline = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);

            int width = (bbox.xMax - bbox.xMin) >> 6;
            int rows = (bbox.yMax - bbox.yMin) >> 6;

            int sizeNeed = width * rows * sizeof(unsigned char);
            auto cache = WeCCharFontManager::getInstance()->tryGetStrokerReusedCache(sizeNeed);

            if (cache == nullptr)
            {
                if (glyph)
                    FT_Done_Glyph(glyph);
                glyph = nullptr;
                break;
            }

            FT_Bitmap bmp;
            bmp.buffer = cache;
            memset(bmp.buffer, 0, width * rows);
            bmp.width = (int)width;
            bmp.rows = (int)rows;
            bmp.pitch = (int)width;
            bmp.pixel_mode = FT_PIXEL_MODE_GRAY;
            bmp.num_grays = 256;

            FT_Raster_Params params;
            memset(&params, 0, sizeof(params));
            params.source = outline;
            params.target = &bmp;
            params.flags = FT_RASTER_FLAG_AA;
            FT_Outline_Translate(outline, -bbox.xMin, -bbox.yMin);
            FT_Outline_Render(library, outline, &params);

            auto& metrics = face->glyph->metrics;
            outRect.origin.x = metrics.horiBearingX >> 6;
            outRect.origin.y = -(metrics.horiBearingY >> 6);
            outRect.size.width = (metrics.width >> 6);
            outRect.size.height = (metrics.height >> 6);
            xadvance = (static_cast<int>(face->glyph->metrics.horiAdvance >> 6));
            ascender = static_cast<int>(face->size->metrics.ascender >> 6);

            outWidth = width;
            outHeight = rows;
            re = cache;
        }

        invalidChar = false;
    } while (0);

    if (invalidChar)
    {
        outRect.size.width = 0;
        outRect.size.height = 0;
        xadvance = 0;

        return nullptr;
    }

    return re;
}

unsigned char* WeCFontFreeType::getBitmap(unsigned short code, int &outwith,
    int &outheight, Rect &outrect, int &xadvance, int &ascender)
{
    // Should implemented at concrete class.
    return nullptr;
}

FT_Library WeCFontFreeType::_getFTLibrary(void)
{
#ifndef CC_STUDIO_ENABLED_VIEW
    return FontFreeType::getFTLibrary();
#else
    initFreeType();
    return sFTlibrary;
#endif
}

#ifdef CC_STUDIO_ENABLED_VIEW
bool WeCFontFreeType::initFreeType()
{
    if (sFTLibraryInit == false)
    {
        // begin freetype
        if (FT_Init_FreeType(&sFTlibrary))
            return false;

        sFTLibraryInit = true;
    }

    return  sFTLibraryInit;
}
#endif

bool WeCFontFreeType::isValid(void) const
{
    return mIsValid;
}

WeCFontFreeTypeTTF::WeCFontFreeTypeTTF(const std::string &fontName, int fontSize, float outline, bool ignoreScale)
    :mFace(nullptr), mStroker(nullptr), mEncoding(FT_ENCODING_UNICODE), mFontName(fontName), mFontSize(fontSize),
    mOutline(outline), mLineHeight(0), mIgnoreScale(ignoreScale)
{
    // Try to load the TTF. If the TTF is not founded, the FreeType should not be validated.
    if (fontSize <= 0)
        return;

    if (fontName.empty())
        return;

    if (!_getFontFace(fontName, mFace, mEncoding))
        return;

    if (!prepareRender())
        return;

    mIsValid = true;
}

WeCFontFreeTypeTTF::~WeCFontFreeTypeTTF(void)
{
    // if (nullptr != mFace)
    //      FT_Done_Face(mFace);
    _doneFontface(mFontName);

    if (mStroker != nullptr)
    {
        FT_Stroker_Done(mStroker);
        mStroker = nullptr;
    }
}

bool WeCFontFreeTypeTTF::prepareRender(void)
{
    mLineHeight = 0;

#ifndef CC_STUDIO_ENABLED_VIEW
    int printFontSize = mFontSize;
    if (mIgnoreScale == false)
    {
        float scale = WeCCharFontManager::getInstance()->sFontScale;
        printFontSize = std::ceil(mFontSize * scale);
    }

#else
    int printFontSize = fontSize;
#endif

    FT_Error error = FT_Set_Pixel_Sizes(mFace, printFontSize, printFontSize);

    if (error != 0)
        return false;

    mLineHeight = static_cast<int>(mFace->size->metrics.height >> 6);

    do 
    {
        if (mOutline <= 0)
            break;

        auto library = cocos2d::FontFreeType::getFTLibrary();

        if (library == nullptr)
            break;

        FT_Error err = 0;

        if (mStroker == nullptr)
            err = FT_Stroker_New(library, &mStroker);

        if (err != 0)
        {
            if (mStroker != nullptr)
            {
                FT_Stroker_Done(mStroker);
                mStroker = nullptr;
            }
            break;
        }

        FT_Stroker_Set(mStroker, (int)(mOutline * 64),
            FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    } while (0);

    return true;
}

unsigned char* WeCFontFreeTypeTTF::getBitmap(unsigned short code, int &outWidth,
    int &outHeight, Rect &outRect, int &xadvance, int &ascender)
{
    if (!prepareRender())
        return nullptr;

    return _getBitmap(code, mFace, mStroker, mEncoding, mOutline, outWidth,
        outHeight, outRect, xadvance, ascender);
}

int WeCFontFreeTypeTTF::getPrintedLineHeight(void) const
{
    return mLineHeight;
}

WeCFontAtlasLineInfo::WeCFontAtlasLineInfo(void)
    : nextXOffset(0), lineOffset(0), lineCount(0), textureIndex(0)
{
}

WeCFontTextureInfo::WeCFontTextureInfo(void)
    : textureCachedData(nullptr),
    format(cocos2d::Texture2D::PixelFormat::A8),
    usage(WeCCharFontManager::sAtlasHeight / WeCCharFontManager::sAtlasLineOffset),
    dirty(false)
{
}
WeCCharFontManager *WeCCharFontManager::sInstance = nullptr;
WeCSystemFontInfoMap WeCCharFontManager::sfontInfoMap = WeCSystemFontInfoMap();
int WeCCharFontManager::sAtlasWidth = 1024;
int WeCCharFontManager::sAtlasHeight = 1024;
int WeCCharFontManager::sAtlasLineOffset = 2;
float WeCCharFontManager::sFontScale = 1.0f;
bool WeCCharFontManager::sHasInit = false;
cocos2d::Mat4 WeCCharFontManager::sProjectionMat = cocos2d::Mat4::IDENTITY;
GLProgramState* WeCCharFontManager::sWecLabelProgram = nullptr;
GLProgramState* WeCCharFontManager::sWecLabelImgProgram = nullptr;
GLProgramState* WeCCharFontManager::sWecLabelImgETCProgram = nullptr;

unsigned int WeCCharFontManager::sWeCLabelId = 0;

unsigned int WeCCharFontManager::genWeCLabelId()
{
    return ++sWeCLabelId;
}

WeCCharFontManager::WeCCharFontManager(void)
    :mPreCameraDraw(nullptr),
    mAfterDraw(nullptr),
    mAfterVisit(nullptr),
    mHasNoMem(false),
    mClearMemLimit(0),
    mStrokerReusedCache(nullptr),
    mStrokerReusedCacheSize(0)
{

#ifndef CC_STUDIO_ENABLED_VIEW
    mPreCameraDraw = Director::getInstance()->getEventDispatcher()->addCustomEventListener(Director::EVENT_BEFORE_CAMERA_DRAW,
        std::bind(&WeCCharFontManager::_preCameraDraw, this, std::placeholders::_1));
    mPreCameraDraw->retain();

    mAfterDraw = Director::getInstance()->getEventDispatcher()->addCustomEventListener(Director::EVENT_AFTER_DRAW,
        std::bind(&WeCCharFontManager::_afterDraw, this, std::placeholders::_1));
    mAfterDraw->retain();

    // Add this event for notification node text rendering.
    mAfterVisit = Director::getInstance()->getEventDispatcher()->addCustomEventListener(Director::EVENT_AFTER_VISIT,
        std::bind(&WeCCharFontManager::_preCameraDraw, this, std::placeholders::_1));
    mAfterVisit->retain();
#endif
}

WeCCharFontManager::~WeCCharFontManager(void)
{
    for (auto &iter : mFontTextures)
    {
        if (nullptr != iter)
        {
            iter->release();
        }
    }
    mFontTextures.clear();

    for (auto &iter : mCachedTextureData)
    {
        if (nullptr != iter->textureCachedData)
        {
            free(iter->textureCachedData);
            iter->textureCachedData = nullptr;
        }
    }

    mCachedTextureData.clear();

    mPreCameraDraw->release();
    mPreCameraDraw = nullptr;

    mAfterDraw->release();
    mAfterDraw = nullptr;

    mAfterVisit->release();
    mAfterVisit = nullptr;
}

WeCCharFontManager* WeCCharFontManager::getInstance()
{
    if (nullptr == sInstance)
    {
        initialize();
        sInstance = new WeCCharFontManager();
    }
    return sInstance;
}

void WeCCharFontManager::_preCameraDraw(EventCustom* event)
{
    do 
    {
        auto camera = Camera::getVisitingCamera();

        if (camera == nullptr)
        {
            if (sWecLabelProgram != nullptr)
                sWecLabelProgram->setUniformMat4("projMat", sProjectionMat);

            if (sWecLabelImgProgram != nullptr)
                sWecLabelImgProgram->setUniformMat4("projMat", sProjectionMat);

            if (sWecLabelImgETCProgram != nullptr)
                sWecLabelImgETCProgram->setUniformMat4("projMat", sProjectionMat);

            break;
        }

        float scale = WeCCharFontManager::getInstance()->sFontScale;

        unsigned short mask = static_cast<unsigned short>(camera->getCameraFlag());

        if (mask & static_cast<unsigned short>(CameraFlag::DEFAULT))
        {
            if (sWecLabelProgram != nullptr)
                sWecLabelProgram->setUniformMat4("projMat", sProjectionMat);

            if (sWecLabelImgProgram != nullptr)
                sWecLabelImgProgram->setUniformMat4("projMat", sProjectionMat);

            if (sWecLabelImgETCProgram != nullptr)
                sWecLabelImgETCProgram->setUniformMat4("projMat", sProjectionMat);

            break;
        }

        if (sWecLabelProgram != nullptr)
            sWecLabelProgram->setUniformMat4("projMat", camera->getViewProjectionMatrix());

        if (sWecLabelImgProgram != nullptr)
            sWecLabelImgProgram->setUniformMat4("projMat", camera->getViewProjectionMatrix());

        if (sWecLabelImgETCProgram != nullptr)
            sWecLabelImgETCProgram->setUniformMat4("projMat", camera->getViewProjectionMatrix());
    } while (0);

    _updateTextures();
}

void WeCCharFontManager::_afterDraw(EventCustom* event)
{
    if (mClearMemLimit > 0)
    {
        mClearMemLimit -= 1;
        return;
    }

    if (mHasNoMem)
    {
        _clearMemory();

        // For each WeCLabel, set dirty.
        for (auto &iter : mWeCLabelMap)
            iter.second->mDirty = true;

        mHasNoMem = false;

        // Should clear the memory after 100 frames later to avoid performance problem.
        mClearMemLimit = 100;
    }
}

const TextureList& WeCCharFontManager::getTextures() const
{
    return mFontTextures;
}

void WeCCharFontManager::initialize()
{
    // Can init only once.
    if (sHasInit == true)
        return;

    auto glView = Director::getInstance()->getOpenGLView();
    float scale = glView != nullptr ? glView->getScaleX() : 1.0f;
    // scale = scale < 1.0f ? 1.0f : scale; // Disable Display size limit.
    // scale = scale > 1.6875f ? 1.6875f : scale; // max size is 1080p. // TODO:eranzhao, limit the scale size.

    sFontScale = scale;

    auto originalSize = glView->getFrameSize();

    cocos2d::Mat4::createOrthographicOffCenter(0, originalSize.width, 0, originalSize.height,
        -1024, 1024, &sProjectionMat);


#ifdef CC_STUDIO_ENABLED_VIEW
    // Create the shader with the const shader source string.
    do
    {
        if (GLProgramCache::getInstance()->getGLProgram(sShaderName))
            break;

        auto program = GLProgram::createWithByteArrays(sVertexShaderSource, sFragmentShaderSource);

        if (nullptr == program)
            break;

        GLProgramCache::getInstance()->addGLProgram(sWecLabelProgram, sShaderName);
    }
    while (0);

    sWecLabelProgram = GLProgramState::getOrCreateWithGLProgramName(sShaderName);
#else
    sWecLabelProgram = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL);
    sWecLabelImgProgram = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG);
    sWecLabelImgETCProgram = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG_ETC);
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    // Determine the version of current android system.
    // Parse fonts.xml when version larger equal than 5;
    // Parse system_fonts.xml and fallbackfonts.xml when version is larger equal than 4 and smaller than 5;
    // Init failed if the file not found.
    sHasInit = _parseFontsAndroid("/system/fonts", sfontInfoMap);

#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    sHasInit = _parseFontsAndroid("C:/system/fonts", sfontInfoMap);
#endif
}

cocos2d::GLProgramState* WeCCharFontManager::getWeCLabelShader()
{
    return sWecLabelProgram;
}

cocos2d::GLProgramState* WeCCharFontManager::getWeCLabelImgShader()
{
    return sWecLabelImgProgram;
}

cocos2d::GLProgramState* WeCCharFontManager::getWeCLabelImgETCShader()
{
    return sWecLabelImgETCProgram;
}

void WeCCharFontManager::_colletInternal(const std::u16string &str, const std::string &fontName, int fontSize, float outline)
{
    std::string id = generateKey(fontName, fontSize, outline, false);

    // Check the config is existed or not.
    auto found = mConfigs.find(id);

    if (found == mConfigs.end())
    {
        WeCFontConfig config;
        config.fontName = fontName;
        config.fontSize = fontSize;
        config.outline = outline;

        mConfigs.emplace(id, config);
    }

    // Collect them to the cache.
    if (mCollectedChars.count(id) <= 0)
        mCollectedChars.emplace(id, CharCodeRefMap());

    auto listFound = mCollectedChars.find(id);
    if (listFound != mCollectedChars.end())
    {
        auto &list = listFound->second;
        for (auto &iter : str)
        {
            if (list.count(iter) <= 0)
                list.emplace(iter, 1);
            else
                list[iter] += 1;
        }
    }
}

unsigned char* WeCCharFontManager::tryGetStrokerReusedCache(int sizeInBytes)
{
    if (sizeInBytes <= 0)
        return nullptr;

    if (sizeInBytes > mStrokerReusedCacheSize)
    {
        mStrokerReusedCache = static_cast<unsigned char*>(realloc(mStrokerReusedCache, sizeInBytes));
        mStrokerReusedCacheSize = sizeInBytes;
    }

    return mStrokerReusedCache;
}

void WeCCharFontManager::collectCharacters(const std::u16string &str, const std::string &fontName, int fontSize, float outline)
{
    _colletInternal(str, fontName, fontSize, outline);

    // Just process.
    _process();
}

WeCChartFontDefinitionPtr WeCCharFontManager::getCharFontDefinition(const std::string &configId, unsigned short code)
{
    auto found = mChars.find(configId);

    if (found == mChars.end())
        return nullptr;

    auto chFound = found->second.find(code);

    if (chFound == found->second.end())
        return nullptr;

    return chFound->second;
}

bool WeCCharFontManager::_processCollectedChars(bool &hasNoMem)
{
    for (auto &iter : mCollectedChars)
    {
        auto id = iter.first;

        if (mChars.count(id) <= 0)
            mChars.emplace(id, WeCChartFontDefinitionList()); // Create a new collection of font config.

        auto found = mChars.find(id);

        auto & list = found->second;
        auto &collect = iter.second;

        // Loop all characters, check the character is created or not.
        // If not existed, create a new one, and record the information.
        for (auto &iter : collect)
        {
            auto code = iter.first;
            if (list.find(code) != list.end())
                continue;

            // Has no generate the font yet, try to generate the font.
            _drawCharacterToAtlas(id, code, hasNoMem);
        }
    }

    return true;
}

void WeCCharFontManager::_clearMemory()
{
    // CCLOG("WeCCharFontManager::_clearMemory");

    for (auto &iter : mFontTextures)
    {
        if (nullptr != iter)
        {
            iter->release();
        }
    }
    mFontTextures.clear();

    for (auto &iter : mCachedTextureData)
    {
        if (nullptr != iter->textureCachedData)
        {
            free(iter->textureCachedData);
            iter->textureCachedData = nullptr;
        }
    }

    mCachedTextureData.clear();
    mAtlasLineConfigMap.clear();
    mChars.clear();

    // Clear the stroker cache memory.
    if (mStrokerReusedCache != nullptr)
    {
        free(mStrokerReusedCache);
        mStrokerReusedCache = nullptr;
    }

    mStrokerReusedCacheSize = 0;
}

void WeCCharFontManager::_process(void)
{
    _processCollectedChars(mHasNoMem);
    mCollectedChars.clear();
}

void WeCCharFontManager::_updateTextures()
{
    // process the dirty textures.
    int texCount = mCachedTextureData.size();
    for (int i = 0; i < texCount; ++i)
    {
        auto &iter = mCachedTextureData[i];
        auto tex = mFontTextures[i];
        if (iter->dirty)
        {
            tex->updateWithData(iter->textureCachedData, 0, 0, sAtlasWidth, sAtlasHeight);
            iter->dirty = false;
        }
    }
}

WeCFontFreeTypePtr WeCCharFontManager::getFontFreeType(const std::string &configId)
{
    WeCFontFreeTypePtr fontGenerator = nullptr;

    auto found = mFreeTypeMap.find(configId);
    if (found == mFreeTypeMap.end())
    {
        // Try to create a new generator.
        auto config = mConfigs.find(configId);
        if (config == mConfigs.end())
            return nullptr;

        auto fontName = config->second.fontName;
        int fontSize = config->second.fontSize;
        float outline = config->second.outline;

        if (fontSize <= 0)
            return nullptr;

        if (!FileUtils::getInstance()->isFileExist(fontName))
        {
            // CCLOG("getFontFreeType failed: %s not existed.", fontName.c_str());
            return nullptr;
        }

        fontGenerator = std::make_shared<WeCFontFreeTypeTTF>(fontName, fontSize, outline);

        mFreeTypeMap.emplace(configId, fontGenerator);
    }
    else
    {
        fontGenerator = found->second;
    }

    return fontGenerator;
}

WeCFontFreeTypePtr WeCCharFontManager::createFontFreeType(const std::string &fontName, int fontSize,
    float outline, bool ignoreScale)
{
    WeCFontFreeTypePtr fontGenerator = nullptr;

    std::string configId = generateKey(fontName, fontSize, outline, ignoreScale);

    auto found = mFreeTypeMap.find(configId);
    if (found == mFreeTypeMap.end())
    {
        if (fontSize <= 0)
            return nullptr;

        if (!FileUtils::getInstance()->isFileExist(fontName))
            return nullptr;

        fontGenerator = std::make_shared<WeCFontFreeTypeTTF>(fontName, fontSize, outline, ignoreScale);

        mFreeTypeMap.emplace(configId, fontGenerator);
    }
    else
    {
        fontGenerator = found->second;
    }

    return fontGenerator;
}

std::string WeCCharFontManager::getSystemFontsByCode(unsigned short code)
{
    /*
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

    do
    {
        std::string fontName = "/system/fonts/NotoSansCJK-Regular.ttc";

        auto found = WeCCharFontManager::sfontInfoMap.find(fontName);
        if (found == WeCCharFontManager::sfontInfoMap.end())
            break;

        // Check the code is in the font or not.
        if (found->second->codeSet.count(code) <= 0)
            break;

        return fontName;
    } while (0);

#endif
    */

    // Check the code range.
    // Find in the different fonts by code range.
    // If has not been founded, find in the other font family.
    for (auto &iter : sfontInfoMap)
    {
        if (iter.second->codeSet.count(code) == 0)
        {
            // CCLOG("WeCCharFontManager::getSystemFontsByCode: %s not contain %d", iter.first.c_str(), code);
            continue;
        }

        // CCLOG("WeCCharFontManager::getSystemFontsByCode success %d %s", code, iter.first.c_str());

        return iter.first;
    }

    // CCLOG("WeCCharFontManager::getSystemFontsByCode: could not find code %d", code);
    return "";
}

int WeCCharFontManager::_drawCharacterToAtlas(const std::string &configId, unsigned short code, bool &hasNoMem)
{
    Rect rect;
    int outWidth = 0;
    int outHeight = 0;
    int xAdvance = 0;
    int ascender = 0;
    int lineheight = 0;
    unsigned char* bitmap = nullptr;

    // Try to get the free type generator, if not found, try to create a new one.
    auto fontGenerator = getFontFreeType(configId);

    // Draw the font, to get the bitmap, width and height.
    if (fontGenerator != nullptr)
    {
        bitmap = fontGenerator->getBitmap(code, outWidth, outHeight, rect, xAdvance, ascender);

        // Try to put the font into the memory line.
        lineheight = fontGenerator->getPrintedLineHeight();
    }

    // If no bitmap generated, try to use system font instead.
    do
    {
        // If is blank, skip this operation.
        if (code == ' ')
            break;

        if (bitmap != nullptr)
            break;

        auto config = mConfigs.find(configId);

        if (config == mConfigs.end())
            break;

        // Find the character in the system fonts.
        // Try to select the font in some special font family first.
        std::string systemFont = getSystemFontsByCode(code);

        // CCLOG("WeCCharFontManager::_drawCharacterToAtlas new font: %s", systemFont.c_str());

        if (systemFont.empty())
        {
            // CCLOG("WeCCharFontManager::_drawCharacterToAtlas find second font failed.");
            break;
        }

        // Should insert a new config, will be used by getFontFreeType.
        // These codes are to ugly...
        std::string newConfig = generateKey(systemFont, config->second.fontSize, config->second.outline);
        auto found = mConfigs.find(newConfig);
        if (found == mConfigs.end())
        {
            WeCFontConfig tmp;
            tmp.fontName = systemFont;
            tmp.fontSize = config->second.fontSize;

            // CCLOG("WeCCharFontManager::_drawCharacterToAtlas emplace %s %d", systemFont.c_str(), config->second.fontSize);

            mConfigs.emplace(newConfig, tmp);
        }

        // Get font from the font family config.
        auto fontGenerator2 = getFontFreeType(newConfig);

        if (fontGenerator2 == nullptr)
        {
            // CCLOG("generate second font generator failed, config %s", newConfig.c_str());
            break;
        }

        bitmap = fontGenerator2->getBitmap(code, outWidth, outHeight, rect, xAdvance, ascender);

        // Try to fix the crash.
        // Should reset the line height to the new line height.
        lineheight = fontGenerator2->getPrintedLineHeight();

    } while (0);

    // Fix the bug of the system font, some system font has error here.
    // TODO(eranzhao): reconstruct the system font.
    if (lineheight <= 0)
    {
        // CCLOG("lineheight is zero");
        return 0; // false
    }

    auto line = _getAtlasLine(configId, outWidth, lineheight, hasNoMem);

    if (nullptr == line)
    {
        // CCLOG("WeCCharFontManager::_drawCharacterToAtlas failed, line is not enough.");
        return -1;  // false, -1 means memory is not enough.
    }

    // Draw the bitmap to the specified line.
    // Get the start position.
    int x = line->nextXOffset + 1;
    int y = line->lineOffset * sAtlasLineOffset + 1;

    // Get the cached data.
    auto &textureInfo = mCachedTextureData[line->textureIndex];

    // Copy memory by the position and size.
    if (nullptr != bitmap)
    {
        int tmpWidth = outWidth;
        int tmpHeight = outHeight;

        tmpWidth = (tmpWidth + x) >= sAtlasWidth ? sAtlasWidth - x : tmpWidth;
        tmpHeight = (tmpHeight + y) >= sAtlasHeight ? sAtlasHeight - y : tmpHeight;

        for (int i = 0; i < tmpWidth; ++i)
        {
            for (int j = 0; j < tmpHeight; ++j)
            {
				int outIndex = (y + j) * sAtlasWidth + x + i;
				int inIndex = j * outWidth + i;
				textureInfo->textureCachedData[outIndex] = bitmap[inIndex];
            }
        }

        line->nextXOffset += outWidth + 2;
        textureInfo->dirty = true;
    }

    // Record the information of the char.
    auto charInfo = std::make_shared<WeCChartFontDefinition>();
    charInfo->code = code;
    charInfo->U = x;
    charInfo->V = y;
    charInfo->printWidth = outWidth;
    charInfo->printHeight = outHeight;
    charInfo->printOffsetX = rect.origin.x;
    charInfo->printOffsetY = ascender + rect.origin.y;
    charInfo->printXAdvance = xAdvance;
    charInfo->texIndex = line->textureIndex;
    charInfo->printlineHeight = lineheight;

    auto charListFound = mChars.find(configId);

    if (charListFound == mChars.end())
    {
        // CCLOG("WeCCharFontManager::_drawCharacterToAtlas failed, mChars not find %s", configId.c_str());
        return 0; // false
    }

    auto &list = charListFound->second;
    list.emplace(code, charInfo);

    return 1; // true
}

WeCFontAtlasLineInfoPtr WeCCharFontManager::_getAtlasLine(const std::string &configId, int width, int height, bool &hasNoMem)
{
    if (height <= 0)
        return nullptr;

    int tmpHeight = height / sAtlasLineOffset;
    tmpHeight = height % sAtlasLineOffset > 0 ? tmpHeight + 1 : tmpHeight;
    tmpHeight += 1;

    // Try to get the compatible line by the config.
    WeCFontAtlasLineInfoPtr line = nullptr;

    auto usedLines = mAtlasLineConfigMap.find(configId);
    if (usedLines == mAtlasLineConfigMap.end())
    {
        // Has no data, try to get a line from the unused line, than add it to the map.
        line = _getAtlasLineFromUnusedMemory(configId, height, hasNoMem);
    }
    else
    {
        // Try to find a compatible line.
        auto &lines = usedLines->second;
        for (auto &iter : lines)
        {
            if (iter->nextXOffset + width > sAtlasWidth) // Detect the x offset is enough or not.
                continue;

            if (iter->lineCount != tmpHeight) // Detect the line height is enough or not.
                continue;

            line = iter;
            break;
        }

        if (nullptr == line)
        {
            line = _getAtlasLineFromUnusedMemory(configId, height, hasNoMem);
        }
    }

    return line;
}

WeCFontAtlasLineInfoPtr WeCCharFontManager::_getAtlasLineFromUnusedMemory(const std::string &configId, int height, bool &hasNoMem)
{
    int tmpHeight = height / sAtlasLineOffset;
    tmpHeight = height % sAtlasLineOffset > 0 ? tmpHeight + 1 : tmpHeight;
    tmpHeight += 1;

    do
    {
        if (mCachedTextureData.size() <= 0)
            break;

        int cachedDataCount = static_cast<int>(mCachedTextureData.size());
        for (int t = 0; t < cachedDataCount; ++t)
        {
            if (t > 0)
                hasNoMem = true;    // Memory is not enough, should clear next frame.

            auto &iter = mCachedTextureData[t];

            // Parse the usage of the texture data, if enough to use, set the usage to true.
            int size = iter->usage.size();
            size -= tmpHeight;
            for (int i = 0; i < size; ++i)
            {
                bool unused = true;
                for (int j = i; j < i + tmpHeight; ++j)
                {
                    if (iter->usage[j] == true)
                    {
                        unused = false;
                        break;
                    }
                }

                if (unused)
                {
                    // Create a new line.
                    for (int j = i; j < i + tmpHeight; ++j)
                        iter->usage[j] = true;

                    WeCFontAtlasLineInfoPtr line = std::make_shared<WeCFontAtlasLineInfo>();
                    line->lineCount = tmpHeight;
                    line->lineOffset = i;
                    line->textureIndex = t;

                    // Add it to the map by config id.
                    auto lineInfo = mAtlasLineConfigMap.find(configId);
                    if (lineInfo == mAtlasLineConfigMap.end())
                    {
                        // Add a new one.
                        mAtlasLineConfigMap.emplace(configId, WeCFontAtlasLineInfoList());
                        lineInfo = mAtlasLineConfigMap.find(configId);
                    }

                    lineInfo->second.emplace_back(line);

                    return line;
                }
            }
        }

    } while (0);

    return _getAtlasLineFromNewTexture(configId, height);
}

WeCFontAtlasLineInfoPtr WeCCharFontManager::_getAtlasLineFromNewTexture(const std::string &configId, int height)
{
    if (height <= 0)
        return nullptr;

    int tmpHeight = height / sAtlasLineOffset;
    tmpHeight = height % sAtlasLineOffset > 0 ? tmpHeight + 1 : tmpHeight;
    tmpHeight += 1;

    int sizeInByte = sizeof(unsigned char) * sAtlasWidth * sAtlasHeight;

    // Create a new texture and cached data.
    WeCFontTextureInfoPtr textureInfo = std::make_shared<WeCFontTextureInfo>();
    textureInfo->format = cocos2d::Texture2D::PixelFormat::A8;
    textureInfo->textureCachedData = (unsigned char*)malloc(sizeInByte);
    memset(textureInfo->textureCachedData, 0, sizeInByte);
    textureInfo->usage.resize(sAtlasHeight / sAtlasLineOffset, false);
    mCachedTextureData.emplace_back(textureInfo);

    // Create a new texture.
    auto tex = new Texture2D();
    tex->retain();
    tex->autorelease();
    tex->initWithData(textureInfo->textureCachedData, sizeInByte, textureInfo->format, sAtlasWidth, sAtlasHeight, Size(sAtlasWidth, sAtlasHeight));
    // tex->setAliasTexParameters();
    mFontTextures.emplace_back(tex);

    // Parse the usage of the texture data, if enough to use, set the usage to true.
    int size = textureInfo->usage.size();
    size -= tmpHeight;
    for (int i = 0; i < size; ++i)
    {
        bool unused = true;
        for (int j = i; j < i + tmpHeight; ++j)
        {
            if (textureInfo->usage[j] == true)
            {
                unused = false;
                break;
            }
        }

        if (unused)
        {
            // Create a new line.
            for (int j = i; j < i + tmpHeight; ++j)
                textureInfo->usage[j] = true;

            WeCFontAtlasLineInfoPtr line = std::make_shared<WeCFontAtlasLineInfo>();
            line->lineOffset = i;
            line->lineCount = tmpHeight;
            line->textureIndex = mFontTextures.size() - 1;

            // Add it to the map by config id.
            auto lineInfo = mAtlasLineConfigMap.find(configId);
            if (lineInfo == mAtlasLineConfigMap.end())
            {
                // Add a new one.
                mAtlasLineConfigMap.emplace(configId, WeCFontAtlasLineInfoList());
                lineInfo = mAtlasLineConfigMap.find(configId);
            }

            lineInfo->second.emplace_back(line);

            return line;
        }
    }

    return nullptr;
}

#ifndef CC_STUDIO_ENABLED_VIEW

bool WeCCharFontManager::_parseFontsAndroid(const std::string &path, WeCSystemFontInfoMap & fontInfoMap)
{
    // Scan the font path, collect the font file informations.
    std::vector<std::string> files;
    FileUtils::getInstance()->getFilesInDirectory(path, files, false);

    for (auto &fontFile : files)
    {
        // If is not noto fonts, continue.
        if (fontFile.find("NotoSans") == std::string::npos &&
            fontFile.find("NotoNaskh") == std::string::npos &&
            fontFile.find("DroidSans") == std::string::npos)
        {
            // CCLOG("WeCCharFontManager::_parseFontsAndroid failed because the font is not correct: %s", fontFile.c_str());
            continue;
        }

        // CCLOG("WeCCharFontManager::_parseFontsAndroid: %s", fontFile.c_str());

        // Load char, to get the code.
        auto instance = cocos2d::FontFreeType::getFTLibrary();

        FT_Face face;
        auto result = FT_New_Face(instance, fontFile.c_str(), 0, &face);

        if (0 != result)
        {
            // CCLOG("FT_New_Face failed with error code: %d", result);
            continue;
        }

        // Parse the style of the font.
        if (face->style_flags & FT_STYLE_FLAG_BOLD)
        {
            // CCLOG("FT_Done_Face because the font %s is bold", fontFile.c_str());
            FT_Done_Face(face);
            continue;
        }

        if (face->style_flags & FT_STYLE_FLAG_ITALIC)
        {
            // CCLOG("FT_Done_Face because the font %s is italic", fontFile.c_str());
            FT_Done_Face(face);
            continue;
        }

        WeCSystemFontInfoPtr fontInfo = std::make_shared<WeCSystemFontInfo>();

        fontInfo->name = fontFile;

        FT_UInt glyphIndex;
        FT_ULong charCode = FT_Get_First_Char(face, &glyphIndex);
        while (0 != glyphIndex)
        {
            if (fontInfo->codeSet.find(charCode) == fontInfo->codeSet.end())
                fontInfo->codeSet.emplace(charCode);

            charCode = FT_Get_Next_Char(face, charCode, &glyphIndex);
        }

        // Release the memory of the font.
        FT_Done_Face(face);

        fontInfoMap.emplace(fontInfo->name, fontInfo);
    }

    return true;
}

#endif

#define MAX_FONT_KEY_LEN 1024

std::string WeCCharFontManager::generateKey(const std::string &fontName, int fontSize, float outline, bool ignoreScale)
{
    char key[MAX_FONT_KEY_LEN];
    snprintf(key, MAX_FONT_KEY_LEN, "%s_%d_%f_%d", fontName.c_str(), fontSize, outline, ignoreScale == true ? 1 : 0);
    std::string str(key);
    return str;
}

std::string WeCCharFontManager::printDebugMsg()
{
    std::string str;

    const int maxCharSize = 1024;
    char tmpStr[maxCharSize];

    // Texture informations.
    int totalSize = 0;

    for (auto &info : mAtlasLineConfigMap)
    {
        auto& lines = info.second;

        for (auto &line : lines)
        {
            // float usage = (float)line->nextXOffset / (float)sAtlasWidth;
            auto size = line->nextXOffset * line->lineCount * sAtlasLineOffset;
            totalSize += size;
        }
    }

    snprintf(tmpStr, maxCharSize, "Texture count: %u\n", mFontTextures.size());
    str += tmpStr;

    int textureSize = sAtlasWidth * sAtlasHeight;
    float usage = (float)totalSize / (float)textureSize;

    snprintf(tmpStr, maxCharSize, "\nTotal texture usage: %f\n", usage);
    str += tmpStr;

    auto lastTextureSize = textureSize - totalSize;
    float last = (float)lastTextureSize / 1024.0f / 1024.0f;

    snprintf(tmpStr, maxCharSize, "Free texture size: %f\n", last);
    str += tmpStr;

    for (auto &iter : mConfigs)
    {
        str += "\n";
        str += iter.first + "\n"; // name

        int printFontSize = iter.second.fontSize;
        auto glView = Director::getInstance()->getOpenGLView();
        float scale = WeCCharFontManager::getInstance()->sFontScale;
        printFontSize = std::ceil(iter.second.fontSize * scale);

        snprintf(tmpStr, maxCharSize, "   Font Size: %d\n", iter.second.fontSize);
        str += tmpStr;

        snprintf(tmpStr, maxCharSize, "   Font Print Size: %d\n", printFontSize);
        str += tmpStr;

        // Char information.
        auto found = mChars.find(iter.first);
        if (found != mChars.end())
        {
            snprintf(tmpStr, maxCharSize, "   Total chars:%u\n", found->second.size());
            str += tmpStr;
        }

        // Line formation.
        auto lineFound = mAtlasLineConfigMap.find(iter.first);
        if (lineFound != mAtlasLineConfigMap.end())
        {
            snprintf(tmpStr, maxCharSize, "   Total lines:%u\n", lineFound->second.size());
            str += tmpStr;

            // Print each line information.

            for (size_t i = 0; i < lineFound->second.size(); ++i)
            {
                auto &line = lineFound->second[i];

                float usage = (float)line->nextXOffset / (float)sAtlasWidth;

                snprintf(tmpStr, maxCharSize, "      line %d: usage: %f\n",
                    i, usage);
                str += tmpStr;
            }
        }
    }

    return str;
}

void WeCCharFontManager::emplaceWeCLabel(unsigned int id, cocos2d::ui::WeCLabel *label)
{
    WeCCharFontManager::getInstance()->mWeCLabelMap.emplace(id, label);
}

void WeCCharFontManager::eraseWeCLabel(unsigned int id)
{
    if (WeCCharFontManager::sInstance != nullptr)
    {
        WeCCharFontManager::sInstance->mWeCLabelMap.erase(id);
    }
}

NS_CC_END
