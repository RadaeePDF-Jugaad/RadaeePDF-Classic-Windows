using RDPDFLib.pdf;
using System;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFView
        {

            static public int viewMode = 0;
            static public float ovalWidth = 2;
            static public uint textColor = 0xFFffffFF;
            static public uint ovalColor = 0xFF0000FF;
            static public float rectWidth = 2;
            static public uint rectColor = 0xFFFF0000;
            static public float inkWidth = 3;
            static public uint inkColor = 0xFFFF0000;
            static public float lineWidth = 2;
            static public uint lineColor = 0xFFFF0000;
            static public PDF_RENDER_MODE renderQuality = PDF_RENDER_MODE.mode_normal;
            static public float zoomLevel = 3;
            public enum PDFV_STATUS
            {
                STA_NONE = 0,
                STA_ZOOM = 1,
                STA_SELECT = 2,
                STA_ANNOT = 3,
                STA_NOTE = 4,
                STA_INK = 5,
                STA_RECT = 6,
                STA_ELLIPSE = 7,
                STA_LINE = 8,
                STA_TEXT_EDIT = 9
            }

            public enum PDFV_MODE
            {
                MODE_VERTICAL = 0,
                MODE_HROIZONTAL = 1,
                MODE_DUAL_PAGE = 2,
                MODE_OTHER = 3,
            }
            protected PDFDoc m_doc = null;
            protected ScrollViewer m_scroller = new ScrollViewer();
            protected Canvas m_parent = null;
            protected PDFVLayout m_layout = null;
            protected PDFVCanvas m_extra = null;
            protected float m_w;
            protected float m_h;
            protected float m_docw;
            protected float m_doch;
            private int m_lock = 0;
            protected PDFVThread m_thread;
            protected PDFVFinder m_finder;
            protected float m_scale;
            protected int m_page_gap = 4;
            protected PDFVPage[] m_pages = null;
            protected PDFV_STATUS m_status = PDFV_STATUS.STA_NONE;
            protected bool m_is_thumb = false;
            private Boolean m_drawbmp = false;
            private Boolean m_modified = false;
            protected PDFViewListener m_listener = null;
            //protected float m_maxw;
            protected PDFVPage m_max_page;
            protected PDFV_MODE m_mode;
            /// <summary>
            /// call-back listener class, passed to vOpen.
            /// </summary>
            public interface PDFViewListener
            {
                /// <summary>
                /// fired when page no changed.
                /// </summary>
                /// <param name="pageno">page NO.</param>
                void OnPDFPageChanged(int pageno);
                /// <summary>
                /// fired when single tapped on page, without annotations.
                /// </summary>
                /// <param name="x"></param>
                /// <param name="y"></param>
                /// <returns></returns>
                Boolean OnPDFSingleTapped(float x, float y);
                void OnPDFPageTapped(PDFVPage vpage);
                void OnPDFLongPressed(float x, float y);
                /// <summary>
                /// fired when text found.
                /// </summary>
                /// <param name="found">true if found, false if end of PDF.</param>
                void OnPDFFound(Boolean found);
                /// <summary>
                /// fired when a page displayed.
                /// </summary>
                /// <param name="canvas">Canvas object to draw.</param>
                /// <param name="vpage">PDFVPage object</param>
                void OnPDFPageDisplayed(Canvas canvas, PDFVPage vpage);
                /// <summary>
                /// fired when selecting.
                /// </summary>
                /// <param name="canvas">Canvas object to draw.</param>
                /// <param name="rect1">first char's location, in Canvas coordinate.</param>
                /// <param name="rect2">last char's location, in Canvas coordinate.</param>
                void OnPDFSelecting(Canvas canvas, PDFRect rect1, PDFRect rect2);
                /// <summary>
                /// fired when text selected.
                /// using vGetSelText to get texts.
                /// using vSelEnd to end selecting status.
                /// using vSelMarkup to set markup annotation.
                /// </summary>
                void OnPDFSelected();
                /// <summary>
                /// fired when an annotation single tapped.
                /// this means enter into annotation status.
                /// </summary>
                /// <param name="page">page object annotation included.</param>
                /// <param name="annot">clicked annotation.</param>
                void OnPDFAnnotClicked(PDFPage page, PDFAnnot annot);
                /// <summary>
                /// fired when annotation status leaved.
                /// </summary>
                void OnPDFAnnotEnd();

                /// <summary>
                /// fired when goto link annotation performed.
                /// </summary>
                /// <param name="pageno"></param>
                void OnPDFAnnotGoto(int pageno);
                /// <summary>
                /// fired when uri link annotation performed.
                /// </summary>
                /// <param name="uri">uri address.</param>
                void OnPDFAnnotURI(String uri);
                ///// <summary>
                ///// fired when a movie annotation performed.
                ///// </summary>
                ///// <param name="annot">Annotation object</param>
                ///// <param name="name">file name of movie without path.</param>
                //void OnPDFAnnotMovieAsync(PDFAnnot annot, String name);
                ///// <summary>
                ///// fired when a sound annotation performed.
                ///// </summary>
                ///// <param name="annot">Annotation object</param>
                ///// <param name="name">file name of sound without path.</param>
                //void OnPDFAnnotSound(PDFAnnot annot, String name);
                /// <summary>
                /// fired when an annotation with rich media performed
                /// </summary>
                /// <param name="annot">Annotation object</param>
                System.Threading.Tasks.Task OnPDFAnnotRichMediaAsync(PDFAnnot annot);

                /// <summary>
                /// fired when vAnnotPerform invoked or Note Annotation added.
                /// </summary>
                /// <param name="annot">Annotation object</param>
                /// <param name="subj">subject</param>
                /// <param name="text">text in popup window</param>
                void OnPDFAnnotPopup(PDFAnnot annot, String subj, String text);

                /// <summary>
                /// fired when remote destination (external file) annotation performed.
                /// </summary>
                /// <param name="dest">remote dest (the external file name with optional page number).</param>
                void OnPDFAnnotRemoteDest(String dest);
                /// <summary>
                /// fired when file link annotation performed.
                /// </summary>
                /// <param name="filelink">file link annotation path.</param>
                void OnPDFAnnotFileLink(String filelink);

                //void OnPDFZoomStart();
                //void OnPDFZoomEnd();
            };
            /// <summary>
            /// class for position in view
            /// </summary>
            public struct PDFPos
            {
                /// <summary>
                /// page NO.
                /// </summary>
                public int pageno;
                /// <summary>
                /// x in PDF coordinate.
                /// </summary>
                public float x;
                /// <summary>
                /// y in PDF coordinate.
                /// </summary>
                public float y;
            };
            /// <summary>
            /// redraw screen.
            /// </summary>
            private void vOnRender()
            {
                vDraw();
            }
            /// <summary>
            /// text finding notify.
            /// </summary>
            private void vOnFound()
            {
                if (m_finder.find_get_page() >= 0)//succeeded
                {
                    vFindGoto();
                    if (m_listener != null)
                        m_listener.OnPDFFound(true);
                }
                else
                {
                    if (m_listener != null)
                        m_listener.OnPDFFound(false);
                }
            }
            /// <summary>
            /// fired when Page resized.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            private void vOnSizeChanged(object sender, SizeChangedEventArgs e)
            {
                PDFPos pos = vGetPos(m_w / 2, m_h / 2);
                vInkEnd();
                vLineEnd();
                vNoteEnd();
                vAnnotEnd();
                vRectEnd();
                vEllipseEnd();
                vSelEnd();
                vResize((float)e.NewSize.Width, (float)e.NewSize.Height);
                if (pos.pageno >= 0)
                    vSetPos(pos, m_w / 2, m_h / 2);
            }
            TextBlock m_tdebug = null;
            public void vSetDebugTextBox(TextBlock tdebug) { m_tdebug = tdebug; }
            private float m_oldx = 0;
            private float m_oldy = 0;
            private float m_curx = 0;
            private float m_cury = 0;
            /// <summary>
            /// timer notifier
            /// </summary>
            private void vOnTimer()
            {
                if (m_pages != null && m_status != PDFV_STATUS.STA_ZOOM)
                {
                    int cur = m_prange_start;
                    int cnt = m_prange_end;
                    float tmp = 1 / m_scroller.ZoomFactor;
                    m_curx = (float)m_scroller.HorizontalOffset * tmp;
                    m_cury = (float)m_scroller.VerticalOffset * tmp;
                    if (m_curx != m_oldx || m_cury != m_oldy)
                    {
                        m_oldx = m_curx;
                        m_oldy = m_cury;
                        vDraw();
                    }
                    else if (m_drawbmp)
                    {
                        while (cur < cnt)
                        {
                            if (m_pages[cur].NeedBmp())
                                break;
                            cur++;
                        }
                        if (cur >= cnt)
                        {
                            m_drawbmp = false;
                            cur = 0;
                            while (cur < cnt)
                            {
                                m_pages[cur].DeleteBmp();
                                cur++;
                            }
                            vDraw();
                        }
                    }
                    else
                    {
                        while (cur < cnt)
                        {
                            if (!m_pages[cur].IsFinished())
                            {
                                vDraw();
                                break;
                            }
                            cur++;
                        }
                    }
                }
            }
            /// <summary>
            /// touch event on selecting status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns>true if processed, flase if not correct status.</returns>
            private bool OnSelTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_SELECT) return false;

                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                        point.X -= offset;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                        point.Y -= offset;
                    }
                }
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;

                //if( m_listener != null )
                //    m_listener.OnSelStart(point.X: point.Y);
                return true;
            }
            /// <summary>
            /// set text selection by coordinate.
            /// </summary>
            /// <param name="x1"></param>
            /// <param name="y1"></param>
            /// <param name="x2"></param>
            /// <param name="y2"></param>
            private void SetSel(float x1, float y1, float x2, float y2)
            {
                int pageno = vGetPage(x1, y1);
                if (pageno < 0) return;
                PDFVPage vpage = m_pages[pageno];
                float izFactor = 1 / m_scroller.ZoomFactor;
                vpage.SetSel(x1 * izFactor, y1 * izFactor, x2 * izFactor, y2 * izFactor,
                    (float)m_scroller.HorizontalOffset * izFactor, (float)m_scroller.VerticalOffset * izFactor);
            }
            /// <summary>
            /// touch moving event on selecting status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnSelTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_SELECT) return false;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                        point.X -= offset;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                        point.Y -= offset;
                    }
                }
                SetSel(m_hold_x, m_hold_y, (float)point.X, (float)point.Y);
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end event on selecting status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnSelTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_SELECT) return false;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                        point.X -= offset;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                        point.Y -= offset;
                    }
                }
                SetSel(m_hold_x, m_hold_y, (float)point.X, (float)point.Y);
                vDraw();
                if (m_listener != null)
                    m_listener.OnPDFSelected();
                return true;
            }
            /// <summary>
            /// touche event on annotation status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnAnnotTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return false;

                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_shold_x = m_hold_x;
                m_shold_y = m_hold_y;
                return true;
            }
            /// <summary>
            /// touch moving on annotation status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnAnnotTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return false;
                if (m_doc.CanSave)
                {

                    m_shold_x = (float)point.X;
                    m_shold_y = (float)point.Y;
                }
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end on annotation status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnAnnotTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return false;

                if (m_doc != null && m_doc.CanSave)
                {
                    m_modified = true;
                    float dx = (float)(point.X - m_hold_x);
                    float dy = (float)(point.Y - m_hold_y);
                    m_annot_rect.left += dx;
                    m_annot_rect.top += dy;
                    m_annot_rect.right += dx;
                    m_annot_rect.bottom += dy;
                    dx = 1 / m_scroller.ZoomFactor;
                    m_annot_rect.left *= dx;
                    m_annot_rect.top *= dx;
                    m_annot_rect.right *= dx;
                    m_annot_rect.bottom *= dx;
                    PDFPos pos = vGetPos((float)point.X, (float)point.Y);
                    PDFVPage vpage = m_pages[m_annot_pos.pageno];
                    if (pos.pageno == m_annot_pos.pageno)
                    {
                        PDFMatrix mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * dx, (float)m_scroller.VerticalOffset * dx);
                        m_annot_rect = mat.TransformRect(m_annot_rect);
                        m_annot.Rect = m_annot_rect;
                        vRenderSync(vpage);
                    }
                    else
                    {
                        PDFVPage vdest = m_pages[pos.pageno];
                        PDFPage dpage = vdest.GetPage();
                        if (dpage != null)
                        {
                            PDFMatrix mat = vdest.CreateInvertMatrix((float)m_scroller.HorizontalOffset * dx, (float)m_scroller.VerticalOffset * dx);
                            m_annot_rect = mat.TransformRect(m_annot_rect);
                            m_annot.MoveToPage(dpage, m_annot_rect);
                            vRenderSync(vpage);
                            vRenderSync(vdest);
                        }
                    }
                    vAnnotEnd();
                }
                return true;
            }
            /// <summary>
            /// touch event on note status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnNoteTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_NOTE) return false;
                return true;
            }
            /// <summary>
            /// touch moving on note status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnNoteTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_NOTE) return false;
                return true;
            }
            /// <summary>
            /// touch end on note status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnNoteTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_NOTE) return false;
                if (m_pages == null) return false;
                PDFPos pos = vGetPos((float)point.X, (float)point.Y);
                PDFVPage vpage = m_pages[pos.pageno];
                //point = vTransferPoint(point);
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                        point.X -= offset;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                        point.Y -= offset;
                    }
                }
                pos = vGetPos((float)point.X, (float)point.Y);
                if (vpage != null)
                {
                    PDFPage page = vpage.GetPage();
                    if (page != null)
                    {
                        if (page.AddAnnotTextNote(pos.x, pos.y))
                        {
                            m_notes[m_notes_cnt].vpage = vpage;
                            m_notes[m_notes_cnt].index = page.AnnotCount - 1;
                            vRenderSync(vpage);
                            vDraw();
                            if (m_listener != null)
                            {
                                page = vpage.GetPage();
                                PDFAnnot annot = page.GetAnnot(m_notes[m_notes_cnt].index);
                                m_listener.OnPDFAnnotPopup(annot, "", "");
                            }
                            m_notes_cnt++;
                        }
                    }
                }
                return true;
            }
            /// <summary>
            /// touch event on ink status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnInkTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_INK) return false;
                if (m_ink == null)
                {
                    //(宽度，颜色)
                    m_hold_x = (float)point.X;
                    m_hold_y = (float)point.Y;
                    m_ink = new PDFInk(inkWidth, inkColor);
                    m_ink_path = new Windows.UI.Xaml.Shapes.Path();
                    m_ink_path.Data = new PathGeometry();
                    m_ink_pos = 0;
                    Color clr;
                    clr.A = (byte)(inkColor >> 24);
                    clr.R = (byte)(inkColor >> 16);
                    clr.G = (byte)(inkColor >> 8);
                    clr.B = (byte)(inkColor);
                    m_ink_path.Stroke = new SolidColorBrush(clr);
                    m_ink_path.StrokeThickness = inkWidth;
                    m_ink_path.StrokeStartLineCap = PenLineCap.Round;
                    m_ink_path.StrokeLineJoin = PenLineJoin.Round;
                }
                float offsetx = 0;
                float offsety = 0;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        offsetx = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        offsety = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                m_ink.Down((float)point.X - offsetx, (float)point.Y - offsety);
                PathFigure inkf = new PathFigure();
                ((PathGeometry)m_ink_path.Data).Figures.Add(inkf);
                return true;
            }
            /// <summary>
            /// touch moving on ink status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnInkTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_INK) return false;
                float offsetx = 0;
                float offsety = 0;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        offsetx = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        offsety = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                m_ink.Move((float)point.X - offsetx, (float)point.Y - offsety);
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end on ink status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnInkTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_INK) return false;
                float offsetx = 0;
                float offsety = 0;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        offsetx = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        offsety = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                m_ink.Up((float)point.X - offsetx, (float)point.Y - offsety);
                vDraw();
                return true;
            }
            /// <summary>
            /// touch event on rect status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnRectTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_RECT) return false;
                if (m_rects_cnt >= 256) return true;
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_rects[m_rects_cnt << 1].x = m_hold_x;
                m_rects[m_rects_cnt << 1].y = m_hold_y;
                m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
                m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
                m_rects_cnt++;
                return true;
            }
            /// <summary>
            /// touch moving on rect status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnRectTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_RECT) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end on rect status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnRectTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_RECT) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                //vRectEnd();
                return true;
            }

            private bool OnEditTextBoxTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_TEXT_EDIT) return false;
                if (m_rects_cnt >= 256) return true;
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_rects[m_rects_cnt << 1].x = m_hold_x;
                m_rects[m_rects_cnt << 1].y = m_hold_y;
                m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
                m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
                m_rects_cnt++;
                return true;
            }

            private bool OnEditTextBoxTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_TEXT_EDIT) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            private bool OnEditTextBoxTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_TEXT_EDIT) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                //vRectEnd();
                return true;
            }
            /// <summary>
            /// touch event on ellipse status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnEllipseTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ELLIPSE) return false;
                if (m_rects_cnt >= 256) return true;
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_rects[m_rects_cnt << 1].x = m_hold_x;
                m_rects[m_rects_cnt << 1].y = m_hold_y;
                m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
                m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
                m_rects_cnt++;
                return true;
            }
            /// <summary>
            /// touch moving on ellipse status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnEllipseTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ELLIPSE) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end on ellipse status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnEllipseTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_ELLIPSE) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            /// <summary>
            /// touch event on line status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnLineTouchBegin(Point point)
            {
                if (m_status != PDFV_STATUS.STA_LINE) return false;
                if (m_rects_cnt >= 256) return true;
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_rects[m_rects_cnt << 1].x = m_hold_x;
                m_rects[m_rects_cnt << 1].y = m_hold_y;
                m_rects[(m_rects_cnt << 1) + 1].x = m_hold_x;
                m_rects[(m_rects_cnt << 1) + 1].y = m_hold_y;
                m_rects_cnt++;
                return true;
            }
            /// <summary>
            /// touch moving on line status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnLineTouchMove(Point point)
            {
                if (m_status != PDFV_STATUS.STA_LINE) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            /// <summary>
            /// touch end on line status.
            /// </summary>
            /// <param name="point"></param>
            /// <returns></returns>
            private bool OnLineTouchEnd(Point point)
            {
                if (m_status != PDFV_STATUS.STA_LINE) return false;
                m_rects[(m_rects_cnt << 1) - 1].x = (float)point.X;
                m_rects[(m_rects_cnt << 1) - 1].y = (float)point.Y;
                vDraw();
                return true;
            }
            /// <summary>
            /// perform annotation, must on annotation status.
            /// single tapped on the annotation enter annotation status.
            /// </summary>
            public void vAnnotPerform()
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return;
                int pageno = m_annot.Dest;
                if (pageno >= 0)//goto page
                {
                    if (m_listener != null)
                        m_listener.OnPDFAnnotGoto(pageno);
                    vAnnotEnd();
                    return;
                }
                if (m_annot.IsRemoteDest)
                {
                    if (m_listener != null)
                        m_listener.OnPDFAnnotRemoteDest(m_annot.RemoteDest);
                    vAnnotEnd();
                    return;
                }
                if (m_annot.IsFileLink)
                {
                    if (m_listener != null)
                        m_listener.OnPDFAnnotFileLink(m_annot.FileLink);
                    vAnnotEnd();
                    return;
                }
                if (m_annot.IsURI)//open url
                {
                    if (m_listener != null)
                        m_listener.OnPDFAnnotURI(m_annot.URI);
                    vAnnotEnd();
                    return;
                }
                if (m_annot.RichMediaItemCount > 0)     // Check if the annotation is a rich media container
                {
                    if (m_listener != null)
                        m_listener.OnPDFAnnotRichMediaAsync(m_annot);
                    vAnnotEnd();
                    return;
                }
                //if (m_annot.IsMovie)
                //{
                //    if (m_listener != null)
                //        m_listener.OnPDFAnnotMovieAsync(m_annot, m_annot.GetMovieName());
                //    vAnnotEnd();
                //    return;
                //}
                //if (m_annot.IsSound)
                //{
                //    if (m_listener != null)
                //        m_listener.OnPDFAnnotSound(m_annot, m_annot.GetSoundName());
                //    vAnnotEnd();
                //    return;
                //}
                if (m_annot.IsPopup && m_annot.Type == 1) //Nermeen add type check as it enters also for highlight annot
                {
                    //popup dialog to show text and subject.
                    //nuri is text content.
                    //subj is subject string.
                    if (m_listener != null)
                        m_listener.OnPDFAnnotPopup(m_annot, m_annot.PopupSubject, m_annot.PopupText);
                    vAnnotEnd();
                    return;
                }
                vAnnotEnd();
                return;
            }
            /// <summary>
            /// remove the selected annotation, must on annotation status.
            /// single tapped on the annotation enter annotation status.
            /// </summary>
            public void vAnnotRemove()
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return;
                bool r = m_annot.RemoveFromPage();
                vRenderSync(m_pages[m_annot_pos.pageno]);
                vDraw();
                vAnnotEnd();
            }
            /// <summary>
            /// end annotation status.
            /// </summary>
            public void vAnnotEnd()
            {
                if (m_status != PDFV_STATUS.STA_ANNOT) return;
                m_status = PDFV_STATUS.STA_NONE;
                m_scroller.IsEnabled = true;
                m_annot = null;
                vDraw();
                if (m_listener != null)
                    m_listener.OnPDFAnnotEnd();
            }
            /// <summary>
            /// enter Note status.
            /// int this status, you can tapped on page, to add each note.
            /// </summary>
            /// <returns></returns>
            public bool vNoteStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_scroller.IsEnabled = false;
                    m_status = PDFV_STATUS.STA_NOTE;
                }
                return true;
            }

            /// remove last note object last added, this mothed only works on note status.
            /// </summary>
            public void vNoteRemoveLast()
            {
                if (m_notes_cnt <= 0) return;
                PDFVPage vpage = m_notes[m_notes_cnt - 1].vpage;
                int index = m_notes[m_notes_cnt - 1].index;
                PDFPage page = vpage.GetPage();
                if (page != null)
                {
                    PDFAnnot annot = page.GetAnnot(index);
                    annot.RemoveFromPage();
                    m_notes[m_notes_cnt - 1].vpage = null;
                    m_notes_cnt--;
                    vRenderSync(vpage);
                    vDraw();
                }
            }
            /// <summary>
            /// cancel note status, all note object added to pdf file, on current note status, shall be cleared.
            /// </summary>
            public void vNoteCancel()
            {
                if (m_status == PDFV_STATUS.STA_NOTE)
                {
                    m_scroller.IsEnabled = true;
                    m_status = PDFV_STATUS.STA_NONE;
                    PDFVPage[] vpages = new PDFVPage[256];
                    int vpages_cnt = 0;
                    int index;
                    for (int cur = m_notes_cnt - 1; cur >= 0; cur--)
                    {
                        PDFVPage vpage = m_notes[cur].vpage;
                        for (index = 0; index < vpages_cnt; index++)
                        {
                            if (vpages[index] == vpage) break;
                        }
                        if (index >= vpages_cnt)
                        {
                            vpages[vpages_cnt] = vpage;
                            vpages_cnt++;
                        }
                        PDFPage page = vpage.GetPage();
                        if (page != null)
                        {
                            PDFAnnot annot = page.GetAnnot(m_notes[cur].index);
                            annot.RemoveFromPage();
                        }
                        m_notes[cur].vpage = null;
                    }
                    m_notes_cnt = 0;
                    for (index = 0; index < vpages_cnt; index++)
                    {
                        vRenderSync(vpages[index]);
                    }
                    vDraw();
                }
            }
            /// <summary>
            /// end note status.
            /// </summary>
            public void vNoteEnd()
            {
                if (m_status == PDFV_STATUS.STA_NOTE)
                {
                    m_scroller.IsEnabled = true;
                    m_status = PDFV_STATUS.STA_NONE;
                    if (m_notes_cnt > 0)
                        m_modified = true;
                    for (int cur = m_notes_cnt - 1; cur >= 0; cur--)
                        m_notes[cur].vpage = null;
                    m_notes_cnt = 0;
                    vDraw();
                }
            }
            private PDFInk m_ink;
            private Windows.UI.Xaml.Shapes.Path m_ink_path;
            private int m_ink_pos;
            /// <summary>
            /// enter ink status.
            /// users can draw inks by touching.
            /// you can invoke cancel(vInkCancel) or end(vInkEnd) on this status.
            /// </summary>
            /// <returns></returns>
            public bool vInkStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_scroller.IsEnabled = false;
                    m_ink = null;
                    m_ink_path = null;
                    m_ink_pos = 0;
                    m_status = PDFV_STATUS.STA_INK;
                    return true;
                }
                return false;
            }
            /// <summary>
            /// end ink status, and remove ink object.
            /// </summary>
            public void vInkCancel()
            {
                if (m_status == PDFV_STATUS.STA_INK)
                {
                    m_scroller.IsEnabled = true;
                    m_status = PDFV_STATUS.STA_NONE;
                    m_ink = null;
                    m_ink_path = null;
                    m_ink_pos = 0;
                    vDraw();
                }
            }
            /// <summary>
            /// end ink status. and apply the ink object.
            /// </summary>
            public void vInkEnd()
            {
                if (m_status == PDFV_STATUS.STA_INK)
                {
                    if (m_ink != null)
                    {
                        int pageno = vGetPage(m_hold_x, m_hold_y);

                        if (pageno >= 0)
                        {
                            PDFVPage vpage = m_pages[pageno];
                            float tmp = 1 / m_scroller.ZoomFactor;
                            PDFMatrix mat = new PDFMatrix(tmp, tmp, 0, 0);
                            mat.TransformInk(m_ink);
                            mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * tmp, (float)m_scroller.VerticalOffset * tmp);
                            PDFPage page = vpage.GetPage();
                            mat.TransformInk(m_ink);
                            page.AddAnnotInk(m_ink);
                            vRenderSync(vpage);
                            m_modified = true;
                        }
                    }
                    m_scroller.IsEnabled = true;
                    m_status = PDFV_STATUS.STA_NONE;
                    m_ink = null;
                    m_ink_path = null;
                    m_ink_pos = 0;
                    vDraw();
                }
            }
            private PDFPoint[] m_rects = new PDFPoint[256];
            private int m_rects_cnt = 0;
            struct PDFNoteRec
            {
                public PDFVPage vpage;
                public int index;
            };
            private PDFNoteRec[] m_notes = new PDFNoteRec[256];
            private int m_notes_cnt = 0;
            /// <summary>
            /// enter ellipse status.
            /// users can draw ovals by touching.
            /// you can invoke vEllipseCancel or vEllipseEnd to end this status.
            /// </summary>
            /// <returns></returns>
            public bool vEllipseStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_status = PDFV_STATUS.STA_ELLIPSE;
                    m_rects_cnt = 0;
                    m_scroller.IsEnabled = false;
                    return true;
                }
                return false;
            }
            /// <summary>
            /// end ellipse status, and remove all ellipse, added on current status.
            /// </summary>
            public void vEllipseCancel()
            {
                if (m_status == PDFV_STATUS.STA_ELLIPSE)
                {
                    m_scroller.IsEnabled = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;
                    vDraw();
                }
            }
            /// <summary>
            /// end ellipse status, and applied ovals.
            /// </summary>
            public void vEllipseEnd()
            {
                if (m_status == PDFV_STATUS.STA_ELLIPSE)
                {
                    PDFVPage[] pages = new PDFVPage[128];
                    int cur;
                    int end;
                    int pages_cnt = 0;
                    int pt_cur = 0;
                    int pt_end = m_rects_cnt * 2;
                    float tmp = 1 / m_scroller.ZoomFactor;
                    while (pt_cur < pt_end)
                    {
                        PDFRect rect;
                        PDFPoint pt0 = m_rects[pt_cur];
                        PDFPoint pt1 = m_rects[pt_cur + 1];
                        if (m_mode == PDFV_MODE.MODE_VERTICAL)
                        {
                            if (m_scroller.ExtentWidth < m_scroller.Width)
                            {
                                float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                                pt0.x -= offset;
                                pt1.x -= offset;
                            }
                        }
                        else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                        {
                            if (m_scroller.ExtentHeight < m_scroller.Height)
                            {
                                float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                                pt0.y -= offset;
                                pt1.y -= offset;
                            }
                        }
                        int pageno = vGetPage(pt0.x, pt0.y);
                        if (pageno >= 0)
                        {
                            PDFVPage vpage = m_pages[pageno];
                            cur = 0;
                            end = pages_cnt;
                            while (cur < end)
                            {
                                if (pages[cur] == vpage) break;
                                cur++;
                            }
                            if (cur >= end)
                            {
                                pages[cur] = vpage;
                                pages_cnt++;
                            }
                            if (pt0.x > pt1.x)
                            {
                                rect.right = pt0.x * tmp;
                                rect.left = pt1.x * tmp;
                            }
                            else
                            {
                                rect.left = pt0.x * tmp;
                                rect.right = pt1.x * tmp;
                            }
                            if (pt0.y > pt1.y)
                            {
                                rect.bottom = pt0.y * tmp;
                                rect.top = pt1.y * tmp;
                            }
                            else
                            {
                                rect.top = pt0.y * tmp;
                                rect.bottom = pt1.y * tmp;
                            }
                            PDFPage page = vpage.GetPage();
                            PDFMatrix mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * tmp, (float)m_scroller.VerticalOffset * tmp);
                            rect = mat.TransformRect(rect);
                            page.AddAnnotEllipse(rect, (ovalWidth * tmp) / vpage.GetScale(), ovalColor, 0);
                        }
                        pt_cur += 2;
                    }
                    if (m_rects.Length != 0)
                        m_modified = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;

                    cur = 0;
                    end = pages_cnt;
                    while (cur < end)
                    {
                        vRenderSync(pages[cur]);
                        cur++;
                    }
                    vDraw();
                    m_scroller.IsEnabled = true;
                }
            }
            /// <summary>
            /// enter rect status.
            /// users can draw rects by touching.
            /// you can invoke vRectCancel or vRectEnd to end this status.
            /// </summary>
            /// <returns></returns>
            public bool vRectStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_status = PDFV_STATUS.STA_RECT;
                    m_rects_cnt = 0;
                    m_scroller.IsEnabled = false;
                    return true;
                }
                return false;
            }
            /// <summary>
            /// end rect status, and remove all rects, added on current status.
            /// </summary>
            public void vRectCancel()
            {
                if (m_status == PDFV_STATUS.STA_RECT)
                {
                    m_scroller.IsEnabled = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;
                    vDraw();
                }
            }
            /// <summary>
            /// end rect status, and applied rects.
            /// </summary>
            public void vRectEnd()
            {
                if (m_status == PDFV_STATUS.STA_RECT)
                {
                    PDFVPage[] pages = new PDFVPage[128];
                    int cur;
                    int end;
                    int pages_cnt = 0;
                    int pt_cur = 0;
                    int pt_end = m_rects_cnt * 2;
                    float tmp = 1 / m_scroller.ZoomFactor;
                    while (pt_cur < pt_end)
                    {
                        PDFRect rect;
                        PDFPoint pt0 = m_rects[pt_cur];
                        PDFPoint pt1 = m_rects[pt_cur + 1];
                        if (m_mode == PDFV_MODE.MODE_VERTICAL)
                        {
                            if (m_scroller.ExtentWidth < m_scroller.Width)
                            {
                                float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                                pt0.x -= offset;
                                pt1.x -= offset;
                            }
                        }
                        else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                        {
                            if (m_scroller.ExtentHeight < m_scroller.Height)
                            {
                                float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                                pt0.y -= offset;
                                pt1.y -= offset;
                            }
                        }
                        int pageno = vGetPage(pt0.x, pt0.y);
                        if (pageno >= 0)
                        {
                            PDFVPage vpage = m_pages[pageno];
                            cur = 0;
                            end = pages_cnt;
                            while (cur < end)
                            {
                                if (pages[cur] == vpage) break;
                                cur++;
                            }
                            if (cur >= end)
                            {
                                pages[cur] = vpage;
                                pages_cnt++;
                            }
                            if (pt0.x > pt1.x)
                            {
                                rect.right = pt0.x * tmp;
                                rect.left = pt1.x * tmp;
                            }
                            else
                            {
                                rect.left = pt0.x * tmp;
                                rect.right = pt1.x * tmp;
                            }
                            if (pt0.y > pt1.y)
                            {
                                rect.bottom = pt0.y * tmp;
                                rect.top = pt1.y * tmp;
                            }
                            else
                            {
                                rect.top = pt0.y * tmp;
                                rect.bottom = pt1.y * tmp;
                            }
                            PDFPage page = vpage.GetPage();
                            PDFMatrix mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * tmp, (float)m_scroller.VerticalOffset * tmp);
                            rect = mat.TransformRect(rect);
                            page.AddAnnotRect(rect, (rectWidth * tmp) / vpage.GetScale(), rectColor, 0);
                        }
                        pt_cur += 2;
                    }
                    if (m_rects.Length != 0)
                        m_modified = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;

                    cur = 0;
                    end = pages_cnt;
                    while (cur < end)
                    {
                        vRenderSync(pages[cur]);
                        cur++;
                    }
                    vDraw();
                    m_scroller.IsEnabled = true;
                }
            }

            public bool vTextEditStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_status = PDFV_STATUS.STA_TEXT_EDIT;
                    m_rects_cnt = 0;
                    m_scroller.IsEnabled = false;
                    return true;
                }
                return false;
            }

            public void vTextEditCancel()
            {
                if (m_status == PDFV_STATUS.STA_TEXT_EDIT)
                {
                    m_scroller.IsEnabled = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;
                    vDraw();
                }
            }

            public void vTextEditEnd()
            {
                if (m_status == PDFV_STATUS.STA_TEXT_EDIT)
                {
                    PDFVPage[] pages = new PDFVPage[128];
                    int cur;
                    int end;
                    int pages_cnt = 0;
                    int pt_cur = 0;
                    int pt_end = m_rects_cnt * 2;
                    float tmp = 1 / m_scroller.ZoomFactor;
                    while (pt_cur < pt_end)
                    {
                        PDFRect rect;
                        PDFPoint pt0 = m_rects[pt_cur];
                        PDFPoint pt1 = m_rects[pt_cur + 1];
                        if (m_mode == PDFV_MODE.MODE_VERTICAL)
                        {
                            if (m_scroller.ExtentWidth < m_scroller.Width)
                            {
                                float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                                pt0.x -= offset;
                                pt1.x -= offset;
                            }
                        }
                        else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                        {
                            if (m_scroller.ExtentHeight < m_scroller.Height)
                            {
                                float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                                pt0.y -= offset;
                                pt1.y -= offset;
                            }
                        }
                        int pageno = vGetPage(pt0.x, pt0.y);
                        if (pageno >= 0)
                        {
                            PDFVPage vpage = m_pages[pageno];
                            cur = 0;
                            end = pages_cnt;
                            while (cur < end)
                            {
                                if (pages[cur] == vpage) break;
                                cur++;
                            }
                            if (cur >= end)
                            {
                                pages[cur] = vpage;
                                pages_cnt++;
                            }
                            if (pt0.x > pt1.x)
                            {
                                rect.right = pt0.x * tmp;
                                rect.left = pt1.x * tmp;
                            }
                            else
                            {
                                rect.left = pt0.x * tmp;
                                rect.right = pt1.x * tmp;
                            }
                            if (pt0.y > pt1.y)
                            {
                                rect.bottom = pt0.y * tmp;
                                rect.top = pt1.y * tmp;
                            }
                            else
                            {
                                rect.top = pt0.y * tmp;
                                rect.bottom = pt1.y * tmp;
                            }
                            PDFPage page = vpage.GetPage();
                            PDFMatrix mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * tmp, (float)m_scroller.VerticalOffset * tmp);
                            rect = mat.TransformRect(rect);
                            page.AddAnnotEditbox(rect, 0xFF0000FF, 2, 0x00FFFFFF, 12f, 0xFF0000FF);
                            //page.AddAnnotRect(rect, (rectWidth * tmp) / vpage.GetScale(), rectColor, 0);
                        }
                        pt_cur += 2;
                    }
                    if (m_rects.Length != 0)
                        m_modified = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;

                    cur = 0;
                    end = pages_cnt;
                    while (cur < end)
                    {
                        vRenderSync(pages[cur]);
                        cur++;
                    }
                    vDraw();
                    m_scroller.IsEnabled = true;
                }
            }

            ///<summary></summary>

            /// <summary>
            /// enter line status.
            /// users can draw lines by touching.
            /// you can invoke vLineCancel or vLineEnd to end this status.
            /// </summary>
            /// <returns></returns>
            public bool vLineStart()
            {
                if (m_doc == null || !m_doc.CanSave || m_pages == null) return false;
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_status = PDFV_STATUS.STA_LINE;
                    m_rects_cnt = 0;
                    m_scroller.IsEnabled = false;
                    return true;
                }
                return false;
            }
            /// <summary>
            /// end line status, and remove all lines, added on current status.
            /// </summary>
            public void vLineCancel()
            {
                if (m_status == PDFV_STATUS.STA_RECT)
                {
                    m_scroller.IsEnabled = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_LINE;
                    vDraw();
                }
            }
            /// <summary>
            /// end line status, and applied lines.
            /// </summary>
            public void vLineEnd()
            {
                if (m_status == PDFV_STATUS.STA_LINE)
                {
                    PDFVPage[] pages = new PDFVPage[128];
                    int cur;
                    int end;
                    int pages_cnt = 0;
                    int pt_cur = 0;
                    int pt_end = m_rects.Length;
                    float tmp = 1 / m_scroller.ZoomFactor;
                    while (pt_cur < pt_end)
                    {
                        PDFPoint pt0 = m_rects[pt_cur];
                        PDFPoint pt1 = m_rects[pt_cur + 1];
                        int pageno = vGetPage(pt0.x, pt0.y);
                        if (pageno >= 0)
                        {
                            PDFVPage vpage = m_pages[pageno];
                            cur = 0;
                            end = pages_cnt;
                            while (cur < end)
                            {
                                if (pages[cur] == vpage) break;
                                cur++;
                            }
                            if (cur >= end)
                            {
                                pages[cur] = vpage;
                                pages_cnt++;
                            }
                            pt0.x *= tmp;
                            pt0.y *= tmp;
                            pt1.x *= tmp;
                            pt1.y *= tmp;
                            PDFPage page = vpage.GetPage();
                            PDFMatrix mat = vpage.CreateInvertMatrix((float)m_scroller.HorizontalOffset * tmp, (float)m_scroller.VerticalOffset * tmp);
                            pt0 = mat.TransformPoint(pt0);
                            pt1 = mat.TransformPoint(pt1);
                            page.AddAnnotLine(pt0.x, pt0.y, pt1.x, pt1.y, 1, 0, (lineWidth * tmp) / vpage.GetScale(), lineColor, 0);
                        }
                        pt_cur += 2;
                    }
                    if (m_rects.Length != 0)
                        m_modified = true;
                    m_rects_cnt = 0;
                    m_status = PDFV_STATUS.STA_NONE;

                    cur = 0;
                    end = pages_cnt;
                    while (cur < end)
                    {
                        vRenderSync(pages[cur]);
                        cur++;
                    }
                    vDraw();
                    m_scroller.IsEnabled = true;
                }
            }
            protected float m_hold_x;
            protected float m_hold_y;
            private float m_shold_x;
            private float m_shold_y;
            protected bool m_pressed = false;

            private ulong m_tstamp;
            private ulong m_tstamp_tap;
            private void OnNoneTouchBegin(Point point, ulong timestamp)
            {
                m_tstamp = timestamp;
                m_tstamp_tap = m_tstamp;
                m_hold_x = (float)point.X;
                m_hold_y = (float)point.Y;
                m_shold_x = (float)m_scroller.HorizontalOffset;
                m_shold_y = (float)m_scroller.VerticalOffset;
                m_pressed = true;
            }

            private void OnNoneTouchMove(Point point, ulong timestamp)
            {
                if (m_pressed)
                {
                    ulong del = timestamp - m_tstamp;
                    if (del > 0)
                    {
                        double dx = point.X - m_hold_x;
                        double dy = point.Y - m_hold_y;
                        double vx = dx * 1000000 / del;
                        double vy = dy * 1000000 / del;
                        dx = 0;
                        dy = 0;
                        if (vx > 50 || vx < -50)
                            dx = vx;
                        if (vy > 50 || vy < -50)
                            dy = vy;
                        else if (timestamp - m_tstamp_tap > 1000000)//long pressed
                        {
                            dx = point.X - m_hold_x;
                            dy = point.Y - m_hold_y;
                            if (dx < 10 && dx > -10 && dy < 10 && dy > -10)
                            {
                                m_status = PDFV_STATUS.STA_NONE;
                                if (m_listener != null)
                                    m_listener.OnPDFLongPressed((float)point.X, (float)point.Y);
                            }
                        }
                    }
                    m_scroller.ChangeView(m_shold_x + m_hold_x - point.X, m_shold_y + m_hold_y - point.Y, m_scroller.ZoomFactor, true);
                }
            }
            /// <summary>
            /// double touch event.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            protected virtual void vOnDoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
            {
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    if (m_scroller.ZoomFactor * 1.2f < m_scroller.MaxZoomFactor)
                    {
                        Point point = e.GetPosition(m_scroller);
                        //PDFPos pos = vGetPos((float)point.X, (float)point.Y);
                        double offsetx = point.X * 0.2d;
                        double offsety = point.Y * 0.2d;
                        m_scroller.ChangeView(m_scroller.HorizontalOffset * 1.2d + offsetx, m_scroller.VerticalOffset * 1.2d + offsety, m_scroller.ZoomFactor * 1.2f, false);
                        //vSetPos(pos, (float)point.X, (float)point.Y);
                    }
                    else
                    {
                        double tmp = 1 / m_scroller.ZoomFactor;
                        m_scroller.ChangeView(m_scroller.HorizontalOffset * tmp, m_scroller.VerticalOffset * tmp, 1, false);
                    }
                    m_scroller.UpdateLayout();
                }
            }
            private PDFAnnot m_annot;
            private PDFPos m_annot_pos;
            private PDFRect m_annot_rect;
            /// <summary>
            /// single tapped event.
            /// enter annotation event, if single tapped on an annotation.
            /// return a callback, if no annotation tapped.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            private void vOnTapped(object sender, TappedRoutedEventArgs e)
            {
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    Point point = e.GetPosition(m_scroller);
                    m_annot_pos = vGetPos((float)point.X, (float)point.Y);
                    if (m_annot_pos.pageno >= 0)
                    {
                        PDFVPage vpage = m_pages[m_annot_pos.pageno];
                        if (vpage == null)//shall not happen
                        {
                            if (m_listener != null) m_listener.OnPDFSingleTapped((float)point.X, (float)point.Y);
                            return;
                        }
                        if (m_mode == PDFV_MODE.MODE_VERTICAL)
                        {
                            if (m_scroller.ExtentWidth < m_scroller.Width)
                            {
                                float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                                point.X -= offset;
                            }
                        }
                        else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                        {
                            if (m_scroller.ExtentHeight < m_scroller.Height)
                            {
                                float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                                point.Y -= offset;
                            }
                        }
                        m_annot_pos = vGetPos((float)point.X, (float)point.Y);
                        PDFPage page = vpage.GetPage();
                        if (page == null)
                        {
                            if (m_listener != null)
                            {
                                m_listener.OnPDFPageTapped(vpage);
                                m_listener.OnPDFSingleTapped((float)point.X, (float)point.Y);
                            }
                            return;
                        }
                        m_annot = page.GetAnnot(m_annot_pos.x, m_annot_pos.y);
                        if (m_annot != null)//enter annotation status.
                        {
                            m_scroller.IsEnabled = false;
                            m_status = PDFV_STATUS.STA_ANNOT;
                            m_annot_rect = m_annot.Rect;
                            m_annot_rect.left = vpage.GetX() + vpage.ToDIBX(m_annot_rect.left);
                            m_annot_rect.right = vpage.GetX() + vpage.ToDIBX(m_annot_rect.right);
                            float tmp = m_annot_rect.top;
                            m_annot_rect.top = vpage.GetY() + vpage.ToDIBY(m_annot_rect.bottom);
                            m_annot_rect.bottom = vpage.GetY() + vpage.ToDIBY(tmp);
                            tmp = m_scroller.ZoomFactor;
                            m_annot_rect.left *= tmp;
                            m_annot_rect.top *= tmp;
                            m_annot_rect.right *= tmp;
                            m_annot_rect.bottom *= tmp;
                            tmp = (float)m_scroller.HorizontalOffset;
                            m_annot_rect.left -= tmp;
                            m_annot_rect.right -= tmp;
                            tmp = (float)m_scroller.VerticalOffset;
                            m_annot_rect.top -= tmp;
                            m_annot_rect.bottom -= tmp;

                            m_shold_x = m_hold_x;
                            m_shold_y = m_hold_y;
                            handleFormFields(vpage); //nermeen
                            vDraw();
                            if (m_listener != null)
                            {
                                m_listener.OnPDFPageTapped(vpage);
                                m_listener.OnPDFAnnotClicked(page, m_annot);
                            }
                        }
                        else
                        {
                            if (m_listener != null)
                            {
                                m_listener.OnPDFPageTapped(vpage);
                                m_listener.OnPDFSingleTapped((float)point.X, (float)point.Y);
                            }
                        }
                    }
                }
            }

            private void handleFormFields(PDFVPage vpage)
            {
                int check = m_annot.GetCheckStatus();
                if (m_doc.CanSave && check >= 0)
                {
                    switch (check)
                    {
                        case 0:
                            m_annot.SetCheckValue(true);
                            break;
                        case 1:
                            m_annot.SetCheckValue(false);
                            break;
                        case 2:
                        case 3:
                            m_annot.SetRadio();
                            break;
                    }
                    vRenderSync(vpage);
                    vAnnotEnd();
                    m_modified = true;
                }
            }

            private bool m_touched = false;
            /// <summary>
            /// touch event
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            private void vOnTouchDown(object sender, PointerRoutedEventArgs e)
            {
                PointerPoint ppt = e.GetCurrentPoint(m_scroller);
                Point pt = ppt.Position;
                m_touched = true;
                if (OnSelTouchBegin(pt)) return;
                if (OnAnnotTouchBegin(pt)) return;
                if (OnNoteTouchBegin(pt)) return;
                if (OnInkTouchBegin(pt)) return;
                if (OnRectTouchBegin(pt)) return;
                if (OnEllipseTouchBegin(pt)) return;
                if (OnLineTouchBegin(pt)) return;
                if (OnEditTextBoxTouchBegin(pt)) return;
                //if (e.Pointer.PointerDeviceType == Windows.Devices.Input.PointerDeviceType.Mouse)
                //{
                //    if (ppt.Properties.IsLeftButtonPressed)
                //    {
                OnNoneTouchBegin(pt, ppt.Timestamp);
                //    }
                //}
            }
            /// <summary>
            /// touch moving event.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            private void vOnTouchMove(object sender, PointerRoutedEventArgs e)
            {
                PointerPoint ppt = e.GetCurrentPoint(m_scroller);
                Point pt = ppt.Position;
                if (m_touched)
                {
                    if (OnSelTouchMove(pt)) return;
                    if (OnAnnotTouchMove(pt)) return;
                    if (OnNoteTouchMove(pt)) return;
                    if (OnInkTouchMove(pt)) return;
                    if (OnRectTouchMove(pt)) return;
                    if (OnEllipseTouchMove(pt)) return;
                    if (OnLineTouchMove(pt)) return;
                    if (OnEditTextBoxTouchMove(pt)) return;
                }
                OnNoneTouchMove(pt, ppt.Timestamp);
            }
            /// <summary>
            /// touch end event.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            protected virtual void vOnTouchUp(object sender, PointerRoutedEventArgs e)
            {
                PointerPoint ppt = e.GetCurrentPoint(m_scroller);
                Point pt = ppt.Position;
                if (m_touched)
                {
                    m_touched = false;
                    if (OnSelTouchEnd(pt)) return;
                    if (OnAnnotTouchEnd(pt)) return;
                    if (OnNoteTouchEnd(pt)) return;
                    if (OnInkTouchEnd(pt)) return;
                    if (OnRectTouchEnd(pt)) return;
                    if (OnEllipseTouchEnd(pt)) return;
                    if (OnLineTouchEnd(pt)) return;
                    if (OnEditTextBoxTouchEnd(pt)) return;
                }
                if (m_pressed)
                    m_pressed = false;
            }
            private float m_oldZoom = 1;
            private void vOnViewChanged(Object sender, ScrollViewerViewChangedEventArgs arg)
            {
                m_touched = false;
                if (m_oldZoom != m_scroller.ZoomFactor)
                {
                    if (m_status == PDFV_STATUS.STA_NONE)
                        ZoomStart();
                    if (m_status == PDFV_STATUS.STA_ZOOM)
                        ZoomSet(m_scroller.ZoomFactor);
                    m_oldZoom = m_scroller.ZoomFactor;
                }
                else
                {
                    if (m_status == PDFV_STATUS.STA_ZOOM)
                    {
                        ZoomEnd();
                    }
                }
            }
            /// <summary>
            /// layout all pages.
            /// </summary>
            /// <param name="sender"></param>
            /// <param name="e"></param>
            private void OnLayout(object sender, object ele)
            {
                if (m_doc == null)
                    return;
                if (m_goto_pos.pageno >= 0)
                {
                    if (m_goto_pos.pageno >= m_doc.PageCount)
                        m_goto_pos.pageno = m_doc.PageCount - 1;
                    vSetPos(m_goto_pos, 0, 0);
                    m_goto_pos.pageno = -1;
                }
            }
            /// <summary>
            /// contruction function.
            /// </summary>
            /// <param name="parent">canvas for view attached.</param>
            public PDFView(Canvas parent)
            {
                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("View_mode"))
                    viewMode = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["View_mode"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ink_width"))
                    inkWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ink_width"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("ink_color"))
                    inkColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["ink_color"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Rect_width"))
                    rectWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Rect_width"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Rect_color"))
                    rectColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Rect_color"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ellipse_width"))
                    ovalWidth = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ellipse_width"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Ellipse_color"))
                    ovalColor = Convert.ToUInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Ellipse_color"]);

                if (Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey("Render_quality"))
                {
                    int value = Convert.ToInt32(Windows.Storage.ApplicationData.Current.LocalSettings.Values["Render_quality"]);
                    if (value == 0)
                        renderQuality = PDF_RENDER_MODE.mode_poor;
                    else if (value == 1)
                        renderQuality = PDF_RENDER_MODE.mode_normal;
                    else if (value == 2)
                        renderQuality = PDF_RENDER_MODE.mode_best;
                }

                m_parent = parent;
                m_parent.SizeChanged += vOnSizeChanged;
                m_parent.PointerPressed += vOnTouchDown;
                m_parent.PointerMoved += vOnTouchMove;
                m_parent.PointerReleased += vOnTouchUp;
                m_parent.PointerCanceled += vOnTouchUp;
                m_parent.PointerExited += vOnTouchUp;
                m_parent.Tapped += vOnTapped;
                m_parent.DoubleTapped += vOnDoubleTapped;

                m_scroller.ZoomMode = ZoomMode.Enabled;
                m_scroller.ViewChanged += vOnViewChanged;
                m_scroller.IsZoomChainingEnabled = false;
                m_scroller.LayoutUpdated += OnLayout;
                m_scroller.MinZoomFactor = 1;
                m_scroller.MaxZoomFactor = zoomLevel;
                //m_scroller.MouseLeftButtonDown += new MouseButtonEventHandler(vOnTouchDown);
                //m_scroller.MouseLeftButtonUp += new MouseButtonEventHandler(vOnTouchUp);
                //m_scroller.MouseMove += new MouseEventHandler(vOnTouchMove);
                m_scroller.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;
                m_scroller.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                //m_scroller.IsDeferredScrollingEnabled = true;
                m_scroller.IsHoldingEnabled = true;
                m_scroller.IsScrollInertiaEnabled = true;
                m_scroller.IsHitTestVisible = true;
                m_goto_pos.pageno = -1;
                m_goto_pos.x = 0;
                m_goto_pos.y = 0;

            }

            public Canvas vGetOffScreenCanvas() { return m_extra; }
            /// <summary>
            /// set background color.
            /// </summary>
            /// <param name="color"></param>
            public void vSetBackColor(uint color)
            {
                Color clr = new Color();
                clr.A = (byte)(color >> 24);
                clr.R = (byte)(color >> 16);
                clr.G = (byte)(color >> 8);
                clr.B = (byte)color;
                m_parent.Background = new SolidColorBrush(clr);
                vDraw();
            }
            /// <summary>
            /// set gap between pages.
            /// </summary>
            /// <param name="gap"></param>
            public void vSetPageGap(int gap)
            {
                m_page_gap = gap;
                PDFPos pos = vGetPos(0, 0);
                vLayout();
                vSetPos(pos, 0, 0);
                vDraw();
            }
            /// <summary>
            /// private method invoked when resize event arrived.
            /// </summary>
            /// <param name="w"></param>
            /// <param name="h"></param>
            protected virtual void vResize(float w, float h)
            {
                if (w == 0 || h == 0 || m_lock == 4) return;
                m_w = w;
                m_h = h;
                vLayout();
                ZoomStart();
                ZoomEnd();
                vDraw();
                if (m_listener != null)
                    m_listener.OnPDFPageChanged(vGetPage(m_w / 4, m_h / 4));
            }
            /// <summary>
            /// private method, draw on ink status.
            /// </summary>
            private void drawInk()
            {
                if (m_status == PDFV_STATUS.STA_INK && m_ink != null)
                {
                    float offsetx = 0;
                    float offsety = 0;
                    if (m_mode == PDFV_MODE.MODE_VERTICAL)
                    {
                        if (m_scroller.ExtentWidth < m_scroller.Width)
                        {
                            offsetx = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                            //point.X -= offset;
                        }
                    }
                    else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                    {
                        if (m_scroller.ExtentHeight < m_scroller.Height)
                        {
                            offsety = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                            //point.Y -= offset;
                        }
                    }
                    m_ink_pos = m_extra.draw_ink(m_ink, m_ink_path, m_ink_pos, offsetx, offsety);
                }
            }
            /// <summary>
            /// private method, draw on annotation status.
            /// </summary>
            private void drawAnnot()
            {
                if (m_status == PDFV_STATUS.STA_ANNOT)
                {
                    float dx = (float)(m_shold_x - m_hold_x);
                    float dy = (float)(m_shold_y - m_hold_y);
                    PDFRect rect = m_annot_rect;
                    //PDFRect rect = m_annot.Rect;
                    rect.left += dx;
                    rect.top += dy;
                    rect.right += dx;
                    rect.bottom += dy;
                    if (m_mode == PDFV_MODE.MODE_VERTICAL)
                    {
                        if (m_scroller.ExtentWidth < m_scroller.Width)
                        {
                            float offset = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                            rect.left += offset;
                            rect.right += offset;
                        }
                    }
                    else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                    {
                        if (m_scroller.ExtentHeight < m_scroller.Height)
                        {
                            float offset = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                            rect.top += offset;
                            rect.bottom += offset;
                        }
                    }
                    m_extra.draw_rect(rect, 1, 0xFF000000);
                }
            }
            /// <summary>
            /// private method, draw on rect status.
            /// </summary>
            private void drawRects()
            {
                if (m_status == PDFV_STATUS.STA_RECT && m_rects_cnt > 0)
                {
                    m_extra.draw_rects(m_rects, m_rects_cnt, rectWidth, rectColor);
                }
            }
            private void drawTextEditRects()
            {
                if (m_status == PDFV_STATUS.STA_TEXT_EDIT && m_rects_cnt > 0)
                {
                    m_extra.draw_rects(m_rects, m_rects_cnt, 2, 0xFF0000FF);
                }
            }
            /// <summary>
            /// private method, draw on ellipse status.
            /// </summary>
            private void drawEllipse()
            {
                if (m_status == PDFV_STATUS.STA_ELLIPSE && m_rects_cnt > 0)
                {
                    m_extra.draw_ovals(m_rects, m_rects_cnt, ovalWidth, ovalColor);
                }
            }
            /// <summary>
            /// private method, draw on line status.
            /// </summary>
            private void drawLines()
            {
                if (m_status == PDFV_STATUS.STA_LINE && m_rects_cnt > 0)
                {
                    m_extra.draw_lines(m_rects, m_rects_cnt, lineWidth, lineColor);
                }
            }
            /// <summary>
            /// draw the view.
            /// </summary>
            protected virtual void vDraw()
            {
                if (m_pages == null) return;
                vFlushRange();
                float left = m_curx;
                float top = m_cury;
                int cur = m_prange_start;
                int cnt = m_prange_end;
                m_extra.Children.Clear();
                m_extra.set_scale(m_scroller.ZoomFactor);
                PDFRect sel_rect1;
                PDFRect sel_rect2;
                sel_rect1.left = 0;
                sel_rect1.right = 0;
                sel_rect1.top = 0;
                sel_rect1.bottom = 0;
                sel_rect2.left = 0;
                sel_rect2.right = 0;
                sel_rect2.top = 0;
                sel_rect2.bottom = 0;
                float baseOffsetx = 0;
                float baseOffsety = 0;
                if (m_mode == PDFV_MODE.MODE_VERTICAL)
                {
                    if (m_scroller.ExtentWidth < m_scroller.Width)
                    {
                        baseOffsetx = (float)(m_scroller.Width - m_layout.Width * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                else if (m_mode == PDFV_MODE.MODE_HROIZONTAL || m_mode == PDFV_MODE.MODE_DUAL_PAGE)
                {
                    if (m_scroller.ExtentHeight < m_scroller.Height)
                    {
                        baseOffsety = (float)(m_scroller.Height - m_layout.Height * m_scroller.ZoomFactor) * 0.5f;
                    }
                }
                if (m_drawbmp)
                {
                    while (cur < cnt)
                    {
                        PDFVPage vpage = m_pages[cur];
                        if (m_status != PDFV_STATUS.STA_ZOOM) m_thread.start_render(vpage, m_is_thumb);
                        if (sel_rect1.left != sel_rect1.right && sel_rect2.left != sel_rect2.right)
                        {
                            sel_rect1 = vpage.GetSelRect1(left, top);
                            sel_rect2 = vpage.GetSelRect2(left, top);
                        }

                        vpage.Draw(m_extra, left, top, baseOffsetx, baseOffsety);
                        //not display find result while zooming.
                        //if (m_finder.find_get_page() == cur)
                        //    m_finder.find_draw(m_extra, vpage, left, top);
                        cur++;
                    }
                }
                else
                {
                    while (cur < cnt)
                    {
                        PDFVPage vpage = m_pages[cur];
                        m_thread.start_render(vpage, m_is_thumb);
                        if (sel_rect1.left != sel_rect1.right && sel_rect2.left != sel_rect2.right)
                        {
                            sel_rect1 = vpage.GetSelRect1(left, top);
                            sel_rect2 = vpage.GetSelRect2(left, top);
                        }
                        float tmp = 1 / vpage.GetZoom();
                        float offsetx = baseOffsetx * tmp;
                        float offsety = baseOffsety * tmp;
                        //vpage.Draw(m_extra, left, top, baseOffsetx, baseOffsety);
                        vpage.Draw(m_extra, left, top, offsetx, offsety);
                        if (m_finder.find_get_page() == cur)
                            m_finder.find_draw(m_extra, vpage, left, top, offsetx, offsety);

                        cur++;
                    }
                }
                if (m_listener != null)
                {
                    cur = m_prange_start;
                    cnt = m_prange_end;
                    while (cur < cnt)
                    {
                        m_listener.OnPDFPageDisplayed(m_extra, m_pages[cur]);
                        cur++;
                    }
                    if (sel_rect1.left != sel_rect1.right && sel_rect2.left != sel_rect2.right)
                        m_listener.OnPDFSelecting(m_extra, sel_rect1, sel_rect2);
                }
                drawAnnot();
                drawInk();
                drawEllipse();
                drawRects();
                drawTextEditRects();
                drawLines();
                m_parent.UpdateLayout();
            }
            /// <summary>
            /// open a pdf file to the view.
            /// a vClose is needed if the view closed, otherwise, memory leaks appears.
            /// </summary>
            /// <param name="doc">Document object.</param>
            /// <param name="page_gap">gap between pages</param>
            /// <param name="back_color">background color</param>
            /// <param name="listener">callback event object, if null, no events are returned.</param>
            public virtual void vOpen(PDFDoc doc, int page_gap, uint back_color, PDFViewListener listener)
            {
                vClose();
                m_doc = doc;
                m_layout = new PDFVLayout(m_scroller);
                m_parent.Children.Add(m_scroller);
                m_extra = new PDFVCanvas(m_parent, this);
                PDFVUIHandler handler;
                handler.disp = m_parent.Dispatcher;
                handler.render = new DispatchedHandler(vOnRender);
                handler.finder = new DispatchedHandler(vOnFound);
                handler.timer = new DispatchedHandler(vOnTimer);
                m_thread = new PDFVThread();
                m_thread.create(handler);
                m_page_gap = page_gap;
                vSetBackColor(back_color);
                m_finder = new PDFVFinder();
                m_listener = listener;
                vResize((int)m_parent.ActualWidth, (int)m_parent.ActualHeight);
                vLayout();
                vDraw();
            }
            /// <summary>
            /// get modified flag.
            /// </summary>
            /// <returns>true, if modified, or false</returns>
            public Boolean vIsModified()
            {
                return m_modified;
            }
            /// <summary>
            /// free memory used for the view.
            /// a vClose is needed if the view closed, otherwise, memory leaks appears.
            /// </summary>
            public virtual void vClose()
            {
                if (m_finder != null)
                {
                    m_finder.find_end();
                    m_finder = null;
                }
                if (m_pages != null)
                {
                    int cur = 0;
                    int cnt = m_pages.Length;
                    while (cur < cnt)
                    {
                        if (m_pages[cur] != null)
                        {
                            m_thread.end_render(m_pages[cur]);
                            m_thread.end_thumb(m_pages[cur]);
                        }
                        cur++;
                    }
                    m_layout.Children.Clear();
                    m_pages = null;
                }
                if (m_thread != null)
                {
                    m_thread.destroy();
                    m_thread = null;
                }
                if (m_layout != null)
                {
                    m_scroller.Content = null;
                    m_parent.Children.Remove(m_scroller);
                    m_scroller.Content = null;
                    m_layout = null;
                }
                if (m_extra != null)
                {
                    m_parent.Children.Remove(m_extra);
                    m_extra = null;
                }
                m_drawbmp = false;
                m_doc = null;
                m_modified = false;
            }
            /// <summary>
            /// layout all pages, the implement depends on derived class.
            /// </summary>
            protected virtual void vLayout()
            {
            }
            /// <summary>
            /// get PDFVPage object.
            /// </summary>
            /// <param name="pageno">0 based page NO.</param>
            /// <returns>PDFVPage object or null.</returns>
            public PDFVPage vGetPage(int pageno)
            {
                if (m_pages == null) return null;
                if (pageno < 0 || pageno >= m_pages.Length) return null;
                return m_pages[pageno];
            }
            /// <summary>
            /// get 0 based page NO by coordoinate.
            /// implement in derived class.
            /// </summary>
            /// <param name="x"></param>
            /// <param name="y"></param>
            /// <returns>page NO, or -1.</returns>
            protected virtual int vGetPage(float x, float y)
            {
                return 0;
            }
            private int m_prange_start = 0;
            private int m_prange_end = 0;
            protected int m_pageno = -1;
            protected PDFPos m_goto_pos;
            /// <summary>
            /// refresh pages range on screen.
            /// </summary>
            protected void vFlushRange()
            {
                if (m_status == PDFV_STATUS.STA_ZOOM) return;
                int pageno1 = vGetPage(0, 0);
                int pageno2 = vGetPage(m_w, m_h);
                if (pageno1 >= 0 && pageno2 >= 0)
                {
                    if (pageno1 > pageno2)
                    {
                        int tmp = pageno1;
                        pageno1 = pageno2;
                        pageno2 = tmp;
                    }
                    pageno2++;
                    if (m_prange_start < pageno1)
                    {
                        int start = m_prange_start;
                        int end = pageno1;
                        if (end > m_prange_end) end = m_prange_end;
                        while (start < end)
                        {
                            PDFVPage vpage = m_pages[start];
                            m_thread.end_render(vpage);
                            vpage.DeleteBmp();
                            start++;
                        }
                    }
                    if (m_prange_end > pageno2)
                    {
                        int start = pageno2;
                        int end = m_prange_end;
                        if (start < m_prange_start) start = m_prange_start;
                        while (start < end)
                        {
                            PDFVPage vpage = m_pages[start];
                            m_thread.end_render(vpage);
                            vpage.DeleteBmp();
                            start++;
                        }
                    }
                }
                else
                {
                    int start = m_prange_start;
                    int end = m_prange_end;
                    while (start < end)
                    {
                        PDFVPage vpage = m_pages[start];
                        m_thread.end_render(vpage);
                        vpage.DeleteBmp();
                        start++;
                    }
                }
                m_prange_start = pageno1;
                m_prange_end = pageno2;
                pageno1 = vGetPage(m_w / 4, m_h / 4);
                if (m_listener != null && pageno1 != m_pageno)
                {
                    m_listener.OnPDFPageChanged(m_pageno = pageno1);
                }
            }
            /// <summary>
            /// get Position from point in view coordinate, implement in derived class.
            /// pass (0,0) to get position of left-top corner
            /// </summary>
            /// <param name="vx"></param>
            /// <param name="vy"></param>
            /// <returns>position in PDF coordinate.</returns>
            public virtual PDFPos vGetPos(float vx, float vy)
            {
                PDFPos pos;
                pos.pageno = -1;
                pos.x = 0;
                pos.y = 0;
                if (m_pages == null || m_pages.Length <= 0) return pos;
                int pageno = vGetPage(vx, vy);
                if (pageno < 0) return pos;
                pos.pageno = pageno;
                float tmp = 1 / m_scroller.ZoomFactor;
                pos.x = m_pages[pageno].ToPDFX(vx * tmp, (float)m_scroller.HorizontalOffset * tmp);
                pos.y = m_pages[pageno].ToPDFY(vy * tmp, (float)m_scroller.VerticalOffset * tmp);
                return pos;
            }
            /// <summary>
            /// set Position to point in view coordinate, implement in derived class.
            /// pass (0,0) to set position to left-top corner. 
            /// </summary>
            /// <param name="pos">position in PDF coordinate.</param>
            /// <param name="vx"></param>
            /// <param name="vy"></param>
            public virtual void vSetPos(PDFPos pos, float vx, float vy)
            {
                if (m_w <= 0 || m_h <= 0 || m_pages == null)
                {
                    m_goto_pos = pos;
                    return;
                }
                if (pos.pageno < 0 || pos.pageno >= m_pages.Length) return;
                PDFVPage vpage = m_pages[pos.pageno];
                float x = vpage.GetX() + vpage.ToDIBX(pos.x);
                float y = vpage.GetY() + vpage.ToDIBY(pos.y);
                x *= m_scroller.ZoomFactor;
                y *= m_scroller.ZoomFactor;
                x -= vx;
                y -= vy;
                float docw = m_docw * m_scroller.ZoomFactor;
                float doch = m_doch * m_scroller.ZoomFactor;
                if (x > docw - m_w) x = docw - m_w;
                if (x < 0) x = 0;
                if (y > doch - m_h) y = doch - m_h;
                if (y < 0) y = 0;
                m_scroller.ChangeView(x, y, m_scroller.ZoomFactor, true);
                m_scroller.UpdateLayout();
                vDraw();
            }
            /// <summary>
            /// goto page NO.
            /// </summary>
            /// <param name="pageno">0 based page NO.</param>
            public virtual void vGotoPage(int pageno)
            {
                if (m_doc == null || pageno < 0) return;
                if (m_w <= 0 || m_h <= 0 || m_pages == null || pageno >= m_pages.Length)
                {
                    m_goto_pos.pageno = pageno;
                    if (m_goto_pos.pageno >= m_doc.PageCount)
                        m_goto_pos.pageno = m_doc.PageCount - 1;
                    m_goto_pos.x = 0;
                    m_goto_pos.y = m_doc.GetPageHeight(m_goto_pos.pageno);
                    return;
                }
                PDFVPage vpage = m_pages[pageno];
                float x = vpage.GetX() * m_scroller.ZoomFactor;
                float y = vpage.GetY() * m_scroller.ZoomFactor;
                float docw = m_docw * m_scroller.ZoomFactor;
                float doch = m_doch * m_scroller.ZoomFactor;
                if (x > docw - m_w) x = docw - m_w;
                if (x < 0) x = 0;
                if (y > doch - m_h) y = doch - m_h;
                if (y < 0) y = 0;
                m_scroller.ChangeView(x, y, m_scroller.ZoomFactor, true);
                m_scroller.UpdateLayout();
                vDraw();
            }
            /// <summary>
            /// zoom operations
            /// </summary>
            /// <param name="scale">scale value apply to view.</param>
            public void vZoom(float scale)
            {
                if (m_status != PDFV_STATUS.STA_NONE || m_pages == null) return;
                m_scroller.ChangeView(m_scroller.HorizontalOffset, m_scroller.VerticalOffset, scale, true);
            }
            /// <summary>
            /// private method for zooming start
            /// </summary>
            protected void ZoomStart()
            {

                if (m_status != PDFV_STATUS.STA_NONE || m_pages == null) return;
                m_drawbmp = true;
                m_status = PDFV_STATUS.STA_ZOOM;
            }
            /// <summary>
            /// private method for zooming end
            /// </summary>
            protected void ZoomEnd()
            {
                if (m_status != PDFV_STATUS.STA_ZOOM || m_pages == null) return;
                m_status = PDFV_STATUS.STA_NONE;
                int cur = 0;
                int end = m_pages.Length;
                while (cur < end)
                {
                    m_pages[cur].SetZoom(m_scroller.ZoomFactor);
                    cur++;
                }
                cur = m_prange_start;
                end = m_prange_end;
                while (cur < end)
                {
                    m_thread.start_render(m_pages[cur], m_is_thumb);
                    cur++;
                }
            }
            /// <summary>
            /// private method for zooming
            /// </summary>
            /// <param name="factor">scale factor</param>
            /// <param name="fx">x of fixed point</param>
            /// <param name="fy">y of fixed point</param>
            protected void ZoomSet(float scale)
            {
                if (m_status != PDFV_STATUS.STA_ZOOM || m_pages == null) return;
                int cur = m_prange_start;
                int cnt = m_prange_end;
                while (cur < cnt)
                {
                    m_pages[cur].CreateBmp();
                    m_thread.end_render(m_pages[cur]);
                    cur++;
                }
                m_drawbmp = true;
                vDraw();
            }
            /// <summary>
            /// get current zoom factor.
            /// </summary>
            /// <returns></returns>
            public float vGetScale()
            {
                return m_scale * m_scroller.ZoomFactor;
            }
            /// <summary>
            /// get min zoom factor.
            /// </summary>
            /// <returns></returns>
            public float vGetScaleMin()
            {
                return m_scale;
            }
            /// <summary>
            /// get max zoom factor.
            /// </summary>
            /// <returns></returns>
            public float vGetScaleMax()
            {
                return m_scale * zoomLevel;
            }
            /// <summary>
            /// re-render a page in async way.
            /// this means not block the UI thread.
            /// </summary>
            /// <param name="page"></param>
            public void vRenderAsync(PDFVPage page)
            {
                if (m_pages == null || page == null) return;
                m_thread.end_render(page);
                m_thread.start_render(page, m_is_thumb);
            }
            /// <summary>
            /// re-render a page in sync way.
            /// this means block the UI thread.
            /// </summary>
            /// <param name="page"></param>
            public void vRenderSync(PDFVPage page)
            {
                if (m_pages == null || page == null) return;
                m_thread.end_render(page);
                page.RenderPrepare(m_is_thumb);
                page.RenderSync();
            }
            /// <summary>
            /// set locks
            /// </summary>
            /// <param name="locked">
            /// 0: non-lock or unlock all.
            /// 1: lock horizontal moving.
            /// 2: lock vertical moving.
            /// 3: lock moving and zooming.
            /// </param>
            public void vSetLock(int locked)
            {
                if (locked == 1)
                {
                    m_scroller.HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled;
                    m_scroller.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                }
                else if (locked == 2)
                {
                    m_scroller.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;
                    m_scroller.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
                }
                else if (locked == 3)
                {
                    //m_scroller.HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled;
                    //m_scroller.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
                    m_scroller.IsEnabled = false;
                }
                else
                {
                    m_scroller.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;
                    m_scroller.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
                    m_scroller.IsEnabled = true;
                }
                m_lock = locked;
            }
            /// <summary>
            /// get lock value.
            /// </summary>
            /// <returns>
            /// 0: non-lock or unlock all.
            /// 1: lock horizontal moving.
            /// 2: lock vertical moving.
            /// 3: lock moving and zooming.
            /// </returns>
            public int vGetLock()
            {
                return m_lock;
            }
            /// <summary>
            /// enter text select status.
            /// </summary>
            public void vSelStart()
            {
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    m_status = PDFV_STATUS.STA_SELECT;
                    m_scroller.IsEnabled = false;
                }
            }
            /// <summary>
            /// end text select status.
            /// </summary>
            public void vSelEnd()
            {
                if (m_status == PDFV_STATUS.STA_SELECT)
                {
                    vSelCancel();
                    m_scroller.IsEnabled = true;
                    m_status = PDFV_STATUS.STA_NONE;
                }
            }
            /// <summary>
            /// cancel text select status.
            /// </summary>
            public void vSelCancel()
            {
                if (m_status == PDFV_STATUS.STA_SELECT)
                {
                    //Nermeen clear selection
                    int pageno = vGetPage(m_hold_x, m_hold_y);
                    if (pageno >= 0)
                    {
                        PDFVPage vpage = m_pages[pageno];
                        vpage.ClearSel();
                        vDraw();
                    }
                }
            }
            /// <summary>
            /// get selected text.
            /// </summary>
            /// <returns>null or selected text</returns>
            public String vSelGetText()
            {
                if (m_status != PDFV_STATUS.STA_SELECT) return null; ;
                int pageno = vGetPage(m_hold_x, m_hold_y);
                if (pageno >= 0)
                {
                    PDFVPage vpage = m_pages[pageno];
                    return vpage.GetSel();
                }
                return null;
            }
            /// <summary>
            /// set current selected text as text-markup annotation.
            /// </summary>
            /// <param name="color">color applied to markup annotation.</param>
            /// <param name="type">0:highlight 1:underline 2:strikeout</param>
            /// <returns>true or false.</returns>
            public bool vSelMarkup(uint color, int type)
            {
                if (m_status != PDFV_STATUS.STA_SELECT) return false;
                int pageno = vGetPage(m_hold_x, m_hold_y);
                if (pageno >= 0)
                {
                    PDFVPage vpage = m_pages[pageno];
                    vpage.SetSelMarkup(color, type);
                    vpage.ClearSel();
                    vRenderSync(vpage);
                    vDraw();
                    m_modified = true;
                    return true;
                }
                return false;
            }
            /// <summary>
            /// start to find text.
            /// </summary>
            /// <param name="key">key to find.</param>
            /// <param name="match_case">match case?</param>
            /// <param name="whole_word">whole word?</param>
            public void vFindStart(String key, Boolean match_case, Boolean whole_word)
            {
                if (m_pages == null) return;
                int pageno = vGetPage(0, 0);
                m_finder.find_end();
                m_finder.find_start(m_doc, pageno, key, match_case, whole_word);
            }
            /// <summary>
            /// private method goto find result.
            /// </summary>
            protected void vFindGoto()
            {
                if (m_pages == null) return;
                int pg = m_finder.find_get_page();
                if (pg < 0 || pg >= m_doc.PageCount) return;
                PDFRect pos = m_finder.find_get_pos();
                if (pos.left == pos.right) return;
                pos.left = m_pages[pg].ToDIBX(pos.left) + m_pages[pg].GetX();
                float tmp = pos.top;
                pos.top = m_pages[pg].ToDIBY(pos.bottom) + m_pages[pg].GetY();
                pos.right = m_pages[pg].ToDIBX(pos.right) + m_pages[pg].GetX();
                pos.bottom = m_pages[pg].ToDIBY(tmp) + m_pages[pg].GetY();
                pos.left *= m_scroller.ZoomFactor;
                pos.top *= m_scroller.ZoomFactor;
                pos.right *= m_scroller.ZoomFactor;
                pos.bottom *= m_scroller.ZoomFactor;
                float x = (float)m_scroller.HorizontalOffset;
                float y = (float)m_scroller.VerticalOffset;
                if (x > pos.left - m_w / 8) x = pos.left - m_w / 8;
                if (x < pos.right - m_w * 7 / 8) x = pos.right - m_w * 7 / 8;
                if (y > pos.top - m_h / 8) y = pos.top - m_h / 8;
                if (y < pos.bottom - m_h * 7 / 8) y = pos.bottom - m_h * 7 / 8;
                float docw = m_docw * m_scroller.ZoomFactor;
                float doch = m_doch * m_scroller.ZoomFactor;
                if (x > docw - m_w) x = docw - m_w;
                if (x < 0) x = 0;
                if (y > doch - m_h) y = doch - m_h;
                if (y < 0) y = 0;
                m_scroller.ChangeView(x, y, m_scroller.ZoomFactor, true);
                m_scroller.UpdateLayout();
                vDraw();
            }
            /// <summary>
            /// find operation.
            /// </summary>
            /// <param name="dir">-1 means find prev, 1 means find next.</param>
            /// <returns>0 if found. 1 if pending to find, -1 if not found.</returns>
            public int vFind(int dir)
            {
                if (m_pages == null) return -1;
                int ret = m_finder.find_prepare(dir);
                if (ret == 1)
                {
                    if (m_listener != null)
                        m_listener.OnPDFFound(true);
                    vFindGoto();
                    return 0;//succeeded
                }
                if (ret == 0)
                {
                    if (m_listener != null)
                        m_listener.OnPDFFound(false);
                    return -1;//failed
                }
                m_thread.start_find(m_finder);//need thread operation.
                return 1;
            }
            /// <summary>
            /// end find.
            /// </summary>
            public void vFindEnd()
            {
                if (m_pages == null) return;
                m_finder.find_end();
            }
            /// <summary>
            /// center to page.
            /// </summary>
            /// <param name="pageno"></param>
            public virtual void vCenterPage(int pageno)
            {
                if (m_pages == null || m_doc == null || m_w <= 0 || m_h <= 0) return;
                float left = m_pages[pageno].GetX() - m_page_gap / 2;
                float top = m_pages[pageno].GetY() - m_page_gap / 2;
                float w = m_pages[pageno].GetWidth() + m_page_gap;
                float h = m_pages[pageno].GetHeight() + m_page_gap;
                float x = left + (w - m_w) / 2;
                float y = top + (h - m_h) / 2;
                m_scroller.ChangeView(x, y, m_scroller.ZoomFactor, true);
            }
            /// <summary>
            /// get x in scrollview.
            /// </summary>
            /// <returns></returns>
            public float vGetX()
            {
                return (float)m_scroller.HorizontalOffset;
            }
            /// <summary>
            /// get y in scrollview.
            /// </summary>
            /// <returns></returns>
            public float vGetY()
            {
                return (float)m_scroller.VerticalOffset;
            }
            /// <summary>
            /// total x in scrollview.
            /// </summary>
            /// <returns></returns>
            public float vGetTotalX()
            {
                return m_docw * m_scroller.ZoomFactor;
            }
            /// <summary>
            /// total y in scrollview.
            /// </summary>
            /// <returns></returns>
            public float vGetTotalY()
            {
                return m_doch * m_scroller.ZoomFactor;
            }

            public PDFVPage getCurrentVPage()
            {
                if (m_pages != null)
                    return m_pages[m_pageno];
                else
                    return null;
            }
        }
        public class PDFViewVert : PDFView
        {
            public PDFViewVert(Canvas parent)
                : base(parent)
            {
                m_mode = PDFV_MODE.MODE_VERTICAL;
            }
            protected override void vLayout()
            {
                if (m_doc == null || m_w <= m_page_gap || m_h <= m_page_gap) return;
                //m_scroller.MinZoomFactor = m_w / 600;
                m_scroller.MinZoomFactor = 0.2f;
                m_scroller.MaxZoomFactor = zoomLevel * m_w / 600;
                ZoomEnd();
                m_scroller.Width = m_w;
                m_scroller.Height = m_h;
                if (m_pages != null) return;
                int cur = 0;
                int cnt = m_doc.PageCount;
                float maxw = 0;
                while (cur < cnt)
                {
                    float w = m_doc.GetPageWidth(cur);
                    if (maxw < w)
                    {
                        maxw = w;
                    }
                    cur++;
                }
                m_scale = ((float)(m_w - m_page_gap)) / maxw;
                m_pages = new PDFVPage[cnt];
                float top = m_page_gap / 2;
                float left = m_page_gap / 2;
                cur = 0;
                m_docw = 0;
                m_doch = 0;
                while (cur < cnt)
                {
                    float w = m_doc.GetPageWidth(cur);
                    m_pages[cur] = new PDFVPage(m_doc, cur);
                    m_pages[cur].SetRect(m_layout, left + (maxw - w) / 2 * m_scale, top, m_scale);
                    top += m_pages[cur].GetHeight() + m_page_gap;
                    if (m_docw < m_pages[cur].GetWidth()) m_docw = m_pages[cur].GetWidth();
                    if (maxw == w)
                        m_max_page = m_pages[cur];
                    cur++;
                }
                m_doch = top;
                m_docw += m_page_gap;
                float minh = m_doch * m_scroller.MinZoomFactor;
                if (minh < m_h)
                    m_doch = m_h / m_scroller.MinZoomFactor;
                m_layout.resize(m_docw, m_doch);
                m_scroller.UpdateLayout();
                m_extra.m_isVerticalMode = true;
            }
            protected override int vGetPage(float vx, float vy)
            {
                if (m_pages == null || m_pages.Length <= 0) return -1;
                int left = 0;
                int right = m_pages.Length - 1;
                float y = ((float)m_scroller.VerticalOffset + vy) / m_scroller.ZoomFactor;
                int gap = m_page_gap >> 1;
                while (left <= right)
                {
                    int mid = (left + right) >> 1;
                    PDFVPage pg1 = m_pages[mid];
                    if (y < pg1.GetY() - gap)
                    {
                        right = mid - 1;
                    }
                    else if (y > pg1.GetY() + pg1.GetHeight() + gap)
                    {
                        left = mid + 1;
                    }
                    else
                    {
                        return mid;
                    }
                }
                if (right < 0) return 0;
                else return m_pages.Length - 1;
            }
        }
        public class PDFViewThumb : PDFView
        {
            private int m_direction;
            private int m_cols;
            private int m_rows;
            static private int CELL_W = 256;
            static private int CELL_H = 256;
            public PDFViewThumb(Canvas parent, int dir)
                : base(parent)
            {
                m_scroller.ZoomMode = ZoomMode.Disabled;
                m_direction = dir;
                m_is_thumb = true;
                m_mode = PDFV_MODE.MODE_OTHER;
            }
            protected override void vLayout()
            {
                if (m_doc == null || m_w <= m_page_gap || m_h <= m_page_gap) return;
                m_scroller.MinZoomFactor = 1;
                m_scroller.MaxZoomFactor = 1;
                m_scroller.Width = m_w;
                m_scroller.Height = m_h;
                int cnt = m_doc.PageCount;
                int cur;
                if (m_pages == null)
                    m_pages = new PDFVPage[cnt];
                if (m_direction == 0)//horz
                {
                    m_rows = (int)m_h / (CELL_H + m_page_gap);
                    if (m_rows < 1) m_rows = 1;
                    m_cols = (m_doc.PageCount + m_rows - 1) / m_rows;
                    int x = 0;
                    int cell_h = (int)m_h / m_rows;
                    for (cur = 0; cur < cnt; cur++)
                    {
                        int row = (cur % m_rows);
                        int y = (cell_h) * row + ((cell_h - CELL_H) >> 1);
                        if (m_pages[cur] == null) m_pages[cur] = new PDFVPage(m_doc, cur);
                        float scale1 = CELL_W / m_doc.GetPageWidth(cur);
                        float scale2 = CELL_H / m_doc.GetPageHeight(cur);
                        if (scale1 > scale2) scale1 = scale2;
                        int w = (int)(m_doc.GetPageWidth(cur) * scale1);
                        int off = (CELL_W - w + m_page_gap) >> 1;
                        m_pages[cur].SetRect(m_layout, x + off, y, scale1);
                        if (row == m_rows - 1) x += (CELL_W + m_page_gap);
                    }
                }
                else//vert
                {
                    m_cols = (int)m_w / (CELL_W + m_page_gap);
                    if (m_cols < 1) m_cols = 1;
                    m_rows = (m_doc.PageCount + m_cols - 1) / m_cols;
                    int y = 0;
                    int cell_w = (int)m_w / m_cols;
                    for (cur = 0; cur < cnt; cur++)
                    {
                        int col = (cur % m_cols);
                        int x = cell_w * col + ((cell_w - CELL_W) >> 1);
                        if (m_pages[cur] == null) m_pages[cur] = new PDFVPage(m_doc, cur);
                        float scale1 = CELL_W / m_doc.GetPageWidth(cur);
                        float scale2 = CELL_H / m_doc.GetPageHeight(cur);
                        if (scale1 > scale2) scale1 = scale2;
                        int h = (int)(m_doc.GetPageHeight(cur) * scale1);
                        int off = (CELL_H - h + m_page_gap) >> 1;
                        m_pages[cur].SetRect(m_layout, x, y + off, scale1);
                        if (col == m_cols - 1) y += CELL_H + m_page_gap;
                    }
                }

                m_doch = (CELL_H + m_page_gap) * m_rows;
                m_docw = (CELL_W + m_page_gap) * m_cols;
                if (m_docw < m_w) m_docw = m_w;
                if (m_doch < m_h) m_doch = m_h;
                m_layout.resize(m_docw, m_doch);
                m_scroller.UpdateLayout();
            }
            protected override int vGetPage(float vx, float vy)
            {
                if (m_pages == null || m_pages.Length <= 0) return -1;
                int cnt = m_doc.PageCount;
                int pgno = 0;
                if (m_direction == 0)
                {
                    float x = (float)m_scroller.HorizontalOffset + vx;
                    float y = vy;
                    int cell_h = (int)m_h / m_rows;
                    int col = (int)x / (CELL_W + m_page_gap);
                    int row = (int)y / cell_h;
                    pgno = col * m_rows + row;
                }
                else
                {
                    float x = vx;
                    float y = (float)m_scroller.VerticalOffset + vy;
                    int cell_w = (int)m_w / m_cols;
                    int col = (int)x / cell_w;
                    int row = (int)y / (CELL_H + m_page_gap);
                    pgno = row * m_cols + col;
                }
                if (pgno >= cnt) return cnt - 1;
                else if (pgno < 0) return 0;
                else return pgno;
            }
            protected override void vResize(float w, float h)
            {
                int pgno = vGetPage(0, 0);
                base.vResize(w, h);
                vGotoPage(pgno);
            }
        }
        public class PDFViewHorz : PDFView
        {
            private Boolean m_rtol = false;
            public PDFViewHorz(Canvas parent)
                : base(parent)
            {
                m_mode = PDFV_MODE.MODE_HROIZONTAL;
            }
            public void vSetDirection(Boolean rtol)
            {
                m_rtol = rtol;
            }
            public override void vOpen(PDFDoc doc, int page_gap, uint back_color, PDFViewListener listener)
            {
                base.vOpen(doc, page_gap, back_color, listener);
                if (m_rtol)
                {
                    m_scroller.ChangeView(m_docw, m_scroller.VerticalOffset, m_scroller.ZoomFactor, true);
                }
            }
            protected override void vResize(float w, float h)
            {
                Boolean set = (m_rtol && (m_w <= 0 || m_h <= 0));
                base.vResize(w, h);
                if (set)
                    m_scroller.ChangeView(m_docw, m_scroller.VerticalOffset, m_scroller.ZoomFactor, true);
            }
            protected override void vLayout()
            {
                if (m_doc == null || m_w <= m_page_gap || m_h <= m_page_gap) return;
                //m_scroller.MinZoomFactor = m_h / 800;
                m_scroller.MinZoomFactor = 0.2f;
                m_scroller.MaxZoomFactor = zoomLevel * m_h / 800;
                ZoomEnd();
                m_scroller.Width = m_w;
                m_scroller.Height = m_h;
                if (m_pages != null) return;
                int cur = 0;
                int cnt = m_doc.PageCount;
                float maxh = 0;
                while (cur < cnt)
                {
                    float h = m_doc.GetPageHeight(cur);
                    if (maxh < h) maxh = h;
                    cur++;
                }
                m_scale = ((float)(m_h - m_page_gap)) / maxh;
                m_pages = new PDFVPage[cnt];
                float left = m_page_gap / 2;
                float top = m_page_gap / 2;
                m_docw = 0;
                m_doch = 0;
                if (m_rtol)
                {
                    cur = cnt - 1;
                    while (cur >= 0)
                    {
                        m_pages[cur] = new PDFVPage(m_doc, cur);
                        m_pages[cur].SetRect(m_layout, left, top, m_scale);
                        left += m_pages[cur].GetWidth() + m_page_gap;
                        if (m_doch < m_pages[cur].GetHeight()) m_doch = m_pages[cur].GetHeight();
                        cur--;
                    }
                }
                else
                {
                    cur = 0;
                    while (cur < cnt)
                    {
                        m_pages[cur] = new PDFVPage(m_doc, cur);
                        m_pages[cur].SetRect(m_layout, left, top, m_scale);
                        left += m_pages[cur].GetWidth() + m_page_gap;
                        if (m_doch < m_pages[cur].GetHeight()) m_doch = m_pages[cur].GetHeight();
                        cur++;
                    }
                }
                m_docw = left;
                m_doch += m_page_gap;
                float minw = m_docw * m_scroller.MinZoomFactor;
                if (minw < m_w)
                    m_docw = m_w / m_scroller.MinZoomFactor;
                m_layout.resize(m_docw, m_doch);
                m_scroller.UpdateLayout();
            }
            protected override int vGetPage(float vx, float vy)
            {
                if (m_pages == null || m_pages.Length <= 0) return -1;
                int left = 0;
                int right = m_pages.Length - 1;
                int gap = m_page_gap >> 1;
                float x = ((float)m_scroller.HorizontalOffset + vx) / m_scroller.ZoomFactor;
                if (!m_rtol)//ltor
                {
                    while (left <= right)
                    {
                        int mid = (left + right) >> 1;
                        PDFVPage pg1 = m_pages[mid];
                        if (x < pg1.GetX() - gap)
                        {
                            right = mid - 1;
                        }
                        else if (x > pg1.GetX() + pg1.GetWidth() + gap)
                        {
                            left = mid + 1;
                        }
                        else
                        {
                            return mid;
                        }
                    }
                }
                else//rtol
                {
                    while (left <= right)
                    {
                        int mid = (left + right) >> 1;
                        PDFVPage pg1 = m_pages[mid];
                        if (x < pg1.GetX() - gap)
                        {
                            left = mid + 1;
                        }
                        else if (x > pg1.GetX() + pg1.GetWidth() + gap)
                        {
                            right = mid - 1;
                        }
                        else
                        {
                            return mid;
                        }
                    }
                }
                if (right < 0) return 0;
                else return m_pages.Length - 1;
            }
        }

        public class PDFViewDual : PDFView
        {
            private Boolean[] m_vert_dual;
            private Boolean[] m_horz_dual;
            private Boolean m_rtol = false;

            private int mPageOnManipulation;
            private double mHorizontalOffsetOnManipulation;
            public struct PDFCell
            {
                public float left;
                public float right;
                public int page_left;
                public int page_right;
            }
            private PDFCell[] m_cells;
            public PDFViewDual(Canvas parent)
                : base(parent)
            {
                m_mode = PDFV_MODE.MODE_DUAL_PAGE;
                m_scroller.IsScrollInertiaEnabled = false;
                m_scroller.DirectManipulationStarted += M_scroller_DirectManipulationStarted;
                m_scroller.DirectManipulationCompleted += M_scroller_DirectManipulationCompleted;
            }

            private bool mIsManipulating = false;
            private bool mIsProcessed = false;

            private void M_scroller_DirectManipulationStarted(object sender, Object e)
            {
                mPageOnManipulation = getCurrentVPage().GetPageNo();
                mHorizontalOffsetOnManipulation = m_scroller.HorizontalOffset;
                mIsManipulating = true;
            }

            private void M_scroller_DirectManipulationCompleted(object sender, object e)
            {
                if (!mIsManipulating)
                    return;
                if (m_scroller.HorizontalOffset - 20 > mHorizontalOffsetOnManipulation && mPageOnManipulation < m_doc.PageCount - 1)
                {
                    if (m_horz_dual == null)
                        mPageOnManipulation += 2;
                    else if (m_horz_dual[mPageOnManipulation])
                        mPageOnManipulation += 2;
                    else
                        mPageOnManipulation++;
                }
                else if (mHorizontalOffsetOnManipulation - 20 > m_scroller.HorizontalOffset && mPageOnManipulation > 0)
                {
                    if (m_horz_dual == null)
                        mPageOnManipulation -= 2;
                    else if (m_horz_dual[mPageOnManipulation])
                        mPageOnManipulation -= 2;
                    else
                        mPageOnManipulation--;
                }
                vGotoPage(mPageOnManipulation);
                mHorizontalOffsetOnManipulation = m_scroller.HorizontalOffset;
                mIsManipulating = false;
                mIsProcessed = true;
            }



            protected override void vOnTouchUp(object sender, PointerRoutedEventArgs e)
            {
                base.vOnTouchUp(sender, e);
                if (mIsProcessed)
                {
                    mIsProcessed = false;
                    return;
                }
                if (m_status != PDFV_STATUS.STA_NONE)
                {
                    return;
                }
                PointerPoint ppt = e.GetCurrentPoint(m_scroller);
                Point pt = ppt.Position;
                PDFVPage vpage = getCurrentVPage();
                if (vpage == null)
                {
                    vGotoPage(0);
                    return;
                }
                int page = vpage.GetPageNo();
                if (pt.X < 25 || pt.X > m_parent.Width - 25 || pt.Y > m_parent.Height - 25 || pt.Y < 70)
                {
                    vGotoPage(page);
                    return;
                }

                double horzDistance = m_hold_x - pt.X;

                int newPage = page;
                if (horzDistance > 20)
                    newPage++;
                else if (horzDistance < -20)
                    newPage--;
                if (newPage != page)
                {
                    vGotoPage(newPage);
                }
            }

            protected override void vOnDoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
            {
                if (m_status == PDFV_STATUS.STA_NONE)
                {
                    if (m_scroller.ZoomFactor < m_scroller.MaxZoomFactor)
                    {
                        float fixx = m_w * 0.5f;
                        float fixy = m_h * 0.5f;
                        float zoom_val = m_scroller.MaxZoomFactor / m_scroller.ZoomFactor;
                        if (zoom_val > 1.2f) zoom_val = 1.2f;
                        m_scroller.ChangeView((m_scroller.HorizontalOffset + fixx) * zoom_val - fixx, (m_scroller.VerticalOffset + fixy) * zoom_val - fixy, m_scroller.ZoomFactor * zoom_val, true);
                    }
                    else
                    {
                        int ccur = 0;
                        int pageno = getCurrentVPage().GetPageNo();
                        while (ccur < m_cells.Length)
                        {
                            PDFCell cell = m_cells[ccur];
                            if (pageno == cell.page_left || pageno == cell.page_right)
                            {
                                float left = m_cells[ccur].left;
                                float w = (m_cells[ccur].right - left);
                                m_scroller.ChangeView(left + (w - m_w) / 2, m_scroller.VerticalOffset, 1, true);
                                break;
                            }
                            ccur++;
                        }
                        //float tmp = 1 / m_scroller.ZoomFactor;
                        //m_scroller.ChangeView(m_scroller.HorizontalOffset * tmp, m_scroller.VerticalOffset * tmp, 1, false);
                        //m_scroller.UpdateLayout();
                    }
                }
            }

            /**
             * set layout parameters.
             * @param verts applied duals flag for vertical screen
             * @param horzs applied duals flag for landscape screen<br/>
             * Element which set to true mean this cell treat as dual page, otherwise treat as single page.<br/>
             * For example, book has a cover(first page treat as single) just codes:<br/>
             * &nbsp;&nbsp;verts = null;<br/>
             * &nbsp;&nbsp;horzs = new boolean[1];<br/>
             * &nbsp;&nbsp;horzs[1] = false;<br/>
             * Pages, those out of array bound:<br/>
             * in vertical screen: treat as single page(false).<br/>
             * in landscape screen: treat as dual page(true).<br/>
             */
            public void vSetLayoutPara(Boolean[] verts, Boolean[] horzs, Boolean rtol)
            {
                m_vert_dual = verts;
                m_horz_dual = horzs;
                m_rtol = rtol;
                vLayout();
                vDraw();
            }
            public override void vOpen(PDFDoc doc, int page_gap, uint back_color, PDFViewListener listener)
            {
                base.vOpen(doc, page_gap, back_color, listener);
                if (m_rtol)
                    m_scroller.ChangeView(m_docw, m_scroller.VerticalOffset, m_scroller.ZoomFactor, true);
            }
            protected override void vResize(float w, float h)
            {
                Boolean set = (m_rtol && (m_w <= 0 || m_h <= 0));
                base.vResize(w, h);
                if (set)
                    m_scroller.ChangeView(m_docw, m_scroller.VerticalOffset, m_scroller.ZoomFactor, true);
            }
            public override void vGotoPage(int pageno)
            {
                vCenterPage(pageno);
            }

            public override void vSetPos(PDFPos pos, float vx, float vy)
            {
                vCenterPage(pos.pageno);
            }

            protected override void vLayout()
            {
                if (m_doc == null || m_w <= m_page_gap || m_h <= m_page_gap) return;
                int pcur = 0;
                int pcnt = m_doc.PageCount;
                int ccur = 0;
                int ccnt = 0;
                float max_w = 0;
                float max_h = 0;
                if (m_pages == null) m_pages = new PDFVPage[pcnt];
                if (m_h > m_w || m_w < 800) //single mode
                {
                    while (pcur < pcnt)
                    {
                        if (m_vert_dual != null && ccnt < m_vert_dual.Length && m_vert_dual[ccnt] && pcur < pcnt - 1)
                        {
                            float w = m_doc.GetPageWidth(pcur) + m_doc.GetPageWidth(pcur + 1);
                            if (max_w < w) max_w = w;
                            float h = m_doc.GetPageHeight(pcur);
                            if (max_h < h) max_h = h;
                            h = m_doc.GetPageHeight(pcur + 1);
                            if (max_h < h) max_h = h;
                            pcur += 2;
                        }
                        else
                        {
                            float w = m_doc.GetPageWidth(pcur);
                            if (max_w < w) max_w = w;
                            float h = m_doc.GetPageHeight(pcur);
                            if (max_h < h) max_h = h;
                            pcur++;
                        }
                        ccnt++;
                    }
                    m_scale = ((float)(m_w - m_page_gap)) / max_w;
                    //float tmph = m_h * 600 / m_w;
                    //m_scroller.MinZoomFactor = m_w / 600;
                    //m_scroller.MaxZoomFactor = zoomLevel * m_w / 600;
                    //ZoomEnd();
                    //m_doch = (max_h * m_scale) + m_page_gap;
                    //if (m_doch < tmph) m_doch = tmph;
                    float scale = ((float)(m_h - m_page_gap)) / max_h;
                    if (m_scale > scale) m_scale = scale;

                    m_scroller.MinZoomFactor = 1;
                    m_scroller.MaxZoomFactor = zoomLevel;
                    ZoomEnd();
                    m_doch = (max_h * m_scale) + m_page_gap;
                    if (m_doch < (int)m_h) m_doch = (int)m_h;
                    m_cells = new PDFCell[ccnt];
                    pcur = 0;
                    ccur = 0;
                    int left = 0;
                    while (ccur < ccnt)
                    {
                        PDFCell cell = new PDFCell();
                        int w = 0;
                        int cw = 0;
                        if (m_vert_dual != null && ccur < m_vert_dual.Length && m_vert_dual[ccur] && pcur < pcnt - 1)
                        {
                            w = (int)((m_doc.GetPageWidth(pcur) + m_doc.GetPageWidth(pcur + 1)) * m_scale);
                            if (w + m_page_gap < (int)m_w) cw = (int)m_w;
                            else cw = w + m_page_gap;
                            cell.page_left = pcur;
                            cell.page_right = pcur + 1;
                            cell.left = left;
                            cell.right = left + cw;
                            if (m_pages[pcur] == null) m_pages[pcur] = new PDFVPage(m_doc, pcur);
                            if (m_pages[pcur + 1] == null) m_pages[pcur + 1] = new PDFVPage(m_doc, pcur + 1);
                            m_pages[pcur].SetRect(m_layout, left + (cw - w) / 2,
                                    (int)(m_doch - m_doc.GetPageHeight(pcur) * m_scale) / 2, m_scale);
                            m_pages[pcur + 1].SetRect(m_layout, m_pages[pcur].GetX() + m_pages[pcur].GetWidth(),
                                    (int)(m_doch - m_doc.GetPageHeight(pcur + 1) * m_scale) / 2, m_scale);
                            pcur += 2;
                        }
                        else
                        {
                            w = (int)(m_doc.GetPageWidth(pcur) * m_scale);
                            if (w + m_page_gap < (int)m_w) cw = (int)m_w;
                            else cw = w + m_page_gap;
                            cell.page_left = pcur;
                            cell.page_right = -1;
                            cell.left = left;
                            cell.right = left + cw;
                            if (m_pages[pcur] == null) m_pages[pcur] = new PDFVPage(m_doc, pcur);
                            m_pages[pcur].SetRect(m_layout, left + (cw - w) / 2,
                                    (int)(m_doch - m_doc.GetPageHeight(pcur) * m_scale) / 2, m_scale);
                            pcur++;
                        }
                        left += cw;
                        m_cells[ccur] = cell;
                        ccur++;
                    }
                    m_docw = left;
                }
                else
                {
                    while (pcur < pcnt)
                    {
                        if ((m_horz_dual == null || ccnt >= m_horz_dual.Length || m_horz_dual[ccnt]) && pcur < pcnt - 1)
                        {
                            float w = m_doc.GetPageWidth(pcur) + m_doc.GetPageWidth(pcur + 1);
                            if (max_w < w) max_w = w;
                            float h = m_doc.GetPageHeight(pcur);
                            if (max_h < h) max_h = h;
                            h = m_doc.GetPageHeight(pcur + 1);
                            if (max_h < h) max_h = h;
                            pcur += 2;
                        }
                        else
                        {
                            float w = m_doc.GetPageWidth(pcur);
                            if (max_w < w) max_w = w;
                            float h = m_doc.GetPageHeight(pcur);
                            if (max_h < h) max_h = h;
                            pcur++;
                        }
                        ccnt++;
                    }
                    m_scale = ((float)(m_w - m_page_gap)) / max_w;
                    float scale = ((float)(m_h - m_page_gap)) / max_h;
                    if (m_scale > scale) m_scale = scale;
                    m_scroller.MinZoomFactor = 1;
                    m_scroller.MaxZoomFactor = zoomLevel;
                    ZoomEnd();
                    m_doch = (int)(max_h * m_scale) + m_page_gap;
                    if (m_doch < (int)m_h) m_doch = (int)m_h;
                    m_cells = new PDFCell[ccnt];
                    pcur = 0;
                    ccur = 0;
                    int left = 0;
                    while (ccur < ccnt)
                    {
                        PDFCell cell = new PDFCell();
                        int w = 0;
                        int cw = 0;
                        if ((m_horz_dual == null || ccur >= m_horz_dual.Length || m_horz_dual[ccur]) && pcur < pcnt - 1)
                        {
                            w = (int)((m_doc.GetPageWidth(pcur) + m_doc.GetPageWidth(pcur + 1)) * m_scale);
                            if (w + m_page_gap < (int)m_w) cw = (int)m_w;
                            else cw = w + m_page_gap;
                            cell.page_left = pcur;
                            cell.page_right = pcur + 1;
                            cell.left = left;
                            cell.right = left + cw;
                            if (m_pages[pcur] == null) m_pages[pcur] = new PDFVPage(m_doc, pcur);
                            if (m_pages[pcur + 1] == null) m_pages[pcur + 1] = new PDFVPage(m_doc, pcur + 1);
                            m_pages[pcur].SetRect(m_layout, left + (cw - w) / 2,
                                    (int)(m_doch - m_doc.GetPageHeight(pcur) * m_scale) / 2, m_scale);
                            m_pages[pcur + 1].SetRect(m_layout, m_pages[pcur].GetX() + m_pages[pcur].GetWidth(),
                                    (int)(m_doch - m_doc.GetPageHeight(pcur + 1) * m_scale) / 2, m_scale);
                            pcur += 2;
                        }
                        else
                        {
                            w = (int)(m_doc.GetPageWidth(pcur) * m_scale);
                            if (w + m_page_gap < (int)m_w) cw = (int)m_w;
                            else cw = w + m_page_gap;
                            cell.page_left = pcur;
                            cell.page_right = -1;
                            cell.left = left;
                            cell.right = left + cw;
                            if (m_pages[pcur] == null) m_pages[pcur] = new PDFVPage(m_doc, pcur);
                            m_pages[pcur].SetRect(m_layout, left + (cw - w) / 2,
                                    (int)(m_doch - m_doc.GetPageHeight(pcur) * m_scale) / 2, m_scale);
                            pcur++;
                        }
                        left += cw;
                        m_cells[ccur] = cell;
                        ccur++;
                    }
                    m_docw = left;
                }
                if (m_rtol)
                {
                    ccur = 0;
                    pcur = 0;
                    while (ccur < ccnt)
                    {
                        PDFCell cell = m_cells[ccur];
                        float tmp = cell.left;
                        cell.left = m_docw - cell.right;
                        cell.right = m_docw - tmp;
                        if (cell.page_right >= 0)
                        {
                            int pg = cell.page_left;
                            cell.page_left = cell.page_right;
                            cell.page_right = pg;
                        }
                        ccur++;
                    }
                    while (pcur < pcnt)
                    {
                        PDFVPage vpage = m_pages[pcur];
                        vpage.m_x = m_docw - (vpage.m_x + vpage.m_w);
                        pcur++;
                    }
                }
                m_layout.resize(m_docw, m_doch);
                m_scroller.Width = m_w;
                m_scroller.Height = m_h;
                m_scroller.UpdateLayout();
            }
            protected override int vGetPage(float vx, float vy)
            {
                if (m_pages == null || m_pages.Length <= 0) return -1;
                int left = 0;
                int right = m_cells.Length - 1;
                float x = ((float)m_scroller.HorizontalOffset + vx) / m_scroller.ZoomFactor;
                if (!m_rtol)//ltor
                {
                    while (left <= right)
                    {
                        int mid = (left + right) >> 1;
                        PDFCell pg1 = m_cells[mid];
                        if (x < pg1.left)
                        {
                            right = mid - 1;
                        }
                        else if (x > pg1.right)
                        {
                            left = mid + 1;
                        }
                        else
                        {
                            PDFVPage vpage = m_pages[pg1.page_left];
                            if (pg1.page_right >= 0 && x > vpage.GetX() + vpage.GetWidth())
                                return pg1.page_right;
                            else
                                return pg1.page_left;
                        }
                    }
                }
                else//rtol
                {
                    while (left <= right)
                    {
                        int mid = (left + right) >> 1;
                        PDFCell pg1 = m_cells[mid];
                        if (x < pg1.left)
                        {
                            left = mid + 1;
                        }
                        else if (x > pg1.right)
                        {
                            right = mid - 1;
                        }
                        else
                        {
                            PDFVPage vpage = m_pages[pg1.page_left];
                            if (pg1.page_right >= 0 && x > vpage.GetX() + vpage.GetWidth())
                                return pg1.page_right;
                            else
                                return pg1.page_left;
                        }
                    }
                }
                if (right < 0)
                {
                    return 0;
                }
                else
                {
                    return m_pages.Length - 1;
                }
            }
            public override void vCenterPage(int pageno)
            {
                if (m_pages == null || m_doc == null || m_w <= 0 || m_h <= 0)
                {
                    m_goto_pos.pageno = pageno;
                    m_goto_pos.x = 0;
                    m_goto_pos.y = 0;
                    return;
                }
                int ccur = 0;
                while (ccur < m_cells.Length)
                {
                    PDFCell cell = m_cells[ccur];
                    if (pageno == cell.page_left || pageno == cell.page_right)
                    {
                        float left = m_cells[ccur].left * m_scroller.ZoomFactor;
                        float w = (m_cells[ccur].right - m_cells[ccur].left) * m_scroller.ZoomFactor;
                        m_scroller.ChangeView(left + (w - m_w) / 2, m_scroller.VerticalOffset, m_scroller.ZoomFactor, true);
                        break;
                    }
                    ccur++;
                }
            }
        }
    }
}
