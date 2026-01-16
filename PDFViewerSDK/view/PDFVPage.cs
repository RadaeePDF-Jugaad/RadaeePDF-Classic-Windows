using RDPDFLib.pdf;
using System;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFVPage
        {
            static PDFBmp ms_back = null;
            static void init()
            {
                if (ms_back == null)
                {
                    ms_back = new PDFBmp(1, 1);
                    ms_back.Reset(0xFFFFFFFF);
                }
            }
            protected PDFDoc m_doc;
            public PDFVCache m_cache;
            public PDFVThumb m_thumb;
            protected PDFVSel m_sel;
            protected int m_pageno;
            protected float m_scale = 0;
            protected float m_zoom = 1;
            public float m_w = 0;
            public float m_h = 0;
            public float m_x;
            public float m_y;
            protected PDFBmp m_bmp = null;
            protected Image m_img = null;
            public PDFVPage(PDFDoc doc, int pageno)
            {
                init();
                m_pageno = pageno;
                m_doc = doc;
            }
            public Boolean SetRect(PDFVLayout canvas, float x, float y, float scale)
            {
                if (m_x == x && m_y == y && m_scale == scale) return false;
                m_scale = scale;
                m_x = x;
                m_y = y;
                m_w = scale * m_doc.GetPageWidth(m_pageno);
                m_h = scale * m_doc.GetPageHeight(m_pageno);
                if (m_img == null)
                {
                    m_img = new Image();
                    m_img.Source = ms_back.Data;
                    m_img.Stretch = Stretch.Fill;
                    canvas.Children.Add(m_img);
                }
                m_img.Width = m_w;
                m_img.Height = m_h;

                //m_img.SetValue(Panel.MarginProperty, new Windows.UI.Xaml.Thickness(m_x, m_y, 0, 0));

                m_img.SetValue(Canvas.LeftProperty, m_x);
                m_img.SetValue(Canvas.TopProperty, m_y);
                return true;
            }
            public void SetZoom(float zoom)
            {
                if (m_zoom == zoom) return;
                m_zoom = zoom;
            }

            public float GetZoom()
            {
                return m_zoom;
            }
            public Boolean IsFinished()
            {
                if (m_cache != null) return m_cache.m_status == 1;
                if (m_thumb != null) return m_thumb.m_status == 1;
                return true;
            }
            public int RenderPrepare(bool is_thumb)
            {
                float scale = m_scale * m_zoom;
                int w = (int)(scale * m_doc.GetPageWidth(m_pageno));
                int h = (int)(scale * m_doc.GetPageHeight(m_pageno));
                if (m_cache == null)
                {
                    m_cache = new PDFVCache(m_doc, m_pageno, scale, w, h, is_thumb);
                    return 0;
                }
                if (m_cache.UIIsSame(scale, w, h)) return 1;
                return 2;
            }
            public void RenderSync()
            {
                if (m_cache != null)
                    m_cache.Render();
                if (m_thumb != null)
                    m_thumb.Render();
            }
            /// <summary>
            /// cancel render session, mostly it invoked by UI thread.
            /// </summary>
            /// <returns></returns>
            public PDFVCache CancelRender()
            {
                if (m_cache == null) return null;
                if (m_sel != null)
                {
                    m_sel.Clear();
                    m_sel = null;
                }
                //to free memory.
                m_img.Source = ms_back.Data;
                m_img.Stretch = Stretch.Fill;
                PDFVCache cache = m_cache;
                m_cache = null;
                cache.UIRenderCancel();
                return cache;
            }
            /// <summary>
            /// set text selection by coordinates.
            /// </summary>
            /// <param name="x1"></param>
            /// <param name="y1"></param>
            /// <param name="x2"></param>
            /// <param name="y2"></param>
            /// <param name="scrollx"></param>
            /// <param name="scrolly"></param>
            public void SetSel(float x1, float y1, float x2, float y2, float scrollx, float scrolly)
            {
                if (m_sel == null) m_sel = new PDFVSel(m_doc.GetPage(m_pageno));
                m_sel.SetSel(ToPDFX(x1, scrollx), ToPDFY(y1, scrolly), ToPDFX(x2, scrollx), ToPDFY(y2, scrolly));
            }
            /// <summary>
            /// set selected texts as markup annotation.
            /// </summary>
            /// <param name="color"></param>
            /// <param name="type"></param>
            /// <returns></returns>
            public Boolean SetSelMarkup(uint color, int type)
            {
                if (m_sel != null)
                    return m_sel.SetSelMarkup(color, type);
                return false;
            }
            /// <summary>
            /// get selected texts.
            /// </summary>
            /// <returns></returns>
            public String GetSel()
            {
                if (m_sel == null) return null;
                return m_sel.GetSelString();
            }
            /// <summary>
            /// clear text selection.
            /// </summary>
            public void ClearSel()
            {
                if (m_sel != null)
                {
                    m_sel.m_index1 = -1;
                    m_sel.m_index2 = -1;
                }
            }
            /// <summary>
            /// draw the page.
            /// </summary>
            /// <param name="canvas"></param>
            /// <param name="scrollx"></param>
            /// <param name="scrolly"></param>
            public void Draw(PDFVCanvas canvas, float scrollx, float scrolly, float offsetx, float offsety)
            {
                PDFVCache cache = m_cache;
                if (m_bmp != null)
                {
                    m_img.Source = m_bmp.Data;
                    m_img.Stretch = Stretch.Fill;
                    m_bmp.Data.Invalidate();
                }
                else if (cache != null && cache.m_dib != null)
                {
                    m_img.Source = cache.m_dib.Data;
                    m_img.Stretch = Stretch.Fill;
                    cache.m_dib.Data.Invalidate();
                }
                else//just draw white
                {
                    m_img.Source = ms_back.Data;
                    m_img.Stretch = Stretch.Fill;
                }
                //if (m_scale < 1) {
                //    float tmp = 1 / m_zoom;
                //    offsetx *= tmp;
                //    offsety *= tmp;
                //}

                if (m_sel != null)
                    m_sel.DrawSel(canvas, m_scale, m_doc.GetPageHeight(m_pageno), m_x - scrollx + offsetx, m_y - scrolly + offsety);
            }
            public PDFRect GetSelRect1(float scrollx, float scrolly)
            {
                if (m_sel == null)
                {
                    PDFRect rect;
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                return m_sel.GetRect1(m_scale, m_doc.GetPageHeight(m_pageno), m_x - scrollx, m_y - scrolly);
            }
            public PDFRect GetSelRect2(float scrollx, float scrolly)
            {
                if (m_sel == null)
                {
                    PDFRect rect;
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
                return m_sel.GetRect2(m_scale, m_doc.GetPageHeight(m_pageno), m_x - scrollx, m_y - scrolly);
            }
            public int ThumbPrepare()
            {
                if (m_thumb == null)
                {
                    m_thumb = new PDFVThumb(m_doc, m_pageno, m_scale, (int)m_w, (int)m_h);
                    return 0;
                }
                return 1;
            }
            public PDFVThumb CancelThumb()
            {
                if (m_thumb == null) return null;
                PDFVThumb thumb = m_thumb;
                m_thumb = null;
                thumb.UIRenderCancel();
                //to free memory.
                m_img.Source = ms_back.Data;
                m_img.Stretch = Stretch.Fill;
                return thumb;
            }
            public void DrawThumb(Canvas canvas, int scrollx, int scrolly)
            {
                if (m_thumb != null && m_thumb.m_bmp != null)
                {
                    m_img.Source = m_thumb.m_bmp.Data;
                    m_img.Stretch = Stretch.None;
                    m_thumb.m_bmp.Data.Invalidate();
                    return;
                }
                //canvas.drawRect(m_rect, m_thumb_paint);
            }
            public void DeleteBmp()
            {
                if (m_bmp != null)
                {
                    m_bmp = null;
                    GC.Collect();
                }
            }
            public void CreateBmp()
            {
                if (m_cache == null || m_cache.m_status != 1 || m_bmp != null) return;
                m_bmp = m_cache.m_dib;
                m_cache.Clear();
                m_cache = null;
                if (m_sel != null)
                {
                    m_sel.Clear();
                    m_sel = null;
                }
            }
            public Boolean NeedBmp()
            {
                if (m_bmp == null) return false;
                if (m_cache != null)
                {
                    float scale = m_scale * m_zoom;
                    int w = (int)(scale * m_doc.GetPageWidth(m_pageno));
                    int h = (int)(scale * m_doc.GetPageHeight(m_pageno));
                    return (m_cache.m_status != 1 || !m_cache.UIIsSame(scale, w, h));
                }
                else
                    return true;
            }
            /// <summary>
            /// map x position in view to PDF coordinate
            /// </summary>
            /// <param name="x">x position in view</param>
            /// <param name="scrollx">x scroll position of scrollview.</param>
            /// <returns></returns>
            public float ToPDFX(float x, float scrollx)
            {
                float dibx = scrollx + x - m_x;
                return dibx / m_scale;
            }
            /// <summary>
            /// map y position in view to PDF coordinate
            /// </summary>
            /// <param name="y">y position in view</param>
            /// <param name="scrolly">y scroll position of scrollview.</param>
            /// <returns></returns>
            public float ToPDFY(float y, float scrolly)
            {
                float diby = scrolly + y - m_y;
                return (m_h - diby) / m_scale;
            }
            /// <summary>
            /// map x to DIB coordinate
            /// </summary>
            /// <param name="x">x position in PDF coordinate</param>
            /// <returns></returns>
            public float ToDIBX(float x)
            {
                return x * m_scale;
            }
            /// <summary>
            /// map y to DIB coordinate
            /// </summary>
            /// <param name="y">y position in PDF coordinate</param>
            /// <returns></returns>
            public float ToDIBY(float y)
            {
                return (m_doc.GetPageHeight(m_pageno) - y) * m_scale;
            }
            /// <summary>
            /// get page object.
            /// </summary>
            /// <returns>null if not rendered, page object if rendered.</returns>
            public PDFPage GetPage()
            {
                if (m_cache == null) return null;
                return m_cache.m_page;
            }
            /// <summary>
            /// get page NO.
            /// </summary>
            /// <returns></returns>
            public int GetPageNo()
            {
                return m_pageno;
            }
            /// <summary>
            /// get x position of layout contents.
            /// </summary>
            /// <returns></returns>
            public float GetX()
            {
                return m_x;
            }
            /// <summary>
            /// get y position of layout contents.
            /// </summary>
            /// <returns></returns>
            public float GetY()
            {
                return m_y;
            }
            /// <summary>
            /// get scale factor.
            /// </summary>
            /// <returns></returns>
            public float GetScale()
            {
                return m_scale;
            }
            /// <summary>
            /// get x position in View
            /// </summary>
            /// <param name="scrollx">x scroll position of scrollview.</param>
            /// <returns></returns>
            public float GetVX(float scrollx)
            {
                return m_x - scrollx;
            }
            public float GetVY(float scrolly)
            {
                return m_y - scrolly;
            }
            /// <summary>
            /// get y position in View
            /// </summary>
            /// <param name="scrolly">y scroll position of scrollview.</param>
            /// <returns></returns>
            public float GetWidth()
            {
                return m_w;
            }
            /// <summary>
            /// get page width in view
            /// </summary>
            /// <returns></returns>
            public float GetHeight()
            {
                return m_h;
            }
            /// <summary>
            /// create a Matrix object maps PDF coordinate to DIB coordinate.
            /// </summary>
            /// <returns></returns>
            public PDFMatrix CreateMatrix()
            {
                return new PDFMatrix(m_scale, -m_scale, 0, m_h);
            }
            /// <summary>
            /// create an Inverted Matrix maps screen coordinate to PDF coordinate.
            /// </summary>
            /// <param name="scrollx">current x for PDFView</param>
            /// <param name="scrolly">current y for PDFView</param>
            /// <returns></returns>
            public PDFMatrix CreateInvertMatrix(float scrollx, float scrolly)
            {
                return new PDFMatrix(1 / m_scale, -1 / m_scale, (scrollx - m_x) / m_scale, (m_y + m_h - scrolly) / m_scale);
            }
            /// <summary>
            /// convert size to PDF size
            /// </summary>
            /// <param name="val">size value, mostly are line width.</param>
            /// <returns>size value in PDF coordinate</returns>
            public float ToPDFSize(float val)
            {
                return val / m_scale;
            }
        }
    }
}
