// color_button.hpp --- Color Button for Win32
// Copyright (C) 2019 Katayama Hirofumi MZ.
// This file is public domain software.
#ifndef COLOR_BUTTON_HPP_
#define COLOR_BUTTON_HPP_

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif
#include <cassert>

struct COLOR_BUTTON
{
    HWND m_hwndButton;
    COLORREF m_rgbColor;

    COLOR_BUTTON(COLORREF rgbColor = RGB(255, 255, 255))
        : m_hwndButton(NULL)
        , m_rgbColor(rgbColor)
    {
    }

    COLOR_BUTTON(HWND hwndButton, COLORREF rgbColor = RGB(255, 255, 255))
        : m_hwndButton(hwndButton)
        , m_rgbColor(rgbColor)
    {
        assert(GetWindowLong(hwndButton, GWL_STYLE) & BS_OWNERDRAW);
    }

    COLORREF GetColor() const
    {
        return m_rgbColor;
    }

    void SetColor(COLORREF rgbColor)
    {
        m_rgbColor = rgbColor;
        InvalidateRect(m_hwndButton, NULL, TRUE);
    }

    void SetHWND(HWND hwndButton)
    {
        assert(GetWindowLong(hwndButton, GWL_STYLE) & BS_OWNERDRAW);
        m_hwndButton = hwndButton;
    }

    static COLORREF *GetColorTable()
    {
        static COLORREF s_rgbColorTable[] =
        {
            RGB(0, 0, 0),
            RGB(0x33, 0x33, 0x33),
            RGB(0x66, 0x66, 0x66),
            RGB(0x99, 0x99, 0x99),
            RGB(0xCC, 0xCC, 0xCC),
            RGB(0xFF, 0xFF, 0xFF),
            RGB(0xFF, 0xFF, 0xCC),
            RGB(0xFF, 0xCC, 0xFF),
            RGB(0xFF, 0xCC, 0xCC),
            RGB(0xCC, 0xFF, 0xFF),
            RGB(0xCC, 0xFF, 0xCC),
            RGB(0xCC, 0xCC, 0xFF),
            RGB(0xCC, 0xCC, 0xCC),
            RGB(0, 0, 0xCC),
            RGB(0, 0xCC, 0),
            RGB(0xCC, 0, 0),
        };
        return s_rgbColorTable;
    }

    BOOL DoChooseColor()
    {
        CHOOSECOLORW cc;
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = m_hwndButton;
        cc.rgbResult = m_rgbColor;
        cc.lpCustColors = GetColorTable();
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColorW(&cc))
        {
            SetColor(cc.rgbResult);
            return TRUE;
        }
        return FALSE;
    }

    BOOL OnParentDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
    {
        if (m_hwndButton == NULL || m_hwndButton != lpDrawItem->hwndItem)
            return FALSE;

        if (lpDrawItem->CtlType != ODT_BUTTON)
            return FALSE;

        HDC hdc = lpDrawItem->hDC;
        BOOL bSelected = !!(lpDrawItem->itemState & ODS_SELECTED);
        BOOL bFocus = !!(lpDrawItem->itemState & ODS_FOCUS);
        RECT rcItem = lpDrawItem->rcItem;

        ::DrawFrameControl(hdc, &rcItem, DFC_BUTTON,
            DFCS_BUTTONPUSH | (bSelected ? DFCS_PUSHED : 0));

        HBRUSH hbr = ::CreateSolidBrush(m_rgbColor);
        ::InflateRect(&rcItem, -4, -4);
        ::FillRect(hdc, &rcItem, hbr);
        ::DeleteObject(hbr);

        if (bFocus)
        {
            ::InflateRect(&rcItem, 2, 2);
            ::DrawFocusRect(hdc, &rcItem);
        }

        return TRUE;
    }
};

#endif  // ndef COLOR_BUTTON_HPP_
