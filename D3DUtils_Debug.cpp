#pragma once
#include "D3DUtils_Debug.h"

#include <fstream>          //File Loading
#include <comdef.h>         //_com_error, ::Wide..
#include <codecvt>          //For wstring conversion


#include "SpriteBatch.h"    //FPS Screen
#include "SpriteFont.h"     //FPS Screen


using Microsoft::WRL::ComPtr;
using namespace DirectX;

//-------------------------------------------------------------------------------
// FPS Related Functions
//-------------------------------------------------------------------------------

static float debugFrameTimer = 0;
static int debugFrameCount = 0;

void DebuggerFPS(float dTime)
{
    debugFrameCount++;
    debugFrameTimer += dTime;
    if (debugFrameTimer >= 1)
    {
        DBOUT("FPS=" << debugFrameCount);
        debugFrameCount = 0;
        debugFrameTimer -= 1;
    }
}

//Debug info
static float screenDebugFrameTimer = 0;
static int screenDebugFrameCount = 0;
static int screenDebugOldFrame = 0;

//void ScreenDebugFPS(int windowX, int windowY, float dTime, DirectX::SpriteBatch& batch, DirectX::SpriteFont& font)
//{
//    screenDebugFrameCount++;
//    screenDebugFrameTimer += dTime;
//
//    std::wstringstream ss;
//
//    if (screenDebugFrameTimer >= 1)
//    {
//        //Set string and draw
//        ss << L"FPS: " << screenDebugFrameCount << L"\nUpdate Time: " << dTime;
//        font.DrawString(&batch, ss.str().c_str(), DirectX::SimpleMath::Vector2(windowX * 0.001f, windowY * 0.0f),
//            Colours::AliceBlue, 0, DirectX::SimpleMath::Vector2(0, 0), DirectX::SimpleMath::Vector2(0.8f, 0.8f));
//        //Store current FPS
//        screenDebugOldFrame = screenDebugFrameCount;
//        screenDebugFrameCount = 0;
//        //Reset timer
//        screenDebugFrameTimer -= 1;
//    }
//    else
//    {
//        ss << L"FPS: " << screenDebugOldFrame << L"\nUpdate Time: " << dTime;
//        font.DrawString(&batch, ss.str().c_str(), DirectX::SimpleMath::Vector2(windowX * 0.001f, windowY * 0.0f),
//            Colours::AliceBlue, 0, DirectX::SimpleMath::Vector2(0, 0), DirectX::SimpleMath::Vector2(0.8f, 0.8f));
//    }
//
//}




DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}


std::wstring DxException::ToString()const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::string msg = err.ErrorMessage();
    std::wstring newMsg = std::wstring_convert <std::codecvt_utf8<wchar_t>>().from_bytes(msg);

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + newMsg;
}



