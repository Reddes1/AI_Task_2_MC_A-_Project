#ifndef INPUT_H
#define INPUT_H

#define WIN32_LEAN_AND_MEAN //Trims the usually unused windows APIs
#include <Windows.h>
#include <Xinput.h>
#include <string>
#include <assert.h>
#include <math.h>
#include <sstream>

#include <bitset>

#include "D3D.h"
#include "D3DUtils.h"

/*ideally we'd create a complete set of our key codes
completely independently of windows, but as I'm lazy
I'll just add a few extra codes to the VK_ list windows
provides in "winuser.h", to make it more consistent.
*/
#define VK_NUMPAD_ENTER 0xF0		//these three are very naughty
#define VK_LALT			0xF1		//they are actually some oem
#define VK_RALT			0xF2		//keys ATTN/FINISH/COPY - but lets pinch them :)


#define VK_0	0x30
#define VK_1	0x31
#define VK_2	0x32
#define VK_3	0x33
#define VK_4	0x34
#define VK_5	0x35
#define VK_6	0x36
#define VK_7	0x37
#define VK_8	0x38
#define VK_9	0x39
//these codes match ascii
#define VK_A	0x41
#define VK_B	0x42
#define VK_C	0x43
#define VK_D	0x44
#define VK_E	0x45
#define VK_F	0x46
#define VK_G	0x47
#define VK_H	0x48
#define VK_I	0x49
#define VK_J	0x4a
#define VK_K	0x4b
#define VK_L	0x4c
#define VK_M	0x4d
#define VK_N	0x4e
#define VK_O	0x4f
#define VK_P	0x50
#define VK_Q	0x51
#define VK_R	0x52
#define VK_S	0x53
#define VK_T	0x54
#define VK_U	0x55
#define VK_V	0x56
#define VK_W	0x57
#define VK_X	0x58
#define VK_Y	0x59
#define VK_Z	0x5a

/*
Mouse and keyboard need to bypass the normal windows messages as we
don't want repeat delays and other user-configurable interference.
This class uses rawinput of keyboard and mouse data
*/

class KeyboardAndMouse
{
public:

    KeyboardAndMouse() { Reset(); }
    ~KeyboardAndMouse() { }

    ///////////////
    /// Setters ///
    ///////////////

    //Set all possible mouse statues
    void SetMouseStatus(bool showMouse, bool confineMouse);
    //Set if the mouse is visible or not
    void SetMouseVisible(bool showMouse);
    //Set if the mouse is to be confined to the window or not
    void SetMouseWindowConfine(bool confineMouse);


    ///////////////
    /// Getters ///
    ///////////////

    //Text list with all keys being currently held down
    void GetPressedKeyNames(std::wstring& msg);
    //Text list with all mouse buttons and cursor position info
    void GetMouseNames(std::wstring& msg);
    //Get a mouse collider value, useful for menu interaction. Take optional rect size.
    DirectX::SimpleMath::Vector4 GetMouseCollider(DirectX::SimpleMath::Vector2 mouseSize = { 2, 2 });

    /*
        Get Relative/Absoulte mouse position. If TRUE, return 2D windows coords of the cursor.
        If FALSE, then return 2D windows coordinates change in position since last up date (push).
    */
    DirectX::SimpleMath::Vector2 GetMousePos(bool absolute) const
    {
        if (absolute)
            return mouseScreen_;
        return mouseMove_;
    }

    //enum check if mouse button is currently down
    typedef enum { LBUTTON = 0, MBUTTON = 1, RBUTTON = 2 } ButtonT;
    bool GetMouseButton(ButtonT b) const { return mouseButtons_[b]; }


    ///////////////////////
    /// Setup/Utilities ///
    ///////////////////////

    /*
        Start up Mouse/Key systems, called once at start.
        Pass in main windows handle, set show mouse, set confine mouse to screen.
    */
    void Initialise(HWND hwnd, bool showMouse = true, bool confineMouse = false);
    /*
    Add this to the windows message pump. rawInput comes form lparam after a WM_INPUT message.
    case WM_INPUT:
        inpu.MessageEvent((HRAWINPUT)lParam);
    */
    void MessageEvent(HRAWINPUT rawInput);
    //Zero everything so we can start again
    void Reset();
    //Call this after game is finished using the mouse, like after rendering
    void PostProcess();
    //Optional resetting of mouse position if mouse lost or after events (e.g window<->fullscreen)
    void OnLost();
    void OnReset();
    //Set manual mouse position (useful when changing state maybe).
    void SetMousePosition(float windowX, float windowY) { mouseScreen_ = { windowX, windowY }; }


    /////////////////
    /// Processes ///
    /////////////////

    //Check if specific key is pressed
    bool IsPressed(unsigned short vKeyCode) const;
    //Is the key released?
    bool IsKeyReleased(unsigned short vKeyCode) const;
    //Clear the Key Release Array
    void ClearKeyReleaseArray();


private:
    //main window handle
    HWND hwnd_;
    //Define values for buffers, max buttons
    enum { RAWBUFF_SIZE = 512, KEYBUFF_SIZE = 255, KMASK_IS_DOWN = 1, MAX_BUTTONS = 3 };
    //Raw input buffer
    BYTE inBuffer_[RAWBUFF_SIZE];
    //Virtual key codes, but standard ones (first 256)
    unsigned short keyBuffer_[KEYBUFF_SIZE];
    //Key up array
    unsigned short keyUpBuffer_[KEYBUFF_SIZE];
    //Support for 3 mouse buttons (L, R, Middle)
    bool mouseButtons_[MAX_BUTTONS];
    //Track mouse position changes
    DirectX::SimpleMath::Vector2 mouseScreen_, mouseMove_;
    //If the mouse is confined, then track the size of the window
    bool confineMouse_;
    RECT newClip_;  //New area for ClipCursor, i.e current window size
    RECT oldClip_;  //Previous area for ClipCursor

    void ProcessKeys(RAWINPUT* raw);
    void ProcessMouse(RAWINPUT* raw);
    void GetMousePosAbsolute(DirectX::SimpleMath::Vector2& pos);
};


//Simple input handler, currently only supports 1 pad but scalable.

class Gamepads
{
public:

    //Gamepad Data, one used per pad
    struct State
    {
        int port = -1;  //controller ID
        float leftStickX = 0;
        float leftStickY = 0;
        float rightStickX = 0;
        float rightStickY = 0;
        float leftTrigger = 0;
        float rightTrigger = 0;
        float deadzoneX = 0.1f;
        float deadzoneY = 0.1f;
        bool inputEnabled_ = true;
        XINPUT_STATE state;
    };

    ///////////////////////
    /// Setup/Utilities ///
    ///////////////////////

    //Call once at startup
    void Initialise();

    //NEEDS TESTING, presumably cleared the input buffer of any input, usefull from going from one state to another
    void ClearInputBuffer();


    ///////////////
    /// Setters ///
    ///////////////

    //Mechanical devices can provide input when not being touched (stick drift), due to age or damage.
    //Deadzone will specify a range of input values to ignore.
    void SetDeadZone(int padID, float x, float y)
    {
        //Check that pad ID is valid
        assert(padID >= 0 && padID < XUSER_MAX_COUNT);
        pads_[padID].deadzoneX = x;
        pads_[padID].deadzoneY = y;
    }


    ///////////////
    /// Getters ///
    ///////////////

        //Get the state of a single pad
    const State& GetState(int padID) const
    {
        //Check that pad ID is valid
        assert(padID >= 0 && padID < XUSER_MAX_COUNT);
        return pads_[padID];
    }

    //Get a list of pad activity - motion of sticks and triggers
    void GetAnalogueNames(int padID, std::wstring& msg);

    //Get a list of pad activity - button presses
    void GetDigitalNames(int padID, std::wstring& msg);


    /////////////////
    /// Processes ///
    /////////////////

    //Called every update
    void Update();

    //Is a key pressed?
    bool IsPressed(int padID, unsigned short buttonID);

    //Check to see if pad is still plugged in
    bool IsConnected(int padID)
    {
        //Check that pad ID is valid
        assert(padID >= 0 && padID < XUSER_MAX_COUNT);
        return pads_[padID].port != -1;
    }

    //Use previous state data to check if the button is still down between frames
    bool IsButtonDown(int padID, unsigned short buttonID);


private:
    //a copy of state for each of 4 pads
    State pads_[XUSER_MAX_COUNT];
    //Hold the previous state of each pad
    State previousStates_[XUSER_MAX_COUNT];
    //Stick range
    const int STICK_RANGE = 32767;
};

#endif
