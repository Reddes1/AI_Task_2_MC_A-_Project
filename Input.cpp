#include "Input.h"

using namespace DirectX::SimpleMath;

bool KeyboardAndMouse::IsPressed(unsigned short vKeyCode) const
{
    assert(vKeyCode < KEYBUFF_SIZE);
    bool pressed = keyBuffer_[vKeyCode] != 0;

    return pressed;
}

bool KeyboardAndMouse::IsKeyReleased(unsigned short vKeyCode) const
{
    assert(vKeyCode < KEYBUFF_SIZE);
    bool isKeyUp = keyUpBuffer_[vKeyCode] != 0;

    return isKeyUp;
}

void KeyboardAndMouse::ClearKeyReleaseArray()
{
    ZeroMemory(keyUpBuffer_, sizeof(keyUpBuffer_));
}

void KeyboardAndMouse::Reset()
{
    ZeroMemory(inBuffer_, sizeof(inBuffer_));
    ZeroMemory(keyBuffer_, sizeof(keyBuffer_));
    ZeroMemory(keyUpBuffer_, sizeof(keyUpBuffer_));
    mouseButtons_[0] = mouseButtons_[1] = mouseButtons_[2] = false;
    mouseScreen_ = mouseMove_ = DirectX::SimpleMath::Vector2(0, 0);
}

void KeyboardAndMouse::ProcessKeys(RAWINPUT* raw)
{
    //standard key code
    unsigned short flags = raw->data.keyboard.Flags;
    unsigned short vkey = raw->data.keyboard.VKey;
    //the scan code might be useful, but we won't use it here
    unsigned short scanCode = raw->data.keyboard.MakeCode;

    //ignore anything larger than this, escape codes, fake keys or weird keys
    if (vkey >= 255)
        return;

    if (vkey == VK_SHIFT)
    {
        // correct left-hand / right-hand SHIFT (VK_LSHIFT / VK_RSHIFT)
        vkey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
    }
    else if (vkey == VK_NUMLOCK)
    {
        // correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
        scanCode = (MapVirtualKey(vkey, MAPVK_VK_TO_VSC) | 0x100);
    }

    // e0 and e1 are escape sequences used for certain special keys, such as PRINT and PAUSE/BREAK.
    // see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
    const bool isE0 = ((flags & RI_KEY_E0) != 0);
    const bool isE1 = ((flags & RI_KEY_E1) != 0);

    if (isE1)
    {
        // for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
        // however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
        if (vkey == VK_PAUSE)
            scanCode = 0x45;
        else
            scanCode = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    }

    switch (vkey)
    {
        // right-hand CONTROL and ALT have their e0 bit set
    case VK_CONTROL:
        if (isE0)
            vkey = VK_RCONTROL; //right control
        else
            vkey = VK_LCONTROL;  //left control
        break;

    case VK_MENU:
        if (isE0)
            vkey = VK_RALT;  //right alt
        else
            vkey = VK_LALT;	//left alt
        break;

        // NUMPAD ENTER has its e0 bit set
    case VK_RETURN:
        if (isE0)
            vkey = VK_NUMPAD_ENTER; //numpad enter
        break;
    }

    //ignore anything larger than this, escape codes, fake keys or weird keys
    if (vkey >= 255)
        return;

    if (flags & RI_KEY_BREAK)
    {
        //key up
        keyBuffer_[vkey] = 0;
        keyUpBuffer_[vkey] = 1; //We just let go of the key, hold on to that
    }
    else
    {
        //key down
        keyBuffer_[vkey] = scanCode;
    }

}

void KeyboardAndMouse::ProcessMouse(RAWINPUT* raw)
{
    unsigned short flags = raw->data.mouse.usButtonFlags;

    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN)
        mouseButtons_[LBUTTON] = true;
    if (flags & RI_MOUSE_LEFT_BUTTON_UP)
        mouseButtons_[LBUTTON] = false;
    if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
        mouseButtons_[MBUTTON] = true;
    if (flags & RI_MOUSE_MIDDLE_BUTTON_UP)
        mouseButtons_[MBUTTON] = false;
    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN)
        mouseButtons_[RBUTTON] = true;
    if (flags & RI_MOUSE_RIGHT_BUTTON_UP)
        mouseButtons_[RBUTTON] = false;

    Vector2 last(mouseScreen_);
    GetMousePosAbsolute(mouseScreen_);
    mouseMove_ = mouseScreen_ - last;
}

void KeyboardAndMouse::GetMousePosAbsolute(Vector2& pos)
{
    POINT mpos;
    if (GetCursorPos(&mpos))
    {
        if (ScreenToClient(hwnd_, &mpos))
        {
            pos = Vector2(static_cast<float>(mpos.x), static_cast<float>(mpos.y));
        }
    }
}

void KeyboardAndMouse::MessageEvent(HRAWINPUT rawInput)
{
    UINT dwSize = 0;

    UINT res = GetRawInputData(rawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    if (res != 0)
        return;//bad input so ignore it

    if (dwSize >= RAWBUFF_SIZE)
    {
        DBOUT("Buffer too small. Is " << RAWBUFF_SIZE << " wants " << dwSize);
        assert(false);
    }

    if (GetRawInputData(rawInput, RID_INPUT, inBuffer_, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
    {
        DBOUT("GetRawInputData failed");
        assert(false);
    }

    RAWINPUT* raw = (RAWINPUT*)inBuffer_;

    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        ProcessKeys(raw);
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        ProcessMouse(raw);
    }
}

void KeyboardAndMouse::GetPressedKeyNames(std::wstring& msg)
{
    msg = L"";
    for (int i = 0; i < KEYBUFF_SIZE; ++i)
    {
        if (keyBuffer_[i] == 0)
            continue;
        //numeric pad and arrow keys conflict, ise0 is set for arrow keys
        //here we just assume the codes refer to arrow keys
        switch (i)
        {
        case VK_RIGHT:
            msg += L"RIGHT ARROW ";
            break;
        case VK_LEFT:
            msg += L"LEFT ARROW ";
            break;
        case VK_UP:
            msg += L"UP ARROW ";
            break;
        case VK_DOWN:
            msg += L"DOWN ARROW ";
            break;
        default:
            wchar_t key[256];
            if (GetKeyNameTextW(keyBuffer_[i] << 16, key, sizeof(key)))
            {
                msg += key;
                msg += L" ";
            }
        }
    }
}

void KeyboardAndMouse::GetMouseNames(std::wstring& msg)
{
    msg = L"";
    static float oldX = 0, oldY = 0;
    if (!Equals(mouseMove_.x, 0))
        oldX = mouseMove_.x;
    if (!Equals(mouseMove_.y, 0))
        oldY = mouseMove_.y;
    std::wstringstream str;
    str.precision(0);
    str << L"x=" << mouseScreen_.x << L" y=" << mouseScreen_.y;
    str << L" xinc=" << oldX << L" yinc=" << oldY;
    if (mouseButtons_[LBUTTON])
        str << L" LButton";
    if (mouseButtons_[MBUTTON])
        str << L" MButton";
    if (mouseButtons_[RBUTTON])
        str << L" RButton";

    msg = str.str();
}

Vector4 KeyboardAndMouse::GetMouseCollider(Vector2 mouseSize)
{
    return Vector4(mouseScreen_.x, mouseScreen_.y, mouseSize.x, mouseSize.y);
}

void KeyboardAndMouse::Initialise(HWND hwnd, bool showMouse, bool confineMouse)
{
    //Get window handle
    hwnd_ = hwnd;

    RAWINPUTDEVICE Rid[2];

    //these ids trigger mouse input
    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x02;
    Rid[0].dwFlags = RIDEV_INPUTSINK;// RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
    Rid[0].hwndTarget = hwnd;

    //these ids trigger keyboard input
    Rid[1].usUsagePage = 0x01;
    Rid[1].usUsage = 0x06;
    Rid[1].dwFlags = RIDEV_INPUTSINK;// RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
    Rid[1].hwndTarget = hwnd;

    // RIDEV_NOLEGACY will stop normal message pump WM_CHAR type messages
    // sometimes we might want that, for now we'll leave it, then we can
    // still use the normal message pump for things like player name entry

    if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
        MessageBox(0, "Cannot get keyboard and mouse input", 0, 0);
        assert(false);
    }

    ShowCursor(showMouse);
    confineMouse_ = confineMouse;
    GetClipCursor(&oldClip_);
    if (confineMouse_)
    {
        GetWindowRect(hwnd, &newClip_);
        ClipCursor(&newClip_);
    }
    GetMousePosAbsolute(mouseScreen_);
}

void KeyboardAndMouse::SetMouseStatus(bool showMouse, bool confineMouse)
{
    ShowCursor(showMouse);
    if (confineMouse)
    {
        GetWindowRect(hwnd_, &newClip_);
        ClipCursor(&newClip_);
    }
}

void KeyboardAndMouse::SetMouseVisible(bool showMouse)
{
    ShowCursor(showMouse);
}

void KeyboardAndMouse::SetMouseWindowConfine(bool confineMouse)
{
    if (confineMouse)
    {
        GetWindowRect(hwnd_, &newClip_);
        ClipCursor(&newClip_);
    }
}

void KeyboardAndMouse::OnLost()
{
    Reset();
    ClipCursor(&oldClip_);
}

void KeyboardAndMouse::OnReset()
{
    Reset();
    ClipCursor(&newClip_);
}

void KeyboardAndMouse::PostProcess()
{
    mouseMove_.x = 0;
    mouseMove_.y = 0;

    ClearKeyReleaseArray();
}

void Gamepads::Update()
{
    for (short i(0); i < XUSER_MAX_COUNT; ++i)
    {
        //Get a reference of the pads state information
        State& s = pads_[i];
        s.port = -1;

        //Capture the last state
        previousStates_[i] = s;

        //Zero the values of the pad
        ZeroMemory(&s.state, sizeof(XINPUT_STATE));

         if (XInputGetState(i, &s.state) == ERROR_SUCCESS)
        {
            //Left Stick
            float normLX = fmaxf(-1, (float)s.state.Gamepad.sThumbLX / STICK_RANGE);
            float normLY = fmaxf(-1, (float)s.state.Gamepad.sThumbLY / STICK_RANGE);

            s.leftStickX = (abs(normLX) < s.deadzoneX ? 0 : (abs(normLX) - s.deadzoneX) * (normLX / abs(normLX)));
            s.leftStickY = (abs(normLY) < s.deadzoneY ? 0 : (abs(normLY) - s.deadzoneY) * (normLY / abs(normLY)));

            //Adjust for deadzone settings
            if (s.deadzoneX > 0)
                s.leftStickX *= 1 / (1 - s.deadzoneX);
            if (s.deadzoneY > 0)
                s.leftStickY *= 1 / (1 - s.deadzoneY);

            //Right Stick
            float normRX = fmaxf(-1, (float)s.state.Gamepad.sThumbRX / STICK_RANGE);
            float normRY = fmaxf(-1, (float)s.state.Gamepad.sThumbRY / STICK_RANGE);

            s.rightStickX = (abs(normRX) < s.deadzoneX ? 0 : (abs(normRX) - s.deadzoneX) * (normRX / abs(normRX)));
            s.rightStickY = (abs(normRY) < s.deadzoneY ? 0 : (abs(normRY) - s.deadzoneY) * (normRY / abs(normRY)));

            //Adjust for deadzone settings
            if (s.deadzoneX > 0)
                s.rightStickX *= 1 / (1 - s.deadzoneX);
            if (s.deadzoneY > 0)
                s.rightStickY *= 1 / (1 - s.deadzoneY);

            s.leftTrigger = (float)s.state.Gamepad.bLeftTrigger / 255;
            s.rightTrigger = (float)s.state.Gamepad.bRightTrigger / 255;

            s.port = i;
        }
    }
}

void Gamepads::Initialise()
{
    for (short i(0); i < XUSER_MAX_COUNT; ++i)
    {
        pads_[i].port = -1;
    }
}

bool Gamepads::IsPressed(int padID, unsigned short buttonID)
{
    assert(padID >= 0 && padID < XUSER_MAX_COUNT);
    if (pads_[padID].port == -1) //If pad isn't connected, jump out
        return false;
    return(pads_[padID].state.Gamepad.wButtons & buttonID) != 0;
}

bool Gamepads::IsButtonDown(int padID, unsigned short buttonID)
{
    if (pads_[padID].port == -1) //If pad isn't connected, jump out
        return false;
    return(previousStates_[padID].state.Gamepad.wButtons & buttonID) != 0;
}

void Gamepads::GetAnalogueNames(int padID, std::wstring& msg)
{
    assert(padID >= 0 && padID < XUSER_MAX_COUNT);
    State& s = pads_[padID];
    msg = L"";
    if (s.port == -1)
        return;

    std::wstringstream str;
    str.precision(2);
    str << L"Left stick(x=" << s.leftStickX << L",y=" << s.leftStickY << L")";
    str << L" Right stick(x=" << s.rightStickX << L",y=" << s.rightStickY << L")";
    str << L" Trigger Left=" << s.leftTrigger << L", right=" << s.rightTrigger;
    msg = str.str();

}

void Gamepads::GetDigitalNames(int padID, std::wstring& msg)
{
    assert(padID >= 0 && padID < XUSER_MAX_COUNT);
    State& s = pads_[padID];
    msg = L"";
    if (s.port == -1)
        return;

    std::wstringstream str;
    if (IsPressed(padID, XINPUT_GAMEPAD_DPAD_UP))
        str << L"DPAD_UP ";
    if (IsPressed(padID, XINPUT_GAMEPAD_DPAD_DOWN))
        str << L"DPAD DOWN ";
    if (IsPressed(padID, XINPUT_GAMEPAD_DPAD_LEFT))
        str << L"DPAD LEFT ";
    if (IsPressed(padID, XINPUT_GAMEPAD_DPAD_RIGHT))
        str << L"DPAD RIGHT ";
    if (IsPressed(padID, XINPUT_GAMEPAD_START))
        str << L"START ";
    if (IsPressed(padID, XINPUT_GAMEPAD_BACK))
        str << L"BACK ";
    if (IsPressed(padID, XINPUT_GAMEPAD_LEFT_THUMB))
        str << L"LEFT THUMB ";
    if (IsPressed(padID, XINPUT_GAMEPAD_RIGHT_THUMB))
        str << L"RIGHT THUMB ";
    if (IsPressed(padID, XINPUT_GAMEPAD_LEFT_SHOULDER))
        str << L"LEFT SHOULDER ";
    if (IsPressed(padID, XINPUT_GAMEPAD_RIGHT_SHOULDER))
        str << L"RIGHT SHOULDER ";
    if (IsPressed(padID, XINPUT_GAMEPAD_A))
        str << L"A ";
    if (IsPressed(padID, XINPUT_GAMEPAD_B))
        str << L"B ";
    if (IsPressed(padID, XINPUT_GAMEPAD_X))
        str << L"X ";
    if (IsPressed(padID, XINPUT_GAMEPAD_Y))
        str << L"Y ";
    msg = str.str();

}

void Gamepads::ClearInputBuffer()
{
    for (short i(0); i < XUSER_MAX_COUNT; ++i)
    {
        State& s = pads_[i];
        ZeroMemory(&s.state, sizeof(XINPUT_STATE));
    }
}


