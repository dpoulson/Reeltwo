#ifndef CommandScreenTouchHandler_h
#define CommandScreenTouchHandler_h

#include "ReelTwo.h"
#include "menus/CommandScreen.h"

///////////////////////////////////////////////////////////////////////////////
//
// Command screen handler for SSD 1306 using a rotary dial for events
// 
///////////////////////////////////////////////////////////////////////////////

template <typename COMMAND_DISPLAY>
class CommandScreenTouchDisplay: public CommandScreenHandler
{
public:
    CommandScreenTouchDisplay(COMMAND_DISPLAY& display, bool (*init)(void) = nullptr) :
        fDisplay(display),
        fInitProc(init)
    {
        resetDisplay();
    }

    bool begin()
    {
        return (fInitProc != nullptr) ? fInitProc() : true;
    }

    virtual void wakeDevice() override
    {
        fDisplay.wakeDevice();
    }

    virtual void sleepDevice() override
    {
        fDisplay.sleepDevice();
        invertDisplay(false);
        clearDisplay();
        display();
        switchToScreen(kMainScreen);
    }

    inline void invertDisplay(bool invert)
    {
    #ifdef USE_SMQ
        fInvert = invert;
    #endif
        fDisplay.invertDisplay(invert);
    }

    inline void clearDisplay()
    {
    #ifdef USE_SMQ
        fClearDisplay = true;
    #endif
        fDisplay.clearDisplay();
    }

    inline void setTextSize(int siz)
    {
    #ifdef USE_SMQ
        fTextSize = siz;
    #endif
        fDisplay.setTextSize(siz);
    }

    inline void getTextBounds(const char *string, int16_t x, int16_t y,
                    int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        fDisplay.getTextBounds(string, x, y, x1, y1, w, h);
    }

    void setRotation(uint8_t r)
    {
        fDisplay.setRotation(r);
    }

    void setTextColor(uint16_t c)
    {
        fDisplay.setTextColor(c);
    }

    void setCursor(uint8_t x, uint8_t y)
    {
    #ifdef USE_SMQ
        fX = x;
        fY = y;
    #endif
        fDisplay.setCursor(x, y);
    }

    void print(String text)
    {
    #ifdef USE_SMQ
        fString += text;
    #endif
        fDisplay.print(text);
    }

    void println(unsigned val)
    {
    #ifdef USE_SMQ
        fString += String("\n") + String(val);
    #endif
        fDisplay.println(val);
    }

    void println(String text)
    {
    #ifdef USE_SMQ
        fString += String("\n") + text;
    #endif
        fDisplay.println(text);
    }

    void display()
    {
        fDisplay.display();
    // #ifdef USE_SMQ
    //     if (SMQ::sendTopic("LCD", "Remote"))
    //     {
    //         SMQ::send_uint8("x", fX);
    //         SMQ::send_uint8("y", fY);
    //         SMQ::send_boolean("invert", fInvert);
    //         SMQ::send_boolean("centered", fCentered);
    //         SMQ::send_uint8("size", fTextSize);
    //         SMQ::send_string("text", fString.c_str());
    //         SMQ::sendEnd();
    //         resetDisplay();
    //     }
    // #endif
    }

    void drawTextCentered(String text)
    {
        int16_t x1, y1;
        uint16_t w = 0, h;
        fDisplay.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        fDisplay.setCursor(SCREEN_WIDTH / 2 - w / 2, 0);
        fDisplay.print(text);
    #ifdef USE_SMQ
        fCentered = true;
        fString = text;
    #endif
    }

    void remoteDialEvent(long newValue, long oldValue)
    {
    #ifdef USE_SMQ
        fNewDialValue = newValue;
    #endif
    }

    void remoteButtonEvent(uint8_t id, bool pressed, bool repeat)
    {
    #ifdef USE_SMQ
        fButtonID = id;
        fButtonPressed = pressed;
        fButtonRepeat = repeat;
    #endif
    }

    void remoteActive()
    {
    #ifdef USE_SMQ
        setEnabled(true);
        fRemoteActive = true;
    #endif
    }

    virtual bool handleEvent() override
    {
        bool ret = false;
    #ifdef USE_SMQ
        if (fRemoteActive)
        {
            switchToScreen(kMainScreen);
            fRemoteActive = false;
        }
    #endif
        CommandScreen* currentScr = current();
        if (currentScr != nullptr)
        {
        #ifdef USE_SMQ
            if (fDialValue != fNewDialValue || fButtonID != 0)
            {
                // Reset the screen blanking timer if the encoder changed
                // or if any of the buttons are currently pressed
                ret = true;
            }
            if (fDialValue != fNewDialValue)
            {
                currentScr->buttonDial(fNewDialValue, fDialValue);
                fDialValue = fNewDialValue;
            }
            if (fButtonID)
            {
                if (fButtonID == 1 && fButtonPressed)
                    currentScr->buttonUpPressed();
                if (fButtonID == 2 && fButtonPressed)
                    currentScr->buttonLeftPressed();
                if (fButtonID == 3 && fButtonPressed)
                    currentScr->buttonDownPressed();
                if (fButtonID == 4 && fButtonPressed)
                    currentScr->buttonRightPressed();
                if (fButtonID == 5 && fButtonPressed)
                    currentScr->buttonInPressed();
                if (fLastScreenID == currentScr->ID())
                {
                    if (fButtonID == 1 && !fButtonPressed)
                        currentScr->buttonUpReleased();
                    if (fButtonID == 2 && !fButtonPressed)
                        currentScr->buttonLeftReleased();
                    if (fButtonID == 3 && !fButtonPressed)
                        currentScr->buttonDownReleased();
                    if (fButtonID == 4 && !fButtonPressed)
                        currentScr->buttonRightReleased();
                    if (fButtonID == 5 && !fButtonPressed)
                        currentScr->buttonInReleased();
                }
                fLastScreenID = currentScr->ID();
            }
            else if (fLastScreenID == currentScr->ID())
            {
                if (fButtonID == 1 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonUpPressed(true);
                }
                else if (fButtonID == 2 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonLeftPressed(true);
                }
                else if (fButtonID == 3 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonDownPressed(true);
                }
                else if (fButtonID == 4 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonRightPressed(true);
                }
                else if (fButtonID == 5 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonInPressed(true);
                }
            }
        #endif
        }
    #ifdef USE_SMQ
        fButtonID = 0;
        fButtonPressed = false;
        fButtonRepeat = false;
        fDialValue = fNewDialValue;
    #endif
        return ret;
    }

    COMMAND_DISPLAY&    fDisplay;
    bool                (*fInitProc)(void);
    long                fDialValue = 0;
    uint32_t            fLastKeyEvent = 0;
    ScreenID            fLastScreenID = kInvalid;
    bool                fSkipButtonReleased = false;
#ifdef USE_SMQ
    long        fNewDialValue = 0;
    uint8_t     fButtonID = 0;
    bool        fRemoteActive = false;
    bool        fButtonPressed = false;
    bool        fButtonRepeat = false;
    bool        fInvert;
    bool        fClearDisplay;
    bool        fCentered;
    String      fString;
    uint8_t     fX;
    uint8_t     fY;
    uint8_t     fTextSize;
#endif
    inline void resetDisplay()
    {
    #ifdef USE_SMQ
        fInvert = false;
        fClearDisplay = false;
        fTextSize = 0;
        fCentered = false;
        fString = "";
        fX = 0;
        fY = 0;
    #endif
    }
};
#endif
