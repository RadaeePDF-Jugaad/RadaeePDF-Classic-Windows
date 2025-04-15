#pragma once
#include "PDFCore.h"
namespace RDPDFLib
{
    namespace view
    {
        class CRDVSel
        {
        protected:
            int m_pageno;
            double m_offx;
            double m_offy;
            PDFPage ^m_page;
            int m_index1;
            int m_index2;
            bool m_ok = false;
            bool m_swiped = false;
        public:
            CRDVSel(PDFDoc ^doc, int pageno, double offx, double offy)
            {
                m_pageno = pageno;
                m_page = doc->GetPage(m_pageno);
                m_page->ObjsStart();
                m_offx = offx;
                m_offy = offy;
                m_index1 = -1;
                m_index2 = -1;
                m_ok = false;
                m_swiped = false;
            }
            void Clear()
            {
                if (m_page)
                {
                    m_page->Close();
                    m_page = nullptr;
                }
            }
            PDFRect GetRect1(float scale, float page_height, float orgx, float orgy)
            {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                if (m_swiped)
                    rect = m_page->ObjsGetCharRect(m_index2);
                else
                    rect = m_page->ObjsGetCharRect(m_index1);
                PDFRect rect_draw;
                rect_draw.left = (rect.left * scale) + orgx;
                rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
                rect_draw.right = (rect.right * scale) + orgx;
                rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
                return rect_draw;
            }
            PDFRect GetRect2(float scale, float page_height, float orgx, float orgy)
            {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                if (m_swiped)
                    rect = m_page->ObjsGetCharRect(m_index1);
                else
                    rect = m_page->ObjsGetCharRect(m_index2);
                PDFRect rect_draw;
                rect_draw.left = (rect.left * scale) + orgx;
                rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
                rect_draw.right = (rect.right * scale) + orgx;
                rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
                return rect_draw;
            }
            inline int GetPageNo()
            {
                return m_pageno;
            }
            void SetSel(float x1, float y1, float x2, float y2)
            {
                if (!m_ok)
                {
                    m_page->ObjsStart();
                    m_ok = true;
                }
                m_index1 = m_page->ObjsGetCharIndex(x1, y1);
                m_index2 = m_page->ObjsGetCharIndex(x2, y2);
                if (m_index1 > m_index2)
                {
                    int tmp = m_index1;
                    m_index1 = m_index2;
                    m_index2 = tmp;
                    m_swiped = true;
                }
                else
                    m_swiped = false;
                m_index1 = m_page->ObjsAlignWord(m_index1, -1);
                m_index2 = m_page->ObjsAlignWord(m_index2, 1);
            }
            bool SetSelMarkup(unsigned int color, int type)
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return false;
                return m_page->AddAnnotMarkup(m_index1, m_index2, color, type);
            }
            String ^GetSelString()
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return nullptr;
                return m_page->ObjsGetString(m_index1, m_index2 + 1);
            }
            void DrawSel(IVCallback ^canvas, CRDVPage *page)
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return;
                PDFRect rect;
                PDFRect rect_word;
                PDFRect rect_draw;
                rect = m_page->ObjsGetCharRect(m_index1);
                rect_word = rect;
                int tmp = m_index1 + 1;
                while (tmp <= m_index2)
                {
                    rect = m_page->ObjsGetCharRect(tmp);
                    float gap = (rect.bottom - rect.top) * 0.5f;
                    if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
                        rect_word.right + gap > rect.left&& rect_word.left - gap < rect.right)
                    {
                        if (rect_word.left > rect.left) rect_word.left = rect.left;
                        if (rect_word.right < rect.right) rect_word.right = rect.right;
                    }
                    else
                    {
                        rect_draw.left = page->GetVX(rect_word.left) - m_offx;
                        rect_draw.top = page->GetVY(rect_word.bottom) - m_offy;
                        rect_draw.right = page->GetVX(rect_word.right) - m_offx;
                        rect_draw.bottom = page->GetVY(rect_word.top) - m_offy;
                        canvas->vpDrawSelRect(rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
                        rect_word = rect;
                    }
                    tmp++;
                }
                rect_draw.left = page->GetVX(rect_word.left) - m_offx;
                rect_draw.top = page->GetVY(rect_word.bottom) - m_offy;
                rect_draw.right = page->GetVX(rect_word.right) - m_offx;
                rect_draw.bottom = page->GetVY(rect_word.top) - m_offy;
                canvas->vpDrawSelRect(rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
            }
        };

        public ref class RDVSel sealed
        {
        private:
            int m_pageno;
            double m_offx;
            double m_offy;
            PDFPage^ m_page;
            int m_index1;
            int m_index2;
            bool m_ok = false;
            bool m_swiped = false;
        public:
            RDVSel(PDFDoc^ doc, int pageno, double offx, double offy)
            {
                m_pageno = pageno;
                m_page = doc->GetPage(m_pageno);
                m_page->ObjsStart();
                m_offx = offx;
                m_offy = offy;
                m_index1 = -1;
                m_index2 = -1;
                m_ok = false;
                m_swiped = false;
            }
            void Clear()
            {
                if (m_page)
                {
                    m_page->Close();
                    m_page = nullptr;
                }
            }
            PDFRect GetRect1(float scale, float page_height, float orgx, float orgy)
            {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                if (m_swiped)
                    rect = m_page->ObjsGetCharRect(m_index2);
                else
                    rect = m_page->ObjsGetCharRect(m_index1);
                PDFRect rect_draw;
                rect_draw.left = (rect.left * scale) + orgx;
                rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
                rect_draw.right = (rect.right * scale) + orgx;
                rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
                return rect_draw;
            }
            PDFRect GetRect2(float scale, float page_height, float orgx, float orgy)
            {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                if (m_swiped)
                    rect = m_page->ObjsGetCharRect(m_index1);
                else
                    rect = m_page->ObjsGetCharRect(m_index2);
                PDFRect rect_draw;
                rect_draw.left = (rect.left * scale) + orgx;
                rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
                rect_draw.right = (rect.right * scale) + orgx;
                rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
                return rect_draw;
            }
            inline int GetPageNo()
            {
                return m_pageno;
            }
            PDFPage^ GetPage()
            {
                return m_page;
            }
            void SetSel(float x1, float y1, float x2, float y2)
            {
                if (!m_ok)
                {
                    m_page->ObjsStart();
                    m_ok = true;
                }
                m_index1 = m_page->ObjsGetCharIndex(x1, y1);
                m_index2 = m_page->ObjsGetCharIndex(x2, y2);
                if (m_index1 > m_index2)
                {
                    int tmp = m_index1;
                    m_index1 = m_index2;
                    m_index2 = tmp;
                    m_swiped = true;
                }
                else
                    m_swiped = false;
                m_index1 = m_page->ObjsAlignWord(m_index1, -1);
                m_index2 = m_page->ObjsAlignWord(m_index2, 1);
            }
            bool SetSelMarkup(unsigned int color, int type)
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return false;
                return m_page->AddAnnotMarkup(m_index1, m_index2, color, type);
            }
            String^ GetSelString()
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return nullptr;
                return m_page->ObjsGetString(m_index1, m_index2 + 1);
            }
            void DrawSel(IVCallback^ canvas, long long vpage)
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return;
                CRDVPage* page = (CRDVPage*)vpage;
                PDFRect rect;
                PDFRect rect_word;
                PDFRect rect_draw;
                rect = m_page->ObjsGetCharRect(m_index1);
                rect_word = rect;
                int tmp = m_index1 + 1;
                while (tmp <= m_index2)
                {
                    rect = m_page->ObjsGetCharRect(tmp);
                    float gap = (rect.bottom - rect.top) * 0.5f;
                    if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
                        rect_word.right + gap > rect.left && rect_word.left - gap < rect.right)
                    {
                        if (rect_word.left > rect.left) rect_word.left = rect.left;
                        if (rect_word.right < rect.right) rect_word.right = rect.right;
                    }
                    else
                    {
                        rect_draw.left = page->GetVX(rect_word.left) - m_offx;
                        rect_draw.top = page->GetVY(rect_word.bottom) - m_offy;
                        rect_draw.right = page->GetVX(rect_word.right) - m_offx;
                        rect_draw.bottom = page->GetVY(rect_word.top) - m_offy;
                        canvas->vpDrawSelRect(rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
                        rect_word = rect;
                    }
                    tmp++;
                }
                rect_draw.left = page->GetVX(rect_word.left) - m_offx;
                rect_draw.top = page->GetVY(rect_word.bottom) - m_offy;
                rect_draw.right = page->GetVX(rect_word.right) - m_offx;
                rect_draw.bottom = page->GetVY(rect_word.top) - m_offy;
                canvas->vpDrawSelRect(rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
            }
        };

    }
}
