#pragma once
#include "PDFCore.h"
#include "RDVPage.h"
#include "RDVFinder.h"
#include "RDVThread.h"
#include "RDVCallback.h"
using namespace RDPDFLib::pdf;
namespace RDPDFLib
{
	namespace view
	{
        public value struct PDFPos
        {
            double x;
            double y;
            int pageno;
        };
        class CRDVLayout
		{
        public:
            CRDVLayout()
            {
                Windows::Graphics::Display::DisplayInformation^ disp = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
                m_dpi = disp->LogicalDpi;
                m_doc = nullptr;
                m_scale = -1;
                m_layw = 0;
                m_layh = 0;
                m_vx = 0;
                m_vy = 0;
                m_vw = 0;
                m_vh = 0;
                m_page_gap = 4;
                m_pageno1 = -1;
                m_pageno2 = -1;
                m_callback = nullptr;
                m_pages = NULL;
                m_pages_cnt = 0;
                m_auto_fit = false;
                m_closing = false;

                if (CRDVBlk::m_cell_size <= 0)
                {
                    Windows::Graphics::Display::DisplayInformation^ disp = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
                    int width = disp->ScreenWidthInRawPixels;// GetSystemMetrics(SM_CXSCREEN);
                    int height = disp->ScreenHeightInRawPixels;// GetSystemMetrics(SM_CYSCREEN);

                    CRDVBlk::m_cell_size = width;
                    if (CRDVBlk::m_cell_size > height) CRDVBlk::m_cell_size = height;
                    if (CRDVBlk::m_cell_size > 2048) CRDVBlk::m_cell_size = 2048;
                    else if (CRDVBlk::m_cell_size > 1024) CRDVBlk::m_cell_size = 1024;
                    else CRDVBlk::m_cell_size = 512;
                }
            }
            virtual ~CRDVLayout()
            {
                vClose();
            }
            void vOpen(PDFDoc ^doc, IVCallback ^callback, int page_gap)
            {
                m_doc = doc;
                m_page_gap = page_gap;
                m_callback = callback;
                m_closing = false;
                if (callback)
                {
                    DispatchedHandler^ render = ref new DispatchedHandler([this]() {
                        if (m_closing) return;
                        vDraw();
                        });
                    DispatchedHandler ^finder = ref new DispatchedHandler([this]() {
                        if (m_closing) return;
                        if (m_finder.find_get_page() >= 0)
                        {
                            vFindGoto();
                            if (m_callback) m_callback->vpOnFound(true);
                        }
                        else
                            if (m_callback) m_callback->vpOnFound(false);
                    });
                    DispatchedHandler^ destroy = ref new DispatchedHandler([this]() {
                        delete this;//call by backing thread.
                    });
                    m_thread.set_callback(callback->vpGetDisp(), render, finder, destroy);
                }

                m_thread.start();
                m_pages_cnt = m_doc->PageCount;
                m_pages = new CRDVPage[m_pages_cnt];
                for (int pcur = 0; pcur < m_pages_cnt; pcur++)
                    m_pages[pcur].init(m_doc, pcur);
            }
            void vClose()
            {
                if (m_closing) return;
                m_closing = true;
                if (m_thread.is_run())
                {
                    CRDVPage* cur = m_pages;
                    CRDVPage* end = cur + m_pages_cnt;
                    while (cur < end)
                    {
                        cur->ui_end_zoom(m_callback, m_thread);
                        cur->ui_end(m_callback, m_thread);
                        cur++;
                    }
                    m_thread.destroy();
                }
                if (m_pages)
                {
                    delete[]m_pages;
                    m_pages = NULL;
                    m_pages_cnt = 0;
                }
                m_doc = nullptr;
                m_callback == nullptr;
                m_scale = -1;
                m_layw = 0;
                m_layh = 0;
                m_vw = 0;
                m_vh = 0;
                m_pageno1 = -1;
                m_pageno2 = -1;
            }
            virtual void vLayout(double scale) = 0;
            virtual int vGetPage(double vx, double vy) = 0;
            inline void vSetPos(double vx, double vy, const PDFPos& pos)
            {
                if (m_vw <=0 || m_vh <= 0 || pos.pageno < 0) return;
                CRDVPage* gpage = m_pages + pos.pageno;
                vSetX(gpage->GetVX(pos.x) - vx);
                vSetY(gpage->GetVY(pos.y) - vy);
            }
            inline void vGotoPage(int pageno)
            {
                if (!m_pages || pageno < 0 || pageno >= m_pages_cnt) return;
                CRDVPage* page = m_pages + pageno;
                float x = page->GetLeft() - (m_page_gap >> 1);
                float y = page->GetTop() - (m_page_gap >> 1);
                if (x > m_layw - m_vw) x = m_layw - m_vw;
                if (x < 0) x = 0;
                if (y > m_layh - m_vh) y = m_layh - m_vh;
                if (y < 0) y = 0;
                m_vx = x;
                m_vy = y;
            }
            inline PDFPos vGetPos(double vx, double vy)
            {
                PDFPos pos;
                pos.pageno = -1;
                pos.x = 0;
                pos.y = 0;
                int pgno = vGetPage(vx, vy);
                if (pgno < 0 || pgno >= m_pages_cnt) return pos;
                CRDVPage* gpage = m_pages + pgno;
                pos.pageno = pgno;
                pos.x = gpage->GetPDFX(vGetX() + vx);
                pos.y = gpage->GetPDFY(vGetY() + vy);
                return pos;
            }

            inline void vResize(double cx, double cy)
            {
                if (cx <= 0 || cy <= 0) return;
                if (cx == m_vw && cy == m_vh) return;
                m_vw = cx;
                m_vh = cy;
                vLayout(m_scale);
            }
            inline void vZoomStart()
            {
                if (m_pageno1 < 0 || m_pageno2 < 0) return;
                CRDVPage* cur = m_pages;
                CRDVPage* end = cur + m_pages_cnt;
                while (cur < end)
                {
                    cur->ui_zoom_start(m_callback, m_thread);
                    cur++;
                }
            }
            inline void vZoomConfirm()
            {
                for (int cur = 0; cur < m_pages_cnt; cur++)
                {
                    if (cur < m_pageno1 || cur >= m_pageno2)
                        m_pages[cur].ui_end_zoom(m_callback, m_thread);
                    m_pages[cur].ui_reset(m_callback, m_thread);
                }
                m_auto_fit = false;
            }
            inline void vZoomSet(double zoom)
            {
                if (!m_pages || m_vw <= 0 || m_vh <= 0) return;
                zoom = zoom * m_dpi / 72;
                if (m_scale != zoom)
                    vLayout(zoom);
            }
            virtual void vDraw()
            {
                if (!m_doc) return;
                v_flush_range();
                int vx = vGetX();
                int vy = vGetY();
                CRDVPage* cur = m_pages + m_pageno1;
                CRDVPage* end = m_pages + m_pageno2;
                while (cur < end)
                {
                    cur->ui_draw(m_callback, m_thread, vx, vy, m_vw, m_vh);
                    cur++;
                }
            }
            inline void vDrawFind(double offx, double offy)
            {
                int pfound = m_finder.find_get_page();
                if (pfound >= m_pageno1 && pfound < m_pageno2)
                    m_finder.find_draw(m_callback, m_pages + pfound, offx, offy);
            }
            inline double vGetX()
            {
                int x = m_vx;
                if (x > m_layw - m_vw) x = m_layw - m_vw;
                if (x < 0) x = 0;
                return x;
            }
            inline void vSetX(double x)
            {
                if (x > m_layw - m_vw) x = m_layw - m_vw;
                if (x < 0) x = 0;
                m_vx = x;
            }
            inline double vGetY()
            {
                int y = m_vy;
                if (y > m_layh - m_vh) y = m_layh - m_vh;
                if (y < 0) y = 0;
                return y;
            }
            inline void vSetY(double y)
            {
                if (y > m_layh - m_vh) y = m_layh - m_vh;
                if (y < 0) y = 0;
                m_vy = y;
            }
            inline bool vGetAutoFit()
            {
                return m_auto_fit;
            }
            inline void vSetAutoFit(bool val)
            {
                m_auto_fit = val;
            }
            inline double vGetScale()
            {
                return m_scale * 72 / m_dpi;
            }
            inline CRDVPage* vGetPage(int pageno)
            {
                if (pageno < 0 || pageno >= m_pages_cnt) return NULL;
                return m_pages + pageno;
            }
            inline void vFindStart(String ^key, bool match_case, bool whole_word)
            {
                if (!m_pages) return;
                int pageno = vGetPage(0, 0);
                m_finder.find_end();
                m_finder.find_start(m_doc, pageno, key, match_case, whole_word);
            }
            inline void vFindGoto()
            {
                if (!m_pages) return;
                int pg = m_finder.find_get_page();
                if (pg < 0 || pg >= m_doc->PageCount) return;
                int x = vGetX();
                int y = vGetY();
                PDFRect pos = m_finder.find_get_pos();
                if (pos.top >= pos.bottom) return;
                CRDVPage* page = m_pages + pg;
                pos.left = page->GetVX(pos.left);
                pos.top = page->GetVY(pos.top);
                pos.right = page->GetVX(pos.right);
                pos.bottom = page->GetVY(pos.bottom);
                if (x > pos.left - m_vw / 8) x = pos.left - m_vw / 8;
                if (x < pos.right - m_vw * 7 / 8) x = pos.right - m_vw * 7 / 8;
                if (y > pos.top - m_vh / 8) y = pos.top - m_vh / 8;
                if (y < pos.bottom - m_vh * 7 / 8) y = pos.bottom - m_vh * 7 / 8;
                if (x > m_layw - m_vw) x = m_layw - m_vw;
                if (x < 0) x = 0;
                if (y > m_layh - m_vh) y = m_layh - m_vh;
                if (y < 0) y = 0;
                m_vx = x;
                m_vy = y;
            }
            inline int vFind(int dir)
            {
                if (!m_pages) return -1;
                int ret = m_finder.find_prepare(dir);
                if (ret == 1)
                {
                    if (m_callback)
                        m_callback->vpOnFound(true);
                    vFindGoto();
                    return 0;//succeeded
                }
                if (ret == 0)
                {
                    if (m_callback)
                        m_callback->vpOnFound(false);
                    return -1;//failed
                }
                m_thread.find_start(&m_finder);//need thread operation.
                return 1;
            }
            inline void vFindEnd() {
                if (!m_pages) return;
                m_finder.find_end();
            }
            inline bool vHasFind()
            {
                if (!m_pages) return false;
                int pageno0 = m_finder.find_get_page();
                return (pageno0 >= m_pageno1 && pageno0 < m_pageno2);
            }
            inline double vGetLayW() { return m_layw; }
            inline double vGetLayH() { return m_layh; }
            inline double vGetVW() { return m_vw; }
            inline double vGetVH() { return m_vh; }
        protected:
            virtual void v_flush_range()
            {
                int pageno1 = vGetPage(-CRDVBlk::m_cell_size, -CRDVBlk::m_cell_size);
                int pageno2 = vGetPage(m_vw + CRDVBlk::m_cell_size, m_vh + CRDVBlk::m_cell_size);
                if (pageno1 >= 0 && pageno2 >= 0)
                {
                    if (pageno1 > pageno2)
                    {
                        int tmp = pageno1;
                        pageno1 = pageno2;
                        pageno2 = tmp;
                    }
                    pageno2++;
                    if (m_pageno1 < pageno1)
                    {
                        int start = m_pageno1;
                        int end = pageno1;
                        if (end > m_pageno2) end = m_pageno2;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                    if (m_pageno2 > pageno2)
                    {
                        int start = pageno2;
                        int end = m_pageno2;
                        if (start < m_pageno1) start = m_pageno1;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                }
                else
                {
                    int start = m_pageno1;
                    int end = m_pageno2;
                    while (start < end)
                    {
                        CRDVPage* vpage = m_pages + start;
                        vpage->ui_end_zoom(m_callback, m_thread);
                        vpage->ui_reset(m_callback, m_thread);
                        start++;
                    }
                }
                m_pageno1 = pageno1;
                m_pageno2 = pageno2;
            }
            IVCallback ^m_callback;
            PDFDoc ^m_doc;
            CRDVPage* m_pages;
            CRDVFinder m_finder;
            int m_pages_cnt;
            int m_page_gap;
            double m_scale;
            double m_vx;
            double m_vy;
            double m_vw;
            double m_vh;
            double m_layw;
            double m_layh;

            int m_pageno1;
            int m_pageno2;
            double m_dpi;
            bool m_auto_fit;
            bool m_closing;
            CRDVThread m_thread;
        };
		class CRDVLayoutVert : public CRDVLayout
		{
        public:
            enum ALIGN
            {
                ALIGN_CENTER = 0,
                ALIGN_LEFT = 1,
                ALIGN_RIGHT = 2,
            };
            ALIGN m_align;
            bool m_same_width;
            CRDVLayoutVert(ALIGN align, bool same_width) : CRDVLayout()
            {
                m_same_width = same_width;
                m_align = align;
            }
            virtual int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vy += vGetY();
                if (vy < 0) return 0;
                if (vy > m_layh) return m_pages_cnt - 1;

                int pl = 0;
                int pr = m_pages_cnt - 1;
                int hg = (m_page_gap >> 1);
                while (pr >= pl)
                {
                    int mid = (pl + pr) >> 1;
                    CRDVPage* pmid = m_pages + mid;
                    if (vy < pmid->GetTop() - hg)
                        pr = mid - 1;
                    else if (vy >= pmid->GetBottom() + hg)
                        pl = mid + 1;
                    else return mid;
                }
                return (pr < 0) ? 0 : pr;
            }

            virtual void vLayout(double scale)
            {
                if (m_vw <= 0 || m_vh <= 0) return;
                PDFPoint size = m_doc->MaxPageSize;
                if (m_auto_fit) scale = m_vw / (size.x + m_page_gap);
                if (scale > 0 || !m_callback)
                    m_scale = scale;
                else
                    m_scale = m_dpi / 72.0f;
                m_layw = (size.x + m_page_gap) * m_scale;
                //if (m_layw < m_vw) m_layw = m_vw;
                double y = m_scale * (m_page_gap >> 1);
                for (int pcur = 0; pcur < m_pages_cnt; pcur++)
                {
                    double x;
                    float pg_scale = m_scale;
                    float pg_width = m_doc->GetPageWidth(pcur);
                    if (m_same_width)
                        pg_scale = m_scale * size.x / pg_width;
                    switch (m_align)
                    {
                    case ALIGN_LEFT:
                        x = m_scale * (m_page_gap >> 1);
                        break;
                    case ALIGN_RIGHT:
                        x = m_layw - m_scale * (m_page_gap >> 1);
                        break;
                    default:
                        x = (m_layw - pg_width * pg_scale) * 0.5f;
                        break;
                    }
                    CRDVPage* page = m_pages + pcur;
                    if (page->GetScale() != pg_scale)
                    {
                        page->ui_end(m_callback, m_thread);
                        page->ui_layout(x, y, pg_scale);
                    }
                    else
                    {
                        page->ui_layout(x, y, pg_scale);
                    }
                    y += (pg_scale * m_doc->GetPageHeight(pcur)) + m_scale * m_page_gap;
                }
                m_layh = y - m_scale * (m_page_gap >> 1);
            }
        };
        class CRDVLayoutHorz : public CRDVLayout
        {
        public:
            enum ALIGN
            {
                ALIGN_CENTER = 0,
                ALIGN_TOP = 1,
                ALIGN_BOTTOM = 2,
            };
            ALIGN m_align;
            bool m_same_height;
            CRDVLayoutHorz(ALIGN align, bool same_height) : CRDVLayout()
            {
                m_same_height = same_height;
                m_align = align;
            }
            virtual int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vx += vGetX();
                if (vx < 0) return 0;
                if (vx > m_layw) return m_pages_cnt - 1;

                int pl = 0;
                int pr = m_pages_cnt - 1;
                int hg = (m_page_gap >> 1);
                while (pr >= pl)
                {
                    int mid = (pl + pr) >> 1;
                    CRDVPage* pmid = m_pages + mid;
                    if (vx < pmid->GetLeft() - hg)
                        pr = mid - 1;
                    else if (vx >= pmid->GetRight() + hg)
                        pl = mid + 1;
                    else return mid;
                }
                return (pr < 0) ? 0 : pr;
            }

            virtual void vLayout(double scale)
            {
                if (m_vw <= 0 || m_vh <= 0) return;
                PDFPoint size = m_doc->MaxPageSize;
                if (m_auto_fit) scale = m_vh / (size.y + m_page_gap);
                if (scale > 0 || !m_callback)
                    m_scale = scale;
                else
                    m_scale = m_dpi / 72.0f;
                m_layh = (size.y + m_page_gap) * m_scale;
                //if (m_layw < m_vw) m_layw = m_vw;
                double x = m_scale * (m_page_gap >> 1);
                for (int pcur = 0; pcur < m_pages_cnt; pcur++)
                {
                    double y;
                    float pg_scale = m_scale;
                    float pg_height = m_doc->GetPageHeight(pcur);
                    if (m_same_height)
                        pg_scale = m_scale * size.y / pg_height;
                    switch (m_align)
                    {
                    case ALIGN_TOP:
                        y = m_scale * (m_page_gap >> 1);
                        break;
                    case ALIGN_BOTTOM:
                        y = m_layh - m_scale * (m_page_gap >> 1);
                        break;
                    default:
                        y = (m_layh - pg_height * pg_scale) * 0.5f;
                        break;
                    }
                    CRDVPage* page = m_pages + pcur;
                    if (page->GetScale() != pg_scale)
                    {
                        page->ui_end(m_callback, m_thread);
                        page->ui_layout(x, y, pg_scale);
                    }
                    else
                    {
                        page->ui_layout(x, y, pg_scale);
                    }
                    x += (pg_scale * m_doc->GetPageWidth(pcur)) + m_scale * m_page_gap;
                }
                m_layw = x - m_scale * (m_page_gap >> 1);
            }
        };
        class CRDVLayoutThumbH : public CRDVLayout
        {
        public:
            enum ALIGN
            {
                ALIGN_CENTER = 0,
                ALIGN_TOP = 1,
                ALIGN_BOTTOM = 2,
            };
            ALIGN m_align;
            bool m_same_height;
            CRDVLayoutThumbH(ALIGN align, bool same_height) : CRDVLayout()
            {
                m_same_height = same_height;
                m_align = align;
            }
            virtual int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vx += vGetX();
                if (vx < 0) return 0;
                if (vx > m_layw) return m_pages_cnt - 1;

                int pl = 0;
                int pr = m_pages_cnt - 1;
                int hg = (m_page_gap >> 1);
                while (pr >= pl)
                {
                    int mid = (pl + pr) >> 1;
                    CRDVPage* pmid = m_pages + mid;
                    if (vx < pmid->GetLeft() - hg)
                        pr = mid - 1;
                    else if (vx >= pmid->GetRight() + hg)
                        pl = mid + 1;
                    else return mid;
                }
                return (pr < 0) ? 0 : pr;
            }

            virtual void vLayout(double scale)
            {
                if (m_vw <= 0 || m_vh <= 0)
                    return;
                PDFPoint size = m_doc->MaxPageSize;
                m_scale = (m_vh - m_page_gap) / size.y;
                m_layh = m_vh;
                double x = m_vw * 0.5;
                for (int pcur = 0; pcur < m_pages_cnt; pcur++)
                {
                    double y;
                    float pg_scale = m_scale;
                    float pg_height = m_doc->GetPageHeight(pcur);
                    if (m_same_height)
                        pg_scale = m_scale * size.y / pg_height;
                    switch (m_align)
                    {
                    case ALIGN_TOP:
                        y = m_scale * (m_page_gap >> 1);
                        break;
                    case ALIGN_BOTTOM:
                        y = m_layh - m_scale * (m_page_gap >> 1);
                        break;
                    default:
                        y = (m_layh - pg_height * pg_scale) * 0.5f;
                        break;
                    }
                    CRDVPage* page = m_pages + pcur;
                    //if (page->GetScale() != pg_scale)
                    {
                        page->ui_end(m_callback, m_thread);
                        page->ui_layout(x, y, pg_scale);
                    }
                    //else
                    //{
                    //    page->ui_layout(x, y, pg_scale);
                    //}
                    x += (pg_scale * m_doc->GetPageWidth(pcur)) + m_scale * m_page_gap;
                }
                m_layw = x - m_scale * (m_page_gap >> 1);
                m_layw += m_vw * 0.5;
            }
            virtual void v_flush_range()
            {
                int pageno1 = vGetPage(0, 0);
                int pageno2 = vGetPage(m_vw, m_vh);
                if (pageno1 >= 0 && pageno2 >= 0)
                {
                    if (pageno1 > pageno2)
                    {
                        int tmp = pageno1;
                        pageno1 = pageno2;
                        pageno2 = tmp;
                    }
                    pageno2++;
                    if (m_pageno1 < pageno1)
                    {
                        int start = m_pageno1;
                        int end = pageno1;
                        if (end > m_pageno2) end = m_pageno2;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_end_pno(m_callback);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                    if (m_pageno2 > pageno2)
                    {
                        int start = pageno2;
                        int end = m_pageno2;
                        if (start < m_pageno1) start = m_pageno1;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_end_pno(m_callback);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                }
                else
                {
                    int start = m_pageno1;
                    int end = m_pageno2;
                    while (start < end)
                    {
                        CRDVPage* vpage = m_pages + start;
                        vpage->ui_end_zoom(m_callback, m_thread);
                        vpage->ui_end_pno(m_callback);
                        vpage->ui_reset(m_callback, m_thread);
                        start++;
                    }
                }
                m_pageno1 = pageno1;
                m_pageno2 = pageno2;
            }
            virtual void vDraw()
            {
                if (!m_doc) return;
                v_flush_range();
                int vx = vGetX();
                int vy = vGetY();
                CRDVPage* cur = m_pages + m_pageno1;
                CRDVPage* end = m_pages + m_pageno2;
                while (cur < end)
                {
                    cur->ui_draw(m_callback, m_thread, vx, vy, m_vw, m_vh);
                    cur->ui_draw_pno(m_callback);
                    cur++;
                }
            }
        };
        class CRDVLayoutThumbV : public CRDVLayout
        {
        public:
            enum ALIGN
            {
                ALIGN_CENTER = 0,
                ALIGN_LEFT = 1,
                ALIGN_RIGHT = 2,
            };
            ALIGN m_align;
            bool m_same_width;
            CRDVLayoutThumbV(ALIGN align, bool same_width) : CRDVLayout()
            {
                m_same_width = same_width;
                m_align = align;
            }
            virtual void v_flush_range()
            {
                int pageno1 = vGetPage(0, 0);
                int pageno2 = vGetPage(m_vw, m_vh);
                if (pageno1 >= 0 && pageno2 >= 0)
                {
                    if (pageno1 > pageno2)
                    {
                        int tmp = pageno1;
                        pageno1 = pageno2;
                        pageno2 = tmp;
                    }
                    pageno2++;
                    if (m_pageno1 < pageno1)
                    {
                        int start = m_pageno1;
                        int end = pageno1;
                        if (end > m_pageno2) end = m_pageno2;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_end_pno(m_callback);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                    if (m_pageno2 > pageno2)
                    {
                        int start = pageno2;
                        int end = m_pageno2;
                        if (start < m_pageno1) start = m_pageno1;
                        while (start < end)
                        {
                            CRDVPage* vpage = m_pages + start;
                            vpage->ui_end_zoom(m_callback, m_thread);
                            vpage->ui_end_pno(m_callback);
                            vpage->ui_reset(m_callback, m_thread);
                            start++;
                        }
                    }
                }
                else
                {
                    int start = m_pageno1;
                    int end = m_pageno2;
                    while (start < end)
                    {
                        CRDVPage* vpage = m_pages + start;
                        vpage->ui_end_zoom(m_callback, m_thread);
                        vpage->ui_end_pno(m_callback);
                        vpage->ui_reset(m_callback, m_thread);
                        start++;
                    }
                }
                m_pageno1 = pageno1;
                m_pageno2 = pageno2;
            }
            virtual int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vy += vGetY();
                if (vy < 0) return 0;
                if (vy > m_layh) return m_pages_cnt - 1;

                int pl = 0;
                int pr = m_pages_cnt - 1;
                int hg = (m_page_gap >> 1);
                while (pr >= pl)
                {
                    int mid = (pl + pr) >> 1;
                    CRDVPage* pmid = m_pages + mid;
                    if (vy < pmid->GetTop() - hg)
                        pr = mid - 1;
                    else if (vy >= pmid->GetBottom() + hg)
                        pl = mid + 1;
                    else return mid;
                }
                return (pr < 0) ? 0 : pr;
            }

            virtual void vLayout(double scale)
            {
                if (m_vw <= 0 || m_vh <= 0) return;
                PDFPoint size = m_doc->MaxPageSize;
                m_scale = (m_vw - m_page_gap) / size.x;
                m_layw = (size.x + m_page_gap) * m_scale;
                double y = m_vh * 0.5;
                for (int pcur = 0; pcur < m_pages_cnt; pcur++)
                {
                    double x;
                    float pg_scale = m_scale;
                    float pg_width = m_doc->GetPageWidth(pcur);
                    if (m_same_width)
                        pg_scale = m_scale * size.x / pg_width;
                    switch (m_align)
                    {
                    case ALIGN_LEFT:
                        x = m_scale * (m_page_gap >> 1);
                        break;
                    case ALIGN_RIGHT:
                        x = m_layw - m_scale * (m_page_gap >> 1);
                        break;
                    default:
                        x = (m_layw - pg_width * pg_scale) * 0.5f;
                        break;
                    }
                    CRDVPage* page = m_pages + pcur;
                    //if (page->GetScale() != pg_scale)
                    //{
                        page->ui_end(m_callback, m_thread);
                        page->ui_layout(x, y, pg_scale);
                    //}
                    //else
                    //{
                    //    page->ui_layout(x, y, pg_scale);
                    //}
                    y += (pg_scale * m_doc->GetPageHeight(pcur)) + m_scale * m_page_gap;
                }
                m_layh = y - m_scale * (m_page_gap >> 1);
                m_layh += m_vh * 0.5;
            }
            virtual void vDraw()
            {
                if (!m_doc) return;
                v_flush_range();
                int vx = vGetX();
                int vy = vGetY();
                CRDVPage* cur = m_pages + m_pageno1;
                CRDVPage* end = m_pages + m_pageno2;
                while (cur < end)
                {
                    cur->ui_draw(m_callback, m_thread, vx, vy, m_vw, m_vh);
                    cur->ui_draw_pno(m_callback);
                    cur++;
                }
            }
        };
        class CRDVLayoutDual : public CRDVLayout
        {
        public:
            enum SCALEMODE
            {
                SCALE_NONE = 0,
                SCALE_SAME_WIDTH = 1,
                SCALE_SAME_HEIGHT = 2,
            };
            virtual ~CRDVLayoutDual()
            {
                vClose();
                delete[]m_cells;
                m_cells = NULL;
                m_cells_cnt = 0;
            }
        private:
            inline bool dual_at(int icell)
            {
                if (icell == 0) return m_cover_dual;
                return true;
            }
            inline int get_cell(double vy)
            {
                if (!m_pages || m_pages_cnt <= 0 || !m_cells) return -1;
                int left = 0;
                int right = m_cells_cnt - 1;
                while (left <= right)
                {
                    int mid = (left + right) >> 1;
                    PDFCell* cell = m_cells + mid;
                    if (vy < cell->top)
                    {
                        right = mid - 1;
                    }
                    else if (vy > cell->bottom)
                    {
                        left = mid + 1;
                    }
                    else
                    {
                        return mid;
                    }
                }
                if (right < 0) return 0;
                else return m_cells_cnt - 1;
            }
            bool m_cover_dual;
            bool m_rtol;
            SCALEMODE m_scale_mode;
            struct PDFCell
            {
                double left;
                double right;
                double top;
                double bottom;
                double scale;
                int page_left;
                int page_right;
            };
            PDFCell* m_cells;
            int m_cells_cnt;
        public:
            CRDVLayoutDual(SCALEMODE scale_mode, bool rtol, bool cover_dual) : CRDVLayout()
            {
                m_scale_mode = scale_mode;
                m_rtol = rtol;
                m_cover_dual = cover_dual;
                m_cells = NULL;
                m_cells_cnt = 0;
            }
            int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vy += vGetY();
                vx += vGetX();
                int pl = 0;
                int pr = m_cells_cnt - 1;
                while (pr >= pl) {
                    int mid = (pl + pr) >> 1;
                    PDFCell* pmid = m_cells + mid;
                    if (vy < pmid->top)
                        pr = mid - 1;
                    else if (vy >= pmid->bottom)
                        pl = mid + 1;
                    else {
                        CRDVPage* page = m_pages + pmid->page_left;
                        if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
                        else return pmid->page_left;
                    }
                }
                int mid = (pr < 0) ? 0 : pr;
                PDFCell* pmid = m_cells + mid;
                CRDVPage* page = m_pages + pmid->page_left;
                if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
                else return pmid->page_left;
            }
            inline void layout_ltor(double scale)
            {
                if (m_vw <= 0 || m_vh <= 0) return;
                double maxw = 0;
                double maxh = 0;
                double minw = 9999999999;
                double minh = 9999999999;
                int pcur = 0;
                int ccnt = 0;
                while (pcur < m_pages_cnt)
                {
                    float cw = m_doc->GetPageWidth(pcur);
                    float ch = m_doc->GetPageHeight(pcur);
                    if (dual_at(ccnt))
                    {
                        if (pcur < m_pages_cnt - 1)
                        {
                            cw += m_doc->GetPageWidth(pcur + 1);
                            float ch2 = m_doc->GetPageHeight(pcur + 1);
                            if (ch < ch2) ch = ch2;
                            pcur += 2;
                        }
                        else pcur++;
                    }
                    else pcur++;
                    if (maxw < cw) maxw = cw;
                    if (maxh < ch) maxh = ch;
                    if (minw > cw) minw = cw;
                    if (minh > ch) minh = ch;
                    ccnt++;
                }

                if (m_auto_fit) scale = m_vw / (maxw + m_page_gap);
                if (!m_cells || m_cells_cnt != ccnt)
                {
                    delete[]m_cells;
                    m_cells = new PDFCell[ccnt];
                    m_cells_cnt = ccnt;
                }

                if (scale > 0 || !m_callback)
                    m_scale = scale;
                else
                    m_scale = m_dpi / 72.0;
                m_layw = (maxw + m_page_gap) * m_scale;

                //if(m_scale == scale) return;
                m_layh = 0;
                pcur = 0;
                for (int ccur = 0; ccur < ccnt; ccur++)
                {
                    float cw = m_doc->GetPageWidth(pcur);
                    float ch = m_doc->GetPageHeight(pcur);
                    PDFCell* cell = m_cells + ccur;
                    if (dual_at(ccur))
                    {
                        if (pcur < m_pages_cnt - 1)
                        {
                            cw += m_doc->GetPageWidth(pcur + 1);
                            float ch2 = m_doc->GetPageHeight(pcur + 1);
                            if (ch < ch2) ch = ch2;

                            cell->page_left = pcur;
                            cell->page_right = pcur + 1;
                            pcur += 2;
                        }
                        else
                        {
                            cell->page_left = pcur++;
                            cell->page_right = -1;
                        }
                    }
                    else
                    {
                        cell->page_left = pcur++;
                        cell->page_right = -1;
                    }

                    switch (m_scale_mode)
                    {
                    case SCALE_SAME_WIDTH:
                        cell->scale = minw / cw;
                        break;
                    case SCALE_SAME_HEIGHT:
                        cell->scale = minh / ch;
                        break;
                    default:
                        cell->scale = 1;
                        break;
                    }
                    double pg_scale = m_scale * cell->scale;
                    double cellh = (ch + m_page_gap) * pg_scale;
                    cell->left = 0;
                    cell->right = m_layw;
                    cell->top = m_layh;
                    cell->bottom = m_layh + cellh;
                    double cellw = cw * pg_scale;

                    CRDVPage* pleft = m_pages + cell->page_left;
                    double ph = pg_scale * m_doc->GetPageHeight(cell->page_left);

                    if (pleft->GetScale() != pg_scale)
                    {
                        pleft->ui_end(m_callback, m_thread);
                        pleft->ui_layout((m_layw - cellw) * 0.5, m_layh + (cellh - ph) * 0.5, pg_scale);
                    }
                    else
                    {
                        pleft->ui_layout((m_layw - cellw) * 0.5, m_layh + (cellh - ph) * 0.5, pg_scale);
                    }
                    if (cell->page_right >= 0)
                    {
                        CRDVPage* pright = m_pages + cell->page_right;
                        ph = pg_scale * m_doc->GetPageHeight(cell->page_right);
                        if (pright->GetScale() != pg_scale)
                        {
                            pright->ui_end(m_callback, m_thread);
                            pright->ui_layout(pleft->GetRight(), m_layh + (cellh - ph) * 0.5, pg_scale);
                        }
                        else
                            pright->ui_layout(pleft->GetRight(), m_layh + (cellh - ph) * 0.5, pg_scale);
                    }
                    m_layh = cell->bottom;
                }
            }
            /*
            private final void layout_rtol(float scale, boolean zoom, boolean para[])
            {
                if (m_vw <= 0 || m_vh <= 0) return;
                if (m_vw <= 0 || m_vh <= 0) return;
                float maxw = 0;
                float maxh = 0;
                int minscalew = 0x40000000;
                int minscaleh = 0x40000000;
                int pcur = 0;
                int ccnt = 0;
                boolean last_dual = false;
                while (pcur < m_page_cnt)
                {
                    float cw = m_doc.GetPageWidth(pcur);
                    float ch = m_doc.GetPageHeight(pcur);
                    if (dual_at(para, ccnt))
                    {
                        if (pcur < m_page_cnt - 1)
                        {
                            cw += m_doc.GetPageWidth(pcur + 1);
                            float ch2 = m_doc.GetPageHeight(pcur + 1);
                            if (ch < ch2) ch = ch2;
                            pcur += 2;
                            if (pcur == m_page_cnt) last_dual = true;
                        }
                        else pcur++;
                    }
                    else pcur++;
                    if (maxw < cw) maxw = cw;
                    if (maxh < ch) maxh = ch;
                    float scalew = (m_vw - m_page_gap) / cw;
                    float scaleh = (m_vh - m_page_gap) / ch;
                    if (scalew > scaleh) scalew = scaleh;
                    cw *= scalew;
                    ch *= scalew;
                    if (minscalew > (int)cw) minscalew = (int)cw;
                    if (minscaleh > (int)ch) minscaleh = (int)ch;
                    ccnt++;
                }

                boolean changed = (m_cells == null || m_cells.length != ccnt);
                if (changed) m_cells = new PDFCell[ccnt];
                m_scale_min = (float)(m_vw - m_page_gap) / maxw;
                float scalew;
                float scaleh = (float)(m_vh - m_page_gap) / maxh;
                if (m_scale_min > scaleh) m_scale_min = scaleh;
                float max_scale = m_scale_min * m_max_zoom;
                if (scale < m_scale_min) scale = m_scale_min;
                if (scale > max_scale) scale = max_scale;
                //if(m_scale == scale) return;
                m_scale = scale;
                m_layw = 0;
                m_layh = 0;
                pcur = m_page_cnt - 1;
                for (int ccur = 0; ccur < ccnt; ccur++)
                {
                    float cw = m_doc.GetPageWidth(pcur);
                    float ch = m_doc.GetPageHeight(pcur);
                    if (changed) m_cells[ccur] = new PDFCell();
                    PDFCell cell = m_cells[ccur];
                    if (dual_at(para, ccnt - ccur - 1))
                    {
                        if (pcur > 0 || last_dual)
                        {
                            last_dual = false;

                            cw += m_doc.GetPageWidth(pcur - 1);
                            float ch2 = m_doc.GetPageHeight(pcur - 1);
                            if (ch < ch2) ch = ch2;

                            cell.page_left = pcur - 1;
                            cell.page_right = pcur;
                            pcur -= 2;
                        }
                        else
                        {
                            cell.page_left = pcur--;
                            cell.page_right = -1;
                        }
                    }
                    else
                    {
                        cell.page_left = pcur--;
                        cell.page_right = -1;
                    }
                    switch (m_scale_mode)
                    {
                    case SCALE_SAME_WIDTH:
                        scalew = minscalew / cw;
                        cell.scale = scalew / m_scale_min;
                        break;
                    case SCALE_SAME_HEIGHT:
                        scaleh = minscaleh / ch;
                        cell.scale = scaleh / m_scale_min;
                        break;
                    case SCALE_FIT:
                        scalew = (m_vw - m_page_gap) / cw;
                        scaleh = (m_vh - m_page_gap) / ch;
                        cell.scale = ((scalew > scaleh) ? scaleh : scalew) / m_scale_min;
                        break;
                    default:
                        cell.scale = 1;
                        break;
                    }
                    cell.left = m_layw;
                    int cellw = (int)(cw * scale * cell.scale) + m_page_gap;
                    int cellh = (int)(ch * scale * cell.scale) + m_page_gap;
                    int x = m_page_gap >> 1;
                    int y = m_page_gap >> 1;
                    if (cellw < m_vw) { x = (m_vw - cellw) >> 1; cellw = m_vw; }
                    switch (m_align_type)
                    {
                    case ALIGN_TOP:
                        if (cellh < m_vh) { cellh = m_vh; }
                        break;
                    case ALIGN_BOTTOM:
                        if (cellh < m_vh) { y = (m_vh - cellh) - (m_page_gap >> 1); cellh = m_vh; }
                        break;
                    default:
                        if (cellh < m_vh) { y = (m_vh - cellh) >> 1; cellh = m_vh; }
                        break;
                    }
                    cell.right = cell.left + cellw;
                    PDFGLPage pleft = m_pages[cell.page_left];
                    pleft.gl_layout(m_layw + x, y, scale * cell.scale);
                    if (!zoom) pleft.gl_alloc();
                    if (cell.page_right >= 0)
                    {
                        PDFGLPage pright = m_pages[cell.page_right];
                        pright.gl_layout(pleft.GetRight(), y, scale * cell.scale);
                        if (!zoom) pright.gl_alloc();
                    }
                    m_layw = cell.right;
                    if (m_layh < cellh) m_layh = cellh;
                }
            }
            */
            void vLayout(double scale)
            {
                layout_ltor(scale);
            }
        };
        class CRDVLayoutDualH : public CRDVLayout
        {
        public:
            enum SCALEMODE
            {
                SCALE_NONE = 0,
                SCALE_SAME_WIDTH = 1,
                SCALE_SAME_HEIGHT = 2,
                SCALE_FIT = 3,
            };
            enum ALIGNTYPE
            {
                ALIGN_CENTER = 0,
                ALIGN_TOP = 1,
                ALIGN_BOTTOM = 2,
            };
            CRDVLayoutDualH(SCALEMODE scale_mode, ALIGNTYPE align_type, bool rtol, const bool *vert_dual, int vert_dual_cnt, const bool* horz_dual, int horz_dual_cnt) : CRDVLayout()
            {
                m_scale_mode = scale_mode;
                m_align_type = align_type;
                m_rtol = rtol;
                m_cells = NULL;
                m_cells_cnt = 0;
                if (vert_dual)
                {
                    m_vert_dual = (bool*)malloc(sizeof(bool) * vert_dual_cnt);
                    memcpy(m_vert_dual, vert_dual, sizeof(bool) * vert_dual_cnt);
                    m_vert_dual_cnt = vert_dual_cnt;
                }
                if (horz_dual)
                {
                    m_horz_dual = (bool*)malloc(sizeof(bool) * horz_dual_cnt);
                    memcpy(m_horz_dual, horz_dual, sizeof(bool) * horz_dual_cnt);
                    m_horz_dual_cnt = horz_dual_cnt;
                }
            }
            virtual ~CRDVLayoutDualH()
            {
                vClose();
                if (m_cells)
                {
                    delete[]m_cells;
                    m_cells = NULL;
                    m_cells_cnt = 0;
                }
                if (m_vert_dual)
                {
                    free(m_vert_dual);
                    m_vert_dual = NULL;
                    m_vert_dual_cnt = 0;
                }
                if (m_horz_dual)
                {
                    free(m_horz_dual);
                    m_horz_dual = NULL;
                    m_horz_dual_cnt = 0;
                }
            }
            int vGetPage(double vx, double vy)
            {
                if (m_vw <= 0 || m_vh <= 0) return -1;
                vy += vGetY();
                vx += vGetX();
                int pl = 0;
                int pr = m_cells_cnt - 1;
                while (pr >= pl) {
                    int mid = (pl + pr) >> 1;
                    PDFCell* pmid = m_cells + mid;
                    if (vx < pmid->left)
                        pr = mid - 1;
                    else if (vx >= pmid->right)
                        pl = mid + 1;
                    else {
                        CRDVPage* page = m_pages + pmid->page_left;
                        if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
                        else return pmid->page_left;
                    }
                }
                int mid = (pr < 0) ? 0 : pr;
                PDFCell* pmid = m_cells + mid;
                CRDVPage* page = m_pages + pmid->page_left;
                if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
                else return pmid->page_left;
            }
            void layout_ltor(double scale, bool *para, int para_cnt)
            {
                if (m_vw <= m_page_gap || m_vh <= m_page_gap) return;
                float maxw = 0;
                float maxh = 0;
                int minscalew = 0x40000000;
                int minscaleh = 0x40000000;
                int pcur = 0;
                int ccnt = 0;
                while (pcur < m_pages_cnt) {
                    float cw = m_doc->GetPageWidth(pcur);
                    float ch = m_doc->GetPageHeight(pcur);
                    if (dual_at(para, para_cnt, ccnt)) {
                        if (pcur < m_pages_cnt - 1) {
                            cw += m_doc->GetPageWidth(pcur + 1);
                            float ch2 = m_doc->GetPageHeight(pcur + 1);
                            if (ch < ch2) ch = ch2;
                            pcur += 2;
                        }
                        else pcur++;
                    }
                    else pcur++;
                    if (maxw < cw) maxw = cw;
                    if (maxh < ch) maxh = ch;
                    float scalew = (m_vw - m_page_gap) / cw;
                    float scaleh = (m_vh - m_page_gap) / ch;
                    if (scalew > scaleh) scalew = scaleh;
                    cw *= scalew;
                    ch *= scalew;
                    if (minscalew > (int) cw) minscalew = (int)cw;
                    if (minscaleh > (int)ch) minscaleh = (int)ch;
                    ccnt++;
                }

                if (m_auto_fit) scale = m_vw / (maxw + m_page_gap);
                if (!m_cells || m_cells_cnt != ccnt)
                {
                    delete[]m_cells;
                    m_cells = new PDFCell[ccnt];
                    m_cells_cnt = ccnt;
                }

                float scalew;
                float scaleh = (float)(m_vh - m_page_gap) / maxh;

                if (scale > 0 || !m_callback)
                    m_scale = scale;
                else
                    m_scale = m_dpi / 72.0;
                m_layw = 0;
                m_layh = 0;
                pcur = 0;
                for (int ccur = 0; ccur < ccnt; ccur++) {
                    float cw = m_doc->GetPageWidth(pcur);
                    float ch = m_doc->GetPageHeight(pcur);
                    PDFCell *cell = m_cells + ccur;
                    if (dual_at(para, para_cnt, ccur)) {
                        if (pcur < m_pages_cnt - 1) {
                            cw += m_doc->GetPageWidth(pcur + 1);
                            float ch2 = m_doc->GetPageHeight(pcur + 1);
                            if (ch < ch2) ch = ch2;

                            cell->page_left = pcur;
                            cell->page_right = pcur + 1;
                            pcur += 2;
                        }
                        else {
                            cell->page_left = pcur++;
                            cell->page_right = -1;
                        }
                    }
                    else {
                        cell->page_left = pcur++;
                        cell->page_right = -1;
                    }
                    switch (m_scale_mode) {
                    case SCALE_SAME_WIDTH:
                        scalew = minscalew / cw;
                        cell->scale = scalew / m_scale;
                        break;
                    case SCALE_SAME_HEIGHT:
                        scaleh = minscaleh / ch;
                        cell->scale = scaleh / m_scale;
                        break;
                    case SCALE_FIT:
                        scalew = (m_vw - m_page_gap) / cw;
                        scaleh = (m_vh - m_page_gap) / ch;
                        cell->scale = ((scalew > scaleh) ? scaleh : scalew) / m_scale;
                        break;
                    default:
                        cell->scale = 1;
                        break;
                    }
                    cell->left = m_layw;
                    int cellw = (int)(cw * m_scale * cell->scale) + m_page_gap;
                    int cellh = (int)(ch * m_scale * cell->scale) + m_page_gap;
                    int x = m_page_gap >> 1;
                    int y = m_page_gap >> 1;
                    if (cellw < m_vw) {
                        x = ((int)m_vw - cellw) >> 1;
                        cellw = m_vw;
                    }
                    switch (m_align_type) {
                    case ALIGN_TOP:
                        if (cellh < m_vh) {
                            cellh = m_vh;
                        }
                        break;
                    case ALIGN_BOTTOM:
                        if (cellh < m_vh) {
                            y = (m_vh - cellh) - (m_page_gap >> 1);
                            cellh = m_vh;
                        }
                        break;
                    default:
                        if (cellh < m_vh) {
                            y = ((int)m_vh - cellh) >> 1;
                            cellh = m_vh;
                        }
                        break;
                    }
                    cell->right = cell->left + cellw;
                    CRDVPage *pleft = m_pages + cell->page_left;
                    pleft->ui_layout(m_layw + x, y, m_scale* cell->scale);
                    if (cell->page_right >= 0) {
                        CRDVPage *pright = m_pages + cell->page_right;
                        pright->ui_layout(pleft->GetRight(), y, m_scale* cell->scale);
                    }
                    m_layw = cell->right;
                    if (m_layh < cellh) m_layh = cellh;
                }
            }
            void vLayout(double scale)
            {
                if (m_vh > m_vw || m_vw < 800)
                    layout_ltor(scale, m_vert_dual, m_vert_dual_cnt);
                else
                    layout_ltor(scale, m_horz_dual, m_horz_dual_cnt);
            }
        private:
            static bool dual_at(bool *para, int para_cnt, int icell) {
                if (!para || icell >= para_cnt) return false;
                return para[icell];
            }
            inline int get_cell(double vx)
            {
                if (!m_pages || m_pages_cnt <= 0 || !m_cells) return -1;
                int left = 0;
                int right = m_cells_cnt - 1;
                while (left <= right)
                {
                    int mid = (left + right) >> 1;
                    PDFCell* cell = m_cells + mid;
                    if (vx < cell->left)
                    {
                        right = mid - 1;
                    }
                    else if (vx > cell->right)
                    {
                        left = mid + 1;
                    }
                    else
                    {
                        return mid;
                    }
                }
                if (right < 0) return 0;
                else return m_cells_cnt - 1;
            }
            bool m_rtol;
            ALIGNTYPE m_align_type;
            SCALEMODE m_scale_mode;
            struct PDFCell
            {
                double left;
                double right;
                double top;
                double bottom;
                double scale;
                int page_left;
                int page_right;
            };
            bool* m_vert_dual;
            int m_vert_dual_cnt;
            bool* m_horz_dual;
            int m_horz_dual_cnt;
            PDFCell* m_cells;
            int m_cells_cnt;
        };
    }
}