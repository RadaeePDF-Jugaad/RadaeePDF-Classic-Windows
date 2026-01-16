using System;
using RDPDFLib.pdf;
using Windows.UI.Xaml.Media.Imaging;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFVCache
        {
            protected PDFDoc m_doc;
            protected int m_pageno;
            public PDFPage m_page = null;
            protected float m_scale = 1;
            public PDFBmp m_dib = null;
            protected int m_dibw = 0;
            protected int m_dibh = 0;
            public int m_status = 0;
            protected bool m_is_thumb;
            public PDFVCache(PDFDoc doc, int pageno, float scale, int w, int h, bool is_thumb)
            {
                m_doc = doc;
                m_pageno = pageno;
                m_scale = scale;
                m_dibw = w;
                m_dibh = h;
                //int tick = System.Environment.TickCount;
                m_dib = new PDFBmp(m_dibw, m_dibh);
                //int tick1 = System.Environment.TickCount;
                //int tick_n = tick1 - tick;
                m_dib.Reset(0xFFFFFFFF);
                //tick = System.Environment.TickCount - tick1;
                m_status = 0;
                m_is_thumb = is_thumb;
            }
            public Boolean UIIsSame(float scale, int w, int h)
            {
                return (m_scale == scale && m_dibw == w && m_dibh == h);
            }
            public void Clear()
            {
                if (m_page != null)
                {
                    m_page.Close();
                    m_page = null;
                }
                m_dib = null;
                m_status = 0;
                GC.Collect();
            }
            public void UIRenderCancel()
            {
                if (m_status == 0)
                {
                    m_status = 2;
                    PDFPage page = m_page;
                    if (page != null)
                    {
                        page.RenderCancel();
                        page = null;
                    }
                }
            }
            public void Render()
            {
                if (m_status == 2 || m_dib == null) return;
                if (m_page == null)
                    m_page = m_doc.GetPage(m_pageno);
                m_page.RenderPrepare();
                if (m_status == 2) return;
                PDFMatrix mat = new PDFMatrix(m_scale, -m_scale, 0, m_dibh);
                m_page.RenderToBmp(m_dib, mat, true, PDFView.renderQuality);
                mat = null;
                if (m_status != 2)
                {
                    m_status = 1;
                    if (m_is_thumb && m_page != null)
                    {
                        m_page.Close();
                        m_page = null;
                    }
                }
            }
        }
        public class PDFVThumb
        {
            protected PDFDoc m_doc;
            protected int m_pageno;
            protected PDFPage m_page = null;
            protected float m_scale = 1;
            public PDFBmp m_bmp = null;
            protected int m_bmpw = 0;
            protected int m_bmph = 0;
            public int m_status = 0;
            public PDFVThumb(PDFDoc doc, int pageno, float scale, int w, int h)
            {
                m_doc = doc;
                m_pageno = pageno;
                m_scale = scale;
                m_bmpw = w;
                m_bmph = h;
                m_bmp = new PDFBmp(m_bmpw, m_bmph);
                m_bmp.Reset(0xFFFFFFFF);
                m_status = 0;
            }
            public void Clear()
            {
                if (m_page != null)
                {
                    m_page.Close();
                    m_page = null;
                }
                m_bmp = null;
                GC.Collect();
            }
            public void UIRenderCancel()
            {
                if (m_status == 0)
                {
                    m_status = 2;
                    PDFPage page = m_page;
                    if (page != null) page.RenderCancel();
                }
            }
            public void Render()
            {
                if (m_status == 2) return;
                m_page = m_doc.GetPage(m_pageno);
                if (m_status == 2) return;
                if (m_bmpw > 0 && m_bmph > 0)
                {
                    m_page.RenderPrepare();
                    if (m_status == 2) return;
                    PDFMatrix mat = new PDFMatrix(m_scale, -m_scale, 0, m_bmph);
                    m_page.RenderToBmp(m_bmp, mat, true, PDF_RENDER_MODE.mode_normal);
                    if (m_status != 2)
                    {
                        m_status = 1;
                        if (m_page != null)
                        {
                            m_page.Close();
                            m_page = null;
                        }
                    }
                }
            }
        }

    }
}